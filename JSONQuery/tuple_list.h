#ifndef __TUPLE_LIST_H__
#define __TUPLE_LIST_H__

#include "utility.h"

#define MAX_TUPLE_LIST 5000000
#define MAX_TEXT 500

typedef struct Tuple{
    int start_position;
    int end_position;
    int state;
    char text[MAX_TEXT];
    int next_index;
}Tuple;

typedef struct TupleList{
    Tuple element[MAX_TUPLE_LIST];
    int count;
}TupleList;

static inline void initTupleList(TupleList* tl)
{
    //tl->element = (Tuple*)malloc(MAX_TUPLE_LIST*sizeof(Tuple));
    tl->count = -1;
}

static inline TupleList* createTupleList()
{
    TupleList* tl = (TupleList*)malloc(sizeof(TupleList));
    initTupleList(tl);
    return tl;
}

static inline void freeTupleList(TupleList* tl)
{
    free(tl);
}

static inline void addTuple(TupleList* tl, int state, char* text)
{ 
    int index = (++tl->count);
    tl->element[index].state = state;
    tl->element[index].start_position = -1;
    strcopy(text, tl->element[index].text); 
    tl->element[index].next_index  = -1;
}

static inline void addVirtualTuple(TupleList* tl, int state, int start_position, int end_position)
{//printf("<<<<<<<<start %d end %d\n", start_position, end_position);
    int index = (++tl->count);
    tl->element[index].state = state;
    tl->element[index].start_position = start_position; 
    tl->element[index].end_position = end_position;
    tl->element[index].next_index  = -1;
}

static inline void linkTuples(TupleList* tl, int current_index, int next_index)
{
    tl->element[current_index].next_index = next_index;
}

static inline Tuple getTuple(TupleList* tl, int index)
{
    return tl->element[index];
}

static inline Tuple getNextTuple(TupleList* tl, int current_index)
{
    int next_index = tl->element[current_index].next_index;
    return tl->element[next_index];
}

static inline int getTupleListSize(TupleList* tl)
{
    return tl->count+1;
}
#endif // !__TUPLE_LIST_H__
