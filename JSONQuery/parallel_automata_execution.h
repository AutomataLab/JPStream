#ifndef __PARALLEL_AUTOMATA_EXECUTION_H__
#define __PARALLEL_AUTOMATA_EXECUTION_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <time.h>
#include <unistd.h>
#include "utility.h"
#include "file_partition.h"
#include "worker_automaton.h"
#include "constraint.h"

#ifndef __USE_GNU
#define __USE_GNU 1
#endif
#include <pthread.h>

#define NOWARMUP 0
#define WARMUP 1
#define MAX_THREAD 100

//data structure for each thread
typedef struct ThreadInfo{
    pthread_t thread;
    int thread_id;
    char* input_stream;
    double execution_time;
    int cpu_warmup;  
    WorkerAutomaton* worker_automaton;
}ThreadInfo;

//main function for each thread
void *main_thread(void *arg)
{
    struct timeval start_timestamp, end_timestamp;
    double exe_timestamp;
    ThreadInfo* ti = (ThreadInfo*)arg;
    int t_id = ti->thread_id;

    //CPU warmup
    if(ti->cpu_warmup == WARMUP){
	gettimeofday(&start_timestamp,NULL);
        cpu_set_t mask;
        //cpu_set_t get;
        CPU_ZERO(&mask);
        CPU_SET(t_id, &mask);
        if(pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask)<0)
            printf("CPU failed\n");
        while(1)
        {
            gettimeofday(&end_timestamp,NULL);
            exe_timestamp=1000000*(end_timestamp.tv_sec-start_timestamp.tv_sec)+end_timestamp.tv_usec-start_timestamp.tv_usec;
            if(exe_timestamp>2000000) break;
        }
    }

    printf("thread %d starts.\n", t_id);
    gettimeofday(&start_timestamp,NULL);
    executeWorkerAutomaton(ti->worker_automaton, ti->input_stream); 
    gettimeofday(&end_timestamp,NULL);
    printf("thread %d finishes.\n", t_id);  
    exe_timestamp=1000000*(end_timestamp.tv_sec-start_timestamp.tv_sec)+end_timestamp.tv_usec-start_timestamp.tv_usec;
    ti->execution_time = exe_timestamp/1000000;
    printf("The execution time of thread %d is %lf\n", t_id, ti->execution_time); 
    return NULL;
}

// below are some transition functions used in serial streaming automaton, used during mering phase
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
        if(!((lower==0&&upper==0)||(counter>=lower && counter<upper)))  
        {
            match = 0; 
        }
    }
    return match;
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

// merging results from different chunks
TupleList* combine(ThreadInfo* thread_info, int num_thread)
{
    int i,j,k;
    //get some information for thread 0
    WorkerAutomaton* seq_wa = thread_info[0].worker_automaton;
    SyntaxStack ss = seq_wa->syntax_stack;
    QueryStacks qs = seq_wa->query_stacks;   
    QueryStacksElement qs_elt = seq_wa->query_stacks_element;
    QueryStatesInfo* qs_info = seq_wa->query_states_info;
    UnitList unit_list = seq_wa->unit_list;
    TupleList* tuple_list = seq_wa->tuple_list; 
    //query automaton
    JSONQueryDFA* qa = seq_wa->query_automaton;   
    //save information for current unmatched symbol
    UnmatchedSymbol us;

   /////////// printf("syntax size %d query size %d query_state %d\n", syntaxStackSize(&ss), qs.top_item, qs.node[qs_elt.start].query_state);
    int sum = getTupleListSize(tuple_list); 
    ///////////printf("sum 0 is %d\n", sum);

    for(int i = 1; i<num_thread; i++)
    {
        WorkerAutomaton* wa = thread_info[i].worker_automaton;
        UnitList ul = wa->unit_list; 
        TupleList* tl = wa->tuple_list;  
        //whether stacks for the current chunk needs to be merged after processing
        int merge_needed = 1; 
        //////////printf("thread %d count units %d\n", i, ul.count_units);  
        for(int j = 0; j<=ul.count_units; j++)
        { ///  if(i==30) {printf("j %d root num %d\n", j, ul.units[j].num_root); sleep(3);}
            //get current state information from query stack qs
            int node_index = qs_elt.start;
            TreeNode c_node = qs.node[node_index];
            int query_state = c_node.query_state;  //////if(i==1&&j==1) printf("special state is %d\n", query_state);
            Unit unit = ul.units[j];
            int count_unmatched_symbols = unit.count_unmatched_symbols;
            /////////////printf("unmatched symbols %d\n", count_unmatched_symbols);
            //handle unmatched tokens
            for(int k = 0; k<=count_unmatched_symbols; k++)
            {
                node_index = qs_elt.start;
                c_node = qs.node[node_index];
                query_state = c_node.query_state; 
                us = unit.unmatched_symbols[k];
                if(us.token_type==RCB)
                {
                    //////printf("}\n");
                    ///////if(i==1) printf("} unit %d state %d\n", j, query_state);
                    //get candidate objects across different chunks 
                    if(syntaxStackSize(&ss) > 0)
                    {
                        if(c_node.matched_start!=INVALID)
                        {
                            int matched_type = getMatchedType(qa,&c_node);
                            if(matched_type==DFA_OUTPUT_CANDIDATE)
                            {  
                                int left_len = strlen(thread_info[i-1].input_stream)-c_node.matched_start;
                                int right_len = us.index+1;
                                int strlen = left_len+right_len;
                                char* object_text = (char*)malloc((strlen+1)*sizeof(char));
                                substring_in_place(object_text, thread_info[i-1].input_stream, c_node.matched_start, c_node.matched_start+left_len);
                                //printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!thread %d %d %s %d %d %s\n", i, c_node.matched_start, object_text, c_node.matched_start+left_len, us.index, substring(thread_info[i].input_stream, 0, us.index+1));
                                substring_in_place(object_text+left_len, thread_info[i].input_stream, 0, us.index+1);
                                addTupleInfo(&qs, node_index, c_node.query_state, object_text, tuple_list);
                                qs.node[node_index].matched_start = INVALID;
                                
                                ///if(wa->id==10) printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!thread %d %d %s\n", i, c_node.matched_start, object_text);
                                sum++;
                            }
                        }
                    }
                    //syntax stack only has one '{'
                    if(syntaxStackSize(&ss) == 1)   
                    {
                        obj_e(&ss); 
                    }
                    else if(syntaxStackSize(&ss) > 1)
                    {
                        //after popping out '{', the next element on top of syntax stack is a key field
                        if(syntaxStackSecondTop(&ss)==KY)
                        {   //////if(i==30) printf("before changing state key %d %d syn size %d query size %d\n", qs.node[qs_elt.start].query_state, qs.top_item, syntaxStackSize(&ss), qs.top_item);
                            val_obj_e(&qs_elt, &ss, &qs);
                        }
                        //after popping out '{', the next element on top of syntax stack is '['
                        else if(syntaxStackSecondTop(&ss)==LB)
                        {
                            ///////if(i==30) printf("before changing state array %d %d syn size %d query size %d\n", qs.node[qs_elt.start].query_state, qs.top_item, syntaxStackSize(&ss), qs.top_item);
                            elt_obj_e(&ss);
                            //increase array counter 
                            qs.node[qs_elt.start].count++;  ///////////////if(i==2) 
                            //////printf("******increase counter %d state %d\n", qs.node[node_index].count, qs.node[node_index].query_state);
                            int matched_type = getMatchedType(qa,&c_node);
                            if(matched_type==DFA_PREDICATE)
                            {
                                sum++; 
                                addTupleInfo(&qs, node_index, c_node.query_state, "}", tuple_list);
                            }
                        }
                    }
                    //////if(i==30) printf("after changing state is %d %d syn size %d query size %d\n", qs.node[qs_elt.start].query_state, qs.top_item, syntaxStackSize(&ss), qs.top_item);
                }
                else if(us.token_type==RB) 
                {
                    //if(i==1) printf("] %d %d\n", j, c_node.query_state);
                    //get candidate arrays across different chunks  
                    if(syntaxStackSize(&ss) > 0)
                    {
                        QueryStacksElement top_qs_elt = queryStacksTop(&qs);
                        c_node = qs.node[top_qs_elt.start]; //if(i==25) printf("] %d %d %d\n", j, c_node.matched_start, c_node.query_state);
                        if(c_node.matched_start!=INVALID)
                        {
                            
                            int matched_type = getMatchedType(qa,&c_node);
                            if(matched_type==DFA_OUTPUT_CANDIDATE)
                            {  
                                int left_len = strlen(thread_info[i-1].input_stream)-c_node.matched_start;
                                int right_len = us.index;
                                int strlen = left_len+right_len;
                                char* object_text = (char*)malloc((strlen+1)*sizeof(char));
                                substring_in_place(object_text, thread_info[i-1].input_stream, c_node.matched_start, c_node.matched_start+left_len);
                     //////           printf("unit %d !!!!!!!!!!!!!!!!!!!!!!!!!!!thread %d %d %s %d %d %s\n", j, i, c_node.matched_start, object_text, c_node.matched_start+left_len, us.index, substring(thread_info[i].input_stream, 0, us.index+1));
                                substring_in_place(object_text+left_len, thread_info[i].input_stream, 0, us.index+1);
                                addTupleInfo(&qs, node_index, c_node.query_state, object_text, tuple_list);
                                qs.node[node_index].matched_start = INVALID;
                               /// qs.node[node_index].count++;
                     /////           printf("unit %d !!!!!!!!!!!!!!!!!!!!!!!!!!!thread %d %d %s len %d state %d count %d\n", j, i, c_node.matched_start, object_text, strlen, c_node.query_state, c_node.count); sleep(1);
                                sum++;
                            }
                        }
                    }
                    //syntax stack only has one '['
                    if(syntaxStackSize(&ss) == 1)  
                    {   
                        ary_e(&qs_elt, &ss, &qs); 
                    }
                    else if(syntaxStackSize(&ss) > 1)  
                    {   
                        //after popping out '[', the next element on top of syntax stack is a key field
                        if(syntaxStackSecondTop(&ss)==KY)  
                        {   
                          //////  if(i==31) printf("[before changing state key %d %d syn size %d query size %d\n", qs.node[qs_elt.start].query_state, qs.top_item, syntaxStackSize(&ss), qs.top_item);
                            val_ary_e(&qs_elt, &ss, &qs); 
                        }
                        //after popping out '[', the next element on top of syntax stack is '['
                        else if(syntaxStackSecondTop(&ss)==LB)  
                        {  
                         ///////   if(i==31) printf("[before changing state key %d %d syn size %d query size %d\n", qs.node[qs_elt.start].query_state, qs.top_item, syntaxStackSize(&ss), qs.top_item);
                            elt_ary_e(&qs_elt, &ss, &qs); 
                            //increase array counter
                            qs.node[qs_elt.start].count++;  /// printf("new count %d\n", qs.node[node_index].count);
                        }
                      //////  if(i==31) printf("after changing state is %d %d syn size %d query size %d\n", qs.node[qs_elt.start].query_state, qs.top_item, syntaxStackSize(&ss), qs.top_item);
                    }
                }
                else if(us.token_type==OBJECT)
                {
                    //////printf("{}\n");
                    val_obj_e(&qs_elt, &ss, &qs);
                }
                else if(us.token_type==ARRAY)
                {
                    //////printf("[]\n");
                    val_ary_e(&qs_elt, &ss, &qs); 
                }
            }
          ///  if(unit.num_root==-1) {printf("%d %d zero_num_root %d\n", i, j, ul.count_units); sleep(3);} 

            // find the matched paths inside the unit by using the current state
            int l = 0, tuple_index = -1;
            for(l = 0; l< unit.num_root; l++)
            {
                if(unit.root_node[l].query_state == qs.node[qs_elt.start].query_state){
                    tuple_index = unit.root_node[l].first_tuple_index;
                    break;
                }
            }
              //if(i==25) printf("real state %d %d top qs %d start %d end %d %d\n", qs.node[qs_elt.start].query_state, j, qs.top_item, unit.start-thread_info[i].input_stream, unit.end-thread_info[i].input_stream);
            
         //////   if(i==30&&j==1) printf("** %d state is %d real state %d num root %d unit num %d\n", j, unit.root_node[l].query_state, qs.node[qs_elt.start].query_state, unit.num_root, ul.count_units);
            //verification and reprocessing 
            //char* streaming = substring(thread_info[i].input_stream, 200, 200000);
            //executeWorkerAutomaton(wa, streaming);
            
            // runtime integration result for the current unit is wrong
            if((l==unit.num_root) && (unit.num_root>0) && (unit.unit_state!=UNIT_UNMATCHED))
            { 
//////printf("num root %d reprocessing %d %d real state %d %d values %d %d %d qs size %d unit root %d verified state %d %d start %d end %d start check %d\n", unit.num_root, i, j, qs.node[qs_elt.start].query_state, unit.num_root, unit.root_node[0].query_state, unit.root_node[1].query_state, unit.end-unit.start, qs.top_item, unit.num_root, unit.root_node[0].query_state, unit.root_node[1].query_state, unit.start-thread_info[i].input_stream, unit.end-thread_info[i].input_stream, (unit.start==NULL));
                //reprocess the current unit
                int length = unit.end-unit.start+1;
                int start_index = unit.start-thread_info[i].input_stream;
                char* streaming = NULL;
                if(unit.end>unit.start) streaming = substring(thread_info[i].input_stream, start_index, start_index+length-1);
                //if(length<2000)
             ///  if(i==63) 
//////////printf("syntax size %d reprocessing %d %d length %d real state %d start %d end %d %s topqs %d\n", syntaxStackSize(&ss), i, j, unit.end-unit.start, qs.node[qs_elt.start].query_state, start_index, start_index+length-1, streaming, qs.top_item);
                /*else if(!(i==15&&j==2)){
                    printf("%d %d first half %s second half %s\n", i, j, substring(streaming, 0, 1000), substring(streaming, strlen(streaming)-1000, strlen(streaming)));
                }*/
            //   else printf("start %d end %d\n", start_index, start_index+length-1);
                if(unit.end>unit.start && streaming!=NULL)
                {
                    seq_wa->syntax_stack = ss;
                    seq_wa->query_stacks = qs;
                    seq_wa->query_stacks_element = qs_elt;
                    seq_wa->need_reprocess = REPROCESS;
                    printf("start reprocessing the %dth unit in the %dth chunk\n", j, i);
                    executeWorkerAutomaton(seq_wa, streaming); 
                    printf("the %dth unit in the %dth chunk has been reprocessed\n", j, i);
///printf("new state1 is %d %d %d %d %d\n", seq_wa->query_stacks.node[qs_elt.start].query_state, qs.top_item, qs_elt.start, seq_wa->query_stacks_element.start, 1);
                    free(streaming);
                    ss = seq_wa->syntax_stack;
                    qs = seq_wa->query_stacks;
                    qs_elt = seq_wa->query_stacks_element;   
                    if(j==ul.count_units) merge_needed = 0;
                    //////printf("new state is %d %d %d %d %d\n", seq_wa->query_stacks.node[qs_elt.start].query_state, qs.top_item, qs_elt.start, seq_wa->query_stacks_element.start, 1);
                    continue;
                }
               // printf("state %d %d\m", seq->query_stacks[ 
            }
          /*  if(i==1) printf(" unit root i %d j %d %d %d %d\n", i, j, unit.num_root, unit.root_node[0].query_state, unit.root_node[1].query_state);
            if(l<unit.num_root) printf("%d %d %d\n", i, j, unit.root_node[l].query_state);
            else printf("error %d %d real %d in fact \n", i, j, qs.node[qs_elt.start].query_state);*/
           // int tuple_index = unit.root_node[qs.node[qs_elt.start].query_state].first_tuple_index;
            ///int last_tuple_index = unit.root_node[qs.node[qs_elt.start].query_state].last_tuple_index;
            //sum = 0;
            //check array indexes
            /*if(syntaxStackTop(&ss)==LB)
            {
                c_node = qs.node[qs_elt.start];
                JSONQueryIndexPair pair = getDFAArrayIndex(qa, c_node.query_state);
                int lower = pair.lower; 
                int upper = pair.upper;
                int counter = qs_elt->count;
                //check array indexes
                if((lower==0&&upper==0)||(counter>=lower && counter<upper))
                {
                    
                }
            }*/

            //collect correct 2-tuple lists for current unit
            while(tuple_index!=-1)  
            {
                Tuple tuple = getTuple(tl, tuple_index); 
/////printf("!!!!!!() start %d end %d state %d start node index %d end node index %d\n", tuple.start_position, tuple.end_position, qs.node[qs_elt.start].query_state, qs_elt.start, qs_elt.end);
                //printf("tuple index %d next index %d\n", tuple_index, tuple.next_index);
                tuple_index = tuple.next_index;
                int query_state = qs.node[qs_elt.start].query_state;
                if(query_state == 0) continue;
                JSONQueryIndexPair pair = getDFAArrayIndex(qa, query_state);
                int lower = pair.lower; 
                int upper = pair.upper; 
                int counter = qs.node[qs_elt.start].count; 
             /////   printf("tuple text %s\n",tuple.text);
//////        if(i==1&&j>=1)        
         ///    printf("text %s unit %d %d top_s %d counter %d %d state %d low %d high %d\n", tuple.text, i, j, syntaxStackTop(&ss), qs.node[qs_elt.start].count, getTupleListSize(tuple_list), query_state, lower, upper);
                
                //check array indexes
                if(!(lower==0&&upper==0))
                {
                    if(strcmp(tuple.text, "}")==0||strcmp(tuple.text, "]")==0) { 
                        qs.node[qs_elt.start].count++; 
                        if(qs.node[qs_elt.start].count>=upper) qs.node[qs_elt.start].query_state = 0; 
                        continue;
                    }
                    else if(!(counter>=lower && counter<upper)) { //printf("should be 0\n");
                        if(qs.node[qs_elt.start].count>=upper) qs.node[qs_elt.start].query_state = 0;
             ///           printf("skip %s counter %d lower %d upper %d %d\n", tuple.text, counter, lower, upper, getTupleListSize(tuple_list)); 
                        continue;
                    }
                    else
                    {
                        int matched_type = getMatchedType(qa, &qs.node[qs_elt.start]);
                        if(matched_type==DFA_OUTPUT_CANDIDATE) qs.node[qs_elt.start].count++;
                    }
                }    
                // add corresponding tuple into the tuple list 
                if(tuple.start_position==-1)
                    addTuple(tuple_list, tuple.state, tuple.text);
                else
                {   /////printf("!!!!!! start %d end %d\n", tuple.start_position, tuple.end_position);
                    substring_in_place(tuple.text, thread_info[i].input_stream, tuple.start_position, tuple.end_position); /////printf("changed tuple text %s\n", tuple.text);
                    addTuple(tuple_list, tuple.state, tuple.text); //////printf("!!!!!! start %d end %d %s\n", tuple.start_position, tuple.end_position, tuple.text);
                }
     /////           if(wa->id==2) printf("text tuple %s\n", tuple.text);
                //printf("%d\n", first_tuple_index);
                sum++;
            }
          ///  if(i==29&&j==1) printf("** %d state is %d unit %d real state %d\n", j, unit.root_node[l].query_state, ul.count_units, qs.node[qs_elt.start].query_state);
          //  qs.node[qs_elt.start].query_state = unit.root_node[l].query_state;
           ////// printf("query state %d first output %d last output %d output %d\n", qs.node[qs_elt.start].query_state, tuple_index, last_tuple_index, sum);
            /*if(i==1&&1==2)
            {
                int x;
                for(x=0;x<=8;x++)
                {
                    if(x==4) continue;
                    int first_tuple_index = unit.root_node[x].first_tuple_index;
                    last_tuple_index = unit.root_node[x].last_tuple_index;
                    int sum = 0;
                    while(first_tuple_index<=last_tuple_index&&first_tuple_index!=-1)
                    {
                        Tuple tuple = getTuple(tl, first_tuple_index);
                        first_tuple_index = tuple.next_index;
                        //printf("%d\n", first_tuple_index);
                        sum++;
                    }
                    printf("query state %d first output %d last output %d output %d\n", x, first_tuple_index, last_tuple_index, sum);
                }
            }*/
     //////       printf("syntax size %d query size %d\n", syntaxStackSize(&ss), qs.top_item);
          //////  if(i==30) printf("%d state is %d root %d\n", j, qs.node[qs_elt.start].query_state, unit.root_node[0].query_state);
        }
      // if(i==30) printf("final ** state is %d root %d\n", qs.node[qs_elt.start].query_state, unit.root_node[0].query_state);
        //merge stack  
        if(merge_needed==1)
        {
            combineSyntaxStack(&ss, &wa->syntax_stack); ///printf("before thread %d seg %d finish merging syntax size %d query size %d wa %d wa nodes %d start is %d current state is %d %d %d %d\n", i, j, syntaxStackSize(&ss), qs.top_item, wa->query_stacks.top_item, wa->query_stacks.num_node, wa->query_stacks.node[0].query_state, qs.node[qs_elt.start].query_state, wa->query_stacks.node[1].query_state, wa->query_stacks.node[2].query_state, wa->query_stacks.node[1].root_range.start,wa->query_stacks.node[1].root_range.end);
            combineQueryStacks(&qs, &qs_elt, &wa->query_stacks, &wa->query_stacks_element);  //merge stack and update qs_elt 
            
        //    printf("thread %d seg %d finish merging syntax size %d query size %d wa %d wa nodes %d start is %d current state is %d %d %d %d\n", i, j, syntaxStackSize(&ss), qs.top_item, wa->query_stacks.top_item, wa->query_stacks.num_node, wa->query_stacks.node[0].query_state, qs.node[qs_elt.start].query_state, wa->query_stacks.node[1].query_state, wa->query_stacks.node[2].query_state, wa->query_stacks.node[1].root_range.start,wa->query_stacks.node[1].root_range.end);  if(i==30) sleep(10);
            
            
        }
        ///printf("after %dth thread syntax size %d query size %d state %d\n", i, syntaxStackSize(&ss), qs.top_item, qs.node[qs_elt.start]);
    }
    printf("syntax stack size %d query stack size %d\n", syntaxStackSize(&ss), qs.top_item+1);
    printf("size of 2-tuple list before filtering %d\n", getTupleListSize(tuple_list));
    /*for(i = 0; i<getTupleListSize(tuple_list); i++)
    {
        Tuple tuple = getTuple(tuple_list, i);
        printf("%dth state %d text %s\n", i, tuple.state, tuple.text);
    }*/
    return tuple_list;
}

//par_info -- partitioned input stream, qa -- query automaton
TupleList* executeParallelAutomata(PartitionInfo par_info, JSONQueryDFA* qa, int num_cores, int warmup_cpu, ConstraintTable* ct) 
{
    ThreadInfo ti[MAX_THREAD]; 
    int i;
    for(i = 0; i<num_cores; i++)
    {
        //initialize each thread
        ti[i].thread_id = i;
        ti[i].input_stream = par_info.stream[i];
        ti[i].cpu_warmup = warmup_cpu;
        ti[i].worker_automaton = createWorkerAutomaton(qa, i, ct);
        int rc=pthread_create(&ti[i].thread, NULL, main_thread, &ti[i]);  
    	if (rc)
        {
            printf("ERROR; return code is %d\n", rc);
            return 0;
        }
    }
    for (i = 0; i <num_cores; i++)
        pthread_join(ti[i].thread, NULL);
    printf("worker automata: finish parallel query processing\n");
    TupleList* tl = combine(ti, num_cores);
    printf("combiner: finish merging 2-tuple lists\n");
    for (i = 0; i <num_cores; i++)
    {
        freeWorkerAutomaton(ti[i].worker_automaton); 
    } 
    return tl;
}

#endif // !__PARALLEL_AUTOMATA_EXECUTION_H__
