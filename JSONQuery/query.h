#include "lexer.h"

int streaming_automata(xml_Text *pText, xml_Token *pToken);  //parse and query data in an xmlText, return value:0--success -1--error
int execute_query(); //execute JSONPath query
int writing_results(); //write results into file
void print_debug_info(); //print out more information on tracking the performance
