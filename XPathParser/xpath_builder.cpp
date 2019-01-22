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

using namespace std;
using namespace dragontooth;


typedef enum SEType {
    set_dot_all, set_parent_all, 
    set_dot_property, set_parent_property,
    set_array_all, set_array_index
} SEType;


struct StackElement {
    SEType stype;
    union {
        const char* str;
        pair<int, int> range;
    };

    StackElement(SEType stype) : stype(stype) {}
    StackElement(SEType stype, const char* str) : stype(stype), str(str) {}
    StackElement(SEType stype, int a, int b) : stype(stype), range(a, b) {}
    ~StackElement() {}
};

struct StackContext {
    RegexModel* model;
    XPathNode* root;
    vector<StackElement> st;
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

static void createDFA(StackContext* ctx) {

}

static void construct_filter(XPathNode* node, StackContext* ctx) {
    
}

static void construct_dfa(XPathNode* node, StackContext* ctx) {
    if (node->left) {
        construct_dfa(node->left, ctx);
    }

    switch (node->node_type) {
    case xnt_concat: {
        // .*
        if (node->right && node->right->node_type == xnt_wildcard)
            ctx->st.push_back({set_dot_all});
        // .property
        if (node->right && node->right->node_type == xnt_id)
            ctx->st.push_back({set_dot_property, node->right->string});
        break;
    }
    case xnt_parent_concat: {
        // ..*
        if (node->right && node->right->node_type == xnt_wildcard)
            ctx->st.push_back({set_parent_all});
        // ..property
        if (node->right && node->right->node_type == xnt_id)
            ctx->st.push_back({set_parent_property, node->right->string});
        break;
    }
    case xnt_predicate: {
        // []
        if (node->right) {
            XPathNode* p = node->right;

            switch (p->node_type) {
            // [start:end]
            case xnt_range: {
                int begin, end;
                if (p->left) begin = p->left->number;
                else begin = INT_MIN;
                if (p->right) end = p->right->number;
                else end = INT_MAX;
                ctx->st.push_back({set_array_index, begin, end});
            }
            // [*]
            case xnt_wildcard: {
                ctx->st.push_back({set_array_all});
            }
            // [?()]
            case xnt_fliter: {
                ctx->st.push_back({set_array_all});
                construct_filter(p->left, ctx);
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

static void create_jsonpath_dfa(const char* json_path, RegexModel* model) {
    XPathNode* root = xpb_Analysis(json_path);
    xpv_ModifyRef(root);
    xpn_PrintJSON(root);


    model->input_max = 10;
}

static JQ_DFA* create_dfa(DFACompressed* cpd_dfa) {

    return NULL;
}


JQ_DFA* xpb_Create(const char* json_path) {
    RegexModel* model = new RegexModel();
    create_jsonpath_dfa(json_path, model);

    SetConverter* start = new SetConverter();
    RegexBuilder* builder = new RegexBuilder();
    DFAMixer* mixer = new DFAMixer();
    start->next(builder)->next(mixer);

    RegexModel* data = start->ExecuteAll<RegexModel>(model);
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