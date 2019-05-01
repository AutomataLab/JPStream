#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/file.h>
#include "path_processor.h"

void Test1()
{
    char* input_stream = loadInputStream("../../dataset/bb.json");
    PathProcessor* path_processor = createPathProcessor("$.root.products[?(@.sku&&@.productId)].categoryPath[1:3]");
    if(path_processor==NULL) return;
    printf("start executing JSONPath query\n");
    Output* output = serialRun(path_processor, input_stream);
    printf("size of final output is %d\n", getOutputSize(output)); 

    //free up dynamic memories
    free(input_stream);
    freePathProcessor(path_processor);
    freeOutput(output);
}

void Test2()
{
    char* input_stream = loadInputStream("../../dataset/bb.json");
    PathProcessor* path_processor = createPathProcessor("$.root.products[?(@.sku&&@.productId)].categoryPath[1:3]");
    if(path_processor==NULL) return;

    printf("start executing JSONPath query\n");
    int num_core = 64;
    Output* output = parallelRun(path_processor, input_stream, num_core);
    printf("size of final output is %d\n", getOutputSize(output));
    
    //free up dynamic memories
    free(input_stream);
    freePathProcessor(path_processor);
    freeOutput(output);
}

void Test3()
{
    PathProcessor* path_processor = createPathProcessor("$.root.products[?(@.sku&&@.productId)].categoryPath[1:3]");
    if(path_processor==NULL) return;

    printf("start collecting data constraints\n");
    //collecting data constraints is optional, but can often make the parallel execution more efficient
    char* train_stream = loadInputStream("../../dataset/bb.json");
    ConstraintTable* ct = collectDataConstraints(path_processor, train_stream);
    free(train_stream);
    printf("finish collecting data constraints\n");

    printf("start executing JSONPath query\n");
    char* input_stream = loadInputStream("../../dataset/bb.json");
    int num_core = 64;
    Output* output = parallelRunOpt(path_processor, input_stream, num_core, ct);
    printf("size of final output is %d\n", getOutputSize(output));

    //free up dynamic memories
    free(input_stream);
    freePathProcessor(path_processor);
    freeOutput(output);
}

void Test4()
{
    char* input_stream = loadInputStream("../../dataset/wiki.json");
    PathProcessor* path_processor = createPathProcessor("$.root[*].claims.P150[?(@.id&&@.type)].mainsnak.property");
    if(path_processor==NULL) return;
    printf("start executing JSONPath query\n");
    Output* output = serialRun(path_processor, input_stream);
    printf("size of final output is %d\n", getOutputSize(output)); 

    //free up dynamic memories
    free(input_stream);
    freePathProcessor(path_processor);
    freeOutput(output);
}

void Test5()
{
    char* input_stream = loadInputStream("../../dataset/wiki.json");
    PathProcessor* path_processor = createPathProcessor("$.root[*].claims.P150[?(@.id&&@.type)].mainsnak.property");
    if(path_processor==NULL) return;

    printf("start executing JSONPath query\n");
    int num_core = 64;
    Output* output = parallelRun(path_processor, input_stream, num_core);
    printf("size of final output is %d\n", getOutputSize(output));
    
    //free up dynamic memories
    free(input_stream);
    freePathProcessor(path_processor);
    freeOutput(output);
}

void Test6()
{
    PathProcessor* path_processor = createPathProcessor("$.root[*].claims.P150[?(@.id&&@.type)].mainsnak.property");
    if(path_processor==NULL) return;

    printf("start collecting data constraints\n");
    //collecting data constraints is optional, but can often make the parallel execution more efficient
    char* train_stream = loadInputStream("../../dataset/wiki.json");
    ConstraintTable* ct = collectDataConstraints(path_processor, train_stream);
    free(train_stream);
    printf("finish collecting data constraints\n");

    printf("start executing JSONPath query\n");
    char* input_stream = loadInputStream("../../dataset/wiki.json");
    int num_core = 64;
    Output* output = parallelRunOpt(path_processor, input_stream, num_core, ct);
    printf("size of final output is %d\n", getOutputSize(output));

    //free up dynamic memories
    freeConstraintTable(ct);
    free(input_stream);
    freePathProcessor(path_processor);
    freeOutput(output);
}

void Test7()
{
    PathProcessor* path_processor = createPathProcessor("$.root.products[?(@.sku&&@.productId)].categoryPath[1:3]");
    if(path_processor==NULL) return;

    printf("start executing JSONPath query\n");
    StreamingContext ci; 
    initStreamingContext(&ci);
    char* input_stream = NULL;
    Output* output = NULL;
    int start_pos = 0;  //pointer to the starting position of the file
    while(1)
    {
        input_stream = loadBoundedInputStream("../../dataset/bb.json", &start_pos);
        if(input_stream == NULL) break;
        output = serialPartialRun(path_processor, input_stream, &ci);
        free(input_stream);
    }
    printf("size of final output is %d\n", getOutputSize(output));
    //free up dynamic memories
    destroyStreamingContext(&ci);
    freePathProcessor(path_processor);
    freeOutput(output);
}

void Test8()
{
    PathProcessor* path_processor = createPathProcessor("$.root[*].quoted_status.entities.user_mentions[0:1].indices[0:1]");
    if(path_processor==NULL) return;

    printf("start executing JSONPath query\n");
    StreamingContext ci; 
    initStreamingContext(&ci);
    char* input_stream = NULL;
    Output* output = NULL;
    int start_pos = 0;  //pointer to the starting position of the file
    while(1)
    {
        input_stream = loadBoundedInputStream("../../dataset/twitter.json", &start_pos);
        if(input_stream == NULL) break;
        output = serialPartialRun(path_processor, input_stream, &ci);
        free(input_stream);
    }
    printf("size of final output is %d\n", getOutputSize(output));
    //free up dynamic memories
    destroyStreamingContext(&ci);
    freePathProcessor(path_processor);
    freeOutput(output);
}

void Test9()
{
    PathProcessor* path_processor = createPathProcessor("$.root.products[?(@.sku&&@.productId)].categoryPath[1:3]");
    if(path_processor==NULL) return;

    printf("start executing JSONPath query\n");
    StreamingContext ci; 
    initStreamingContext(&ci);
    int num_core = 32;
    char* input_stream = NULL;
    Output* output = NULL;
    int start_pos = 0; //pointer to the starting position of the file
    while(1)
    {
        input_stream = loadBoundedInputStream("../../dataset/bb.json", &start_pos);
        if(input_stream == NULL) break;
        output = parallelPartialRun(path_processor, input_stream, num_core, &ci);
        free(input_stream);
    }
    printf("size of final output is %d\n", getOutputSize(output));
    //free up dynamic memories
    destroyStreamingContext(&ci);
    freePathProcessor(path_processor);
    freeOutput(output);
}

void Test10()
{
    PathProcessor* path_processor = createPathProcessor("$.root[*].quoted_status.entities.user_mentions[0:1].indices[0:1]");
    if(path_processor==NULL) return;

    printf("start executing JSONPath query\n");
    StreamingContext ci; 
    initStreamingContext(&ci);
    int num_core = 32;
    char* input_stream = NULL;
    Output* output = NULL;
    int start_pos = 0; //pointer to the starting position of the file
    while(1)
    {
        input_stream = loadBoundedInputStream("../../dataset/twitter.json", &start_pos);
        if(input_stream == NULL) break;
        output = parallelPartialRun(path_processor, input_stream, num_core, &ci);
        free(input_stream);
    }
    printf("size of final output is %d\n", getOutputSize(output));
    //free up dynamic memories
    destroyStreamingContext(&ci);
    freePathProcessor(path_processor);
    freeOutput(output);
}

void Test11()
{
    PathProcessor* path_processor = createPathProcessor("$.root.products[?(@.sku&&@.productId)].categoryPath[1:3]");
    if(path_processor==NULL) return;

    printf("start collecting data constraints\n");
    //collecting data constraints is optional, but can often make the parallel execution more efficient
    char* train_stream = loadInputStream("../../dataset/bb.json");
    ConstraintTable* ct = collectDataConstraints(path_processor, train_stream);
    free(train_stream);
    printf("finish collecting data constraints\n");

    printf("start executing JSONPath query\n");
    StreamingContext ci; 
    initStreamingContext(&ci);
    int num_core = 16;
    char* input_stream = NULL;
    Output* output = NULL;
    int start_pos = 0;    //pointer to the starting position of the file
    while(1)
    {
        input_stream = loadBoundedInputStream("../../dataset/bb.json", &start_pos);
        if(input_stream == NULL) break;
        output = parallelPartialRunOpt(path_processor, input_stream, num_core, ct, &ci);
        free(input_stream);
    }
    printf("size of final output is %d\n", getOutputSize(output));
    //free up dynamic memories
    freeConstraintTable(ct);
    destroyStreamingContext(&ci);
    freePathProcessor(path_processor);
    freeOutput(output);
}

void Test12()
{
    PathProcessor* path_processor = createPathProcessor("$.root[*].claims.P150[?(@.id&&@.type)].mainsnak.property");
    if(path_processor==NULL) return;

    printf("start collecting data constraints\n");
    //collecting data constraints is optional, but can often make the parallel execution more efficient
    char* train_stream = loadInputStream("../../dataset/wiki.json");
    ConstraintTable* ct = collectDataConstraints(path_processor, train_stream);
    free(train_stream);
    printf("finish collecting data constraints\n");

    printf("start executing JSONPath query\n");
    StreamingContext ci; 
    initStreamingContext(&ci);
    int num_core = 16;
    char* input_stream = NULL;
    Output* output = NULL;
    int start_pos = 0;    //pointer to the starting position of the file
    while(1)
    {
        input_stream = loadBoundedInputStream("../../dataset/wiki.json", &start_pos);
        if(input_stream == NULL) break;
        output = parallelPartialRunOpt(path_processor, input_stream, num_core, ct, &ci);
        free(input_stream);
    }
    printf("size of final output is %d\n", getOutputSize(output));
    //free up dynamic memories
    freeConstraintTable(ct);
    destroyStreamingContext(&ci);
    freePathProcessor(path_processor);
    freeOutput(output);
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
   Test11();
   Test12();
   return 1;
}
