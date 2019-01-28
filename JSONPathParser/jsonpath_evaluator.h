#ifndef __XPATH_VERIFY_H__
#define __XPATH_VERIFY_H__

#include "jsonpath_model.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum JSONPathValueType {
    jvt_null = 0, jvt_number, jvt_string, jvt_boolean
} JSONPathValueType;

typedef struct JSONPathValue {
    JSONPathValueType vtype;
    union {
        double number;
        char* string;
        bool boolean;
    };
} JSONPathValue;

typedef struct JSONPathKeyValuePair {
    char* key;
    JSONPathValue value;
} JSONPathKeyValuePair;

static inline JSONPathValue jpe_Find(const char* name, JSONPathKeyValuePair* table) {
    int i = 0;
    while (table[i].key != NULL) {
        int r = strcmp(table[i].key, name);
        if (r == 0) {
            return table[i].value;
        }
        i++;
    }
    JSONPathValue v = {jvt_null};
    return v;
}

static inline JSONPathValue jpe_Opt1(JSONPathOperatorType opt, JSONPathValue v1) {
    JSONPathValue v = {jvt_null};
    return v;
}

/**
    jot_or = 0, jot_and, // boolean
    jot_equal, jot_less, jot_greater, jot_neq, jot_leq, jot_geq,  // comparing
    jot_add, jot_minus, jot_multiply, jot_div, jot_mod, // calculation
    jot_union // node-set union
*/

static inline JSONPathValue jpe_Opt2(JSONPathOperatorType opt, JSONPathValue v1, JSONPathValue v2) {
    JSONPathValue v;
    switch (opt) {
        case jot_or:
            v.vtype = jvt_boolean;
            v.boolean = v1.boolean || v2.boolean;
            return v;
        case jot_and:
            v.vtype = jvt_boolean;
            v.boolean = v1.boolean && v2.boolean;
            return v;
        case jot_equal:
            v.vtype = jvt_boolean;
            if (v1.vtype == v2.vtype && v1.vtype == jvt_boolean)
                v.boolean = (v1.boolean == v2.boolean);
            else if (v1.vtype == v2.vtype && v1.vtype == jvt_number)
                v.boolean = (v1.number == v2.number);
            else if (v1.vtype == v2.vtype && v1.vtype == jvt_string) {
                int r = strcmp(v1.string, v2.string);
                v.boolean = (r != 0);
            }
            return v;
        case jot_neq:
            v.vtype = jvt_boolean;
            if (v1.vtype == v2.vtype && v1.vtype == jvt_boolean)
                v.boolean = (v1.boolean != v2.boolean);
            else if (v1.vtype == v2.vtype && v1.vtype == jvt_number)
                v.boolean = (v1.number != v2.number);
            else if (v1.vtype == v2.vtype && v1.vtype == jvt_string) {
                int r = strcmp(v1.string, v2.string);
                v.boolean = (r == 0);
            }
            return v;
        case jot_less:
            v.vtype = jvt_boolean;
            v.boolean = (v1.number < v2.number);
            return v;
        case jot_greater:
            v.vtype = jvt_boolean;
            v.boolean = (v1.number > v2.number);
            return v;
        case jot_leq:
            v.vtype = jvt_boolean;
            v.boolean = (v1.number <= v2.number);
            return v;
        case jot_geq:
            v.vtype = jvt_boolean;
            v.boolean = (v1.number >= v2.number);
            return v;
        case jot_add:
            v.vtype = jvt_number;
            v.number = (v1.number + v2.number);
            return v;
        case jot_minus:
            v.vtype = jvt_number;
            v.number = (v1.number - v2.number);
            return v;
        case jot_multiply:
            v.vtype = jvt_number;
            v.number = (v1.number * v2.number);
            return v;
        case jot_div:
            v.vtype = jvt_number;
            v.number = (v1.number / v2.number);
            return v;
        case jot_mod:
            v.vtype = jvt_number;
            v.number = (long long int)(v1.number) % (long long int)(v2.number);
            return v;
    }
    return v;
}


/**
 * Before calling this function, please replace the '@.proptery' subtree to a single node - jnt_variable
 * You can create it by calling @jpn_CreateVariable(var_name)
 * When I calculate the result, I will try to search the table to find the values of those variable nodes.
 */
static inline JSONPathValue jpe_Evaluate(JSONPathNode* node, JSONPathKeyValuePair* table) {
    switch (node->node_type) {
        case jnt_operator: {
            JSONPathValue v1, v2;
            v1 = jpe_Evaluate(node->left, table);
            if (node->right) {
                v2 = jpe_Evaluate(node->right, table);
                return jpe_Opt2(node->opt, v1, v2);
            }
            return jpe_Opt1(node->opt, v1);
        }
        case jnt_variable: {
            JSONPathValue v = jpe_Find(node->string, table);
            return v;
        }
        default: {
            JSONPathValue v = {jvt_null};
            return v;
        }
    }

}



static inline JSONPathNode* jpe_ModifyRef(JSONPathNode* node) {
    JSONPathNode* ret = 0;
    if (node->left) {
        ret = jpe_ModifyRef(node->left);
        if (ret != NULL) node->left = ret;
    }
    if (node->right) {
        ret = jpe_ModifyRef(node->right);
        if (ret != NULL) node->right = ret;
    }
    if (node->node_type == jnt_reference) {
        return NULL;
    }
    if (node->node_type == jnt_variable) {
        return NULL;
    }
    if (node->node_type == jnt_concat) {
        if (node->left->node_type == jnt_reference) {
            char* name = (char*) malloc(sizeof(char) * (strlen(node->right->string) + 1));
            strcpy(name, node->right->string);
            JSONPathNode* new_node = jpn_CreateVariable(name);
            free(node->left);
            free(node->right->string);
            free(node->right);
            free(node);
            return new_node;
        } 
        if (node->left->node_type == jnt_variable) {
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