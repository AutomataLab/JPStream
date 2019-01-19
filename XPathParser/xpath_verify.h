#ifndef __XPATH_VERIFY_H__
#define __XPATH_VERIFY_H__

#include "xpath_model.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum XPathValueType {
    xvt_null = 0, xvt_number, xvt_string, xvt_boolean
} XPathValueType;

typedef struct XPathValue {
    XPathValueType vtype;
    union {
        double number;
        char* string;
        bool boolean;
    };
} XPathValue;

typedef struct XPathKeyValuePair {
    char* key;
    XPathValue value;
} XPathKeyValuePair;

static inline XPathValue xpv_Find(const char* name, XPathKeyValuePair* table) {
    int i = 0;
    while (table[i].key != NULL) {
        int r = strcmp(table[i].key, name);
        if (r == 0) {
            return table[i].value;
        }
        i++;
    }
    XPathValue v = {xvt_null};
    return v;
}

static inline XPathValue xpv_Opt1(XPathOperatorType opt, XPathValue v1) {
    XPathValue v = {xvt_null};
    return v;
}

/**
    xot_or = 0, xot_and, // boolean
    xot_equal, xot_less, xot_greater, xot_neq, xot_leq, xot_geq,  // comparing
    xot_add, xot_minus, xot_multiply, xot_div, xot_mod, // calculation
    xot_union // node-set union
*/

static inline XPathValue xpv_Opt2(XPathOperatorType opt, XPathValue v1, XPathValue v2) {
    XPathValue v;
    switch (opt) {
        case xot_or:
            v.vtype = xvt_boolean;
            v.boolean = v1.boolean || v2.boolean;
            return v;
        case xot_and:
            v.vtype = xvt_boolean;
            v.boolean = v1.boolean && v2.boolean;
            return v;
        case xot_equal:
            v.vtype = xvt_boolean;
            if (v1.vtype == v2.vtype && v1.vtype == xvt_boolean)
                v.boolean = (v1.boolean == v2.boolean);
            else if (v1.vtype == v2.vtype && v1.vtype == xvt_number)
                v.boolean = (v1.number == v2.number);
            else if (v1.vtype == v2.vtype && v1.vtype == xvt_string) {
                int r = strcmp(v1.string, v2.string);
                v.boolean = (r != 0);
            }
            return v;
        case xot_neq:
            v.vtype = xvt_boolean;
            if (v1.vtype == v2.vtype && v1.vtype == xvt_boolean)
                v.boolean = (v1.boolean != v2.boolean);
            else if (v1.vtype == v2.vtype && v1.vtype == xvt_number)
                v.boolean = (v1.number != v2.number);
            else if (v1.vtype == v2.vtype && v1.vtype == xvt_string) {
                int r = strcmp(v1.string, v2.string);
                v.boolean = (r == 0);
            }
            return v;
        case xot_less:
            v.vtype = xvt_boolean;
            v.boolean = (v1.number < v2.number);
            return v;
        case xot_greater:
            v.vtype = xvt_boolean;
            v.boolean = (v1.number > v2.number);
            return v;
        case xot_leq:
            v.vtype = xvt_boolean;
            v.boolean = (v1.number <= v2.number);
            return v;
        case xot_geq:
            v.vtype = xvt_boolean;
            v.boolean = (v1.number >= v2.number);
            return v;
        case xot_add:
            v.vtype = xvt_number;
            v.number = (v1.number + v2.number);
            return v;
        case xot_minus:
            v.vtype = xvt_number;
            v.number = (v1.number - v2.number);
            return v;
        case xot_multiply:
            v.vtype = xvt_number;
            v.number = (v1.number * v2.number);
            return v;
        case xot_div:
            v.vtype = xvt_number;
            v.number = (v1.number / v2.number);
            return v;
        case xot_mod:
            v.vtype = xvt_number;
            v.number = (long long int)(v1.number) % (long long int)(v2.number);
            return v;
    }
}


/**
 * Before calling this function, please replace the '@.proptery' subtree to a single node - xnt_variable
 * You can create it by calling @xpn_CreateVariable(var_name)
 * When I calculate the result, I will try to search the table to find the values of those variable nodes.
 */
static inline XPathValue xpv_Calculate(XPathNode* node, XPathKeyValuePair* table) {
    switch (node->node_type) {
        case xnt_operator: {
            XPathValue v1, v2;
            v1 = xpv_Calculate(node->left, table);
            if (node->right) {
                v2 = xpv_Calculate(node->right, table);
                return xpv_Opt2(node->opt, v1, v2);
            }
            return xpv_Opt1(node->opt, v1);
        }
        case xnt_variable: {
            XPathValue v = xpv_Find(node->string, table);
            return v;
        }
        default: {
            XPathValue v = {xvt_null};
            return v;
        }
    }

}



static inline XPathNode* xpv_ModifyRef(XPathNode* node) {
    XPathNode* ret = 0;
    if (node->left) {
        ret = xpv_ModifyRef(node->left);
        if (ret != NULL) node->left = ret;
    }
    if (node->right) {
        ret = xpv_ModifyRef(node->right);
        if (ret != NULL) node->right = ret;
    }
    if (node->node_type == xnt_reference) {
        return NULL;
    }
    if (node->node_type == xnt_variable) {
        return NULL;
    }
    if (node->node_type == xnt_concat) {
        if (node->left->node_type == xnt_reference) {
            char* name = (char*) malloc(sizeof(char) * (strlen(node->right->string) + 1));
            strcpy(name, node->right->string);
            XPathNode* new_node = xpn_CreateVariable(name);
            free(node->left);
            free(node->right->string);
            free(node->right);
            free(node);
            return new_node;
        } 
        if (node->left->node_type == xnt_variable) {
            char* name = (char*) malloc(sizeof(char) * (
                strlen(node->right->string) + strlen(node->left->string) + 2));
            strcpy(name, node->left->string);
            strcat(name, ".");
            strcat(name, node->right->string);
            free(node->left->string);
            free(node->right->string);
            free(node->right);
            free(node);
            node->left->string = name;
            return node->left;
        }
    }
    return NULL;
}





#ifdef __cplusplus
}
#endif


#endif