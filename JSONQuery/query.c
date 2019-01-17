/********************************************************************************************************************
Copyright (C).
FileName: query.c
Author: Lin Jiang
Version : V1.0
Date: 07/11/2017
Description: This program could execute the JSONPath query to get some necessary information from a large JSON dataset. 
It could also divide it into several parts and deal with each part in parallel.
**********************************************************************************************************************/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <malloc.h>
#include <sys/time.h>
#include <sys/file.h>
#include <unistd.h>
#include <sched.h>
#include "basic.h"
#include "file_operation.h"
#include "input.h"
//#include "lexer.h"
#include "constraint.h"
#include "query.h"
#include "global.h"

int temp_err = 0;
int runtime = 0;
int percentage = 1;

/*data structure for each thread*/
#define MAX_THREAD 70
#define MAX_LINE 100
pthread_t thread[MAX_THREAD]; 
int thread_args[MAX_THREAD];
int finish_args[MAX_THREAD];
double time_step[MAX_THREAD];

char * buffFiles[MAX_THREAD]; 
outputele outputs[MAX_THREAD];

//winter 2018
int goggle_flag = 0;
int warmup_flag = 0;
int debug_flag = 0;
int statistic_flag = 0;
int collect_flag = 0;
int num_objects = 0;
int num_arrays = 0;
int num_keys = 0;
int num_values = 0;

int choose;
int num_threads;
char* file_name;
char* xmlPath;
char* jsonPath;
int warmup_flag;
int pversion;
int statistic_flag;
int debug_flag;

typedef struct thread_info
{
	double exe_time;
	int n_start_states[MAX_THREAD];
	int top_start_states;
	int n_child_states[MAX_THREAD];
	int n_child_states_count[MAX_THREAD];
	int top_n_child_states;
	int memory_bytes[MAX_THREAD];
	int n_memory_bytes[MAX_THREAD];
	int top_memory_bytes;
}thread_info;

thread_info tinfo[MAX_THREAD];
double reexecution_time[MAX_THREAD];

#define MAX_FILE 1000
 Automata stateMachine[MAX_SIZE];   //save automata for XPath

 int stateCount=0; //the number of states for XPath
 int machineCount=1; //the number of nodes for automata

typedef struct TreeEle{
	int value;
    char text[1000];
}TreeEle;

typedef struct Tree{
	TreeEle element[399];
	int top_tree;
}Tree;

Tree start_tree[MAX_THREAD];

int st_type[MAX_THREAD];

//fall 2017 put it inside a function!
grammer_tree g_tree;

stack sta;

/*data structrure for sub-xpath queries*/
char* XPath[3*MAX_LINE];
int top_XPath=-1;
int relation[3*MAX_LINE];
int top_relation=-1;

char* mXPath[3*MAX_LINE];
int top_mXPath=-1;

char* inputXPath[3*MAX_LINE];
int top_inputXPath=-1;

typedef struct branch_state
{
	int state;
	char keyword[MAX_LINE];
}branch_state;

branch_state b_states[MAX_LINE];
int top_b_states=-1;

int pversion=1;   // 0-- without grammar, 1-- with grammar

taginfo tags[MAX_SIZE];
int top_tags=-1;

tagstack tstack[MAX_SIZE/5];
int top_tstack=-1;

int final_top_output=-1;

int final_last_output=-1;

typedef struct pre_condition
{
	int index;
	int condition_str[MAX_THREAD];
	int top_condition_str;
	int relation[MAX_THREAD];
	int top_relation;
}pre_condition;

pre_condition p_condition[MAX_THREAD];
int top_p_condition=-1;

typedef struct condition
{
	char key[MAX_LINE];
	char condition_str[MAX_THREAD][MAX_LINE];
	int top_condition_str;
	int relation[MAX_THREAD];
	int top_relation;
	int condition_tags[MAX_THREAD];
}condition;

condition c_condition[MAX_THREAD];
int top_c_condition=-1;

typedef struct xpath_mapping
{
	int condition_index;
	char output_string[4*MAX_THREAD];
}xpath_mapping;

xpath_mapping x_mapping[4*MAX_THREAD];
int top_x_mapping=-1;

/* new data structure for stack in json--JSONPROJ*/
//cs 299
#define LBRACE 1
#define LBRACKET 2
#define KEY 3
#define TEMP 4
#define PUSH 1
#define POP 2
#define RBRACE 1
#define RBRACKET 2
#define PRIMITIVE 3
#define OBJECT 5
#define ARRAY 6

#define LCB 1
#define RCB 2
#define LB 3
#define RB 4
#define COM 5
#define KY 6
#define PRI 7

typedef struct child_tuple
{
	int children[500];
	int child_count;
        int supplement[11];
}child_tuple;

child_tuple ct_array[MAX_THREAD];

typedef struct value_tuple
{
	int values[500];
	int from[500];
	int to[500];
    //fall 2017
    int fromr[500];
    int tor[500];
    //winter 2018
    int flag[500];
    int counter[500];
	int value_count;
    int supplement[3];
}value_tuple;

value_tuple vt_array[MAX_THREAD];

/* root tuple saves root information for each element on the stack tree */
typedef struct root_tuple
{
	int root[500];
	int root_count;
    int supplement[11];
}root_tuple;

root_tuple rt_array[MAX_THREAD];

typedef struct query_stack{
    value_tuple* vt;
    child_tuple* ct;
    root_tuple* rt;
}query_stack;

query_stack qstack[MAX_THREAD];

typedef struct parent_values{
    int values[30];
    int top_values;
    int parent_values[30][30];
    int top_parent_values[30];
}parent_values;

/* information for segments */
typedef struct t_segs
{
    int values[MAX_SIZE];
    int num_values;
    int start_tree_sp;
    int start_tree_ep;
    int start;
    int end;
}t_segs;

typedef struct segs
{
    t_segs ele[20];
    int num_segs;
    int supplement[3];
}segs;

segs tsegs[MAX_THREAD];

int special_start = -1;
double reprocessing_time = 0.0;
int error_seg = 0;

//#define MAX_OUTPUT 100000
typedef struct oiele{
        int output_index[1][MAX_SIZE];
        int top_output_index;
        int supplement[15];
}oiele;

oiele output_indexes[MAX_THREAD];

typedef struct root_outputs{
    tuple_array roots[20][MAX_SIZE][100];
    int top_roots[20][MAX_SIZE];
    int thread_id[500];
    int supplement[6];
}root_outputs;
root_outputs rot[MAX_THREAD];

typedef struct predicate_states{
    int state;
    int condition_list[50];
    int condition_value[50];
    int last_count;
    int supplement[9];
    int top_condition_list;
}predicate_states;

predicate_states pstate[50];
int top_pstate = -1;

typedef struct predicate_state_list
{
    predicate_states list[50];
    int top_list;
    int start_index;
    int end_index;
    int supplement[13];
}predicate_state_list;
predicate_state_list common_list[MAX_THREAD];
predicate_state_list special_list[MAX_THREAD][20];

int top_special_list[MAX_THREAD];
typedef struct pstack{
   int predicate_stack[50];
   int start[50];
   int end[50];
   int top_predicate_stack;
   int supplement[9];
}pstack;

pstack predicate_stacks[MAX_THREAD];

stack_tag stack_tags[MAX_THREAD];

int thread_numbers = 64;
int tempo_states[MAX_THREAD];

int last_flag = 0;

double err_segments=0.0;

/*before thread creation*/
char* ReadXPath(char* xpath_name);  //load XPath into memory
void createAutoMachine(char* xmlPath,int final_output);   //create automachine for XPath.txt
void get_multixpath(char *xmlPath); //get all sub-queries from input XPath

/*main functions for each thread*/
void createTree_first(int start_state); //create tree for the first thread
void createTree(int thread_num); //create tree for other threads
int streaming_automata(xml_Text *pText, xml_Token *pToken);  //parse and query data in an xmlText, return value:0--success -1--error
int parallel_automata(xml_Text *pText, xml_Token *pToken, int thread_num);  //parallel parsing and querying

/*************************************************
Function: char* ReadXPath(char* xpath_name);
Description: load XPath from related file
Input: xpath_name--the name for the XPath file
Return: the contents in the Xpath file; error--can't open the XPath file
*************************************************/
char* ReadXPath(char* xpath_name)
{
	FILE *fp;
	char* buf=(char*)malloc(MAX_LINE*sizeof(char));
	char* xpath=(char*)malloc(MAX_LINE*sizeof(char));
	xpath=strcpy(xpath,"");
	if((fp = fopen(xpath_name,"r")) == NULL)
    {
        xpath=strcpy(xpath,"error");
    }
    else{
    	while(fgets(buf,MAX_LINE,fp) != NULL)
    	{
    		xpath=strcat(xpath,buf);
		}
	}
	free(buf);
    return xpath;
}

char temp_transition_strings[MAX_LINE][MAX_LINE];
int top_t_strings=-1;

/*************************************************************
Function: void get_t_strings(char *t_string);
Description: used for XPath parsing and automaton construction
Input: tstring -- a string needs to be further processed
*************************************************************/
void get_t_strings(char *t_string)
{
	top_t_strings=-1;
	char record[MAX_LINE];
	int top_record=-1;
	int i;
	for(i=0;i<=strlen(t_string);i++)
	{
		if(t_string[i]=='|')
		{
			record[++top_record]='\0';
			strcopy(record,temp_transition_strings[++top_t_strings]);
			top_record=-1;
		}
		else if(t_string[i]=='\0')
		{
			record[++top_record]='\0';
			strcopy(record,temp_transition_strings[++top_t_strings]);
			top_record=-1;
			break;
		}
		else
		{
			record[++top_record]=t_string[i];
		}
	}
}

/*************************************************
Function: void createAutoMachine(char* xmlPath,int final_output);
Description: create an automata by the XPath Query command
Called By: int main(void);
Input: xmlPath--XPath Query command,final_output--0 is not the last XPath,1 is the last XPath
*************************************************/
void createAutoMachine(char* xmlPath,int final_output)
{
	/*get_t_strings("namerica\|samerica");
	int ti;
	for(ti=0;ti<=top_t_strings;ti++)
	{
		printf("%s; ",temp_transition_strings[ti]);
	}
	printf("\n");*/
	char seps[] = "/"; 
	char subpaths[MAX_THREAD][MAX_LINE];
	int top_sub_xpaths=-1;
	int i,start,end,j,k;
	int first_special_tag=0;
	if(xmlPath[0]=='/'&&xmlPath[1]=='/')
	    first_special_tag=1;
	top_b_states=-1;
	for(i=0,start=0,end=0;i<strlen(xmlPath);i++)
	{
		if(xmlPath[i]=='/'&&xmlPath[i+1]=='/')
		{
			if(i>0)
			{
				end=i;
				char *sub=substring(xmlPath,start,end);
				if(sub!=NULL)
				{
					strcopy(sub,subpaths[++top_sub_xpaths]);
					start=i+2;
					i=i+2;
					//printf("sub %s %d\n",subpaths[top_sub_xpaths],start);
					free(sub);
					continue;
				}
			}
			else
			{
				start=i+2;
				i=i+2;
				continue;
			}
		}
	}
	//printf("finish %d %d\n",start,strlen(xmlPath)-1);
	if(start<=strlen(xmlPath)-1)
	{
		end=strlen(xmlPath);
		char *sub=substring(xmlPath,start,end);
		if(sub!=NULL)
		{
			strcopy(sub,subpaths[++top_sub_xpaths]);
			//printf("sub %s\n",subpaths[top_sub_xpaths]);
			free(sub);
		}
	}
	//stateMachine[0].n_transitions=-1;
	int equal_point=1;  //record the current position before finding the right start status for XPath with predicates
	int equal_tags=0; //1--find the position of the right start status for XPath with predicates
	int start_value=1;
	int end_value=2;
	char temp_token[MAX_LINE];
	char temp_first_path[MAX_LINE];
	
	
	for(i=0;i<=top_sub_xpaths;i++)
	{
		//printf("the %dth subpath:%s\n",i,subpaths[i]);
		if(i==0) strcopy(subpaths[i],temp_first_path);
		char *token = strtok(subpaths[i], seps);
		int first=1;
		int first_ele=1;
		int continue_tag;
		int special_tag=0;
		int p_tag=0;
		while(token!= NULL) 
	    {
		    //printf("first %s %d %d %d %d\n",token,i,first,machineCount,equal_point);
		    if(equal_tags==0&&machineCount>1)
		    {
		    	strcopy(token,temp_token);
		    	 //printf("check %s %d %d %d %d %s\n",token,i,stateMachine[equal_point].n_transitions,machineCount,equal_point,stateMachine[equal_point].str[0]);
		    	for(k=0;k<=stateMachine[equal_point].n_transitions;k++)
		        {
		        	//printf("string checking %s %s %d %d\n",stateMachine[equal_point].str[k],temp_token,start_value,strcmp(stateMachine[equal_point].str[k],temp_token));
		        	if(strcmp(stateMachine[equal_point].str[k],temp_token)==0)
		    		{
		    			//printf("string equal %s %s %d %d\n",stateMachine[equal_point].str[k],temp_token,start_value,equal_point);
		    		    break;
					}
				}
				if(k<=stateMachine[equal_point].n_transitions)
				{
					//printf("string equal %s\n",stateMachine[equal_point].str[k]);
					start_value=stateMachine[equal_point].end[k];
					equal_point=2*stateMachine[equal_point].end[k]-1;
					equal_tags=0;
					token=strtok(NULL,seps);
					if(equal_point<=machineCount) 
					{
						first_ele=0;
					    continue;
					}
					//else special_tag=0;
				}
				else
				{
					//special_tag=1;
					//printf("string not equal %s %s %d %d %d %d %d\n",token,stateMachine[equal_point].str[k],start_value,equal_point,stateCount,machineCount,stateMachine[equal_point].n_transitions);
				}
	        }
	        //printf("equal point %d %d %s %d\n",equal_point,machineCount,token,start_value);
	        if(equal_point>machineCount&&token==NULL) break;
            if(equal_tags==0&&machineCount>1&&first==1&&final_output!=2&&top_XPath>0) //link the first state
            {
            	//printf("equal point %d %d %s %d %d\n",equal_point,machineCount,token,start_value,stateCount);
            	//printf("link*************%s %d\n",token,start_value);
            	equal_tags=1;
            	++stateMachine[equal_point].n_transitions;
            	//start_value=start_value-1;
            	stateMachine[equal_point].start[stateMachine[equal_point].n_transitions]=start_value;
            	strcopy(token,stateMachine[equal_point].str[stateMachine[equal_point].n_transitions]);
            	stateMachine[equal_point].end[stateMachine[equal_point].n_transitions]=stateCount+1;
            	machineCount++;
            	special_tag=1;
            	if((i>0&&first==1&&first_ele==1)||(i==0&&first_special_tag==1&&first_ele==1))
            	{
            		 //printf("&&&&&&&&&&&special // point state:%d; token:%s\n",stateCount+1,token);
            		 b_states[++top_b_states].state=stateCount+1;
            		 strcopy(token,b_states[top_b_states].keyword);
				}
			}
			//printf("2\n");
			if(machineCount==equal_point&&special_tag==1&&final_output!=2)
			{
				//stateCount++;
			}
			else{
			//printf("equal point after %d %d %s %d %d %d\n",equal_point,machineCount,token,start_value,special_tag,stateCount);
		    stateCount++;
		   	//++stateMachine[machineCount].n_transitions;
		    //stateMachine[machineCount].start[stateMachine[machineCount].n_transitions]=start_value;
		    //printf("first %s %d\n",token,machineCount);
		    //strcopy(token,stateMachine[machineCount].str[stateMachine[machineCount].n_transitions]);
		    int ti;
			char temp_ts[MAX_LINE];
			strcopy(token,temp_ts);
			get_t_strings(temp_ts);
			for(ti=0;ti<=top_t_strings;ti++)
			{
			    ++stateMachine[machineCount].n_transitions;
			    stateMachine[machineCount].start[stateMachine[machineCount].n_transitions]=start_value;
			    ///printf("**********split %s %s\n",temp_transition_strings[ti],token);
			    strcopy(temp_transition_strings[ti],stateMachine[machineCount].str[stateMachine[machineCount].n_transitions]);
			    if(special_tag==1) stateMachine[machineCount].end[stateMachine[machineCount].n_transitions]=stateCount;
		        else stateMachine[machineCount].end[stateMachine[machineCount].n_transitions]=stateCount+1;
			}
			stateMachine[machineCount].isoutput=0;
		    //stateCount--;
		    
		}
			if((i>0&&first==1&&first_ele==1)||(i==0&&first_special_tag==1&&first_ele==1))  //point to itself
			{
				//if(!(machineCount==equal_point&&special_tag==1&&final_output!=2)) stateCount--;
				if(first_ele==1) first_ele=0;
				//printf("link it self %s %d %d %d %d\n",token,machineCount,stateCount,special_tag,final_output);
				if(special_tag==1&&final_output!=2&&start_value<stateCount) stateCount--;
				//stateMachine[machineCount+2].n_transitions=-1;
				++stateMachine[machineCount+2].n_transitions;
				stateMachine[machineCount+2].start[stateMachine[machineCount+2].n_transitions]=stateCount+1;
				strcopy(token,stateMachine[machineCount+2].str[stateMachine[machineCount+2].n_transitions]);
				stateMachine[machineCount+2].end[stateMachine[machineCount+2].n_transitions]=stateCount+1;
				
				//stateMachine[machineCount+1].n_transitions=-1;
				//point to /
				++stateMachine[machineCount+1].n_transitions;
				stateMachine[machineCount+1].start[stateMachine[machineCount+1].n_transitions]=stateCount+1;
				stateMachine[machineCount+1].str[stateMachine[machineCount+1].n_transitions][0]='/';
				strcopy(token,stateMachine[machineCount+1].str[stateMachine[machineCount+1].n_transitions]+1);
				stateMachine[machineCount+1].end[stateMachine[machineCount+1].n_transitions]=stateCount+1;
				
				//point to state 0
				//printf("zero point before %s %d\n",stateMachine[0].str[stateMachine[0].n_transitions],stateMachine[0].n_transitions);
 
				++stateMachine[0].n_transitions;
			    if(top_XPath==0||start_value==1) stateMachine[0].start[stateMachine[0].n_transitions]=0;
			    else stateMachine[0].start[stateMachine[0].n_transitions]=start_value;
			    ///printf("!!!!!!!!!!!start_value %d %d\n",start_value,stateCount);
			    strcopy(token,stateMachine[0].str[stateMachine[0].n_transitions]);
		        stateMachine[0].end[stateMachine[0].n_transitions]=stateCount+1;
		        //printf("zero point %s %d\n",stateMachine[0].str[stateMachine[0].n_transitions],stateMachine[0].n_transitions);
 
                //point to /
		        ++stateMachine[machineCount+1].n_transitions;
			    stateMachine[machineCount+1].start[stateMachine[machineCount+1].n_transitions]=stateCount+1;
			    stateMachine[machineCount+1].str[stateMachine[machineCount+1].n_transitions][0]='/';
				strcopy(token,stateMachine[machineCount+1].str[stateMachine[machineCount+1].n_transitions]+1);
				stateMachine[machineCount+1].end[stateMachine[machineCount+1].n_transitions]=0;
				if(special_tag==1&&final_output!=2&&start_value<stateCount) stateCount++;
				//first=0;
				//first_special_tag=0;
			}
			//if(special_tag==1) special_tag=0;
			if(machineCount==equal_point&&special_tag==1&&final_output!=2)
			{
				stateCount++;
			}
		    machineCount++;
		    if(stateCount>=1)
		    {
		    	int ti;
			    char temp_ts[MAX_LINE];
			    strcopy(token,temp_ts);
			    get_t_strings(temp_ts);
			    for(ti=0;ti<=top_t_strings;ti++)
			    {
			        ++stateMachine[machineCount].n_transitions;
			        if(special_tag==1) stateMachine[machineCount].start[stateMachine[machineCount].n_transitions]=stateCount;
			        else stateMachine[machineCount].start[stateMachine[machineCount].n_transitions]=stateCount+1;
			        stateMachine[machineCount].str[stateMachine[machineCount].n_transitions][0]='/';
			        strcopy(temp_transition_strings[ti],stateMachine[machineCount].str[stateMachine[machineCount].n_transitions]+1);
				    stateMachine[machineCount].end[stateMachine[machineCount].n_transitions]=start_value;
		    	}
		    	stateMachine[machineCount].isoutput=0;
		    }
		    token=strtok(NULL,seps);  
		    ///printf("**********next token %s\n",token);
            if(first_ele==1) first_ele=0;
		    if(token==NULL&&i==top_sub_xpaths)
		    {
		    	if(final_output==1)
		    	{
		    		//printf("&&&&&&the out put %d %d\n",stateCount,machineCount);
		    		stateMachine[machineCount-1].isoutput=1;
		 	        stateMachine[machineCount].isoutput=1;
				}
				else
				{
					//printf("************top output==2",stateMachine[machineCount-1].start[0]);
					stateMachine[machineCount-1].isoutput=2;
		 	        stateMachine[machineCount].isoutput=2;
				}
			    
		 	    if(top_sub_xpaths>0&&first==1&&first_ele==1&&final_output==1)
		 	    {
		 	        //printf("%s %d last //\n",stateMachine[machineCount].str[0],machineCount);
		 	        ///printf("enter into special point %d %s %s\n",stateCount,token,stateMachine[machineCount-1].str[0]);
		             if(top_sub_xpaths>0){
		            //point to itself	            
		            ++stateMachine[machineCount].n_transitions;
		            stateMachine[machineCount].start[stateMachine[machineCount].n_transitions]=stateCount+1;
		              if(stateMachine[machineCount-1].n_transitions>0)
		                strcopy(stateMachine[machineCount-1].str[stateMachine[machineCount-1].n_transitions-1],stateMachine[machineCount].str[stateMachine[machineCount].n_transitions]);
		            else strcopy(stateMachine[machineCount-1].str[0],stateMachine[machineCount].str[stateMachine[machineCount].n_transitions]);
			        stateMachine[machineCount].end[stateMachine[machineCount].n_transitions]=stateCount+1;
		            }
		        }
		    }
		    else
			{
				machineCount++;
			} 
			if(first_ele==1) first_ele=0;
			if(first==1&&i==0) first=2;
			else if(i==0&&first==2) first=0;
			else if(first==1) first=0;
			
			if(special_tag==1)
			{
				stateCount--;
				//machineCount-=2;
				special_tag=0;
			} 
			start_value=stateCount+1;
			
	    }
        if(i==top_sub_xpaths) stateCount++;
	}
	for(i=0;i<=top_b_states;i++)
	{
		int state=b_states[i].state;
		//++stateMachine[2*state-1].n_transitions;
		for(j=state+1;j<=stateCount;j++)
		{
			++stateMachine[2*j-1].n_transitions;
			stateMachine[2*j-1].start[stateMachine[2*j-1].n_transitions]=j;
			stateMachine[2*j-1].end[stateMachine[2*j-1].n_transitions]=state;
			strcopy(b_states[i].keyword,stateMachine[2*j-1].str[stateMachine[2*j-1].n_transitions]);
		}
	}
}

/*************************************************
Function: void get_subxpath(char *xmlPath);
Description: get sub queries
Input: xmlPath--Query command
*************************************************/
void get_subxpath(char *xmlPath)
{
	char seps[] = "["; 
	
	//printf("enter1 %s\n",xmlPath);
	char *token = strtok(xmlPath, seps); 
	//printf("enter2\n");
	//char *token = strtok(xmlPath, seps); 
	char *xpath[MAX_LINE];
	int first=1;
	int top_xpath=-1;
	//printf("enter3\n");
	while(token!= NULL) 
	{
		//printf("xpath %s %d\n",token,strlen(token));
		
		if(top_xpath>-1)   xpath[++top_xpath]=(char*)malloc((strlen(token)+1)*sizeof(char));
		else if(top_xpath==-1) xpath[++top_xpath]=(char*)malloc(MAX_LINE*sizeof(char));
		//printf("temp\n");
		xpath[top_xpath]=strcpy(xpath[top_xpath],token);
		//printf("xpath1 %s\n",xpath[top_xpath]);
		token=strtok(NULL,seps);  
	}
	//printf("enter4\n");
	if(top_xpath<0) return;
	//relation[++top_relation]=1;
	char seps1[]="]";
	XPath[++top_XPath]=(char*)malloc((strlen(xpath[0])+1)*sizeof(char));
	
	XPath[top_XPath]=strcpy(XPath[top_XPath],xpath[0]);
	//printf("first path %s\n",XPath[0]);
	int i,j;
	int top=-1;
	char* temp;
	char* record=(char*)malloc(MAX_LINE*sizeof(char));
	char *temp_xpath;
	int past=0;
	//printf("0xpath %s\n",xpath[0]);
	for(i=1;i<=top_xpath;i++)
	{
		/*if(xpath[i][0]=='/')
		{
			printf("******************add special xpath %s\n",xpath[i]);
			xpath[0]=strcat(xpath[i],xpath[0]);
		}*/
		
		token=strtok(xpath[i], seps1);
		///printf("token %s\n",token);
		
		//printf("temp %s %d\n",temp,strlen(temp));
		p_condition[++top_p_condition].index=top_XPath;
		p_condition[top_p_condition].top_condition_str=-1;
		p_condition[top_p_condition].top_relation=-1;

		while(token!= NULL) 
	    {
	    	
	    	//p_condition[top_p_condition].relation[++p_condition.top_relation]=1;
	    	relation[++top_relation]=1;
	    	temp=(char*)malloc((strlen(token)+1)*sizeof(char));
		    temp=strcpy(temp,token);
	    	//printf("token %s\n",token);
	    	//printf("temp %s %d\n",temp,strlen(temp));
	    	for(j=0;j<=strlen(temp);j++)
	    	{
	    		//printf("temps %c ",temp[j]);
	    		if(temp[j]==' ')
	    		{
	    			record[++top]='\0';
	    			//printf("record %s\n",record[top]);
	    			if(strcmp(record,"and")==0)
	    			{
	    				//p_condition[top_p_condition].condition_str[++p_condition[top_p_condition].condition_str]=top_XPath;
	    				p_condition[top_p_condition].relation[++p_condition[top_p_condition].top_relation]=1;
	    				relation[++top_relation]=1;
					}
					else if(strcmp(record,"or")==0)
					{
						//p_condition[top_p_condition].condition_str[++p_condition[top_p_condition].condition_str]=top_XPath;
	    				p_condition[top_p_condition].relation[++p_condition[top_p_condition].top_relation]=0;
						relation[++top_relation]=0;
					}
					else
					{
						if(record[0]=='/')
				            XPath[++top_XPath]=(char*)malloc((strlen(xpath[0])+strlen(record)+2)*sizeof(char));
				        else XPath[++top_XPath]=(char*)malloc((strlen(xpath[0])+strlen(record)+3)*sizeof(char));
	    			    XPath[top_XPath]=strcpy(XPath[top_XPath],xpath[0]);
	    			    if(record[0]!='/')
						    XPath[top_XPath]=strcat(XPath[top_XPath],"/");
	    			    XPath[top_XPath]=strcat(XPath[top_XPath],record);
	    			    if(record[0]=='/'&&record[1]!='/') xpath[0]=strcat(xpath[0],record);
	    			    p_condition[top_p_condition].condition_str[++p_condition[top_p_condition].top_condition_str]=top_XPath;
					}
					top=-1;
				}
				else if(temp[j]=='(')
				{
					relation[++top_relation]=3;
				}
				else if(temp[j]==')')
				{
					relation[++top_relation]=4;
				}
				else
				{
					record[++top]=temp[j];
					//printf("record %c\n",record[top]);
				}
			}
			if(top!=-1)
			{
				record[++top]='\0';
				//printf("record %c\n",record[0]);
				if(record[0]=='/')
				    XPath[++top_XPath]=(char*)malloc((strlen(xpath[0])+strlen(record)+3)*sizeof(char));
				else XPath[++top_XPath]=(char*)malloc((strlen(xpath[0])+strlen(record)+3)*sizeof(char));
	    		XPath[top_XPath]=strcpy(XPath[top_XPath],xpath[0]);
	    		//printf("topxpath %d xpath0 %s\n",top_XPath,XPath[top_XPath]);
	    		if(record[0]!='/')
	    		    XPath[top_XPath]=strcat(XPath[top_XPath],"/");
	    		XPath[top_XPath]=strcat(XPath[top_XPath],record);
	    		//printf("end topxpath %d xpath0 %s\n",top_XPath,XPath[top_XPath]);
	    		if(record[0]=='/'&&record[1]!='/') xpath[0]=strcat(xpath[0],record);
	    		//printf("recordPath %s\n",XPath[top_XPath]);
	    		top=-1;
	    		if(record[0]!='/') p_condition[top_p_condition].condition_str[++p_condition[top_p_condition].top_condition_str]=top_XPath;
			}
			token=strtok(NULL,seps1);  
		//xpath[++top_xpath]=(char*)malloc((strlen(token)+1)*sizeof(char));
		//xpath[++top_xpath]=strcpy(xpath[++top_xpath],token);
	    }
	}
}

/*************************************************
Function: void get_multixpath(char *xmlPath);
Description: get sub queries
Input: xmlPath-- Query command
*************************************************/
void get_multixpath(char *xmlPath)
{
	char seps[] = ";"; 
	//printf("*********call %s\n",xmlPath);
	//char temp[500];
	//strcopy(xmlPath,temp);
	char *token = strtok(xmlPath, seps); 

	while(token!= NULL) 
	{
		mXPath[++top_mXPath]=(char*)malloc((strlen(token)+1)*sizeof(char));
		mXPath[top_mXPath]=strcpy(mXPath[top_mXPath],token);
		//printf("*********call %s\n",mXPath[top_mXPath]);
		//get_subxpath(mXPath[top_mXPath]);
		//printf("xpath1 %s\n",xpath[top_xpath]);
		token=strtok(NULL,seps);  
		//printf("*********call3 %s\n",token);
	}
	int i;

	for(i=0;i<=top_mXPath;i++)
	{
		get_subxpath(mXPath[i]);
	}

}

/*************************************************
Function: void get_inputxpath(char *xmlPath);
Description: separate input paths by semicolon
Input: xmlPath-- Query command
*************************************************/
void get_inputxpath(char *xmlPath)
{
    char seps[] = ";";
    char *token = strtok(xmlPath, seps);

    while(token!= NULL)
    {
        inputXPath[++top_inputXPath]=(char*)malloc((strlen(token)+1)*sizeof(char));
        inputXPath[top_inputXPath]=strcpy(inputXPath[top_inputXPath],token);
        token=strtok(NULL,seps);
    }
}

/*****************************************************************************
Function: void print_xpath();
Description: print all separated queries, program will process them one by one
******************************************************************************/
void print_xpath()
{
	int i;
	printf("the final xpaths\n");
	for(i=0;i<=top_XPath;i++)
	{
		printf("the %dth xpath: %s\n",i,XPath[i]);
	}
	printf("\n");
	for(i=0;i<=top_relation;i++)
	{
		printf("the %dth relation %d\n",i,relation[i]);
	}
}

/*****************************************************************************
Function: void createTree1(int thread_num);
Input: thread_num -- the number of the current thread
Description: initialize start tree for the current thread
******************************************************************************/
void createTree1(int thread_num)
{
	start_tree[thread_num].top_tree=-1;
}

/***************************************************************************************************************************************************************************************
Function: tuple_array pushing(int thread_num, char *str, tuple_array *current_states,stack_tag *stack_tag, value_tuple *vt_arr, child_tuple *ct_arr, root_tuple *rt_arr);
Description: push current_states into stack, then update current_states based on input string. 
Input: current state and stack information
Return: updated current states
*****************************************************************************************************************************************************************************************/
static inline tuple_array pushing(int thread_num, char *str, tuple_array *current_states,stack_tag *stack_tag, value_tuple *vt_arr, child_tuple *ct_arr, root_tuple *rt_arr)
{
    int i,j,k;

    int top_root_children=-1;
    int tag_zero=0;
    int tag_initiate=0;

    tuple_array next_states;
    next_states.from = 0;
    next_states.to = -1;

    int parent_values[30][30];
    int top_parent_values[30];

    int root_values[30][30];
    int top_root_values[30];

    for(i=0; i<30; i++)
        top_root_values[i] = -1;
    for(i=0; i<30; i++)
        top_parent_values[i] = -1;
    int tempccount = ct_arr->child_count;
    int temprcount = rt_arr->root_count;
    //getting all possible starting states
    //initiate finish tree
    if(current_states->from== -1)
    {
        int start[MAX_SIZE];
        int end[MAX_SIZE];
        int count=-1;
        if(thread_num==0)  //the first thread
        {
            start[++count]=1;
            if(strcmp(stateMachine[1].str[stateMachine[1].n_transitions],str)==0)
            {
                end[count]=stateMachine[1].end[stateMachine[1].n_transitions];
            }
            else
            {
                end[count]=0;
            }
        }
        //reprocessing
        if(special_start != -1) {
            start[++count] = special_start;
            if(special_start == 0) end[count] = 0;
            else {
                if(strcmp(stateMachine[2*special_start-1].str[stateMachine[2*special_start-1].n_transitions],str)==0)
                {
                    end[count]=stateMachine[2*special_start-1].end[stateMachine[2*special_start-1].n_transitions];
                }
                else
                {
                    end[count]=0;
                }
            }
        }
        else
        {
        	//get possible starting states
            for(i=0; i<=top_tags; i++)
            {
                if(strcmp(tags[i].tagname,str)==0)
                {
                    for(j=0; j<=tags[i].top_point; j++)
                    {
                        start[++count]=tags[i].start[j];
                        end[count]=tags[i].end[j];
                        //break;
                    }
                    break;
                }
            }
            if(count==-1)
            {
                for(j=machineCount+2; j>=0; j=j-2) //all possible start states
                {
                    start[++count]=j/2;
                    int automata_value = j/2;
                    int automata_position = 2*automata_value - 1;
                    if(automata_value == 0) automata_position = 0;
                    if(automata_position<=machineCount)
                    {
                        for(k=0; k<=stateMachine[automata_position].n_transitions; k++)
                        {
                            if((strcmp(stateMachine[automata_position].str[k],"*")==0&&stateMachine[automata_position].start[k]==automata_value)||(strcmp(stateMachine[automata_position].str[k],str)==0&&stateMachine[automata_position].start[k]==automata_value))
                            {
                                end[count] = stateMachine[automata_position].end[k];
                                break;
                            }
                        }
                        if(k>stateMachine[automata_position].n_transitions)
                            end[count]=0;
                    }
                    else end[count] = 0;
                }
            }
            //record segment
            int seg_index = (++tsegs[thread_num].num_segs);
            tsegs[thread_num].ele[seg_index].num_values = -1;
            for(j=0; j<=count; j++)
            {
                int n_value = (++tsegs[thread_num].ele[seg_index].num_values);
                tsegs[thread_num].ele[seg_index].values[n_value] = start[j];
            }
            tsegs[thread_num].ele[seg_index].start_tree_sp = start_tree[thread_num].top_tree;

        }
        if(count>=1)
        {
            vt_arr->value_count=-1;
            ct_arr->child_count=-1;
            rt_arr->root_count = -1;
            //create current_states
            for(i=0; i<=count; i++)
            {
                int vt_index = (++vt_arr->value_count);
                vt_arr->values[vt_index] = start[i];
                vt_arr->from[vt_index] = -1;
                vt_arr->fromr[vt_index] = -1;
                int m_index = 2*start[i]-1;
            }
            current_states->from = 0;
            current_states->to = vt_arr->value_count;
            next_states.from = current_states->to + 1;
            next_states.to = next_states.from - 1;
            //create next_states
            for(i=0; i<=count; i++)
            {
                for(j=next_states.from; j<=next_states.to; j++)
                {
                    if(vt_arr->values[j]==end[i])
                    {
                        break;
                    }
                }
                if(j<=next_states.to)
                {
                    int current = (++ct_arr->child_count);
                    ct_arr->children[current] = start[i];
                    vt_arr->to[j] = current;
                    int vtvalues = vt_arr->values[j];
                    parent_values[vtvalues][++top_parent_values[vtvalues]] = start[i];
                    current = (++rt_arr->root_count);
                    rt_arr->root[current] = start[i];
                    root_values[vtvalues][++top_root_values[vtvalues]] = start[i];
                    vt_arr->tor[j] = current;
                }
                else
                {
                    int from = (++ct_arr->child_count);
                    int to = from;
                    ct_arr->children[from] = start[i];
                    int current = (++vt_arr->value_count);
                    vt_arr->values[current] = end[i];
                    int vtvalues = vt_arr->values[current];
                    parent_values[vtvalues][++top_parent_values[vtvalues]] = start[i];
                    vt_arr->from[current] = from;
                    vt_arr->to[current] = to;
                    next_states.to = current;
                    from = (++rt_array[thread_num].root_count);
                    to = from;
                    rt_array[thread_num].root[from] = start[i];
                    root_values[vtvalues][++top_root_values[vtvalues]] = start[i];
                    vt_arr->fromr[current] = from;
                    vt_arr->tor[current] = to;
                    next_states.tor = current;
                }
            }

            ct_arr->child_count = tempccount;
            rt_arr->root_count = temprcount;
            int froms = next_states.from;
            int tos = next_states.to;
            while(froms<=tos) {
                int cvalue = vt_arr->values[froms];
                int cfrom = -1;;
                int cto = -1;
                for(j=0; j<=top_parent_values[cvalue]; j++)
                {
                    ct_arr->children[++ct_arr->child_count] = parent_values[cvalue][j];
                    if(j==0) {
                        cfrom = ct_arr->child_count;
                        cto = ct_arr->child_count;
                    }
                    else cto = ct_arr->child_count;
                }
                vt_arr->from[froms] = cfrom;
                vt_arr->to[froms] = cto;
                int rfrom = -1;
                int rto = -1;
                for(j=0; j<=top_root_values[cvalue]; j++)
                {
                    rt_arr->root[++rt_arr->root_count] = root_values[cvalue][j];
                    if(j==0) {
                        rfrom = rt_arr->root_count;
                        rto = rt_arr->root_count;
                    }
                    else rto = rt_arr->root_count;
                }
                vt_arr->fromr[froms] = rfrom;
                vt_arr->tor[froms] = rto;
                froms++;
            }
            stack_tag->tuple_arrs[stack_tag->top_stack_tag]=*current_states;
            return next_states;
        }
        if(count==0)
        {
            current_states->from = 0;
            current_states->to = 0;
            vt_arr->value_count = 0;
            vt_arr->values[0] = start[count];
            vt_arr->from[0] = -1;
            vt_arr->to[0] = -1;
            vt_arr->fromr[0] = -1;
            vt_arr->tor[0] = -1;
            vt_arr->flag[0] = 1;
            vt_arr->counter[0] = 0;

            int m_index = 2*start[count]-1;

            next_states.from = 1;
            next_states.to = 1;
            vt_arr->value_count = 1;
            vt_arr->values[1] = end[count];
            vt_arr->from[1] = 0;
            vt_arr->to[1] = 0;

            vt_arr->flag[1] = 1;
            vt_arr->counter[1] = 0;

            ct_arr->child_count = 0;
            ct_arr->children[0] = start[count];
            int vtvalues = vt_arr->values[1];
            rt_arr->root_count = 0;
            rt_arr->root[0] = start[count];
            vt_arr->from[0] = -1;
            vt_arr->fromr[1] = 0;
            vt_arr->tor[1] = 0;
            stack_tag->tuple_arrs[stack_tag->top_stack_tag]=*current_states;
            return next_states;
        }
    }
    //push str into finish tree
    i = current_states->from;
    int children=current_states->to;
    next_states.from = children+1;
    next_states.to = children;
    int top_value=0;
    int tag_first_layer = 1; //0-- not first layer, 1 -- first layer
    if(vt_arr->fromr[i]==-1) {
        rt_arr->root_count = -1;
        tag_first_layer = 1;
        ct_arr->child_count = -1;
    }
    else tag_first_layer = 0;
    for(; i<=children; i++)
    {
        int prior_counter = vt_arr->counter[i];
        int current_counter = 0;
        int current_counter_flag = vt_arr->flag[i];

        int automata_position;
        int automata_value=vt_arr->values[i];
        if(automata_value!=0)
        {
            automata_position=2*automata_value-1;
            if(automata_position==machineCount+1)    automata_position=machineCount;
        }
        else
        {
            automata_position=0;
        }
        if(automata_position<=machineCount)
        {
            for(k=0; k<=stateMachine[automata_position].n_transitions; k++)
            {
                if((strcmp(stateMachine[automata_position].str[k],"*")==0&&stateMachine[automata_position].start[k]==automata_value)||(strcmp(stateMachine[automata_position].str[k],str)==0&&stateMachine[automata_position].start[k]==automata_value))
                {
                    int child_index = 0;
                    if(stateMachine[automata_position].end[k]>0)
                        child_index = 2*stateMachine[automata_position].end[k] - 1;
                    if(current_counter_flag == 1&&(stateMachine[automata_position].low>prior_counter || stateMachine[automata_position].high < prior_counter))// || (prior_counter-stateMachine[automata_position].low)%2!=0))
                        continue;

                    if(current_counter_flag == 0&&(stateMachine[automata_position].high < prior_counter))
                        continue;

                    int aa;
                    aa = next_states.from;
                    top_root_children = next_states.to;
                    for(; aa<=top_root_children; aa++)
                    {
                        if(vt_arr->values[aa]==stateMachine[automata_position].end[k])
                            break;
                    }
                    if(aa<=top_root_children) //under current node
                    {
                        int current = (++ct_arr->child_count);
                        ct_arr->children[current] = stateMachine[automata_position].start[k];
                        //add child node
                        vt_arr->to[aa] = current;
                        int vtvalues = vt_arr->values[aa];
                        parent_values[vtvalues][++top_parent_values[vtvalues]] = automata_value;
                        if(tag_first_layer == 1)
                        {
                            current = (++rt_arr->root_count);
                            rt_arr->root[current] = vt_arr->values[i];
                            root_values[vtvalues][++top_root_values[vtvalues]] = vt_arr->values[i];
                            vt_arr->tor[aa] = current;
                        }
                        else
                        {
                            //traverse all root nodes for its parent and add them into its root node list
                            int bb;
                            for(bb = vt_arr->fromr[i]; bb <= vt_arr->tor[i]; bb++)
                            {
                                current = (++rt_arr->root_count);
                                rt_arr->root[current] = rt_arr->root[bb];
                                root_values[vtvalues][++top_root_values[vtvalues]] = rt_arr->root[bb];
                                vt_arr->tor[aa] = current;
                            }
                        }
                        break;
                    }
                    else //create new node
                    {
                        int from = (++ct_arr->child_count);
                        int to = from;
                        ct_arr->children[from] =automata_value;
                        int current = (++vt_arr->value_count);
                        vt_arr->values[current] = stateMachine[automata_position].end[k];
                        int vtvalues = vt_arr->values[current];
                        parent_values[vtvalues][++top_parent_values[vtvalues]] = automata_value;
                        vt_arr->from[current] = from;
                        vt_arr->to[current] = to;
                        vt_arr->counter[current] = current_counter;
                        vt_arr->flag[current] = current_counter_flag;
                        if(thread_num>0&&strcmp(str,"array")==0)
                            vt_arr->flag[current] = 1;
                        next_states.to = current;
                        if(tag_first_layer == 1)
                        {
                            int current1 = (++rt_arr->root_count);
                            rt_arr->root[current1] = vt_arr->values[i];
                            root_values[vtvalues][++top_root_values[vtvalues]] = vt_arr->values[i];
                            vt_arr->fromr[current] = current1;
                            vt_arr->tor[current] = current1;
                        }
                        else
                        {
                            //traverse all root nodes for its parent and add them into its root node list
                            int bb;
                            for(bb = vt_arr->fromr[i]; bb <= vt_arr->tor[i]; bb++)
                            {
                                int current1 = (++rt_arr->root_count);
                                rt_arr->root[current1] = rt_arr->root[bb];
                                root_values[vtvalues][++top_root_values[vtvalues]] = rt_arr->root[bb];
                                vt_arr->tor[current] = current1;
                                if(bb == vt_arr->fromr[i])
                                {
                                    vt_arr->fromr[current] = current1;
                                }
                            }
                        }
                        break;
                    }
                }
            }
            if(k>stateMachine[automata_position].n_transitions) //next state is 0
            {
                int aa;
                aa = next_states.from;
                top_root_children = next_states.to;

                for(; aa<=top_root_children; aa++)
                {
                    if(vt_arr->values[aa]==0)
                        break;
                }
                if(aa<=top_root_children)
                {
                    int current = (++ct_arr->child_count);
                    ct_arr->children[current] = automata_value;
                    //add child node
                    vt_arr->to[aa] = current;
                    //winter 2018
                    int vtvalues = vt_arr->values[aa];
                    parent_values[vtvalues][++top_parent_values[vtvalues]] = automata_value;
                    if(tag_first_layer == 1)
                    {
                        current = (++rt_arr->root_count); 
                        rt_arr->root[current] = vt_arr->values[i];
                        root_values[vtvalues][++top_root_values[vtvalues]] = vt_arr->values[i];
                        vt_arr->tor[aa] = current;
                    }
                    else
                    {
                        int bb;
                        for(bb = vt_arr->fromr[i]; bb <= vt_arr->tor[i]; bb++)
                        {
                            current = (++rt_arr->root_count);
                            rt_arr->root[current] = rt_arr->root[bb];
                            root_values[vtvalues][++top_root_values[vtvalues]] = rt_arr->root[bb];
                            vt_arr->tor[aa] = current;
                        }
                    }

                }
                else
                {
                    int from = (++ct_arr->child_count);
                    int to = from;
                    ct_arr->children[from] = automata_value;
                    int current = (++vt_arr->value_count);
                    vt_arr->values[current] = 0;
                    int vtvalues = vt_arr->values[aa];
                    parent_values[vtvalues][++top_parent_values[vtvalues]] = automata_value;
                    vt_arr->from[current] = from;
                    vt_arr->to[current] = to;
                    vt_arr->counter[current] = current_counter;
                    vt_arr->flag[current] = current_counter_flag;
                    if(thread_num>0&&strcmp(str,"array")==0)
                        vt_arr->flag[current] = 1;
                    next_states.to = current;
                    if(tag_first_layer == 1)
                    {
                        int current1 = (++rt_arr->root_count);
                        rt_arr->root[current1] = vt_arr->values[i];
                        root_values[vtvalues][++top_root_values[vtvalues]] = vt_arr->values[i];
                        vt_arr->fromr[current] = current1;
                        vt_arr->tor[current] = current1;
                    }
                    else
                    {
                        int bb;
                        for(bb = vt_arr->fromr[i]; bb <= vt_arr->tor[i]; bb++)
                        {
                            int current1 = (++rt_arr->root_count);
                            rt_arr->root[current1] = rt_arr->root[bb];
                            root_values[vtvalues][++top_root_values[vtvalues]] = rt_arr->root[bb];
                            vt_arr->tor[current] = current1;
                            if(bb == vt_arr->fromr[i])
                            {
                                vt_arr->fromr[current] = current1;
                            }
                        }
                    }
                }
            }
        }
    }
    
    ct_arr->child_count = tempccount;
    rt_arr->root_count = temprcount;
    int froms = next_states.from;
    int tos = next_states.to;
    while(froms<=tos) {
        int cvalue = vt_arr->values[froms];
        int cfrom = -1;;
        int cto = -1;
        for(j=0; j<=top_parent_values[cvalue]; j++)
        {
            ct_arr->children[++ct_arr->child_count] = parent_values[cvalue][j];
            if(j==0) {
                cfrom = ct_arr->child_count;
                cto = ct_arr->child_count;
            }
            else cto = ct_arr->child_count;
        }
        vt_arr->from[froms] = cfrom;
        vt_arr->to[froms] = cto;
        int rfrom = -1;
        int rto = -1;
        for(j=0; j<=top_root_values[cvalue]; j++)
        {
            rt_arr->root[++rt_arr->root_count] = root_values[cvalue][j];
            if(j==0) {
                rfrom = rt_arr->root_count;
                rto = rt_arr->root_count;
            }
            else rto = rt_arr->root_count;
        }
        vt_arr->fromr[froms] = rfrom;
        vt_arr->tor[froms] = rto;
        froms++;
    }

    stack_tag->tuple_arrs[stack_tag->top_stack_tag]=*current_states;
    return next_states;
}

/* following two functions handle unmatched ending symbols */
static inline void syntax_inference(stack_tag* stack_tag_b, char* p)
{
	char *sp = p+1;
    if(*sp=='\n') sp = p+2;
    char *endp = sp+1;
    while((*endp<=0)||(*endp==' ')||(*endp==10)||(*endp==2)||(*endp=='\n')||(*endp==3))
        endp++;
    int keytag = 0;
    if(*endp=='"') keytag = 2;
    if((*sp==',')&&(*endp=='"')) {
        endp++;
        while(*endp!='"') {
            endp++;
        }
        while(*endp==' '||*endp==32)
            endp++;
        endp++;
        if(*endp==':') keytag = 1;
    }
    if(thread>0&&!((*sp=='}')||(*sp==','&&keytag==1)))
    {
        stack_tag_b->element[++stack_tag_b->top_stack_tag]=2;
    }
}

static inline void record_unmatched_symbol(int thread_num, tuple_array* current_states, int symbol)
{
	if(symbol!=OBJECT&&symbol!=ARRAY)
	{
		int current = (++start_tree[thread_num].top_tree);
        start_tree[thread_num].element[current].value = symbol;
        if(thread_num>0) current_states->from = -1;
	}
	else
	{
		if(start_tree[thread_num].element[start_tree[thread_num].top_tree].value != symbol){
            int current = (++start_tree[thread_num].top_tree);
            start_tree[thread_num].element[current].value = symbol;
            if(thread_num>0) current_states->from = -1;
		}
	}
	if(symbol==ARRAY) strcopy("]",outputs[thread_num].output[++outputs[thread_num].top_output]);
}

static inline void record_unmatched_primitive(int thread_num, tuple_array* current_states, char* text)
{
	int current = (++start_tree[thread_num].top_tree);
    start_tree[thread_num].element[current].value = PRIMITIVE;
    if(thread_num>0) current_states->from = -1;
	strcopy(text, start_tree[thread_num].element[current].text);
}

/* following three functions handle both temporary outputs and final outputs */
static inline void predicate_output(int thread_num, char* start_text, tuple_array *current_states, query_stack* q_stack)
{
	value_tuple *vt_arr = q_stack->vt;
	root_tuple *rt_arr = q_stack->rt;
	int tempk;
    for(tempk=current_states->from; tempk<=current_states->to; tempk++)
    {
        int values = vt_arr->values[tempk];
        if(values!=0&&stateMachine[2*values-1].isoutput>0)
        {
            strcopy("array", start_text);
            if(stateMachine[2*values-1].isoutput == 2)
            {
                sprintf(start_text, "%d", values);
            }
            if(strcmp(start_text,"array")!=0) {
                int top_output = (++outputs[thread_num].top_output);
                strcopy(start_text, outputs[thread_num].output[top_output]);
                int xb;
                for(xb = vt_arr->fromr[tempk]; xb <=vt_arr->tor[tempk]; xb++)
                {
                    int segment_index = tsegs[thread_num].num_segs;
                    int temp_state = rt_arr->root[xb];
                    int temp_top = rot[thread_num].top_roots[segment_index][temp_state];
                    if(temp_top>-1)
                    {
                        if(top_output ==  rot[thread_num].roots[segment_index][temp_state][temp_top].to + 1)
                        {
                            rot[thread_num].roots[segment_index][temp_state][temp_top].to++;
                        }
                        else {
                            temp_top = (++rot[thread_num].top_roots[segment_index][temp_state]);
                            rot[thread_num].roots[segment_index][temp_state][temp_top].from = top_output;
                            rot[thread_num].roots[segment_index][temp_state][temp_top].to = top_output;
                        }
                    }
                    else
                    {
                        temp_top = (++rot[thread_num].top_roots[segment_index][temp_state]);
                        rot[thread_num].roots[segment_index][temp_state][temp_top].from = top_output;
                        rot[thread_num].roots[segment_index][temp_state][temp_top].to = top_output;
                    }
                }
            }
        }
    }
}

static inline void array_output(int thread_num, char * start_text, tuple_array * current_states, query_stack* q_stack) {
    value_tuple *vt_arr = q_stack->vt;
	root_tuple *rt_arr = q_stack->rt;
	int tempk;
    for (tempk = current_states->from; tempk <= current_states->to; tempk++) {
        int values = vt_arr->values[tempk];
        if (values != 0 && stateMachine[2 * values - 1].isoutput == 2) {
            int top_output = (++outputs[thread_num].top_output);
            if (stateMachine[2 * values - 1].isoutput == 2) {
                sprintf(start_text, "%d", values);
            }
            strcopy("]", start_text);
            strcopy(start_text, outputs[thread_num].output[top_output]);
            int xb;
            for (xb = vt_arr->fromr[tempk]; xb <= vt_arr->tor[tempk]; xb++) {
                int segment_index = tsegs[thread_num].num_segs;
                int temp_state = rt_arr->root[xb];
                int temp_top = rot[thread_num].top_roots[segment_index][temp_state];
                if (temp_top > -1) {
                    if (top_output == rot[thread_num].roots[segment_index][temp_state][temp_top].to + 1) {
                        rot[thread_num].roots[segment_index][temp_state][temp_top].to++;
                    } else {
                        temp_top = (++rot[thread_num].top_roots[segment_index][temp_state]);
                        rot[thread_num].roots[segment_index][temp_state][temp_top].from = top_output;
                        rot[thread_num].roots[segment_index][temp_state][temp_top].to = top_output;
                    }
                } else {
                    temp_top = (++rot[thread_num].top_roots[segment_index][temp_state]);
                    rot[thread_num].roots[segment_index][temp_state][temp_top].from = top_output;
                    rot[thread_num].roots[segment_index][temp_state][temp_top].to = top_output;
                }
            }
        }
    }
}

static inline void query_output(int thread_num, char* start_text, tuple_array *current_states, query_stack* q_stack)
{
	value_tuple *vt_arr = q_stack->vt;
	root_tuple *rt_arr = q_stack->rt;
	int tempk;
    for(tempk=current_states->from; tempk<=current_states->to; tempk++)
    {
        int values = vt_arr->values[tempk];
        if(values!=0&&stateMachine[2*values-1].isoutput ==1&&(stateMachine[2*values-1].low==0||(vt_arr->counter[values]>=stateMachine[2*values-1].low&&vt_arr->counter[values]<=stateMachine[2*values-1].high)))  //cs 299
        {
            int top_output = (++outputs[thread_num].top_output);
            int tl = strlen(start_text);
            start_text[tl] = '"';
            start_text[tl+1] = '\0';
            strcopy(start_text, outputs[thread_num].output[top_output]);
            int xb;
            for(xb = vt_arr->fromr[tempk]; xb <=vt_arr->tor[tempk]; xb++)
            {
                int segment_index = tsegs[thread_num].num_segs;
                int temp_state = rt_arr->root[xb];
                int temp_top = rot[thread_num].top_roots[segment_index][temp_state];
                if(temp_top>-1)
                {
                    if(top_output ==  rot[thread_num].roots[segment_index][temp_state][temp_top].to + 1)
                    {
                        rot[thread_num].roots[segment_index][temp_state][temp_top].to++;
                    }
                    else {
                        temp_top = (++rot[thread_num].top_roots[segment_index][temp_state]);
                        rot[thread_num].roots[segment_index][temp_state][temp_top].from = top_output;
                        rot[thread_num].roots[segment_index][temp_state][temp_top].to = top_output;
                    }
                }
                else
                {
                    temp_top = (++rot[thread_num].top_roots[segment_index][temp_state]);
                    rot[thread_num].roots[segment_index][temp_state][temp_top].from = top_output;
                    rot[thread_num].roots[segment_index][temp_state][temp_top].to = top_output;
                }
            }
        }
    }
}

static inline void pop(tuple_array *current_states, stack_tag* stack_tag, value_tuple *vt_arr, child_tuple *ct_arr, root_tuple *rt_arr)
{
	*current_states=stack_tag->tuple_arrs[stack_tag->top_stack_tag];
	vt_arr->value_count=current_states->to;
	ct_arr->child_count = vt_arr->to[vt_arr->value_count];
    rt_arr->root_count = vt_arr->tor[vt_arr->value_count];
	--stack_tag->top_stack_tag;
}

static inline void generate_constraints(char* start_text, tuple_array *current_states, tuple_array *next_states, value_tuple *vt_arr)
{
    int po = 0;
    for(po =0; po<=top_tags; po++)
    {
        if(strcmp(tags[po].tagname, start_text)==0) break;
    }
    if(po<=top_tags)
    {
        int tp = 0;
        for(tp = 0; tp<=tags[po].top_point; tp++)
        {
            if(tags[po].start[tp]==vt_arr->values[current_states->from]&&tags[po].end[tp]==vt_arr->values[next_states->from]) break;
        }
        if(tp>tags[po].top_point) {
            int point = (++tags[po].top_point);
            tags[po].start[point] = vt_arr->values[current_states->from];
            tags[po].end[point]  = vt_arr->values[next_states->from];
        }
    }
    else {
        strcopy(start_text,tags[++top_tags].tagname);
        tags[top_tags].top_point = -1;
        int point = (++tags[top_tags].top_point);
        tags[top_tags].start[point] = vt_arr->values[current_states->from];
        tags[top_tags].end[point]  = vt_arr->values[next_states->from];
    }
}

//update syntax stack
static inline void update_syntax_info(int thread_num, stack_tag* syn_stack, int value, int operation)  //update syntax stack 
{
	if(operation == PUSH)
	{
		syn_stack->element[++syn_stack->top_stack_tag]=value;
	}
	else if(operation == POP)
	{
		--syn_stack->top_stack_tag;
	}
}

//update query stack
static inline void update_query_info(int thread_num, tuple_array* current_states, char* key_text, stack_tag* syn_stack, query_stack* q_stack, int operation)  //update query state and stack
{
	if(operation == PUSH)
	{
		tuple_array next_states = pushing(thread_num, key_text, current_states, syn_stack, q_stack->vt, q_stack->ct, q_stack->rt);			
		if(runtime==1)
        {
            generate_constraints(key_text, current_states, &next_states, q_stack->vt);
	    }
	    *current_states = next_states;
	}
	else if(operation == POP)
	{
		*current_states=syn_stack->tuple_arrs[syn_stack->top_stack_tag];
		q_stack->vt->value_count = current_states->to;
		q_stack->ct->child_count = q_stack->vt->to[q_stack->vt->value_count];
        q_stack->rt->root_count = q_stack->vt->tor[q_stack->vt->value_count];
	}
}

//update counter information
static inline void update_counter(tuple_array* current_states, query_stack* q_stack)
{
	int tx;
    for(tx = current_states->from; tx<=current_states->to; tx++)
    {
        q_stack->vt->counter[tx]++;
    }
}

/* following 10 functions implement state transitions in Figure 7 of our paper */
static inline void obj_s(int thread_num, stack_tag* syn_stack)
{
	update_syntax_info(thread_num, syn_stack, LBRACE, PUSH);
}

static inline void ary_s(int thread_num, tuple_array* current_states, stack_tag* syn_stack, query_stack* q_stack)
{
	update_syntax_info(thread_num, syn_stack, LBRACKET, PUSH);
	update_query_info(thread_num, current_states, "array", syn_stack, q_stack, PUSH);
}

static inline void key(int thread_num, tuple_array* current_states, char* key_text, stack_tag* syn_stack, query_stack* q_stack)
{
	update_syntax_info(thread_num, syn_stack, KEY, PUSH);
	update_query_info(thread_num, current_states, key_text, syn_stack, q_stack, PUSH);
}

static inline void obj_e(int thread_num, stack_tag* syn_stack)
{
	update_syntax_info(thread_num, syn_stack, NULL, POP);
}

static inline void val_obj_e(int thread_num, tuple_array* current_states, stack_tag* syn_stack, query_stack* q_stack)
{
	update_syntax_info(thread_num, syn_stack, NULL, POP);
	update_query_info(thread_num, current_states, 0, syn_stack, q_stack, POP);
	update_syntax_info(thread_num, syn_stack, NULL, POP);
}

static inline void elt_obj_e(int thread_num, stack_tag* syn_stack)
{
	update_syntax_info(thread_num, syn_stack, NULL, POP);
}

static inline void ary_e(int thread_num, char* array_text, tuple_array* current_states, stack_tag* syn_stack, query_stack* q_stack)
{
	update_query_info(thread_num, current_states, 0, syn_stack, q_stack, POP);
	update_syntax_info(thread_num, syn_stack, NULL, POP);
}

static inline void val_ary_e(int thread_num, tuple_array* current_states, stack_tag* syn_stack, query_stack* q_stack)
{
	update_query_info(thread_num, current_states, 0, syn_stack, q_stack, POP);
	update_syntax_info(thread_num, syn_stack, NULL, POP);
	update_query_info(thread_num, current_states, 0, syn_stack, q_stack, POP);
    update_syntax_info(thread_num, syn_stack, NULL, POP);
}

static inline void elt_ary_e(int thread_num,tuple_array* current_states, stack_tag* syn_stack, query_stack* q_stack)
{
	update_query_info(thread_num, current_states, 0, syn_stack, q_stack, POP);
	update_syntax_info(thread_num, syn_stack, NULL, POP);
}

static inline void val_pmt(int thread_num, tuple_array* current_states, char* text, stack_tag* syn_stack, query_stack* q_stack)
{
	update_query_info(thread_num, current_states, 0, syn_stack, q_stack, POP);
	update_syntax_info(thread_num, syn_stack, NULL, POP);
}

//get top element from syntax stack
static inline int top_syntax(stack_tag* syn_stack)
{
	if(syn_stack->top_stack_tag>-1)
	    return syn_stack->element[syn_stack->top_stack_tag];
	else return NULL;
}

//get second top element from syntax stack
static inline int top_syntax_second(stack_tag* syn_stack)
{
	if(syn_stack->top_stack_tag>0)
	    return syn_stack->element[syn_stack->top_stack_tag-1];
	else return NULL;
}

//get current total number of segments
static inline int get_current_segment(int thread_num)
{
	return tsegs[thread_num].num_segs;
}

//update segment information after automata restarts
static inline void update_segment_position(int thread_num, int pre_segment, token_info* tInfo)
{
	tsegs[thread_num].ele[pre_segment].end = tInfo->current-buffFiles[thread_num]-1;
    tsegs[thread_num].ele[tsegs[thread_num].num_segs].start =  tInfo->current-buffFiles[thread_num];
}

//initialize tokens
static inline void token_info_initialization(xml_Text *pText, xml_Token *pToken, token_info* tInfo, int thread_num)
{
    tInfo->start = pToken->text.p + pToken->text.len;
    tInfo->head = buffFiles[thread_num];
    tInfo->current = tInfo->start;
    tInfo->end = pText->p + pText->len;
    tInfo->lex_state = (int*)malloc(sizeof(int));
    *(tInfo->lex_state)= 0;
    if(runtime == 1) tInfo->end = tInfo->start+(tInfo->end-tInfo->start)*percentage/100;
    if(temp_err == 1) tInfo->end=tInfo->end-1;  //control errors from data constraint learning
}

//initialize parser information
static inline void parser_info_initialization(tuple_array* current_states, stack_tag* syn_stack, query_stack* q_stack, int thread_num)
{
	current_states->from=-1;
	syn_stack->top_stack_tag = -1;
    tsegs[thread_num].num_segs = -1;
    tsegs[thread_num].ele[0].start = 0;
    q_stack->ct->child_count = -1;
    q_stack->vt->value_count = -1;
    q_stack->rt->root_count = -1;
    rt_array[thread_num].root_count = -1;
    int i,j;
    for(i=0;i<20;i++)
    {
        for(j=0;j<50;j++)
           rot[thread_num].top_roots[i][j] = -1;
    }
}

//update global parser information for current thread once it finishes
static inline void global_updates(int thread_num, tuple_array* current_states, stack_tag* syn_stack, query_stack* q_stack, token_info* tInfo)
{
	tsegs[thread_num].ele[tsegs[thread_num].num_segs].end = tInfo->current-buffFiles[thread_num] -1;
    syn_stack->element[++syn_stack->top_stack_tag] = TEMP;
    syn_stack->tuple_arrs[syn_stack->top_stack_tag] = *current_states;
    stack_tags[thread_num].top_stack_tag = syn_stack->top_stack_tag;
    int i;
    for(i=0;i<=syn_stack->top_stack_tag;i++)
    {
        stack_tags[thread_num].element[i] = syn_stack->element[i];
        stack_tags[thread_num].tuple_arrs[i] = syn_stack->tuple_arrs[i];   
	}
    for(i=0;i<=q_stack->vt->value_count;i++)
    {
        vt_array[thread_num].values[i] = q_stack->vt->values[i];
        vt_array[thread_num].from[i] = q_stack->vt->from[i];
        vt_array[thread_num].to[i] = q_stack->vt->to[i];
        vt_array[thread_num].fromr[i] = q_stack->vt->fromr[i];
        vt_array[thread_num].tor[i] = q_stack->vt->to[i];
        vt_array[thread_num].counter[i] = q_stack->vt->counter[i];
    }
    vt_array[thread_num].value_count = q_stack->vt->value_count;
    for(i=0;i<=q_stack->ct->child_count;i++)
    {
        ct_array[thread_num].children[i] = q_stack->ct->children[i];
    }
    ct_array[thread_num].child_count = q_stack->ct->child_count;
    for(i=0;i<=q_stack->rt->root_count;i++)
    {
        rt_array[thread_num].root[i] = q_stack->rt->root[i]; 
    }
    rt_array[thread_num].root_count = q_stack->rt->root_count;
    
    qstack[thread_num].vt = &vt_array[thread_num];
    qstack[thread_num].ct = &ct_array[thread_num];
    qstack[thread_num].rt = &rt_array[thread_num];
}

/************************************************************************************************************************************************
Function: int streaming_automata(xml_Text *pText, xml_Token *pToken);
Description: finish streaming compilation; elements in semi-structure data are used as inputs; both syntax stack and query stack are used.
Input: pText-the content of the json file; pToken-the type of the current json element; 
Return: 0--success -1--error 
*************************************************************************************************************************************************/
int streaming_automata(xml_Text *pText, xml_Token *pToken)  
{
	int thread_num = 0;
	//lexer info
	token_info tInfo;
	token_info_initialization(pText, pToken, &tInfo, thread_num);
	char text[40000];
	
	//parser info 
	tuple_array current_states;
	struct stack_tag syn_stack;    //syntax stack
	struct stack_tag syn_stack_back;
    syn_stack_back.top_stack_tag = -1;
    child_tuple ct_arr;  //save parent states
    value_tuple vt_arr;  //stack in tree structure, save states
    root_tuple rt_arr; //save root states
    query_stack q_stack;      //query stack
    q_stack.vt = &vt_arr;
    q_stack.ct = &ct_arr;
    q_stack.rt = &rt_arr;
    parser_info_initialization(&current_states, &syn_stack, &q_stack, thread_num);
    int pre_segment; 
	int lex = lexer(pText, pToken, &tInfo, text);
	while(lex!=-1)
	{
		switch(lex)
		{
			case LCB:
				obj_s(thread_num, &syn_stack);
				break;
			case RCB:
				if(syn_stack.top_stack_tag == 0)
				{
					obj_e(thread_num, &syn_stack);
                    update_counter(&current_states, &q_stack);
				}
				else if(syn_stack.top_stack_tag > 0)
				{
					if(top_syntax_second(&syn_stack)==KEY)
					{
						val_obj_e(thread_num, &current_states, &syn_stack, &q_stack);
					}
					else if(top_syntax_second(&syn_stack)==LBRACKET)
					{
						elt_obj_e(thread_num, &syn_stack);
						update_counter(&current_states, &q_stack);
					}
				}
				break;
			case LB:
				ary_s(thread_num, &current_states, &syn_stack, &q_stack);
				//predicate output
                predicate_output(thread_num, text, &current_states, &q_stack);
                break;
            case RB:
                if(syn_stack.top_stack_tag >= 0) array_output(thread_num, text, &current_states, &q_stack);
                if(syn_stack.top_stack_tag == 0)
				{
					ary_e(thread_num, "array", &current_states, &syn_stack, &q_stack);
                    update_counter(&current_states, &q_stack);
				}
				else if(syn_stack.top_stack_tag > 0)
				{
					if(top_syntax_second(&syn_stack)==KEY)
					{
						val_ary_e(thread_num, &current_states, &syn_stack, &q_stack);
					}
					else if(top_syntax_second(&syn_stack)==LBRACKET)
					{
						elt_ary_e(thread_num, &current_states, &syn_stack, &q_stack);
						update_counter(&current_states, &q_stack);
					}
				}
			    break;
			case COM:  //ignore comma
			    break;
			case KY:
				key(thread_num, &current_states, text, &syn_stack, &q_stack);
                //temporary output
                predicate_output(thread_num, text, &current_states, &q_stack);
                break;
            case PRI:
            	if(syn_stack.top_stack_tag >= 0)
			    {
					if(top_syntax(&syn_stack)==KEY)
					{
						query_output(thread_num, text, &current_states, &q_stack);
						val_pmt(thread_num, &current_states, text, &syn_stack, &q_stack);
					}
					else if(top_syntax(&syn_stack)==LBRACKET)
					{
						update_counter(&current_states, &q_stack);
						query_output(thread_num, text, &current_states, &q_stack);
					}
				}
				break;
		}
		lex = lexer(pText, pToken, &tInfo, text);
	}
	//write results into global variables
	global_updates(thread_num, &current_states, &syn_stack, &q_stack, &tInfo);
    return 0;
}

/************************************************************************************************************************************************
Function: int parallel_automata(xml_Text *pText, xml_Token *pToken, int thread_num);
Description: execute streaming compilation in parallel; inputs are separated into different chunks; both syntax stack and query stack are used.
Input: pText-the content of the json file; pToken-the type of the current json element; thread_num-the number of the thread; 
Return: 0--success -1--error 
*************************************************************************************************************************************************/
int parallel_automata(xml_Text *pText, xml_Token *pToken, int thread_num)  
{
	//lexer info
	token_info tInfo;
	token_info_initialization(pText, pToken, &tInfo, thread_num);
	char text[40000];
	
	//parser info 
	tuple_array current_states;
	struct stack_tag syn_stack;    //syntax stack
	struct stack_tag syn_stack_back;
    syn_stack_back.top_stack_tag = -1;
    child_tuple ct_arr;  //save parent states
    value_tuple vt_arr;  //stack in tree structure, save states
    root_tuple rt_arr; //save root states
    query_stack q_stack;      //query stack
    q_stack.vt = &vt_arr;
    q_stack.ct = &ct_arr;
    q_stack.rt = &rt_arr;
    parser_info_initialization(&current_states, &syn_stack, &q_stack, thread_num);
    int pre_segment; 
	int lex = lexer(pText, pToken, &tInfo, text);
	while(lex!=-1)
	{
		switch(lex)
		{
			case LCB:
				obj_s(thread_num, &syn_stack);
				break;
			case RCB:
				if(syn_stack.top_stack_tag == -1)  //unmatched ending symbol 
			    {
					record_unmatched_symbol(thread_num, &current_states, RBRACE);
					//syntax feasible path inference
                    if(syn_stack_back.top_stack_tag==-1){
                        syntax_inference(&syn_stack_back, tInfo.current);
                    }
				}
				else if(syn_stack.top_stack_tag == 0)
				{
					obj_e(thread_num, &syn_stack);
					if(syn_stack_back.top_stack_tag==-1){
                        syntax_inference(&syn_stack_back, tInfo.current);
                    }
                    update_counter(&current_states, &q_stack);
                    record_unmatched_symbol(thread_num, &current_states, OBJECT);
				}
				else if(syn_stack.top_stack_tag > 0)
				{
					if(top_syntax_second(&syn_stack)==KEY)
					{
						val_obj_e(thread_num, &current_states, &syn_stack, &q_stack);
					}
					else if(top_syntax_second(&syn_stack)==LBRACKET)
					{
						elt_obj_e(thread_num, &syn_stack);
						update_counter(&current_states, &q_stack);
					}
				}
				break;
			case LB:
				pre_segment = get_current_segment(thread_num);
				ary_s(thread_num, &current_states, &syn_stack, &q_stack);
				//predicate output
                predicate_output(thread_num, text, &current_states, &q_stack);
                update_segment_position(thread_num, pre_segment, &tInfo);
                break;
            case RB:
            	if(syn_stack.top_stack_tag == -1)  //unmatched ending symbol 
				{
					record_unmatched_symbol(thread_num, &current_states, RBRACKET);
					//syntax feasible path inference
                    if(syn_stack_back.top_stack_tag==-1){
                        syntax_inference(&syn_stack_back, tInfo.current);
                    }
				}
                if(syn_stack.top_stack_tag >= 0) array_output(thread_num, text, &current_states, &q_stack);
                if(syn_stack.top_stack_tag == 0)
				{
					ary_e(thread_num, "array", &current_states, &syn_stack, &q_stack);
					if(syn_stack_back.top_stack_tag==-1){
                        syntax_inference(&syn_stack_back, tInfo.current);
                    }
                    update_counter(&current_states, &q_stack);
                    record_unmatched_symbol(thread_num, &current_states, ARRAY);
				}
				else if(syn_stack.top_stack_tag > 0)
				{
					if(top_syntax_second(&syn_stack)==KEY)
					{
						val_ary_e(thread_num, &current_states, &syn_stack, &q_stack);
					}
					else if(top_syntax_second(&syn_stack)==LBRACKET)
					{
						elt_ary_e(thread_num, &current_states, &syn_stack, &q_stack);
						update_counter(&current_states, &q_stack);
					}
				}
			    break;
			case COM:  //ignore comma
			    break;
			case KY:
				pre_segment = get_current_segment(thread_num);
				key(thread_num, &current_states, text, &syn_stack, &q_stack);
                //temporary output
                predicate_output(thread_num, text, &current_states, &q_stack);
                update_segment_position(thread_num, pre_segment, &tInfo);
                break;
            case PRI:
            	if(syn_stack.top_stack_tag==-1)
				{
					record_unmatched_primitive(thread_num, &current_states, text);
				}
				else if(syn_stack.top_stack_tag >= 0)
			    {
					if(top_syntax(&syn_stack)==KEY)
					{
						query_output(thread_num, text, &current_states, &q_stack);
						val_pmt(thread_num, &current_states, text, &syn_stack, &q_stack);
					}
					else if(top_syntax(&syn_stack)==LBRACKET)
					{
						update_counter(&current_states, &q_stack);
						query_output(thread_num, text, &current_states, &q_stack);
					}
				}
				break;
		}
		lex = lexer(pText, pToken, &tInfo, text);
	}
	//write results into global variables
	global_updates(thread_num, &current_states, &syn_stack, &q_stack, &tInfo);
    return 0;
}

//reprocessing misspeculative part
static inline void reprocess(int n, int* temp_err, tuple_array *current_states)
{
	struct timeval begin,end;
    gettimeofday(&begin,NULL);
    xml_Text xml;
    xml_Token token;
    int multiExp = 0;
    int multiCDATA = 0;
    int j;
    createTree1(n+1);
    int structure_type=0;
    outputs[n+1].top_output=-1;
    xml_initText(&xml,buffFiles[n+1]);
    xml_initToken(&token, &xml);
    special_start = vt_array[0].values[current_states->from];
    int ret = parallel_automata(&xml, &token, n+1);
    printf("return from reprocessing\n");
    if(stack_tags[n+1].top_stack_tag==0) vt_array[0].values[current_states->from] = vt_array[n+1].values[vt_array[n+1].value_count] ;
    printf("first return value %d\n", vt_array[0].values[current_states->from]);
    if(outputs[n+1].top_output>-1) {
        tuple_array tuple;
        tuple.from = outputs[0].top_output+1;
        int o;
        for(o = 0; o<= outputs[n+1].top_output; o++)
            strcopy(outputs[n+1].output[o], outputs[0].output[++outputs[0].top_output]);
        tuple.to = outputs[0].top_output;
    }
    *temp_err = 0;
    gettimeofday(&end,NULL);
    double duration = 1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec;
    reprocessing_time += duration/1000000;
}

/* following 5 functions work for merging phase */
static inline void get_current_states(int thread_num, tuple_array* current_states)
{
	*current_states = stack_tags[thread_num].tuple_arrs[stack_tags[thread_num].top_stack_tag--];
}

static inline void add_final_output(tuple_array* current_states, char* text, query_stack* q_stack)
{
	int current_value = q_stack->vt->values[current_states->to];
    if(stateMachine[2*current_value-1].isoutput == 1)
    {
        int tvalue = q_stack->vt->values[current_states->from];
        int temp_count = (++q_stack->vt->counter[tvalue]);
        if(temp_count>=stateMachine[2*tvalue-1].low&&temp_count<=stateMachine[2*tvalue-1].high)
            strcopy(text, outputs[0].output[++outputs[0].top_output]);
    }
}

static inline void predicate_updates(int thread_num, tuple_array* current_states, query_stack* q_stack)
{
	//update counter
    int i = thread_num;
    q_stack->vt->counter[current_states->from]++;
    int current_value = q_stack->vt->values[current_states->to];
    if(stateMachine[2*current_value-1].isoutput == 2)  //output for filtering
    {
        int ttop = (++rot[0].top_roots[0][1]);
        rot[0].roots[0][1][ttop].from = -2;
        rot[0].roots[0][1][ttop].to = -2; 
        rot[0].thread_id[ttop] = i;                          
    }
    q_stack->vt->counter[current_states->to] = 0;
}

static inline void collect_final_outputs_for_units(int thread_num, int segment_no, tuple_array* current_states, query_stack* q_stack, int* tag_for_curly)
{
	int i,j,k;
	i = thread_num;
	j = segment_no;
	for(k=0;i>0&&k<=tsegs[i].ele[j].num_values;k++)
	{
		if(tsegs[i].ele[j].values[k]==q_stack->vt->values[current_states->from])
		{
            int tvalue = q_stack->vt->values[current_states->from];
            int tcounter = q_stack->vt->counter[current_states->from]; 
            int tlow;
            if(stateMachine[2*tvalue-1].low - tcounter >=0)
                tlow = stateMachine[2*tvalue-1].low;
            else tlow = 0;
            int thigh = stateMachine[2*tvalue-1].high;
            int xk;
            int times = tcounter;
            for(xk = 0; xk <= rot[i].top_roots[j][tvalue] && times <=thigh; xk++)
            { 
                if(times>=tlow && times<=thigh){
                    int ttop = (++rot[0].top_roots[0][1]);
                    tuple_array ttuple;
                    rot[0].roots[0][1][ttop] = rot[i].roots[j][tvalue][xk];
                    rot[0].thread_id[ttop] = i;
                }
                if(strcmp(outputs[i].output[rot[i].roots[j][tvalue][xk].to+1],"}")==0|| strcmp(outputs[i].output[rot[i].roots[j][tvalue][xk].to+1],"]")==0)
                {
                    *tag_for_curly = 1;
                    times = (++vt_array[0].counter[current_states->from]);
                }
            }
			break;
		}
	}
}

static inline void merge_stack(tuple_array *current_states, stack_tag* src_syn_stack, stack_tag* tar_syn_stack, query_stack* src_q_stack, query_stack* tar_q_stack)
{
    int j;
	for(j=0;j<src_syn_stack->top_stack_tag;j++)
	{
        if(j<src_syn_stack->top_stack_tag){
			tar_syn_stack->element[++tar_syn_stack->top_stack_tag]=src_syn_stack->element[j];
			tar_syn_stack->tuple_arrs[tar_syn_stack->top_stack_tag]=*current_states;
		}
        int next_one;
		if(j>=0&&(src_syn_stack->element[j]==KEY ||src_syn_stack->element[j]==LBRACKET||src_syn_stack->element[j]==TEMP))
		{
            next_one = j+1;
            while(next_one<=src_syn_stack->top_stack_tag)
            {
                if(src_syn_stack->element[next_one]==KEY ||src_syn_stack->element[next_one]==LBRACKET||src_syn_stack->element[next_one]==TEMP)
                    break;
                next_one++;
            }
			int from = src_syn_stack->tuple_arrs[next_one].from;
            int to = src_syn_stack->tuple_arrs[next_one].to;
            while(from<=to)
			{
                int cfrom = src_q_stack->vt->from[from];
				int cto = src_q_stack->vt->to[from];
				int value = src_q_stack->vt->values[from];
                while(cfrom>-1&&cfrom<=cto)
				{
                    if(src_q_stack->ct->children[cfrom]==tar_q_stack->vt->values[current_states->from])
					{
						break;	
					}
					cfrom++;
				}
				if(cfrom>-1&&cfrom<=cto)
				{
					int vt_index= (++tar_q_stack->vt->value_count);
					tar_q_stack->vt->values[vt_index] = value;
                    tar_q_stack->vt->counter[vt_index] = src_q_stack->vt->counter[from];
					current_states->from = vt_index;
					current_states->to = vt_index;
					break;
				}
				from++;
			}
		}
	}
}

/****************************************************
Function: void merge(int n)
Description: merge results generated from all threads
****************************************************/
void merge(int n)
{
    int error_tag = 0;
    rot[0].top_roots[0][1] = -1;
	int i,j,k;
	tuple_array current_states;
	get_current_states(0, &current_states);  //0 is the first thread
	///iterate through all threads sequentially except for the first one
    for(i=1;i<=n;i++)
	{  
        error_tag = 0;
		int unmatched_ending_index = 0;  //special ending symbols
        int tag_for_curly  = 0;
        //segment combination
        //iterate through each unit (automata restarts)
		for(j=0;j<=tsegs[i].num_segs||unmatched_ending_index<=start_tree[i].top_tree;j++)
		{
			int unmatched_ending_index_end;
			if(j<=tsegs[i].num_segs) unmatched_ending_index_end = tsegs[i].ele[j].start_tree_sp;
			else unmatched_ending_index_end = start_tree[i].top_tree;
			//process unmatched ending symbols
			while(unmatched_ending_index<=unmatched_ending_index_end)
			{
				int unmatched_ending_symbol = start_tree[i].element[unmatched_ending_index].value;
				if(unmatched_ending_symbol == RBRACE)
				{
					if(stack_tags[0].top_stack_tag==0)
					{
						obj_e(0, &stack_tags[0]);
					}
					else if(stack_tags[0].top_stack_tag>0)
					{
						if(top_syntax_second(&stack_tags[0])==KEY)
						{
							val_obj_e(0, &current_states, &stack_tags[0], &qstack[0]);
						}
						else if(top_syntax_second(&stack_tags[0])==LBRACKET)
						{
							elt_obj_e(0, &stack_tags[0]);
							update_counter(&current_states, &qstack[0]);
						}
					}
				}
				else if(unmatched_ending_symbol==PRIMITIVE ||unmatched_ending_symbol==OBJECT || unmatched_ending_symbol==ARRAY)
				{
                    if((unmatched_ending_symbol==OBJECT||unmatched_ending_symbol==ARRAY) && tag_for_curly == 0)
                    {
                    	//update counter
                        update_counter(&current_states, &qstack[0]);
                    }
                    else tag_for_curly = 0;
					                    
					if(top_syntax(&stack_tags[0])==KEY)
					{
						add_final_output(&current_states, start_tree[i].element[unmatched_ending_index].text, &qstack[0]);
						val_pmt(0, &current_states, start_tree[i].element[unmatched_ending_index].text, &stack_tags[0], &qstack[0]);
				    }
                    else if(unmatched_ending_symbol==PRIMITIVE&&top_syntax(&stack_tags[0])==LBRACKET)
                    {
                    	add_final_output(&current_states, start_tree[i].element[unmatched_ending_index].text, &qstack[0]);
                    } 
				}
                else if(unmatched_ending_symbol=RBRACKET)
                {
                	if(top_syntax(&stack_tags[0])==LBRACKET)
                	{
                		predicate_updates(i, &current_states, &qstack[0]);
					}
                	if(stack_tags[0].top_stack_tag == 0)
					{
					    ary_e(0, "array", &current_states, &stack_tags[0], &qstack[0]);
					}
					else if(stack_tags[0].top_stack_tag > 0)
					{
						if(top_syntax_second(&stack_tags[0])==KEY)
						{
							val_ary_e(0, &current_states, &stack_tags[0], &qstack[0]);
						}
						else if(top_syntax_second(&stack_tags[0])==LBRACKET)
						{
							elt_ary_e(0, &current_states, &stack_tags[0], &qstack[0]);
							update_counter(&current_states, &qstack[0]);
						}
					}
                }
				unmatched_ending_index++;
			}
            if(j>tsegs[i].num_segs)
			    break;
			//collect output elements every time when automata restarts
			collect_final_outputs_for_units(i, j, &current_states, &qstack[0], &tag_for_curly);
		    //reprocessing when speculation is wrong
            if(k>tsegs[i].ele[j].num_values){
                temp_err = 1;
                error_tag = 1;
                error_seg++;
                if(tsegs[i].ele[j].start==-1) tsegs[i].ele[j].start=0;
                buffFiles[n+1] =substring(buffFiles[i],tsegs[i].ele[j].start, tsegs[i].ele[j].end+1);
                reprocess(n, &temp_err, &current_states);
            }
		}
	    
        int tempi = i;
        if(error_tag == 1 &&stack_tags[n+1].top_stack_tag>0) i = n+1;  
        //merge stack in current chunk
        merge_stack(&current_states, &stack_tags[i], &stack_tags[0], &qstack[i], &qstack[0]);
        if(error_tag == 1 &&stack_tags[i].top_stack_tag>0) i = tempi;
	}
	if(stack_tags[0].top_stack_tag==-1)
	    printf("program has been successfully executed %d %d %d\n",current_states.from,current_states.to,vt_array[0].values[current_states.from]);
	else printf("program has some problems, top stack is %d from %d to %d\n",stack_tags[0].top_stack_tag,current_states.from,current_states.to);
}

/********************************************************
Function: void generate_final_outputs();
Description: get final output strings from merged results
*********************************************************/
void generate_final_outputs()
{
    printf("before generating final outputs\n");
    int i;
    int top_output;
    tuple_array tuple;
    int from,to,thread_no;
    for(i = 0; i <= rot[0].top_roots[0][1]; i++)
    {
        tuple = rot[0].roots[0][1][i];
        from  = tuple.from;
        to    = tuple.to;
        thread_no = rot[0].thread_id[i];
        if(from == -2)
        {
            top_output = (++outputs[0].top_output);
            strcopy("]", outputs[0].output[top_output]); 
            continue;
        }
        while(from <= to)
        {
            top_output = (++outputs[0].top_output);
            strcopy(outputs[thread_no].output[from], outputs[0].output[top_output]);
            from++;
        }
    }
    printf("the number of final outputs are %d\n", outputs[0].top_output);
}

/*void print_output()
{
    printf("the total number of outputs are %d\n", outputs[0].top_output);
    int i;
    for(i = 0; i <= outputs[0].top_output; i++)
    {
        printf(" %s \n", outputs[0].output[i]);
    }
}*/

/*************************************************
Function: void *main_thread(void *arg);
Description: main function for each thread. 
Called By: int main(void);
Input: arg--the number of this thread; 
*************************************************/
void *main_thread(void *arg)
{
	struct timeval begin,end;
	double duration;
    int i=(int)(*((int*)arg));
    if(warmup_flag == 1){
	    gettimeofday(&begin,NULL);
        cpu_set_t mask;
        cpu_set_t get;
        CPU_ZERO(&mask);
        CPU_SET(i, &mask);
        if(pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask)<0)
            printf("CPU failed\n");
        while(1)
        {
            gettimeofday(&end,NULL);
            duration=1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec;
            if(duration>2000000) break;
        }
    }
    printf("start to deal with thread %d.\n",i);
    int ret = 0;
    xml_Text xml;
    xml_Token token;               
    
    int j;
	createTree1(i);
	int structure_type=0;
    outputs[i].top_output=-1; 
    xml_initText(&xml,buffFiles[i]);
    xml_initToken(&token, &xml);
    if(i==0) ret = streaming_automata(&xml, &token);
    else ret = parallel_automata(&xml, &token, i);
    
    if(ret==-1)
    {
    	printf("There is something wrong with your XML format, please check it!\n");
    	printf("finish dealing with thread %d.\n",i);
    	return NULL;
	}
    finish_args[i]=1;
    st_type[i]=structure_type;
    printf("finish dealing with thread %d.\n",i);
    if(debug_flag == 1)
    {
        gettimeofday(&end,NULL);   
        duration=1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec; 
        if(warmup_flag == 1)
            printf("The duration for generating temporary result is %lf\n",duration/1000000-2);
        else printf("The duration for generating temporary result is %lf\n",duration/1000000); 
    }
	return NULL;
}

/*************************************************
Function: void thread_wait(int n);
Description: waiting for all the threads finish their tasks. 
Called By: int main(void);
Input: n--the number of all threads; 
*************************************************/
void thread_wait(int n)
{
	int t;
	while(1){
		for( t = 0; t <= n; t++)
	    {
	    	if(finish_args[t]==0)
	    	{
	    		break;
			}
	    }
	    if(t>n) break;
	   usleep(1000);
	}   
}

/*************************************************
Function: void initialization() 
Description: initialize some global variables
*************************************************/
void initialization()
{
    stateCount=0; 
    machineCount=1; 
    top_XPath=-1;
    top_relation=-1;
    top_mXPath=-1;
    top_b_states=-1;
    top_tags=-1;
    top_tstack=-1;
    final_top_output=-1;
    final_last_output=-1;
    top_p_condition=-1;
    top_c_condition=-1;
    top_x_mapping=-1;
    err_segments=0.0;
    top_t_strings=-1;
    top_pstate = -1; 
    temp_err = 0;
    runtime = 0;
    percentage = 1;
}

/***********************************************
Function: int writing_results();
Description: writing results into an output file
Return: 1--success, 0--error
************************************************/
int writing_results()
{
	int ret;
	printf("writting final results into %s\n", "result_0.txt");
    if(top_pstate>-1)
        ret = write_file_with_predicates("result_0.txt", num_threads-1);
    else  ret = write_file("result_0.txt");
    printf("finish writing final results, please check the relevant file\n");
    return ret;
}

/*******************************************
Function: void print_debug_info();
Description: print out more information on 
tracking the performance
********************************************/
void print_debug_info()
{
	printf("statistical information for dataset\n");
    printf("the number of objects is %d; the number of arrays is %d; the number of key-value pairs is %d; the number of basic values is %d\n",num_objects,num_arrays, num_keys, num_values);
    printf("statistical information for execution %d\n",num_threads);
    double segments=1.0;
    double num_starts=1.0;
    double avg_starts=0.0;
    int i,j;
    for(i=1;i<=(num_threads-1);i++){
        for(j=0;j<=tsegs[i].num_segs;j++)
        {
            segments+=1.0;
            num_starts+=((double)tsegs[i].ele[j].num_values+1.0);
        }
    }
    avg_starts=num_starts/segments;
    printf("the average starting path is %lf, the total number of segments is %lf\n",avg_starts,segments);
}

/**********************************************************
Function: void gather_predicate_info();
Description: gather predicate information from stateMachine
***********************************************************/
void gather_predicate_info()
{
	char *out = " is an output"; 
	int i,k;
	int output_indexes[MAX_SIZE];
    int output_values[MAX_SIZE];
    int top_output_indexes=-1; 
    
    for(i=1;i<=machineCount;i=i+2)
    {
        for(k=0;k<=stateMachine[i].n_transitions;k++)
        {
            if(stateMachine[i].isoutput == 0)
            {
                int value_index = 2*stateMachine[i].end[k]-1;
                int xa;
                for(xa=0;xa<=top_output_indexes;xa++)
                {
                    if(output_indexes[xa] == value_index)
                    {
                        output_values[xa] = 0;
                    }
                }
            }
            else if(stateMachine[i].isoutput==1||stateMachine[i].isoutput==2)
            {
                int value_index = 2*stateMachine[i].end[k]-1;
                int output = stateMachine[i].isoutput;
                output_indexes[++top_output_indexes] = value_index;
                output_values[top_output_indexes] = output;
                if(stateMachine[i].start[k]==stateMachine[i].end[k]) output_values[top_output_indexes]=0;
            }
        }
        if(stateMachine[i].isoutput==1||stateMachine[i].isoutput == 2)
            stateMachine[i].isoutput=0;
    }
    int in=0;
    for(in=0;in<=top_output_indexes;in++)
    {
        stateMachine[output_indexes[in]].isoutput=output_values[in];
    }
	
	for(i=1;i<=machineCount;i=i+2)
    {
        stateMachine[i].low = -1;
        stateMachine[i].high = 999999999;
        for(k=0;k<=stateMachine[i].n_transitions;k++)
        {
            if(stateMachine[2*stateMachine[i].end[k]-1].isoutput==2)
            {
                if(strcmp(stateMachine[i].str[k],"array")==0)
                {
                    pstate[++top_pstate].state = stateMachine[i].end[k];
                    pstate[top_pstate].top_condition_list = -1;                
                }
                else
                {
                    int wx;
                    for(wx = 0; wx <= pstate[top_pstate].top_condition_list; wx++)
                    {
                        if(pstate[top_pstate].condition_list[wx] == stateMachine[i].end[k])
                            break;
                    }
                    if(wx > pstate[top_pstate].top_condition_list){
                        int cd = (++pstate[top_pstate].top_condition_list);
                        pstate[top_pstate].condition_list[cd] = stateMachine[i].end[k];
                    }
                }
            }
        }
    }

    if(debug_flag==1)
    {
    	printf("predicate state information\n");
        for(i=0;i<=top_pstate;i++)
        {
            printf("predicate state %d\n", pstate[i].state);
            int ww;
            for(ww=0;ww<=pstate[i].top_condition_list;ww++)
            {
                printf("%dth condition state %d ",ww, pstate[i].condition_list[ww]);
                pstate[i].condition_value[ww] = 0;
            }
            printf("\n");
        }
        
        for(i=1;i<=machineCount;i=i+2)
        {
            printf("%d ",i);
            for(k=0;k<=stateMachine[i].n_transitions;k++)
            {
                printf("start %d",stateMachine[i].start[k]);
                printf(" (str:%s",stateMachine[i].str[k]);
                if(stateMachine[i].isoutput==1)
                {
                    printf("%s",out);
                }
                if(stateMachine[i].isoutput==2)
                {
                    printf("%s %s",out,"special output");
                }
                printf(") end %d;",stateMachine[i].end[k]);
                if(stateMachine[2*stateMachine[i].end[k]-1].isoutput==1)
                    printf("%s",out);
            }
            printf("\n");
       }
        printf("\n");
	}
    stateMachine[0].n_transitions = -1;
}

/**************************************************************
Function: vvoid filter_output_predicates();
Description: extract final outputs for JSONPath with predicates
***************************************************************/
void filter_output_predicates()
{
    printf("%d\n", outputs[0].top_output);
    predicate_states tpstate[50];
    int top_tpstate = top_pstate;
    int i,j,k,l,current_pstate = -1;
    for(i=0;i<=top_tpstate;i++)
    {
        tpstate[i] = pstate[i];
    }
    predicate_stacks[0].top_predicate_stack = -1;
    int tempo_state = 0;
    outputs[1].top_output = -1;  //save the temporary output
    outputs[2].top_output = -1;  //save the final output
    int start = 0;
    int end = outputs[0].top_output+1;
    int start_out_index = -1;

    char temp_buffer[100][500];
    int top_temp_buffer = -1;
    int start_temp = 0;
    int end_temp = outputs[1].top_output;

    int counter = 0;
    int top_testoutput = 0;
    //iterate through every output element from outputs[0]
    for(i = start; i < end; i++)
    {
        if(outputs[0].output[i][strlen(outputs[0].output[i])-1]=='"')  top_testoutput++;
        for(j=0;j<=top_tpstate;j++)
        {
            if(tpstate[j].state==atoi(outputs[0].output[i])) break;
        }
        if(outputs[0].output[i][strlen(outputs[0].output[i])-1]!='"'&&j<=top_pstate){   
            char tempstr[500];
            sprintf(tempstr, "%d", atoi(outputs[0].output[i]));
            if(strcmp(tempstr,outputs[0].output[i])==0)
            {
                predicate_stacks[0].predicate_stack[++predicate_stacks[0].top_predicate_stack] = j;
                predicate_stacks[0].start[predicate_stacks[0].top_predicate_stack] = outputs[1].top_output;
                 continue;
            }
        }
        else if(predicate_stacks[0].top_predicate_stack>-1)
        {
            int index = predicate_stacks[0].predicate_stack[predicate_stacks[0].top_predicate_stack];
            if(outputs[0].output[i][strlen(outputs[0].output[i])-1]=='"') k = tpstate[index].top_condition_list+1;
            else
                        {
                for(k=0;k<=tpstate[index].top_condition_list;k++)
                {
                    if(tpstate[index].condition_list[k]==atoi(outputs[0].output[i]))   
                    {
                        tpstate[index].condition_value[k] = 1;
                        break;
                    }
                }
            }
            if(k>tpstate[index].top_condition_list&& strcmp(outputs[0].output[i], "]") != 0)  
            {
                strcopy(outputs[0].output[i],outputs[1].output[++outputs[1].top_output]);
                continue;
            }
           else if(k<=tpstate[index].top_condition_list) continue;
        }

        if(predicate_stacks[0].top_predicate_stack>-1 && strcmp(outputs[0].output[i], "]") == 0)   
        {
            int satisfy = 0;
            int index = predicate_stacks[0].predicate_stack[predicate_stacks[0].top_predicate_stack];

            for(k=0;k<=tpstate[index].top_condition_list;k++)
            {
                if(tpstate[index].condition_value[k]!=1)
                {
                    break;
                }
            }
            if(k>tpstate[index].top_condition_list)
               satisfy = 1;
            if(satisfy == 0)  
            {
            	int prior = predicate_stacks[0].start[predicate_stacks[0].top_predicate_stack];
                outputs[1].top_output = prior;
                tempo_state = 0;
            }
            else{
                if(predicate_stacks[0].top_predicate_stack == 0 && predicate_stacks[0].predicate_stack[0]==0){ 
                    int starts = 0;
                    if(last_flag==0&&tpstate[predicate_stacks[0].predicate_stack[predicate_stacks[0].top_predicate_stack]].top_condition_list!=-1){
                        for(j=starts;j<=outputs[1].top_output;j++)
                        {
                            if(strcmp(outputs[1].output[j],"]")!=0)
                                strcopy(outputs[1].output[j],outputs[2].output[++outputs[2].top_output]);
                        }
                    }
                    else{  
                        int flagbranket = 0;
                        if(predicate_stacks[0].predicate_stack[predicate_stacks[0].top_predicate_stack]<top_tpstate)
                        {
                            flagbranket = 1;
                        }
                        if(flagbranket == 1)
                        {
                            int index1 = predicate_stacks[0].predicate_stack[predicate_stacks[0].top_predicate_stack];
                            int top1 = outputs[1].top_output;
                            for(j=0;j<tpstate[index1].last_count&&top1>=0;)
                            {
                                if(strcmp(outputs[1].output[top1],"]")==0) {j++;}
                                else
                                {
                                    strcopy(outputs[1].output[top1], outputs[2].output[++outputs[2].top_output]);
                                }
                                top1--;
                            }
                        }
                        else{
                                int index1 = predicate_stacks[0].predicate_stack[predicate_stacks[0].top_predicate_stack];
                            int top1 = outputs[1].top_output;
                            for(j=0;j<tpstate[index1].last_count&&top1>=0;j++)
                            {
                            	 if(strcmp(outputs[1].output[top1],"]")!=0)
                                {
                                    strcopy(outputs[1].output[top1], outputs[2].output[++outputs[2].top_output]);
                                }
                                else break;
                                top1--;
                            }
                        }
                    }
                    outputs[1].top_output = starts-1;
                    tempo_state = 0;
                }
                if(predicate_stacks[0].top_predicate_stack > 0||(predicate_stacks[0].top_predicate_stack==0&&predicate_stacks[0].predicate_stack[0]!=0)){
                    tempo_state = 1;
                    int index1 = predicate_stacks[0].predicate_stack[predicate_stacks[0].top_predicate_stack];
                    if(tpstate[index1].top_condition_list==-1)   
                    {
                        top_temp_buffer = -1;
                        int flagbranket = 0;
                        if(index1<top_tpstate)
                        {
                            flagbranket = 1;
                        }
                        if(flagbranket == 1)
                        {
                            int index1 = predicate_stacks[0].predicate_stack[predicate_stacks[0].top_predicate_stack];
                            int top1 = outputs[1].top_output;
                            for(j=0;j<tpstate[index1].last_count&&top1>=0;)
                            {
                                if(strcmp(outputs[1].output[top1],"]")==0) {j++;}
                                strcopy(outputs[1].output[top1], temp_buffer[++top_temp_buffer]);
                                top1--;
                            }
                            outputs[1].top_output = predicate_stacks[0].start[predicate_stacks[0].top_predicate_stack];          
                        }
                        else{
                            int top1 = outputs[1].top_output;
                            for(j=0;j<tpstate[index1].last_count&&top1-j>=0;j++)
                            {
                                if(strcmp(outputs[1].output[top1-j],"]")==0) break;
                                else
                                {
                                	 strcopy(outputs[1].output[top1-j], temp_buffer[++top_temp_buffer]);
                                }
                            }
                            outputs[1].top_output = predicate_stacks[0].start[predicate_stacks[0].top_predicate_stack];      
                        }
                        for(j=0;j<=top_temp_buffer;j++)
                        {
                            strcopy(temp_buffer[j],outputs[1].output[++outputs[1].top_output]);
                        }
                        top_temp_buffer = -1;
                    }
                    strcopy("]", outputs[1].output[++outputs[1].top_output]);
                    top_temp_buffer = -1;
                }
            }
            for(k=0;k<=tpstate[index].top_condition_list;k++)
            {
               if(tpstate[index].condition_value[k]==1)
               {
                   tpstate[index].condition_value[k] = 0;
               }
            }
            --predicate_stacks[0].top_predicate_stack;
        }
    }
    printf("number of final outputs is %d\n", outputs[2].top_output);
}

/********************************************************
Function: void data_constraint_learning(char* file_name);
Description: learn data constraints from input file
Input: file_name -the name of the json file 
*********************************************************/
void data_constraint_learning0(char* file_name)
{
	load_file(file_name);
    printf("getting all possible start states for each opening symbol (data constraint learning)\n");
    runtime = 1;
    int ret = 0;
    xml_Text xml;
    xml_Token token;
    createTree1(0);
    xml_initText(&xml,buffFiles[0]);
    xml_initToken(&token, &xml);
    ret = streaming_automata(&xml, &token);
    free(buffFiles[0]);
    buffFiles[0] = NULL;	
    printf("finish learning data constraints\n");
    runtime = 0;
}

/*******************************************
Function: int execute_query();
Description: executing input JSONPath query
Return: 1--success, 0--error
********************************************/
int execute_query()
{
	int i,rc;
	struct timeval begin,end;
	double duration;
	gather_predicate_info();
	//data constraint learning
    if(pversion == 1)
    { 
        data_constraint_learning(file_name);
	}
	else top_tags = -1;
	
	//file split
	int n = split_file(file_name,num_threads);
	if(n!=(num_threads-1)) return 0;
	
	//query execution
	printf("begin executing input JSONPath query\n");
	gettimeofday(&begin,NULL);
	for(i=0;i<=n;i++)
    {
        thread_args[i]=i;
    	finish_args[i]=0;
    	rc=pthread_create(&thread[i], NULL, main_thread, &thread_args[i]);  //parallel JSONPath querying
    	if (rc)
        {
            printf("ERROR; return code is %d\n", rc);
            return 0;
        }
	}
	thread_wait(n);
	
	//merging phase
	printf("All the subthread ended, now the program is merging its results.\n");
	printf("begin merging results\n");
	merge(n);
	printf("finish merging these results.\n");
	generate_final_outputs();
	
	//filtering phase
	if(top_pstate>-1)
    {
    	filter_output_predicates();
    	printf("successfully filtered\n");
    }
    gettimeofday(&end,NULL);
    duration=1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec;
    if(warmup_flag == 1)
        printf("the total query execution time is %lf\n", duration/1000000-2);  
    else printf("the total query execution time is %lf\n", duration/1000000); 
    printf("reprocessing time is %lf\n", reprocessing_time);
    
    //free up dynamic memories
    for(i=0;i<=n;i++)
    {
        free(buffFiles[i]);
    }
    return 1;
}

/*********************************************************************************************/
int automata(void)
{
    int ret = 0;
    int choose=-1;
    int n=-1;
    get_multixpath(xmlPath); 
    printf("the number of sub-queries is %d\n",top_XPath+1);
    int j,k;  
    int i,rc;
    
    for(i=0;i<=400;i++)
    {
    	stateMachine[i].n_transitions=-1;
	}
    if(top_XPath==0) j=0;
    else j=1;
    printf("begin creating automata\n");
    for(j=0;j<=top_XPath;j++)
	{
		if(j==top_XPath) createAutoMachine(XPath[j],1);     //create automata by xmlpath
		else if(j>0) createAutoMachine(XPath[j],0);
		else if(j==0) createAutoMachine(XPath[j],2);
	}
	printf("finish creating automata\n");
    printf("machineCount is %d stateCount is %d\n", machineCount, stateCount);
    
  /*  ret = execute_query();
    if(ret==0)
    {
    	printf("query execution exception, please check your input\n");
    	exit(0);
	}
    writing_results();
    if(debug_flag==1) print_debug_info();
    */return 0;
}
