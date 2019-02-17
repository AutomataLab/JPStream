#ifndef __XPATH_MODEL_H__
#define __XPATH_MODEL_H__


#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ASTNodeType {
    jnt_root, jnt_concat, jnt_parent_concat, jnt_descendants, jnt_id, jnt_operator, jnt_predicate, jnt_wildcard,
    jnt_number, jnt_string, jnt_function, jnt_variable, jnt_attribute, jnt_reference, jnt_script, jnt_fliter, jnt_range
} ASTNodeType;

typedef enum JSONPathOperatorType {
    jot_or = 0, jot_and, jot_not, // boolean
    jot_equal, jot_less, jot_greater, jot_neq, jot_leq, jot_geq,  // comparing
    jot_add, jot_minus, jot_multiply, jot_div, jot_mod, // calculation
    jot_union // node-set union
} JSONPathOperatorType;

typedef struct ASTNode {
    ASTNodeType node_type; 
    union {
        double number;
        char* string;
        bool boolean;
        JSONPathOperatorType opt;
    };
    struct ASTNode *left, *right;
} ASTNode;

static inline ASTNode* ASTNodeCreate() {
    ASTNode* jpn = (ASTNode*) malloc(sizeof(ASTNode));
    jpn->left = NULL;
    jpn->right = NULL;
    return jpn;
}

static inline ASTNode* ASTNodeCreateNumber(double data) {
    ASTNode* jpn = ASTNodeCreate();
    jpn->node_type = jnt_number;
    jpn->number = data;
    return jpn;
}

static inline ASTNode* ASTNodeCreateString(char* data) {
    ASTNode* jpn = ASTNodeCreate();
    jpn->node_type = jnt_string;
    jpn->string = data;
    return jpn;
}

static inline ASTNode* ASTNodeCreateID(char* name) {
    ASTNode* jpn = ASTNodeCreate();
    jpn->node_type = jnt_id;
    jpn->string = name;
    return jpn;
}

static inline ASTNode* ASTNodeCreateFunction(char* name) {
    ASTNode* jpn = ASTNodeCreate();
    jpn->node_type = jnt_function;
    jpn->string = name;
    return jpn;
}

static inline ASTNode* ASTNodeCreateAttribute(char* name) {
    ASTNode* jpn = ASTNodeCreate();
    jpn->node_type = jnt_attribute;
    jpn->string = name;
    return jpn;
}

static inline ASTNode* ASTNodeCreateVariable(char* name) {
    ASTNode* jpn = ASTNodeCreate();
    jpn->node_type = jnt_variable;
    jpn->string = name;
    return jpn;
}

static inline ASTNode* ASTNodeCreateRoot() {
    ASTNode* jpn = ASTNodeCreate();
    jpn->node_type = jnt_root;
    return jpn;
}

static inline ASTNode* ASTNodeCreateWildcard() {
    ASTNode* jpn = ASTNodeCreate();
    jpn->node_type = jnt_wildcard;
    return jpn;
}

static inline ASTNode* ASTNodeCreateRef() {
    ASTNode* jpn = ASTNodeCreate();
    jpn->node_type = jnt_reference;
    return jpn;
}


static inline ASTNode* ASTNodeCreateConcat(ASTNode* left, ASTNode* right) {
    ASTNode* jpn = ASTNodeCreate();
    jpn->node_type = jnt_concat;
    jpn->left = left;
    jpn->right = right;
    return jpn;
}

static inline ASTNode* ASTNodeCreateRange(ASTNode* left, ASTNode* right) {
    ASTNode* jpn = ASTNodeCreate();
    jpn->node_type = jnt_range;
    jpn->left = left;
    jpn->right = right;
    return jpn;
}

static inline ASTNode* ASTNodeCreateParentConcat(ASTNode* left, ASTNode* right) {
    ASTNode* jpn = ASTNodeCreate();
    jpn->node_type = jnt_parent_concat;
    jpn->left = left;
    jpn->right = right;
    return jpn;
}


static inline ASTNode* ASTNodeCreatePredicate(ASTNode* left, ASTNode* right) {
    ASTNode* jpn = ASTNodeCreate();
    jpn->node_type = jnt_predicate;
    jpn->left = left;
    jpn->right = right;
    return jpn;
}

static inline ASTNode* ASTNodeCreateOperator(JSONPathOperatorType opt, ASTNode* left, ASTNode* right) {
    ASTNode* jpn = ASTNodeCreate();
    jpn->node_type = jnt_operator;
    jpn->opt = opt;
    jpn->left = left;
    jpn->right = right;
    return jpn;
}

static inline ASTNode* ASTNodeCreateOperatorOne(JSONPathOperatorType opt, ASTNode* left) {
    ASTNode* jpn = ASTNodeCreate();
    jpn->node_type = jnt_operator;
    jpn->opt = opt;
    jpn->left = left;
    return jpn;
}


static inline ASTNode* ASTNodeCreateScript(ASTNode* left) {
    ASTNode* jpn = ASTNodeCreate();
    jpn->node_type = jnt_script;
    jpn->left = left;
    return jpn;
}

static inline ASTNode* ASTNodeCreateFliter(ASTNode* left) {
    ASTNode* jpn = ASTNodeCreate();
    jpn->node_type = jnt_fliter;
    jpn->left = left;
    return jpn;
}


static inline void printIndentation(int n) {
    for (int i = 0; i < n; ++i) printf("  ");
}
/**
    jot_or = 0, jot_and, // boolean
    jot_equal, jot_less, jot_greater, jot_neq, jot_leq, jot_geq,  // comparing
    jot_add, jot_minus, jot_multiply, jot_div, jot_mod, // calculation
    jot_union // node-set union
*/
static inline void printJsonPathOperator(JSONPathOperatorType opt) {
    const char* name_mapping[] = {
        "or", "and", "!", 
        "=", "<", ">", "!=", "<=", ">=",
        "+", "-", "*", "div", "mod",
        "|"
    };
    printf("%s", name_mapping[opt]);
}


static inline void printJsonPathSubAST(ASTNode* node, int depth, bool print_return) {
    bool children_print_return = true;
    int right_is_child = 0;
    printIndentation(depth);
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
            printJsonPathOperator(node->opt);
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
    if (node->left) printJsonPathSubAST(node->left, depth+1, children_print_return);
    if (node->right) printJsonPathSubAST(node->right, depth+1, children_print_return);
}

static inline void printJsonPathAST(ASTNode* root) {
    printJsonPathSubAST(root, 0, true);
} 


#ifdef __cplusplus
}
#endif

#endif 