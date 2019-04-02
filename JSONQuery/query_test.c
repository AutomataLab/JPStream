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

char* loadJSONStream(char* file_name)
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

void Test1()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/bb.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root.products[?(@.sku&&@.productId)].categoryPath[?(@.name)].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, NULL);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa); 
}

void Test1cl()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/bb.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root.products[?(@.sku&&@.productId)].categoryPath[?(@.name)].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/bb.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n"); 

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test1clr()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/bb.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root.products[?(@.sku&&@.productId)].categoryPath[?(@.name)].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/bb.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //make up a wrong constraint information
    ConstraintTable* ct = streaming_automaton.constraint_table;    
    int last = ct->num_constraint_info; 
    int i = 0;
    while(i<last)
    {
        if(strcmp("studio", ct->constraint_info[i].token_name)==0)
        {
            ct->constraint_info[i].num_state = 1;
            ct->constraint_info[i].state_set[0] = 3;
            break;
        }
        i++;
    }      

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test1cl_incomplete()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/bb.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root.products[?(@.sku&&@.productId)].categoryPath[?(@.name)].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/bb.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //make up a wrong constraint information
    ConstraintTable* ct = streaming_automaton.constraint_table;    
    int last = ct->num_constraint_info; 
    int i = 0;
    while(i<last)
    {
        if(strcmp("studio", ct->constraint_info[i].token_name)==0)
        {
            ct->constraint_info[i].num_state = 0;
            ct->constraint_info[i].state_set[0] = 3;
            break;
        }
        i++;
    }      

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test1_large()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("bb.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root.products[?(@.sku&&@.productId)].categoryPath[?(@.name)].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, NULL);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa); 
}

void Test1_large_xeon()
{
    //loading and splitting the input stream
    int num_core = 16;
    PartitionInfo pInfo = partitionFile("bb.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root.products[?(@.sku&&@.productId)].categoryPath[?(@.name)].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, NULL);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa); 
}

void Test1cl_large()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("bb.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root.products[?(@.sku&&@.productId)].categoryPath[?(@.name)].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("bb.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n"); 

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test1clr_large()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("bb.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root.products[?(@.sku&&@.productId)].categoryPath[?(@.name)].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("bb.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //make up a wrong constraint information
    ConstraintTable* ct = streaming_automaton.constraint_table;    
    int last = ct->num_constraint_info; 
    int i = 0;
    while(i<last)
    {
        if(strcmp("studio", ct->constraint_info[i].token_name)==0)
        {
            ct->constraint_info[i].num_state = 1;
            ct->constraint_info[i].state_set[0] = 3;
            break;
        }
        i++;
    }      

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test1cl_incomplete_large()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("bb.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root.products[?(@.sku&&@.productId)].categoryPath[?(@.name)].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("bb.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //make up a wrong constraint information
    ConstraintTable* ct = streaming_automaton.constraint_table;    
    int last = ct->num_constraint_info; 
    int i = 0;
    while(i<last)
    {
        if(strcmp("studio", ct->constraint_info[i].token_name)==0)
        {
            ct->constraint_info[i].num_state = 0;
            ct->constraint_info[i].state_set[0] = 3;
            break;
        }
        i++;
    }      

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test2()
{
    //loading and splitting the input stream
    int num_core = 16;
    PartitionInfo pInfo = partitionFile("../../dataset/bb.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root.products[*].categoryPath[*].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, NULL);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa); 
}

void Test2cl()
{
    //loading and splitting the input stream
    int num_core = 16;
    PartitionInfo pInfo = partitionFile("../../dataset/bb.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root.products[*].categoryPath[*].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/bb.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test3()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/bb.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    //char* path = "$.root.products[*].categoryPath[?(@.name)].id";
    char* path = "$..root..id";
    //char* path = "$.root.products[?(@.sku&&@.productId)].categoryPath[*]";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, NULL);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa); 
}

void Test3cl()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/bb.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    //char* path = "$.root.products[*].categoryPath[?(@.name)].id";
    char* path = "$..root..id";
    //char* path = "$.root.products[?(@.sku&&@.productId)].categoryPath[*]";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/bb.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test3_large()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("bb.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    //char* path = "$.root.products[*].categoryPath[?(@.name)].id";
    char* path = "$..root..id";
    //char* path = "$.root.products[?(@.sku&&@.productId)].categoryPath[*]";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, NULL);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa); 
}

void Test3cl_large()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("bb.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    //char* path = "$.root.products[*].categoryPath[?(@.name)].id";
    char* path = "$..root..id";
    //char* path = "$.root.products[?(@.sku&&@.productId)].categoryPath[*]";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("bb.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test4()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/random.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[?((@.index>0&&@.index<2)&&(@.guid||@.name))].friends[?(@.name)].id";
    //char* path = "$.root[?((@.index||@.guid||@.name))].friends[?(@.name)].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, NULL);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa); 
}

void Test4cl()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/random.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[?((@.index>0&&@.index<2)&&(@.guid||@.name))].friends[?(@.name)].id";
    //char* path = "$.root[?((@.index||@.guid||@.name))].friends[?(@.name)].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/random.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test4clr()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/random.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[?((@.index>0&&@.index<2)&&(@.guid||@.name))].friends[?(@.name)].id";
    //char* path = "$.root[?((@.index||@.guid||@.name))].friends[?(@.name)].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/random.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //make up a wrong constraint information
    ConstraintTable* ct = streaming_automaton.constraint_table;    
    int last = ct->num_constraint_info; 
    int i = 0;
    while(i<last)
    {
        if(strcmp("index", ct->constraint_info[i].token_name)==0)
        {
            ct->constraint_info[i].num_state = 1;
            ct->constraint_info[i].state_set[0] = 5;
            break;
        }
        i++;
    }    

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test4cl_incomplete()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/random.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[?((@.index>0&&@.index<2)&&(@.guid||@.name))].friends[?(@.name)].id";
    //char* path = "$.root[?((@.index||@.guid||@.name))].friends[?(@.name)].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/random.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //make up a wrong constraint information
    ConstraintTable* ct = streaming_automaton.constraint_table;    
    int last = ct->num_constraint_info; 
    int i = 0;
    while(i<last)
    {
        if(strcmp("index", ct->constraint_info[i].token_name)==0)
        {
            ct->constraint_info[i].num_state = 0;
            ct->constraint_info[i].state_set[0] = 5;
            break;
        }
        i++;
    }    

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test5()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/twitter.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[?(@.id&&@.user.screen_name)].quoted_status.entities.user_mentions[?(@.indices&&@.id_str)].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, NULL);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa); 
}

void Test5cl()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/twitter.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[?(@.id&&@.user.screen_name)].quoted_status.entities.user_mentions[?(@.indices&&@.id_str)].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/twitter.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");
    
    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test5_large()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("twitter_store1.txt", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[?(@.id&&@.user.screen_name)].quoted_status.entities.user_mentions[?(@.indices&&@.id_str)].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, NULL);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa); 
}

void Test5cl_large()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("twitter_store1.txt", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[?(@.id&&@.user.screen_name)].quoted_status.entities.user_mentions[?(@.indices&&@.id_str)].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("twitter_store1.txt");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");
    
    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test6()
{
    //loading and splitting the input stream
    int num_core = 26;
    PartitionInfo pInfo = partitionFile("../../dataset/rowstest.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.meta.view.columns[?(@.id&&@.name&&@.cachedContents)].position";  
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, NULL);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa); 
}

void Test6cl()
{
    //loading and splitting the input stream
    int num_core = 26;
    PartitionInfo pInfo = partitionFile("../../dataset/rowstest.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.meta.view.columns[?(@.id&&@.name&&@.cachedContents)].position";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/rowstest.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test6clr()
{
    //loading and splitting the input stream
    int num_core = 26;
    PartitionInfo pInfo = partitionFile("../../dataset/rowstest.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.meta.view.columns[?(@.id&&@.name&&@.cachedContents)].position";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/rowstest.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //make up a wrong constraint information
    ConstraintTable* ct = streaming_automaton.constraint_table;    
    int last = ct->num_constraint_info; 
    int i = 0;
    while(i<last)
    {
        if(strcmp("id", ct->constraint_info[i].token_name)==0)
        {
            ct->constraint_info[i].num_state = 1;
            ct->constraint_info[i].state_set[0] = 4;
            break;
        }
        i++;
    }    

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test6cl_incomplete()
{
    //loading and splitting the input stream
    int num_core = 26;
    PartitionInfo pInfo = partitionFile("../../dataset/rowstest.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.meta.view.columns[?(@.id&&@.name&&@.cachedContents)].position";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/rowstest.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //make up a wrong constraint information
    ConstraintTable* ct = streaming_automaton.constraint_table;    
    int last = ct->num_constraint_info; 
    int i = 0;
    while(i<last)
    {
        if(strcmp("id", ct->constraint_info[i].token_name)==0)
        {
            ct->constraint_info[i].num_state = 0;
            ct->constraint_info[i].state_set[0] = 4;
            break;
        }
        i++;
    }    

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test7()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/wiki.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[*].claims.P150[?(@.id&&@.type)].mainsnak.property";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, NULL);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa); 
}

void Test7cl()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/wiki.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[*].claims.P150[?(@.id&&@.type)].mainsnak.property";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/wiki.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test7clr()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/wiki.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[*].claims.P150[?(@.id&&@.type)].mainsnak.property";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/wiki.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //make up a wrong constraint information
    ConstraintTable* ct = streaming_automaton.constraint_table;    
    int last = ct->num_constraint_info; 
    int i = 0;
    while(i<last)
    {
        if(strcmp("property", ct->constraint_info[i].token_name)==0)
        {
            ct->constraint_info[i].num_state = 1;
            ct->constraint_info[i].state_set[0] = 5;
            break;
        }
        i++;
    }    

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test7cl_incomplete()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/wiki.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[*].claims.P150[?(@.id&&@.type)].mainsnak.property";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/wiki.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //make up a wrong constraint information
    ConstraintTable* ct = streaming_automaton.constraint_table;    
    int last = ct->num_constraint_info; 
    int i = 0;
    while(i<last)
    {
        if(strcmp("property", ct->constraint_info[i].token_name)==0)
        {
            ct->constraint_info[i].num_state = 0;
            ct->constraint_info[i].state_set[0] = 5;
            break;
        }
        i++;
    }    

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test8()
{
    //loading and splitting the input stream
    int num_core = 16;
    PartitionInfo pInfo = partitionFile("../../dataset/wiki.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[*].claims.P150[*].mainsnak.property";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, NULL);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa); 
}

void Test8cl()
{
    //loading and splitting the input stream
    int num_core = 16;
    PartitionInfo pInfo = partitionFile("../../dataset/wiki.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[*].claims.P150[*].mainsnak.property";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

      //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/wiki.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}


void Test9()
{
    //loading and splitting the input stream
    int num_core = 16;
    PartitionInfo pInfo = partitionFile("../../dataset/rowstest.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.meta.view.columns[*].position";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, NULL);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa); 
}

void Test9cl()
{
    //loading and splitting the input stream
    int num_core = 16;
    PartitionInfo pInfo = partitionFile("../../dataset/rowstest.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.meta.view.columns[*].position";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

        //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/rowstest.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test10()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/random.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[*].friends[*].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, NULL);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa); 
}

void Test10cl()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/random.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[*].friends[*].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/random.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test11()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/wiki.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
   // printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[*].claims.P150[2:4].mainsnak.property";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, NULL);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa); 
}

void Test11cl()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/wiki.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
   // printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[*].claims.P150[2:4].mainsnak.property";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/wiki.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test11clr()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/wiki.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    ///printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[*].claims.P150[2:4].mainsnak.property";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/wiki.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //make up a wrong constraint information
    ConstraintTable* ct = streaming_automaton.constraint_table;    
    int last = ct->num_constraint_info; 
    int i = 0;
    while(i<last)
    {
        if(strcmp("id", ct->constraint_info[i].token_name)==0)
        {
            ct->constraint_info[i].num_state = 1;
            ct->constraint_info[i].state_set[0] = 6;
            break;
        }
        i++;
    }    

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test11cl_incomplete()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/wiki.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[*].claims.P150[2:4].mainsnak.property";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/wiki.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //make up a wrong constraint information
    ConstraintTable* ct = streaming_automaton.constraint_table;    
    int last = ct->num_constraint_info; 
    int i = 0;
    while(i<last)
    {
        if(strcmp("id", ct->constraint_info[i].token_name)==0)
        {
            ct->constraint_info[i].num_state = 0;
            ct->constraint_info[i].state_set[0] = 6;
            break;
        }
        i++;
    }    

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test12()
{
    //loading and splitting the input stream
    int num_core = 26;
    PartitionInfo pInfo = partitionFile("../../dataset/rowstest.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.meta.view.columns[2:6].position";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, NULL);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test12cl()
{
    //loading and splitting the input stream
    int num_core = 26;
    PartitionInfo pInfo = partitionFile("../../dataset/rowstest.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.meta.view.columns[2:6].position";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/rowstest.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test12clr()
{
    //loading and splitting the input stream
    int num_core = 26;
    PartitionInfo pInfo = partitionFile("../../dataset/rowstest.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.meta.view.columns[2:6].position";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/rowstest.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //make up a wrong constraint information
    ConstraintTable* ct = streaming_automaton.constraint_table;    
    int last = ct->num_constraint_info; 
    int i = 0;
    while(i<last)
    {
        if(strcmp("name", ct->constraint_info[i].token_name)==0)
        {
            ct->constraint_info[i].num_state = 1;
            ct->constraint_info[i].state_set[0] = 6;
            break;
        }
        i++;
    }    

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test12cl_incomplete()
{
    //loading and splitting the input stream
    int num_core = 26;
    PartitionInfo pInfo = partitionFile("../../dataset/rowstest.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.meta.view.columns[2:6].position";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/rowstest.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    //print data constraint learning schema
 /*   if(streaming_automaton.constraint_table!=NULL)
        printConstraintTable(streaming_automaton.constraint_table);*/
    printf("end data constraint_learning\n");

    //make up a wrong constraint information
    ConstraintTable* ct = streaming_automaton.constraint_table;    
    int last = ct->num_constraint_info; 
    int i = 0;
    while(i<last)
    {
        if(strcmp("name", ct->constraint_info[i].token_name)==0)
        {
            ct->constraint_info[i].num_state = 0;
            ct->constraint_info[i].state_set[0] = 6;
            break;
        }
        i++;
    }    

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test13()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/random.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[*].friends[1:3].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, NULL);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test13cl()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/random.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[*].friends[1:3].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/random.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test13clr()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/random.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[*].friends[1:3].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/random.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //make up a wrong constraint information
    ConstraintTable* ct = streaming_automaton.constraint_table;    
    int last = ct->num_constraint_info; 
    int i = 0;
    while(i<last)
    {
        if(strcmp("friends", ct->constraint_info[i].token_name)==0)
        {
            ct->constraint_info[i].num_state = 1;
            ct->constraint_info[i].state_set[0] = 5;
            break;
        }
        i++;
    }    

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test13cl_incomplete()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/random.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[*].friends[1:3].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/random.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //make up a wrong constraint information
    ConstraintTable* ct = streaming_automaton.constraint_table;    
    int last = ct->num_constraint_info; 
    int i = 0;
    while(i<last)
    {
        if(strcmp("friends", ct->constraint_info[i].token_name)==0)
        {
            ct->constraint_info[i].num_state = 0;
            ct->constraint_info[i].state_set[0] = 5;
            break;
        }
        i++;
    }    

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test14()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/bb.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
   /// printChunk(stream, num_chunk);

    //loading dfa
    //char* path = "$.root.products[*].categoryPath[?(@.name)].id";
    //char* path = "$..root..id";
    char* path = "$.root.products[?(@.sku&&@.productId)].categoryPath[1:3]";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;


    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, NULL);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test14cl()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/bb.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
   /// printChunk(stream, num_chunk);

    //loading dfa
    //char* path = "$.root.products[*].categoryPath[?(@.name)].id";
    //char* path = "$..root..id";
    char* path = "$.root.products[?(@.sku&&@.productId)].categoryPath[1:3]";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/bb.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test14clr()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/bb.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root.products[?(@.sku&&@.productId)].categoryPath[1:3]";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/bb.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //make up a wrong constraint information
    ConstraintTable* ct = streaming_automaton.constraint_table;    
    int last = ct->num_constraint_info; 
    int i = 0;
    while(i<last)
    {
        if(strcmp("studio", ct->constraint_info[i].token_name)==0)
        {
            ct->constraint_info[i].num_state = 1;
            ct->constraint_info[i].state_set[0] = 3;
            break;
        }
        i++;
    }      

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test14cl_incomplete()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/bb.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root.products[?(@.sku&&@.productId)].categoryPath[1:3]";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/bb.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //make up a wrong constraint information
    ConstraintTable* ct = streaming_automaton.constraint_table;    
    int last = ct->num_constraint_info; 
    int i = 0;
    while(i<last)
    {
        if(strcmp("studio", ct->constraint_info[i].token_name)==0)
        {
            ct->constraint_info[i].num_state = 0;
            ct->constraint_info[i].state_set[0] = 3;
            break;
        }
        i++;
    }      

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test14_large()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("bb.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
   /// printChunk(stream, num_chunk);

    //loading dfa
    //char* path = "$.root.products[*].categoryPath[?(@.name)].id";
    //char* path = "$..root..id";
    char* path = "$.root.products[?(@.sku&&@.productId)].categoryPath[1:3]";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;


    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, NULL);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test14_large_xeon()
{
    //loading and splitting the input stream
    int num_core = 16;
    PartitionInfo pInfo = partitionFile("bb.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
   /// printChunk(stream, num_chunk);

    //loading dfa
    //char* path = "$.root.products[*].categoryPath[?(@.name)].id";
    //char* path = "$..root..id";
    char* path = "$.root.products[?(@.sku&&@.productId)].categoryPath[1:3]";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;


    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, NULL);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test14cl_large()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("bb.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
   /// printChunk(stream, num_chunk);

    //loading dfa
    //char* path = "$.root.products[*].categoryPath[?(@.name)].id";
    //char* path = "$..root..id";
    char* path = "$.root.products[?(@.sku&&@.productId)].categoryPath[1:3]";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("bb.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test14cl_large_xeon()
{
    //loading and splitting the input stream
    int num_core = 16;
    PartitionInfo pInfo = partitionFile("bb.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
   /// printChunk(stream, num_chunk);

    //loading dfa
    //char* path = "$.root.products[*].categoryPath[?(@.name)].id";
    //char* path = "$..root..id";
    char* path = "$.root.products[?(@.sku&&@.productId)].categoryPath[1:3]";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("bb.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test14clr_large()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("bb.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root.products[?(@.sku&&@.productId)].categoryPath[1:3]";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("bb.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //make up a wrong constraint information
    ConstraintTable* ct = streaming_automaton.constraint_table;    
    int last = ct->num_constraint_info; 
    int i = 0;
    while(i<last)
    {
        if(strcmp("studio", ct->constraint_info[i].token_name)==0)
        {
            ct->constraint_info[i].num_state = 1;
            ct->constraint_info[i].state_set[0] = 3;
            break;
        }
        i++;
    }      

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);
    destroyStreamingAutomaton(&streaming_automaton);
    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test14cl_incomplete_large()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("bb.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root.products[?(@.sku&&@.productId)].categoryPath[1:3]";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("bb.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //make up a wrong constraint information
    ConstraintTable* ct = streaming_automaton.constraint_table;    
    int last = ct->num_constraint_info; 
    int i = 0;
    while(i<last)
    {
        if(strcmp("studio", ct->constraint_info[i].token_name)==0)
        {
            ct->constraint_info[i].num_state = 0;
            ct->constraint_info[i].state_set[0] = 3;
            break;
        }
        i++;
    }      

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);
    destroyStreamingAutomaton(&streaming_automaton);
    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test15()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/bb.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    //char* path = "$.root.products[*].categoryPath[?(@.name)].id";
    //char* path = "$..root..id";
    char* path = "$.root.products[1:5].categoryPath[?(@.name)]";  //?(@.name)
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;


    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, NULL);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test15cl()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/bb.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    //char* path = "$.root.products[*].categoryPath[?(@.name)].id";
    //char* path = "$..root..id";
    char* path = "$.root.products[1:5].categoryPath[?(@.name)]";  //?(@.name)
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/bb.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test15_large()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("bb.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    //char* path = "$.root.products[*].categoryPath[?(@.name)].id";
    //char* path = "$..root..id";
    char* path = "$.root.products[1:5].categoryPath[?(@.name)]";  //?(@.name)
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;


    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, NULL);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test15cl_large()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("bb.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    //char* path = "$.root.products[*].categoryPath[?(@.name)].id";
    //char* path = "$..root..id";
    char* path = "$.root.products[1:5].categoryPath[?(@.name)]";  //?(@.name)
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("bb.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test16cl()
{
    //loading and splitting the input stream
    int num_core = 26;
    PartitionInfo pInfo = partitionFile("../../dataset/rowstest.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.data[1:3]";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/rowstest.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test16()
{
    //loading and splitting the input stream
    int num_core = 26;
    PartitionInfo pInfo = partitionFile("../../dataset/rowstest.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.data[1:3]";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, NULL);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test17clr()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/twitter.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
   /// printChunk(stream, num_chunk);

    //loading dfa
    //char* path = "$.data[1:3]";
    char* path = "$.root[?(@.text&&(!@.contributors))].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/twitter.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //make up a wrong constraint information
    ConstraintTable* ct = streaming_automaton.constraint_table;    
    int last = ct->num_constraint_info; 
    int i = 0;
    while(i<last)
    {
        if(strcmp("geo", ct->constraint_info[i].token_name)==0)
        {
            ct->constraint_info[i].num_state = 1;
            ct->constraint_info[i].state_set[0] = 4;
            break;
        }
        i++;
    }    

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test17cl_incomplete()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/twitter.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
   /// printChunk(stream, num_chunk);

    //loading dfa
    //char* path = "$.data[1:3]";
    char* path = "$.root[?(@.text&&(!@.contributors))].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/twitter.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //make up a wrong constraint information
    ConstraintTable* ct = streaming_automaton.constraint_table;    
    int last = ct->num_constraint_info; 
    int i = 0;
    while(i<last)
    {
        if(strcmp("geo", ct->constraint_info[i].token_name)==0)
        {
            ct->constraint_info[i].num_state = 0;
            ct->constraint_info[i].state_set[0] = 4;
            break;
        }
        i++;
    }    

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test17cl()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/twitter.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
   /// printChunk(stream, num_chunk);

    //loading dfa
    //char* path = "$.data[1:3]";
    char* path = "$.root[?(@.text&&(!@.contributors))].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/twitter.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test17()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/twitter.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[?(@.text&&(!@.contributors))].id";
    //char* path = "$.data[1:3]";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, NULL);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test17cl_incomplete_large()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("twitter_store1.txt", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
   /// printChunk(stream, num_chunk);

    //loading dfa
    //char* path = "$.data[1:3]";
    char* path = "$.root[?(@.text&&(!@.contributors))].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("twitter_store1.txt");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //make up a wrong constraint information
    ConstraintTable* ct = streaming_automaton.constraint_table;    
    int last = ct->num_constraint_info; 
    int i = 0;
    while(i<last)
    {
        if(strcmp("geo", ct->constraint_info[i].token_name)==0)
        {
            ct->constraint_info[i].num_state = 0;
            ct->constraint_info[i].state_set[0] = 4;
            break;
        }
        i++;
    }    

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test17clr_large()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("twitter_store1.txt", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
   /// printChunk(stream, num_chunk);

    //loading dfa
    //char* path = "$.data[1:3]";
    char* path = "$.root[?(@.text&&(!@.contributors))].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("twitter_store1.txt");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //make up a wrong constraint information
    ConstraintTable* ct = streaming_automaton.constraint_table;    
    int last = ct->num_constraint_info; 
    int i = 0;
    while(i<last)
    {
        if(strcmp("geo", ct->constraint_info[i].token_name)==0)
        {
            ct->constraint_info[i].num_state = 1;
            ct->constraint_info[i].state_set[0] = 4;
            break;
        }
        i++;
    }    

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test17_large_xeon()
{
    //loading and splitting the input stream
    int num_core = 16;
    PartitionInfo pInfo = partitionFile("twitter_store1.txt", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
   /// printChunk(stream, num_chunk);

    //loading dfa
    //char* path = "$.data[1:3]";
    char* path = "$.root[?(@.text&&(!@.contributors))].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, NULL);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test17cl_large()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("twitter_store1.txt", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
   /// printChunk(stream, num_chunk);

    //loading dfa
    //char* path = "$.data[1:3]";
    char* path = "$.root[?(@.text&&(!@.contributors))].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("twitter_store1.txt");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test17_large()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("twitter_store1.txt", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[?(@.text&&(!@.contributors))].id";
    //char* path = "$.data[1:3]";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, NULL);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test18()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/twitter.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[*].quoted_status.entities.user_mentions[0:1].indices[0:1]";
    //char* path = "$.root[?(@.text&&(!@.contributors))].id";
    //char* path = "$.data[1:3]";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, NULL);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test18cl()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/twitter.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
   /// printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[*].quoted_status.entities.user_mentions[0:1].indices[0:1]";
    //char* path = "$.data[1:3]";
    //char* path = "$.root[?(@.text&&(!@.contributors))].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/twitter.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test18clr()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/twitter.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
   /// printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[*].quoted_status.entities.user_mentions[0:1].indices[0:1]";
    //char* path = "$.data[1:3]";
    //char* path = "$.root[?(@.text&&(!@.contributors))].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/twitter.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //make up a wrong constraint information
    ConstraintTable* ct = streaming_automaton.constraint_table;    
    int last = ct->num_constraint_info; 
    int i = 0;
    while(i<last)
    {
        if(strcmp("contributors", ct->constraint_info[i].token_name)==0)
        {
            ct->constraint_info[i].num_state = 1;
            ct->constraint_info[i].state_set[0] = 5;
            break;
        }
        i++;
    }    

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test18cl_incomplete()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/twitter.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
   /// printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[*].quoted_status.entities.user_mentions[0:1].indices[0:1]";
    //char* path = "$.data[1:3]";
    //char* path = "$.root[?(@.text&&(!@.contributors))].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/twitter.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //make up a wrong constraint information
    ConstraintTable* ct = streaming_automaton.constraint_table;    
    int last = ct->num_constraint_info; 
    int i = 0;
    while(i<last)
    {
        if(strcmp("contributors", ct->constraint_info[i].token_name)==0)
        {
            ct->constraint_info[i].num_state = 0;
            ct->constraint_info[i].state_set[0] = 5;
            break;
        }
        i++;
    }    

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test18_large()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("twitter_store1.txt", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[*].quoted_status.entities.user_mentions[0:1].indices[0:1]";
    //char* path = "$.root[?(@.text&&(!@.contributors))].id";
    //char* path = "$.data[1:3]";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, NULL);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test18_large_xeon()
{
    //loading and splitting the input stream
    int num_core = 16;
    PartitionInfo pInfo = partitionFile("twitter_store1.txt", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[*].quoted_status.entities.user_mentions[0:1].indices[0:1]";
    //char* path = "$.root[?(@.text&&(!@.contributors))].id";
    //char* path = "$.data[1:3]";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, NULL);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test18cl_large()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("twitter_store1.txt", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
   /// printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[*].quoted_status.entities.user_mentions[0:1].indices[0:1]";
    //char* path = "$.data[1:3]";
    //char* path = "$.root[?(@.text&&(!@.contributors))].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("twitter_store1.txt");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test18cl_large_xeon()
{
    //loading and splitting the input stream
    int num_core = 16;
    PartitionInfo pInfo = partitionFile("twitter_store1.txt", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
   /// printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[*].quoted_status.entities.user_mentions[0:1].indices[0:1]";
    //char* path = "$.data[1:3]";
    //char* path = "$.root[?(@.text&&(!@.contributors))].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("twitter_store1.txt");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test18clr_large()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("twitter_store1.txt", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
   /// printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[*].quoted_status.entities.user_mentions[0:1].indices[0:1]";
    //char* path = "$.data[1:3]";
    //char* path = "$.root[?(@.text&&(!@.contributors))].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("twitter_store1.txt");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //make up a wrong constraint information
    ConstraintTable* ct = streaming_automaton.constraint_table;    
    int last = ct->num_constraint_info; 
    int i = 0;
    while(i<last)
    {
        if(strcmp("contributors", ct->constraint_info[i].token_name)==0)
        {
            ct->constraint_info[i].num_state = 1;
            ct->constraint_info[i].state_set[0] = 5;
            break;
        }
        i++;
    }    

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test18cl_incomplete_large()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("twitter_store1.txt", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
   /// printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[*].quoted_status.entities.user_mentions[0:1].indices[0:1]";
    //char* path = "$.data[1:3]";
    //char* path = "$.root[?(@.text&&(!@.contributors))].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("twitter_store1.txt");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //make up a wrong constraint information
    ConstraintTable* ct = streaming_automaton.constraint_table;    
    int last = ct->num_constraint_info; 
    int i = 0;
    while(i<last)
    {
        if(strcmp("contributors", ct->constraint_info[i].token_name)==0)
        {
            ct->constraint_info[i].num_state = 0;
            ct->constraint_info[i].state_set[0] = 5;
            break;
        }
        i++;
    }    

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test27()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/bb.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    //char* path = "$.root.products[*].categoryPath[?(@.name)].id";
    char* path = "$.root.products[?(@.sku&&@.productId)].categoryPath[?(@.name)].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/bb.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");
    if(streaming_automaton.constraint_table!=NULL)
        printConstraintTable(streaming_automaton.constraint_table);

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);
    free(train_stream);
    
    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test28()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/random.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[?((@.index>0&&@.index<2)&&(@.guid||@.name))].friends[?(@.name)].id";
    //char* path = "$.root[?((@.index||@.guid||@.name))].friends[?(@.name)].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/random.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test35()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("twitter_store1.txt", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[?(@.id&&@.user.screen_name)].quoted_status.entities.user_mentions[?(@.indices&&@.id_str)].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("twitter_store1.txt");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}


//test array indexes
void Test49()
{
    //loading and splitting the input stream
    int num_core = 16;
    PartitionInfo pInfo = partitionFile("../../dataset/wiki.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
   // printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[*].claims.P150[2:4].mainsnak.property";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/wiki.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);//OPEN);
    printf("end data constraint_learning\n");

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test50()
{
    //loading and splitting the input stream
    int num_core = 26;
    PartitionInfo pInfo = partitionFile("../../dataset/rowstest.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.meta.view.columns[2:6].position";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/rowstest.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);//OPEN);
    printf("end data constraint_learning\n");

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test51()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/random.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[*].friends[1:3].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/random.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);//OPEN);
    printf("end data constraint_learning\n");

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}
void Test52()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/bb.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    //char* path = "$.root.products[*].categoryPath[?(@.name)].id";
    //char* path = "$..root..id";
    char* path = "$.root.products[?(@.sku&&@.productId)].categoryPath[1:2]";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/bb.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);//OPEN);
    printf("end data constraint_learning\n");

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}


void Test54()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/bb.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    //char* path = "$.root.products[*].categoryPath[?(@.name)].id";
    //char* path = "$..root..id";
    char* path = "$.root.products[1:5].categoryPath[?(@.name)]";  //?(@.name)
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/bb.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);//OPEN);
    printf("end data constraint_learning\n");

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test56()
{
    //loading and splitting the input stream
    int num_core = 26;
    PartitionInfo pInfo = partitionFile("../../dataset/rowstest.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.data[1:3]";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/rowstest.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);//OPEN);
    printf("end data constraint_learning\n");

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test57()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("bb.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    //char* path = "$.root.products[*].categoryPath[?(@.name)].id";
    //char* path = "$..root..id";
    char* path = "$.root.products[?(@.sku&&@.productId)].categoryPath[1:2]";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;
    
    //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("bb.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    struct timeval begin,end;
    double duration;
    gettimeofday(&begin,NULL);
    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);
    gettimeofday(&end,NULL);
    duration=1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec;
    printf("the total query execution time is %lf\n", duration/1000000);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test58()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("bb.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    //char* path = "$.root.products[*].categoryPath[?(@.name)].id";
    //char* path = "$..root..id";
    char* path = "$.root.products[1:5].categoryPath[1:2]";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

     //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("bb.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test59()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("bb.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    //char* path = "$.root.products[*].categoryPath[?(@.name)].id";
    //char* path = "$..root..id";
    char* path = "$.root.products[1:5].categoryPath[?(@.name)]";  //?(@.name)
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("bb.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, CLOSE);
    printf("end data constraint_learning\n");
    ///printf("num constraint info %d\n", streaming_automaton.constraint_table->num_constraint_info);

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

void Test62()
{
    //loading and splitting the input stream
    int num_core = 1;
    PartitionInfo pInfo = partitionFile("twitter_store1.txt", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[*].quoted_status.entities.user_mentions[0:1].indices[0:1]";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("twitter_store1.txt");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP, streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    freeTupleList(tl);
    destoryJSONQueryDFA(dfa);
}

int main()
{//Test14();
   //group 1: small file, no constraint learning
  /* Test1();  //78
   Test2();  //78
   Test3();  //78
   Test4(); 
   Test5(); 
   Test6(); 
   Test7(); 
   Test8(); 
   Test9(); 
   Test10();
   Test11();
   Test12();
   Test13();
   Test14();
   Test15();
   Test16();
   Test17();
   Test18();*/
   //group 2: small file, with constraint learning (100% accuracy)
   Test1cl();
   Test2cl();
   Test3cl();
   Test4cl();
   Test5cl();
   Test6cl();
   Test7cl();
   Test8cl();
   Test9cl();
   Test10cl();
   Test11cl();
   Test12cl();
   Test13cl();
   Test14cl();
   Test15cl();
   Test16cl();
   Test17cl();
   Test18cl();
   //group 3: small file, with incomplete constraint learning
   /*Test1cl_incomplete();
   Test4cl_incomplete();
   Test6cl_incomplete();
   Test7cl_incomplete();
   Test11cl_incomplete();
   Test12cl_incomplete();
   Test13cl_incomplete();
   Test14cl_incomplete();
   Test17cl_incomplete();
   Test18cl_incomplete();*/
   //group 4: small file, with constraint learning and reprocessing
   /*Test1clr();
   Test4clr();
   Test6clr();
   Test7clr();
   Test11clr();
   Test12clr();
   Test13clr();
   Test14clr();
   Test17clr();
   Test18clr();*/
   
   //group 5: large file, no constraint learning
   Test1_large();
   Test3_large();
//   Test14_large();
   Test15_large();
   Test5_large();
   Test17_large();
//   Test18_large();
   //group 6: large file, with constraint learning (100% accuracy)
   Test1cl_large();
   Test3cl_large();
//   Test14cl_large();
   Test15cl_large();
   Test5cl_large();
   Test17cl_large();
 //  Test18cl_large();
   //group 7: large file, with incomplete constraint learning
   /*Test1cl_incomplete_large();
   Test14cl_incomplete_large();
   Test17cl_incomplete_large();
   Test18cl_incomplete_large();*/
   //group 8: large file, with constraint learning and reprocessing
   //Test1clr_large();
   //Test14clr_large();
   //Test17clr_large();
   //Test18clr_large();

   //group 9: performance testing (xeon)
 /*  Test18cl_large_xeon();
   Test14cl_large_xeon();
   Test14_large_xeon();
   Test18_large_xeon();*/

   //group 10: performance testing (xeon-phi)
   /*Test18cl_large();
   Test14cl_large();
   Test14_large();
   Test18_large();*/



   /* Test15as();
    Test16as();
    Test17as();
    Test18as();
    Test19as();
    Test20as();
    Test21as();
    Test22as();*/

//    Test19(); 
/*    Test27();
    Test28();*/

   /* Test49();
    Test50(); 
    Test51();
    Test52();
    Test54();
    Test56();*/

    //large data verification
    /*Test35();
    Test57();
    Test58();
    Test62();
*/
    //Test59();
    /*Test1(); //78
    Test2(); //78
    Test3(); //2
    Test4(); //43
    Test5(); //32
    Test6(); //189
    Test7(); //2
    Test8(); //1
    Test9(); //26
    
    Test10();
    Test11();
    Test12();
    Test13();
    Test14();*/
    return 1;
}
