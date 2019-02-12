#ifndef __STREAMING_AUTOMATON_H__
#define __STREAMING_AUTOMATON_H__

#include "dfa_builder.h"
#include "stack.h"
//#include "list.h"
#include "tuple_list.h"
#include "lexing.h"

typedef struct StreamingAutomaton{
    JSONQueryDFA* query_automaton;
    SyntaxStack syntax_stack;
    QueryStack query_stack; 
    //List* output_list;
    TupleList* tuple_list;
    QueryStackElement current_state; //remove this and add three new variables
}StreamingAutomaton;


static inline void initStreamingAutomaton(StreamingAutomaton* streaming_automaton, JSONQueryDFA* query_automaton)
{
    //streaming_automaton->json_stream = json_stream;  //remove this
    streaming_automaton->query_automaton = query_automaton;
    initSyntaxStack(&streaming_automaton->syntax_stack);
    initQueryStack(&streaming_automaton->query_stack);
    //initLexer(&streaming_automaton->lexer, json_stream);
    streaming_automaton->tuple_list = createTupleList(); 
    ///streaming_automaton->output_list = createList();
    //change the implemenations for the following 4 sentences
    streaming_automaton->current_state.state = 1;   //starting state
    streaming_automaton->current_state.count = 0;  
    streaming_automaton->current_state.matched_start = -1;
    //streaming_automaton->current_state.end_obj = -1;
}

static inline void destroyStreamingAutomaton(StreamingAutomaton* streaming_automaton)
{
    /*if(streaming_automaton->json_stream != NULL)  //remove this
    {
        jps_freeJSONStream(streaming_automaton->json_stream);
    }*/
    if(streaming_automaton->query_automaton != NULL)
    {
        destoryJSONQueryDFA(streaming_automaton->query_automaton);
    }
    /*if(streaming_automaton->output_list != NULL)
    {
        freeList(streaming_automaton->output_list);
    }*/
    if(streaming_automaton->tuple_list != NULL)
    {
        freeTupleList(streaming_automaton->tuple_list);
    }  
}

void executeAutomaton(StreamingAutomaton* streaming_automaton, char* json_stream);
#endif // !__STREAMING_AUTOMATON_H__
