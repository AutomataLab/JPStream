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

#define MAX_EXTENSION 1000

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

//split input stream into several chunks
PartitionInfo partitionInputStream(char* input_stream, int num_core)
{
    PartitionInfo pInfo;
    pInfo.num_chunk = 0;
    char** stream = NULL;
    long stream_size, chunk_size;
    stream_size = strlen(input_stream);
    chunk_size = (stream_size/num_core)+1;
    stream = (char**)malloc(num_core*sizeof(char*));
    int i;
    for(i = 0; i<num_core; i++)
        stream[i] = NULL;
    long sum_size = 0;   //the number of bytes that have been processed
    char ch = -1; 
    for(i = 0; i<num_core-1; i++)
    {
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
    long remain_size = stream_size-sum_size;
    if(remain_size>0)
    {
        stream[i] = substring(input_stream, sum_size, stream_size);
        pInfo.num_chunk = i+1;
    }
    pInfo.num_chunk = i+1;
    pInfo.stream = stream;
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


static inline Output* serialRun(PathProcessor* path_processor, char* input_stream)
{
    JSONQueryDFAContext* ctx = path_processor->query_context;
    JSONQueryDFA* dfa = path_processor->query_automaton;

    //create streaming automaton
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);

    //run streaming automaton
    executeAutomaton(&streaming_automaton, input_stream, CLOSE);

    //predicate filtering
    PredicateFilter pf;
    initPredicateFilter(&pf, streaming_automaton.tuple_list, ctx);
    Output* output = generateFinalOutput(&pf);
    
    //free up dynamic memories
    destroyPredicateFilter(&pf);
    destroyStreamingAutomaton(&streaming_automaton);
    return output;
}

static inline ConstraintTable* collectDataConstraints(PathProcessor* path_processor, char* input_stream)
{
    JSONQueryDFAContext* ctx = path_processor->query_context;
    JSONQueryDFA* dfa = path_processor->query_automaton;

    //create streaming automaton
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);

    //run streaming automaton
    executeAutomaton(&streaming_automaton, input_stream, OPEN);

    //free up dynamic memories
    destroyStreamingAutomaton(&streaming_automaton);
    return streaming_automaton.constraint_table;
}

static inline Output* parallelRun(PathProcessor* path_processor, char* input_stream, int num_core)
{
    PartitionInfo pInfo = partitionInputStream(input_stream, num_core);

    JSONQueryDFAContext* ctx = path_processor->query_context;
    JSONQueryDFA* dfa = path_processor->query_automaton;
    
    //create parallel streaming automata 
    ParallelAutomata pa;
    initParallelAutomata(&pa, dfa);

    //execute parallel streaming automata
    executeParallelAutomata(&pa, pInfo, NULL);
    TupleList* tl = pa.tuple_list;

    //predicate filtering
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* output = generateFinalOutput(&pf);
   
    //free up dynamic memories
    freeInputChunks(pInfo);
    destroyPredicateFilter(&pf);
    destroyParallelAutomata(&pa);
    return output; 
}

static inline Output* parallelRunOpt(PathProcessor* path_processor, char* input_stream, int num_core, ConstraintTable* ct)
{
    PartitionInfo pInfo = partitionInputStream(input_stream, num_core);

    JSONQueryDFAContext* ctx = path_processor->query_context;
    JSONQueryDFA* dfa = path_processor->query_automaton;

    //create parallel streaming automata
    ParallelAutomata pa;
    initParallelAutomata(&pa, dfa);

    //execute parallel streaming automata
    executeParallelAutomata(&pa, pInfo, ct);
    TupleList* tl = pa.tuple_list;

    //predicate filtering
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* output = generateFinalOutput(&pf);

    //free up dynamic memories
    freeInputChunks(pInfo);
    destroyPredicateFilter(&pf);
    destroyParallelAutomata(&pa);
    return output;
}
#endif // !__PATH_PROCESSOR_H__
