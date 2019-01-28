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
} JQ_CONTEXT;

extern JQ_DFA* jpb_Create(const char* json_path, JQ_CONTEXT* context);

extern JQ_DFA* jpb_CreateFromAST(JSONPathNode* json_path, JQ_CONTEXT* context);
    
// extern JQ_DFA* jpb_CreateMultiple(int num, const char* json_path[]);

// extern JQ_DFA* jpb_CreateMultipleFromAST(int num, JSONPathNode* json_path[]);

static inline JSONPathNode* jqc_getSubtree(JQ_CONTEXT* ctx, int stop_state) {
    return ctx->subtrees[stop_state];
}

static inline int jqc_getSizeOfMapping(JQ_CONTEXT* ctx, int stop_state) {
    return ctx->states_mapping[stop_state].value_size;
}

static inline int* jqc_getValueOfMapping(JQ_CONTEXT* ctx, int stop_state) {
    return ctx->states_mapping[stop_state].value;
}


static inline void jqc_print(JQ_CONTEXT* ctx) {
    for (int i = 0; i < ctx->states_num; ++i) {
        if (ctx->subtrees[i]) {
            printf("state %d:\n", i);
            jpn_Print(ctx->subtrees[i]);
        }
    }
    printf("-----------------------\n");
    for (int i = 0; i < ctx->states_num; ++i) {
        printf("\t%d:", i);
        int* vec = jqc_getValueOfMapping(ctx, i);
        for (int j = 0; j < jqc_getSizeOfMapping(ctx, i); ++j)
            printf("\t%d", vec[j]);
        printf("\n");
    }
    printf("-----------------------\n");
}

#ifdef __cplusplus
}
#endif

#endif 