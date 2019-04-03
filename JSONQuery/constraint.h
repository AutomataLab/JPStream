#ifndef __CONSTRAINT_H__
#define __CONSTRAINT_H__

#include <stdlib.h>
#include "lexing.h"

#define MAX_KEY_TEXT 100
#define MAX_CONSTRAINT 1000
#define MAX_STATES 100
#define INVALID -1

typedef struct ConstraintInfo{
    char token_name[MAX_KEY_TEXT];
    int type;
    int state_set[MAX_STATES];
    int num_state;
}ConstraintInfo;

typedef struct ConstraintTable{
    ConstraintInfo constraint_info[MAX_CONSTRAINT];
    int num_constraint_info;
}ConstraintTable;

static inline void initConstraintTable(ConstraintTable* ct)
{
    ct->num_constraint_info = 0;
}

static inline ConstraintTable* createConstraintTable()
{
    ConstraintTable* ct = (ConstraintTable*)malloc(sizeof(ConstraintTable));
    initConstraintTable(ct);
    return ct;
}

static inline void updateStateInfo(ConstraintTable* ct, ConstraintInfo* ci)
{
    int type = ci->type;
    ci->num_state = INVALID;
    if(type==KY)  //key field
    {
        int i = 0;
        int last = ct->num_constraint_info;
        char* constraint_key_name = ci->token_name;
        while(i<last)
        {
            if(strcmp(constraint_key_name, ct->constraint_info[i].token_name)==0)
            {
                ci->num_state = ct->constraint_info[i].num_state;
                int j = 0;
                while(j<ci->num_state)  //update state info
                {
                    ci->state_set[j] = ct->constraint_info[i].state_set[j]; 
                    j++;
                }                
                break;
            }
            i++;
        }  
    }
    else if(type==LB)  //array
    {
        int i = 0;
        int last = ct->num_constraint_info; 
        while(i<last)
        {
            if(ct->constraint_info[i].type==3)
            {
                ci->num_state = ct->constraint_info[i].num_state;
                int j = 0;
                while(j<ci->num_state)  //update state info
                {
                    ci->state_set[j] = ct->constraint_info[i].state_set[j];
                    j++;
                }
                break;
            }
            i++;
        } 
    }
}

//add the correspondence between the current state and the next symbol
static inline void addConstraintInfo(ConstraintTable* ct, int state, int type, char* name)
{
    int index = ct->num_constraint_info;
    int last = ct->num_constraint_info; 
    if(type==KY) //key field
    {
        int i = 0;
        while(i<last)
        {
            if(strcmp(name, ct->constraint_info[i].token_name)==0)
            {
                index = i;
                break;
            }
            i++;
        }
    }
    else if(type==LB) //array
    {
        int i = 0;
        while(i<last)
        {
            if(ct->constraint_info[i].type==LB)
            {   
                index = i;
                break;
            }
            i++;
        }
    }
    if(index!=last)
    {   
        int state_index = 0;
        int state_last = ct->constraint_info[index].num_state;
        while(state_index<state_last)
        {
            if(ct->constraint_info[index].state_set[state_index]==state)
                break;
            state_index++;
        } 
        if(state_index==state_last) //add state information
        {
            int state_index = (ct->constraint_info[index].num_state++);
            ct->constraint_info[index].state_set[state_index] = state;
        }
    }
    else{
        ++ct->num_constraint_info;
        ct->constraint_info[index].type = type;
        if(type==KY) strcopy(name, ct->constraint_info[index].token_name);
        ct->constraint_info[index].num_state = 0;
        int state_index = (ct->constraint_info[index].num_state++);
        ct->constraint_info[index].state_set[state_index] = state;
    } 
}

static void printConstraintTable(ConstraintTable* ct)
{ 
    printf("num constraint info %d\n", ct->num_constraint_info);
    int i = 0, j = 0;
    while(i<ct->num_constraint_info)
    {
        printf("starting states for key %s or array %d; number of states %d\n", ct->constraint_info[i].token_name, ct->constraint_info[i].type, ct->constraint_info[i].num_state);
        j = 0;
        while(j<ct->constraint_info[i].num_state)
        {
            printf("starting state %d %d ;", j, ct->constraint_info[i].state_set[j]);
            j++;
        }
        printf("\n");
        i++;
    }
}

static inline void freeConstraintTable(ConstraintTable* ct)
{
    free(ct);
}


#endif // !__CONSTRAINT_H__
