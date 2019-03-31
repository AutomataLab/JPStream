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

//void *main_thread(void *arg);

//void executeParallelAutomata(PartitionInfo par_info, JSONQueryDFA* qa, int num_cores, int warmup_cpu);

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
    //printf("\n%dth chunk is start: %s end: %s\n", t_id, substring(ti->input_stream,0,100), substring(ti->input_stream, strlen(ti->input_stream)-100, strlen(ti->input_stream)));
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

TupleList* combine(ThreadInfo* thread_info, int num_thread)
{
    int i,j,k;
    //get some information for thread 0
    SyntaxStack ss = thread_info[0].worker_automaton->syntax_stack;
    QueryStacks qs = thread_info[0].worker_automaton->query_stacks;   
    QueryStacksElement qs_elt = thread_info[0].worker_automaton->query_stacks_element;
    QueryStatesInfo* qs_info = thread_info[0].worker_automaton->query_states_info;
    UnitList unit_list = thread_info[0].worker_automaton->unit_list;
    TupleList* tuple_list = thread_info[0].worker_automaton->tuple_list; 
    //query automaton
    JSONQueryDFA* qa = thread_info[0].worker_automaton->query_automaton;   
    //save information for current unmatched symbol
    UnmatchedSymbol us;

   /////////// printf("syntax size %d query size %d query_state %d\n", syntaxStackSize(&ss), qs.top_item, qs.node[qs_elt.start].query_state);
    int sum = getTupleListSize(tuple_list); int sum0 = sum;
    for(int i = 1; i<num_thread; i++)
    {
        WorkerAutomaton* wa = thread_info[i].worker_automaton;
        UnitList ul = wa->unit_list; 
        TupleList* tl = wa->tuple_list;  
        //////////printf("thread %d count units %d\n", i, ul.count_units);  
        for(int j = 0; j<=ul.count_units; j++)
        {
            //get current node in qs
            int node_index = qs_elt.start;
            TreeNode c_node = qs.node[node_index];
            int query_state = c_node.query_state;
            Unit unit = ul.units[j];
            //handle unmatched tokens
            int count_unmatched_symbols = unit.count_unmatched_symbols;
            /////////////printf("unmatched symbols %d\n", count_unmatched_symbols);
            for(int k = 0; k<=count_unmatched_symbols; k++)
            {
                node_index = qs_elt.start;
                c_node = qs.node[node_index];
                query_state = c_node.query_state; 
                us = unit.unmatched_symbols[k];
                if(us.token_type==RCB)
                {
                    //////printf("}\n");
                    //////if(i==25) printf("} unit %d\n", j);
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
                                substring1(object_text, thread_info[i-1].input_stream, c_node.matched_start, c_node.matched_start+left_len);
                                //printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!thread %d %d %s %d %d %s\n", i, c_node.matched_start, object_text, c_node.matched_start+left_len, us.index, substring(thread_info[i].input_stream, 0, us.index+1));
                                substring1(object_text+left_len, thread_info[i].input_stream, 0, us.index+1);
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
                        {
                            val_obj_e(&qs_elt, &ss, &qs);
                        }
                        //after popping out '{', the next element on top of syntax stack is '['
                        else if(syntaxStackSecondTop(&ss)==LB)
                        {
                            elt_obj_e(&ss);
                            //increase array counter
                            qs.node[node_index].count++;  ///////////////if(i==2) printf("******increase counter %d state %d\n", qs.node[node_index].count, qs.node[node_index].query_state);
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
                    //if(i==25) printf("] %d %d\n", j, c_node.query_state));
                    //get candidate arrays across different chunks  -- todo list
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
                                substring1(object_text, thread_info[i-1].input_stream, c_node.matched_start, c_node.matched_start+left_len);
                     //////           printf("unit %d !!!!!!!!!!!!!!!!!!!!!!!!!!!thread %d %d %s %d %d %s\n", j, i, c_node.matched_start, object_text, c_node.matched_start+left_len, us.index, substring(thread_info[i].input_stream, 0, us.index+1));
                                substring1(object_text+left_len, thread_info[i].input_stream, 0, us.index+1);
                                addTupleInfo(&qs, node_index, c_node.query_state, object_text, tuple_list);
                                qs.node[node_index].matched_start = INVALID;
                      ///          printf("unit %d !!!!!!!!!!!!!!!!!!!!!!!!!!!thread %d %d %s len %d\n", j, i, c_node.matched_start, object_text, strlen); sleep(1);
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
                            qs.node[node_index].count++;
                        }
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
            //collect correct 2-tuple lists for current unit   -- todo list
            int tuple_index = unit.root_node[qs.node[qs_elt.start].query_state].first_tuple_index;
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
            while(tuple_index!=-1)  //first_tuple_index<=last_tuple_index&&
            {
                Tuple tuple = getTuple(tl, tuple_index); /////printf("!!!!!!() start %d end %d state %d start node index %d end node index %d\n", tuple.start_position, tuple.end_position, qs.node[qs_elt.start].query_state, qs_elt.start, qs_elt.end);
                //printf("tuple index %d next index %d\n", tuple_index, tuple.next_index);
                tuple_index = tuple.next_index;
                int query_state = qs.node[qs_elt.start].query_state;
                if(query_state == 0) continue;
                JSONQueryIndexPair pair = getDFAArrayIndex(qa, query_state);
                int lower = pair.lower; 
                int upper = pair.upper; 
                int counter = qs.node[qs_elt.start].count; 
        //////        printf("text %s unit %d %d top_s %d counter %d %d state %d low %d high %d\n", tuple.text, i, j, syntaxStackTop(&ss), qs.node[qs_elt.start].count, getTupleListSize(tuple_list), query_state, lower, upper);
                
                //check array indexes
                if(!(lower==0&&upper==0))
                {
                    if(strcmp(tuple.text, "}")==0||strcmp(tuple.text, "]")==0) { 
                        qs.node[qs_elt.start].count++; 
                        if(qs.node[qs_elt.start].count>=upper) qs.node[qs_elt.start].query_state = 0; 
                        continue;
                    }
                    else if(!(counter>=lower && counter<upper)) { 
                        qs.node[qs_elt.start].query_state = 0;
         ///////               printf("skip %s counter %d lower %d upper %d %d\n", tuple.text, counter, lower, upper, getTupleListSize(tuple_list)); 
                        continue;
                    }
                    else
                    {
                        int matched_type = getMatchedType(qa, &qs.node[qs_elt.start]);
                        if(matched_type==DFA_OUTPUT_CANDIDATE) qs.node[qs_elt.start].count++;
                    }
                }    
                
                if(tuple.start_position==-1)
                    addTuple(tuple_list, tuple.state, tuple.text);
                else
                {   /////printf("!!!!!! start %d end %d\n", tuple.start_position, tuple.end_position);
                    substring1(tuple.text, thread_info[i].input_stream, tuple.start_position, tuple.end_position);
                    addTuple(tuple_list, tuple.state, tuple.text); //////printf("!!!!!! start %d end %d %s\n", tuple.start_position, tuple.end_position, tuple.text);
                }
     /////           if(wa->id==2) printf("text tuple %s\n", tuple.text);
                //printf("%d\n", first_tuple_index);
                sum++;
            }
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
        }
        //merge stack  
        combineSyntaxStack(&ss, &wa->syntax_stack);
        combineQueryStacks(&qs, &qs_elt, &wa->query_stacks, &wa->query_stacks_element);  //merge stack and update qs_elt  -- todo list
    }
    printf("syntax size %d query size %d\n", syntaxStackSize(&ss), qs.top_item);
    printf("output size %d %d\n", sum, getTupleListSize(tuple_list));
    /*for(i = 0; i<getTupleListSize(tuple_list); i++)
    {
        Tuple tuple = getTuple(tuple_list, i);
        printf("%dth state %d text %s\n", i, tuple.state, tuple.text);
    }*/
    return tuple_list;
}

//par_info -- partitioned input stream, qa -- query automaton
TupleList* executeParallelAutomata(PartitionInfo par_info, JSONQueryDFA* qa, int num_cores, int warmup_cpu) //JSONQueryDFA* qa,
{
    ThreadInfo ti[MAX_THREAD]; 
    int i;
    for(i = 0; i<num_cores; i++)
    {
        //initialize each thread
        ti[i].thread_id = i;
        ti[i].input_stream = par_info.stream[i];
        ti[i].cpu_warmup = warmup_cpu;
        ti[i].worker_automaton = createWorkerAutomaton(qa, i);
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
        //free(ti[i].input_stream); printf("%d free\n", i);
        freeWorkerAutomaton(ti[i].worker_automaton); //printf("%d free\n", i);
    }
    printf("all free\n");
    return tl;
}

#endif // !__PARALLEL_AUTOMATA_EXECUTION_H__
