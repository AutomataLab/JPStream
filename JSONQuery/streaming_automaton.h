#ifndef __STREAMING_AUTOMATON_H__
#define __STREAMING_AUTOMATON_H__

#include "json_stream.h"
#include "dfa_builder.h"
#include "stack.h"
#include "output.h"
#include "lexing.h"

typedef struct StreamingAutomaton{
    JSONStream* json_stream;  //remove this
    JQ_DFA* query_automaton;
    SyntaxStack syntax_stack;
    QueryStack query_stack;
    Lexer lexer;              //remove this
    OutputList* output_list;
    QueryElement current_state;  //remove this and add three new variables
}StreamingAutomaton;


static inline void jsr_StreamingAutomatonCtor(StreamingAutomaton* streaming_automaton, JSONStream* json_stream, JQ_DFA* query_automaton)  //remove the first parameter
{
    streaming_automaton->json_stream = json_stream;  //remove this
    streaming_automaton->query_automaton = query_automaton;
    jps_SyntaxStackCtor(&streaming_automaton->syntax_stack);
    jps_QueryStackCtor(&streaming_automaton->query_stack);
    jsl_LexerCtor(&streaming_automaton->lexer, json_stream);
    streaming_automaton->output_list = jpo_createOutputList();
    //change the implemenations for the following 4 sentences
    streaming_automaton->current_state.state = 1;   //starting state 
    streaming_automaton->current_state.count = 0;  
    streaming_automaton->current_state.start_obj = -1;
    streaming_automaton->current_state.end_obj = -1;
}

static inline void jsr_StreamingAutomatonDtor(StreamingAutomaton* streaming_automaton)
{
    if(streaming_automaton->json_stream != NULL)  //remove this
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
    jsl_LexerDtor(&streaming_automaton->lexer);
}

void jsr_automaton_execution(StreamingAutomaton* streaming_automaton); //add a new parameter char*
#endif // !__STREAMING_AUTOMATON_H__
