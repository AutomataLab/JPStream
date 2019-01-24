#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#define MAX_SIZE 500
#define MAX_THREAD 80
#define MAX_LINE 100
#define MAX_FILE 1000
/*data structure for files in each thread*/
extern char * buffFiles[MAX_THREAD]; 

#define MAX_OUTPUT 2390000
typedef struct outputele{
	char output[MAX_OUTPUT][MAX_LINE];
	int top_output;
    int supplement[15];
}outputele;

extern outputele outputs[MAX_THREAD];
extern int goggle_flag;

/*data structure for automata*/
typedef struct{
	int start[MAX_THREAD];
	char str[MAX_THREAD][MAX_THREAD];
	int end[MAX_THREAD];
    int low;
    int high;       
	int n_transitions;
	int isoutput; 
}Automata;
extern Automata stateMachine[MAX_SIZE];   //save automata for XPath
 
extern  int stateCount; //the number of states for XPath
extern  int machineCount; //the number of nodes for automata

/* structure of grammer tree 20160520*/
typedef struct grammer_ele{
	char text[MAX_LINE];
	int index;
	int parent;
	int children[MAX_SIZE];
	int child_count;
}grammer_ele;

typedef struct grammer_tree{
	grammer_ele element[1000*MAX_SIZE];
	int root[MAX_LINE];	
	int root_num;
	int top_tree;
}grammer_tree;

//fall 2017 put it inside a function!
extern grammer_tree g_tree;

typedef struct stack{
	int index[MAX_SIZE/5];
	int top_stack;
}stack;

extern stack sta;

typedef struct QueueEle1{
	int index;
	int layer;
}QueueEle1;

typedef struct tuple_array
{
	int from;
	int to;
    int fromr;
    int tor;
}tuple_array;

typedef struct stack_tag
{
    int element[MAX_LINE];   //1--IO 2--IA 3--QST
    tuple_array tuple_arrs[MAX_LINE];
	int top_stack_tag;
    int supplement[3];
}stack_tag;
extern stack_tag stack_tags[MAX_THREAD];

/* data structure for the possible status of XML tags*/
#define MAX_CHA 50
typedef struct taginfo{
	char tagname[MAX_CHA];
	int start[MAX_SIZE/5];
	int end[MAX_SIZE/5];
	int top_point;
}taginfo;

extern taginfo tags[MAX_SIZE];
extern int top_tags;

typedef struct tagstack{
	int tag_index;
	int child_point;
}tagstack;

extern tagstack tstack[MAX_SIZE/5];
extern int top_tstack;

/* required parameters*/
extern int choose;
extern int num_threads;
extern char* file_name;
extern char* xmlPath;
extern char* jsonPath;
extern int warmup_flag;
extern int pversion;
extern int statistic_flag;
extern int debug_flag;
extern int percentage;
extern int runtime;

#endif // !__GLOBAL_H__
