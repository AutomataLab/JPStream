#ifndef __QUERY_H__
#define __QUERY_H__

#include "lexer.h"

#include "dfa_builder.h"
#include "jsonpath_evaluator.h"

int streaming_automata(xml_Text *pText, xml_Token *pToken);  //parse and query data in an xmlText, return value:0--success -1--error
int execute_query(); //execute JSONPath query
int writing_results(); //write results into file
void print_debug_info(); //print out more information on tracking the performance

JQ_CONTEXT* automata(void);
void createTree1(int thread_num);
void initialization();

#endif // !__QUERY_H__
