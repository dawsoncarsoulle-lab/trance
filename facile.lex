%{
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define TOK_IF 258
#define TOK_THEN 259
#define TOK_SEMI_COLON 260
#define TOK_AFFECTATION 261
#define TOK_ADD 262
#define TOK_SUB 263
#define TOK_MUL 264
#define TOK_DIV 265

#define TOK_IDENTIFIER 266
#define TOK_INTEGER 267

#define TOK_CURLY_BRACE_L 268
#define TOK_CURLY_BRACE_R 269

#define TOK_PARENTHESIS_L 270
#define TOK_PARENTHESIS_R 271

#define TOK_DOUBLE_QUOTE 272
%}

%option noyywrap

%%
if {
    // assert(printf("'if' found"));
    assert(printf("TOK_IF"));
    return TOK_IF;
}

then {
    // assert(printf("'then' found"));
    assert(printf("TOK_THEN"));
    return TOK_THEN;
}

";" {
    // assert(printf("';' found"));
    assert(printf("TOK_SEMI_COLON"));
    return TOK_SEMI_COLON;
}

":=" {
    // assert(printf("':=' found"));
    assert(printf("TOK_AFFECTATION"));
    return TOK_AFFECTATION;
}

"+" {
    // assert(printf("'+' found"));
    assert(printf("TOK_ADD"));
    return TOK_ADD;
}

"-" {
    // assert(printf("'-' found"));
    assert(printf("TOK_SUB"));
    return TOK_SUB;
}

"*" {
    // assert(printf("'*' found"));
    assert(printf("TOK_MUL"));
    return TOK_MUL;
}

"/" {
    // assert(printf("'/' found"));
    assert(printf("TOK_DIV"));
    return TOK_DIV;
}

"(" {
    assert(printf("TOK_PARENTHESIS_L"));
    return TOK_PARENTHESIS_L;
}

")" {
    assert(printf("TOK_PARENTHESIS_R"));
    return TOK_PARENTHESIS_R;
}

"{" {
    assert(printf("TOK_CURLY_BRACE_L"));
    return TOK_CURLY_BRACE_L;
}

"}" {
    assert(printf("TOK_CURLY_BRACE_R"));
    return TOK_CURLY_BRACE_R;
}

"\"" {
    assert(printf("TOK_DOUBLE_QUOTE"));
    return TOK_DOUBLE_QUOTE;
}


[0-9]+ {
    // assert(printf(" %d found", atoi(yytext)));
    assert(printf("TOK_INTEGER"));
    return TOK_INTEGER;
}

[a-zA-Z][a-zA-Z0-9_]* {
    // assert(printf("%s found", yytext));
    assert(printf("TOK_IDENTIFIER"));
    return TOK_IDENTIFIER;
}

%%

int main(int argc, char* argv[]) {

  if (argc == 2) {

  FILE *code = fopen(argv[1], "r");

  if (!code) { return -1; }

  yyin = code;
  while(yylex());

  } else {
    yylex();
    return 0;
  }

}
