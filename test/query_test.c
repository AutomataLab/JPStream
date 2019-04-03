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
    struct timeval begin,end;
    double duration;
    //loading inputs
    char* stream = loadJSONStream("../../dataset/bb.json");
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
    executeAutomaton(&streaming_automaton, stream)
    gettimeofday(&end,NULL);
    duration=1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec;
    printf("the total query execution time is %lf\n", duration/1000000);  
    
    //free up dynamic memories
    destroyStreamingAutomaton(&streaming_automaton);
    destoryJSONQueryDFA(dfa); 
}

int main()
{
    Test1();
    return 0;
}

