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
    ASTNode* sub_tree;  //we'll change its name 
    //condition state list
    JSONPathKeyValuePair* con_state_list; //we'll change its name
    int num_con_state;
}PredicateStateInfo;

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
    // mappings from predicate state to predicate conditions
    //PredicateStateInfo state_mapping[MAX_PREDICATE_STATE];
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
      //  printJSONQueryDFAContext(ctx);
       
        //printf("index %d pred state is %d size %d\n", i, pred_state, con_size); 
        //get sub tree
        //pf->state_mapping[pred_state].sub_tree = getContextSubtree(ctx, pred_state);
        //generate condition state list
        pf->condition_list[pred_state] = (PredicateCondition*)malloc((con_size+1)*sizeof(PredicateCondition)); 
        //pf->state_mapping[pred_state].num_con_state = con_size;
        PredicateCondition* condition_list = pf->condition_list[pred_state];
        //set default values to all elements in predicate condition list
        //clearKeyValuePair(con_state_list);
        //fill in condition state list (needs another field)
        for(int j=0;j<con_size;j++)
        {
            int con_state = getContextValueOfMapping(ctx, pred_state, j); 
            ASTNode* node = getContextSubtree(ctx, con_state);
            condition_list[j].name = node->string; //printf("pred_size %d pred_state %d name %s\n", pred_size, pred_state, condition_list[j].name);
            condition_list[j].text = NULL;
            //con_state_list[j].value.boolean = false; 
        }
        condition_list[con_size].name=NULL;
    }
}

static inline void destroyPredicateFilter(PredicateFilter* pf)
{
    for(int i = 0; i<MAX_PREDICATE_STATE; i++)
    {
        if(pf->condition_list[i])
        {    
            free(pf->condition_list[i]);
            //if(pf->state_mapping[i].con_state_list)  
              //  free(pf->state_mapping[i].con_state_list);
        }
    } 
}

Output* generateFinalOutput(PredicateFilter* pf);
#endif // !__PREDICATE_H__
