#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/file.h>
#include "streaming_automaton.h"

int main()
{
    struct timeval begin,end;
    double duration;
    //loading inputs
    //JSONStream* stream = jps_createJSONStream("../../dataset/twitter.json",1);
    JSONStream* stream = jps_createJSONStream("twitter_store1.txt",1);
    char* path = "$.root.products[*].categoryPath[*].id";

    //loading dfa
    JQ_CONTEXT* ctx = (JQ_CONTEXT*)malloc(sizeof(JQ_CONTEXT));
    JQ_DFA* dfa = dfa_Create(path, ctx);
    if (dfa == NULL) return 0;
   
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
    return 1;
}
