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

#define MAX_THREAD 100
#define INPROGRESS 0
#define FINISH 1

//data structure for each thread
typedef struct ThreadInfo{
    pthread_t thread;
    int thread_id;
    char* input_stream;
    double execution_time;
    WorkerAutomaton* worker_automaton;
}ThreadInfo;

//data structure for parallel automata
typedef struct ParallelAutomata{
    JSONQueryDFA* query_automaton;
    SyntaxStack syntax_stack;
    QueryStacks query_stacks; 
    TupleList* tuple_list;
    WorkerAutomaton* major_automaton;
    int finish_flag;
}ParallelAutomata;

static inline void initParallelAutomata(ParallelAutomata* pa, JSONQueryDFA* qa)
{
    pa->query_automaton = qa;
    pa->tuple_list = NULL;
    pa->major_automaton = NULL;
}

static inline void destroyParallelAutomata(ParallelAutomata* pa)
{
    if(pa->tuple_list != NULL&&pa->finish_flag==INPROGRESS)
    {   
        freeTupleList(pa->tuple_list); 
        pa->tuple_list = NULL;
    }
    if(pa->major_automaton != NULL)
    {  
        if(pa->finish_flag==INPROGRESS)
        {
            pa->major_automaton->tuple_list = createTupleList();
        }
        else if(pa->finish_flag==FINISH)
        {   
            destroyWorkerAutomaton(pa->major_automaton);  
        }
    }
}

//main function for each thread
void *main_thread(void *arg)
{
    struct timeval start_timestamp, end_timestamp;
    double exe_timestamp;
    ThreadInfo* ti = (ThreadInfo*)arg;
    int t_id = ti->thread_id;
    // bind CPU
    /*cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(t_id, &mask);
    if(pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask)<0)
        printf("CPU combination failed for thread %d\n", t_id);*/
    // CPU warmup
    /*gettimeofday(&start_timestamp,NULL);
    while(1)
    {
        gettimeofday(&end_timestamp,NULL);
        exe_timestamp=1000000*(end_timestamp.tv_sec-start_timestamp.tv_sec)+end_timestamp.tv_usec-start_timestamp.tv_usec;
        if(exe_timestamp>3000000) break;
    }*/

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
    int sum = getTupleListSize(tuple_list); 

    for(int i = 1; i<num_thread; i++)
    {
        WorkerAutomaton* wa = thread_info[i].worker_automaton;
        UnitList ul = wa->unit_list; 
        TupleList* tl = wa->tuple_list;  
        //whether stacks for the current chunk needs to be merged after processing
        int merge_needed = 1; 
        for(int j = 0; j<=ul.count_units; j++)
        { 
            //get current state information from query stack qs
            int node_index = qs_elt.start;
            TreeNode c_node = qs.node[node_index];
            int query_state = c_node.query_state;  
            Unit unit = ul.units[j];
            int count_unmatched_symbols = unit.count_unmatched_symbols;
            //handle unmatched tokens
            for(int k = 0; k<=count_unmatched_symbols; k++)
            {
                node_index = qs_elt.start;
                c_node = qs.node[node_index];
                query_state = c_node.query_state; 
                us = unit.unmatched_symbols[k];
                if(us.token_type==RCB)
                {
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
                                substring_in_place(object_text+left_len, thread_info[i].input_stream, 0, us.index+1);
                                addTupleInfo(&qs, node_index, c_node.query_state, object_text, tuple_list);
                                qs.node[node_index].matched_start = INVALID;
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
                        {   val_obj_e(&qs_elt, &ss, &qs);
                        }
                        //after popping out '{', the next element on top of syntax stack is '['
                        else if(syntaxStackSecondTop(&ss)==LB)
                        {
                            elt_obj_e(&ss);
                            //increase array counter 
                            qs.node[qs_elt.start].count++;  
                            int matched_type = getMatchedType(qa,&c_node);
                            if(matched_type==DFA_PREDICATE)
                            {
                                sum++; 
                                addTupleInfo(&qs, node_index, c_node.query_state, "}", tuple_list);
                            }
                        }
                    }
                }
                else if(us.token_type==RB) 
                {
                    //get candidate arrays across different chunks  
                    if(syntaxStackSize(&ss) > 0)
                    {
                        QueryStacksElement top_qs_elt = queryStacksTop(&qs);
                        c_node = qs.node[top_qs_elt.start]; 
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
                                substring_in_place(object_text+left_len, thread_info[i].input_stream, 0, us.index+1);
                                addTupleInfo(&qs, node_index, c_node.query_state, object_text, tuple_list);
                                qs.node[node_index].matched_start = INVALID;
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
                            val_ary_e(&qs_elt, &ss, &qs); 
                        }
                        //after popping out '[', the next element on top of syntax stack is '['
                        else if(syntaxStackSecondTop(&ss)==LB)  
                        {  
                            elt_ary_e(&qs_elt, &ss, &qs); 
                            //increase array counter
                            qs.node[qs_elt.start].count++;  
                        }
                    }
                }
                else if(us.token_type==OBJECT || us.token_type==ARRAY)
                {
                    qs_elt = queryStacksPop(&qs);
                    syntaxStackPop(&ss);
                }
            }

            // find the matched paths inside the unit by using the current state
            int l = 0, tuple_index = -1;
            for(l = 0; l< unit.num_root; l++)
            {
                if(unit.root_node[l].query_state == qs.node[qs_elt.start].query_state){
                    tuple_index = unit.root_node[l].first_tuple_index;
                    break;
                }
            }
        
            //verification and reprocessing             
            //runtime integration result for the current unit is wrong
            if((l==unit.num_root) && (unit.num_root>0) && (unit.unit_state!=UNIT_UNMATCHED))
            { 
                //reprocess the current unit
                int length = unit.end-unit.start+1;
                int start_index = unit.start-thread_info[i].input_stream;
                char* streaming = NULL;
                if(unit.end>unit.start) streaming = substring(thread_info[i].input_stream, start_index, start_index+length-1);
                if(unit.end>unit.start && streaming!=NULL)
                {
                    seq_wa->syntax_stack = ss;
                    seq_wa->query_stacks = qs;
                    seq_wa->query_stacks_element = qs_elt;
                    seq_wa->need_reprocess = REPROCESS;
                    printf("start reprocessing the %dth unit in the %dth chunk\n", j, i);
                    executeWorkerAutomaton(seq_wa, streaming); 
                    printf("the %dth unit in the %dth chunk has been reprocessed successfully\n", j, i);
                    free(streaming);
                    ss = seq_wa->syntax_stack;
                    qs = seq_wa->query_stacks;
                    qs_elt = seq_wa->query_stacks_element;   
                    if(j==ul.count_units) merge_needed = 0;
                    continue;
                }
            }

            //collect correct 2-tuple lists for current unit
            while(tuple_index!=-1)  
            {
                Tuple tuple = getTuple(tl, tuple_index); 
                tuple_index = tuple.next_index;
                int query_state = qs.node[qs_elt.start].query_state;
                if(query_state == 0) continue;
                JSONQueryIndexPair pair = getDFAArrayIndex(qa, query_state);
                int lower = pair.lower; 
                int upper = pair.upper; 
                int counter = qs.node[qs_elt.start].count; 
                //check array indexes
                if(!(lower==0&&upper==0))
                {
                    if(strcmp(tuple.text, "}")==0||strcmp(tuple.text, "]")==0) { 
                        qs.node[qs_elt.start].count++; 
                        if(qs.node[qs_elt.start].count>=upper) qs.node[qs_elt.start].query_state = 0; 
                        continue;
                    }
                    else if(!(counter>=lower && counter<upper)) { 
                        if(qs.node[qs_elt.start].count>=upper) qs.node[qs_elt.start].query_state = 0;
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
                {   
                    substring_in_place(tuple.text, thread_info[i].input_stream, tuple.start_position, tuple.end_position); 
                    addTuple(tuple_list, tuple.state, tuple.text); 
                }
                sum++;
            }
        }
        //merge stack  
        if(merge_needed==1)
        {
            combineSyntaxStack(&ss, &wa->syntax_stack); 
            combineQueryStacks(&qs, &qs_elt, &wa->query_stacks, &wa->query_stacks_element);  //merge query stacks
        }
    }
    seq_wa->syntax_stack = ss;
    seq_wa->query_stacks = qs;
    seq_wa->query_stacks_element = qs_elt;
    //printf("syntax stack size %d query stack size %d\n", syntaxStackSize(&ss), qs.top_item+1);
    //printf("size of 2-tuple list before filtering %d\n", getTupleListSize(tuple_list));
    return tuple_list;
}

//par_info -- partitioned input stream, qa -- query automaton
void executeParallelAutomata(ParallelAutomata* pa, PartitionInfo par_info, ConstraintTable* ct)  
{
    int num_cores = par_info.num_chunk;
    JSONQueryDFA* qa = pa->query_automaton;
    ThreadInfo ti[MAX_THREAD]; 
    int i;
    for(i = 0; i<num_cores; i++)
    {
        //initialize each thread
        ti[i].thread_id = i;
        ti[i].input_stream = par_info.stream[i];
        if((i==0)&&(pa->major_automaton!=NULL))  ti[i].worker_automaton = pa->major_automaton;  
        else 
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
    pa->syntax_stack = ti[0].worker_automaton->syntax_stack;
    pa->query_stacks = ti[0].worker_automaton->query_stacks;
    pa->tuple_list = tl;
    pa->major_automaton = ti[0].worker_automaton;
    for (i = 1; i <num_cores; i++)
    {
        freeWorkerAutomaton(ti[i].worker_automaton); 
    }
}

#endif // !__PARALLEL_AUTOMATA_EXECUTION_H__
