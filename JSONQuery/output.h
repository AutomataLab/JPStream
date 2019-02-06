#ifndef __OUTPUT_H__
#define __OUTPUT_H__

#include "basic.h"

#define MAX_OUTPUTLIST 239000
#define MAX_TEXT 500

typedef struct OutputList{
    char** element;
    int count;
}OutputList;

static inline void jpo_OutputListCtor(OutputList* list)
{
    list->element = (char**)malloc(MAX_OUTPUTLIST*sizeof(char*));
    int i;
    for(i=0; i<MAX_OUTPUTLIST; i++)
    {
        list->element[i] = (char*)malloc(MAX_TEXT*sizeof(char));
    }
    list->count = -1;
}

static inline void jpo_OutputListDtor(OutputList* list)
{
    int i;
    for(i=0; i<MAX_OUTPUTLIST; i++)
    {
        if(list->element[i]!=NULL) free(list->element[i]);
    }
    free(list->element);
}

static inline OutputList* jpo_createOutputList()
{
    OutputList* list = (OutputList*)malloc(sizeof(OutputList));
    jpo_OutputListCtor(list);
    return list;
}

static inline void jpo_freeOutputList(OutputList* list)
{
    jpo_OutputListDtor(list);
    free(list);
}

static inline void jpo_addElement(OutputList* list, char* text)
{
    int index = (++list->count);
    strcopy(text, list->element[index]);
}

static inline void jpo_removeLastElement(OutputList* list)
{
    --list->count;
}

static inline void jpo_removeLastnElements(OutputList* list, int number)
{
    list->count-=number;
}

static inline int jpo_getSize(OutputList* list)
{
    return list->count+1;
}



#endif // !__OUTPUT_H__
