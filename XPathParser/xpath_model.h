#ifndef __XPATH_MODEL_H__
#define __XPATH_MODEL_H__


#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum XPathNodeType {
    xnt_root, xnt_concat, xnt_descendants, xnt_id, xnt_operator, xnt_predicate, 
    xnt_number, xnt_string, xnt_function, xnt_variable, xnt_attribute
} XPathNodeType;

typedef enum XPathOperatorType {
    xot_or = 0, xot_and, // boolean
    xot_equal, xot_less, xot_greater, xot_neq, xot_leq, xot_geq,  // comparing
    xot_add, xot_minus, xot_multiply, xot_div, xot_mod, // calculation
    xot_union // node-set union
} XPathOperatorType;

typedef struct XPathNode {
    XPathNodeType node_type; 
    union {
        double number;
        char* string;
        XPathOperatorType opt;
    };
    struct { struct XPathNode *left, *right; } children;
} XPathNode;

inline XPathNode* xpn_Create() {
    XPathNode* xpn = (XPathNode*) malloc(sizeof(XPathNode));
    xpn->children.left = NULL;
    xpn->children.right = NULL;
    return xpn;
}

inline XPathNode* xpn_CreateNumber(double data) {
    XPathNode* xpn = xpn_Create();
    xpn->node_type = xnt_number;
    xpn->number = data;
    return xpn;
}

inline XPathNode* xpn_CreateString(char* data) {
    XPathNode* xpn = xpn_Create();
    xpn->node_type = xnt_string;
    xpn->string = data;
    return xpn;
}

inline XPathNode* xpn_CreateID(char* name) {
    XPathNode* xpn = xpn_Create();
    xpn->node_type = xnt_id;
    xpn->string = name;
    return xpn;
}

inline XPathNode* xpn_CreateFunction(char* name) {
    XPathNode* xpn = xpn_Create();
    xpn->node_type = xnt_function;
    xpn->string = name;
    return xpn;
}

inline XPathNode* xpn_CreateAttribute(char* name) {
    XPathNode* xpn = xpn_Create();
    xpn->node_type = xnt_attribute;
    xpn->string = name;
    return xpn;
}

inline XPathNode* xpn_CreateVariable(char* name) {
    XPathNode* xpn = xpn_Create();
    xpn->node_type = xnt_variable;
    xpn->string = name;
    return xpn;
}

inline XPathNode* xpn_CreateConcat() {
    XPathNode* xpn = xpn_Create();
    xpn->node_type = xnt_concat;
    return xpn;
}

inline XPathNode* xpn_CreatePredicate() {
    XPathNode* xpn = xpn_Create();
    xpn->node_type = xnt_predicate;
    return xpn;
}

inline XPathNode* xpn_CreateOperator(XPathOperatorType opt) {
    XPathNode* xpn = xpn_Create();
    xpn->node_type = xnt_operator;
    xpn->opt = opt;
    return xpn;
}


inline void xpn_print_space(int n) {
    for (int i = 0; i < n; ++i) printf("  ");
}
/**
    xot_or = 0, xot_and, // boolean
    xot_equal, xot_less, xot_greater, xot_neq, xot_leq, xot_geq,  // comparing
    xot_add, xot_minus, xot_multiply, xot_div, xot_mod, // calculation
    xot_union // node-set union
*/
inline void xpn_print_opt(XPathOperatorType opt) {
    const char* name_mapping[] = {
        "or", "and", 
        "=", "<", ">", "!=", "<=", ">=",
        "+", "-", "*", "div", "mod",
        "|"
    };
    printf("%s", name_mapping[opt]);
}

inline void xpn_print_node(XPathNode* node, int depth, bool print_return) {
    bool children_print_return = true;
    int right_is_child = 0;
    xpn_print_space(depth);
    switch (node->node_type) {
        case xnt_concat: {
            printf("/");
            break;
        }
        case xnt_descendants: {
            printf("//");
            break;
        }
        case xnt_number: {
            printf("number %.3f", node->number);
            break;
        }
        case xnt_id: {
            printf("%s", node->string);
            break;
        }
        case xnt_function: {
            printf("%s (", node->string);
            break;
        }
        case xnt_attribute: {
            printf("@%s", node->string);
            break;
        }
        case xnt_string: {
            printf("\"%s\"", node->string);
            break;
        }
        case xnt_variable: {
            printf("$%s", node->string);
            break;
        }
        case xnt_predicate: {
            printf("[");
            break;
        }
        case xnt_operator: {
            xpn_print_opt(node->opt);
            children_print_return = false;
            right_is_child = 1;
        }
        default:
            break;
    }
    if (print_return) printf("\n");
    if (node->children.left) xpn_print_node(node->children.left, depth+1, children_print_return);
    if (node->children.right) xpn_print_node(node->children.right, depth+right_is_child, children_print_return);
}

inline void xpn_Print(XPathNode* root) {
    xpn_print_node(root, 0, true);
} 


#ifdef __cplusplus
}
#endif

#endif 