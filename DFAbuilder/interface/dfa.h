#ifndef __DFA_H__
#define __DFA_H__

#include <stdint.h>
#include <stdlib.h>

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
    uint32_t array_num; 
    JQ_index_pair* array_index; // array_num
    char** names; // inputs_num
} JQ_DFA;

static inline void jqd_Ctor(JQ_DFA* dfa, uint32_t states_num, uint32_t inputs_num, uint32_t array_num) {
    dfa->states_num = states_num;
    dfa->inputs_num = inputs_num;
    dfa->array_num = array_num;
    dfa->table = (int32_t*) malloc(sizeof(int32_t) * states_num * inputs_num);
    dfa->stop_state = (int32_t*) malloc(sizeof(int32_t) * states_num);
    dfa->array_index = (JQ_index_pair*) malloc(sizeof(JQ_index_pair) * array_num);
    dfa->names = (char**) calloc(sizeof(char*), inputs_num);
}   

static inline JQ_DFA* jqd_Create(uint32_t states_num, uint32_t inputs_num, uint32_t array_num) {
    JQ_DFA* dfa = (JQ_DFA*) malloc(sizeof(JQ_DFA));
    jqd_Ctor(dfa, states_num, inputs_num, array_num);
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


#ifdef __cplusplus
}
#endif

#endif