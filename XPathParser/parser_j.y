/* parser.y */
%code requires {
#include "xpath_model.h"
#undef YY_DECL
#define YY_DECL int jlex (JSTYPE* yylval, JLTYPE * yylloc, yyscan_t yyscanner)
#ifndef FLEX_SCANNER 
#include "scanner_j.h"
#endif 
}

%code {
#include "parser_j.h"
#include "scanner_j.h"
}


%define api.pure full
%define api.prefix {j}
%lex-param {yyscan_t yyscanner}
%locations 
%parse-param {yyscan_t yyscanner}{XPathNode **root}

%define parse.error verbose

%code provides{
YY_DECL;
void jerror (JLTYPE * yylloc, yyscan_t locp, XPathNode **root, const char *msg);
}

%union {
    double number;
	char *string;
    XPathNode *node;
}

%token <string> STRING NCNAME
%token <number> NUMBER
%token NEQ LEQ GEQ OR AND PARENT RE EQ
%type <node> AllPath LocationPath Number Property IndexList Predicate Expr  
%start AllPath

%left OR AND
%left NEQ LEQ GEQ EQ '<' '>'
%left '+' '-'
%left '*' '/' 
%left '(' '[' ')' ']'

%%

AllPath: LocationPath { *root = $1; }
       ;

LocationPath: '$' { $$ = xpn_CreateRoot(); }
            | LocationPath '.' Property { $$ = xpn_CreateConcat($1, $3); }
            | LocationPath PARENT Property  { $$ = xpn_CreateParentConcat($1, $3); }
            | LocationPath '[' Predicate ']' { $$ = xpn_CreatePredicate($1, $3); }
            ;

Property: NCNAME {$$ = xpn_CreateID($1); }
    | '*' { $$ = xpn_CreateWildcard(); }
    ;

Predicate: Number ':' Number {$$ = xpn_CreateRange($1, $3);}
    | ':' Number {$$ = xpn_CreateRange(NULL, $2);}
    | Number ':' {$$ = xpn_CreateRange($1, NULL);}
    | '*' { $$ = xpn_CreateWildcard(); }
    | '?' '(' Expr ')' {$$ = xpn_CreateFliter($3);}
    | '(' Expr ')' {$$ = xpn_CreateScript($2);}
    | IndexList
    ;

IndexList: Number
    | STRING  { $$ = xpn_CreateString($1); }
    | IndexList Number  { $$ = xpn_CreateConcat($1, $2); }
    | IndexList STRING  { $$ = xpn_CreateConcat($1, xpn_CreateString($2)); }
    ;

Expr: Expr OR Expr  { $$ = xpn_CreateOperator(xot_or, $1, $3); }
    | Expr AND Expr  { $$ = xpn_CreateOperator(xot_and, $1, $3); }
    | Expr EQ Expr  { $$ = xpn_CreateOperator(xot_equal, $1, $3); }
    | Expr NEQ Expr  { $$ = xpn_CreateOperator(xot_neq, $1, $3); }
    | Expr '<' Expr  { $$ = xpn_CreateOperator(xot_less, $1, $3); }
    | Expr '>' Expr  { $$ = xpn_CreateOperator(xot_greater, $1, $3); }
    | Expr LEQ Expr  { $$ = xpn_CreateOperator(xot_leq, $1, $3); }
    | Expr GEQ Expr  { $$ = xpn_CreateOperator(xot_geq, $1, $3); }
    | Expr '+' Expr  { $$ = xpn_CreateOperator(xot_add, $1, $3); }
    | Expr '-' Expr  { $$ = xpn_CreateOperator(xot_minus, $1, $3); }
    | Expr '*' Expr  { $$ = xpn_CreateOperator(xot_multiply, $1, $3); }
    | Expr '/' Expr  { $$ = xpn_CreateOperator(xot_div, $1, $3); }
    | Expr '%' Expr  { $$ = xpn_CreateOperator(xot_mod, $1, $3); }
    | '@' { $$ = xpn_CreateRef(); }
    | Number
    | STRING { $$ = xpn_CreateString($1); }
    ;

Number: NUMBER { $$ = xpn_CreateNumber($1); }
    | '-' NUMBER { $$ = xpn_CreateNumber(-$2); }
    ;


%%

void jerror (JLTYPE * yylloc, yyscan_t locp, XPathNode **root, const char *msg) {
	fprintf(stderr, "error> %s\n", msg);
	// TODO: add line number and detail
    fprintf(stderr, "pos: %d\n", yylloc->first_column);
    fprintf(stderr, "end: %d\n", yylloc->last_column);
	exit(1);
}