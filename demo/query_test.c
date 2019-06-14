#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/file.h>
#include "path_processor.h"

void Test1()
{
    PathProcessor* path_processor = createPathProcessor("$.root.products[?(@.sku&&@.productId)].categoryPath[1:3]");
    if(path_processor==NULL) return;
    printf("start executing JSONPath query\n");
    Output* output = serialRun(path_processor, "../../dataset/bb.json");
    printf("size of final output is %d\n", getOutputSize(output)); 

    //free up dynamic memories
    freePathProcessor(path_processor);
    freeOutput(output);
}

void Test2()
{
    PathProcessor* path_processor = createPathProcessor("$.root.products[?(@.sku&&@.productId)].categoryPath[1:3]");
    if(path_processor==NULL) return;

    printf("start executing JSONPath query\n");
    int num_core = 16;
    Output* output = parallelRun(path_processor, "../../dataset/bb.json", num_core);
    printf("size of final output is %d\n", getOutputSize(output));
    
    //free up dynamic memories
    freePathProcessor(path_processor);
    freeOutput(output);
}

void Test3()
{
    PathProcessor* path_processor = createPathProcessor("$.root.products[?(@.sku&&@.productId)].categoryPath[1:3]");
    if(path_processor==NULL) return;

    printf("start collecting data constraints\n");
    //collecting data constraints is optional, but can often make the parallel execution more efficient
    ConstraintTable* ct = collectDataConstraints(path_processor, "../../dataset/bb.json");
    printf("finish collecting data constraints\n");

    printf("start executing JSONPath query\n");
    int num_core = 64;
    Output* output = parallelRunOpt(path_processor, "../../dataset/bb.json", num_core, ct);
    printf("size of final output is %d\n", getOutputSize(output));

    //free up dynamic memories
    freeConstraintTable(ct);
    freePathProcessor(path_processor);
    freeOutput(output);
}

void Test4()
{
    PathProcessor* path_processor = createPathProcessor("$.root[*].claims.P150[?(@.id&&@.type)].mainsnak.property");
    if(path_processor==NULL) return;
    printf("start executing JSONPath query\n");
    Output* output = serialRun(path_processor, "../../dataset/wiki.json");
    printf("size of final output is %d\n", getOutputSize(output)); 

    //free up dynamic memories
    //free(input_stream);
    freePathProcessor(path_processor);
    freeOutput(output);
}

void Test5()
{
    PathProcessor* path_processor = createPathProcessor("$.root[*].claims.P150[?(@.id&&@.type)].mainsnak.property");
    if(path_processor==NULL) return;

    printf("start executing JSONPath query\n");
    int num_core = 64;
    Output* output = parallelRun(path_processor, "../../dataset/wiki.json", num_core);
    printf("size of final output is %d\n", getOutputSize(output));
    
    //free up dynamic memories
    freePathProcessor(path_processor);
    freeOutput(output);
}

void Test6()
{
    PathProcessor* path_processor = createPathProcessor("$.root[*].claims.P150[?(@.id&&@.type)].mainsnak.property");
    if(path_processor==NULL) return;

    printf("start collecting data constraints\n");
    //collecting data constraints is optional, but can often make the parallel execution more efficient
    ConstraintTable* ct = collectDataConstraints(path_processor, "../../dataset/wiki.json");
    ///free(train_stream);
    printf("finish collecting data constraints\n");

    printf("start executing JSONPath query\n");
    int num_core = 64;
    Output* output = parallelRunOpt(path_processor, "../../dataset/wiki.json", num_core, ct);
    printf("size of final output is %d\n", getOutputSize(output));

    //free up dynamic memories
    freeConstraintTable(ct);
    ///free(input_stream);
    freePathProcessor(path_processor);
    freeOutput(output);
}

void Test7()
{
    PathProcessor* path_processor = createPathProcessor("$.root[*].quoted_status.entities.user_mentions[0:1].indices[0:1]");
    if(path_processor==NULL) return;

    printf("start collecting data constraints\n");
    //collecting data constraints is optional, but can often make the parallel execution more efficient
    ConstraintTable* ct = collectDataConstraints(path_processor, "twitter_store1.txt");
    ///free(train_stream);
    printf("finish collecting data constraints\n");

    printf("start executing JSONPath query\n");
    int num_core = 16;
    Output* output = parallelRunOpt(path_processor, "twitter_store1.txt", num_core, ct);
    printf("size of final output is %d\n", getOutputSize(output));

    //free up dynamic memories
    freeConstraintTable(ct);
    ///free(input_stream);
    freePathProcessor(path_processor);
    freeOutput(output);
}


int main()
{
   /*Test1();  
   Test2();  
   Test3();  
   Test4(); 
   Test5(); 
   Test6(); */
   Test7();
   return 1;
}
