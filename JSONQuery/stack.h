#ifndef __STACK_H__
#define __STACK_H__

#include <stdlib.h>

#define MAX_STACK 100  //max size of stack
#define SYN_STACK 1    //tag for syntax stack
#define QY_STACK 2     //tag for query stack
#define INVALID_ST -1  //invalid state

typedef struct QueryStackElement{
    int state;
    int count;
    int start_obj;
    int end_obj;
}QueryStackElement;

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
    QueryStackElement item[MAX_STACK];
    int count;
}QueryStack;

static inline void initSyntaxStack(SyntaxStack* ss)
{
    ss->count = -1;
}

static inline void initQueryStack(QueryStack* qs)
{
    qs->count = -1;
}

static inline void syntaxStackPush(SyntaxStack* ss, int data)
{
    if(ss->count<MAX_STACK-1)
    {
        ++ss->count;
        ss->symbol[ss->count] = data;
    }
}

static inline void queryStackPush(QueryStack* qs, QueryStackElement data)
{
    if(qs->count<MAX_STACK-1)
    {
        ++qs->count;
        qs->item[qs->count] = data;
    }
}

static inline int syntaxStackPop(SyntaxStack* ss)
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

static inline QueryStackElement queryStackPop(QueryStack* qs)
{
    QueryStackElement top_state;
    if(qs->count>-1) {
        top_state = qs->item[qs->count];
        --qs->count;
    }
    else
    {
        QueryStackElement exception;
        exception.state = INVALID_ST;
        exception.count = 0;
        top_state = exception;
    }
    return top_state;
}

static inline int syntaxStackTop(SyntaxStack* ss)
{
    if(ss->count>-1) return ss->symbol[ss->count];
    else
    {
        return INVALID_ST;
    }
}

static inline int syntaxStackSecondTop(SyntaxStack* ss)
{
    if(ss->count>0) return ss->symbol[ss->count-1];
    else
    {
        return INVALID_ST;
    }
}

static inline QueryStackElement queryStackTop(QueryStack* qs)
{
    if(qs->count>-1) return qs->item[qs->count];
    else
    {
        QueryStackElement exception;
        exception.state = INVALID_ST;
        exception.count = 0;
        exception.start_obj = -1;
        exception.end_obj = -1;
        return exception;
    }
}

static inline void queryStackTopUpdate(QueryStack* qs, QueryStackElement data)
{
    qs->item[qs->count] = data;
}

static inline QueryStackElement queryStackSecondTop(QueryStack* qs)
{
    if(qs->count>0) return qs->item[qs->count-1];
    else
    {
        QueryStackElement exception;
        exception.state = INVALID_ST;
        exception.count = 0;
        exception.start_obj = -1;
        exception.end_obj = -1;
        return exception;
    }
}

static inline int syntaxStackSize(SyntaxStack* ss)
{
    return ss->count+1;
}


#endif // !__STACK_H__
