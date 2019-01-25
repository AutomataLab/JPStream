#ifndef __DFA_H__
#define __DFA_H__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct JQ_index_pair {
    int32_t lower, upper;
} JQ_index_pair;

typedef struct JQ_DFA {
    uint32_t states_num, inputs_num;
    int32_t* table; // states_num x inputs_num
    int32_t* stop_state; // states_num
    JQ_index_pair* array_index; // states_num
    char** names; // inputs_num
} JQ_DFA;

static inline void jqd_Ctor(JQ_DFA* dfa, uint32_t states_num, uint32_t inputs_num) {
    dfa->states_num = states_num;
    dfa->inputs_num = inputs_num;
    dfa->table = (int32_t*) calloc(sizeof(int32_t), states_num * inputs_num);
    dfa->stop_state = (int32_t*) calloc(sizeof(int32_t), states_num);
    dfa->array_index = (JQ_index_pair*) calloc(sizeof(JQ_index_pair), states_num);
    dfa->names = (char**) calloc(sizeof(char*), inputs_num);
}   

static inline JQ_DFA* jqd_Create(uint32_t states_num, uint32_t inputs_num) {
    JQ_DFA* dfa = (JQ_DFA*) malloc(sizeof(JQ_DFA));
    jqd_Ctor(dfa, states_num, inputs_num);
    return dfa;
}

static inline void jqd_Dtor(JQ_DFA* dfa) {
    if (dfa->table) free(dfa->table);
    if (dfa->stop_state) free(dfa->stop_state);
    if (dfa->array_index) free(dfa->array_index);
    if (dfa->names) {
        for (int i = 0; i < dfa->inputs_num; ++i) 
            if (dfa->names[i]) free(dfa->names[i]);
        free(dfa->names);
    }
}

static inline void jqd_Free(JQ_DFA** dfa) {
    if (dfa && *dfa) {
        jqd_Dtor(*dfa);
        free(*dfa);
        *dfa = NULL;
    }
}

static inline int32_t jqd_nextState(JQ_DFA* dfa, uint32_t state, uint32_t input) {
    return dfa->table[state * dfa->inputs_num + input];
}

static inline int32_t jqd_getStopState(JQ_DFA* dfa, uint32_t state) {
    return dfa->stop_state[state];
}

static inline const char* jqd_getName(JQ_DFA* dfa, uint32_t input) {
    return dfa->names[input];
}

static inline JQ_index_pair jqd_getArrayIndex(JQ_DFA* dfa, uint32_t array_id) {
    return dfa->array_index[array_id];
}

static inline uint32_t jqd_getStatesNum(JQ_DFA* dfa) {
    return dfa->states_num;
}

static inline uint32_t jqd_getInputsNum(JQ_DFA* dfa) {
    return dfa->inputs_num;
}

static inline void jqd_print(JQ_DFA* dfa) {
    
    uint32_t inputs = jqd_getInputsNum(dfa);
    uint32_t states = jqd_getStatesNum(dfa);

    printf("\tother\tarray");
    for (int i = 3; i < inputs; ++i) {
        printf("\t%s", jqd_getName(dfa, i));
    }
    printf("\n");

    for (int i = 1; i< inputs; ++i) {
        printf("\t%d",i);
    }
    printf("\n");
    for (int i = 1; i< states; ++i) {
        printf("s%d",i);
        int l;
        if ((l = jqd_getStopState(dfa, i)) != 0) 
            printf("#%d",l);
        for (int j = 1; j< inputs; ++j) {
            int next = jqd_nextState(dfa, i, j);
            if (next != 0)
                printf("\ts%d", next);
            else
                printf("\t");
        }
        printf("\n");
    }
    printf("\narray index:\n");
    for (int i = 1; i< states; ++i) {
        printf("\t%d",i);
    }
    printf("\n");
    for (int i = 1; i< states; ++i) {
        JQ_index_pair pair = jqd_getArrayIndex(dfa, i);
        if (pair.lower || pair.upper) {
            if (pair.upper == pair.lower + 1)
                printf("\t[%d]", pair.lower);
            else
                printf("\t[%d:%d]", pair.lower, pair.upper);
        } else printf("\t");
    }
    printf("\n");
}

#ifdef __cplusplus
}
#endif

#endif