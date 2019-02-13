#ifndef __DFA_H__
#define __DFA_H__

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct JSONQueryIndexPair {
    int32_t lower, upper;
} JSONQueryIndexPair;

typedef struct JSONQueryDFA {
    uint32_t states_num, inputs_num;
    int32_t* table; // states_num x inputs_num
    int32_t* stop_state; // states_num
    int32_t* accept_type; // states_num : 1 is output & 2 is predicate
    JSONQueryIndexPair* array_index; // states_num
    char** names; // inputs_num
} JSONQueryDFA;

#define DFA_ARRAY ((const char*)0)

#define DFA_OUTPUT_TYPE (1)
#define DFA_PREDICATE_TYPE (2)

static inline void initJSONQueryDFA(JSONQueryDFA* dfa, uint32_t states_num, uint32_t inputs_num) {
    dfa->states_num = states_num;
    dfa->inputs_num = inputs_num;
    dfa->table = (int32_t*) calloc(sizeof(int32_t), states_num * inputs_num);
    dfa->stop_state = (int32_t*) calloc(sizeof(int32_t), states_num);
    dfa->accept_type = (int32_t*) calloc(sizeof(int32_t), states_num);
    dfa->array_index = (JSONQueryIndexPair*) calloc(sizeof(JSONQueryIndexPair), states_num);
    dfa->names = (char**) calloc(sizeof(char*), inputs_num);
}   

static inline JSONQueryDFA* createJSONQueryDFA(uint32_t states_num, uint32_t inputs_num) {
    JSONQueryDFA* dfa = (JSONQueryDFA*) malloc(sizeof(JSONQueryDFA));
    initJSONQueryDFA(dfa, states_num, inputs_num);
    return dfa;
}

static inline void destoryJSONQueryDFA(JSONQueryDFA* dfa) {
    if (dfa->table) free(dfa->table);
    if (dfa->stop_state) free(dfa->stop_state);
    if (dfa->array_index) free(dfa->array_index);
    if (dfa->names) {
        for (int i = 0; i < dfa->inputs_num; ++i) 
            if (dfa->names[i]) free(dfa->names[i]);
        free(dfa->names);
    }
}

static inline void freeJSONQueryDFA(JSONQueryDFA** dfa) {
    if (dfa && *dfa) {
        destoryJSONQueryDFA(*dfa);
        free(*dfa);
        *dfa = NULL;
    }
}

static inline int32_t dfaNextState(JSONQueryDFA* dfa, uint32_t state, uint32_t input) {
    return dfa->table[state * dfa->inputs_num + input];
}

static inline int32_t getDFAStopState(JSONQueryDFA* dfa, uint32_t state) {
    return dfa->stop_state[state];
}

static inline int32_t getDFAAcceptType(JSONQueryDFA* dfa, uint32_t state) {
    return dfa->accept_type[state];
}

static inline const char* getDFAInputType(JSONQueryDFA* dfa, uint32_t input) {
    return dfa->names[input];
}

static inline JSONQueryIndexPair getDFAArrayIndex(JSONQueryDFA* dfa, uint32_t array_id) {
    return dfa->array_index[array_id];
}

static inline uint32_t getDFAStatesNumber(JSONQueryDFA* dfa) {
    return dfa->states_num;
}

static inline uint32_t getDFAInputsNumber(JSONQueryDFA* dfa) {
    return dfa->inputs_num;
}

static inline int32_t dfaNextStateByStr(JSONQueryDFA* dfa, uint32_t state, const char* input) {
    int cinput; bool found = false;
    if (input == NULL) cinput = 2;
    else {
        for (cinput = 3; cinput < getDFAInputsNumber(dfa); ++cinput) {
            if (dfa->table[state * dfa->inputs_num + cinput] != 0 &&
                strcmp(getDFAInputType(dfa, cinput), input) == 0) {
                found = true; break;
            }
        }
        if (!found) cinput = 1;
    }
    return dfa->table[state * dfa->inputs_num + cinput];
}

static inline void printDFA(JSONQueryDFA* dfa) {
    
    uint32_t inputs = getDFAInputsNumber(dfa);
    uint32_t states = getDFAStatesNumber(dfa);

    printf("\t%-8.7s%-8.7s", "other", "array");
    for (int i = 3; i < inputs; ++i) {
        printf("%-8.7s", getDFAInputType(dfa, i));
    }
    printf("\n");

    for (int i = 1; i< inputs; ++i) {
        printf("\t%d",i);
    }
    printf("\n");
    for (int i = 1; i< states; ++i) {
        printf("s%d",i);
        int l;
        if ((l = getDFAStopState(dfa, i)) != 0) {
            printf("#%d",l);
        }
        if (getDFAAcceptType(dfa, i) == DFA_OUTPUT_TYPE) 
            printf("!");
        if (getDFAAcceptType(dfa, i) == DFA_PREDICATE_TYPE) 
            printf("*");
        for (int j = 1; j< inputs; ++j) {
            int next = dfaNextState(dfa, i, j);
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
        JSONQueryIndexPair pair = getDFAArrayIndex(dfa, i);
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