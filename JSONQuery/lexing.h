#ifndef __LEXING_H__
#define __LEXING_H__

#include "json_stream.h"

#define LCB 1   //'{'
#define RCB 2   //'}'
#define LB 3    //'['
#define RB 4    //']'
#define COM 5   //'/'
#define KY 6    //key field
#define PRI 7   //primitive

#define MAX_SIZE_PRIMITIVE 4000

/*typedef struct JsonToken
{
    char *text;
    int len;
}JsonToken;*/

/* structure for tokens */
typedef struct Lexer{
    char* current_start;     //starting position of the current symbol
    char* next_start;        //starting position of the next symbol
    char* begin_stream;       //starting position of input json
    char* end_stream;         //ending position of input json
    int lex_state;       
    char* content;           //content of the current symbol
}Lexer; 

/*static inline void jsl_initToken(JsonToken *jToken, char *s)
{
    jToken->text = s;
    jToken->len = strlen(s);
}*/

static inline void jsl_LexerCtor(Lexer* lexer, JSONStream* json_stream)
{
    lexer->current_start = json_stream->input_stream[0]; 
    lexer->next_start = lexer->current_start;
    lexer->begin_stream = json_stream->input_stream[0];
    lexer->end_stream = json_stream->input_stream[0] + strlen(json_stream->input_stream[0]);
    lexer->lex_state = 0;
    lexer->content = (char*)malloc(MAX_SIZE_PRIMITIVE*sizeof(char));
    //(int*)malloc(sizeof(int));
    //*(lexer->lex_state)= 0;
}

static inline void jsl_LexerDtor(Lexer* lexer)
{
    if(lexer->content!=NULL) free(lexer->content);
}

static inline Lexer* jsl_createLexer(JSONStream* json_stream)
{
    Lexer* lexer = (Lexer*)malloc(sizeof(Lexer));
    jsl_LexerCtor(lexer, json_stream);
    return lexer;
}

static inline Lexer* jsl_freeLexer(Lexer* lexer)
{
    jsl_LexerDtor(lexer);
    free(lexer);
}

int jsl_next_token(Lexer* lexer); //getting the next symbol from JSON data
#endif // !__LEXING_H__