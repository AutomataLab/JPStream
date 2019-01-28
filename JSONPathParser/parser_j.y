/* parser.y */
%code requires {
#include "jsonpath_parser.h"
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
%parse-param {yyscan_t yyscanner}{JSONPathNode **root}

%define parse.error verbose

%code provides{
YY_DECL;
void jerror (JLTYPE * yylloc, yyscan_t locp, JSONPathNode **root, const char *msg);
}

%union {
    double number;
	char *string;
    JSONPathNode *node;
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
%left '('

%%

AllPath: LocationPath { *root = $1; }
       ;

LocationPath: '$' { $$ = jpn_CreateRoot(); }
            | LocationPath '.' Property { $$ = jpn_CreateConcat($1, $3); }
            | LocationPath PARENT Property  { $$ = jpn_CreateParentConcat($1, $3); }
            | LocationPath '[' Predicate ']' { $$ = jpn_CreatePredicate($1, $3); }
            ;

Property: NCNAME {$$ = jpn_CreateID($1); }
    | '*' { $$ = jpn_CreateWildcard(); }
    ;

Predicate: Number ':' Number {$$ = jpn_CreateRange($1, $3);}
    | ':' Number {$$ = jpn_CreateRange(NULL, $2);}
    | Number ':' {$$ = jpn_CreateRange($1, NULL);}
    | '*' { $$ = jpn_CreateWildcard(); }
    | '?' '(' Expr ')' {$$ = jpn_CreateFliter($3);}
    | '(' Expr ')' {$$ = jpn_CreateScript($2);}
    | IndexList
    ;

IndexList: Number
    | STRING  { $$ = jpn_CreateString($1); }
    | IndexList ',' Number  { $$ = jpn_CreateConcat($1, $3); }
    | IndexList ',' STRING  { $$ = jpn_CreateConcat($1, jpn_CreateString($3)); }
    ;

Expr: Expr OR Expr  { $$ = jpn_CreateOperator(jot_or, $1, $3); }
    | Expr AND Expr  { $$ = jpn_CreateOperator(jot_and, $1, $3); }
    | Expr EQ Expr  { $$ = jpn_CreateOperator(jot_equal, $1, $3); }
    | Expr NEQ Expr  { $$ = jpn_CreateOperator(jot_neq, $1, $3); }
    | Expr '<' Expr  { $$ = jpn_CreateOperator(jot_less, $1, $3); }
    | Expr '>' Expr  { $$ = jpn_CreateOperator(jot_greater, $1, $3); }
    | Expr LEQ Expr  { $$ = jpn_CreateOperator(jot_leq, $1, $3); }
    | Expr GEQ Expr  { $$ = jpn_CreateOperator(jot_geq, $1, $3); }
    | Expr '+' Expr  { $$ = jpn_CreateOperator(jot_add, $1, $3); }
    | Expr '-' Expr  { $$ = jpn_CreateOperator(jot_minus, $1, $3); }
    | Expr '*' Expr  { $$ = jpn_CreateOperator(jot_multiply, $1, $3); }
    | Expr '/' Expr  { $$ = jpn_CreateOperator(jot_div, $1, $3); }
    | Expr '%' Expr  { $$ = jpn_CreateOperator(jot_mod, $1, $3); }
    | '@' { $$ = jpn_CreateRef(); }
    | Number
    | STRING { $$ = jpn_CreateString($1); }
    | '(' Expr ')' { $$ = $2; }
    | Expr '.' Property { $$ = jpn_CreateConcat($1, $3); }
    | Expr PARENT Property  { $$ = jpn_CreateParentConcat($1, $3); }
    ;

Number: NUMBER { $$ = jpn_CreateNumber($1); }
    | '-' NUMBER { $$ = jpn_CreateNumber(-$2); }
    ;

%%

void jerror (JLTYPE * yylloc, yyscan_t locp, JSONPathNode **root, const char *msg) {
	fprintf(stderr, "error> %s\n", msg);
	// TODO: add line number and detail
    fprintf(stderr, "pos: %d\n", yylloc->first_column);
    fprintf(stderr, "end: %d\n", yylloc->last_column);
	exit(1);
}


JSONPathNode* jpp_Analysis(const char* data) {
    JSONPathNode* root;
    yyscan_t sc;
    int res;
    jlex_init(&sc);
    YY_BUFFER_STATE buffer = j_scan_string(data, sc);
    res = jparse(sc, &root);
    j_delete_buffer(buffer, sc);
    jlex_destroy(sc);
    return root;
}
