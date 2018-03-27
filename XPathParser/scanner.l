%{

#include <string.h>
#include "parser.h"

#define SAVE_TOKEN yylval->str = maketoken(yytext, yyleng)
#define SAVE_STRING yylval->str = makestring(yytext, yyleng)

static char* maketoken(const char* data, int len);
static char* makestring(const char* data, int len);

%}

/* show the line number */
%option yylineno
%option reentrant noyywrap
%option never-interactive
%option prefix="xx"

NameStartChar 	":"|[A-Z]|"_"|[a-z]
NameChar	    {NameStartChar}|"-"|"."|[0-9]
Name			{NameStartChar}({NameChar})*

NCNameStartChar [A-Z]|"_"|[a-z]
NCNameChar	    {NCNameStartChar}|"-"|"."|[0-9]
NCName			{NCNameStartChar}({NCNameChar})*

QName 			{NCName}":"{NCName}|{NCName}

Digits			[0-9]+

%%

[ \t\v\f]               ; /* ignore blank token */

"*"						return '*';

"+"						return '+';

"-"						return '-';

"/"						return '/';

"//"					return DESC;

"|"                     return '|';

"["						return '[';

"]"						return ']';

"("						return '(';

")"						return ')';

"="						return '=';

"!="					return NEQ;

"<"						return '<';

">"						return '>';

"<="					return LEQ;

">="					return GEQ;

"or"					return OR;

"and"					return AND;

"mod"					return MOD;

"div"					return DIV;

"$"						return '$';

"@"						return '@';

"."						return CURRENT;

".."					return PARENT;

":"						return ':';

"::"					return AXES;

\"(\\.|[^\"\\])*\"		SAVE_STRING; return STRING;

\'(\\.|[^\'\\])*\'		SAVE_STRING; return STRING;

{Digits}("."{Digits})?|"."{Digits} 		SAVE_TOKEN; return NUMBER;

{NCName}				SAVE_TOKEN; return NCNAME;



%%


static char* maketoken(const char* data, int len) {
	char* str = new char[len+1];
	strncpy(str, data, len);
	str[len] = 0;
	return str;
}

static char* makestring(const char* data, int len) {
	char* str = new char[len-1];
	strncpy(str, data+1, len-2);
	str[len-2] = 0;
	return str;
}
