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

//par_info -- partitioned input stream, qa -- query automaton
void executeParallelAutomata(PartitionInfo par_info, JSONQueryDFA* qa, int num_cores, int warmup_cpu) 
{
    ThreadInfo ti[MAX_THREAD]; 
    int i;
    for(i = 0; i<num_cores; i++)
    {
        //initialize each thread
        ti[i].thread_id = i;
        ti[i].input_stream = par_info.stream[i];
        ti[i].cpu_warmup = warmup_cpu;
        ti[i].worker_automaton = createWorkerAutomaton(qa);
        int rc=pthread_create(&ti[i].thread, NULL, main_thread, &ti[i]);  
    	if (rc)
        {
            printf("ERROR; return code is %d\n", rc);
            return 0;
        }
    }
    for (i = 0; i <num_cores; i++)
        pthread_join(ti[i].thread, NULL);
    printf("done\n");
}

#endif // !__PARALLEL_AUTOMATA_EXECUTION_H__
