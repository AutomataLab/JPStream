#ifndef __OUTPUT_H__
#define __OUTPUT_H__

#include "utility.h"

#define MAX_OUTPUT 2000000//800000 //290000


typedef struct Output{
    char** element;
    int count;
    int max_size;
}Output;

static inline void initOutput(Output* output)
{
    output->element = (char**)malloc(MAX_OUTPUT*sizeof(char*));
    output->count = -1;
    output->max_size = MAX_OUTPUT;
}

static inline void destroyOutput(Output* output)
{
    int i;
    for(i=0; i<output->count; i++)
    {   
        if(output->element[i]!=NULL) free(output->element[i]); 
    }
    free(output->element);
}

static inline Output* createOutput()
{
    Output* output = (Output*)malloc(sizeof(Output));
    initOutput(output);
    return output;
}

static inline void freeOutput(Output* output)
{
    destroyOutput(output);
    free(output);
}

static inline void addOutputElement(Output* output, char* text)
{
    if(output->count==output->max_size-1)
    {
        output->element = (char**)realloc(output->element, (2*output->max_size)*sizeof(char*));
        output->max_size = 2*output->max_size;
    }
    int index = (++output->count);
    output->element[index] = text;
    //strcopy(text, output->element[index]); 
    //printf("text %s %d\n", text, index);
}

static inline char* getOutputElement(Output* output, int index)
{
    return output->element[index];
}

static inline void resetOutputElement(Output* output, int index)
{
    output->element[index] = NULL;
}

static inline void removeOutputElement(Output* output, int number)
{
    int i;
    int count = output->count;
    for(i = count; i>count-number; i--)
    {
        free(output->element[i]); 
        output->element[i] = NULL;
    }
    output->count-=number; 
}

static inline int getOutputSize(Output* output)
{
    return output->count+1;
}



#endif // !__OUTPUT_H__
