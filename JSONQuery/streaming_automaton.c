#include "streaming_automaton.h"

static inline void increase_counter(QueryStackElement* current_state)
{
    current_state->count++;
}

// verify whether the current state and current counter satisfy the matched condition
// 1 -- satisfy 0 -- not satisfy
static int is_a_match(JSONQueryDFA* query_automaton, QueryStackElement* current_state)
{
    int match = 0;
    //it is a matched output
    if(getDFAAcceptType(query_automaton, current_state->state)==JSONQueryDFA_OUTPUT_TYPE)  
    {
        JSONQueryIndexPair pair = getDFAArrayIndex(query_automaton, current_state->state);
        int lower = pair.lower; 
        int upper = pair.upper;
        //check array indexes
        if(((lower==0&&upper==0)||(current_state->count>=lower && current_state->count<upper)))  
        {
            match = 1;
        }
    }
    return match;
}

// implementation of state transition rules based on based on input symbol, query state, top elements on syntax stack and query stack
static inline void obj_s(int symbol, SyntaxStack* syn_stack)
{
    syntaxStackPush(syn_stack, symbol);
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

static inline void ary_s(JSONQueryDFA* query_automaton, QueryStackElement* current_state, int symbol, SyntaxStack* syn_stack, QueryStack* query_stack)  
{
    syntaxStackPush(syn_stack, symbol);
    //push current state into query stack
    queryStackPush(query_stack, *current_state);   
    //move onto next state
    QueryStackElement next_state;                      
    JSONQueryIndexPair pair = getDFAArrayIndex(query_automaton, current_state->state);
    int lower = pair.lower;
    int upper = pair.upper; 
    int counter = current_state->count;
    //check array indexes 
    if(!((lower==0&&upper==0)||(counter>=lower && counter<upper)))  
        next_state.state = 0;
    else{ next_state.state = dfaNextStateByStr(query_automaton, current_state->state, JSONQueryDFA_ARRAY);} 
    next_state.count = 0;
    next_state.matched_start = -1;
    //update current state
    *current_state  = next_state; 
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

static inline void key(JSONQueryDFA* query_automaton, QueryStackElement* current_state, int symbol, char* symbol_content, SyntaxStack* syn_stack, QueryStack* query_stack)  
{
    syntaxStackPush(syn_stack, symbol);
    //push current state into query stack
    queryStackPush(query_stack, *current_state);
    //move onto next state    
    QueryStackElement next_state;                 
    JSONQueryIndexPair pair = getDFAArrayIndex(query_automaton, current_state->state);
    int lower = pair.lower; 
    int upper = pair.upper; 
    int counter = current_state->count;
    //check array indexes
    if(!((lower==0&&upper==0)||(counter>=lower && counter<upper)))  
        next_state.state = 0; 
    else //get next state based on key field
        next_state.state = dfaNextStateByStr(query_automaton, current_state->state, symbol_content);  
    next_state.count = 0;
    next_state.matched_start = -1;
    //update current state
    *current_state  = next_state;
}

static inline void val_pmt(QueryStackElement* current_state, SyntaxStack* syn_stack, QueryStack* query_stack)
{
    *current_state = queryStackPop(query_stack);
    syntaxStackPop(syn_stack);
}

// the execution of streaming automaton
void executeAutomaton(StreamingAutomaton* streaming_automaton, char* json_stream)
{
    JSONQueryDFA* query_automaton = streaming_automaton->query_automaton; //query automaton
    SyntaxStack* syntax_stack = &streaming_automaton->syntax_stack;   //syntax stack
    QueryStack* query_stack = &streaming_automaton->query_stack;  //query stack
    TupleList* tuple_list = streaming_automaton->tuple_list;  //tuple list
    QueryStackElement* current_state = &streaming_automaton->current_state;

    //initialize lexer
    Lexer lexer;
    initLexer(&lexer, json_stream);

    int symbol = nextToken(&lexer);
    
    //select transition rules based on input symbol, query state, top elements on syntax stack and query stack
    while(symbol!=END)
    {   
        switch(symbol)
        {   
            case LCB:   //left curly branket
                obj_s(symbol, syntax_stack);  
                //it is a matched output
                if(is_a_match(query_automaton, current_state)==1)  
                { 
                    current_state->matched_start = lexer.next_start - lexer.begin_stream - 1; 
                }
                break;
            case RCB:   //right curly branket
                //syntax stack only has one '{'
                if(syntaxStackSize(syntax_stack) == 1)   
                {
                    obj_e(syntax_stack); 
                }
                //syntax stack has at least two elements, the top element is '{'
                else if(syntaxStackSize(syntax_stack) > 1)  
                {
                     //after we pop out '{', the next element on top of syntax stack is a key field
                     if(syntaxStackSecondTop(syntax_stack)==KY)	 
                     {
                         val_obj_e(current_state, syntax_stack, query_stack); 
                     }
                     //after we pop out '{', the next element on top of syntax stack is '['
                     else if(syntaxStackSecondTop(syntax_stack)==LB)  
                     {
                         elt_obj_e(syntax_stack);
                         increase_counter(current_state); 
                     }
                } 
                //it is a matched output
                if(current_state->matched_start>-1&&is_a_match(query_automaton, current_state)==1)  
                { 
                    int position = lexer.next_start - lexer.begin_stream;
                    char* output_text = substring(lexer.begin_stream, current_state->matched_start, position); 
                    current_state->matched_start = -1;
                    addTupleListElement(tuple_list, current_state->state, output_text); 
                }
                break;
            case LB:   //left square branket 
                //it is a matched output
                if(is_a_match(query_automaton, current_state)==1)  
                { 
                    current_state->matched_start = lexer.next_start - lexer.begin_stream - 1;  
                }
                ary_s(query_automaton, current_state, symbol, syntax_stack, query_stack);
                break;
            case RB:   //right square branket
                if(syntaxStackSize(syntax_stack) >= 1)  
                {
                    QueryStackElement checked_state = queryStackTop(query_stack);
                    //it is a matched output
                    if(is_a_match(query_automaton, &checked_state)==1)  
                    { 
                        int position = lexer.next_start - lexer.begin_stream;
                        char* output_text = substring(lexer.begin_stream, checked_state.matched_start, position); 
                        checked_state.matched_start = -1;
                        addTupleListElement(tuple_list, checked_state.state, output_text);  
                    }
                    //syntax stack only has one '['
                    if(syntaxStackSize(syntax_stack) == 1)  
                    {   
                        ary_e(current_state, syntax_stack, query_stack); 
                    }
                    //syntax stack has at least two elements, the top element is '['
                    else if(syntaxStackSize(syntax_stack) > 1)  
                    {   
                        //after we pop out '[', the next element on top of syntax stack is a key field
                        if(syntaxStackSecondTop(syntax_stack)==KY)  
                        {   
                            val_ary_e(current_state, syntax_stack, query_stack); 
                        }
                        //after we pop out '[', the next element on top of syntax stack is '['
                        else if(syntaxStackSecondTop(syntax_stack)==LB)  
                        {  
                            elt_ary_e(current_state, syntax_stack, query_stack); 
                            increase_counter(current_state);
                        }
                    }
                }
                break;
            case COM:   //comma
                break;
            case KY:    //key field
                key(query_automaton, current_state, symbol, lexer.content, syntax_stack, query_stack); 
                break;
            case PRI:   //primitive
                if(syntaxStackSize(syntax_stack) >= 1)
                {
                    //it is a matched output
                    if(is_a_match(query_automaton, current_state)==1)  
                    { 
                        addTupleListElement(tuple_list, current_state->state, lexer.content);  
                    }
                    //the top element on syntax stack is a key field
                    if(syntaxStackTop(syntax_stack)==KY)  
                    {
                        val_pmt(current_state, syntax_stack, query_stack); 
                    }
                    //the top element on syntax stack is '['
                    else if(syntaxStackTop(syntax_stack)==LB)  
                    {
                        increase_counter(current_state);
                    }
                }
                break;
        }
        symbol = nextToken(&lexer); 
    }
    destroyLexer(&lexer);
    printf("syntax size %d query state %d %d\n", syntaxStackSize(syntax_stack), current_state->state, query_stack->top_item);
    printf("output size %d\n", getTupleListSize(tuple_list));
}


