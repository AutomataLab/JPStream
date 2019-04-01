#ifndef __MULTI_STACK_H__
#define __MULTI_STACK_H__

#include <stdlib.h>
#include "dfa_builder.h"
#include "tuple_list.h"
#include "stack.h"

#define MAX_NODE 500 //max node in query stack tree
#define INVALID -1  //invalid state

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
typedef Range QueryStacksElement;

typedef struct QueryStacks
{
    //stack tree information
    TreeNode node[MAX_NODE];
    int num_node;
    int parent_pointers[MAX_NODE];
    int num_parent_pointers;
    int root_pointers[MAX_NODE];
    int num_root_pointers;
    //pointers for nodes that access all possible state information in different levels
    QueryStacksElement item[MAX_STACK];
    int top_item;
}QueryStacks;

static inline QueryStacksElement initQueryStacks(QueryStacks* qs, int* start_states, int num_start_states)
{
    qs->top_item = -1;
    qs->num_node = 0;
    qs->num_parent_pointers = 0;
    qs->num_root_pointers = 0;
    int i;
    for(i = 0; i<num_start_states; i++)
    {
        int node_index = (qs->num_node++);
        qs->node[node_index].query_state = start_states[i];  //printf("start_states %d\n", start_states[i]);
        qs->node[node_index].count = 0;
        qs->node[node_index].matched_start = INVALID;
        qs->node[node_index].parent_range.start = 0; 
        qs->node[node_index].parent_range.end = -1; 
        qs->node[node_index].root_range.start = 0; 
        qs->node[node_index].root_range.end = -1; 
        qs->node[node_index].first_tuple_index = -1;
        qs->node[node_index].last_tuple_index = -1; 
    } //printf("\n");
    QueryStacksElement qs_elt;
    qs_elt.start = 0;
    qs_elt.end = num_start_states - 1;
    return qs_elt;
}

static inline QueryStacksElement pruneQueryPaths(QueryStacks* qs, int* start_states, int num_start_states)
{
    QueryStacks back_qs = *(qs);
    qs->num_node = 0;
    int i;
    for(i = 0; i<num_start_states; i++)
    {
        int node_index = (qs->num_node++);
        qs->node[node_index] = back_qs.node[start_states[i]]; 
    }
    QueryStacksElement qs_elt;
    qs_elt.start = 0;
    qs_elt.end = num_start_states - 1;
    return qs_elt; 
}

static inline QueryStacksElement queryStacksPush(QueryStacks* qs, QueryStacksElement qs_elt, JSONQueryDFA* qa, char* key_string)
{
    //push current states into stack
    qs->item[++qs->top_item] = qs_elt;
    //iterate through the current states, and move on to the next states
    QueryStacksElement next_qs_elt;
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
        int next_state;
       
        JSONQueryIndexPair pair = getDFAArrayIndex(qa, cur_state);
        int lower = pair.lower;
        int upper = pair.upper;
        int counter = qs->node[i].count;
        if(qs->node[i].root_range.end==-1)
        {
            if(!((lower==0&&upper==0)||(counter<upper))) next_state = 0;
            else next_state = dfaNextStateByStr(qa, cur_state, key_string); 
        }
        else{
            if(!((lower==0&&upper==0)||(counter>=lower && counter<upper))) next_state = 0; 
            else next_state = dfaNextStateByStr(qa, cur_state, key_string);
        }

        //check whether a new node needs to be created for next_state
        for(j = next_qs_elt.start; j<=next_qs_elt.end; j++)
        {
            //node exists
            if(qs->node[j].query_state == next_state)
            {
                //add parent pointer
                parent_pointer[next_state][num_parent_pointer[next_state]++] = i;//cur_state;
                ///printf("parent %d %d\n", num_parent_pointer[next_state], parent_pointer[next_state][num_parent_pointer[next_state]]);
                //add root pointer from its parent
                Range p_root_range = qs->node[i].root_range;
                //parent node is the root node
                if(p_root_range.end == -1)
                {
                    root_pointer[next_state][num_root_pointer[next_state]++] = i;//cur_state;
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
            //printf("parent %d %d %d %d %d\n", num_parent_pointer[next_state], parent_pointer[next_state][num_parent_pointer[next_state]-1], i, cur_state, next_state);
            //add root pointer from its parent
            Range p_root_range = qs->node[i].root_range;
            //parent node is the root node
            if(p_root_range.end == -1)
            {
                root_pointer[next_state][num_root_pointer[next_state]++] = i;//cur_state;
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

// pop out QueryStacksElement from query stack
static inline QueryStacksElement queryStacksPop(QueryStacks* qs)
{   
    QueryStacksElement top_qs_elt;
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
static inline void printQueryStacks(QueryStacks* qs, QueryStacksElement qs_elt)
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
    
    /*printf("\nprint output information\n");
    for(i = 0; i<qs->num_root_pointers; i++)
    {
        int root_index = qs->root_pointers[i];
        if(qs->node[root_index].first_tuple_index>=0)
        {
            printf("%dth root index is %d; ", i, root_index);
            int current = qs->node[root_index].first_tuple_index;
            while(current<=qs->node[root_index].last_tuple_index)
            {
                printf("output is %s\n",getTuple(tl, current_index); 
                current++;
            }
        }
    }*/ 
}

static inline void printTupleInfo(QueryStacks* qs, QueryStacksElement qs_elt, TupleList* tl)
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

// get the top element on query stack
static inline QueryStacksElement queryStacksTop(QueryStacks* qs)
{
    if(qs->top_item>-1) return qs->item[qs->top_item];
    else
    {
        QueryStacksElement exception;
        exception.end = INVALID;
        return exception;
    }
}

// update top element on query stack
static inline void queryStacksTopUpdate(QueryStacks* qs, QueryStacksElement data)
{
    qs->item[qs->top_item] = data;
}

// get the second top element on query stack
static inline QueryStacksElement queryStacksSecondTop(QueryStacks* qs)
{
    if(qs->top_item>0) return qs->item[qs->top_item-1];
    else
    {
        QueryStacksElement exception;
        exception.end = INVALID;
        return exception;
    }
}

//add tuple information into tuple list and corresponding root node on stack tree, works for primitive matches
static inline void addTupleInfo(QueryStacks* qs, int node_index, int query_state, char* text_content, TupleList* tl)
{ 
     //iterate through all root nodes
     int root_s = qs->node[node_index].root_range.start;
     int root_e = qs->node[node_index].root_range.end;

     while(root_s<=root_e)
     {
         int root_index = qs->root_pointers[root_s];
         TreeNode root = qs->node[root_index];
         addTuple(tl, query_state, text_content);
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
         addTuple(tl, query_state, text_content);
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

//similar function as addTupleInfo, but only works for object or array matches
static inline void addVirtualTupleInfo(QueryStacks* qs, int node_index, int query_state, int matched_start, int matched_end, TupleList* tl)
{ 
     //iterate through all root nodes
     int root_s = qs->node[node_index].root_range.start;
     int root_e = qs->node[node_index].root_range.end;

     while(root_s<=root_e)
     {
         int root_index = qs->root_pointers[root_s];
         TreeNode root = qs->node[root_index];
         addVirtualTuple(tl, query_state, matched_start, matched_end);
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
         addVirtualTuple(tl, query_state, matched_start, matched_end);
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

static inline void combineQueryStacks(QueryStacks* dst_qs, QueryStacksElement* dst_qs_elt, QueryStacks* src_qs, QueryStacksElement* src_qs_elt)
{
    int i;
    src_qs->item[++src_qs->top_item] = (*src_qs_elt);
    int top_src = src_qs->top_item;
    int query_state = dst_qs->node[dst_qs_elt->start].query_state;
   ///// printf("top src is %d\n", top_src);
    if(top_src>=0)  //merge src_qs into dst_qs
    {
        //deal with the root level in src_qs
        int root_index = -1;
        QueryStacksElement root_qs_elt = src_qs->item[0];         
        int root_start = root_qs_elt.start;
        int root_end = root_qs_elt.end; 
        while(root_start<=root_end)
        {
            if(src_qs->node[root_start].query_state == query_state)
            {
                root_index = root_start;
                //combine some root information if needed later  --todo list
                break;
            } 
            root_start++;
        }
        if(root_index!=-1)
        {
            //use root info to find the correct paths in src_qs
            for(i = 1; i<=top_src; i++)
            {
                dst_qs->item[++dst_qs->top_item] = (*dst_qs_elt);
                QueryStacksElement src_qs_elt = src_qs->item[i];
                int start = src_qs_elt.start;
                int end = src_qs_elt.end;
                while(start<=end)
                {
                    root_start = src_qs->node[start].root_range.start;
                    root_end = src_qs->node[start].root_range.end;
                    while(root_start<=root_end)
                    {
                        if(src_qs->root_pointers[root_start]==root_index)
                        {
                            int node_index = (dst_qs->num_node++); 
                            dst_qs->node[node_index].query_state = src_qs->node[start].query_state;
                            dst_qs->node[node_index].count = src_qs->node[start].count;
                            dst_qs->node[node_index].matched_start = src_qs->node[start].matched_start;
                            dst_qs_elt->start = node_index;
                            dst_qs_elt->end = node_index;
                            break;
                        }
                        root_start++;
                    }
                    if(root_start<=root_end)
                    {
                        break;
                    }
                    start++;
                }
            }
        }
    }
}

#endif // !__MULTI_STACK_H__
