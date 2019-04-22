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

//group 1: demos for serial streaming automaton 
void Test1()
{
    //loading inputs
    char* stream = loadJSONStream("../../dataset/bb.json");
    char* path = "$.root.products[?(@.sku&&@.productId)].categoryPath[1:3]";  

    //loading dfa
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;
   
    //create streaming automaton
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);

    //run streaming automaton
    printf("begin executing input JSONPath query\n");
    executeAutomaton(&streaming_automaton, stream, CLOSE);

    //predicate filtering
    PredicateFilter pf;
    initPredicateFilter(&pf, streaming_automaton.tuple_list, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    
    //free up dynamic memories
    destroyPredicateFilter(&pf);
    freeOutput(final);
    destroyStreamingAutomaton(&streaming_automaton);
    destoryJSONQueryDFA(dfa); 
}

void Test2()
{
    //loading inputs
    char* stream = loadJSONStream("../../dataset/twitter.json");
    char* path = "$.root[?(@.id&&@.user.screen_name)].quoted_status.entities.user_mentions[?(@.indices&&@.id_str)].id"; 

    //loading dfa
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;
   
    //create streaming automaton
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);

    //run streaming automaton
    printf("begin executing input JSONPath query\n");
    executeAutomaton(&streaming_automaton, stream, CLOSE);

    //predicate filtering
    PredicateFilter pf;
    initPredicateFilter(&pf, streaming_automaton.tuple_list, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    
    //free up dynamic memories
    destroyPredicateFilter(&pf);
    freeOutput(final);
    destroyStreamingAutomaton(&streaming_automaton);
    destoryJSONQueryDFA(dfa); 
}

void Test3()
{
    //loading inputs
    char* stream = loadJSONStream("../../dataset/wiki.json");
    char* path = "$.root[*].claims.P150[?(@.id&&@.type)].mainsnak.property";

    //loading dfa
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;
   
    //create streaming automaton
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);

    //run streaming automaton
    printf("begin executing input JSONPath query\n");
    executeAutomaton(&streaming_automaton, stream, CLOSE);

    //predicate filtering
    PredicateFilter pf;
    initPredicateFilter(&pf, streaming_automaton.tuple_list, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    
    //free up dynamic memories
    destroyPredicateFilter(&pf);
    freeOutput(final);
    destroyStreamingAutomaton(&streaming_automaton);
    destoryJSONQueryDFA(dfa); 
}

//group 2: demos for parallel streaming automaton without data constraint learning
void Test4()
{
    //loading and splitting the input stream
    int num_core = 16;
    PartitionInfo pInfo = partitionFile("../../dataset/bb.json", num_core); 
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root.products[?(@.sku&&@.productId)].categoryPath[?(@.name)].id"; 
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //create parallel streaming automata 
    ParallelAutomata pa;
    initParallelAutomata(&pa, dfa);

    //execute parallel streaming automata
    executeParallelAutomata(&pa, pInfo, NULL);
    TupleList* tl = pa.tuple_list;

    //predicate filtering
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    
    //free up dynamic memories
    destroyPredicateFilter(&pf);
    destroyParallelAutomata(&pa);
    freeOutput(final);
    destoryJSONQueryDFA(dfa); 
}

void Test5()
{
    //loading and splitting the input stream
    int num_core = 64;
    PartitionInfo pInfo = partitionFile("../../dataset/random.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[?((@.index>0&&@.index<2)&&(@.guid||@.name))].friends[?(@.name)].id";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //create parallel streaming automata
    ParallelAutomata pa;
    initParallelAutomata(&pa, dfa);

    //execute parallel streaming automata
    executeParallelAutomata(&pa, pInfo, NULL);
    TupleList* tl = pa.tuple_list;

    //predicate filtering
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));

    //free up dynamic memories
    destroyPredicateFilter(&pf);
    destroyParallelAutomata(&pa);
    freeOutput(final);
    destoryJSONQueryDFA(dfa);
}

void Test6()
{
    //loading and splitting the input stream
    int num_core = 64; 
    PartitionInfo pInfo = partitionFile("../../dataset/twitter.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[*].quoted_status.entities.user_mentions[0:1].indices[0:1]"; 
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //create parallel streaming automata
    ParallelAutomata pa;
    initParallelAutomata(&pa, dfa);

    //execute parallel streaming automata
    executeParallelAutomata(&pa, pInfo, NULL);
    TupleList* tl = pa.tuple_list;

    //predicate filtering
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));

    //free up dynamic memories
    destroyPredicateFilter(&pf);
    destroyParallelAutomata(&pa);
    freeOutput(final);
    destoryJSONQueryDFA(dfa);
}

void Test7()
{
    //loading and splitting the input stream
    int num_core = 16;
    PartitionInfo pInfo = partitionFile("../../dataset/wiki.json", num_core);
    int num_chunk = pInfo.num_chunk;
    char** stream = pInfo.stream;
    //printChunk(stream, num_chunk);

    //loading dfa
    char* path = "$.root[*].claims.P150[2:4].mainsnak.property";
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    //create parallel streaming automata
    ParallelAutomata pa;
    initParallelAutomata(&pa, dfa);

    //execute parallel streaming automata
    executeParallelAutomata(&pa, pInfo, NULL);
    TupleList* tl = pa.tuple_list;

    //predicate filtering
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));

    //free up dynamic memories
    destroyPredicateFilter(&pf);
    destroyParallelAutomata(&pa);
    freeOutput(final);
    destoryJSONQueryDFA(dfa);
}

//group 3: demos for parallel streaming automaton with data constraint learning
void Test8()
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

    //create parallel streaming automata
    ParallelAutomata pa;
    initParallelAutomata(&pa, dfa);

    //execute parallel streaming automata
    executeParallelAutomata(&pa, pInfo, streaming_automaton.constraint_table);
    TupleList* tl = pa.tuple_list;
    destroyStreamingAutomaton(&streaming_automaton);

    //predicate filtering
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));

    //free up dynamic memories
    destroyPredicateFilter(&pf);
    destroyParallelAutomata(&pa);
    freeOutput(final);
    destoryJSONQueryDFA(dfa);
}

void Test9()
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

    //create parallel streaming automata
    ParallelAutomata pa;
    initParallelAutomata(&pa, dfa);

    //execute parallel streaming automata
    executeParallelAutomata(&pa, pInfo, streaming_automaton.constraint_table);
    TupleList* tl = pa.tuple_list;
    destroyStreamingAutomaton(&streaming_automaton);

    //predicate filtering
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));

    //free up dynamic memories
    destroyPredicateFilter(&pf);
    destroyParallelAutomata(&pa);
    freeOutput(final);
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

    //data constraint learning
    printf("begin data constraint learning\n");
    char* train_stream = loadJSONStream("../../dataset/random.json");
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);
    executeAutomaton(&streaming_automaton, train_stream, OPEN);
    printf("end data constraint_learning\n");

    //create parallel streaming automata
    ParallelAutomata pa;
    initParallelAutomata(&pa, dfa);

    //execute parallel streaming automata
    executeParallelAutomata(&pa, pInfo, streaming_automaton.constraint_table);
    TupleList* tl = pa.tuple_list;
    destroyStreamingAutomaton(&streaming_automaton);

    //predicate filtering
    PredicateFilter pf;
    initPredicateFilter(&pf, tl, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));

    //free up dynamic memories
    destroyPredicateFilter(&pf);
    destroyParallelAutomata(&pa);
    freeOutput(final);
    destoryJSONQueryDFA(dfa);
}

int main()
{
   Test1();  
   Test2();  
   Test3();  
   Test4(); 
   Test5(); 
   Test6(); 
   Test7(); 
   Test8(); 
   Test9(); 
   Test10(); 
   return 1;
}
