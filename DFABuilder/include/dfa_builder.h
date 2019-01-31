#ifndef __XPATH_BUILDER_H__
#define __XPATH_BUILDER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "dfa.h"
#include "jsonpath_model.h"

typedef struct JQ_IntVerPair {
    int* value;
    int value_size;
} JQ_IntVerPair;

typedef struct JQ_CONTEXT {
    int states_num;
    JSONPathNode** subtrees;
    JQ_IntVerPair* states_mapping;
    JQ_IntVerPair array_predicate_states;
} JQ_CONTEXT;

extern JQ_DFA* dfa_Create(const char* json_path, JQ_CONTEXT* context);

extern JQ_DFA* dfa_CreateFromAST(JSONPathNode* json_path, JQ_CONTEXT* context);
    
// extern JQ_DFA* dfa_CreateMultiple(int num, const char* json_path[]);

// extern JQ_DFA* dfa_CreateMultipleFromAST(int num, JSONPathNode* json_path[]);

static inline JSONPathNode* dfa_getSubtree(JQ_CONTEXT* ctx, int stop_state) {
    return ctx->subtrees[stop_state];
}

static inline int dfa_getSizeOfMapping(JQ_CONTEXT* ctx, int stop_state) {
    return ctx->states_mapping[stop_state].value_size;
}

static inline int dfa_getValueOfMapping(JQ_CONTEXT* ctx, int stop_state, int idx) {
    return ctx->states_mapping[stop_state].value[idx];
}

static inline int dfa_getSizeOfPredicateStates(JQ_CONTEXT* ctx) {
    return ctx->array_predicate_states.value_size;
} 

static inline int dfa_getPredicateStates(JQ_CONTEXT* ctx, int idx) {
    return ctx->array_predicate_states.value[idx];
}

static inline void dfa_print(JQ_CONTEXT* ctx) {
    for (int i = 0; i < ctx->states_num; ++i) {
        if (ctx->subtrees[i]) {
            printf("state %d:\n", i);
            jpn_Print(ctx->subtrees[i]);
        }
    }
    printf("-----------------------\n");
    for (int i = 0; i < ctx->states_num; ++i) {
        printf("\t%d:", i);
        int* vec = dfa_getValueOfMapping(ctx, i);
        for (int j = 0; j < dfa_getSizeOfMapping(ctx, i); ++j)
            printf("\t%d", vec[j]);
        printf("\n");
    }
    int k = dfa_getSizeOfPredicateStates(ctx);
    printf("predicate states from array:");
    for (int i = 0; i < k; ++i) {
        printf(" %d", dfa_getPredicateStates(ctx, i));
    }
    printf("\n");
    printf("-----------------------\n");
}

#ifdef __cplusplus
}
#endif

#endif 