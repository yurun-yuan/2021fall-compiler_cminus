%option noyywrap 
%{
/*****************声明和选项设置  begin*****************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "syntax_tree.h"
#include "syntax_analyzer.h"

int lines = 0;
int pos_start;
int pos_end;

void pass_node(char *text)
{
     yylval.node = new_syntax_tree_node(text);
}

void advance(int diff)
{
     pos_start = pos_end;
     pos_end += diff;
}

/*****************声明和选项设置  end*****************/

%}

%%
    /* to do for students */
    /* two cases for you, pass_node will send flex's token to bison */

int { advance(strlen(yytext)); pass_node(yytext);  return INT;} 
float { advance(strlen(yytext)); pass_node(yytext);  return FLOAT;} 
if { advance(strlen(yytext)); pass_node(yytext);  return IF;}
else { advance(strlen(yytext)); pass_node(yytext);  return ELSE;} 
return { advance(strlen(yytext)); pass_node(yytext);  return RETURN;} 
void { advance(strlen(yytext)); pass_node(yytext);  return VOID;} 
while { advance(strlen(yytext)); pass_node(yytext);  return WHILE;}
(\r\n|\r|\n) { lines++; pos_start = pos_end = 0;}
[ \t] { advance(1);} 
\+ { advance(1); pass_node(yytext);  return ADD;} 
\- { advance(1); pass_node(yytext);  return MINUS;}
\* { advance(1); pass_node(yytext);  return MUL;}
\/ { advance(1); pass_node(yytext);  return DIV;}
\< { advance(1); pass_node(yytext);  return LESS;} 
\<= { advance(2); pass_node(yytext);  return LESSEQ;} 
> { advance(1); pass_node(yytext);  return GREATER;}
>= { advance(2); pass_node(yytext);  return GREATEREQ;} 
== { advance(2); pass_node(yytext);  return EQ;} 
!= { advance(2); pass_node(yytext);  return NEQ;} 
= { advance(1); pass_node(yytext);  return ASSIGN;} 
; { advance(1); pass_node(yytext);  return SEMICOLON;} 
, { advance(1); pass_node(yytext);  return COMMA;} 
\( { advance(1); pass_node(yytext);  return LPAREN;} 
\) { advance(1); pass_node(yytext);  return RPAREN;} 
\[ {  advance(1);  pass_node(yytext);    return LBRACKET; } 
\] { advance(1); pass_node(yytext);  return RBRACKET;} 
\{ { advance(1); pass_node(yytext);  return LBRACE;}
"}" { advance(1); pass_node(yytext);  return RBRACE;} 
"/*"((\*)*[^\*\/]|(\/))*(\*)*"*/" {
     int last_lf_pos=0;
     int lf_num=0;
     for(int i = 0; i < strlen(yytext); i++){
          if(yytext[i] == '\n'){
               lf_num ++;
               last_lf_pos = i;
          }
     }
     if(lf_num){
          lines += lf_num;
          pos_start = pos_end;
          pos_end = strlen(yytext) - last_lf_pos;
     }
     else
          advance(strlen(yytext));
}
[a-zA-Z]+ { advance(strlen(yytext)); pass_node(yytext);  return ID;} 
[0-9]+ { advance(strlen(yytext)); pass_node(yytext);  return INTEGER;} 
([0-9]+\.|[0-9]*\.[0-9]+) { advance(strlen(yytext)); pass_node(yytext); return FLOATPOINT;} 
. { advance(1);  return ERROR;}

%%
