#ifndef __STACK_P_H__
#define __STACK_P_H__

#include <stdlib.h>
#include "dfa_builder.h"
#include "tuple_list.h"

#define MAX_STACK 100  //max size of syntax stack
#define MAX_NODE 500 //max node in query stack tree
#define INVALID -1  //invalid state

typedef struct SyntaxStack
{
    int symbol[MAX_STACK];
    int top_item;
}SyntaxStack;

typedef struct Range{
    int start;
    int end;
}Range;

typedef struct TreeNode{
    //query state
    int query_state;    
    //array counter
    int count;          
    //matched starting position of object or array
    int matched_start;  
    //pointer to parent node 
    Range parent_range; 
    //pointer to root node
    Range root_range;
    //pointer to the first tuple
    int first_tuple_index;
    //pointer to the second tuple
    int last_tuple_index;
}TreeNode;

//points to current states on query tree
typedef Range QueryStackElement;

typedef struct QueryStack
{
    //stack tree information
    TreeNode node[MAX_NODE];
    int num_node;
    int parent_pointers[MAX_NODE];
    int num_parent_pointers;
    int root_pointers[MAX_NODE];
    int num_root_pointers;
    //pointers for nodes that access all possible state information in different levels
    QueryStackElement item[MAX_STACK];
    int top_item;
}QueryStack;

static inline void initSyntaxStack(SyntaxStack* ss)
{
    ss->top_item = -1;
}

static inline QueryStackElement initQueryStack(QueryStack* qs, int* start_states, int num_start_states)
{
    qs->top_item = -1;
    qs->num_node = 0;
    qs->num_parent_pointers = 0;
    qs->num_root_pointers = 0;
    int i;
    for(i = 0; i<num_start_states; i++)
    {
        int node_index = (qs->num_node++);
        qs->node[node_index].query_state = start_states[i];
        qs->node[node_index].count = 0;
        qs->node[node_index].matched_start = INVALID;
        qs->node[node_index].parent_range.start = 0; 
        qs->node[node_index].parent_range.end = -1; 
        qs->node[node_index].root_range.start = 0; 
        qs->node[node_index].root_range.end = -1; 
        qs->node[node_index].first_tuple_index = -1;
        qs->node[node_index].last_tuple_index = -1; 
    }
    QueryStackElement qs_elt;
    qs_elt.start = 0;
    qs_elt.end = num_start_states - 1;
    return qs_elt;
}

static inline void syntaxStackPush(SyntaxStack* ss, int data)
{
    if(ss->top_item<MAX_STACK-1)
    {
        ++ss->top_item;
        ss->symbol[ss->top_item] = data;
    }
}

static inline QueryStackElement queryStackPush(QueryStack* qs, QueryStackElement qs_elt, JSONQueryDFA* qa, char* key_string)
{
    //push current states into stack
    qs->item[++qs->top_item] = qs_elt;
    //iterate through the current states, and move on to the next states
    QueryStackElement next_qs_elt;
    next_qs_elt.start = qs_elt.end + 1;
    next_qs_elt.end = -1;
    int i, j, k;
    /*
      during each iteration, parent pointers and root pointers for one single node might not be placed in consecutive position,
      so we need to record them temporarily, and update stack tree after iteration
    */
    int parent_pointer[MAX_NODE][MAX_NODE];
    int num_parent_pointer[MAX_NODE];

    int root_pointer[MAX_NODE][MAX_NODE];
    int num_root_pointer[MAX_NODE];

    for(i = qs_elt.start; i<=qs_elt.end; i++)
    {
        int cur_state = qs->node[i].query_state;
        int next_state = dfaNextStateByStr(qa, cur_state, key_string);
        //check whether a new node needs to be created for next_state
        for(j = next_qs_elt.start; j<=next_qs_elt.end; j++)
        {
            //node exists
            if(qs->node[j].query_state == next_state)
            {
                //add parent pointer
                parent_pointer[next_state][num_parent_pointer[next_state]++] = i;
                //add root pointer from its parent
                Range p_root_range = qs->node[i].root_range;
                //parent node is the root node
                if(p_root_range.end == -1)
                {
                    root_pointer[next_state][num_root_pointer[next_state]++] = cur_state;
                }
                else
                {   
                    for(k = p_root_range.start; k<=p_root_range.end; k++)
                    {
                        root_pointer[next_state][num_root_pointer[next_state]++] = qs->root_pointers[k];
                    }
                }
                break;
            }
        }
        //create new node
        if(j>next_qs_elt.end) 
        {
            int node_index = (qs->num_node++);
            qs->node[node_index].query_state = next_state;
            qs->node[node_index].count = 0;
            qs->node[node_index].matched_start = INVALID;
            qs->node[node_index].parent_range.start = 0; 
            qs->node[node_index].parent_range.end = -1;
            qs->node[node_index].root_range.start = 0; 
            qs->node[node_index].root_range.end = -1;
            next_qs_elt.end = node_index;
            num_parent_pointer[next_state] = 0;
            num_root_pointer[next_state] = 0;
            //add parent pointer
            parent_pointer[next_state][num_parent_pointer[next_state]++] = i;
            //add root pointer from its parent
            Range p_root_range = qs->node[i].root_range;
            //parent node is the root node
            if(p_root_range.end == -1)
            {
                root_pointer[next_state][num_root_pointer[next_state]++] = cur_state;
            }
            else
            {
                for(k = p_root_range.start; k<=p_root_range.end; k++)
                {
                    root_pointer[next_state][num_root_pointer[next_state]++] = qs->root_pointers[k];
                }
            }
        } 
    }
    //update parent and root pointers for each new node
    for(i = next_qs_elt.start; i<=next_qs_elt.end; i++)
    {
        int query_state = qs->node[i].query_state;
        qs->node[i].parent_range.start = qs->num_parent_pointers;
        qs->node[i].parent_range.end = qs->num_parent_pointers-1;
        for(j = 0; j<num_parent_pointer[query_state]; j++)
        { 
            int parent_index = (qs->num_parent_pointers++);
            qs->parent_pointers[parent_index] = parent_pointer[query_state][j];  
            qs->node[i].parent_range.end = parent_index;
        }
        qs->node[i].root_range.start = qs->num_root_pointers;
        qs->node[i].root_range.end = qs->num_root_pointers-1;
        for(j = 0; j<num_root_pointer[query_state]; j++)
        { 
            int root_index = (qs->num_root_pointers++); 
            qs->root_pointers[root_index] = root_pointer[query_state][j];
            qs->node[i].root_range.end = root_index;
        }
    }
    return next_qs_elt;
}

// pop out QueryStackElement from query stack
static inline QueryStackElement queryStackPop(QueryStack* qs)
{   
    QueryStackElement top_qs_elt;
    if(qs->top_item>-1) {
        //remove top element from query tree
        top_qs_elt = qs->item[qs->top_item];
        qs->num_node = top_qs_elt.end+1;
        int last_elt_index = top_qs_elt.end;
        qs->num_parent_pointers = qs->node[last_elt_index].parent_range.end+1;
        qs->num_root_pointers = qs->node[last_elt_index].root_range.end+1;
        --qs->top_item;
    }
    else top_qs_elt.end = INVALID;
    return top_qs_elt;
}

//just used for debugging
static inline void printQueryStack(QueryStack* qs, QueryStackElement qs_elt)
{
    int i;
    printf("print tree information\n");
    printf("node information, number of nodes %d\n", qs->num_node);
    for(i = 0; i<qs->num_node; i++)
    {
        TreeNode node = qs->node[i];
        printf("%dth node query state is %d; parent start index %d parend end index %d; root start index %d root end index %d;\n", i, node.query_state, node.parent_range.start, node.parent_range.end, node.root_range.start, node.root_range.end);
    }
    printf("\nparent pointer information, number of pointers %d\n", qs->num_parent_pointers);
    for(i = 0; i<qs->num_parent_pointers; i++)
    {
        int parent_index = qs->parent_pointers[i];
        printf("%dth parent index is %d; ", i, parent_index);
    }
    printf("\nroot pointer information, number of roots %d\n", qs->num_root_pointers);
    for(i = 0; i<qs->num_root_pointers; i++)
    {
        int root_index = qs->root_pointers[i];
        printf("%dth root index is %d; ", i, root_index);
    }
    printf("\ncurrent state information start_node_index %d end_node_index %d\n", qs_elt.start, qs_elt.end);
}

static inline void printTupleInfo(QueryStack* qs, QueryStackElement qs_elt, TupleList* tl)
{
    printf("\nprint output information\n");
    int i;
    for(i = 0; i<qs->num_root_pointers; i++)
    {
        int root_index = qs->root_pointers[i];
        if(qs->node[root_index].first_tuple_index>=0)
        {
            printf("%dth root index is %d; ", i, root_index);
            int current = qs->node[root_index].first_tuple_index;
            while(current<=qs->node[root_index].last_tuple_index)
            {
                printf("output is %s\n",getTuple(tl, current).text);
                current++;
            }
        }
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
        exception.end = INVALID;
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
        exception.end = INVALID;
        return exception;
    }
}

//add tuple information into tuple list and corresponding root node on stack tree
static inline void addTupleInfo(QueryStack* qs, int node_index, int query_state, char* text_content, TupleList* tl)
{
     addTuple(tl, query_state, text_content);
     int tl_index = getTupleListSize(tl)-1;
     //iterate through all root nodes
     int root_s = qs->node[node_index].root_range.start;
     int root_e = qs->node[node_index].root_range.end;

     while(root_s<=root_e)
     {
         int root_index = qs->root_pointers[root_s];
         TreeNode root = qs->node[root_index];
         if(root.first_tuple_index>=0){
             int tuple_index = getTupleListSize(tl)-1;
             linkTuples(tl, root.last_tuple_index, tuple_index);
             qs->node[root_index].last_tuple_index = tuple_index;
         }
         else{
             int tuple_index = getTupleListSize(tl)-1;
             qs->node[root_index].first_tuple_index = tuple_index;
             qs->node[root_index].last_tuple_index = tuple_index;
         }
         root_s++;
     }
     //node_index represents a root node
     if(root_e==-1)
     {
         int root_index = node_index;
         TreeNode root = qs->node[root_index];
         if(root.first_tuple_index>=0){
             int tuple_index = getTupleListSize(tl)-1;
             linkTuples(tl, root.last_tuple_index, tuple_index);
             qs->node[root_index].last_tuple_index = tuple_index;
         }
         else{
             int tuple_index = getTupleListSize(tl)-1;
             qs->node[root_index].first_tuple_index = tuple_index;
             qs->node[root_index].last_tuple_index = tuple_index;
         }
     }
}

// get the size of syntax stack
static inline int syntaxStackSize(SyntaxStack* ss)
{
    return ss->top_item+1;
}


#endif // !__STACK_P_H__
