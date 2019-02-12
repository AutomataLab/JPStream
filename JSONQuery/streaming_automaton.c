#include "streaming_automaton.h"

static inline void increase_counter(QueryStackElement* current_state)
{
    current_state->count++;
}

static inline void add_primitive_output(JSONQueryDFA* query_automaton, QueryStackElement* current_state, char* text_content, List* output_list) 
{
    if(getDFAAcceptType(query_automaton, current_state->state))  //current_state is a match
    {
        JSONQueryIndexPair pair = getDFAArrayIndex(query_automaton, current_state->state);
        int lower = pair.lower; 
        int upper = pair.upper;
        if(((lower==0&&upper==0)||(current_state->count>=lower && current_state->count<upper)))  //check array indexes
        {
            addListElement(output_list, text_content);
        }
    }
}

static inline void add_object_output(JSONQueryDFA* query_automaton, QueryStackElement* current_state, char* text_content, Lexer* lexer, List* output_list) 
{
    if(getDFAAcceptType(query_automaton, current_state->state)==JSONQueryDFA_OUTPUT_TYPE)  //current_state is a match
    {
        JSONQueryIndexPair pair = getDFAArrayIndex(query_automaton, current_state->state);
        int lower = pair.lower; 
        int upper = pair.upper;
        if(((lower==0&&upper==0)||(current_state->count>=lower && current_state->count<upper)))  //check array indexes
        {
            if(strcmp(text_content, "{")==0||strcmp(text_content, "[")==0)
            {
                int position = lexer->next_start - lexer->begin_stream - 1; 
                current_state->start_obj = position;  
            }
            else if(current_state->start_obj>0&&(strcmp(text_content, "}")==0||strcmp(text_content, "]")==0))
            {   
                int position = lexer->next_start - lexer->begin_stream;
                char* output_text;
                output_text = substring(lexer->begin_stream, current_state->start_obj, position);  
                current_state->start_obj = -1;
                addListElement(output_list, output_text);
            }
        }
    }
}

//different state transition rules based on input symbol and syntax stack
static inline void obj_s(int input, SyntaxStack* syn_stack)
{
    syntaxStackPush(syn_stack, input);
}

static inline void obj_e(SyntaxStack* syn_stack)
{
    syntaxStackPop(syn_stack);
}

static inline void val_obj_e(QueryStackElement* current_state, SyntaxStack* syn_stack, QueryStack* query_stack)
{
    *current_state = queryStackPop(query_stack);
    syntaxStackPop(syn_stack);
    syntaxStackPop(syn_stack);
}

static inline void elt_obj_e(SyntaxStack* syn_stack)
{
    syntaxStackPop(syn_stack);
}

static inline void ary_s(JSONQueryDFA* query_automaton, QueryStackElement* current_state, int input, SyntaxStack* syn_stack, QueryStack* query_stack)  
{
    syntaxStackPush(syn_stack, input);
    queryStackPush(query_stack, *current_state);   //push current state into stack
    QueryStackElement next_state;                      //move onto next state
    JSONQueryIndexPair pair = getDFAArrayIndex(query_automaton, current_state->state);
    int lower = pair.lower;
    int upper = pair.upper; 
    if(!((lower==0&&upper==0)||(current_state->count>=lower && current_state->count<upper)))  //check array indexes 
        next_state.state = 0;
    else{ next_state.state = dfaNextStateByStr(query_automaton, current_state->state, JSONQueryDFA_ARRAY);} 
    next_state.count = 0;
    next_state.start_obj = -1;
    *current_state  = next_state; //update current state
}

static inline void ary_e(QueryStackElement* current_state, SyntaxStack* syn_stack, QueryStack* query_stack)
{
    *current_state = queryStackPop(query_stack);
    syntaxStackPop(syn_stack);
}

static inline void val_ary_e(QueryStackElement* current_state, SyntaxStack* syn_stack, QueryStack* query_stack)
{
    queryStackPop(query_stack);
    *current_state = queryStackPop(query_stack); 
    syntaxStackPop(syn_stack); 
    syntaxStackPop(syn_stack); 
}

static inline void elt_ary_e(QueryStackElement* current_state, SyntaxStack* syn_stack, QueryStack* query_stack)
{
    *current_state = queryStackPop(query_stack);
    syntaxStackPop(syn_stack); 
}

static inline void key(JSONQueryDFA* query_automaton, QueryStackElement* current_state, int input, char* content, SyntaxStack* syn_stack, QueryStack* query_stack)  
{
    syntaxStackPush(syn_stack, input);
    QueryStackElement c_state;
    c_state.state = current_state->state;
    c_state.count = current_state->count;
    if(current_state->start_obj>0) c_state.start_obj = current_state->start_obj;
    else c_state.start_obj = -1;
    queryStackPush(query_stack, c_state);     //push current state in query stack
    QueryStackElement next_state;                 //move onto next state
    JSONQueryIndexPair pair = getDFAArrayIndex(query_automaton, c_state.state);
    int lower = pair.lower; 
    int upper = pair.upper; 
    if(!((lower==0&&upper==0)||(c_state.count>=lower && c_state.count<upper)))  //check array indexes
        next_state.state = 0; 
    else 
        next_state.state = dfaNextStateByStr(query_automaton, current_state->state, content);  //get next state based on content  
    next_state.count = 0;
    next_state.start_obj = -1;
    *current_state  = next_state; //update current state
}

static inline void val_pmt(QueryStackElement* current_state, SyntaxStack* syn_stack, QueryStack* query_stack)
{
    *current_state = queryStackPop(query_stack);
    syntaxStackPop(syn_stack);
}

//state transition rules for streaming automaton
void jsr_state_transition(StreamingAutomaton* streaming_automaton)
{
    JSONQueryDFA* query_automaton = streaming_automaton->query_automaton;
    SyntaxStack* syntax_stack = &streaming_automaton->syntax_stack;
    QueryStack* query_stack = &streaming_automaton->query_stack;
    Lexer* lexer = &streaming_automaton->lexer;
    List* output_list = streaming_automaton->output_list;
    QueryStackElement* current_state = &streaming_automaton->current_state;

    int symbol = nextToken(lexer);
    
    while(symbol!=END)
    {   
        switch(symbol)
        {   
            case LCB:   //left curly branket
                obj_s(symbol, syntax_stack);  
                add_object_output(query_automaton, current_state, "{", lexer, NULL); 
                break;
            case RCB:   //right curly branket
                if(syntaxStackSize(syntax_stack) == 1)   //syntax stack only has one '{'
                {
                    obj_e(syntax_stack); 
                }
                else if(syntaxStackSize(syntax_stack) > 1)  //syntax stack has at least two elements, the top element is '{'
                {
                     if(syntaxStackSecondTop(syntax_stack)==KY)	 //after we pop out '{', the next element on top of syntax stack is a key field
                     {
                         val_obj_e(current_state, syntax_stack, query_stack); 
                     }
                     else if(syntaxStackSecondTop(syntax_stack)==LB)  //after we pop out '{', the next element on top of syntax stack is '['
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
                if(syntaxStackSize(syntax_stack) == 1)  //syntax stack only has one '['
                {   
                    ary_e(current_state, syntax_stack, query_stack); 
                    add_object_output(query_automaton, current_state, "]", lexer, output_list);
                }
                else if(syntaxStackSize(syntax_stack) > 1)  //syntax stack has at least two elements, the top element is '['
                {   
                    if(syntaxStackSecondTop(syntax_stack)==KY)  //after we pop out '[', the next element on top of syntax stack is a key field
                    {   
                        QueryStackElement checked_state = queryStackTop(query_stack);
                        add_object_output(query_automaton, &checked_state, "]", lexer, output_list);
                        val_ary_e(current_state, syntax_stack, query_stack); 
                    }
                    else if(syntaxStackSecondTop(syntax_stack)==LB)  //after we pop out '[', the next element on top of syntax stack is '['
                    {  
                        elt_ary_e(current_state, syntax_stack, query_stack); 
                        increase_counter(current_state);
                        add_object_output(query_automaton, current_state, "]", lexer, output_list);
                    }
                }
                
                break;
            case COM:   //comma
                break;
            case KY:    //key field
                key(query_automaton, current_state, symbol, lexer->content, syntax_stack, query_stack); 
                break;
            case PRI:   //primitive
                if(syntaxStackSize(syntax_stack) >= 1)
                {
                    if(syntaxStackTop(syntax_stack)==KY)  //the top element on syntax stack is a key field
                    {
                        add_primitive_output(query_automaton, current_state, lexer->content, output_list);
                        val_pmt(current_state, syntax_stack, query_stack); 
                    }
                    else if(syntaxStackTop(syntax_stack)==LB)  //the top element on syntax stack is '['
                    {
                        increase_counter(current_state);
                        add_primitive_output(query_automaton, current_state, lexer->content, output_list); 
                    }
                }
                break;
        }
        symbol = nextToken(lexer); 
    }
    printf("syntax size %d query state %d %d\n", syntaxStackSize(syntax_stack), current_state->state, query_stack->count);
    printf("output size %d\n", getListSize(output_list));
}

void executeAutomaton(StreamingAutomaton* streaming_automaton)
{
    jsr_state_transition(streaming_automaton);
}


