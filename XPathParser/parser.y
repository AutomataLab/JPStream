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
    XPathNode *node;
}

%token <string> STRING NCNAME REFERENCE
%token <double> NUMBER
%token DESC NEQ LEQ GEQ OR AND MOD DIV PARENT AXES 
%type <node> LocationPath AbsoluteLocationPath RelativeLocationPath Predicates Step AxisSpecifier NodeTest Predicate Expr ExprList NameTest QName 
%start LocationPath

%%

LocationPath: RelativeLocationPath 
            | AbsoluteLocationPath
            ;

AbsoluteLocationPath: '/'
                    | '/' RelativeLocationPath
                    | DESC RelativeLocationPath
                    ;

RelativeLocationPath: Step
                    | RelativeLocationPath '/' Step	
                    | RelativeLocationPath DESC Step
                    ;

Predicates: Predicate
          | Predicates Predicate
          | %empty
          ;

Step: AxisSpecifier NodeTest Predicates	
    | '.'
    | PARENT
    ;

AxisSpecifier: NCNAME AXES 
             | '@'
             | %empty
             ;

NodeTest: NameTest
        | NCNAME '(' ')'
        | NCNAME '(' STRING ')'	
        ;
    
Predicate: '[' Expr ']'	
         ;

Expr: Expr OR Expr
    | Expr AND Expr
    | Expr '=' Expr
    | Expr NEQ Expr
    | Expr '<' Expr
    | Expr '>' Expr
    | Expr LEQ Expr
    | Expr GEQ Expr
    | Expr '+' Expr
    | Expr '-' Expr
    | Expr '*' Expr
    | Expr DIV Expr
    | Expr MOD Expr
    | '-' Expr
    | '(' Expr ')'
    | NUMBER
    | REFERENCE
    | STRING
    | NCNAME '(' ExprList ')'
    ;

ExprList: %empty
        | Expr
        | ExprList Expr
        ;

NameTest: '*'
        | NCNAME ':' '*'
        | QName
        ;

QName: NCNAME ':' NCNAME
     | NCNAME
     ;

%%

void xxerror (XXLTYPE * yylloc, yyscan_t locp, XPathNode **root, const char *msg) {
	fprintf(stderr, "error> %s\n", msg);
	// TODO: add line number and detail
	exit(1);
}