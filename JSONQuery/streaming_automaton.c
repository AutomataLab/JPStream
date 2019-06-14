#include "streaming_automaton.h"

static inline void increaseArrayCounter(QueryStackElement* qs_elt)
{
    qs_elt->count++;
}

/* 
   return matched type for current state
   0 -- not match DFA_OUTPUT_CANDIDATE -- output candidate 
   DFA_CONDITION -- predicate condition
   DFA_PREDICATE -- array predicate
*/
static int getMatchedType(JSONQueryDFA* qa, QueryStackElement* qs_elt)
{
    int match = getDFAAcceptType(qa, qs_elt->query_state);
    //it is a matched output
    if(match==DFA_OUTPUT_CANDIDATE)  
    {
        JSONQueryIndexPair pair = getDFAArrayIndex(qa, qs_elt->query_state);
        int lower = pair.lower; 
        int upper = pair.upper;
        int counter = qs_elt->count;
        //check array indexes
        if(!((lower==0&&upper==0)||(counter>=lower && counter<upper)))  
        {
            match = 0; 
        }
    }
    return match;
}

/* 
   implementation of state transition rules based on based on input token, query state, top elements on syntax stack and query stack

   in the following 10 functions:
   ss -- syntax stack qs -- query stack;
   qs_elt -- current state infomation  qa -- query automaton
*/
static inline void obj_s(SyntaxStack* ss)
{
    syntaxStackPush(ss, LCB);
}

static inline void obj_e(SyntaxStack* ss)
{
    syntaxStackPop(ss);
}

static inline void val_obj_e(QueryStackElement* qs_elt, SyntaxStack* ss, QueryStack* qs)
{
    *qs_elt = queryStackPop(qs);
    syntaxStackPop(ss);
    syntaxStackPop(ss);
}

static inline void elt_obj_e(SyntaxStack* ss)
{
    syntaxStackPop(ss);
}

static inline void ary_s(JSONQueryDFA* qa, QueryStackElement* qs_elt, SyntaxStack* ss, QueryStack* qs)  
{
    syntaxStackPush(ss, LB);
    //push current state into query stack
    queryStackPush(qs, *qs_elt);   
    //move onto next state
    QueryStackElement next_qs_elt;                      
    JSONQueryIndexPair pair = getDFAArrayIndex(qa, qs_elt->query_state);
    int lower = pair.lower;
    int upper = pair.upper; 
    int counter = qs_elt->count;
    //check array indexes 
    if(!((lower==0&&upper==0)||(counter>=lower && counter<upper)))
        next_qs_elt.query_state = 0; 
    else next_qs_elt.query_state = dfaNextStateByStr(qa, qs_elt->query_state, DFA_ARRAY);
    next_qs_elt.count = 0;
    next_qs_elt.matched_start = INVALID;
    //update current state info
    *qs_elt  = next_qs_elt; 
}

static inline void ary_e(QueryStackElement* qs_elt, SyntaxStack* ss, QueryStack* qs)
{
    *qs_elt = queryStackPop(qs);
    syntaxStackPop(ss);
}

static inline void val_ary_e(QueryStackElement* qs_elt, SyntaxStack* ss, QueryStack* qs)
{
    queryStackPop(qs);
    *qs_elt = queryStackPop(qs); 
    syntaxStackPop(ss); 
    syntaxStackPop(ss); 
}

static inline void elt_ary_e(QueryStackElement* qs_elt, SyntaxStack* ss, QueryStack* qs)
{
    *qs_elt = queryStackPop(qs);
    syntaxStackPop(ss); 
}

static inline void key(JSONQueryDFA* qa, QueryStackElement* qs_elt, char* key_string, SyntaxStack* ss, QueryStack* qs)  
{
    syntaxStackPush(ss, KY);
    //push current state into query stack
    queryStackPush(qs, *qs_elt);
    //move onto next state    
    QueryStackElement next_qs_elt;                 
    JSONQueryIndexPair pair = getDFAArrayIndex(qa, qs_elt->query_state);
    int lower = pair.lower; 
    int upper = pair.upper; 
    int counter = qs_elt->count;
    //check array indexes
    if(!((lower==0&&upper==0)||(counter>=lower && counter<upper)))  
        next_qs_elt.query_state = 0; 
    else //get next state based on key field
        next_qs_elt.query_state = dfaNextStateByStr(qa, qs_elt->query_state, key_string);  
    next_qs_elt.count = 0;
    next_qs_elt.matched_start = INVALID;
    //update current state info
    *qs_elt  = next_qs_elt;
}

static inline void val_pmt(QueryStackElement* qs_elt, SyntaxStack* ss, QueryStack* qs)
{
    *qs_elt = queryStackPop(qs);
    syntaxStackPop(ss);
}

// the execution of streaming automaton
void executeAutomaton(StreamingAutomaton* sa, char* json_stream, int data_constraint_flag)
{
    JSONQueryDFA* qa = sa->query_automaton;   //query automaton
    SyntaxStack* ss = &sa->syntax_stack;      //syntax stack
    QueryStack* qs = &sa->query_stack;        //query stack
    TupleList* tl = sa->tuple_list;           //tuple list
    QueryStackElement qs_element;                 //current state info
    qs_element.query_state = sa->query_state;
    qs_element.count = sa->count;
    qs_element.matched_start = sa->matched_start;
    QueryStackElement* qs_elt = &qs_element;  //address of qs_element

    //initialize lexer
    Lexer lexer;
    initLexer(&lexer, json_stream);

    Token token = nextToken(&lexer);
    int token_type = token.token_type;

    ConstraintTable* ct = sa->constraint_table;
    //initialize constraint table
    if(data_constraint_flag==OPEN)
    {
        //remove old constraint table
        if(ct!=NULL) freeConstraintTable(ct); 
        //create new constraint table
        sa->constraint_table = createConstraintTable();
        sa->constraint_flag = OPEN;
        ct = sa->constraint_table;
    }
    //select transition rules based on input token, query state, top elements on syntax stack and query stack
    while(token_type!=END)
    {   
        switch(token_type)
        {   
            case LCB:   //left curly branket
                {
                    obj_s(ss);  
                    int matched_type = getMatchedType(qa,qs_elt);
                    if(matched_type==DFA_OUTPUT_CANDIDATE)  
                    { 
                        qs_elt->matched_start = lexer.next_start - lexer.begin_stream - 1; 
                    }
                    else if(matched_type==DFA_PREDICATE)
                    {
                        addTuple(tl, qs_elt->query_state, "{"); 
                    }
                    else if(matched_type==DFA_CONDITION)
                    {   
                        addTuple(tl, qs_elt->query_state, "");
                    }
                }
                break;
            case RCB:   //right curly branket
                //it is a matched output
                if(qs_elt->matched_start!=INVALID && getMatchedType(qa, qs_elt)==DFA_OUTPUT_CANDIDATE)
                {
                    int position = lexer.next_start - lexer.begin_stream;
                    addVirtualTuple(tl, qs_elt->query_state, qs_elt->matched_start, position);
                }
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
                        val_obj_e(qs_elt, ss, qs);
                    }
                    //after popping out '{', the next element on top of syntax stack is '['
                    else if(syntaxStackSecondTop(ss)==LB)  
                    {
                        elt_obj_e(ss);
                        increaseArrayCounter(qs_elt);
                        if(getMatchedType(qa, qs_elt)==DFA_PREDICATE)
                        {  
                            addTuple(tl, qs_elt->query_state, "}");
                        }
                    }
                } 
                break;
            case LB:   //left square branket 
                {
                    int matched_type = getMatchedType(qa, qs_elt);                 
                    if(matched_type==DFA_OUTPUT_CANDIDATE)  
                    { 
                        qs_elt->matched_start = lexer.next_start - lexer.begin_stream - 1;  
                    }
                    else if(matched_type==DFA_CONDITION)
                    {
                        addTuple(tl, qs_elt->query_state, "");
                    }
                }
                if(data_constraint_flag==OPEN)
                { 
                    int query_state = qs_elt->query_state;
                    addConstraintInfo(ct, query_state, LB, token.content);
                    JSONQueryIndexPair pair = getDFAArrayIndex(qa, query_state);
                    int lower = pair.lower;
                    int upper = pair.upper;
                    int counter = qs_elt->count;
                    //check array indexes
                    if(!((lower==0&&upper==0)||(counter>=lower && counter<upper)))
                        addConstraintInfo(ct, 0, LB, "");
                }
                ary_s(qa, qs_elt, ss, qs);
                break;
            case RB:   //right square branket
                if(syntaxStackSize(ss) >= 1)  
                {
                    QueryStackElement top_qs_elt = queryStackTop(qs);
                    //it is a matched output
                    if(top_qs_elt.matched_start!=INVALID && getMatchedType(qa, &top_qs_elt)==DFA_OUTPUT_CANDIDATE)  
                    { 
                        int position = lexer.next_start - lexer.begin_stream;
                        addVirtualTuple(tl, qs_elt->query_state, qs_elt->matched_start, position);
                        top_qs_elt.matched_start = INVALID;
                    }
                    //syntax stack only has one '['
                    if(syntaxStackSize(ss) == 1)  
                    {   
                        ary_e(qs_elt, ss, qs); 
                    }
                    //syntax stack has at least two elements, the top element is '['
                    else if(syntaxStackSize(ss) > 1)  
                    {   
                        //after popping out '[', the next element on top of syntax stack is a key field
                        if(syntaxStackSecondTop(ss)==KY)  
                        {   
                            val_ary_e(qs_elt, ss, qs); 
                        }
                        //after popping out '[', the next element on top of syntax stack is '['
                        else if(syntaxStackSecondTop(ss)==LB)  
                        {  
                            elt_ary_e(qs_elt, ss, qs); 
                            increaseArrayCounter(qs_elt);
                        }
                    }
                }
                break;
            case COM:   //comma
                break;
            case KY:    //key field 
                if(data_constraint_flag==OPEN)
                { 
                    int query_state = qs_elt->query_state;
                    addConstraintInfo(ct, query_state, KY, token.content);
                    JSONQueryIndexPair pair = getDFAArrayIndex(qa, query_state);
                    int lower = pair.lower;
                    int upper = pair.upper;
                    int counter = qs_elt->count;
                    //check array indexes
                    if(!((lower==0&&upper==0)||(counter>=lower && counter<upper)))
                        addConstraintInfo(ct, 0, KY, token.content);
                }
                key(qa, qs_elt, token.content, ss, qs);
                break;
            case PRI:   //primitive
                if(syntaxStackSize(ss) >= 1)
                {
                    int matched_type = getMatchedType(qa,qs_elt);
                    if(matched_type==DFA_OUTPUT_CANDIDATE)  
                    { 
                        addVirtualTuple(tl, qs_elt->query_state, lexer.start_content, lexer.end_content);
                    }
                    else if(matched_type==DFA_CONDITION)
                    {
                        addVirtualTuple(tl, qs_elt->query_state, lexer.start_content, lexer.end_content);
                    }
                    //the top element on syntax stack is a key field
                    if(syntaxStackTop(ss)==KY)  
                    {
                        val_pmt(qs_elt, ss, qs); 
                    }
                    //the top element on syntax stack is '['
                    else if(syntaxStackTop(ss)==LB)  
                    {
                        increaseArrayCounter(qs_elt);
                    }
                }
                break;
        }
        token = nextToken(&lexer);
        token_type = token.token_type; 
    }
    destroyLexer(&lexer);
    sa->query_state = qs_elt->query_state;
    sa->count = qs_elt->count;
    sa->matched_start = qs_elt->matched_start;
    ///printf("syntax size %d query state %d %d\n", syntaxStackSize(ss), sa->query_state, qs->top_item);
    printf("size of 2-tuple list before filtering %d\n", getTupleListSize(tl));
}


