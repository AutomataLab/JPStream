#include "worker_automaton.h"

#define DFA_INCOMPLETE_INDEX 4

/* 
   return matched type for current state
   0 -- not match DFA_OUTPUT_CANDIDATE -- output candidate 
   DFA_CONDITION -- predicate condition
   DFA_PREDICATE -- array predicate
*/
static inline int getMatchedType(JSONQueryDFA* qa, TreeNode* tnode)
{
    int match = getDFAAcceptType(qa, tnode->query_state);
    //it is a matched output
    if(match==DFA_OUTPUT_CANDIDATE)  
    {   ///printf("candidate %d\n", tnode->query_state);
        JSONQueryIndexPair pair = getDFAArrayIndex(qa, tnode->query_state);
        int lower = pair.lower; 
        int upper = pair.upper;
        int counter = tnode->count;
        //check array indexes
        /*if(tnode->root_range.end==-1)  //root node
        {   printf("issue %d %d %d\n", lower, upper, tnode->query_state);
            if(!((lower==0&&upper==0)||(counter<upper))) match = 0;
            else if(!(lower==0&&upper==0)) match = DFA_INCOMPLETE_INDEX; printf("issue result %d\n", match);
        }
        else*/ if(!((lower==0&&upper==0)||(counter>=lower && counter<upper)))
        {
            match = 0;
        }
    }
    else if(tnode->root_range.end==-1)
    {
        JSONQueryIndexPair pair = getDFAArrayIndex(qa, tnode->query_state);
        int lower = pair.lower; 
        int upper = pair.upper;
        int counter = tnode->count;
        if(!((lower==0&&upper==0)||(counter<upper))) match = 0;
        else if(!(lower==0&&upper==0)) match = DFA_INCOMPLETE_INDEX; 
    }
    return match;
}

//syntax stack feasibility inference
static inline void syntaxStackInference(Lexer* lexer, int input_token_type, SyntaxStack* shadow_ss, SyntaxStack* ss)
{
    Token first_token = nextToken(lexer);
    //printf("lexer 1 %d %d\n",  lexer->next_start, first_token.token_type);
    Token second_token = nextToken(lexer);
    //printf("first %d second %d %d\n", first_token.token_type, second_token.token_type, lexer->next_start);
    if(input_token_type==RCB)
    {
        if(syntaxStackSize(ss)==1)
        {
            if(first_token.token_type==RCB || (first_token.token_type==COM && second_token.token_type==KY))  //val-obj-e
                syntaxStackPush(shadow_ss, KY);
            else if(!(first_token.token_type==LCB || first_token.token_type==END)) //elt-obj-e
                syntaxStackPush(shadow_ss, LB);
        }
        else if(syntaxStackSize(ss)==0)
        {
            if(first_token.token_type==RCB || (first_token.token_type==COM && second_token.token_type==KY))  //val-obj-e
            {
                syntaxStackPush(shadow_ss, KY);
                syntaxStackPush(shadow_ss, LCB);
            }
            else if(first_token.token_type==LCB || first_token.token_type==END) //obj-e
            {
                syntaxStackPush(shadow_ss, LCB);
            }
            else  //elt-obj-e
            {
                syntaxStackPush(shadow_ss, LB);
                syntaxStackPush(shadow_ss, LCB);
            }
        }
    }
    else if(input_token_type == RB)
    {
        if(syntaxStackSize(ss)==1)
        {
            if(first_token.token_type==RCB || (first_token.token_type==COM && second_token.token_type==KY))  //val-ary-e
                syntaxStackPush(shadow_ss, KY);
            else if(!(first_token.token_type==LB || first_token.token_type==END)) //elt-ary-e
                syntaxStackPush(shadow_ss, LB);
        }
        else if(syntaxStackSize(ss)==0)
        {
            if(first_token.token_type==RCB || (first_token.token_type==COM && second_token.token_type==KY))  //val-ary-e
            {
                syntaxStackPush(shadow_ss, KY);
                syntaxStackPush(shadow_ss, LB);
            }
            else if(first_token.token_type==LB || first_token.token_type==END) //ary-e
            {
                syntaxStackPush(shadow_ss, LB);
            }
            else  //elt-ary-e
            {
                syntaxStackPush(shadow_ss, LB);
                syntaxStackPush(shadow_ss, LB);
            }
        }
    }
    else if(input_token_type == PRI)
    {
        if(first_token.token_type==RCB || (first_token.token_type==COM && second_token.token_type==KY))  //val-pmt
            syntaxStackPush(shadow_ss, KY);
        else //elt-pmt
            syntaxStackPush(shadow_ss, LB);
    }
}

/* 
   implementation of state transition rules based on based on input token, query state, top elements on syntax stack and query stack

   in the following 10 functions:
   ss -- syntax stack qs -- query stack;
   si -- current state infomation  qa -- query automaton
*/
static inline void obj_s(SyntaxStack* ss)
{
    syntaxStackPush(ss, LCB);
}

static inline void obj_e(SyntaxStack* ss)
{
    syntaxStackPop(ss);
}

static inline void val_obj_e(QueryStacksElement* qs_elt, SyntaxStack* ss, QueryStacks* qs)
{
    *qs_elt = queryStacksPop(qs);
    syntaxStackPop(ss);
    syntaxStackPop(ss);
}

static inline void elt_obj_e(SyntaxStack* ss)
{
    syntaxStackPop(ss);
}

static inline void ary_s(JSONQueryDFA* qa, QueryStacksElement* qs_elt, SyntaxStack* ss, QueryStacks* qs)  
{
    syntaxStackPush(ss, LB);
    //push current state into query stack and move onto the next state
    *qs_elt = queryStacksPush(qs, *qs_elt, qa, DFA_ARRAY);
}

static inline void ary_e(QueryStacksElement* qs_elt, SyntaxStack* ss, QueryStacks* qs)
{
    *qs_elt = queryStacksPop(qs);
    syntaxStackPop(ss);
}

static inline void val_ary_e(QueryStacksElement* qs_elt, SyntaxStack* ss, QueryStacks* qs)
{
    queryStacksPop(qs);
    *qs_elt = queryStacksPop(qs); 
    syntaxStackPop(ss); 
    syntaxStackPop(ss); 
}

static inline void elt_ary_e(QueryStacksElement* qs_elt, SyntaxStack* ss, QueryStacks* qs)
{
    *qs_elt = queryStacksPop(qs);
    syntaxStackPop(ss); 
}

static inline void key(JSONQueryDFA* qa, QueryStacksElement* qs_elt, char* key_string, SyntaxStack* ss, QueryStacks* qs)  
{
    syntaxStackPush(ss, KY);
    //push current state into query stack and move onto the next state
    *qs_elt = queryStacksPush(qs, *qs_elt, qa, key_string);
}

static inline void val_pmt(QueryStacksElement* qs_elt, SyntaxStack* ss, QueryStacks* qs)
{
    *qs_elt = queryStacksPop(qs);
    syntaxStackPop(ss);
}

void executeWorkerAutomaton(WorkerAutomaton* wa, char* json_stream)
{
    //query automaton
    JSONQueryDFA* qa = wa->query_automaton;   
   
    /* to maximum the performance benefits, worker automaton operates on its local syntax and query stacks, 
       and update wa->syntax_stack and wa->query_stack after it finishes processing all input tokens*/
    SyntaxStack ss;      
    initSyntaxStack(&ss);
    //shadow syntax stack (used for syntax stack inference)
    SyntaxStack shadow_ss;                    
    initSyntaxStack(&shadow_ss);
    QueryStacks qs;// = wa->query_stack; 
 //   QueryStackElement qs_elt = wa->query_stack_element;      
    /*int query_state[MAX_STATE];
    int num_query_state = getDFAStatesNumber(qa);
    int i; 
    for(i = 1; i<=num_query_state; i++)
        query_state[i-1] = i-1;
    /////query_state[0] = 1; query_state[1] = 2; query_state[2] = 3; query_state[3] = 4; query_state[4] = 5;
    QueryStackElement qs_elt = initQueryStack(&qs, query_state, num_query_state); */
    int query_state[MAX_STATE];
    QueryStacksElement qs_elt;
    int num_query_state;
    if(wa->id==0)
    {
        query_state[0] = 1;
        num_query_state = 1;
        qs_elt = initQueryStacks(&qs, query_state, 1); 
    }
    else
    {
        num_query_state = getDFAStatesNumber(qa);
        int i; 
        for(i = 1; i<=num_query_state; i++)
            query_state[i-1] = i-1;
        qs_elt = initQueryStacks(&qs, query_state, num_query_state);
    } 
    
    /*int num_query_state = getDFAStatesNumber(qa);
    int i; 
    for(i = 1; i<=num_query_state; i++)
        query_state[i-1] = i-1;
    QueryStackElement qs_elt = initQueryStack(&qs, query_state, num_query_state);*/
    
    //constraint table
    int constraint_flag = wa->constraint_flag;
    ConstraintTable* ct = wa->constraint_table;

    //tuple list
    TupleList* tl = wa->tuple_list;           

    //initialize unit list
    UnitList* ul = &wa->unit_list;
    initUnitList(ul);

    //initialize unit
    Unit unit;
    initUnit(&unit);

    //initialize lexer
    Lexer lexer;
    initLexer(&lexer, json_stream);

    Token token = nextToken(&lexer);
    int token_type = token.token_type;

    //save information for current unmatched symbol
    UnmatchedSymbol us;

    int debug_counter = 0;

    //select transition rules based on input token, query state, top elements on syntax stack and query stack
    while(token_type!=END)
    {   
        switch(token_type)
        {   
            case LCB:   //left curly branket
                {
                    if(unit.unit_state!=UNIT_MATCHED)
                        unit.unit_state = UNIT_MATCHED;
                    obj_s(&ss);  
                    //write an iterator to get all nodes for current states from qs_elt
                    int start_index=qs_elt.start, end_index=qs_elt.end;
                    while(start_index<=end_index)
                    {
                        TreeNode c_node = qs.node[start_index];
                        int matched_type = getMatchedType(qa,&c_node); //if(wa->id==1&&ul->count_units==0&&(c_node.query_state==4||c_node.query_state==8)) printf("before predicate %d\n", c_node.query_state);
                        if(matched_type==DFA_OUTPUT_CANDIDATE)  
                        { 
                            qs.node[start_index].matched_start = lexer.next_start - lexer.begin_stream - 1; 
                        }
                        else if(matched_type==DFA_PREDICATE)
                        {   //if(wa->id==1&&ul->count_units==0) printf("predicate { %d %d\n", c_node.query_state, debug_counter++);
                            addTupleInfo(&qs, start_index, c_node.query_state, "{", tl);
                        }
                        else if(matched_type==DFA_CONDITION)
                        {  
                            addTupleInfo(&qs, start_index, c_node.query_state, "", tl);
                        }
                        else if(matched_type==DFA_INCOMPLETE_INDEX)
                        { 
                            //addTupleInfo(&qs, start_index, c_node.query_state, "{", tl); 
                        }
                        start_index++;
                    }
                }
                break;
            case RCB: //right curly branket
                {
                    if(syntaxStackSize(&ss) > 0)
                    {
                        //check matched output 
                        int start_index=qs_elt.start, end_index=qs_elt.end;
                        while(start_index<=end_index)
                        {
                            TreeNode c_node = qs.node[start_index];
                            if(c_node.matched_start!=INVALID)
                            {
                                int matched_type = getMatchedType(qa,&c_node);
                                if(matched_type==DFA_OUTPUT_CANDIDATE)
                                {  
                                    int matched_end = lexer.next_start - lexer.begin_stream;
                                    //char* output_text = substring(lexer.begin_stream, c_node.matched_start, position);
                                    //char output_text[40000];
                                    //substring1(output_text, lexer.begin_stream, c_node.matched_start, position);
                                    ///////////printf("<<< start %d end %d state %d node index %d %d\n", c_node.matched_start, matched_end, c_node.query_state, start_index, c_node.root_range.end);
                                    if(wa->id==0){
                                        char* output_text = substring(lexer.begin_stream, c_node.matched_start, matched_end);
                                        addTupleInfo(&qs, start_index, c_node.query_state, output_text, tl); 
                                    }
                                    else addVirtualTupleInfo(&qs, start_index, c_node.query_state, c_node.matched_start, matched_end, tl);
                                    qs.node[start_index].matched_start = INVALID; 
                                    //////if(wa->id==2) printf("********** %s\n", substring(lexer.begin_stream, c_node.matched_start, matched_end));
                                    //addTupleInfo(&qs, start_index, c_node.query_state, output_text, tl); 
                                    //addVirtualTuple(tl, c_node.query_state, c_node.matched_start, position);
                                }
                            }
                            start_index++;
                        }
                    }
                    if(syntaxStackSize(&ss) == 0)
                    {
                        if(unit.unit_state==UNIT_MATCHED)
                        {
                            //printf("} %d\n", ul->count_units);
                            //create a new unit
                            addRoots(&unit, qs.node, qs.num_node); /////////if(qs.num_node==4) printf("<<>>>>} unmatched %d\n",ul->count_units+1);
                            addUnit(ul, unit);
                            initUnit(&unit);
                            //automata resetting
                            initSyntaxStack(&ss);
                            initSyntaxStack(&shadow_ss);
                            qs_elt = initQueryStacks(&qs, query_state, num_query_state);
                        }
                        us.token_type = RCB;
                        us.index = lexer.next_start - lexer.begin_stream - 1;
                        addUnmatchedSymbol(&unit, us);   
                        //printf("1} %d\n", ul->count_units);
                    }
                    //syntax stack only has one '{'
                    else if(syntaxStackSize(&ss) == 1)
                    {
                        if(syntaxStackSize(&shadow_ss)==0)
                        {   
                            Lexer temp_lexer = lexer;
                            syntaxStackInference(&temp_lexer, token_type, &shadow_ss, &ss);
                        }
                        if(syntaxStackSize(&shadow_ss)==0)
                            obj_e(&ss);
                        else if(syntaxStackTop(&shadow_ss)==KY)
                        {
                            if(unit.unit_state==UNIT_MATCHED)
                            {
                                //create a new unit
                                addRoots(&unit, qs.node, qs.num_node); ///if(qs.num_node==4) printf("<<>>>>} unmatched %d\n",ul->count_units+1);
                                addUnit(ul, unit);
                                initUnit(&unit);
                                //automata resetting
                                initSyntaxStack(&ss);
                                initSyntaxStack(&shadow_ss);
                                qs_elt = initQueryStacks(&qs, query_state, num_query_state);
                            }
                            //simply pop stack for KEY during merging phase
                            us.token_type = OBJECT;
                            addUnmatchedSymbol(&unit, us); 
                        }
                        else if(syntaxStackTop(&shadow_ss)==LB)
                        {
                            obj_e(&ss); 
			    //write an iterator to get all nodes for current states from qs_elt
                            int start_index=qs_elt.start, end_index=qs_elt.end;
                            while(start_index<=end_index)
                            {
                                TreeNode c_node = qs.node[start_index];
                                //increase array counter
                                qs.node[start_index].count++;
                                int matched_type = getMatchedType(qa,&c_node); ///if(wa->id==37&&c_node.query_state==6) printf("matched type %d %d counter %d rootend %d\n", matched_type, c_node.query_state, qs.node[start_index].count, c_node.root_range.end);
                                if(matched_type==DFA_PREDICATE)
                                {
                                    addTupleInfo(&qs, start_index, c_node.query_state, "}", tl);
                                }
                                else if(matched_type==DFA_INCOMPLETE_INDEX)
                                {
                                    addTupleInfo(&qs, start_index, c_node.query_state, "}", tl);  //////if(wa->id==2) printf("**********special }\n");
                                }
                                start_index++;
                            }
                        }
                    }
                    //syntax stack has at least two elements, the top element is '{'
                    else if(syntaxStackSize(&ss) > 1)
                    {
                        //after popping out '{', the next element on top of syntax stack is a key field
                        if(syntaxStackSecondTop(&ss)==KY)
                        {
                            val_obj_e(&qs_elt, &ss, &qs);
                        }
                        //after popping out '{', the next element on top of syntax stack is '['
                        else if(syntaxStackSecondTop(&ss)==LB)
                        {
                            elt_obj_e(&ss);
                            //write an iterator to get all nodes for current states from qs_elt
                            int start_index=qs_elt.start, end_index=qs_elt.end;
                            while(start_index<=end_index)
                            {
                                TreeNode c_node = qs.node[start_index];
                                //increase array counter
				qs.node[start_index].count++;
                                int matched_type = getMatchedType(qa,&c_node);
                                if(matched_type==DFA_PREDICATE)
                                {
                                    addTupleInfo(&qs, start_index, c_node.query_state, "}", tl);
                                }
                                start_index++;
                            }
                        }
                    }
                }
                break;
            case LB:   //left square branket 
                {
                    if(unit.unit_state!=UNIT_MATCHED)
                        unit.unit_state = UNIT_MATCHED;
                    //prune infeasible paths during runtime
                    if(wa->id>0&&constraint_flag==OPEN && unit.has_pruned == UNPRUNED)
                    {
                        ConstraintInfo ci;
                        ci.type = LB;
                        updateStateInfo(ct, &ci);  
                        if(ci.num_state!=INVALID)
                        {   if(wa->id==1) printf("check array %d %d\n", ci.num_state, ci.state_set[0]);
                             qs_elt = pruneQueryPaths(&qs, ci.state_set, ci.num_state);
                            //initQueryStacks(&qs, ci.state_set, ci.num_state);
                            //unit.unit_state = UNIT_PATH_PRUNED;
                        }
                        unit.unit_state = PRUNED;
                    }
                    //write an iterator to get all nodes for current states from qs_elt
                    int start_index=qs_elt.start, end_index=qs_elt.end;
                    while(start_index<=end_index)
                    {   
                        TreeNode c_node = qs.node[start_index];
                        int matched_type = getMatchedType(qa,&c_node);
                        if(matched_type==DFA_OUTPUT_CANDIDATE)  
                        { 
                            qs.node[start_index].matched_start = lexer.next_start - lexer.begin_stream - 1; 
                        }
                        else if(matched_type==DFA_CONDITION)
                        {   
                            addTupleInfo(&qs, start_index, c_node.query_state, "", tl);
                        }
                        else if(matched_type==DFA_INCOMPLETE_INDEX)
                        {
                            //addTupleInfo(&qs, start_index, c_node.query_state, "[", tl);
                        }
                        start_index++;
                    }
                }
                ary_s(qa, &qs_elt, &ss, &qs);
                break;
            case RB:   //right square branket
                if(syntaxStackSize(&ss) >= 1)  
                {
                    QueryStacksElement top_qs_elt = queryStacksTop(&qs);
                    //check matched output
                    int start_index= top_qs_elt.start, end_index= top_qs_elt.end;
                    while(start_index<=end_index)
                    {
                        TreeNode c_node = qs.node[start_index];
                        if(c_node.matched_start!=INVALID)
                        {
                            int matched_type = getMatchedType(qa,&c_node);
                            if(matched_type==DFA_OUTPUT_CANDIDATE)
                            {   
                                int matched_end = lexer.next_start - lexer.begin_stream;
                                //char* output_text = substring(lexer.begin_stream, c_node.matched_start, position);
                                //char output_text[40000];
                                //substring1(output_text, lexer.begin_stream, c_node.matched_start, position);
                                //printf("<<<<<<<<start %d end %d\n", c_node.matched_start, matched_end);
                                if(wa->id==0){
                                    char* output_text = substring(lexer.begin_stream, c_node.matched_start, matched_end);
                                    addTupleInfo(&qs, start_index, c_node.query_state, output_text, tl); 
                                }
                                else addVirtualTupleInfo(&qs, start_index, c_node.query_state, c_node.matched_start, matched_end, tl);
                                qs.node[start_index].matched_start = INVALID; 
                                //addTupleInfo(&qs, start_index, c_node.query_state, output_text, tl);
                                //addVirtualTuple(tl, c_node.query_state, c_node.matched_start, position);
                            }
                            else if(matched_type==DFA_INCOMPLETE_INDEX)
                            {
                                addTupleInfo(&qs, start_index, c_node.query_state, "]", tl);
                            }
                        }
                        start_index++;
                    }
                    //syntax stack only has one '['
                    if(syntaxStackSize(&ss) == 1)  
                    {   
                        if(syntaxStackSize(&shadow_ss)==0)  
                        {
                            Lexer temp_lexer = lexer;
                            syntaxStackInference(&temp_lexer, token_type, &shadow_ss, &ss);
                        }
                        if(syntaxStackSize(&shadow_ss)==0)  
                            ary_e(&qs_elt, &ss, &qs); 
                        else if(syntaxStackTop(&shadow_ss)==KY)	 
                        {   
                            //create a new unit
                            if(unit.unit_state==UNIT_MATCHED)
                            {
                                //create a new unit
                                addRoots(&unit, qs.node, qs.num_node); ///////////if(qs.num_node==4) printf("<<>>>>} unmatched %d\n",ul->count_units+1);
                                addUnit(ul, unit);
                                initUnit(&unit);
                                //automata resetting
                                initSyntaxStack(&ss);
                                initSyntaxStack(&shadow_ss);
                                qs_elt = initQueryStacks(&qs, query_state, num_query_state);
                            }
                            //simply pop stack for KEY during merging phase
                            us.token_type = ARRAY;
                            addUnmatchedSymbol(&unit, us);  
                        }
                        else if(syntaxStackTop(&shadow_ss)==LB)  
                        {
                            ary_e(&qs_elt, &ss, &qs); 
                            //write an iterator to get all nodes for current states from qs_elt
                            int start_index= qs_elt.start, end_index= qs_elt.end;
                            while(start_index<=end_index)
                            {
                                TreeNode c_node = qs.node[start_index];
                                //increase array counter
                                qs.node[start_index].count++;
                                start_index++;
                            }
                        }
                    }
                    //syntax stack has at least two elements, the top element is '['
                    else if(syntaxStackSize(&ss) > 1)  
                    {   
                        //after popping out '[', the next element on top of syntax stack is a key field
                        if(syntaxStackSecondTop(&ss)==KY)  
                        {   
                            val_ary_e(&qs_elt, &ss, &qs); 
                        }
                        //after popping out '[', the next element on top of syntax stack is '['
                        else if(syntaxStackSecondTop(&ss)==LB)  
                        {  
                            elt_ary_e(&qs_elt, &ss, &qs); 
                            //write an iterator to get all nodes for current states from qs_elt
                            int start_index= qs_elt.start, end_index= qs_elt.end;
                            while(start_index<=end_index)
                            {
                                TreeNode c_node = qs.node[start_index];
                                //increase array counter
                                qs.node[start_index].count++;
                                start_index++;
                            }
                        }
                    }
                }
                else
                {
                    //create a new unit
                    if(unit.unit_state==UNIT_MATCHED)
                    {
                        //create a new unit
                        addRoots(&unit, qs.node, qs.num_node); ///////////if(qs.num_node==4) printf("<<>>>>} unmatched %d\n",ul->count_units+1);
                        addUnit(ul, unit);
                        initUnit(&unit);
                        //automata resetting
                        initSyntaxStack(&ss);
                        initSyntaxStack(&shadow_ss);
                        qs_elt = initQueryStacks(&qs, query_state, num_query_state);
                    }
                    us.token_type = RB;
                    us.index = lexer.next_start - lexer.begin_stream - 1;
                    addUnmatchedSymbol(&unit, us); //////if(wa->id==25) printf("&&&&&&&&& %s\n", substring(lexer.begin_stream, 0, us.index));
                }
                break;
            case COM:   //comma
                break;
            case KY:    //key field 
                if(unit.unit_state!=UNIT_MATCHED)
                    unit.unit_state = UNIT_MATCHED;
                //prune infeasible paths during runtime
                if(wa->id>0 && constraint_flag==OPEN && unit.has_pruned==UNPRUNED && qs.top_item == -1)
                {
                    ConstraintInfo ci;
                    ci.type = KY;
                    strcopy(token.content, ci.token_name);
                    updateStateInfo(ct, &ci);
                    if(ci.num_state!=INVALID)
                    {   if(wa->id==1) printf("check %s %d %d %d %d %d %d\n", ci.token_name, qs.top_item, ul->count_units, qs.num_node, ci.num_state, ci.state_set[0], ci.state_set[1]);
                        ///if(strcmp(token.content, "studio")!=0) //ci.state_set[0] = 9;
                        qs_elt = pruneQueryPaths(&qs, ci.state_set, ci.num_state);
                         ///   initQueryStacks(&qs, ci.state_set, ci.num_state);
                        //unit.unit_state = UNIT_PATH_PRUNED;
                    }
                    unit.has_pruned = PRUNED;
                }
                key(qa, &qs_elt, token.content, &ss, &qs);
                break;
            case PRI:   //primitive
                if(syntaxStackSize(&ss) >= 1)
                {
                    int start_index=qs_elt.start, end_index=qs_elt.end;
                    while(start_index<=end_index)
                    {
                        TreeNode c_node = qs.node[start_index];
                        int matched_type = getMatchedType(qa,&c_node);
                        if(matched_type==DFA_OUTPUT_CANDIDATE||matched_type==DFA_CONDITION)
                        {
                            addTupleInfo(&qs, start_index, c_node.query_state, token.content, tl);
                        }
                        if(syntaxStackTop(&ss)==LB)  
                            qs.node[start_index].count++;
                        start_index++;
                    }
                    //the top element on syntax stack is a key field
                    if(syntaxStackTop(&ss)==KY)  
                    {
                        val_pmt(&qs_elt, &ss, &qs); 
                    }
                }
                else{
                    if(syntaxStackSize(&shadow_ss)==0)  
                    {
                         Lexer temp_lexer = lexer;
                         syntaxStackInference(&temp_lexer, token_type, &shadow_ss, &ss);
                    }
                    if(syntaxStackTop(&shadow_ss)==KY)	 
                    {   
                         if(unit.unit_state==UNIT_MATCHED)
                         {
                             //create a new unit
                             addRoots(&unit, qs.node, qs.num_node); ///////////////if(qs.num_node==4) printf("<<>>>>} unmatched %d\n",ul->count_units+1);
                             addUnit(ul, unit);
                             initUnit(&unit);
                             //automata resetting
                             initSyntaxStack(&ss);
                             initSyntaxStack(&shadow_ss);
                             qs_elt = initQueryStacks(&qs, query_state, num_query_state);
                         }
                         //check output and pop key during merge phase
                         us.token_type = PRI;
                         addUnmatchedSymbol(&unit, us);  
                    }
                    else if(syntaxStackTop(&shadow_ss)==LB)  
                    {
                        if(unit.unit_state!=UNIT_MATCHED)
                            unit.unit_state = UNIT_MATCHED;
                        int start_index=qs_elt.start, end_index=qs_elt.end;
                        while(start_index<=end_index)
                        {
                            TreeNode c_node = qs.node[start_index];
                            int matched_type = getMatchedType(qa,&c_node);
                            if(matched_type==DFA_OUTPUT_CANDIDATE||matched_type==DFA_CONDITION)
                            {
                                addTupleInfo(&qs, start_index, c_node.query_state, token.content, tl);
                            }
                            qs.node[start_index].count++;
                            start_index++;
                        }
                    }
                }
                break;
        }
        token = nextToken(&lexer);
        token_type = token.token_type; 
    }
    addRoots(&unit, qs.node, qs.num_node); /////////////if(qs.num_node==4) printf("<<>>>>e} unmatched %d start %d end %d %d\n",ul->count_units+1, qs.item[0].start, qs.item[0].end, wa->id);
    addUnit(ul, unit);
    destroyLexer(&lexer);
    //update syntax stack for worker automaton
    wa->syntax_stack = ss;
    //update query stack for worker automaton
    wa->query_stacks = qs;
    wa->query_stacks_element = qs_elt;
    printf("syntax size %d query size %d %d %d\n", syntaxStackSize(&ss), qs.top_item, wa->query_stacks.top_item, ul->count_units);
    printf("output size %d\n", getTupleListSize(tl));
}

