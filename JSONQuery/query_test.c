#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/file.h>
#include "streaming_automaton.h"

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
    struct timeval begin,end;
    double duration;
    //loading inputs
    char* stream = loadJSONStream("../../dataset/bb.json");
    //JSONStream* stream = jps_createJSONStream("bb.json",1);
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
   //JSONStream* stream = jps_createJSONStream("twitter_store1.txt",1);
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
    char* path = "$.root.products[?(@.sku)].categoryPath[1:2]";

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
