#include "jsonpath_model.h"
#include "jsonpath_evaluator.h"
#include "jsonpath_parser.h"
#include "dfa_builder.h"

#define EXPECT_EQ(a, b) if ((a) != (b)) { printf("Expect %s == %s failed.\n", #a, #b); return 1; }
#define EXPECT_NE(a, b) if ((a) == (b)) { printf("Expect %s != %s failed.\n", #a, #b); return 1; }

int test1() {
    ASTNode* root = analysisJSONPath("$[?(@.a)]");
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
    ASTNode* root = analysisJSONPath("$[?(((@.a.k||@.d.bi.dfa)&&(@.e||@.f))&&(@.b||@.c))]");
    evaluatorModifyReference(root); // never use it like root = evaluatorModifyReference(root)
    printJsonPathAST(root);
    ASTNode* node = root->right->left;
    PredicateCondition table[] = {
        {"a.k", "true"},
        {"d.bi.dfa", "false"},
        {"e", "true"},
        {"f", "false"},
        {"b", "true"},
        {"c", "false"},
        {NULL, NULL}
    };
    bool v = evaluateExpression(node, table);
    EXPECT_EQ(v, true);
    if (v)
        printf("v = true\n");
    else
        printf("v = false\n");

    root = analysisJSONPath("$[?(@.text=='bbb')]");
    evaluatorModifyReference(root);
    node = root->right->left;
    PredicateCondition table2[] = {
        {"text", "\"ccc\""},
        {NULL, NULL}
    };
    printJsonPathAST(node);
    v = evaluateExpression(node, table2);
    EXPECT_EQ(v, false);
    if (v)
        printf("v2 = true\n");
    else
        printf("v2 = false\n");

    root = analysisJSONPath("$[?(@.text=='bbb')]");
    evaluatorModifyReference(root);
    node = root->right->left;
    PredicateCondition table3[] = {
        {"text", "\"bbb\""},
        {NULL, NULL}
    };
    printJsonPathAST(node);
    v = evaluateExpression(node, table3);
    EXPECT_EQ(v, true);
    if (v)
        printf("v3 = true\n");
    else
        printf("v3 = false\n");
    

    root = analysisJSONPath("$[?(!@.value)]");
    evaluatorModifyReference(root);
    node = root->right->left;
    PredicateCondition table4[] = {
        {"value", "10.9"},
        {NULL, NULL}
    };
    printJsonPathAST(node);
    v = evaluateExpression(node, table4);
    EXPECT_EQ(v, false);
    if (v)
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

    dfa = buildJSONQueryDFA("$.a[?(@.b)]", &ctx);
    printJSONQueryDFAContext(&ctx);

    dfa = buildJSONQueryDFA("$.root[?(@.index && @.guid)].friends[?(@.name)].id", &ctx);
    printJSONQueryDFAContext(&ctx);
    printf("next: %d, %s\n", dfaNextStateByStr(dfa, 1, "root"), "root");
    printf("next: %d, %s\n", dfaNextStateByStr(dfa, 2, DFA_ARRAY), "DFA_ARRAY");

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

    dfa = buildJSONQueryDFA("$.root[*].quoted_status.entities.symbols", &ctx);
    if (dfa == NULL) return 1;

    dfa = buildJSONQueryDFA("$.root[*].friends[*].id", &ctx);
    if (dfa == NULL) return 1;

    dfa = buildJSONQueryDFA("$.meta.view.columns[*].position", &ctx);
    if (dfa == NULL) return 1;

    dfa = buildJSONQueryDFA("$.root[?((@.index+@.b>2)&&(@.guid||@.b))].friends[?(@.name)].id", &ctx);
    printJSONQueryDFAContext(&ctx);
    if (dfa == NULL) return 1;

    return 0;
}


int main() {
    if (test_verify()) return 1;
    if (test_buildJSONQueryDFA()) return 1;
    if (test1()) return 1;

    printf("=============\n");
    ASTNode* root = analysisJSONPath("$.store.book[?(@.price < 10)].title");
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
