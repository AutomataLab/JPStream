#ifndef __LEXING_H__
#define __LEXING_H__

#include <malloc.h> 
#include <string.h>

#define LCB 1   //'{'
#define RCB 2   //'}'
#define LB 3    //'['
#define RB 4    //']'
#define COM 5   //'/'
#define KY 6    //key field
#define PRI 7   //primitive
#define END -1  //ends

#define MAX_SIZE_PRIMITIVE 4000

/* structure for tokens */
typedef struct Lexer{
    char* current_start;     //starting position of the current symbol
    char* next_start;        //starting position of the next symbol
    char* begin_stream;       //starting position of input json
    char* end_stream;         //ending position of input json
    int lex_state;       
    char* content;           //content of the current symbol
}Lexer; 


static inline void initLexer(Lexer* lexer, char* json_stream)
{
    lexer->current_start = json_stream; 
    lexer->next_start = lexer->current_start;
    lexer->begin_stream = json_stream;
    lexer->end_stream = json_stream + strlen(json_stream);
    lexer->lex_state = 0;
    lexer->content = (char*)malloc(MAX_SIZE_PRIMITIVE*sizeof(char));
}

static inline void destroyLexer(Lexer* lexer)
{
    if(lexer->content!=NULL) free(lexer->content);
}

static inline Lexer* createLexer(char* json_stream)
{
    Lexer* lexer = (Lexer*)malloc(sizeof(Lexer));
    initLexer(lexer, json_stream);
    return lexer;
}

static inline void freeLexer(Lexer* lexer)
{
    destroyLexer(lexer);
    free(lexer);
}

//getting the next symbol from JSON data
int nextToken(Lexer* lexer); 
#endif // !__LEXING_H__
