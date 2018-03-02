%{

#include <cstring>
#include "r_parser.hpp"

#define SAVE_TOKEN yylval->str = maketoken(yytext, yyleng)
#define SAVE_STRING yylval->str = makestring(yytext, yyleng)

static char* maketoken(const char* data, int len);
static char* makestring(const char* data, int len);

%}

/* show the line number */
%option yylineno
%option reentrant noyywrap
%option never-interactive
%option prefix="zz"

%%

\\.   					{ if (yytext[1] == 'S' || yytext[1] == 's' || yytext[1] == 'D' || 
							yytext[1] == 'd' || yytext[1] == 'W' || yytext[1] == 'w' ) 
							{ SAVE_TOKEN; return R_PRESET; } else { SAVE_TOKEN; return R_CHAR; }
						}
[ \t\v\f]               ; /* ignore blank token */

"*"						return '*';

"?"						return '?';

"+"						return '+';

"|"                     return '|';

"("						return '(';

")"						return ')';

\"(\\.|[^\"\\])*\"		SAVE_STRING; return R_STRING;

\[(\\.|[^\]\\])*\]		SAVE_STRING; return R_SET;

\{(\\.|[^\}\\])*\}		SAVE_STRING; return R_ID;

.     					SAVE_TOKEN; return R_CHAR;

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
