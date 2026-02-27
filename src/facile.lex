%{
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "facile.tab.h"

%}

%option noyywrap

%%

";" {return SEMICOLON;}
":=" {return AFFECTATION;}
"+" {return ADD;}
"-" {return SUB;}
"*" {return MUL;}
"/" {return DIV;}
"(" {return PARENTHESIS_L;}
")" {return PARENTHESIS_R;}
"{" {return CURLY_BRACE_L;}
"}" {return CURLY_BRACE_R;}
"\"" {return DOUBLE_QUOTE;}
"if" {return IF;}
"then" {return THEN;}
"else" {return ELSE;}
"elseif" {return ELSEIF;}
"while" {return WHILE;}
"do" {return DO;}
"endwhile" {return ENDWHILE;}
"end" {return END;}
"endif" {return ENDIF;}
"continue" {return CONTINUE;}
"print" {return PRINT;}
"read" {return READ;}

[0-9]+ {
    yylval.integer = atoi(yytext);
    return INTEGER;
}

[a-zA-Z][a-zA-Z0-9_]* {
    yylval.identifier = strdup(yytext); // standard way
    return IDENTIFIER;
}

[ \t\n] ;
. {return yytext[0];}

%%
