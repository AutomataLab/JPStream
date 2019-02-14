#include "streaming_automaton.h"

static inline void increase_array_counter(QueryStackElement* si)
{
    si->count++;
}

// verify whether the current state and current counter satisfy the matched condition
// 1 -- satisfy 0 -- not satisfy
static int is_matched_state(JSONQueryDFA* qa, QueryStackElement* si)
{
    int match = 0;
    //it is a matched output
    if(getDFAAcceptType(qa, si->query_state)==DFA_OUTPUT_CANDIDATE)  
    {
        JSONQueryIndexPair pair = getDFAArrayIndex(qa, si->query_state);
        int lower = pair.lower; 
        int upper = pair.upper;
        int counter = si->count;
        //check array indexes
        if(((lower==0&&upper==0)||(counter>=lower && counter<upper)))  
        {
            match = 1;
        }
    }
    return match;
}

/* 
   implementation of state transition rules based on based on input symbol, query state, top elements on syntax stack and query stack

   in the following 10 functions:
   ss -- syntax stack qs -- query stack;
   si -- current state infomation  qa -- query automaton
*/
static inline void obj_s(int symbol, SyntaxStack* ss)
{
    syntaxStackPush(ss, symbol);
}

static inline void obj_e(SyntaxStack* ss)
{
    syntaxStackPop(ss);
}

static inline void val_obj_e(QueryStackElement* si, SyntaxStack* ss, QueryStack* qs)
{
    *si = queryStackPop(qs);
    syntaxStackPop(ss);
    syntaxStackPop(ss);
}

static inline void elt_obj_e(SyntaxStack* ss)
{
    syntaxStackPop(ss);
}

static inline void ary_s(JSONQueryDFA* qa, QueryStackElement* si, int symbol, SyntaxStack* ss, QueryStack* qs)  
{
    syntaxStackPush(ss, symbol);
    //push current state into query stack
    queryStackPush(qs, *si);   
    //move onto next state
    QueryStackElement next_si;                      
    JSONQueryIndexPair pair = getDFAArrayIndex(qa, si->query_state);
    int lower = pair.lower;
    int upper = pair.upper; 
    int counter = si->count;
    //check array indexes 
    if(!((lower==0&&upper==0)||(counter>=lower && counter<upper)))  
        next_si.query_state = 0;
    else{ next_si.query_state = dfaNextStateByStr(qa, si->query_state, DFA_ARRAY);} 
    next_si.count = 0;
    next_si.matched_start = INVALID;
    //update current state info
    *si  = next_si; 
}

static inline void ary_e(QueryStackElement* si, SyntaxStack* ss, QueryStack* qs)
{
    *si = queryStackPop(qs);
    syntaxStackPop(ss);
}

static inline void val_ary_e(QueryStackElement* si, SyntaxStack* ss, QueryStack* qs)
{
    queryStackPop(qs);
    *si = queryStackPop(qs); 
    syntaxStackPop(ss); 
    syntaxStackPop(ss); 
}

static inline void elt_ary_e(QueryStackElement* si, SyntaxStack* ss, QueryStack* qs)
{
    *si = queryStackPop(qs);
    syntaxStackPop(ss); 
}

static inline void key(JSONQueryDFA* qa, QueryStackElement* si, int symbol, char* symbol_content, SyntaxStack* ss, QueryStack* qs)  
{
    syntaxStackPush(ss, symbol);
    //push current state into query stack
    queryStackPush(qs, *si);
    //move onto next state    
    QueryStackElement next_si;                 
    JSONQueryIndexPair pair = getDFAArrayIndex(qa, si->query_state);
    int lower = pair.lower; 
    int upper = pair.upper; 
    int counter = si->count;
    //check array indexes
    if(!((lower==0&&upper==0)||(counter>=lower && counter<upper)))  
        next_si.query_state = 0; 
    else //get next state based on key field
        next_si.query_state = dfaNextStateByStr(qa, si->query_state, symbol_content);  
    next_si.count = 0;
    next_si.matched_start = INVALID;
    //update current state info
    *si  = next_si;
}

static inline void val_pmt(QueryStackElement* si, SyntaxStack* ss, QueryStack* qs)
{
    *si = queryStackPop(qs);
    syntaxStackPop(ss);
}

// the execution of streaming automaton
void executeAutomaton(StreamingAutomaton* sa, char* json_stream)
{
    JSONQueryDFA* qa = sa->query_automaton;   //query automaton
    SyntaxStack* ss = &sa->syntax_stack;      //syntax stack
    QueryStack* qs = &sa->query_stack;        //query stack
    TupleList* tl = sa->tuple_list;           //tuple list
    QueryStackElement* si = &sa->state_info;  //current state info

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
                obj_s(symbol, ss);  
                //it is a matched output
                if(is_matched_state(qa, si)==1)  
                { 
                    si->matched_start = lexer.next_start - lexer.begin_stream - 1; 
                }
                break;
            case RCB:   //right curly branket
                //syntax stack only has one '{'
                if(syntaxStackSize(ss) == 1)   
                {
                    obj_e(ss); 
                }
                //syntax stack has at least two elements, the top element is '{'
                else if(syntaxStackSize(ss) > 1)  
                {
                     //after popping out '{', the next element on top of syntax stack is a key field
                     if(syntaxStackSecondTop(ss)==KY)	 
                     {
                         val_obj_e(si, ss, qs); 
                     }
                     //after popping out '{', the next element on top of syntax stack is '['
                     else if(syntaxStackSecondTop(ss)==LB)  
                     {
                         elt_obj_e(ss);
                         increase_array_counter(si); 
                     }
                } 
                //it is a matched output
                if(si->matched_start!=INVALID && is_matched_state(qa, si)==1)  
                { 
                    int position = lexer.next_start - lexer.begin_stream;
                    char* output_text = substring(lexer.begin_stream, si->matched_start, position); 
                    si->matched_start = INVALID;
                    addTupleListElement(tl, si->query_state, output_text); 
                }
                break;
            case LB:   //left square branket 
                //it is a matched output
                if(is_matched_state(qa, si)==1)  
                { 
                    si->matched_start = lexer.next_start - lexer.begin_stream - 1;  
                }
                ary_s(qa, si, symbol, ss, qs);
                break;
            case RB:   //right square branket
                if(syntaxStackSize(ss) >= 1)  
                {
                    QueryStackElement top_si = queryStackTop(qs);
                    //it is a matched output
                    if(top_si.matched_start!=INVALID && is_matched_state(qa, &top_si)==1)  
                    { 
                        int position = lexer.next_start - lexer.begin_stream;
                        char* output_text = substring(lexer.begin_stream, top_si.matched_start, position); 
                        top_si.matched_start = INVALID;
                        addTupleListElement(tl, top_si.query_state, output_text);  
                    }
                    //syntax stack only has one '['
                    if(syntaxStackSize(ss) == 1)  
                    {   
                        ary_e(si, ss, qs); 
                    }
                    //syntax stack has at least two elements, the top element is '['
                    else if(syntaxStackSize(ss) > 1)  
                    {   
                        //after popping out '[', the next element on top of syntax stack is a key field
                        if(syntaxStackSecondTop(ss)==KY)  
                        {   
                            val_ary_e(si, ss, qs); 
                        }
                        //after popping out '[', the next element on top of syntax stack is '['
                        else if(syntaxStackSecondTop(ss)==LB)  
                        {  
                            elt_ary_e(si, ss, qs); 
                            increase_array_counter(si);
                        }
                    }
                }
                break;
            case COM:   //comma
                break;
            case KY:    //key field
                key(qa, si, symbol, lexer.content, ss, qs); 
                break;
            case PRI:   //primitive
                if(syntaxStackSize(ss) >= 1)
                {
                    //it is a matched output
                    if(is_matched_state(qa, si)==1)  
                    { 
                        addTupleListElement(tl, si->query_state, lexer.content);  
                    }
                    //the top element on syntax stack is a key field
                    if(syntaxStackTop(ss)==KY)  
                    {
                        val_pmt(si, ss, qs); 
                    }
                    //the top element on syntax stack is '['
                    else if(syntaxStackTop(ss)==LB)  
                    {
                        increase_array_counter(si);
                    }
                }
                break;
        }
        symbol = nextToken(&lexer); 
    }
    destroyLexer(&lexer);
    printf("syntax size %d query state %d %d\n", syntaxStackSize(ss), si->query_state, qs->top_item);
    printf("output size %d\n", getTupleListSize(tl));
}


