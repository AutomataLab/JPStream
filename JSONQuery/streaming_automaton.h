#ifndef __STREAMING_AUTOMATON_H__
#define __STREAMING_AUTOMATON_H__

#include "dfa_builder.h"
#include "stack.h"
#include "tuple_list.h"
#include "lexing.h"

typedef struct StreamingAutomaton{
    JSONQueryDFA* query_automaton;
    SyntaxStack syntax_stack;
    QueryStack query_stack; 
    TupleList* tuple_list;
    //a structure type, which saves query state, counter, and matched start position of object or array
    QueryStackElement state_info; 
}StreamingAutomaton;

// sa -- streaming automaton qa -- query automaton
static inline void initStreamingAutomaton(StreamingAutomaton* sa, JSONQueryDFA* qa)
{
    sa->query_automaton = qa;
    initSyntaxStack(&sa->syntax_stack);
    initQueryStack(&sa->query_stack);
    sa->tuple_list = createTupleList(); 
    //initialize starting state, counter and matched start position
    sa->state_info.query_state = 1;   
    sa->state_info.count = 0;  
    sa->state_info.matched_start = -1;
}

static inline void destroyStreamingAutomaton(StreamingAutomaton* sa)
{
    if(sa->query_automaton != NULL)
    {
        destoryJSONQueryDFA(sa->query_automaton);
    }
    if(sa->tuple_list != NULL)
    {
        freeTupleList(sa->tuple_list);
    }  
}

void executeAutomaton(StreamingAutomaton* sa, char* json_stream);
#endif // !__STREAMING_AUTOMATON_H__
