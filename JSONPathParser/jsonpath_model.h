#ifndef __XPATH_MODEL_H__
#define __XPATH_MODEL_H__


#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum JSONPathNodeType {
    jnt_root, jnt_concat, jnt_parent_concat, jnt_descendants, jnt_id, jnt_operator, jnt_predicate, jnt_wildcard,
    jnt_number, jnt_string, jnt_function, jnt_variable, jnt_attribute, jnt_reference, jnt_script, jnt_fliter, jnt_range
} JSONPathNodeType;

typedef enum JSONPathOperatorType {
    jot_or = 0, jot_and, // boolean
    jot_equal, jot_less, jot_greater, jot_neq, jot_leq, jot_geq,  // comparing
    jot_add, jot_minus, jot_multiply, jot_div, jot_mod, // calculation
    jot_union // node-set union
} JSONPathOperatorType;

typedef struct JSONPathNode {
    JSONPathNodeType node_type; 
    union {
        double number;
        char* string;
        bool boolean;
        JSONPathOperatorType opt;
    };
    struct JSONPathNode *left, *right;
} JSONPathNode;

static inline JSONPathNode* jpn_Create() {
    JSONPathNode* jpn = (JSONPathNode*) malloc(sizeof(JSONPathNode));
    jpn->left = NULL;
    jpn->right = NULL;
    return jpn;
}

static inline JSONPathNode* jpn_CreateNumber(double data) {
    JSONPathNode* jpn = jpn_Create();
    jpn->node_type = jnt_number;
    jpn->number = data;
    return jpn;
}

static inline JSONPathNode* jpn_CreateString(char* data) {
    JSONPathNode* jpn = jpn_Create();
    jpn->node_type = jnt_string;
    jpn->string = data;
    return jpn;
}

static inline JSONPathNode* jpn_CreateID(char* name) {
    JSONPathNode* jpn = jpn_Create();
    jpn->node_type = jnt_id;
    jpn->string = name;
    return jpn;
}

static inline JSONPathNode* jpn_CreateFunction(char* name) {
    JSONPathNode* jpn = jpn_Create();
    jpn->node_type = jnt_function;
    jpn->string = name;
    return jpn;
}

static inline JSONPathNode* jpn_CreateAttribute(char* name) {
    JSONPathNode* jpn = jpn_Create();
    jpn->node_type = jnt_attribute;
    jpn->string = name;
    return jpn;
}

static inline JSONPathNode* jpn_CreateVariable(char* name) {
    JSONPathNode* jpn = jpn_Create();
    jpn->node_type = jnt_variable;
    jpn->string = name;
    return jpn;
}

static inline JSONPathNode* jpn_CreateRoot() {
    JSONPathNode* jpn = jpn_Create();
    jpn->node_type = jnt_root;
    return jpn;
}

static inline JSONPathNode* jpn_CreateWildcard() {
    JSONPathNode* jpn = jpn_Create();
    jpn->node_type = jnt_wildcard;
    return jpn;
}

static inline JSONPathNode* jpn_CreateRef() {
    JSONPathNode* jpn = jpn_Create();
    jpn->node_type = jnt_reference;
    return jpn;
}


static inline JSONPathNode* jpn_CreateConcat(JSONPathNode* left, JSONPathNode* right) {
    JSONPathNode* jpn = jpn_Create();
    jpn->node_type = jnt_concat;
    jpn->left = left;
    jpn->right = right;
    return jpn;
}

static inline JSONPathNode* jpn_CreateRange(JSONPathNode* left, JSONPathNode* right) {
    JSONPathNode* jpn = jpn_Create();
    jpn->node_type = jnt_range;
    jpn->left = left;
    jpn->right = right;
    return jpn;
}

static inline JSONPathNode* jpn_CreateParentConcat(JSONPathNode* left, JSONPathNode* right) {
    JSONPathNode* jpn = jpn_Create();
    jpn->node_type = jnt_parent_concat;
    jpn->left = left;
    jpn->right = right;
    return jpn;
}


static inline JSONPathNode* jpn_CreatePredicate(JSONPathNode* left, JSONPathNode* right) {
    JSONPathNode* jpn = jpn_Create();
    jpn->node_type = jnt_predicate;
    jpn->left = left;
    jpn->right = right;
    return jpn;
}

static inline JSONPathNode* jpn_CreateOperator(JSONPathOperatorType opt, JSONPathNode* left, JSONPathNode* right) {
    JSONPathNode* jpn = jpn_Create();
    jpn->node_type = jnt_operator;
    jpn->opt = opt;
    jpn->left = left;
    jpn->right = right;
    return jpn;
}

static inline JSONPathNode* jpn_CreateOperatorOne(JSONPathOperatorType opt, JSONPathNode* left) {
    JSONPathNode* jpn = jpn_Create();
    jpn->node_type = jnt_operator;
    jpn->opt = opt;
    jpn->left = left;
    return jpn;
}


static inline JSONPathNode* jpn_CreateScript(JSONPathNode* left) {
    JSONPathNode* jpn = jpn_Create();
    jpn->node_type = jnt_script;
    jpn->left = left;
    return jpn;
}

static inline JSONPathNode* jpn_CreateFliter(JSONPathNode* left) {
    JSONPathNode* jpn = jpn_Create();
    jpn->node_type = jnt_fliter;
    jpn->left = left;
    return jpn;
}


static inline void jpn_print_space(int n) {
    for (int i = 0; i < n; ++i) printf("  ");
}
/**
    jot_or = 0, jot_and, // boolean
    jot_equal, jot_less, jot_greater, jot_neq, jot_leq, jot_geq,  // comparing
    jot_add, jot_minus, jot_multiply, jot_div, jot_mod, // calculation
    jot_union // node-set union
*/
static inline void jpn_print_opt(JSONPathOperatorType opt) {
    const char* name_mapping[] = {
        "or", "and", 
        "=", "<", ">", "!=", "<=", ">=",
        "+", "-", "*", "div", "mod",
        "|"
    };
    printf("%s", name_mapping[opt]);
}


static inline void jpn_print_jnode(JSONPathNode* node, int depth, bool print_return) {
    bool children_print_return = true;
    int right_is_child = 0;
    jpn_print_space(depth);
    switch (node->node_type) {
        case jnt_concat: {
            printf(".");

            break;
        }
        case jnt_descendants: {
            printf("//");
            break;
        }
        case jnt_number: {
            printf("number %.3f", node->number);
            break;
        }
        case jnt_id: {
            printf("%s", node->string);
            break;
        }
        case jnt_function: {
            printf("%s (", node->string);
            break;
        }
        case jnt_attribute: {
            printf("@%s", node->string);
            break;
        }
        case jnt_string: {
            printf("\"%s\"", node->string);
            break;
        }
        case jnt_variable: {
            printf("@.%s", node->string);
            break;
        }
        case jnt_predicate: {
            printf("[");
            break;
        }
        case jnt_operator: {
            jpn_print_opt(node->opt);
            right_is_child = 1;
            break;
        }
        case jnt_root: {
            printf("$");
            break;
        }
        case jnt_script: {
            printf("(");
            break;
        }
        case jnt_fliter: {
            printf("?(");
            break;
        }
        case jnt_range: {
            printf("[]");
            break;
        }
        case jnt_reference: {
            printf("@");
            break;
        }
        default:
            printf("Unknown node");
            break;
    }
    if (print_return) printf("\n");
    if (node->left) jpn_print_jnode(node->left, depth+1, children_print_return);
    if (node->right) jpn_print_jnode(node->right, depth+1, children_print_return);
}

static inline void jpn_Print(JSONPathNode* root) {
    jpn_print_jnode(root, 0, true);
} 


#ifdef __cplusplus
}
#endif

#endif 