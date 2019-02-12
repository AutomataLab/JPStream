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

LocationPath: '$' { $$ = jsonPathNodeCreateRoot(); }
            | LocationPath '.' Property { $$ = jsonPathNodeCreateConcat($1, $3); }
            | LocationPath PARENT Property  { $$ = jsonPathNodeCreateParentConcat($1, $3); }
            | LocationPath '[' Predicate ']' { $$ = jsonPathNodeCreatePredicate($1, $3); }
            ;

Property: NCNAME {$$ = jsonPathNodeCreateID($1); }
    | '*' { $$ = jsonPathNodeCreateWildcard(); }
    ;

Predicate: Number ':' Number {$$ = jsonPathNodeCreateRange($1, $3);}
    | ':' Number {$$ = jsonPathNodeCreateRange(NULL, $2);}
    | Number ':' {$$ = jsonPathNodeCreateRange($1, NULL);}
    | '*' { $$ = jsonPathNodeCreateWildcard(); }
    | '?' '(' Expr ')' {$$ = jsonPathNodeCreateFliter($3);}
    | '(' Expr ')' {$$ = jsonPathNodeCreateScript($2);}
    | IndexList
    ;

IndexList: Number
    | STRING  { $$ = jsonPathNodeCreateString($1); }
    | IndexList ',' Number  { $$ = jsonPathNodeCreateConcat($1, $3); }
    | IndexList ',' STRING  { $$ = jsonPathNodeCreateConcat($1, jsonPathNodeCreateString($3)); }
    ;

Expr: Expr OR Expr  { $$ = jsonPathNodeCreateOperator(jot_or, $1, $3); }
    | Expr AND Expr  { $$ = jsonPathNodeCreateOperator(jot_and, $1, $3); }
    | Expr EQ Expr  { $$ = jsonPathNodeCreateOperator(jot_equal, $1, $3); }
    | Expr NEQ Expr  { $$ = jsonPathNodeCreateOperator(jot_neq, $1, $3); }
    | Expr '<' Expr  { $$ = jsonPathNodeCreateOperator(jot_less, $1, $3); }
    | Expr '>' Expr  { $$ = jsonPathNodeCreateOperator(jot_greater, $1, $3); }
    | Expr LEQ Expr  { $$ = jsonPathNodeCreateOperator(jot_leq, $1, $3); }
    | Expr GEQ Expr  { $$ = jsonPathNodeCreateOperator(jot_geq, $1, $3); }
    | Expr '+' Expr  { $$ = jsonPathNodeCreateOperator(jot_add, $1, $3); }
    | Expr '-' Expr  { $$ = jsonPathNodeCreateOperator(jot_minus, $1, $3); }
    | Expr '*' Expr  { $$ = jsonPathNodeCreateOperator(jot_multiply, $1, $3); }
    | Expr '/' Expr  { $$ = jsonPathNodeCreateOperator(jot_div, $1, $3); }
    | Expr '%' Expr  { $$ = jsonPathNodeCreateOperator(jot_mod, $1, $3); }
    | '@' { $$ = jsonPathNodeCreateRef(); }
    | Number
    | STRING { $$ = jsonPathNodeCreateString($1); }
    | '(' Expr ')' { $$ = $2; }
    | Expr '.' Property { $$ = jsonPathNodeCreateConcat($1, $3); }
    | Expr PARENT Property  { $$ = jsonPathNodeCreateParentConcat($1, $3); }
    ;

Number: NUMBER { $$ = jsonPathNodeCreateNumber($1); }
    | '-' NUMBER { $$ = jsonPathNodeCreateNumber(-$2); }
    ;

%%

void jerror (JLTYPE * yylloc, yyscan_t locp, JSONPathNode **root, const char *msg) {
	fprintf(stderr, "error> %s\n", msg);
	// TODO: add line number and detail
    fprintf(stderr, "pos: %d\n", yylloc->first_column);
    fprintf(stderr, "end: %d\n", yylloc->last_column);
	exit(1);
}


JSONPathNode* analysisJSONPath(const char* data) {
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
