#ifndef __WORKER_AUTOMATON_H__
#define __WORKER_AUTOMATON_H__

#include "dfa_builder.h"
#include "unit_p.h"
#include "tuple_list.h"
#include "lexing.h"

typedef struct QueryStatesInfo{
    // current query state
    int query_state; 
    // array index counter
    int count;  
    // matched starting position of object or array
    int matched_start; 
}QueryStatesInfo;

typedef struct WorkerAutomaton{
    JSONQueryDFA* query_automaton;
    //output information for worker automaton
    SyntaxStack syntax_stack;
    QueryStack query_stack;
    //pointers for nodes that represent current states on query tree
    QueryStackElement query_stack_element;
    //concrete information for current query states
    QueryStatesInfo* query_states_info;
    int num_query_states; 

    UnitList unit_list;
    TupleList* tuple_list;
}WorkerAutomaton;

// wa -- worker automaton qa -- query automaton
static inline void initWorkerAutomaton(WorkerAutomaton* wa, JSONQueryDFA* qa)
{
    wa->query_automaton = qa;
    initSyntaxStack(&wa->syntax_stack);

    //initialize query stack
    int query_state[MAX_STATE];
    int num_query_state = getDFAStatesNumber(qa);
    int i; 
    for(i = 1; i<=num_query_state; i++)
        query_state[i-1] = i-1;
    wa->query_stack_element = initQueryStack(&wa->query_stack, query_state, num_query_state); 

    //initialize information for starting states (query state, counter, matched start position)
    wa->query_states_info = (QueryStatesInfo*)malloc(num_query_state*sizeof(QueryStatesInfo));
    for(i = 0; i<num_query_state; i++)
    {
        wa->query_states_info[i].query_state = query_state[i];
        wa->query_states_info[i].count = 0;
        wa->query_states_info[i].matched_start = INVALID;
    }

    initUnitList(&wa->unit_list);
    wa->tuple_list = createTupleList();
}

static inline void destroyWorkerAutomaton(WorkerAutomaton* wa)
{
    if(wa->query_automaton != NULL)
    {
        destoryJSONQueryDFA(wa->query_automaton);
    }
    if(wa->tuple_list != NULL)
    {
        freeTupleList(wa->tuple_list);
    }  
}

static inline WorkerAutomaton* createWorkerAutomaton(JSONQueryDFA* qa)
{
    WorkerAutomaton* wa = (WorkerAutomaton*)malloc(sizeof(WorkerAutomaton));
    initWorkerAutomaton(wa, qa);
    return wa;
}

static inline void freeWorkerAutomaton(WorkerAutomaton* wa)
{
    if(wa!=NULL)
    {
        destroyWorkerAutomaton(wa);
        free(wa);
    }
}

void executeWorkerAutomaton(WorkerAutomaton* wa, char* json_stream);

#endif // !__WORKER_AUTOMATON_H__
