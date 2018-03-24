/* parser.y */
%code requires {
#include "xpath_model.h"
#undef YY_DECL
#define YY_DECL int xxlex (XXSTYPE* yylval, XXLTYPE * yylloc, yyscan_t yyscanner)
#ifndef FLEX_SCANNER 
#include "scanner.h"
#endif 
}

%code {
#include "parser.h"
#include "scanner.h"
}


%define api.pure full
%define api.prefix {xx}
%lex-param {yyscan_t yyscanner}
%locations 
%parse-param {yyscan_t yyscanner}{XPathNode **root}

%define parse.error verbose

%code provides{
YY_DECL;
void xxerror (XXLTYPE * yylloc, yyscan_t locp, XPathNode **root, const char *msg);
}

%union {
    double number;
	char *string;
}

%token <str> R_ID R_CHAR R_STRING R_SET R_PRESET
%type <item> item item_with_opt
%type <list> list regex
%start regex

%%

regex: list { $$ = new RegexList(true); $$->Add($1); parser->model = $$; }
     | regex '|' list { $$ = $1; $$->Add($3); }
     ;

list: item_with_opt { $$ = new RegexList(); $$->Add($1); }
    | list item_with_opt { $$ = $1; $$->Add($2); }
    ;

item_with_opt: item '?' { $$ = $1; $$->setOpt(dragontooth::RegexItem::RegexExternOpt::re_optional); }
             | item '*' { $$ = $1; $$->setOpt(dragontooth::RegexItem::RegexExternOpt::re_repetition); }
             | item '+' { $$ = $1; $$->setOpt(dragontooth::RegexItem::RegexExternOpt::re_nonzero_repetition); }
             | item { $$ = $1; }
             ;

item: R_SET { $$ = new RegexSet($1); }
    | R_PRESET { $$ = RegexSet::getPreset($1); }
    | R_STRING { $$ = new RegexString($1); }
    | R_CHAR { $$ = new RegexChar($1); }
    | R_ID { $$ = parser->mapper->Find($1); /* This is very special that need parser support holding other substring */ }
    | '(' regex ')' { $$ = $2; }
    ;

%%

void xxerror (XXLTYPE * yylloc, yyscan_t locp, XPathNode **root, const char *msg) {
	fprintf(stderr, "error> %s\n", msg);
	// TODO: add line number and detail
	exit(1);
}