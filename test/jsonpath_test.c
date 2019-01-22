#include "xpath_model.h"
#include "xpath_verify.h"
#include "xpath_builder.h"

#define EXPECT_EQ(a, b) if ((a) != (b)) { printf("Expect %s == %s failed.\n", #a, #b); return 1; }
#define EXPECT_NE(a, b) if ((a) == (b)) { printf("Expect %s != %s failed.\n", #a, #b); return 1; }

int test1() {
    XPathNode* root = xpb_Analysis("$[?(@.a)]");
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
    XPathNode* root = xpb_Analysis("$[?(((@.a.k||@.d.bi.dfa)&&(@.e||@.f))&&(@.b||@.c))]");
    xpv_ModifyRef(root); // never use it like root = xpv_ModifyRef(root)
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


int test_xpb_Create() {
    JQ_DFA* dfa = xpb_Create("$.root[?(@.index && @.guid.uuid)].friends[?(@.name)].id");
    if (dfa == NULL) return 1;

    dfa = xpb_Create("$.root.id");
    if (dfa == NULL) return 1;

    dfa = xpb_Create("$..root.id");
    if (dfa == NULL) return 1;

    dfa = xpb_Create("$.*.root");
    if (dfa == NULL) return 1;

    dfa = xpb_Create("$..*.root");
    if (dfa == NULL) return 1;

    return 0;
}


int main() {
    if (test_xpb_Create()) return 1;
    if (test1()) return 1;
    if (test_verify()) return 1;

    printf("=============\n");
    XPathNode* root = xpb_Analysis("$.store.book[?(@.price < 10)].title");
    xpn_PrintJSON(root);
    xpv_ModifyRef(root); // never use it like root = xpv_ModifyRef(root)
    xpn_PrintJSON(root);
    printf("=============\n");
    
    root = xpb_Analysis("$[?(@.a&&(@.b||@.c))]");
    xpn_PrintJSON(root);
    xpv_ModifyRef(root); // never use it like root = xpv_ModifyRef(root)
    xpn_PrintJSON(root);
    printf("=============\n");
    root = xpb_Analysis("$[?((@.a.k||@.d.bi.dfa&&(@.e||@.f))&&(@.b||@.c))]");
    xpn_PrintJSON(root);
    xpv_ModifyRef(root); // never use it like root = xpv_ModifyRef(root)
    xpn_PrintJSON(root);
    printf("=============\n");
    printf("correct\n");
    return 0;
}
