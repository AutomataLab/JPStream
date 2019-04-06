#ifndef __XPATH_BUILDER_H__
#define __XPATH_BUILDER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "dfa.h"
#include "jsonpath_model.h"

typedef struct JSONQueryIntVecPair {
    int* value;
    int value_size;
} JSONQueryIntVecPair;

typedef struct JSONQueryDFAContext {
    int states_num;
    ASTNode** subtrees;
    JSONQueryIntVecPair* states_mapping;
    JSONQueryIntVecPair array_predicate_states;
} JSONQueryDFAContext;

extern JSONQueryDFA* buildJSONQueryDFA(const char* json_path, JSONQueryDFAContext* context);

extern JSONQueryDFA* buildJSONQueryDFAFromAST(ASTNode* json_path, JSONQueryDFAContext* context);
    
// extern JSONQueryDFA* buildJSONQueryDFAMultiple(int num, const char* json_path[]);

// extern JSONQueryDFA* buildJSONQueryDFAMultipleFromAST(int num, ASTNode* json_path[]);

static inline ASTNode* getContextSubtree(JSONQueryDFAContext* ctx, int stop_state) {
    return ctx->subtrees[stop_state];
}

static inline int getContextSizeOfMapping(JSONQueryDFAContext* ctx, int stop_state) {
    return ctx->states_mapping[stop_state].value_size;
}

static inline int getContextValueOfMapping(JSONQueryDFAContext* ctx, int stop_state, int idx) {
    return ctx->states_mapping[stop_state].value[idx];
}

static inline int getContextSizeOfPredicateStates(JSONQueryDFAContext* ctx) {
    return ctx->array_predicate_states.value_size;
} 

static inline int getContextPredicateStates(JSONQueryDFAContext* ctx, int idx) {
    return ctx->array_predicate_states.value[idx];
}

static inline void printJSONQueryDFAContext(JSONQueryDFAContext* ctx) {
    for (int i = 0; i < ctx->states_num; ++i) {
        if (ctx->subtrees[i]) {
            printf("state %d:\n", i);
            printJsonPathAST(ctx->subtrees[i]);
        }
    }
    printf("-----------------------\n");
    for (int i = 0; i < ctx->states_num; ++i) {
        printf("\t%d:", i);
        for (int j = 0; j < getContextSizeOfMapping(ctx, i); ++j)
            printf("\t%d", getContextValueOfMapping(ctx, i, j));
        printf("\n");
    }
    int k = getContextSizeOfPredicateStates(ctx);
    printf("predicate states from array:");
    for (int i = 0; i < k; ++i) {
        printf(" %d", getContextPredicateStates(ctx, i));
    }
    printf("\n");
    printf("-----------------------\n");
}

#ifdef __cplusplus
}
#endif

#endif 