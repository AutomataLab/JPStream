#ifndef __PREDICATE_H__
#define __PREDICATE_H__

#include "dfa_builder.h"
#include "jsonpath_evaluator.h"
#include "tuple_list.h"
#include "output.h"

#define MAX_PREDICATE_STATE 100
#define MAX_STACK_ELEMENT 100

typedef struct PredicateStackElement{
    int predicate_state;  
    //point to the first candidate
    int output_buffer_pointer;  
}PredicateStackElement;

typedef struct PredicateStack{
    PredicateStackElement elements[MAX_STACK_ELEMENT];
    int top_element;  
}PredicateStack;

typedef struct PredicateFilter{
    TupleList* tuple_list;
    //context information for dfa table
    JSONQueryDFAContext* ctx;
    //condition list for each predicate state
    PredicateCondition* condition_list[MAX_PREDICATE_STATE];
}PredicateFilter;

static inline void initPredicateStack(PredicateStack* ps)
{
    ps->top_element = -1;
}

static inline void pushPredicateStack(PredicateStack* ps, PredicateStackElement ps_elt)
{
    int top_element = (++ps->top_element);
    ps->elements[top_element] = ps_elt;
}

static inline PredicateStackElement popPredicateStack(PredicateStack* ps)
{
    return ps->elements[ps->top_element--];
}

static inline PredicateStackElement getTopPredicateStack(PredicateStack* ps)
{
    return ps->elements[ps->top_element];
}

static inline int getPredicateStackSize(PredicateStack* ps)
{
    return ps->top_element+1;
}

static inline void initPredicateFilter(PredicateFilter* pf, TupleList* tl, JSONQueryDFAContext* ctx)  
{
    pf->tuple_list = tl;
    pf->ctx = ctx;
    int pred_size = getContextSizeOfPredicateStates(ctx);
    //initialize state mapping table
    for(int i = 0; i<MAX_PREDICATE_STATE; i++)
        pf->condition_list[i] = NULL;
    for(int i = 0; i < pred_size; i++)
    {
    	int pred_state = getContextPredicateStates(ctx, i);
        int con_size = getContextSizeOfMapping(ctx, pred_state);

        //generate condition list for each predicate state
        pf->condition_list[pred_state] = (PredicateCondition*)malloc((con_size+1)*sizeof(PredicateCondition)); 
        PredicateCondition* cl = pf->condition_list[pred_state];
        for(int j=0;j<con_size;j++)
        {
            int con_state = getContextValueOfMapping(ctx, pred_state, j); 
            ASTNode* node = getContextSubtree(ctx, con_state);
            cl[j].name = node->string; 
            cl[j].text = NULL;
        }
        cl[con_size].name=NULL;
    }
}

static inline void destroyPredicateFilter(PredicateFilter* pf)
{
    for(int i = 0; i<MAX_PREDICATE_STATE; i++)
    {
        if(pf->condition_list[i]!=NULL)
        {    
            free(pf->condition_list[i]);
        }
    } 
}

Output* generateFinalOutput(PredicateFilter* pf);
#endif // !__PREDICATE_H__
