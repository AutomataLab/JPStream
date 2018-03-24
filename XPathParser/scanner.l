%{

#include <string.h>
#include "parser.hpp"

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

NameStartChar 	":"|[A-Z]|"_"|[a-z]|[#xC0-#xD6]|[#xD8-#xF6]|[#xF8-#x2FF]|[#x370-#x37D]|[#x37F-#x1FFF]|[#x200C-#x200D]|[#x2070-#x218F]|[#x2C00-#x2FEF]|[#x3001-#xD7FF]|[#xF900-#xFDCF]|[#xFDF0-#xFFFD]|[#x10000-#xEFFFF]
NameChar	    NameStartChar|"-"|"."|[0-9]|#xB7|[#x0300-#x036F]|[#x203F-#x2040]
Name			NameStartChar(NameChar)*

NCNameStartChar [A-Z]|"_"|[a-z]|[#xC0-#xD6]|[#xD8-#xF6]|[#xF8-#x2FF]|[#x370-#x37D]|[#x37F-#x1FFF]|[#x200C-#x200D]|[#x2070-#x218F]|[#x2C00-#x2FEF]|[#x3001-#xD7FF]|[#xF900-#xFDCF]|[#xFDF0-#xFFFD]|[#x10000-#xEFFFF]
NCNameChar	    NCNameStartChar|"-"|"."|[0-9]|#xB7|[#x0300-#x036F]|[#x203F-#x2040]
NCName			NCNameStartChar(NCNameChar)*



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

\"(\\.|[^\"\\])*\"		SAVE_STRING; return STRING;





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
