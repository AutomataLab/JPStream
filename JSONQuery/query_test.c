#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/file.h>
#include "streaming_automaton.h"

void Test1()
{
    struct timeval begin,end;
    double duration;
    //loading inputs
    JSONStream* stream = jps_createJSONStream("../../dataset/bb.json",1);
    //JSONStream* stream = jps_createJSONStream("bb.json",1);
    //JSONStream* stream = jps_createJSONStream("twitter_store1.txt",1);
    char* path = "$.root.products[*].categoryPath[*].id";

    //loading dfa
    JQ_CONTEXT* ctx = (JQ_CONTEXT*)malloc(sizeof(JQ_CONTEXT));
    JQ_DFA* dfa = dfa_Create(path, ctx);
    if (dfa == NULL) return;
   
    //create streaming automaton
    StreamingAutomaton streaming_automaton;
    jsr_StreamingAutomatonCtor(&streaming_automaton, stream, dfa);

    //run streaming automaton
    printf("begin executing input JSONPath query\n");
    gettimeofday(&begin,NULL);
    jsr_automaton_execution(&streaming_automaton);

    gettimeofday(&end,NULL);
    duration=1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec;
    printf("the total query execution time is %lf\n", duration/1000000);  
    
    //free up dynamic memories
    jsr_StreamingAutomatonDtor(&streaming_automaton);
}

void Test2()
{
    struct timeval begin,end;
    double duration;
    //loading inputs
    JSONStream* stream = jps_createJSONStream("../../dataset/bb.json",1);
    //JSONStream* stream = jps_createJSONStream("bb.json",1);
    //JSONStream* stream = jps_createJSONStream("twitter_store1.txt",1);
    char* path = "$..root..id";

    //loading dfa
    JQ_CONTEXT* ctx = (JQ_CONTEXT*)malloc(sizeof(JQ_CONTEXT));
    JQ_DFA* dfa = dfa_Create(path, ctx);
    if (dfa == NULL) return;
   
    StreamingAutomaton streaming_automaton;
    jsr_StreamingAutomatonCtor(&streaming_automaton, stream, dfa);

    //query execution
    printf("begin executing input JSONPath query\n");
    gettimeofday(&begin,NULL);
    jsr_automaton_execution(&streaming_automaton);

    gettimeofday(&end,NULL);
    duration=1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec;
    printf("the total query execution time is %lf\n", duration/1000000);  
    
    //free up dynamic memories
    jsr_StreamingAutomatonDtor(&streaming_automaton);
}

void Test3()
{
    struct timeval begin,end;
    double duration;
    //loading inputs
    JSONStream* stream = jps_createJSONStream("../../dataset/twitter.json",1);
    //JSONStream* stream = jps_createJSONStream("bb.json",1);
  // JSONStream* stream = jps_createJSONStream("twitter_store1.txt",1);
   // char* path = "$.root[2:5].id";
    char* path = "$.root[*].quoted_status.entities.user_mentions[*].indices[*]";
    //loading dfa
    JQ_CONTEXT* ctx = (JQ_CONTEXT*)malloc(sizeof(JQ_CONTEXT));
    JQ_DFA* dfa = dfa_Create(path, ctx);
    if (dfa == NULL) return;
   
    StreamingAutomaton streaming_automaton;
    jsr_StreamingAutomatonCtor(&streaming_automaton, stream, dfa);

    //query execution
    printf("begin executing input JSONPath query\n");
    gettimeofday(&begin,NULL);
    jsr_automaton_execution(&streaming_automaton);

    gettimeofday(&end,NULL);
    duration=1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec;
    printf("the total query execution time is %lf\n", duration/1000000);  
    
    //free up dynamic memories
    jsr_StreamingAutomatonDtor(&streaming_automaton);
}

void Test4()
{
    struct timeval begin,end;
    double duration;
    //loading inputs
    JSONStream* stream = jps_createJSONStream("../../dataset/rowstest.json",1);
    char* path = "$.meta.view.columns[*].position";

    //loading dfa
    JQ_CONTEXT* ctx = (JQ_CONTEXT*)malloc(sizeof(JQ_CONTEXT));
    JQ_DFA* dfa = dfa_Create(path, ctx);
    if (dfa == NULL) return;
   
    StreamingAutomaton streaming_automaton;
    jsr_StreamingAutomatonCtor(&streaming_automaton, stream, dfa);

    //query execution
    printf("begin executing input JSONPath query\n");
    gettimeofday(&begin,NULL);
    jsr_automaton_execution(&streaming_automaton);

    gettimeofday(&end,NULL);
    duration=1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec;
    printf("the total query execution time is %lf\n", duration/1000000);  
    
    //free up dynamic memories
    jsr_StreamingAutomatonDtor(&streaming_automaton);
}

void Test5()
{
    struct timeval begin,end;
    double duration;
    //loading inputs
    JSONStream* stream = jps_createJSONStream("../../dataset/wiki.json",1);
    char* path = "$.root[*].claims.P150[2:4].mainsnak.property";

    //loading dfa
    JQ_CONTEXT* ctx = (JQ_CONTEXT*)malloc(sizeof(JQ_CONTEXT));
    JQ_DFA* dfa = dfa_Create(path, ctx);
    if (dfa == NULL) return;
   
    StreamingAutomaton streaming_automaton;
    jsr_StreamingAutomatonCtor(&streaming_automaton, stream, dfa);

    //query execution
    printf("begin executing input JSONPath query\n");
    gettimeofday(&begin,NULL);
    jsr_automaton_execution(&streaming_automaton);

    gettimeofday(&end,NULL);
    duration=1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec;
    printf("the total query execution time is %lf\n", duration/1000000);  
    
    //free up dynamic memories
    jsr_StreamingAutomatonDtor(&streaming_automaton);
}

void Test6()
{
    struct timeval begin,end;
    double duration;
    //loading inputs
    JSONStream* stream = jps_createJSONStream("../../dataset/random.json",1);
    char* path = "$.root[*].friends[*].id";

    //loading dfa
    JQ_CONTEXT* ctx = (JQ_CONTEXT*)malloc(sizeof(JQ_CONTEXT));
    JQ_DFA* dfa = dfa_Create(path, ctx);
    if (dfa == NULL) return;
   
    StreamingAutomaton streaming_automaton;
    jsr_StreamingAutomatonCtor(&streaming_automaton, stream, dfa);

    //query execution
    printf("begin executing input JSONPath query\n");
    gettimeofday(&begin,NULL);
    jsr_automaton_execution(&streaming_automaton);

    gettimeofday(&end,NULL);
    duration=1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec;
    printf("the total query execution time is %lf\n", duration/1000000);  
    
    //free up dynamic memories
    jsr_StreamingAutomatonDtor(&streaming_automaton);
}

void Test7()
{
    struct timeval begin,end;
    double duration;
    //loading inputs
    JSONStream* stream = jps_createJSONStream("../../dataset/rowstest.json",1);
    char* path = "$.data[1:3]";

    //loading dfa
    JQ_CONTEXT* ctx = (JQ_CONTEXT*)malloc(sizeof(JQ_CONTEXT));
    JQ_DFA* dfa = dfa_Create(path, ctx);
    if (dfa == NULL) return;
   
    StreamingAutomaton streaming_automaton;
    jsr_StreamingAutomatonCtor(&streaming_automaton, stream, dfa);

    //query execution
    printf("begin executing input JSONPath query\n");
    gettimeofday(&begin,NULL);
    jsr_automaton_execution(&streaming_automaton);

    gettimeofday(&end,NULL);
    duration=1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec;
    printf("the total query execution time is %lf\n", duration/1000000);  
    
    //free up dynamic memories
    jsr_StreamingAutomatonDtor(&streaming_automaton);
}

void Test8()
{
    struct timeval begin,end;
    double duration;
    //loading inputs
    JSONStream* stream = jps_createJSONStream("../../dataset/twitter.json",1);
    //JSONStream* stream = jps_createJSONStream("bb.json",1);
   // JSONStream* stream = jps_createJSONStream("twitter_store1.txt",1);
   // char* path = "$.root[2:5].id";
    char* path = "$.root[*].quoted_status.entities.symbols";

    //loading dfa
    JQ_CONTEXT* ctx = (JQ_CONTEXT*)malloc(sizeof(JQ_CONTEXT));
    JQ_DFA* dfa = dfa_Create(path, ctx);
    if (dfa == NULL) return;
   
    StreamingAutomaton streaming_automaton;
    jsr_StreamingAutomatonCtor(&streaming_automaton, stream, dfa);

    //query execution
    printf("begin executing input JSONPath query\n");
    gettimeofday(&begin,NULL);
    jsr_automaton_execution(&streaming_automaton);

    gettimeofday(&end,NULL);
    duration=1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec;
    printf("the total query execution time is %lf\n", duration/1000000);  
    
    //free up dynamic memories
    jsr_StreamingAutomatonDtor(&streaming_automaton);
}

void Test9()
{
    struct timeval begin,end;
    double duration;
    //loading inputs
    JSONStream* stream = jps_createJSONStream("../../dataset/bb.json",1);
    //JSONStream* stream = jps_createJSONStream("bb.json",1);
    //JSONStream* stream = jps_createJSONStream("twitter_store1.txt",1);
    char* path = "$.root.products[?(@.sku)].categoryPath[1:2]";

    //loading dfa
    JQ_CONTEXT* ctx = (JQ_CONTEXT*)malloc(sizeof(JQ_CONTEXT));
    JQ_DFA* dfa = dfa_Create(path, ctx);
    if (dfa == NULL) return;
   
    StreamingAutomaton streaming_automaton;
    jsr_StreamingAutomatonCtor(&streaming_automaton, stream, dfa);

    //query execution
    printf("begin executing input JSONPath query\n");
    gettimeofday(&begin,NULL);
    jsr_automaton_execution(&streaming_automaton);

    gettimeofday(&end,NULL);
    duration=1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec;
    printf("the total query execution time is %lf\n", duration/1000000);  
    
    //free up dynamic memories
    jsr_StreamingAutomatonDtor(&streaming_automaton);
}

int main()
{
    Test1(); //78
    Test2(); //78
    Test3(); //2
    Test4(); //43
    Test5(); //32
    Test6(); //189
    Test7(); //2
    Test8(); //1
    Test9(); //26
    return 1;
}
