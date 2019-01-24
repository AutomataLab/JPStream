#ifndef __LEXER_H__
#define __LEXER_H__

//#include "basic.h"
//#include "global.h"
#include "semi_structure.h"

#define LCB 1
#define RCB 2
#define LB 3
#define RB 4
#define COM 5
#define KY 6
#define PRI 7

/* structure for tokens */
typedef struct token_info{
    char* start;
    char* head;
    char* current;
    char* end;
    int* lex_state;
}token_info; 

int lexer(xml_Text *pText, xml_Token *pToken, token_info* tInfo, char* start_text); //getting the next token from JSON data

#endif // !__LEXER_H__
