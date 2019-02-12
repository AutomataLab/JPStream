#ifndef __LIST_H__
#define __LIST_H__

#include "utility.h"

#define MAX_OUTPUTLIST 99000
#define MAX_TEXT 1000

typedef struct List{
    char** element;
    int count;
}List;

static inline void initList(List* list)
{
    list->element = (char**)malloc(MAX_OUTPUTLIST*sizeof(char*));
    int i;
    for(i=0; i<MAX_OUTPUTLIST; i++)
    {
        list->element[i] = (char*)malloc(MAX_TEXT*sizeof(char));
    }
    list->count = -1;
}

static inline void destroyList(List* list)
{
    int i;
    for(i=0; i<MAX_OUTPUTLIST; i++)
    {
        if(list->element[i]!=NULL) free(list->element[i]);
    }
    free(list->element);
}

static inline List* createList()
{
    List* list = (List*)malloc(sizeof(List));
    initList(list);
    return list;
}

static inline void freeList(List* list)
{
    destroyList(list);
    free(list);
}

static inline void addListElement(List* list, char* text)
{
    int index = (++list->count);
    strcopy(text, list->element[index]);
}

static inline void removeListElement(List* list, int number)
{
    list->count-=number;
}

static inline int getListSize(List* list)
{
    return list->count+1;
}



#endif // !__LIST_H__
