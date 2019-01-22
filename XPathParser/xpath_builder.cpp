#include "xpath_builder.h"
#include "pass.hpp"
#include "Lexer/dfa_compressed.hpp"
#include "Lexer/ecset_converter.hpp"
#include "Lexer/regex_builder.hpp"
#include "Lexer/model_core.hpp"
#include "Lexer/dfa_mixer.hpp"
#include "Lexer/set_converter.hpp"
#include "Lexer/regex_model.hpp"

#include "xpath_model.h"
#include "xpath_verify.h"

extern "C" {
#include "parser_j.h"
#include "scanner_j.h"
}

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

private:
    static char* str_copy(const char* src) {
        int len = strlen(src);
        char* buffer = (char*) malloc(len+1);
        strcpy(buffer, src);
        buffer[len] = '\0';
        return buffer;
    }
};

struct StackContext {
    RegexModel* model;
    XPathNode* root;
    vector<StackElement> st;
    unordered_map<string, int> name_mapping;
    // 0 is end, 1 is other, 2 is array 3-n are names
    vector<string> input_mapping;

    StackContext(RegexModel* model, XPathNode* root) : model(model), root(root), st(0), input_mapping(3) {}
    ~StackContext() {}

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
    }

    void construct_filter(XPathNode* node) {
        if (node->node_type == xnt_variable) {
            std::string str(node->string);
            std::istringstream tokenStream(str);
            std::string token;
            int count = 0;
            while (std::getline(tokenStream, token, '.')) {
                count++;
                st.push_back({set_dot_property, token.c_str()});
            }
            create_dfa();
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

    void construct_dfa(XPathNode* node) {
        if (node->left) {
            construct_dfa(node->left);
        }

        switch (node->node_type) {
        case xnt_concat: {
            // .*
            if (node->right && node->right->node_type == xnt_wildcard)
                st.push_back({set_dot_all});
            // .property
            if (node->right && node->right->node_type == xnt_id) 
                st.push_back({set_dot_property, node->right->string});
            break;
        }
        case xnt_parent_concat: {
            // ..*
            if (node->right && node->right->node_type == xnt_wildcard) 
                st.push_back({set_parent_all});
            // ..property
            if (node->right && node->right->node_type == xnt_id) 
                st.push_back({set_parent_property, node->right->string});
            break;
        }
        case xnt_predicate: {
            // []
            XPathNode* p = node->right;
            if (p) {
                switch (p->node_type) {
                // [start:end]
                case xnt_range: {
                    int begin, end;
                    if (p->left) begin = p->left->number;
                    else begin = INT_MIN;
                    if (p->right) end = p->right->number;
                    else end = INT_MAX;
                    st.push_back({set_array_index, begin, end});
                    break;
                }
                // [*]
                case xnt_wildcard: {
                    st.push_back({set_array_all});
                    break;
                }
                // [?()]
                case xnt_fliter: {
                    st.push_back({set_array_all});
                    create_dfa();
                    xpv_ModifyRef(p);
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

XPathNode* xpb_Analysis(const char* data) {
    XPathNode* root;
    yyscan_t sc;
    int res;
    jlex_init(&sc);
    YY_BUFFER_STATE buffer = j_scan_string(data, sc);
    res = jparse(sc, &root);
    j_delete_buffer(buffer, sc);
    jlex_destroy(sc);
    return root;
}


static void create_jsonpath_dfa(const char* json_path, RegexModel* model) {
    printf("\noriginal: %s\n", json_path);
    XPathNode* root = xpb_Analysis(json_path);
    printf("-------------------\n");

    StackContext ctx(model, root);
    ctx.construct_dfa(root);
    ctx.create_dfa();

    printf("-------------------\n");
    model->input_max = ctx.input_mapping.size();
    ctx.print_header();
}

static JQ_DFA* create_dfa(DFACompressed* cpd_dfa) {
    JQ_DFA* dfa = jqd_Create(cpd_dfa->getStateSum(), cpd_dfa->getInputSize(), 0);
    for (int i = 0; i < dfa->states_num; ++i) {
        for (int j = 0; j < dfa->inputs_num; ++j ) {
            dfa->table[i * dfa->inputs_num + j] = cpd_dfa->nextState(i, j);
        }
    }
    return dfa;
}


JQ_DFA* xpb_Create(const char* json_path) {
    RegexModel* model = new RegexModel();
    create_jsonpath_dfa(json_path, model);

    SetConverter* start = new SetConverter();
    RegexBuilder* builder = new RegexBuilder();
    DFAMixer* mixer = new DFAMixer();
    start->next(builder)->next(mixer);

    RegexModel* data = start->ExecuteAll<RegexModel>(model);
    // cout << *data << endl;
    DFACompressed* dfa = (DFACompressed*)(data->main_dfa);
    dfa->print();

    delete model;
    delete start;
    delete builder;
    delete mixer;
    return create_dfa(dfa);
}


JQ_DFA* xpb_CreateMultiple(int num, const char* json_path[]) {
    // TODO: Implement merging multiple JSON Query DFAs
    return NULL;
}


}