%{
#include <stdio.h>
#include <string.h>
#include "ast.h"
#include "facile.tab.h"

%}

%option noyywrap
%option yylineno

%%

";" {return SEMICOLON;}
":" {return COLON;}
":=" {return AFFECTATION;}
"+" {return ADD;}
"-" {return SUB;}
"*" {return MUL;}
"/" {return DIV;}
"(" {return PARENTHESIS_L;}
")" {return PARENTHESIS_R;}
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
"break" {return BREAK;}
"print" {return PRINT;}
"read" {return READ;}
"not" {return NOT;}
"and" {return AND;}
"or" {return OR;}
"true" {return _TRUE;}
"false" {return _FALSE;}
">" {return GREATER_THAN;}
">=" {return GREATER_EQUALS;}
"<" {return LESSER_THAN;}
"<=" {return LESSER_EQUALS;}
"==" {return EQUALS;}
"#" {return HASH;}

"integer"   { return TYPE_INTEGER; }
"string"    { return TYPE_STRING; }

"//"[^\n]* {/* comments */}

"\""[^\"]*"\"" {
    yylval.identifier = strndup(yytext + 1, strlen(yytext) - 2);
    return STRING_LITERAL;
}

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
