%{

#include <string.h>
#include "parser_j.h"

#define SAVE_TOKEN yylval->string = maketoken(yytext, yyleng)
#define SAVE_STRING yylval->string = makestring(yytext, yyleng)
#define SAVE_DOUBLE {char* b = maketoken(yytext, yyleng); yylval->number = atof(b); free(b); }
static char* maketoken(const char* data, int len);
static char* makestring(const char* data, int len);

%}

/* show the line number */
%option yylineno
%option reentrant noyywrap
%option prefix="j"


NCNameStartChar [A-Z]|"_"|[a-z]
NCNameChar	    {NCNameStartChar}|[0-9]
NCName			{NCNameStartChar}({NCNameChar})*

Digits			[0-9]+

%%

[ \t\v\f]               ; /* ignore blank token */

"*"						return '*';

"-"						return '-';

"+"						return '+';

"/"						return '/';

"%"						return '%';

"?"						return '?';

"["						return '[';

"]"						return ']';

"("						return '(';

")"						return ')';

"=~"					return RE;

"=="					return EQ;

"!="					return NEQ;

"<"						return '<';

">"						return '>';

"<="					return LEQ;

">="					return GEQ;

"&&"					return AND;

"||"					return OR;

"$"						return '$';

"@"						return '@';

"."						return '.';

","						return ',';

".."					return PARENT;

":"						return ':';

\"(\\.|[^\"\\])*\"		SAVE_STRING; return STRING;

\'(\\.|[^\'\\])*\'		SAVE_STRING; return STRING;

{Digits}("."{Digits})? 	SAVE_DOUBLE; return NUMBER;

{NCName}				SAVE_TOKEN; return NCNAME;

.						printf("tokenizer error\n");

%%


static char* maketoken(const char* data, int len) {
	char* str = (char*) malloc(len+1); 
	strncpy(str, data, len);
	str[len] = 0;
	return str;
}

static char* makestring(const char* data, int len) {
	char* str = (char*) malloc(len-1); 
	strncpy(str, data+1, len-2);
	str[len-2] = 0;
	return str;
}
