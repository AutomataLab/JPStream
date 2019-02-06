#ifndef __STACK_H__
#define __STACK_H__

#include <stdlib.h>

#define MAX_STACK 100  //max size of stack
#define SYN_STACK 1    //tag for syntax stack
#define QY_STACK 2     //tag for query stack
#define INVALID_ST -1  //invalid state

typedef struct QueryElement{
    int state;
    int count;
}QueryElement;

typedef struct SyntaxStackElement{
    int symbol;
}SyntaxElement;

typedef struct SyntaxStack
{
    int symbol[MAX_STACK];
    int count;
}SyntaxStack;

typedef struct QueryStack
{
    QueryElement item[MAX_STACK];
    int count;
}QueryStack;

static inline void jps_SyntaxStackCtor(SyntaxStack* ss)
{
    ss->count = -1;
}

static inline void jps_QueryStackCtor(QueryStack* qs)
{
    qs->count = -1;
}

static inline void jps_syntaxPush(SyntaxStack* ss, int data)
{
    if(ss->count<MAX_STACK-1)
    {
        ++ss->count;
        ss->symbol[ss->count] = data;
    }
}

static inline void jps_queryPush(QueryStack* qs, QueryElement data)
{
    if(qs->count<MAX_STACK-1)
    {
        ++qs->count;
        qs->item[qs->count] = data;
    }
}

static inline int jps_syntaxPop(SyntaxStack* ss)
{
    int top_symbol; 
    if(ss->count>-1){
        top_symbol = ss->symbol[ss->count];
        --ss->count;
    }
    else
    {
        top_symbol = INVALID_ST;
    }
    return top_symbol;
}

static inline QueryElement jps_queryPop(QueryStack* qs)
{
    QueryElement top_state;
    if(qs->count>-1) {
        top_state = qs->item[qs->count];
        --qs->count;
    }
    else
    {
        QueryElement exception;
        exception.state = INVALID_ST;
        exception.count = 0;
        top_state = exception;
    }
    return top_state;
}

static inline int jps_syntaxTop(SyntaxStack* ss)
{
    if(ss->count>-1) return ss->symbol[ss->count];
    else
    {
        return INVALID_ST;
    }
}

static inline int jps_syntaxSecondTop(SyntaxStack* ss)
{
    if(ss->count>0) return ss->symbol[ss->count-1];
    else
    {
        return INVALID_ST;
    }
}

static inline QueryElement jps_queryTop(QueryStack* qs)
{
    if(qs->count>-1) return qs->item[qs->count];
    else
    {
        QueryElement exception;
        exception.state = INVALID_ST;
        exception.count = 0;
        return exception;
    }
}

static inline void jps_queryTopUpdate(QueryStack* qs, QueryElement data)
{
    qs->item[qs->count] = data;
}

static inline QueryElement jps_querySecondTop(QueryStack* qs)
{
    if(qs->count>0) return qs->item[qs->count-1];
    else
    {
        QueryElement exception;
        exception.state = INVALID_ST;
        exception.count = 0;
        return exception;
    }
}

static inline int jps_syntaxSize(SyntaxStack* ss)
{
    return ss->count+1;
}


#endif // !__STACK_H__
