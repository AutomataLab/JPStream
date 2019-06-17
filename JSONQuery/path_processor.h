#ifndef __PATH_PROCESSOR_H__
#define __PATH_PROCESSOR_H__

#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/file.h>
#include "streaming_automaton.h"
#include "predicate.h"
#include "constraint.h"
#include "unit.h"
#include "worker_automaton.h"
#include "file_partition.h"
#include "parallel_automata_execution.h"

#define MAX_EXTENSION 20000
#define CONTEXT 1
#define NOCONTEXT 0
#define FINISH 1
#define INPROGRESS 0
#define MEMORY_FOOTPRINT 250000000

char* loadInputStream(char* file_name)
{
    FILE *fp; 
    int size;
    fp = fopen(file_name,"rb");
    if (fp==NULL) { return NULL;}
    fseek (fp, 0, SEEK_END);
    size=ftell(fp);
    rewind(fp); 
    char* stream =(char*)malloc((size+1)*sizeof(char));
    fread(stream,1,size,fp); 
    stream[size]='\0';
    fclose(fp);
    return stream;
}

//load the next available input stream with bounded memory footprint
char* loadBoundedInputStream(char* file_name, long* start_pos)
{
    FILE *fp;
    long size;
    fp = fopen(file_name,"rb");
    if (fp==NULL) { return NULL;}
    fseek (fp, 0, SEEK_END);
    size=ftell(fp);
    rewind(fp);

    long loaded_size = *(start_pos);
    size = size - loaded_size;
    if(size > MEMORY_FOOTPRINT)
        size = MEMORY_FOOTPRINT;
    if(size<=0) return NULL;    
    fseek(fp, loaded_size, SEEK_CUR); 
    char* stream =(char*)malloc((size+MAX_EXTENSION)*sizeof(char)); 
    fread(stream,1,size,fp);
    long add = 0;
    stream[size+add] = '\0'; 
    //look for the next complete token
    char ch = fgetc(fp);
    while(1)
    {    
        if(ch==EOF) break;
        if(ch=='"')
        {
            char prev = stream[size+add-1];
            int t = 1;
            while(prev==' '||prev=='\t'||prev=='\n'||prev==13)
            {
                prev = stream[size+add-t]; 
                t++;
            }
            if(prev==','||prev=='{'||prev=='[')
            {
                break;
            }
        }
        stream[size+add] = ch; 
        add = add+1;
        ch = fgetc(fp);
    }
    stream[size+add]='\0';
    long next = loaded_size+size+add;
    *(start_pos) = next;
    fclose(fp);
    return stream;
}

//split input stream into several chunks
PartitionInfo partitionInputStream(char* input_stream, int num_core)
{
    PartitionInfo pInfo;
    pInfo.num_chunk = 0;
    char** stream = NULL;
    int* start_pos = NULL;
    long stream_size, chunk_size;
    stream_size = strlen(input_stream);
    chunk_size = (stream_size/num_core)+1;
    stream = (char**)malloc(num_core*sizeof(char*));
    start_pos = (int*)malloc(num_core*sizeof(int));
    int i;
    for(i = 0; i<num_core; i++)
        stream[i] = NULL;
    long sum_size = 0;   //the number of bytes that have been processed
    char ch = -1;
    for(i = 0; i<num_core-1; i++)
    {   
        start_pos[i] = sum_size;
        stream[i] = (char*)malloc((chunk_size+MAX_EXTENSION)*sizeof(char)); 
        substring_in_place(stream[i], input_stream, sum_size, sum_size+chunk_size);
        sum_size += chunk_size; 
        int add = 0;
        //look for the next complete token
        while(sum_size<=stream_size)
        {    
            ch = input_stream[sum_size+add];
            if(ch=='"')
            {
                char prev = stream[i][chunk_size+add-1];
                int t = 1;
                while(prev==' '||prev=='\t'||prev=='\n'||prev==13)
                {
                    prev = stream[i][chunk_size+add-t]; 
                    t++;
                }
                if(prev==','||prev=='{'||prev=='[')
                {
                    stream[i][chunk_size+add] = ch;
                    break;
                }
            }
            stream[i][chunk_size+add] = ch; 
            add = add+1;
        }
        stream[i][chunk_size+add] = '\0'; 
        sum_size += add;  
        if((sum_size+chunk_size)>=stream_size) {i++; break;}
    } 
    pInfo.num_chunk = i; 
    start_pos[i] = sum_size;
    long remain_size = stream_size-sum_size;
    if(remain_size>0)
    {
        stream[i] = substring(input_stream, sum_size, stream_size);
        pInfo.num_chunk = i+1;
    }
    pInfo.num_chunk = i+1;
    pInfo.stream = stream;
    pInfo.start_pos = start_pos;
    printf("stream partitioner: finish splitting the input stream\n");
    return pInfo;
}
typedef struct PathProcessor{
    JSONQueryDFAContext* query_context; 
    JSONQueryDFA* query_automaton;
}PathProcessor;

static inline PathProcessor* createPathProcessor(char* path)
{
     JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext)); 
     JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
     if (dfa == NULL) return NULL;
     PathProcessor* path_processor = (PathProcessor*)malloc(sizeof(PathProcessor));
     path_processor->query_context = ctx;
     path_processor->query_automaton = dfa;
     return path_processor;
}

static inline void freePathProcessor(PathProcessor* path_processor)
{
     destoryJSONQueryDFA(path_processor->query_automaton); 
     free(path_processor);
}

typedef struct StreamingContext{
    StreamingAutomaton sa;
    ParallelAutomata pa;
    PredicateFilter pf;
    int context_flag;
    int finish_flag;
}StreamingContext;

static inline void initStreamingContext(StreamingContext* ci)
{
    ci->context_flag = NOCONTEXT;
    ci->finish_flag = INPROGRESS;
}

static inline void destroyStreamingContext(StreamingContext* ci)
{
    ci->pa.finish_flag = FINISH;
    destroyParallelAutomata(&(ci->pa));
    ci->sa.finish_flag = FINISH; 
    destroyStreamingAutomaton(&(ci->sa)); 
    destroyPredicateFilter(&(ci->pf));
}

// when input size might exceed the memory limit
static inline Output* serialPartialRun(PathProcessor* path_processor, char* input_stream, StreamingContext* ci)
{
    JSONQueryDFAContext* ctx = path_processor->query_context;
    JSONQueryDFA* dfa = path_processor->query_automaton;
    int cflag = ci->context_flag;
    int fflag = ci->finish_flag;
    //create streaming automaton
    StreamingAutomaton streaming_automaton;
    if(cflag == CONTEXT) streaming_automaton = ci->sa;
    else initStreamingAutomaton(&streaming_automaton, dfa);

    //run streaming automaton
    executeAutomaton(&streaming_automaton, input_stream, CLOSE);
    ci->sa = streaming_automaton;

    //predicate filtering
    PredicateFilter pf;
    if(cflag == CONTEXT) { pf = ci->pf; pf.tuple_list = streaming_automaton.tuple_list; pf.input_stream = input_stream; }
    else initPredicateFilter(&pf, streaming_automaton.tuple_list, ctx, input_stream);
    Output* output = generateFinalOutput(&pf);
    ci->pf = pf;

    //free up dynamic memories
    destroyStreamingAutomaton(&streaming_automaton);

    //set context flag
    ci->context_flag = CONTEXT;
    return output;
}

static inline Output* serialRun(PathProcessor* path_processor, char* file_name)
{
    StreamingContext ci; 
    initStreamingContext(&ci);
    char* input_stream = NULL;
    Output* output = NULL;
    long start_pos = 0;  //pointer to the starting position of the next available input chunk
    while(1)
    {
        input_stream = loadBoundedInputStream(file_name, &start_pos);
        if(input_stream == NULL) break;
        output = serialPartialRun(path_processor, input_stream, &ci);
        free(input_stream);
    }
    destroyStreamingContext(&ci);
    return output;
}

static inline ConstraintTable* collectDataConstraints(PathProcessor* path_processor, char* file_name)
{
    char* input_stream = loadInputStream(file_name);

    JSONQueryDFAContext* ctx = path_processor->query_context;
    JSONQueryDFA* dfa = path_processor->query_automaton;

    //create streaming automaton
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);

    //run streaming automaton
    executeAutomaton(&streaming_automaton, input_stream, OPEN);

    //free up dynamic memories
    destroyStreamingAutomaton(&streaming_automaton);
    free(input_stream);
    return streaming_automaton.constraint_table;
}

// when input size might exceed the memory limit
static inline Output* parallelPartialRun(PathProcessor* path_processor, char* input_stream, int num_core, StreamingContext* ci)
{   
    PartitionInfo pInfo = partitionInputStream(input_stream, num_core); 
    JSONQueryDFAContext* ctx = path_processor->query_context;
    JSONQueryDFA* dfa = path_processor->query_automaton;
    int cflag = ci->context_flag;
    int fflag = ci->finish_flag;

    //create parallel streaming automata
    ParallelAutomata pa;
    if(cflag == CONTEXT) pa = ci->pa;
    else initParallelAutomata(&pa, dfa);

    //execute parallel streaming automata
    executeParallelAutomata(&pa, pInfo, NULL);
    TupleList* tl = pa.tuple_list;
    ci->pa = pa;

    //predicate filtering
    PredicateFilter pf;
    if(cflag == CONTEXT) { pf = ci->pf; pf.tuple_list = tl; pf.input_stream = input_stream;}
    else initPredicateFilter(&pf, tl, ctx, input_stream);
    Output* output = generateFinalOutput(&pf);
    ci->pf = pf;
    printf("finish predicate filtering\n");
    //free up dynamic memories
    freeInputChunks(pInfo);
    destroyParallelAutomata(&pa);
    ci->context_flag = CONTEXT; 
    return output;
}

static inline Output* parallelRun(PathProcessor* path_processor, char* file_name, int num_core)
{
    StreamingContext ci; 
    initStreamingContext(&ci);
    char* input_stream = NULL;
    Output* output = NULL;
    long start_pos = 0; //pointer to the starting position of the next available input chunk
    while(1)
    {
        input_stream = loadBoundedInputStream(file_name, &start_pos);
        if(input_stream == NULL) break;
        output = parallelPartialRun(path_processor, input_stream, num_core, &ci);
        free(input_stream);
    }
    destroyStreamingContext(&ci);
    return output;
}

// when input size might exceed the memory limit
static inline Output* parallelPartialRunOpt(PathProcessor* path_processor, char* input_stream, int num_core, ConstraintTable* ct, StreamingContext* ci)
{   
    PartitionInfo pInfo = partitionInputStream(input_stream, num_core); 
    JSONQueryDFAContext* ctx = path_processor->query_context;
    JSONQueryDFA* dfa = path_processor->query_automaton;
    int cflag = ci->context_flag;
    int fflag = ci->finish_flag;

    //create parallel streaming automata
    ParallelAutomata pa;
    if(cflag == CONTEXT) pa = ci->pa;
    else initParallelAutomata(&pa, dfa);

    //execute parallel streaming automata
    executeParallelAutomata(&pa, pInfo, ct);
    TupleList* tl = pa.tuple_list;

    ci->pa = pa;

    //predicate filtering
    PredicateFilter pf;
    if(cflag == CONTEXT) { pf = ci->pf; pf.tuple_list = tl; pf.input_stream = input_stream; }
    else initPredicateFilter(&pf, tl, ctx, input_stream);
    Output* output = generateFinalOutput(&pf);
    ci->pf = pf;

    //free up dynamic memories
    freeInputChunks(pInfo);     
    destroyParallelAutomata(&pa); 
    ci->context_flag = CONTEXT;
    return output;
}

static inline Output* parallelRunOpt(PathProcessor* path_processor, char* file_name, int num_core, ConstraintTable* ct)
{
    StreamingContext ci; 
    initStreamingContext(&ci);
    char* input_stream = NULL;
    Output* output = NULL; 
    long start_pos = 0; //pointer to the starting position of the next available input chunk
    while(1)
    {
        input_stream = loadBoundedInputStream(file_name, &start_pos);
        if(input_stream == NULL) break; 
        output = parallelPartialRunOpt(path_processor, input_stream, num_core, ct, &ci);
        free(input_stream);
    }
    destroyStreamingContext(&ci);
    return output;
}

#endif // !__PATH_PROCESSOR_H__
