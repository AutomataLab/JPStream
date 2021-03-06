#ifndef __WORKER_AUTOMATON_H__
#define __WORKER_AUTOMATON_H__

#include "dfa_builder.h"
#include "unit.h"
#include "tuple_list.h"
#include "lexing.h"
#include "stack.h"
#include "constraint.h"

#define OPEN 1
#define CLOSE 0
#define REPROCESS 1
#define NOREPROCESS 0
#define INITIAL 1
#define NOINITIAL 0

typedef struct QueryStatesInfo{
    // current query state
    int query_state; 
    // array index counter
    int count;  
    // matched starting position of object or array
    int matched_start; 
}QueryStatesInfo;

typedef struct WorkerAutomaton{
    //thread id
    int id;
    JSONQueryDFA* query_automaton;
    SyntaxStack syntax_stack;
    QueryStacks query_stacks;
    // pointers for nodes that represent current states on query tree
    QueryStacksElement query_stacks_element;
    //concrete information for current query states
    QueryStatesInfo* query_states_info;
    int num_query_states; 
    // saves information for each data unit
    UnitList unit_list;
    TupleList* tuple_list;
    // whether streaming automaton has data constraint
    int constraint_flag;
    // saves data constraint table
    ConstraintTable* constraint_table;
    // whether the input stream needs to be reprocessed
    int need_reprocess;
    int initial_flag;
}WorkerAutomaton;

// wa -- worker automaton qa -- query automaton
static inline void initWorkerAutomaton(WorkerAutomaton* wa, JSONQueryDFA* qa)
{
    wa->query_automaton = qa;
    initSyntaxStack(&wa->syntax_stack);
    initUnitList(&wa->unit_list);
    wa->tuple_list = createTupleList();
    wa->need_reprocess = NOREPROCESS;
    wa->initial_flag = NOINITIAL;
}

static inline void destroyWorkerAutomaton(WorkerAutomaton* wa)
{
    if(wa->id>0 && wa->tuple_list != NULL)
    {
        freeTupleList(wa->tuple_list);
    }   
}

static inline WorkerAutomaton* createWorkerAutomaton(JSONQueryDFA* qa, int thread_id, ConstraintTable* ct)
{
    WorkerAutomaton* wa = (WorkerAutomaton*)malloc(sizeof(WorkerAutomaton));
    wa->id = thread_id;
    initWorkerAutomaton(wa, qa);
    if(ct==NULL) wa->constraint_flag = CLOSE;
    else
    {
        wa->constraint_flag = OPEN;
        wa->constraint_table = ct;
    }
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

/* 
   below are the implementation of 10 state transition rules based on based on input token, query state, top elements on syntax stack and query stack
   some of these functions are called outside "worker_automaton.h" (like combine() in "parallel_automata_execution.h")
*/
static inline void obj_s(SyntaxStack* ss)
{
    syntaxStackPush(ss, LCB);
}

static inline void obj_e(SyntaxStack* ss)
{
    syntaxStackPop(ss);
}

static inline void val_obj_e(QueryStacksElement* qs_elt, SyntaxStack* ss, QueryStacks* qs)
{
    *qs_elt = queryStacksPop(qs);
    syntaxStackPop(ss);
    syntaxStackPop(ss);
}

static inline void elt_obj_e(SyntaxStack* ss)
{
    syntaxStackPop(ss);
}

static inline void ary_s(JSONQueryDFA* qa, QueryStacksElement* qs_elt, SyntaxStack* ss, QueryStacks* qs)  
{
    syntaxStackPush(ss, LB);
    //push current state into query stack and move onto the next state
    *qs_elt = queryStacksPush(qs, *qs_elt, qa, DFA_ARRAY);
}

static inline void ary_e(QueryStacksElement* qs_elt, SyntaxStack* ss, QueryStacks* qs)
{
    *qs_elt = queryStacksPop(qs);
    syntaxStackPop(ss);
}

static inline void val_ary_e(QueryStacksElement* qs_elt, SyntaxStack* ss, QueryStacks* qs)
{
    queryStacksPop(qs);
    *qs_elt = queryStacksPop(qs); 
    syntaxStackPop(ss); 
    syntaxStackPop(ss); 
}

static inline void elt_ary_e(QueryStacksElement* qs_elt, SyntaxStack* ss, QueryStacks* qs)
{
    *qs_elt = queryStacksPop(qs);
    syntaxStackPop(ss); 
}

static inline void key(JSONQueryDFA* qa, QueryStacksElement* qs_elt, char* key_string, SyntaxStack* ss, QueryStacks* qs)  
{
    syntaxStackPush(ss, KY);
    //push current state into query stack and move onto the next state
    *qs_elt = queryStacksPush(qs, *qs_elt, qa, key_string);
}

static inline void val_pmt(QueryStacksElement* qs_elt, SyntaxStack* ss, QueryStacks* qs)
{
    *qs_elt = queryStacksPop(qs);
    syntaxStackPop(ss);
}

//run worker automaton
void executeWorkerAutomaton(WorkerAutomaton* wa, char* json_stream);

#endif // !__WORKER_AUTOMATON_H__
