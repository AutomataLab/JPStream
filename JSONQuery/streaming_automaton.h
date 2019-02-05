#ifndef __STREAMING_AUTOMATON_H__
#define __STREAMING_AUTOMATON_H__

#include "json_stream.h"
#include "dfa_builder.h"
#include "stack.h"
#include "output.h"

typedef struct StreamingAutomaton{
    JSONStream* json_stream;
    JQ_DFA* query_automaton;
    SyntaxStack syntax_stack;
    QueryStack query_stack;
    OutputList* output_list;
    QueryElement current_state;
}StreamingAutomaton;


static inline void jsr_StreamingAutomatonCtor(StreamingAutomaton* streaming_automaton, JSONStream* json_stream, JQ_DFA* query_automaton)
{
    streaming_automaton->json_stream = json_stream;
    streaming_automaton->query_automaton = query_automaton;
    //initialize stacks and output list
    jps_SyntaxStackCtor(&streaming_automaton->syntax_stack);
    jps_QueryStackCtor(&streaming_automaton->query_stack);
    streaming_automaton->output_list = jpo_createOutputList();
    streaming_automaton->current_state.state = 1;   //starting state
    streaming_automaton->current_state.count = 0;  
}

static inline void jsr_StreamingAutomatonDtor(StreamingAutomaton* streaming_automaton)
{
    if(streaming_automaton->json_stream != NULL)
    {
        jps_freeJSONStream(streaming_automaton->json_stream);
    }
    if(streaming_automaton->query_automaton != NULL)
    {
        jqd_Dtor(streaming_automaton->query_automaton);
    }
    if(streaming_automaton->output_list != NULL)
    {
        jpo_freeOutputList(streaming_automaton->output_list);
    }
}

void jsr_automaton_execution(StreamingAutomaton* streaming_automaton);
#endif // !__STREAMING_AUTOMATON_H__
