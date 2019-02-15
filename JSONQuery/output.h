#ifndef __OUTPUT_H__
#define __OUTPUT_H__

#include "utility.h"

#define MAX_OUTPUT 290000
#define MAX_STRING 500

typedef struct Output{
    char** element;
    int count;
}Output;

static inline void initOutput(Output* output)
{
    output->element = (char**)malloc(MAX_OUTPUT*sizeof(char*));
    int i;
    for(i=0; i<MAX_OUTPUT; i++)
    {
        output->element[i] = (char*)malloc(MAX_STRING*sizeof(char));
    }
    output->count = -1;
}

static inline void destroyOutput(Output* output)
{
    int i;
    for(i=0; i<MAX_OUTPUT; i++)
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
    int index = (++output->count);
    strcopy(text, output->element[index]);
}

static inline char* getOutputElement(Output* output, int index)
{
    return output->element[index];
}

static inline void removeOutputElement(Output* output, int number)
{
    output->count-=number;
}

static inline int getOutputSize(Output* output)
{
    return output->count+1;
}



#endif // !__OUTPUT_H__
