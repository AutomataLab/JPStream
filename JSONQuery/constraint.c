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
#include "constraint.h"
#include "global.h"
//#include "lexer.h"
#include "query.h"
#include "file_operation.h"
//#include "semi_structure.h"

/********************************************************
Function: void data_constraint_learning(char* file_name);
Description: learn data constraints from input file
Input: file_name -the name of the json file 
*********************************************************/
void data_constraint_learning(char* file_name)
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
