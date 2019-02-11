#include "streaming_automaton.h"

static inline void increase_counter(QueryElement* current_state)
{
    current_state->count++;
   //printf("new state %d count %d\n", current_state->state, current_state->count);
}

static inline void add_primitive_output(JQ_DFA* query_automaton, QueryElement* current_state, char* text_content, OutputList* output_list) 
{
    if(jqd_getAcceptType(query_automaton, current_state->state))  //current_state is a match
    {
        JQ_index_pair pair = jqd_getArrayIndex(query_automaton, current_state->state);
        int lower = pair.lower; 
        int upper = pair.upper;
        if(((lower==0&&upper==0)||(current_state->count>=lower && current_state->count<upper)))  //check array indexes
        {
            jpo_addElement(output_list, text_content);  //printf("output %s\n", text_content);
        }
    }
}

static inline void add_object_output(JQ_DFA* query_automaton, QueryElement* current_state, char* text_content, Lexer* lexer, OutputList* output_list) 
{
    /*if(strcmp(text_content, "]")==0)
    {
        JQ_index_pair pair = jqd_getArrayIndex(query_automaton, current_state->state);
        int lower = pair.lower; 
        int upper = pair.upper;
        printf("output ] %s count %d state %d lower %d upper %d\n", text_content, current_state->count, current_state->state, lower, upper);
    }*/
    //if(jqd_getAcceptType(query_automaton, current_state->state)==JQ_DFA_PREDICATE_TYPE)
       // printf(" checking extra output %s count %d start %d\n", text_content, current_state->count, current_state->start_obj);
    if(jqd_getAcceptType(query_automaton, current_state->state)==JQ_DFA_OUTPUT_TYPE)  //current_state is a match
    {
        JQ_index_pair pair = jqd_getArrayIndex(query_automaton, current_state->state);
        int lower = pair.lower; 
        int upper = pair.upper;
        if(((lower==0&&upper==0)||(current_state->count>=lower && current_state->count<upper)))  //check array indexes
        {   //printf(" checking output %s lower %d upper %d count %d start %d state %d\n", text_content, lower, upper, current_state->count, current_state->start_obj, current_state->state);
            if(strcmp(text_content, "{")==0||strcmp(text_content, "[")==0)
            {
                int position = lexer->next_start - lexer->begin_stream - 1; 
                current_state->start_obj = position;  
                //printf(" checking output' %s lower %d upper %d count %d start %d state %d\n", text_content, lower, upper, current_state->count, current_state->start_obj, current_state->state);
            }
            else if(current_state->start_obj>0&&(strcmp(text_content, "}")==0||strcmp(text_content, "]")==0))
            {   
                int position = lexer->next_start - lexer->begin_stream;
                char* output_text;
                output_text = substring(lexer->begin_stream, current_state->start_obj, position);  //printf("output %s lower %d upper %d count %d state %d\n", output_text, lower, upper, current_state->count, current_state->state);
                current_state->start_obj = -1;
                jpo_addElement(output_list, output_text);
            }
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

static inline void ary_s(JQ_DFA* query_automaton, QueryElement* current_state, int input, SyntaxStack* syn_stack, QueryStack* query_stack)  
{
    jps_syntaxPush(syn_stack, input);
    jps_queryPush(query_stack, *current_state);   //push current state into stack
    QueryElement next_state;                      //move onto next state
    JQ_index_pair pair = jqd_getArrayIndex(query_automaton, current_state->state);
    int lower = pair.lower;
    int upper = pair.upper; 
    if(!((lower==0&&upper==0)||(current_state->count>=lower && current_state->count<upper)))  //check array indexes 
        next_state.state = 0;
    else{ next_state.state = jqd_nextStateByStr(query_automaton, current_state->state, JQ_DFA_ARRAY);} 
    next_state.count = 0;
    next_state.start_obj = -1;
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

static inline void key(JQ_DFA* query_automaton, QueryElement* current_state, int input, char* content, SyntaxStack* syn_stack, QueryStack* query_stack)  
{
    jps_syntaxPush(syn_stack, input);
    QueryElement c_state;
    c_state.state = current_state->state;
    c_state.count = current_state->count;
    if(current_state->start_obj>0) c_state.start_obj = current_state->start_obj;
    else c_state.start_obj = -1;
    jps_queryPush(query_stack, c_state);     //push current state in query stack
    QueryElement next_state;                 //move onto next state
    JQ_index_pair pair = jqd_getArrayIndex(query_automaton, c_state.state);
    int lower = pair.lower; 
    int upper = pair.upper; 
    if(!((lower==0&&upper==0)||(c_state.count>=lower && c_state.count<upper)))  //check array indexes
        next_state.state = 0; 
    else 
        next_state.state = jqd_nextStateByStr(query_automaton, current_state->state, content);  //get next state based on content  
    next_state.count = 0;
    next_state.start_obj = -1;
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
    Lexer* lexer = &streaming_automaton->lexer;   //change this to the following code
    OutputList* output_list = streaming_automaton->output_list;
    QueryElement* current_state = &streaming_automaton->current_state;

    //initialize lexer
    /*Lexer lexer;
    initLexer(&lexer, json_stream);*/

    int symbol = jsl_next_token(lexer);
    
    while(symbol!=END)
    {   
        switch(symbol)
        {   
            case LCB:   //left curly branket
                obj_s(symbol, syntax_stack);  
                add_object_output(query_automaton, current_state, "{", lexer, NULL); 
                break;
            case RCB:   //right curly branket
                //syntax stack only has one '{'
                if(jps_syntaxSize(syntax_stack) == 1)   
                {
                    obj_e(syntax_stack); 
                }
                //syntax stack has at least two elements, the top element is '{'
                else if(jps_syntaxSize(syntax_stack) > 1)  
                {
                     //after we pop out '{', the next element on top of syntax stack is a key field
                     if(jps_syntaxSecondTop(syntax_stack)==KY)	 
                     {
                         val_obj_e(current_state, syntax_stack, query_stack); 
                     }
                     //after we pop out '{', the next element on top of syntax stack is '['
                     else if(jps_syntaxSecondTop(syntax_stack)==LB)  
                     {
                         elt_obj_e(syntax_stack);
                         increase_counter(current_state); 
                     }
                } 
                add_object_output(query_automaton, current_state, "}", lexer, output_list);
                break;
            case LB:   //left square branket  
                add_object_output(query_automaton, current_state, "[", lexer, NULL);
                ary_s(query_automaton, current_state, symbol, syntax_stack, query_stack);
                break;
            case RB:   //right square branket
                //syntax stack only has one '['
                if(jps_syntaxSize(syntax_stack) == 1)  
                {   
                    ary_e(current_state, syntax_stack, query_stack); 
                    add_object_output(query_automaton, current_state, "]", lexer, output_list);
                }
                //syntax stack has at least two elements, the top element is '['
                else if(jps_syntaxSize(syntax_stack) > 1)  
                {   
                    //after we pop out '[', the next element on top of syntax stack is a key field
                    if(jps_syntaxSecondTop(syntax_stack)==KY)  
                    {   
                        QueryElement checked_state = jps_queryTop(query_stack);
                        add_object_output(query_automaton, &checked_state, "]", lexer, output_list);
                        val_ary_e(current_state, syntax_stack, query_stack); 
                    }
                    //after we pop out '[', the next element on top of syntax stack is '['
                    else if(jps_syntaxSecondTop(syntax_stack)==LB)  
                    {  
                        elt_ary_e(current_state, syntax_stack, query_stack); 
                        add_object_output(query_automaton, current_state, "]", lexer, output_list);
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
                    //the top element on syntax stack is a key field
                    if(jps_syntaxTop(syntax_stack)==KY)  
                    {
                        add_primitive_output(query_automaton, current_state, lexer->content, output_list);
                        val_pmt(current_state, syntax_stack, query_stack); 
                    }
                    //the top element on syntax stack is '['
                    else if(jps_syntaxTop(syntax_stack)==LB)  
                    {
                        increase_counter(current_state);
                        add_primitive_output(query_automaton, current_state, lexer->content, output_list); 
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


