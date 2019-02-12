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
    //a structure type, which saves query_state, counter, and matched start position of object or array
    QueryStackElement current_state; 
}StreamingAutomaton;


static inline void initStreamingAutomaton(StreamingAutomaton* streaming_automaton, JSONQueryDFA* query_automaton)
{
    streaming_automaton->query_automaton = query_automaton;
    initSyntaxStack(&streaming_automaton->syntax_stack);
    initQueryStack(&streaming_automaton->query_stack);
    streaming_automaton->tuple_list = createTupleList(); 
    //initialize starting state, counter and matched start position
    streaming_automaton->current_state.state = 1;   
    streaming_automaton->current_state.count = 0;  
    streaming_automaton->current_state.matched_start = -1;
}

static inline void destroyStreamingAutomaton(StreamingAutomaton* streaming_automaton)
{
    if(streaming_automaton->query_automaton != NULL)
    {
        destoryJSONQueryDFA(streaming_automaton->query_automaton);
    }
    if(streaming_automaton->tuple_list != NULL)
    {
        freeTupleList(streaming_automaton->tuple_list);
    }  
}

void executeAutomaton(StreamingAutomaton* streaming_automaton, char* json_stream);
#endif // !__STREAMING_AUTOMATON_H__
