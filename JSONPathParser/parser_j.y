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
%parse-param {yyscan_t yyscanner}{ASTNode **root}

%define parse.error verbose

%code provides{
YY_DECL;
void jerror (JLTYPE * yylloc, yyscan_t locp, ASTNode **root, const char *msg);
}

%union {
    double number;
	char *string;
    ASTNode *node;
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
%right '!'
%left '('

%%

AllPath: LocationPath { *root = $1; }
       ;

LocationPath: '$' { $$ = ASTNodeCreateRoot(); }
            | LocationPath '.' Property { $$ = ASTNodeCreateConcat($1, $3); }
            | LocationPath PARENT Property  { $$ = ASTNodeCreateParentConcat($1, $3); }
            | LocationPath '[' Predicate ']' { $$ = ASTNodeCreatePredicate($1, $3); }
            ;

Property: NCNAME {$$ = ASTNodeCreateID($1); }
    | '*' { $$ = ASTNodeCreateWildcard(); }
    ;

Predicate: Number ':' Number {$$ = ASTNodeCreateRange($1, $3);}
    | ':' Number {$$ = ASTNodeCreateRange(NULL, $2);}
    | Number ':' {$$ = ASTNodeCreateRange($1, NULL);}
    | '*' { $$ = ASTNodeCreateWildcard(); }
    | '?' '(' Expr ')' {$$ = ASTNodeCreateFliter($3);}
    | '(' Expr ')' {$$ = ASTNodeCreateScript($2);}
    | IndexList
    ;

IndexList: Number
    | STRING  { $$ = ASTNodeCreateString($1); }
    | IndexList ',' Number  { $$ = ASTNodeCreateConcat($1, $3); }
    | IndexList ',' STRING  { $$ = ASTNodeCreateConcat($1, ASTNodeCreateString($3)); }
    ;

Expr: Expr OR Expr  { $$ = ASTNodeCreateOperator(jot_or, $1, $3); }
    | Expr AND Expr  { $$ = ASTNodeCreateOperator(jot_and, $1, $3); }
    | Expr EQ Expr  { $$ = ASTNodeCreateOperator(jot_equal, $1, $3); }
    | Expr NEQ Expr  { $$ = ASTNodeCreateOperator(jot_neq, $1, $3); }
    | Expr '<' Expr  { $$ = ASTNodeCreateOperator(jot_less, $1, $3); }
    | Expr '>' Expr  { $$ = ASTNodeCreateOperator(jot_greater, $1, $3); }
    | Expr LEQ Expr  { $$ = ASTNodeCreateOperator(jot_leq, $1, $3); }
    | Expr GEQ Expr  { $$ = ASTNodeCreateOperator(jot_geq, $1, $3); }
    | Expr '+' Expr  { $$ = ASTNodeCreateOperator(jot_add, $1, $3); }
    | Expr '-' Expr  { $$ = ASTNodeCreateOperator(jot_minus, $1, $3); }
    | Expr '*' Expr  { $$ = ASTNodeCreateOperator(jot_multiply, $1, $3); }
    | Expr '/' Expr  { $$ = ASTNodeCreateOperator(jot_div, $1, $3); }
    | Expr '%' Expr  { $$ = ASTNodeCreateOperator(jot_mod, $1, $3); }
    | '!' Expr { $$ = ASTNodeCreateOperatorOne(jot_not, $2); }
    | '@' { $$ = ASTNodeCreateRef(); }
    | Number
    | STRING { $$ = ASTNodeCreateString($1); }
    | '(' Expr ')' { $$ = $2; }
    | Expr '.' Property { $$ = ASTNodeCreateConcat($1, $3); }
    | Expr PARENT Property  { $$ = ASTNodeCreateParentConcat($1, $3); }
    ;

Number: NUMBER { $$ = ASTNodeCreateNumber($1); }
    | '-' NUMBER { $$ = ASTNodeCreateNumber(-$2); }
    ;

%%

void jerror (JLTYPE * yylloc, yyscan_t locp, ASTNode **root, const char *msg) {
	fprintf(stderr, "error> %s\n", msg);
	// TODO: add line number and detail
    fprintf(stderr, "pos: %d\n", yylloc->first_column);
    fprintf(stderr, "end: %d\n", yylloc->last_column);
	exit(1);
}


ASTNode* analysisJSONPath(const char* data) {
    ASTNode* root;
    yyscan_t sc;
    int res;
    jlex_init(&sc);
    YY_BUFFER_STATE buffer = j_scan_string(data, sc);
    res = jparse(sc, &root);
    j_delete_buffer(buffer, sc);
    jlex_destroy(sc);
    return root;
}
