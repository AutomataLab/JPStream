#ifndef __PREDICATE_H__
#define __PREDICATE_H__

#include "dfa_builder.h"
#include "jsonpath_evaluator.h"
#include "tuple_list.h"
#include "output.h"

#define MAX_PREDICATE_STATE 100
#define MAX_STACK_ELEMENT 100

typedef struct PredicateStateInfo{
    //sub stree for predicate state
    JSONPathNode* sub_tree;  //we'll change its name 
    //condition state list
    JSONPathKeyValuePair* con_state_list; //we'll change its name
    int num_con_state;
}PredicateStateInfo;

typedef struct PredicateStackElement{
    int predicate_state;  
    //start position in output buffer
    int first_candidate_pos;
}PredicateStackElement;

typedef struct PredicateStack{
    PredicateStackElement elements[MAX_STACK_ELEMENT];
    int top_item;
}PredicateStack;

typedef struct PredicateFilter{
    TupleList* tuple_list;
    // mappings from predicate state to predicate conditions
    PredicateStateInfo state_mapping[MAX_PREDICATE_STATE];
}PredicateFilter;

static inline void initPredicateStack(PredicateStack* ps)
{
    ps->top_item = -1;
}

static inline void pushPredicateStack(PredicateStack* ps, PredicateStackElement ps_ele)
{
    int top_item = (++ps->top_item);
    ps->elements[top_item] = ps_ele;
}

static inline PredicateStackElement popPredicateStack(PredicateStack* ps)
{
    return ps->elements[ps->top_item--];
}

static inline PredicateStackElement getTopPredicateStack(PredicateStack* ps)
{
    return ps->elements[ps->top_item];
}

static inline int getPredicateStackSize(PredicateStack* ps)
{
    return ps->top_item+1;
}

static inline void initPredicateFilter(PredicateFilter* pf, TupleList* tl, JSONQueryDFAContext* ctx)  
{
    pf->tuple_list = tl;
    int pred_size = getContextSizeOfPredicateStates(ctx);
    //initialize state mapping table
    for(int i = 0; i<MAX_PREDICATE_STATE; i++)
        pf->state_mapping[i].sub_tree = 0;
    for(int i = 0; i < pred_size; i++)
    {
    	int pred_state = getContextPredicateStates(ctx, i);
        int con_size = getContextSizeOfMapping(ctx, pred_state);
        printJSONQueryDFAContext(ctx);
        
        //get sub tree
        pf->state_mapping[pred_state].sub_tree = getContextSubtree(ctx, pred_state);
        //generate condition state list
        pf->state_mapping[pred_state].con_state_list = (JSONPathKeyValuePair*)malloc((con_size+1)*sizeof(JSONPathKeyValuePair)); 
        pf->state_mapping[pred_state].num_con_state = con_size;
        JSONPathKeyValuePair* con_state_list = pf->state_mapping[pred_state].con_state_list;
        //set default values to all elements in predicate condition list
        //clearKeyValuePair(con_state_list);
        //fill in condition state list (needs another field)
        for(int j=0;j<con_size;j++)
        {
            int con_state = getContextValueOfMapping(ctx, pred_state, j); 
            JSONPathNode* node = getContextSubtree(ctx, con_state);
            con_state_list[j].key = node->string;
            con_state_list[j].state = con_state;
            con_state_list[j].value.boolean = false; 
        }
        con_state_list[con_size].key=NULL;
    }
}

static inline void destroyPredicateFilter(PredicateFilter* pf)
{
    for(int i = 0; i<MAX_PREDICATE_STATE; i++)
    {
        if(pf->state_mapping[i].sub_tree)
        {    
            if(pf->state_mapping[i].con_state_list)  
                free(pf->state_mapping[i].con_state_list);
        }
    }
}

Output* generateFinalOutput(PredicateFilter* pf);
#endif // !__PREDICATE_H__
