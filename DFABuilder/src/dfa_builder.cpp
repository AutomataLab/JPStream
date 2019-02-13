#include "dfa_builder.h"
#include "pass.hpp"
#include "Lexer/dfa_compressed.hpp"
#include "Lexer/ecset_converter.hpp"
#include "Lexer/regex_builder.hpp"
#include "Lexer/model_core.hpp"
#include "Lexer/dfa_mixer.hpp"
#include "Lexer/set_converter.hpp"
#include "Lexer/regex_model.hpp"

#include "jsonpath_model.h"
#include "jsonpath_evaluator.h"
#include "jsonpath_parser.h"

#include <vector>
#include <string>
#include <climits>
#include <sstream>
#include <unordered_map>

using namespace std;
using namespace dragontooth;


typedef enum SEType {
    set_dot_all = 0, set_parent_all, 
    set_dot_property, set_parent_property,
    set_array_all, set_array_index
} SEType;


static char* str_copy(const char* src) {
    int len = strlen(src);
    char* buffer = (char*) malloc(len+1);
    strcpy(buffer, src);
    buffer[len] = '\0';
    return buffer;
}

struct StackElement {
    SEType stype;
    union {
        const char* str;
        pair<int, int> range;
    };

    StackElement(): stype(set_dot_all), range(0, 0) {}
    StackElement(SEType stype) : stype(stype), range(0, 0) { }
    StackElement(SEType stype, const char* _str) : stype(stype), str(str_copy(_str)) {}
    StackElement(SEType stype, int a, int b) : stype(stype), range(a, b) {}
    StackElement(const StackElement& other) {
        this->stype = other.stype;
        if (stype == set_dot_property || stype == set_parent_property) {
            this->str = str_copy(other.str);
        } else 
            this->range = other.range;
    }
    StackElement(StackElement&& other) noexcept {
        this->stype = other.stype; other.stype = set_dot_all;
        if (stype == set_dot_property || stype == set_parent_property) {
            this->str = other.str; other.str = nullptr;
        } else {
            this->range = other.range; other.range = pair<int,int>(0, 0);
        }
    }
    ~StackElement() {
        if (stype == set_dot_property || stype == set_parent_property) {
            if (str) free((void*)str);
        }
    }

};

struct StackContext {
    RegexModel* model;
    JSONPathNode* root;
    vector<StackElement> st;
    bool last_st_generated;
    unordered_map<string, int> name_mapping;
    // 0 is end, 1 is other, 2 is array 3-n are names
    vector<string> input_mapping;
    vector<JSONPathNode*> filter_trees;
    vector< pair<int, int> > array_range;

    map<int, vector<int> > states_mapping;
    int state_handle_now;
    vector<JSONPathNode*> tree_mapping;
    set<int> output_states;

    StackContext(RegexModel* model, JSONPathNode* root) : model(model), root(root), st(0), input_mapping(3), array_range(0) {}
    ~StackContext() {}

    void push(StackElement&& s) {
        last_st_generated = false;
        st.push_back(std::forward<StackElement>(s));
    }

    void print() {
        printf("$");
        for (auto& se: st) {
            switch (se.stype) {
                case set_dot_all:           printf(".*"); break;
                case set_parent_all:        printf("..*"); break;
                case set_dot_property:      printf(".%s", se.str); break;
                case set_parent_property:   printf("..%s", se.str); break;
                case set_array_all:         printf("[*]"); break;
                case set_array_index:       printf("[%d:%d]", se.range.first, se.range.second); break;
            }
        }
        printf("\n");
    }

    void print_header() {
        printf("\tother\tarray");
        for (int i = 3; i < input_mapping.size(); ++i) {
            printf("\t%s", input_mapping[i].c_str());
        }
        printf("\n");
    }

    void create_dfa() {
        print();
        last_st_generated = true;
        for (auto& se: st) 
            if (se.stype == set_dot_property || 
                se.stype == set_parent_property) {
                    string str(se.str);
                if (name_mapping.find(str) == name_mapping.end()) {
                    name_mapping.insert({str, input_mapping.size()});
                    input_mapping.push_back(str);
                }
            }
        
        RegexList* list = new RegexList();

        for (auto& se: st) {
            switch (se.stype) {
                case set_dot_all: { 
                    list->Add(new RegexSet("^"));
                    break; 
                }
                case set_parent_all: { 
                    auto* s = new RegexSet("^");
                    s->setOpt(RegexItem::re_nonzero_repetition);
                    list->Add(s);
                    break; 
                }
                case set_dot_property: { 
                    list->Add(new RegexChar(name_mapping[se.str])); 
                    break; 
                }
                case set_parent_property: { 
                    int ch = name_mapping[se.str];
                    auto* s = new RegexSet("^");
                    s->setOpt(RegexItem::re_repetition);
                    list->Add(s);
                    list->Add(new RegexChar(ch)); 
                    break; 
                }
                case set_array_all: { list->Add(new RegexChar(2)); break; }
                case set_array_index: { list->Add(new RegexChar(2)); break; }
            }
        }
        model->Add(list);
        array_range.push_back({0,0});
        tree_mapping.push_back(NULL);
    }

    void construct_filter(JSONPathNode* node) {
        if (node->node_type == jnt_variable) {
            std::string str(node->string);
            std::istringstream tokenStream(str);
            std::string token;
            int count = 0;
            while (std::getline(tokenStream, token, '.')) {
                count++;
                push({set_dot_property, token.c_str()});
            }
            create_dfa();
            states_mapping[state_handle_now].push_back(model->size()-1);
            tree_mapping[model->size()-1] = node;
            for (int i = 0; i < count; ++i)
                st.pop_back();
        }

        if (node->left) {
            construct_filter(node->left);
        }
        if (node->right) {
            construct_filter(node->right);
        }

    }

    void construct_dfa(JSONPathNode* node) {
        if (node->left) {
            construct_dfa(node->left);
        }

        switch (node->node_type) {
        case jnt_concat: {
            // .*
            if (node->right && node->right->node_type == jnt_wildcard)
                push({set_dot_all});
            // .property
            if (node->right && node->right->node_type == jnt_id) 
                push({set_dot_property, node->right->string});
            break;
        }
        case jnt_parent_concat: {
            // ..*
            if (node->right && node->right->node_type == jnt_wildcard) 
                push({set_parent_all});
            // ..property
            if (node->right && node->right->node_type == jnt_id) 
                push({set_parent_property, node->right->string});
            break;
        }
        case jnt_predicate: {
            // []
            JSONPathNode* p = node->right;
            if (p) {
                switch (p->node_type) {
                // [start:end]
                case jnt_range: {
                    int begin, end;
                    if (p->left) begin = p->left->number;
                    else begin = INT_MIN;
                    if (p->right) end = p->right->number;
                    else end = INT_MAX;
                    push({set_array_index, begin, end});
                    create_dfa();
                    array_range[model->size()-1] = {begin, end};
                    break;
                }
                // [*]
                case jnt_wildcard: {
                    push({set_array_all});
                    break;
                }
                // [?()]
                case jnt_fliter: {
                    push({set_array_all});
                    create_dfa();
                    evaluatorModifyReference(p);
                    state_handle_now = model->size()-1;
                    states_mapping[state_handle_now] = vector<int>(0);
                    filter_trees.push_back(p->left);
                    tree_mapping[state_handle_now] = p->left;
                    construct_filter(p->left);
                    break;
                }
                default:
                    break;
                }
            }
            break;
        }
        default:
            break;
        }
    }


};


extern "C" {

static JSONQueryDFA* create_dfa(StackContext* ctx, DFACompressed* cpd_dfa) {
    JSONQueryDFA* dfa = createJSONQueryDFA(cpd_dfa->getStateSum(), cpd_dfa->getInputSize());
    for (int i = 0; i < dfa->states_num; ++i) {
        for (int j = 0; j < dfa->inputs_num; ++j ) {
            dfa->table[i * dfa->inputs_num + j] = cpd_dfa->nextState(i, j);
        }
    }
    for (int i = 0; i < dfa->states_num; ++i) {
        int stop_state = cpd_dfa->isStopState(i);
        dfa->stop_state[i] = stop_state;
        if (stop_state) {
            if (ctx->output_states.find(stop_state) != ctx->output_states.end())
                dfa->accept_type[i] = DFA_OUTPUT_TYPE;
            else
                dfa->accept_type[i] = DFA_PREDICATE_TYPE;
            auto pair = ctx->array_range[stop_state-1];
            if (pair.first || pair.second) {
                dfa->array_index[i] = {pair.first, pair.second};
                if (dfa->accept_type[i] != DFA_OUTPUT_TYPE) {
                    dfa->stop_state[i] = 0;
                    dfa->accept_type[i] = 0;
                }
            }
        }
    }
    dfa->names[1] = str_copy("other");
    dfa->names[2] = str_copy("array");
    for (int i = 3; i < dfa->inputs_num; ++i) {
        dfa->names[i] = str_copy(ctx->input_mapping[i].c_str());
    }
    return dfa;
}

static int acc_id2state(DFACompressed* cpd_dfa, int acc) {
    for (int i = 0; i < cpd_dfa->getStateSum(); ++i) {
        if (acc == cpd_dfa->isStopState(i)) {
            return i;
        }
    }
    return -1;
} 

static void create_context(StackContext* ctx, DFACompressed* cpd_dfa, JSONQueryDFA* m_dfa, JSONQueryDFAContext* context) {
    context->states_num = cpd_dfa->getStateSum();
    context->subtrees = (JSONPathNode**) calloc(cpd_dfa->getStateSum()+1, sizeof(JSONPathNode*));
    context->states_mapping = (JSONQueryIntVecPair*) calloc(cpd_dfa->getStateSum()+1, sizeof(JSONQueryIntVecPair));
    context->array_predicate_states.value_size = 0;
    context->array_predicate_states.value = (int*)calloc(cpd_dfa->getStateSum(), sizeof(int));
    for (int i = 0; i < cpd_dfa->getStateSum(); ++i) {
        int acc = getDFAStopState(m_dfa, i);
        if (acc) {
            context->subtrees[i] = ctx->tree_mapping[acc-1];
            const auto& vec = ctx->states_mapping[acc-1];
            context->states_mapping[i].value_size = vec.size();
            if (!vec.empty()) {
                context->states_mapping[i].value = (int*)calloc(vec.size(), sizeof(int));
                for (int j = 0; j < vec.size(); ++j) {
                    context->states_mapping[i].value[j] = acc_id2state(cpd_dfa, vec[j]+1);
                }
            }
            if (m_dfa->accept_type[i] == DFA_PREDICATE_TYPE) {
                for (int j = 0; j < cpd_dfa->getStateSum(); ++j) {
                    if (cpd_dfa->nextState(j, 2) == i) {
                        int k = context->array_predicate_states.value_size++;
                        context->array_predicate_states.value[k] = i;
                        break;
                    }
                }
            }
        }
    }
}

JSONQueryDFA* buildJSONQueryDFA(const char* json_path, JSONQueryDFAContext* context) {
    printf("\noriginal: %s\n", json_path);
    JSONPathNode* root = analysisJSONPath(json_path);
    return buildJSONQueryDFAFromAST(root, context);
}

JSONQueryDFA* buildJSONQueryDFAFromAST(JSONPathNode* json_path, JSONQueryDFAContext* context) {
    RegexModel* model = new RegexModel();

    StackContext* ctx = new StackContext(model, json_path);
    ctx->construct_dfa(json_path);
    if (ctx->last_st_generated == false)
        ctx->create_dfa();
    ctx->output_states.insert(model->size());
    
    printf("-------------------\n");
    model->input_max = ctx->input_mapping.size();

    SetConverter* start = new SetConverter();
    RegexBuilder* builder = new RegexBuilder();
    DFAMixer* mixer = new DFAMixer();
    start->next(builder)->next(mixer);

    RegexModel* data = start->ExecuteAll<RegexModel>(model);
    // cout << *data << endl;
    DFACompressed* dfa = (DFACompressed*)(data->main_dfa);
    // dfa->print();

    JSONQueryDFA* m_dfa = create_dfa(ctx, dfa);
    create_context(ctx, dfa, m_dfa, context);
    printDFA(m_dfa);

    delete model;
    delete start;
    delete builder;
    delete mixer;
    delete ctx;
    return m_dfa;
}


JSONQueryDFA* buildJSONQueryDFAMultiple(int num, const char* json_path[]) {
    // TODO: Implement merging multiple JSON Query DFAs
    return NULL;
}

JSONQueryDFA* buildJSONQueryDFAMultipleFromAST(int num, JSONPathNode* json_path[]) {
    return NULL;
}


}