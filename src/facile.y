%{
    #include <stdio.h>
%}

%union {
    int integer;
    char * identifier;
}

%token <integer> INTEGER;
%token <identifier> IDENTIFIER;

%token IF THEN ELSE ELSEIF END ENDIF WHILE DO ENDWHILE CONTINUE BREAK
DOUBLE_QUOTE SEMICOLON ADD SUB MUL DIV CURLY_BRACE_L CURLY_BRACE_R
PARENTHESIS_L PARENTHESIS_R AFFECTATION PRINT READ

%%
    program:
        | program statement
        ;

    statement:
        read_call;

    read_call: READ IDENTIFIER SEMICOLON {
            printf("identifier : %s",$2);
    }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}

int main() {
    return yyparse();
}
