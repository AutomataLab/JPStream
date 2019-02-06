#include "streaming_automaton.h"

static inline void increase_counter(QueryElement* current_state)
{
    current_state->count++;
}

static inline void add_output(JQ_DFA* query_automaton, QueryElement* current_state, char* text_content, OutputList* output_list) //needs adjustment
{
    if(jqd_getAcceptType(query_automaton, current_state->state))  //current_state is a match
    {
        JQ_index_pair pair = jqd_getArrayIndex(query_automaton, current_state->state);
        int lower = pair.lower; 
        int upper = pair.upper;
        if(((lower==0&&upper==0)||(current_state->count>=lower && current_state->count<upper)))  //check array indexes
        {
            jpo_addElement(output_list, text_content);
        }
    }
}

//different state transition rules based on input symbol and syntax stack
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
    JQ_index_pair pair = jqd_getArrayIndex(query_automaton, current_state->state);
    int lower = pair.lower;
    int upper = pair.upper; 
    if(!((lower==0&&upper==0)||(current_state->count>=lower && current_state->count<upper)))  //check array indexes 
        next_state.state = 0;
    else  next_state.state = jqd_nextStateByStr(query_automaton, current_state->state, JQ_DFA_ARRAY);  //get next state
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
    *current_state = jps_queryPop(query_stack); 
    jps_syntaxPop(syn_stack); 
    jps_syntaxPop(syn_stack); 
}

static inline void elt_ary_e(QueryElement* current_state, SyntaxStack* syn_stack, QueryStack* query_stack)
{
    *current_state = jps_queryPop(query_stack);
    jps_syntaxPop(syn_stack); 
}

static inline void key(JQ_DFA* query_automaton, QueryElement* current_state, int input, char* content, SyntaxStack* syn_stack, QueryStack* query_stack)  //needs adjustment
{
    jps_syntaxPush(syn_stack, input);
    QueryElement c_state;
    c_state.state = current_state->state;
    c_state.count = current_state->count;
    jps_queryPush(query_stack, c_state);   
    QueryElement next_state;
    JQ_index_pair pair = jqd_getArrayIndex(query_automaton, c_state.state);
    int lower = pair.lower; 
    int upper = pair.upper; 
    if(!((lower==0&&upper==0)||(c_state.count>=lower && c_state.count<upper)))  //check array indexes
        next_state.state = 0; 
    else 
        next_state.state = jqd_nextStateByStr(query_automaton, current_state->state, content);  //get next state based on content  
    next_state.count = 0;
    *current_state  = next_state; //update current state
}

static inline void val_pmt(QueryElement* current_state, SyntaxStack* syn_stack, QueryStack* query_stack)
{
    *current_state = jps_queryPop(query_stack);
    jps_syntaxPop(syn_stack);
}

//state transition rules for streaming automaton
void jsr_state_transition(StreamingAutomaton* streaming_automaton)
{
    JQ_DFA* query_automaton = streaming_automaton->query_automaton;
    SyntaxStack* syntax_stack = &streaming_automaton->syntax_stack;
    QueryStack* query_stack = &streaming_automaton->query_stack;
    Lexer* lexer = &streaming_automaton->lexer;
    OutputList* output_list = streaming_automaton->output_list;
    QueryElement* current_state = &streaming_automaton->current_state;

    int symbol = jsl_next_token(lexer);
    
    while(symbol!=-1)
    {   
        switch(symbol)
        {   
            case LCB:   //left curly branket
                obj_s(symbol, syntax_stack);  
                break;
            case RCB:   //right curly branket
                if(jps_syntaxSize(syntax_stack) == 1)   //syntax stack only has one '{'
                {
                    obj_e(syntax_stack); 
                }
                else if(jps_syntaxSize(syntax_stack) > 1)  //syntax stack has at least two elements, the top element is '{'
                {
                     if(jps_syntaxSecondTop(syntax_stack)==KY)	 //after we pop out '{', the next element on top of syntax stack is a key field
                     {
                         val_obj_e(current_state, syntax_stack, query_stack); 
                     }
                     else if(jps_syntaxSecondTop(syntax_stack)==LB)  //after we pop out '{', the next element on top of syntax stack is '['
                     {
                         elt_obj_e(syntax_stack);
                         increase_counter(current_state); 
                     }
                }
                break;
            case LB:   //left square branket 
                ary_s(query_automaton, current_state, symbol, syntax_stack, query_stack);
                break;
            case RB:   //right square branket
                if(jps_syntaxSize(syntax_stack) == 1)  //syntax stack only has one '['
                {   
                    ary_e(current_state, syntax_stack, query_stack); 
                }
                else if(jps_syntaxSize(syntax_stack) > 1)  //syntax stack has at least two elements, the top element is '['
                {   
                    if(jps_syntaxSecondTop(syntax_stack)==KY)  //after we pop out '[', the next element on top of syntax stack is a key field
                    {   
                        val_ary_e(current_state, syntax_stack, query_stack); 
                    }
                    else if(jps_syntaxSecondTop(syntax_stack)==LB)  //after we pop out '[', the next element on top of syntax stack is '['
                    {  
                        elt_ary_e(current_state, syntax_stack, query_stack); 
                        increase_counter(current_state);
                    }
                }
                break;
            case COM:   //comma
                break;
            case KY:    //key field
                key(query_automaton, current_state, symbol, lexer->content, syntax_stack, query_stack);
                break;
            case PRI:   //primitive
                if(jps_syntaxSize(syntax_stack) >= 1)
                {
                    if(jps_syntaxTop(syntax_stack)==KY)  //the top element on syntax stack is a key field
                    {
                        add_output(query_automaton, current_state, lexer->content, output_list);
                        val_pmt(current_state, syntax_stack, query_stack); 
                    }
                    else if(jps_syntaxTop(syntax_stack)==LB)  //the top element on syntax stack is '['
                    {
                        increase_counter(current_state);
                        add_output(query_automaton, current_state, lexer->content, output_list); 
                    }
                }
                break;
        }
        symbol = jsl_next_token(lexer); 
    }
    printf("syntax size %d query state %d %d\n", jps_syntaxSize(syntax_stack), current_state->state, query_stack->count);
    printf("output size %d\n", jpo_getSize(output_list));
}

void jsr_automaton_execution(StreamingAutomaton* streaming_automaton)
{
    jsr_state_transition(streaming_automaton);
}


