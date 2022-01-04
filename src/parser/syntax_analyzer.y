%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "syntax_tree.h"

// external functions from lex
extern int yylex();
extern int yyparse();
extern int yyrestart();
extern FILE * yyin;

// external variables from lexical_analyzer module
extern int lines;
extern char * yytext;
extern int pos_end;
extern int pos_start;

// Global syntax tree
syntax_tree *gt;

// Error reporting
void yyerror(const char *s);

// Helper functions written for you with love
syntax_tree_node *node(const char *node_name, int children_num, ...);
%}

/* TODO: Complete this definition. */
%union {
     struct _syntax_tree_node * node;
     char * name;
}

/* TODO: Your tokens here. */
%token <node> ERROR
%token <node> ADD
%token <node> SUB
%token <node> MUL
%token <node> DIV
%token <node> ADDROF
%token <node> MEMACC
%token <node> LT
%token <node> LTE
%token <node> GT
%token <node> GTE
%token <node> EQ
%token <node> NEQ
%token <node> ASSIGN
%token <node> SEMICOLON
%token <node> COMMA
%token <node> LPARENTHESE
%token <node> RPARENTHESE
%token <node> LBRACKET
%token <node> RBRACKET
%token <node> LBRACE
%token <node> RBRACE
%token <node> ELSE
%token <node> IF
%token <node> INT
%token <node> RETURN
%token <node> VOID
%token <node> WHILE
%token <node> STRUCT
%token <node> AUTO
%token <node> IDENTIFIER
%token <node> INTEGER
%token <node> FLOAT
%token <node> FLOATPOINT
%token <node> STRING
%token <node> OPERATOR
%token <node> REINTERPRETCAST
%token <node> TYPENAME
%token <node> TEMPLATE
//%token <node> EOL
//%token <node> BLANK
//%token <node> COMMENT
%type <node> template-param-list template-arg-list class-template-declaration program definition-list definition definitions var-reference var-decl-atom var-decl-element var-decl-expression params param-list var-declaration type-specifier scalar-type-specifier struct-definition fun-definition compound-stmt statements statement-list statement var-definition expression-stmt selection-stmt iteration-stmt return-stmt atom reinterpret-cast element factor multiplicative-expression additive-expression relational-expression expression relop addop mulop integer float string args arg-list 

/* compulsory starting symbol */
%start program

%%

/**
 * Program Structure
 */

program : 	definition-list {$$ = node( "program", 1, $1); gt->root = $$;}
		;

definition-list 	: 	definition-list definition {$$ = node( "definition-list", 2, $1, $2);}
					|	definition {$$ = node( "definition-list", 1, $1);}
					;

definition  : 	var-definition {$$ = node( "definition", 1, $1);}
			| 	fun-definition {$$ = node( "definition", 1, $1);}
			|   class-template-declaration {$$ = node( "definition", 1, $1);}
			;

definitions : definition-list {$$ = node( "definitions", 1, $1);}
            | %empty {$$ = node("definitions", 0);}

/**
 * Definitions and declarations
 */

var-reference   :   IDENTIFIER {$$ = node( "var-reference", 1, $1);}
                |   ADDROF IDENTIFIER {$$ = node( "var-reference", 2, $1, $2);}
				|   ADDROF {$$ = node( "var-reference", 1, $1);}
				|   OPERATOR addop {$$ = node( "var-reference", 2, $1, $2);}
				|   OPERATOR mulop {$$ = node( "var-reference", 2, $1, $2);}
				|   %empty {$$ = node("var-reference", 0);}
				;

var-decl-atom	:   var-reference {$$ = node( "var-decl-atom", 1, $1);}
                |   LPARENTHESE var-decl-expression RPARENTHESE {$$ = node( "var-decl-atom", 3, $1, $2, $3);}
				;

var-decl-element:   var-decl-element LPARENTHESE params RPARENTHESE {$$ = node( "var-decl-element", 4, $1, $2, $3, $4);}
                |   var-decl-element LBRACKET integer RBRACKET {$$ = node( "var-decl-element", 4, $1, $2, $3, $4);}
				|   var-decl-element LBRACKET RBRACKET {$$ = node( "var-decl-element", 3, $1, $2, $3);}
				|   var-decl-atom {$$ = node( "var-decl-element", 1, $1);}
				;

var-decl-expression :   MUL var-decl-expression {$$ = node( "var-decl-expression", 2, $1, $2);}
                |   var-decl-element {$$ = node( "var-decl-expression", 1, $1);}
				;

params 	: 	param-list {$$ = node( "params", 1, $1);}
		|	VOID {$$ = node( "params", 1, $1);}
		|   %empty {$$ = node("params", 0);}
		;

param-list 	: 	param-list COMMA var-declaration {$$ = node( "param-list", 3, $1, $2, $3);}
			| 	var-declaration {$$ = node( "param-list", 1, $1);}
			;

var-declaration : type-specifier var-decl-expression {$$ = node( "var-declaration", 2, $1, $2);}
                ;

type-specifier  :  scalar-type-specifier {$$ = node( "type-specifier", 1, $1);}
                |  struct-definition {$$ = node( "type-specifier", 1, $1);}
				|  STRUCT IDENTIFIER {$$ = node( "type-specifier", 2, $1, $2);}
				|  STRUCT IDENTIFIER LT template-arg-list GT {$$ = node( "type-specifier", 5, $1, $2, $3, $4, $5);}
				|  TYPENAME IDENTIFIER {$$ = node( "type-specifier", 2, $1, $2);}
				;

template-arg-list : template-arg-list COMMA type-specifier {$$ = node( "template-arg-list", 3, $1, $2, $3);}
                    | type-specifier {$$ = node( "template-arg-list", 1, $1);}
                    ;


scalar-type-specifier 	: 	INT {$$ = node( "scalar-type-specifier", 1, $1);}
				        | 	FLOAT { $$ = node( "scalar-type-specifier", 1, $1); }
				        | 	VOID {$$ = node( "scalar-type-specifier", 1, $1);}
						|   AUTO {$$ = node( "scalar-type-specifier", 1, $1);}
				        ;

struct-definition : STRUCT IDENTIFIER LBRACE definitions RBRACE {$$ = node( "struct-definition", 5, $1, $2, $3, $4, $5);}
                  | STRUCT LBRACE definitions RBRACE {$$ = node( "struct-definition", 4, $1, $2, $3, $4);}
				  ;

var-definition : var-declaration SEMICOLON {$$ = node( "var-definition", 2, $1, $2);}
               | var-declaration ASSIGN expression SEMICOLON  {$$ = node( "var-definition", 4, $1, $2, $3, $4);}
			   ;

fun-definition : var-declaration compound-stmt {$$ = node( "fun-definition", 2, $1, $2);}
			   ;

class-template-declaration : TEMPLATE LT template-param-list GT struct-definition SEMICOLON {$$ = node( "class-template-declaration", 6, $1, $2, $3, $4, $5, $6);}
                           ;

template-param-list : template-param-list COMMA TYPENAME IDENTIFIER {$$ = node( "template-param-list", 4, $1, $2, $3, $4);}
                    | TYPENAME IDENTIFIER {$$ = node( "template-param-list", 2, $1, $2);}
                    ;

/**
 * Statements
 */
compound-stmt 	: 	LBRACE statements RBRACE {$$ = node( "compound-stmt", 3, $1, $2, $3);}
				;

statements      :   statement-list {$$ = node( "statements", 1, $1);}
                |   %empty {$$ = node("statements", 0);}
				;

statement-list  :   statement-list statement {$$ = node( "statement-list", 2, $1, $2);}
                |   statement                {$$ = node( "statement-list", 1, $1);}
				;

statement 	: 	expression-stmt {$$ = node( "statement", 1, $1);}
            | 	compound-stmt {$$ = node( "statement", 1, $1);}
			| 	selection-stmt {$$ = node( "statement", 1, $1);}
			| 	iteration-stmt {$$ = node( "statement", 1, $1);}
			| 	return-stmt {$$ = node( "statement", 1, $1);}
			|   var-definition {$$ = node( "statement", 1, $1);}
			;


expression-stmt : 	expression SEMICOLON {$$ = node( "expression-stmt", 2, $1, $2);}
				| 	SEMICOLON {$$ = node( "expression-stmt", 1, $1);}
				;

selection-stmt 	: 	IF LPARENTHESE expression RPARENTHESE statement {$$ = node( "selection-stmt", 5, $1, $2, $3, $4, $5);}
				| 	IF LPARENTHESE expression RPARENTHESE statement ELSE statement {$$ = node( "selection-stmt", 7, $1, $2, $3, $4, $5, $6, $7);}
				;

iteration-stmt 	: 	WHILE LPARENTHESE expression RPARENTHESE statement {$$ = node( "iteration-stmt", 5, $1, $2, $3, $4, $5);}
				;

return-stmt : 	RETURN SEMICOLON {$$ = node( "return-stmt", 1, $1);}
			| 	RETURN expression SEMICOLON {$$ = node( "return-stmt", 3, $1, $2, $3);}
			;


/**
 * Expressions
 */

atom        :  IDENTIFIER {$$ = node( "atom", 1, $1);}
            |  integer {$$ = node( "atom", 1, $1);}
			|  float {$$ = node( "atom", 1, $1);}
			|  string {$$ = node( "atom", 1, $1);}
			|  LPARENTHESE expression RPARENTHESE {$$ = node( "atom", 3, $1, $2, $3);}
			|  reinterpret-cast {$$ = node( "atom", 1, $1);}
			;

reinterpret-cast : REINTERPRETCAST LT var-declaration GT LPARENTHESE expression RPARENTHESE {$$ = node( "reinterpret-cast", 7, $1, $2, $3, $4, $5, $6, $7);}
                 ;

element     :  element LPARENTHESE args RPARENTHESE{$$ = node( "element", 4, $1, $2, $3, $4);}
            |  element LBRACKET expression RBRACKET{$$ = node( "element", 4, $1, $2, $3, $4);}
			|  element MEMACC IDENTIFIER{$$ = node( "element", 3, $1, $2, $3);}
			|  atom{$$ = node( "element", 1, $1);}
			;

factor 	: 	addop factor{$$ = node( "factor", 2, $1, $2);}
        |   MUL factor{$$ = node( "factor", 2, $1, $2);}
		|   ADDROF factor{$$ = node( "factor", 2, $1, $2);}
		|   element{$$ = node( "factor", 1, $1);}
		;

multiplicative-expression 	: 	multiplicative-expression mulop factor {$$ = node( "multiplicative-expression", 3, $1, $2, $3);}
		| 	factor {$$ = node( "multiplicative-expression", 1, $1);}
		;

additive-expression : 	additive-expression addop multiplicative-expression  {$$ = node( "additive-expression", 3, $1, $2, $3);}
					| 	multiplicative-expression {$$ = node( "additive-expression", 1, $1);}
					;
				

relational-expression 	: 	additive-expression relop additive-expression {$$ = node( "relational-expression", 3, $1, $2, $3);}
					| 	additive-expression {$$ = node( "relational-expression", 1, $1);}
					;

expression 	: 	relational-expression ASSIGN expression {$$ = node( "expression", 3, $1, $2, $3);}
			| 	relational-expression {$$ = node( "expression", 1, $1);}
			;
 // ==================================================

relop 	: 	LT {$$ = node( "relop", 1, $1);}
		| 	LTE {$$ = node( "relop", 1, $1);}
		| 	GT {$$ = node( "relop", 1, $1);}
		| 	GTE {$$ = node( "relop", 1, $1);}
		| 	EQ {$$ = node( "relop", 1, $1);}
		| 	NEQ {$$ = node( "relop", 1, $1);}
		;


addop 	: 	ADD {$$ = node( "addop", 1, $1);}
		|	SUB {$$ = node( "addop", 1, $1);}
		;


mulop 	: 	MUL {$$ = node( "mulop", 1, $1);}
		|	DIV {$$ = node( "mulop", 1, $1);}
		;

integer 	: 	INTEGER {$$ = node( "integer", 1, $1);}
		;

float 	: 	FLOATPOINT {$$ = node( "float", 1, $1);}
		;

string  :   STRING  {$$ = node( "string", 1, $1);}

args 	: 	arg-list {$$ = node( "args", 1, $1);}
        |   %empty {$$ = node("args", 0);}
		;

arg-list 	: 	arg-list COMMA expression {$$ = node( "arg-list", 3, $1, $2, $3);}
			| 	expression {$$ = node( "arg-list", 1, $1);}
			;

%%

/// The error reporting function.
void yyerror(const char * s)
{
    // TO STUDENTS: This is just an example.
    // You can customize it as you like.
    fprintf(stderr, "error at line %d column %d: %s\n", lines, pos_start, s);
}

/// Parse input from file `input_path`, and prints the parsing results
/// to stdout.  If input_path is NULL, read from stdin.
///
/// This function initializes essential states before running yyparse().
syntax_tree *parse(const char *input_path)
{
    if (input_path != NULL) {
        if (!(yyin = fopen(input_path, "r"))) {
            fprintf(stderr, "[ERR] Open input file %s failed.\n", input_path);
            exit(1);
        }
    } else {
        yyin = stdin;
    }

    lines = pos_start = pos_end = 1;
    gt = new_syntax_tree();
    yyrestart(yyin);
    yyparse();
    return gt;
}

/// A helper function to quickly construct a tree node.
///
/// e.g. $$ = node("program", 1, $1);
syntax_tree_node *node(const char *name, int children_num, ...)
{
    syntax_tree_node *p = new_syntax_tree_node(name);
    syntax_tree_node *child;
    if (children_num == 0) {
		// Pass
        /* child = new_syntax_tree_node("epsilon");
        syntax_tree_add_child(p, child); */
    } else {
        va_list ap;
        va_start(ap, children_num);
        for (int i = 0; i < children_num; ++i) {
            child = va_arg(ap, syntax_tree_node *);
            syntax_tree_add_child(p, child);
        }
        va_end(ap);
    }
    return p;
}
