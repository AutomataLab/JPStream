#include "xpath_model.h"
#include "xpath_verify.h"
#include "parser_j.h"
#include "scanner_j.h"

#define EXPECT_EQ(a, b) if ((a) != (b)) { printf("Expect %s == %s failed.\n", #a, #b); return 1; }
#define EXPECT_NE(a, b) if ((a) == (b)) { printf("Expect %s != %s failed.\n", #a, #b); return 1; }

XPathNode* Analysis(const char* data) {
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

int test1() {
    XPathNode* root = Analysis("$[?(@.a)]");
    xpn_PrintJSON(root);
    EXPECT_NE(root, NULL);
    EXPECT_EQ(root->node_type, xnt_predicate);
    EXPECT_NE(root->left, NULL);
    EXPECT_EQ(root->left->node_type, xnt_root);
    EXPECT_NE(root->right, NULL);
    EXPECT_EQ(root->right->left->left->node_type, xnt_reference);
    return 0;
}

int test_verify() {
    XPathNode* root = Analysis("$[?(((@.a.k||@.d.bi.dfa)&&(@.e||@.f))&&(@.b||@.c))]");
    modify_ref(root); // never use it like root = modify_ref(root)
    xpn_PrintJSON(root);
    XPathNode* node = root->right->left;
    XPathKeyValuePair table[] = {
        {"a.k", {.vtype = xvt_boolean, .boolean = true}},
        {"d.bi.dfa", {.vtype = xvt_boolean, .boolean = false}},
        {"e", {.vtype = xvt_boolean, .boolean = true}},
        {"f", {.vtype = xvt_boolean, .boolean = false}},
        {"b", {.vtype = xvt_boolean, .boolean = true}},
        {"c", {.vtype = xvt_boolean, .boolean = false}},
        {NULL, {0}}
    };
    XPathValue v = xpv_Calculate(node, table);
    EXPECT_EQ(v.vtype, xvt_boolean);
    EXPECT_EQ(v.boolean, true);
    if (v.boolean)
        printf("v = true\n");
    else
        printf("v = false\n");
    return 0;
}


int main() {
    if (test1()) return 1;
    if (test_verify()) return 1;

    printf("=============\n");
    XPathNode* root = Analysis("$.store.book[?(@.price < 10)].title");
    xpn_PrintJSON(root);
    modify_ref(root); // never use it like root = modify_ref(root)
    xpn_PrintJSON(root);
    printf("=============\n");
    
    root = Analysis("$[?(@.a&&(@.b||@.c))]");
    xpn_PrintJSON(root);
    modify_ref(root); // never use it like root = modify_ref(root)
    xpn_PrintJSON(root);
    printf("=============\n");
    root = Analysis("$[?((@.a.k||@.d.bi.dfa&&(@.e||@.f))&&(@.b||@.c))]");
    xpn_PrintJSON(root);
    modify_ref(root); // never use it like root = modify_ref(root)
    xpn_PrintJSON(root);
    printf("=============\n");
    printf("correct\n");
    return 0;
}
