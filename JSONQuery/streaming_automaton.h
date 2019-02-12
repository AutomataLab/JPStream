#ifndef __STREAMING_AUTOMATON_H__
#define __STREAMING_AUTOMATON_H__

#include "json_stream.h"
#include "dfa_builder.h"
#include "stack.h"
#include "list.h"
#include "lexing.h"

typedef struct StreamingAutomaton{
    JSONStream* json_stream;
    JSONQueryDFA* query_automaton;
    SyntaxStack syntax_stack;
    QueryStack query_stack;
    Lexer lexer;
    List* output_list;
    QueryStackElement current_state;
}StreamingAutomaton;


static inline void initStreamingAutomaton(StreamingAutomaton* streaming_automaton, JSONStream* json_stream, JSONQueryDFA* query_automaton)
{
    streaming_automaton->json_stream = json_stream;
    streaming_automaton->query_automaton = query_automaton;
    initSyntaxStack(&streaming_automaton->syntax_stack);
    initQueryStack(&streaming_automaton->query_stack);
    initLexer(&streaming_automaton->lexer, json_stream);
    streaming_automaton->output_list = createList();
    streaming_automaton->current_state.state = 1;   //starting state
    streaming_automaton->current_state.count = 0;  
    streaming_automaton->current_state.start_obj = -1;
    streaming_automaton->current_state.end_obj = -1;
}

static inline void destroyStreamingAutomaton(StreamingAutomaton* streaming_automaton)
{
    if(streaming_automaton->json_stream != NULL)
    {
        jps_freeJSONStream(streaming_automaton->json_stream);
    }
    if(streaming_automaton->query_automaton != NULL)
    {
        destoryJSONQueryDFA(streaming_automaton->query_automaton);
    }
    if(streaming_automaton->output_list != NULL)
    {
        freeList(streaming_automaton->output_list);
    }
    destroyLexer(&streaming_automaton->lexer);
}

void executeAutomaton(StreamingAutomaton* streaming_automaton);
#endif // !__STREAMING_AUTOMATON_H__
