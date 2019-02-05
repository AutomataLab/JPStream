#include "streaming_automaton.h"
#include "lexer.h"
//#include "automaton.h"

#define PUSH 1
#define POP 2

//initialize tokens
static inline void token_info_initialization(StreamingAutomaton* streaming_automaton, xml_Text *pText, xml_Token *pToken, token_info* tInfo)
{
    tInfo->start = pToken->text.p + pToken->text.len;
    tInfo->head = streaming_automaton->json_stream->input_stream[0];
    tInfo->current = tInfo->start;
    tInfo->end = pText->p + pText->len;
    tInfo->lex_state = (int*)malloc(sizeof(int));
    *(tInfo->lex_state)= 0;
}

static inline void increase_counter(QueryElement* current_state)
{
    current_state->count++;
}

//state transition rules
static inline void obj_s(int input, SyntaxStack* syn_stack)
{
    jps_syntaxPush(syn_stack, input);
}

static inline void obj_e(SyntaxStack* syn_stack)
{
    jps_syntaxPop(syn_stack);
}

static inline void val_obj_e(QueryElement* current_state, SyntaxStack* syn_stack, QueryStack* query_stack)
{
    *current_state = jps_queryPop(query_stack);
    jps_syntaxPop(syn_stack);
    jps_syntaxPop(syn_stack);
}

static inline void elt_obj_e(SyntaxStack* syn_stack)
{
    jps_syntaxPop(syn_stack);
}

static inline void ary_s(JQ_DFA* query_automaton, QueryElement* current_state, int input, SyntaxStack* syn_stack, QueryStack* query_stack)  //needs adjustment
{
    jps_syntaxPush(syn_stack, input);
    jps_queryPush(query_stack, *current_state);
    QueryElement next_state;
    next_state.state = jqd_nextStateByStr(query_automaton, current_state->state, JQ_DFA_ARRAY);  //get next state
    next_state.count = 0;
    *current_state  = next_state; //update current state
}

static inline void ary_e(QueryElement* current_state, SyntaxStack* syn_stack, QueryStack* query_stack)
{
    *current_state = jps_queryPop(query_stack);
    jps_syntaxPop(syn_stack);
}

static inline void val_ary_e(QueryElement* current_state, SyntaxStack* syn_stack, QueryStack* query_stack)
{
    jps_queryPop(query_stack);
    *current_state = jps_queryPop(query_stack); //printf("ssize %d\n", syn_stack->count);
    jps_syntaxPop(syn_stack); 
    jps_syntaxPop(syn_stack); //printf("size %d\n", syn_stack->count);
}

static inline void elt_ary_e(QueryElement* current_state, SyntaxStack* syn_stack, QueryStack* query_stack)
{
    *current_state = jps_queryPop(query_stack);//printf("ssize %d\n", syn_stack->count);
    jps_syntaxPop(syn_stack); //printf("size %d\n", syn_stack->count);
}

static inline void key(JQ_DFA* query_automaton, QueryElement* current_state, int input, char* content, SyntaxStack* syn_stack, QueryStack* query_stack)  //needs adjustment
{
    jps_syntaxPush(syn_stack, input);
    QueryElement c_state;
    c_state.state = current_state->state;
    c_state.count = current_state->count;
    jps_queryPush(query_stack, c_state);   //printf("query stack size %d %d\n", query_stack->count, current_state->state); printf("content %s %d\n", content, c_state.state); //printf("num of states %d\n", jpa_getStatesNum(query_automaton));
    QueryElement next_state;
    next_state.state = jqd_nextStateByStr(query_automaton, current_state->state, content);  //get next state based on content  
    //printf("1\n");
    //printf("%d next state %d %s\n", c_state.state, next_state.state, content);
    next_state.count = 0;
    *current_state  = next_state; //update current state
}

static inline void val_pmt(QueryElement* current_state, SyntaxStack* syn_stack, QueryStack* query_stack)
{
    *current_state = jps_queryPop(query_stack);
    jps_syntaxPop(syn_stack);
}

static inline void add_output(JQ_DFA* query_automaton, QueryElement* current_state, char* text_content, OutputList* output_list) //needs adjustment
{
    if(jqd_getAcceptType(query_automaton, current_state->state))  //current_state is a match
    {
        jpo_addElement(output_list, text_content);
    }
}

void jsr_state_transition(StreamingAutomaton* streaming_automaton, xml_Text *pText, xml_Token *pToken)
{
    token_info tInfo;
    token_info_initialization(streaming_automaton, pText, pToken, &tInfo);
    char text_content[MAX_TEXT];
    int symbol = lexer(pText, pToken, &tInfo, text_content);

    //needs adjustment, temporarily create an inner dfa
    //printf("create automaton\n");
    //////Automaton* automaton =  jpa_loadDfaTable(streaming_automaton->query_automaton);
    //printf("end\n");
    //printf("1 num of states %d %d\n", jpa_getStatesNum(automaton), symbol);
    //printf("begin %d\n", symbol);
    int counter = 0;
    while(symbol!=-1)
    {   //printf("symbol %d size %d top %d %s\n", symbol, jps_syntaxSize(&streaming_automaton->syntax_stack), jps_syntaxTop(&streaming_automaton->syntax_stack), text_content);
        switch(symbol)
        {   
            case LCB: 
                counter++;
                obj_s(symbol, &streaming_automaton->syntax_stack);  
                break;
            case RCB: 
                if(jps_syntaxSize(&streaming_automaton->syntax_stack) == 1)
                {
                    obj_e(&streaming_automaton->syntax_stack); counter--;
                }
                else if(jps_syntaxSize(&streaming_automaton->syntax_stack) > 1)
                {
                     if(jps_syntaxSecondTop(&streaming_automaton->syntax_stack)==KY)		
                     {
                         val_obj_e(&streaming_automaton->current_state, &streaming_automaton->syntax_stack, &streaming_automaton->query_stack); counter-=2;
                     }
                     else if(jps_syntaxSecondTop(&streaming_automaton->syntax_stack)==LB)
                     {
                         elt_obj_e(&streaming_automaton->syntax_stack);
                         increase_counter(&streaming_automaton->current_state); counter--;
                     }
                }
                break;
            case LB: counter++;
                ary_s(streaming_automaton->query_automaton, &streaming_automaton->current_state, symbol, &streaming_automaton->syntax_stack, &streaming_automaton->query_stack);
                break;
            case RB: 
                if(jps_syntaxSize(&streaming_automaton->syntax_stack) == 1)
                {   
                    ary_e(&streaming_automaton->current_state, &streaming_automaton->syntax_stack, &streaming_automaton->query_stack); counter--;
                }
                else if(jps_syntaxSize(&streaming_automaton->syntax_stack) > 1)
                {   //printf("select %d\n", jps_syntaxTop(&streaming_automaton->syntax_stack));
                    if(jps_syntaxSecondTop(&streaming_automaton->syntax_stack)==KY)
                    {   //printf("KEY\n");
                        val_ary_e(&streaming_automaton->current_state, &streaming_automaton->syntax_stack, &streaming_automaton->query_stack); counter-=2;
                    }
                    else if(jps_syntaxSecondTop(&streaming_automaton->syntax_stack)==LB)
                    {   //printf("LB\n");
                        elt_ary_e(&streaming_automaton->current_state, &streaming_automaton->syntax_stack, &streaming_automaton->query_stack); counter--;
                        increase_counter(&streaming_automaton->current_state);
                    }
                }
                break;
            case COM:
                break;
            case KY: counter++;
                key(streaming_automaton->query_automaton, &streaming_automaton->current_state, symbol, text_content, &streaming_automaton->syntax_stack, &streaming_automaton->query_stack);
                break;
            case PRI: 
                //printf(" valpmt %d %d %d\n", streaming_automaton->syntax_stack.count, jps_syntaxSize(&streaming_automaton->syntax_stack), jps_syntaxTop(&streaming_automaton->syntax_stack));
                if(jps_syntaxSize(&streaming_automaton->syntax_stack) >= 1)
                {
                    if(jps_syntaxTop(&streaming_automaton->syntax_stack)==KY)
                    {
                        add_output(streaming_automaton->query_automaton, &streaming_automaton->current_state, text_content, streaming_automaton->output_list); //printf("start valpmt\n");
                        val_pmt(&streaming_automaton->current_state, &streaming_automaton->syntax_stack, &streaming_automaton->query_stack); counter--;
                    }
                    else if(jps_syntaxTop(&streaming_automaton->syntax_stack)==LB)
                    {
                        increase_counter(&streaming_automaton->current_state);
                        add_output(streaming_automaton->query_automaton, &streaming_automaton->current_state, text_content, streaming_automaton->output_list); 
                    }
                }
                break;
        }
        symbol = lexer(pText, pToken, &tInfo, text_content);
    }
    printf("syntax size %d %d query state %d %d\n", jps_syntaxSize(&streaming_automaton->syntax_stack), counter, streaming_automaton->current_state.state, streaming_automaton->query_stack.count);
    printf("output size %d\n", jpo_getSize(streaming_automaton->output_list));
}

void jsr_automaton_execution(StreamingAutomaton* streaming_automaton)
{
    xml_Text xml;
    xml_Token token;
    xml_initText(&xml,streaming_automaton->json_stream->input_stream[0]);
    xml_initToken(&token, &xml);
    //printf("start\n");
    jsr_state_transition(streaming_automaton, &xml, &token); 
}


