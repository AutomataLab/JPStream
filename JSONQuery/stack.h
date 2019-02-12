#ifndef __STACK_H__
#define __STACK_H__

#include <stdlib.h>

#define MAX_STACK 100  //max size of stack
#define SYN_STACK 1    //tag for syntax stack
#define QY_STACK 2     //tag for query stack
#define INVALID -1  //invalid state

typedef struct QueryStackElement{
    int state;
    int count;
    int matched_start; 
}QueryStackElement;

typedef struct SyntaxStack
{
    int symbol[MAX_STACK];
    int top_item;
}SyntaxStack;

typedef struct QueryStack
{
    QueryStackElement item[MAX_STACK];
    int top_item;
}QueryStack;

static inline void initSyntaxStack(SyntaxStack* ss)
{
    ss->top_item = -1;
}

static inline void initQueryStack(QueryStack* qs)
{
    qs->top_item = -1;
}

static inline void syntaxStackPush(SyntaxStack* ss, int data)
{
    if(ss->top_item<MAX_STACK-1)
    {
        ++ss->top_item;
        ss->symbol[ss->top_item] = data;
    }
}

static inline void queryStackPush(QueryStack* qs, QueryStackElement data)
{
    if(qs->top_item<MAX_STACK-1)
    {
        ++qs->top_item;
        qs->item[qs->top_item] = data;
    }
}

static inline int syntaxStackPop(SyntaxStack* ss)
{
    int top_symbol; 
    if(ss->top_item>-1){
        top_symbol = ss->symbol[ss->top_item];
        --ss->top_item;
    }
    else
    {
        top_symbol = INVALID;
    }
    return top_symbol;
}

// pop out QueryStackElement from query stack
static inline QueryStackElement queryStackPop(QueryStack* qs)
{
    QueryStackElement top_state;
    if(qs->top_item>-1) {
        top_state = qs->item[qs->top_item];
        --qs->top_item;
    }
    else
    {
        QueryStackElement exception;
        exception.state = INVALID;
        exception.count = 0;
        top_state = exception;
    }
    return top_state;
}

// get the top element on syntax stack
static inline int syntaxStackTop(SyntaxStack* ss)
{
    if(ss->top_item>-1) return ss->symbol[ss->top_item];
    else
    {
        return INVALID;
    }
}

// get the second top element on syntax stack
static inline int syntaxStackSecondTop(SyntaxStack* ss)
{
    if(ss->top_item>0) return ss->symbol[ss->top_item-1];
    else
    {
        return INVALID;
    }
}

// get the top element on query stack
static inline QueryStackElement queryStackTop(QueryStack* qs)
{
    if(qs->top_item>-1) return qs->item[qs->top_item];
    else
    {
        QueryStackElement exception;
        exception.state = INVALID;
        exception.count = 0;
        exception.matched_start = -1; 
        return exception;
    }
}

// update top element on query stack
static inline void queryStackTopUpdate(QueryStack* qs, QueryStackElement data)
{
    qs->item[qs->top_item] = data;
}

// get the second top element on query stack
static inline QueryStackElement queryStackSecondTop(QueryStack* qs)
{
    if(qs->top_item>0) return qs->item[qs->top_item-1];
    else
    {
        QueryStackElement exception;
        exception.state = INVALID;
        exception.count = 0;
        exception.matched_start = -1; 
        return exception;
    }
}

// get the size of syntax stack
static inline int syntaxStackSize(SyntaxStack* ss)
{
    return ss->top_item+1;
}


#endif // !__STACK_H__
