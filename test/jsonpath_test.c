#include "jsonpath_model.h"
#include "jsonpath_evaluator.h"
#include "jsonpath_parser.h"
#include "dfa_builder.h"

#define EXPECT_EQ(a, b) if ((a) != (b)) { printf("Expect %s == %s failed.\n", #a, #b); return 1; }
#define EXPECT_NE(a, b) if ((a) == (b)) { printf("Expect %s != %s failed.\n", #a, #b); return 1; }

int test1() {
    JSONPathNode* root = jpp_Analysis("$[?(@.a)]");
    jpn_Print(root);
    EXPECT_NE(root, NULL);
    EXPECT_EQ(root->node_type, jnt_predicate);
    EXPECT_NE(root->left, NULL);
    EXPECT_EQ(root->left->node_type, jnt_root);
    EXPECT_NE(root->right, NULL);
    EXPECT_EQ(root->right->left->left->node_type, jnt_reference);
    return 0;
}

int test_verify() {
    JSONPathNode* root = jpp_Analysis("$[?(((@.a.k||@.d.bi.dfa)&&(@.e||@.f))&&(@.b||@.c))]");
    jpe_ModifyRef(root); // never use it like root = jpe_ModifyRef(root)
    jpn_Print(root);
    JSONPathNode* node = root->right->left;
    JSONPathKeyValuePair table[] = {
        {"a.k", {.vtype = jvt_boolean, .boolean = true}},
        {"d.bi.dfa", {.vtype = jvt_boolean, .boolean = false}},
        {"e", {.vtype = jvt_boolean, .boolean = true}},
        {"f", {.vtype = jvt_boolean, .boolean = false}},
        {"b", {.vtype = jvt_boolean, .boolean = true}},
        {"c", {.vtype = jvt_boolean, .boolean = false}},
        {NULL, {0}}
    };
    JSONPathValue v = jpe_Evaluate(node, table);
    EXPECT_EQ(v.vtype, jvt_boolean);
    EXPECT_EQ(v.boolean, true);
    if (v.boolean)
        printf("v = true\n");
    else
        printf("v = false\n");

    root = jpp_Analysis("$[?(@.text=='bbb')]");
    jpe_ModifyRef(root);
    node = root->right->left;
    JSONPathKeyValuePair table2[] = {
        {"text", {.vtype = jvt_string, .string = "ccc"}},
        {NULL, {0}}
    };
    jpn_Print(node);
    v = jpe_Evaluate(node, table2);
    EXPECT_EQ(v.vtype, jvt_boolean);
    EXPECT_EQ(v.boolean, false);
    if (v.boolean)
        printf("v2 = true\n");
    else
        printf("v2 = false\n");

    root = jpp_Analysis("$[?(@.text=='bbb')]");
    jpe_ModifyRef(root);
    node = root->right->left;
    JSONPathKeyValuePair table3[] = {
        {"text", {.vtype = jvt_string, .string = "bbb"}},
        {NULL, {0}}
    };
    jpn_Print(node);
    v = jpe_Evaluate(node, table3);
    EXPECT_EQ(v.vtype, jvt_boolean);
    EXPECT_EQ(v.boolean, true);
    if (v.boolean)
        printf("v3 = true\n");
    else
        printf("v3 = false\n");
    

    root = jpp_Analysis("$[?(@.value)]");
    jpe_ModifyRef(root);
    node = root->right->left;
    JSONPathKeyValuePair table4[] = {
        {"value", {.vtype = jvt_number, .number = 10.9}},
        {NULL, {0}}
    };
    jpn_Print(node);
    v = jpe_Evaluate(node, table4);
    EXPECT_EQ(v.vtype, jvt_boolean);
    EXPECT_EQ(v.boolean, true);
    if (v.boolean)
        printf("v4 = true\n");
    else
        printf("v4 = false\n");
    return 0;
}

int test_dfa_Create() {
    JQ_CONTEXT ctx;
    JQ_DFA* dfa;

    dfa = dfa_Create("$.root[?(@.index && @.guid)].friends[?(@.name)].id", &ctx);
    dfa_print(&ctx);
    if (dfa == NULL) return 1;
    
    dfa = dfa_Create("$.root[12:20].title", &ctx);
    dfa_print(&ctx);
    if (dfa == NULL) return 1;

    dfa = dfa_Create("$.root.id", &ctx);
    if (dfa == NULL) return 1;

    dfa = dfa_Create("$..root.id", &ctx);
    if (dfa == NULL) return 1;

    dfa = dfa_Create("$.*.root", &ctx);
    if (dfa == NULL) return 1;

    dfa = dfa_Create("$..*.root", &ctx);
    if (dfa == NULL) return 1;

    dfa = dfa_Create("$.root[*].title", &ctx);
    if (dfa == NULL) return 1;

    dfa = dfa_Create("$.root[*].claims.P150[?(@.id&&@.type)].mainsnak.property", &ctx);
    if (dfa == NULL) return 1;

    dfa = dfa_Create("$.root.products[?(@.sku&&@.productId)].categoryPath[?(@.name)].id", &ctx);
    if (dfa == NULL) return 1;

    dfa = dfa_Create("$.root.products[*].productId", &ctx);
    if (dfa == NULL) return 1;

    dfa = dfa_Create("$.root[?(@.id)].quoted_status.entities.user_mentions[?(@.indices&&@.id_str)].id", &ctx);
    if (dfa == NULL) return 1;

    dfa = dfa_Create("$.root[?(@.text&&@.contributors)].id", &ctx);
    if (dfa == NULL) return 1;

    dfa = dfa_Create("$.root.meta.view.columns[?(@.id&&@.name&&@.cachedContents)].position", &ctx);
    if (dfa == NULL) return 1;

    return 0;
}


int main() {
    if (test_dfa_Create()) return 1;
    if (test1()) return 1;
    if (test_verify()) return 1;

    printf("=============\n");
    JSONPathNode* root = jpp_Analysis("$.store.book[?(@.price < 10)].title");
    jpn_Print(root);
    jpe_ModifyRef(root); // never use it like root = jpe_ModifyRef(root)
    jpn_Print(root);
    printf("=============\n");
    
    root = jpp_Analysis("$[?(@.a&&(@.b||@.c))]");
    jpn_Print(root);
    jpe_ModifyRef(root); // never use it like root = jpe_ModifyRef(root)
    jpn_Print(root);
    printf("=============\n");
    root = jpp_Analysis("$[?((@.a.k||@.d.bi.dfa&&(@.e||@.f))&&(@.b||@.c))]");
    jpn_Print(root);
    jpe_ModifyRef(root); // never use it like root = jpe_ModifyRef(root)
    jpn_Print(root);
    printf("=============\n");
    printf("correct\n");
    return 0;
}
