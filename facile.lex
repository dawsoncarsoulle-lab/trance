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
#define TOK_ELSE 273
#define TOK_ELSEIF 274
#define TOK_WHILE 275
#define TOK_DO 276
#define TOK_ENDWHILE 277
#define TOK_CODE_WHILE 278
#define TOK_PRINT 279
#define TOK_READ 280
#define TOK_END 281
#define TOK_ENDIF 282
#define TOK_CONTINUE 283
#define TOK_BREAK 284

%}

%option noyywrap

%%
if {
    // assert(printf("'if' found"));
    assert(printf("TOK_IF\n"));
    return TOK_IF;
}

then {
    // assert(printf("'then' found"));
    assert(printf("TOK_THEN\n"));
    return TOK_THEN;
}

";" {
    // assert(printf("';' found"));
    assert(printf("TOK_SEMI_COLON\n"));
    return TOK_SEMI_COLON;
}

":=" {
    // assert(printf("':=' found"));
    assert(printf("TOK_AFFECTATION\n"));
    return TOK_AFFECTATION;
}

"+" {
    // assert(printf("'+' found"));
    assert(printf("TOK_ADD\n"));
    return TOK_ADD;
}

"-" {
    // assert(printf("'-' found"));
    assert(printf("TOK_SUB\n"));
    return TOK_SUB;
}

"*" {
    // assert(printf("'*' found"));
    assert(printf("TOK_MUL\n"));
    return TOK_MUL;
}

"/" {
    // assert(printf("'/' found"));
    assert(printf("TOK_DIV\n"));
    return TOK_DIV;
}

"(" {
    assert(printf("TOK_PARENTHESIS_L\n"));
    return TOK_PARENTHESIS_L;
}

")" {
    assert(printf("TOK_PARENTHESIS_R\n"));
    return TOK_PARENTHESIS_R;
}

"{" {
    assert(printf("TOK_CURLY_BRACE_L\n"));
    return TOK_CURLY_BRACE_L;
}

"}" {
    assert(printf("TOK_CURLY_BRACE_R\n"));
    return TOK_CURLY_BRACE_R;
}

"\"" {
    assert(printf("TOK_DOUBLE_QUOTE\n"));
    return TOK_DOUBLE_QUOTE;
}

"else" {
    assert(printf("TOK_ELSE\n"));
    return TOK_ELSE;
}

"elseif" {
    assert(printf("TOK_ELSEIF\n"));
    return TOK_ELSEIF;
}

"while" {
    assert(printf("TOK_WHILE\n"));
    return TOK_WHILE;
}

"do" {
    assert(printf("TOK_DO\n"));
    return TOK_DO;
}

"endwhile" {
    assert(printf("TOK_ENDWHILE\n"));
    return TOK_ENDWHILE;
}

"code_while" {
    assert(printf("TOK_CODE_WHILE\n"));
    return TOK_CODE_WHILE;
}

"print" {
    assert(printf("TOK_PRINT\n"));
    return TOK_PRINT;
}

"read" {
    assert(printf("TOK_READ\n"));
    return TOK_READ;
}

"end" {
    assert(printf("TOK_END\n"));
    return TOK_END;
}

"endif" {
    assert(printf("TOK_ENDIF\n"));
    return TOK_ENDIF;
}

"continue" {
    assert(printf("TOK_CONTINUE\n"));
    return TOK_CONTINUE;
}

[0-9]+ {
    // assert(printf(" %d found", atoi(yytext)));
    assert(printf("TOK_INTEGER\n"));
    return TOK_INTEGER;
}

[a-zA-Z][a-zA-Z0-9_]* {
    // assert(printf("%s found", yytext));
    // assert(printf("identifier '%s(%d)' found", yytext, yyleng));
    assert(printf("TOK_IDENTIFIER\n"));
    return TOK_IDENTIFIER;
}

[ \t\n] ;

. {
    return yytext[0];
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
