#include "jsonpath_model.h"
#include "jsonpath_evaluator.h"
#include "jsonpath_parser.h"
#include "dfa_builder.h"

#define EXPECT_EQ(a, b) if ((a) != (b)) { printf("Expect %s == %s failed.\n", #a, #b); return 1; }
#define EXPECT_NE(a, b) if ((a) == (b)) { printf("Expect %s != %s failed.\n", #a, #b); return 1; }

int test1() {
    JSONPathNode* root = analysisJSONPath("$[?(@.a)]");
    printJsonPathAST(root);
    EXPECT_NE(root, NULL);
    EXPECT_EQ(root->node_type, jnt_predicate);
    EXPECT_NE(root->left, NULL);
    EXPECT_EQ(root->left->node_type, jnt_root);
    EXPECT_NE(root->right, NULL);
    EXPECT_EQ(root->right->left->left->node_type, jnt_reference);
    return 0;
}

int test_verify() {
    JSONPathNode* root = analysisJSONPath("$[?(((@.a.k||@.d.bi.dfa)&&(@.e||@.f))&&(@.b||@.c))]");
    evaluatorModifyReference(root); // never use it like root = evaluatorModifyReference(root)
    printJsonPathAST(root);
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

    root = analysisJSONPath("$[?(@.text=='bbb')]");
    evaluatorModifyReference(root);
    node = root->right->left;
    JSONPathKeyValuePair table2[] = {
        {"text", {.vtype = jvt_string, .string = "ccc"}},
        {NULL, {0}}
    };
    printJsonPathAST(node);
    v = jpe_Evaluate(node, table2);
    EXPECT_EQ(v.vtype, jvt_boolean);
    EXPECT_EQ(v.boolean, false);
    if (v.boolean)
        printf("v2 = true\n");
    else
        printf("v2 = false\n");

    root = analysisJSONPath("$[?(@.text=='bbb')]");
    evaluatorModifyReference(root);
    node = root->right->left;
    JSONPathKeyValuePair table3[] = {
        {"text", {.vtype = jvt_string, .string = "bbb"}},
        {NULL, {0}}
    };
    printJsonPathAST(node);
    v = jpe_Evaluate(node, table3);
    EXPECT_EQ(v.vtype, jvt_boolean);
    EXPECT_EQ(v.boolean, true);
    if (v.boolean)
        printf("v3 = true\n");
    else
        printf("v3 = false\n");
    

    root = analysisJSONPath("$[?(@.value)]");
    evaluatorModifyReference(root);
    node = root->right->left;
    JSONPathKeyValuePair table4[] = {
        {"value", {.vtype = jvt_number, .number = 10.9}},
        {NULL, {0}}
    };
    printJsonPathAST(node);
    v = jpe_Evaluate(node, table4);
    EXPECT_EQ(v.vtype, jvt_boolean);
    EXPECT_EQ(v.boolean, true);
    if (v.boolean)
        printf("v4 = true\n");
    else
        printf("v4 = false\n");
    return 0;
}

int test_buildJSONQueryDFA() {
    JSONQueryDFAContext ctx;
    JSONQueryDFA* dfa;

    dfa = buildJSONQueryDFA("$.a[1:2].b", &ctx);
    printJSONQueryDFAContext(&ctx);

    dfa = buildJSONQueryDFA("$.a[1:2]", &ctx);
    printJSONQueryDFAContext(&ctx);

    dfa = buildJSONQueryDFA("$.root[?(@.index && @.guid)].friends[?(@.name)].id", &ctx);
    printJSONQueryDFAContext(&ctx);
    printf("next: %d, %s\n", dfaNextStateByStr(dfa, 1, "root"), "root");
    printf("next: %d, %s\n", dfaNextStateByStr(dfa, 2, JSONQueryDFA_ARRAY), "JSONQueryDFA_ARRAY");

    if (dfa == NULL) return 1;
    
    dfa = buildJSONQueryDFA("$.root[12:20].title", &ctx);
    printJSONQueryDFAContext(&ctx);
    if (dfa == NULL) return 1;

    dfa = buildJSONQueryDFA("$.root.id", &ctx);
    if (dfa == NULL) return 1;

    dfa = buildJSONQueryDFA("$..root.id", &ctx);
    if (dfa == NULL) return 1;

    dfa = buildJSONQueryDFA("$.*.root", &ctx);
    if (dfa == NULL) return 1;

    dfa = buildJSONQueryDFA("$..*.root", &ctx);
    if (dfa == NULL) return 1;

    dfa = buildJSONQueryDFA("$.root[*].title", &ctx);
    if (dfa == NULL) return 1;

    dfa = buildJSONQueryDFA("$.root[*].claims.P150[?(@.id&&@.type)].mainsnak.property", &ctx);
    if (dfa == NULL) return 1;

    dfa = buildJSONQueryDFA("$.root.products[?(@.sku&&@.productId)].categoryPath[?(@.name)].id", &ctx);
    if (dfa == NULL) return 1;

    dfa = buildJSONQueryDFA("$.root.products[*].productId", &ctx);
    if (dfa == NULL) return 1;

    dfa = buildJSONQueryDFA("$.root[?(@.id)].quoted_status.entities.user_mentions[?(@.indices&&@.id_str)].id", &ctx);
    if (dfa == NULL) return 1;

    dfa = buildJSONQueryDFA("$.root[?(@.text&&@.contributors)].id", &ctx);
    if (dfa == NULL) return 1;

    dfa = buildJSONQueryDFA("$.root.meta.view.columns[?(@.id&&@.name&&@.cachedContents)].position", &ctx);
    if (dfa == NULL) return 1;

    dfa = dfa_Create("$.root[*].quoted_status.entities.symbols", &ctx);
    if (dfa == NULL) return 1;

    dfa = dfa_Create("$.root[*].friends[*].id", &ctx);
    if (dfa == NULL) return 1;

    dfa = dfa_Create("$.meta.view.columns[*].position", &ctx);
    if (dfa == NULL) return 1;

    return 0;
}


int main() {
    if (test_buildJSONQueryDFA()) return 1;
    if (test1()) return 1;
    if (test_verify()) return 1;

    printf("=============\n");
    JSONPathNode* root = analysisJSONPath("$.store.book[?(@.price < 10)].title");
    printJsonPathAST(root);
    evaluatorModifyReference(root); // never use it like root = evaluatorModifyReference(root)
    printJsonPathAST(root);
    printf("=============\n");
    
    root = analysisJSONPath("$[?(@.a&&(@.b||@.c))]");
    printJsonPathAST(root);
    evaluatorModifyReference(root); // never use it like root = evaluatorModifyReference(root)
    printJsonPathAST(root);
    printf("=============\n");
    root = analysisJSONPath("$[?((@.a.k||@.d.bi.dfa&&(@.e||@.f))&&(@.b||@.c))]");
    printJsonPathAST(root);
    evaluatorModifyReference(root); // never use it like root = evaluatorModifyReference(root)
    printJsonPathAST(root);
    printf("=============\n");
    printf("correct\n");
    return 0;
}
