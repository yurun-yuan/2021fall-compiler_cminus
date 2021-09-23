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
extern FILE *yyin;

// external variables from lexical_analyzer module
extern int lines;
extern char *yytext;
extern int pos_end;
extern int pos_start;

// Global syntax tree
syntax_tree *gt;

// Error reporting
void yyerror(const char *s);

// Helper functions written for you with love
syntax_tree_node *node(const char *node_name, int children_num, ...);

struct _syntax_tree_node;
%}

/* TODO: Complete this definition.
   Hint: See pass_node(), node(), and syntax_tree.h.
         Use forward declaring. */
%union
{
    struct _syntax_tree_node *node;
}

/* TODO: Your tokens here. */
%token<node> INT
%token<node> IF
%token<node> ELSE
%token<node> RETURN
%token<node> VOID
%token<node> WHILE
%token<node> ADD
%token<node> MINUS
%token<node> MUL
%token<node> DIV
%token<node> LESS
%token<node> LESSEQ
%token<node> GREATER
%token<node> GREATEREQ
%token<node> EQ
%token<node> NEQ
%token<node> ASSIGN
%token<node> SEMICOLON
%token<node> COMMA
%token<node> LPAREN
%token<node> RPAREN
%token<node> LBRACKET
%token<node> RBRACKET
%token<node> LBRACE
%token<node> RBRACE
%token<node> COMMENT
%token<node> ID
%token<node> INTEGER
%token<node> FLOAT
%token<node> ERROR
%type<node> program

%start program
%type<node> declaration-list 
%type<node> declaration
%type<node> var-declaration
%type<node> type-specifier
%type<node> fun-declaration
%type<node> params
%type<node> param-list
%type<node> param
%type<node> compound-stmt
%type<node> local-declarations
%type<node> statement-list
%type<node> statement
%type<node> expression-stmt
%type<node> selection-stmt
%type<node> iteration-stmt
%type<node> return-stmt
%type<node> expression
%type<node> var
%type<node> simple-expression
%type<node> relop
%type<node> additive-expression
%type<node> addop
%type<node> term
%type<node> mulop
%type<node> factor
%type<node> integer
%type<node> float
%type<node> call
%type<node> args
%type<node> arg-list

%%
/* TODO: Your rules here. */

/* Example:
program: declaration-list {$$ = node( "program", 1, $1); gt->root = $$;}
       ;
*/

program : declaration-list
{
    $$ = node("program", 1, $1);
    gt->root = $$;
};
declaration-list : declaration-list declaration { $$ = node("declaration-list", 2, $1, $2); }
| declaration{$$ = node("declaration-list", 1, $1);}
;

declaration: var-declaration{$$ = node("declaration", 1, $1);}
| fun-declaration{$$ = node("declaration", 1, $1);}
;

var-declaration: type-specifier ID SEMICOLON {$$ = node("var-declaration", 3, $1, $2, $3);}
| type-specifier ID LBRACKET INTEGER RBRACKET SEMICOLON {$$ = node("var-declaration", 6, $1, $2, $3, $4, $5, $6);}
;

type-specifier: INT{$$=node("type-specifier", 1, $1);}
|FLOAT{$$=node("type-specifier", 1, $1);}
|VOID{$$=node("type-specifier", 1, $1);}
;

fun-declaration: type-specifier ID LPAREN params RPAREN compound-stmt{$$=node("fun-declaration", 6, $1, $2, $3, $4, $5, $6);}
;

params: param-list{$$=node("params", 1, $1);}
|VOID {$$=node("params", 1, $1);}
;

param-list: param-list COMMA param {$$ = node("param-list", 3, $1, $2, $3);}
| param {$$=node("param-list", 1, $1);}
;

param: type-specifier ID{$$=node("param", 2, $1, $2);}
| type-specifier ID LBRACKET RBRACKET {$$=node("param", 4, $1, $2, $3, $4);}
;

compound-stmt: LBRACE local-declarations statement-list RBRACE {$$=node("compound-stmt", 4, $1, $2, $3, $4);}
;

local-declarations: local-declarations var-declaration{$$=node("local-declarations", 2, $1, $2);}
| {$$=node("local-declarations", 0);}
;

statement-list: statement-list statement{$$=node("statement-list", 2, $1, $2);}
|  {$$=node("statement-list", 0);}
;

statement: expression-stmt {$$=node("statement", 1, $1);}
| compound-stmt {$$=node("statement", 1, $1);}
| selection-stmt {$$=node("statement", 1, $1);}
| iteration-stmt {$$=node("statement", 1, $1);}
| return-stmt {$$=node("statement", 1, $1);}
;

expression-stmt: expression SEMICOLON{$$ = node("expression-stmt", 2, $1, $2);}
| SEMICOLON{$$ = node("expression-stmt", 1, $1);}
;

selection-stmt: IF LPAREN expression RPAREN statement {$$=node("selection-stmt", 5, $1, $2, $3, $4, $5);}
| IF LPAREN expression RPAREN statement ELSE statement{$$=node("selection-stmt", 7, $1, $2, $3, $4, $5, $6, $7);}
;

iteration-stmt: WHILE LPAREN expression RPAREN statement {$$ = node("iteration-stmt", 5, $1, $2, $3, $4, $5);}
;

return-stmt: RETURN SEMICOLON{$$ = node("return-stmt", 2, $1, $2);}
| RETURN expression SEMICOLON{$$ = node("return-stmt", 3, $1, $2, $3);}
;

expression: var ASSIGN expression{$$ = node("expression", 3, $1, $2, $3);}
| simple-expression{$$ = node("expression", 1, $1);}
;

var: ID{$$ = node("var", 1, $1);}
|ID LBRACKET expression RBRACKET{$$ = node("var", 4, $1, $2, $3, $4);}
;

simple-expression: additive-expression relop additive-expression{$$=node("simple-expression", 3, $1, $2, $3);}
|additive-expression{$$=node("simple-expression", 1, $1);}
;

relop: LESSEQ{$$=node("relop", 1, $1);}
| LESS{$$=node("relop", 1, $1);}
| GREATER{$$=node("relop", 1, $1);}
|GREATEREQ{$$=node("relop", 1, $1);}
| EQ{$$=node("relop", 1, $1);}
| NEQ{$$=node("relop", 1, $1);}
;

additive-expression: additive-expression addop term{$$=node("additive-expression", 3, $1, $2, $3);}
|term{$$=node("additive-expression", 1, $1);}
;

addop: ADD{$$=node("addop", 1, $1);}
| MINUS{$$=node("addop", 1, $1);}
;

term: term mulop factor{$$=node("term", 3, $1, $2, $3);}
| factor{$$=node("term", 1, $1);}
;

mulop: MUL{$$=node("mulop", 1, $1);}
| DIV{$$=node("mulop", 1, $1);}
;

factor: LPAREN expression RPAREN{$$=node("factor", 3, $1, $2, $3);}
| var{$$=node("factor", 1, $1);}
| call{$$=node("factor", 1, $1);}
| integer{$$=node("factor", 1, $1);}
| float{$$=node("factor", 1, $1);}
;

integer: INTEGER{$$=node("integer", 1, $1);};

float: FLOAT{$$=node("float", 1, $1);};

call: ID LPAREN args RPAREN{$$=node("call", 4, $1, $2, $3, $4);}
;

args: arg-list{$$=node("args", 1, $1);}
| {$$=node("factor", 0);}
;

arg-list: arg-list COMMA expression{$$=node("arg-list", 3, $1, $2, $3);}
| expression{$$=node("arg-list", 1, $1);}
;

%%

    /// The error reporting function.
void yyerror(const char *s)
{
    // TO STUDENTS: This is just an example.
    // You can customize it as you like.
    
}

/// Parse input from file `input_path`, and prints the parsing results
/// to stdout.  If input_path is NULL, read from stdin.
///
/// This function initializes essential states before running yyparse().
syntax_tree *parse(const char *input_path)
{
    if (input_path != NULL)
    {
        if (!(yyin = fopen(input_path, "r")))
        {
            
            exit(1);
        }
    }
    else
    {
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
    if (children_num == 0)
    {
        child = new_syntax_tree_node("epsilon");
        syntax_tree_add_child(p, child);
    }
    else
    {
        va_list ap;
        va_start(ap, children_num);
        for (int i = 0; i < children_num; ++i)
        {
            child = va_arg(ap, syntax_tree_node *);
            syntax_tree_add_child(p, child);
        }
        va_end(ap);
    }
    return p;
}
