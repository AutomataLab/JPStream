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

/*void Test1()
{
    struct timeval begin,end;
    double duration;
    //loading inputs
    char* stream = loadJSONStream("../../dataset/bb.json");
    //char* stream = loadJSONStream("bb.json");
    //JSONStream* stream = jps_createJSONStream("twitter_store1.txt",1);
    char* path = "$.root.products[*].categoryPath[*].id";

    //loading dfa
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;
   
    //create streaming automaton
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);

    //run streaming automaton
    printf("begin executing input JSONPath query\n");
    gettimeofday(&begin,NULL);
    executeAutomaton(&streaming_automaton, stream);

    gettimeofday(&end,NULL);
    duration=1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec;
    printf("the total query execution time is %lf\n", duration/1000000);  
    
    //free up dynamic memories
    destroyStreamingAutomaton(&streaming_automaton);
}

void Test2()
{
    struct timeval begin,end;
    double duration;
    //loading inputs
    char* stream = loadJSONStream("../../dataset/bb.json");
    //JSONStream* stream = jps_createJSONStream("bb.json",1);
    //JSONStream* stream = jps_createJSONStream("twitter_store1.txt",1);
    char* path = "$..root..id";

    //loading dfa
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;
   
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);

    //query execution
    printf("begin executing input JSONPath query\n");
    gettimeofday(&begin,NULL);
    executeAutomaton(&streaming_automaton, stream);

    gettimeofday(&end,NULL);
    duration=1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec;
    printf("the total query execution time is %lf\n", duration/1000000);  
    
    //free up dynamic memories
    destroyStreamingAutomaton(&streaming_automaton);
}

void Test3()
{
    struct timeval begin,end;
    double duration;
    //loading inputs
    char* stream = loadJSONStream("../../dataset/twitter.json");
    //JSONStream* stream = jps_createJSONStream("bb.json",1);
    //char* stream = loadJSONStream("twitter_store1.txt");
   // char* path = "$.root[2:5].id";
    char* path = "$.root[*].quoted_status.entities.user_mentions[*].indices[*]";
    //loading dfa
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;
   
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);

    //query execution
    printf("begin executing input JSONPath query\n");
    gettimeofday(&begin,NULL);
    executeAutomaton(&streaming_automaton, stream);

    gettimeofday(&end,NULL);
    duration=1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec;
    printf("the total query execution time is %lf\n", duration/1000000);  
    
    //free up dynamic memories
    destroyStreamingAutomaton(&streaming_automaton);
}

void Test4()
{
    struct timeval begin,end;
    double duration;
    //loading inputs
    char* stream = loadJSONStream("../../dataset/rowstest.json");
    char* path = "$.meta.view.columns[*].position";

    //loading dfa
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;
   
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);

    //query execution
    printf("begin executing input JSONPath query\n");
    gettimeofday(&begin,NULL);
    executeAutomaton(&streaming_automaton, stream);

    gettimeofday(&end,NULL);
    duration=1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec;
    printf("the total query execution time is %lf\n", duration/1000000);  
    
    //free up dynamic memories
    destroyStreamingAutomaton(&streaming_automaton);
}

void Test5()
{
    struct timeval begin,end;
    double duration;
    //loading inputs
    char* stream = loadJSONStream("../../dataset/wiki.json");
    char* path = "$.root[*].claims.P150[2:4].mainsnak.property";

    //loading dfa
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;
   
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);

    //query execution
    printf("begin executing input JSONPath query\n");
    gettimeofday(&begin,NULL);
    executeAutomaton(&streaming_automaton, stream);

    gettimeofday(&end,NULL);
    duration=1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec;
    printf("the total query execution time is %lf\n", duration/1000000);  
    
    //free up dynamic memories
    destroyStreamingAutomaton(&streaming_automaton);
}

void Test6()
{
    struct timeval begin,end;
    double duration;
    //loading inputs
    char* stream = loadJSONStream("../../dataset/random.json");
    char* path = "$.root[*].friends[*].id";

    //loading dfa
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;
   
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);

    //query execution
    printf("begin executing input JSONPath query\n");
    gettimeofday(&begin,NULL);
    executeAutomaton(&streaming_automaton, stream);

    gettimeofday(&end,NULL);
    duration=1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec;
    printf("the total query execution time is %lf\n", duration/1000000);  
    
    //free up dynamic memories
    destroyStreamingAutomaton(&streaming_automaton);
}

void Test7()
{
    struct timeval begin,end;
    double duration;
    //loading inputs
    char* stream = loadJSONStream("../../dataset/rowstest.json");
    char* path = "$.data[1:3]";

    //loading dfa
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;
   
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);

    //query execution
    printf("begin executing input JSONPath query\n");
    gettimeofday(&begin,NULL);
    executeAutomaton(&streaming_automaton, stream);

    gettimeofday(&end,NULL);
    duration=1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec;
    printf("the total query execution time is %lf\n", duration/1000000);  
    
    //free up dynamic memories
    destroyStreamingAutomaton(&streaming_automaton);
}

void Test8()
{
    struct timeval begin,end;
    double duration;
    //loading inputs
    char* stream = loadJSONStream("../../dataset/twitter.json");
    //JSONStream* stream = jps_createJSONStream("bb.json",1);
   // JSONStream* stream = jps_createJSONStream("twitter_store1.txt",1);
   // char* path = "$.root[2:5].id";
    char* path = "$.root[*].quoted_status.entities.symbols";

    //loading dfa
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;
   
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);

    //query execution
    printf("begin executing input JSONPath query\n");
    gettimeofday(&begin,NULL);
    executeAutomaton(&streaming_automaton, stream);

    gettimeofday(&end,NULL);
    duration=1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec;
    printf("the total query execution time is %lf\n", duration/1000000);  
    
    //free up dynamic memories
    destroyStreamingAutomaton(&streaming_automaton);
}

void Test9()
{
    struct timeval begin,end;
    double duration;
    //loading inputs
    char* stream = loadJSONStream("../../dataset/bb.json");
    //JSONStream* stream = jps_createJSONStream("bb.json",1);
    //JSONStream* stream = jps_createJSONStream("twitter_store1.txt",1);
    char* path = "$.root.products[?(@.sku)].categoryPath[1:3]";  //?(@.sku)

    //loading dfa
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;
   
    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);

    //query execution
    printf("begin executing input JSONPath query\n");
    gettimeofday(&begin,NULL);
    executeAutomaton(&streaming_automaton, stream);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, streaming_automaton.tuple_list, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    gettimeofday(&end,NULL);
    duration=1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec;
    printf("the total query execution time is %lf\n", duration/1000000);  
    
    //free up dynamic memories
    destroyStreamingAutomaton(&streaming_automaton);
}

void Test10()
{
    struct timeval begin,end;
    double duration;
    //loading inputs
    char* stream = loadJSONStream("../../dataset/bb.json");
    //char* stream = loadJSONStream("bb.json");
    //JSONStream* stream = jps_createJSONStream("bb.json",1);
    //JSONStream* stream = jps_createJSONStream("twitter_store1.txt",1);
    char* path = "$.root.products[?(@.sku&&@.productId)].categoryPath[?(@.name)].id";

    //loading dfa
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);

    //query execution
    printf("begin executing input JSONPath query\n");
    gettimeofday(&begin,NULL);
    executeAutomaton(&streaming_automaton, stream);
    gettimeofday(&begin,NULL);
    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, streaming_automaton.tuple_list, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf); 
    free(ctx);
    freeOutput(final);
    free(stream);
    gettimeofday(&end,NULL);
    duration=1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec;
    printf("the total query execution time is %lf\n", duration/1000000);

    //free up dynamic memories
    destroyStreamingAutomaton(&streaming_automaton);
}

void Test11()
{
    struct timeval begin,end;
    double duration;
    //loading inputs
    //char* stream = loadJSONStream("twitter_store1.txt");
    char* stream = loadJSONStream("../../dataset/twitter.json");
    //JSONStream* stream = jps_createJSONStream("bb.json",1);
    //JSONStream* stream = jps_createJSONStream("twitter_store1.txt",1);
    //char* path = "$.root.products[?(@.sku&&@.productId)].categoryPath[?(@.name)].id";
    //char* path = "$.root[?(@.id)&&(@.user.screen_name)].quoted_status.entities.user_mentions[?(@.indices)&&(@.id_str)].id";
    //char* path = "$.root[?(@.text=='@rob_b1991 fine for what?')].id"; 
    char* path = "$.root[?(@.text&&(!@.contributors))].id"; 

    //loading dfa
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);

    //query execution
    printf("begin executing input JSONPath query\n");
    gettimeofday(&begin,NULL);
    executeAutomaton(&streaming_automaton, stream);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, streaming_automaton.tuple_list, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    free(ctx);
    free(stream);
    gettimeofday(&end,NULL);
    duration=1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec;
    printf("the total query execution time is %lf\n", duration/1000000);

    //free up dynamic memories
    destroyStreamingAutomaton(&streaming_automaton);
}

void Test12()
{
    struct timeval begin,end;
    double duration;
    //loading inputs
    char* stream = loadJSONStream("../../dataset/rowstest.json");
    //JSONStream* stream = jps_createJSONStream("bb.json",1);
    //JSONStream* stream = jps_createJSONStream("twitter_store1.txt",1);
    //char* path = "$.root.products[?(@.sku&&@.productId)].categoryPath[?(@.name)].id";
    //char* path = "$.root[?(@.id)&&(@.user.screen_name)].quoted_status.entities.user_mentions[?(@.indices)&&(@.id_str)].id";
    char* path = "$.meta.view.columns[?(@.id&&@.name&&@.cachedContents)].position";   

    //loading dfa
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);

    //query execution
    printf("begin executing input JSONPath query\n");
    gettimeofday(&begin,NULL);
    executeAutomaton(&streaming_automaton, stream);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, streaming_automaton.tuple_list, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    gettimeofday(&end,NULL);
    duration=1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec;
    printf("the total query execution time is %lf\n", duration/1000000);

    //free up dynamic memories
    destroyStreamingAutomaton(&streaming_automaton);
}

void Test13()
{
    struct timeval begin,end;
    double duration;
    //loading inputs
    char* stream = loadJSONStream("../../dataset/wiki.json");
    char* path = "$.root[*].claims.P150[?(@.id&&@.type)].mainsnak.property";

    //loading dfa
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);

    //query execution
    printf("begin executing input JSONPath query\n");
    gettimeofday(&begin,NULL);
    executeAutomaton(&streaming_automaton, stream);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, streaming_automaton.tuple_list, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    gettimeofday(&end,NULL);
    duration=1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec;
    printf("the total query execution time is %lf\n", duration/1000000);

    //free up dynamic memories
    destroyStreamingAutomaton(&streaming_automaton);
}

void Test14()
{
    struct timeval begin,end;
    double duration;
    //loading inputs
    char* stream = loadJSONStream("../../dataset/random.json");
    char* path = "$.root[?((@.index>0&&@.index<2)&&(@.guid||@.name))].friends[?(@.name)].id";  //+@.b

    //loading dfa
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);

    //query execution
    printf("begin executing input JSONPath query\n");
    gettimeofday(&begin,NULL);
    executeAutomaton(&streaming_automaton, stream);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, streaming_automaton.tuple_list, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    gettimeofday(&end,NULL);
    duration=1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec;
    printf("the total query execution time is %lf\n", duration/1000000);

    //free up dynamic memories
    destroyStreamingAutomaton(&streaming_automaton);
}*/

void Test15()
{
    struct timeval begin,end;
    double duration;
    //loading inputs
    char* stream = loadJSONStream("../../dataset/random.json");
    char* path = "$.root[?((@.index>0&&@.index<2)&&(@.guid||@.name))].friends[?(@.name)].id";  //+@.b

    //loading dfa
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);

    //query execution
    printf("begin executing input JSONPath query\n");
    gettimeofday(&begin,NULL);
    executeAutomaton(&streaming_automaton, stream, OPEN);

    //print data constraint learning schema
    if(streaming_automaton.constraint_table!=NULL)
        printConstraintTable(streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, streaming_automaton.tuple_list, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    gettimeofday(&end,NULL);
    duration=1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec;
    printf("the total query execution time is %lf\n", duration/1000000);

    //free up dynamic memories
    destroyStreamingAutomaton(&streaming_automaton);
}

void Test16()
{
    struct timeval begin,end;
    double duration;
    //loading inputs
    char* stream = loadJSONStream("../../dataset/wiki.json");
    char* path = "$.root[*].claims.P150[?(@.id&&@.type)].mainsnak.property";

    //loading dfa
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);

    //query execution
    printf("begin executing input JSONPath query\n");
    gettimeofday(&begin,NULL);
    executeAutomaton(&streaming_automaton, stream, OPEN);

    //print data constraint learning schema
    if(streaming_automaton.constraint_table!=NULL)
        printConstraintTable(streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, streaming_automaton.tuple_list, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    gettimeofday(&end,NULL);
    duration=1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec;
    printf("the total query execution time is %lf\n", duration/1000000);

    //free up dynamic memories
    destroyStreamingAutomaton(&streaming_automaton);
}

void Test17()
{
    struct timeval begin,end;
    double duration;
    //loading inputs
    char* stream = loadJSONStream("../../dataset/rowstest.json");
    char* path = "$.meta.view.columns[?(@.id&&@.name&&@.cachedContents)].position";

    //loading dfa
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);

    //query execution
    printf("begin executing input JSONPath query\n");
    gettimeofday(&begin,NULL);
    executeAutomaton(&streaming_automaton, stream, OPEN);

    //print data constraint learning schema
    if(streaming_automaton.constraint_table!=NULL)
        printConstraintTable(streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, streaming_automaton.tuple_list, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    gettimeofday(&end,NULL);
    duration=1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec;
    printf("the total query execution time is %lf\n", duration/1000000);

    //free up dynamic memories
    destroyStreamingAutomaton(&streaming_automaton);
}

void Test18()
{
    struct timeval begin,end;
    double duration;
    //loading inputs
    char* stream = loadJSONStream("../../dataset/twitter.json");
    char* path = "$.root[?(@.text&&(!@.contributors))].id"; 


    //loading dfa
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);

    //query execution
    printf("begin executing input JSONPath query\n");
    gettimeofday(&begin,NULL);
    executeAutomaton(&streaming_automaton, stream, OPEN);

    //print data constraint learning schema
    if(streaming_automaton.constraint_table!=NULL)
        printConstraintTable(streaming_automaton.constraint_table);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, streaming_automaton.tuple_list, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    gettimeofday(&end,NULL);
    duration=1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec;
    printf("the total query execution time is %lf\n", duration/1000000);

    //free up dynamic memories
    destroyStreamingAutomaton(&streaming_automaton);
}

void Test19()
{
    struct timeval begin,end;
    double duration;
    //loading inputs
    char* stream = loadJSONStream("../../dataset/bb.json");
    char* path = "$.root.products[?(@.sku&&@.productId)].categoryPath[?(@.name)].id";


    //loading dfa
    JSONQueryDFAContext* ctx = (JSONQueryDFAContext*)malloc(sizeof(JSONQueryDFAContext));
    JSONQueryDFA* dfa = buildJSONQueryDFA(path, ctx);
    if (dfa == NULL) return;

    StreamingAutomaton streaming_automaton;
    initStreamingAutomaton(&streaming_automaton, dfa);

    //query execution
    printf("begin executing input JSONPath query\n");
    gettimeofday(&begin,NULL);
    executeAutomaton(&streaming_automaton, stream, OPEN);

    //print data constraint learning schema
    if(streaming_automaton.constraint_table!=NULL)
        printConstraintTable(streaming_automaton.constraint_table);

    /*ConstraintInfo ci;
    strcopy("total", ci.token_name);
    ci.type = 6;*/
    ConstraintInfo ci;
    strcopy("total", ci.token_name);
    ci.type = 3;
    updateStateInfo(streaming_automaton.constraint_table, &ci);
    printf("number of constraints for total is %d\n", ci.num_state);
    printf("all states are \n");
    int i = 0;
    for(i = 0; i<ci.num_state; i++)
        printf("state is %d\n", ci.state_set[i]);

    //filtering phase
    PredicateFilter pf;
    initPredicateFilter(&pf, streaming_automaton.tuple_list, ctx);
    Output* final = generateFinalOutput(&pf);
    printf("size of final output is %d\n", getOutputSize(final));
    destroyPredicateFilter(&pf);
    freeOutput(final);
    gettimeofday(&end,NULL);
    duration=1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec;
    printf("the total query execution time is %lf\n", duration/1000000);

    //free up dynamic memories
    destroyStreamingAutomaton(&streaming_automaton);
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

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP);

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

    //execute parallel streaming automata
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP);

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
    int num_core = 16;
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
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP);

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
    int num_core = 16;
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
    TupleList* tl = executeParallelAutomata(pInfo, dfa, num_chunk, WARMUP);

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
{
   /* Test15();
    Test16();
    Test17();
    Test18();
    Test19();
 
    Test27();
    Test28();
    Test54();*/
    Test59();
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
