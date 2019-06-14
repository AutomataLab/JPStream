#ifndef __STREAMING_AUTOMATON_H__
#define __STREAMING_AUTOMATON_H__

#include "dfa_builder.h"
#include "stack.h"
#include "tuple_list.h"
#include "lexing.h"
#include "constraint.h"

#define OPEN 1
#define CLOSE 0
#define INPROGRESS 0
#define FINISH 1

typedef struct StreamingAutomaton{
    JSONQueryDFA* query_automaton;
    SyntaxStack syntax_stack;
    QueryStack query_stack; 
    TupleList* tuple_list;
    // current query state
    int query_state; 
    // array index counter
    int count;  
    // matched starting position of object or array
    int matched_start; 
    // whether streaming automaton has data constraint
    int constraint_flag;
    // saves data constraint table
    ConstraintTable* constraint_table;
    int finish_flag;
}StreamingAutomaton;

// sa -- streaming automaton qa -- query automaton
static inline void initStreamingAutomaton(StreamingAutomaton* sa, JSONQueryDFA* qa)
{
    sa->query_automaton = qa;
    initSyntaxStack(&sa->syntax_stack);
    initQueryStack(&sa->query_stack);
    sa->tuple_list = createTupleList(); 
    //initialize starting state, counter and matched start position
    sa->query_state = 1;   
    sa->count = 0;  
    sa->matched_start = INVALID;
    //by default don't generate any data constraints
    sa->constraint_flag = CLOSE;
    sa->constraint_table = NULL;
    sa->finish_flag = INPROGRESS;
}

static inline void destroyStreamingAutomaton(StreamingAutomaton* sa)
{
    if(sa->tuple_list != NULL && sa->finish_flag == INPROGRESS)
    {   
        freeTupleList(sa->tuple_list); 
        sa->tuple_list = NULL;
    } 
    if(sa->finish_flag == INPROGRESS)
    {
        sa->tuple_list = createTupleList();
    }
}

// data_constraint_flag: whether sa needs to generate data constraints
void executeAutomaton(StreamingAutomaton* sa, char* json_stream, int data_constraint_flag);
#endif // !__STREAMING_AUTOMATON_H__
