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
PARENTHESIS_L PARENTHESIS_R AFFECTATION PRINT READ NOT AND OR GREATER_THAN
LESSER_THAN HASH EQUALS FALSE TRUE GREATER_EQUALS LESSER_EQUALS


// added but has not relation in language to the precedence of math operators
%left HASH EQUALS GREATER_EQUALS LESSER_EQUALS LESSER_THAN GREATER_THAN NOT OR AND

%left ADD SUB
%left MUL DIV // mul div are "greedy"

%%
    /*
        base "nothing" case handled by program reader parent
    */
    program:
        | program instruction
        ;

    instruction:
        read_call
        | print_call
        | affectation
        | if_statement
        | while_statement
        ;

    expr:
        INTEGER
        | IDENTIFIER
        | expr MUL expr
        | expr DIV expr
        | expr ADD expr
        | expr SUB expr
        | PARENTHESIS_L expr PARENTHESIS_R
        ;

    if_statement:
        IF boolean THEN instruction END
        | IF boolean THEN instruction ENDIF
        | IF boolean THEN else_if_statement ELSE instruction END
        | IF boolean THEN else_if_statement ELSE instruction ENDIF
        ;

    else_if_statement:
        ELSEIF boolean THEN instruction
        | else_if_statement ELSEIF boolean THEN instruction
        ;


    while_statement:
        WHILE boolean DO instruction END
        | WHILE boolean DO instruction ENDWHILE
        | WHILE boolean DO code_while_statement END
        | WHILE boolean DO code_while_statement ENDWHILE
        ;

    code_while_statement:
        |  BREAK code_while_statement
        |  CONTINUE code_while_statement
        ;


    boolean:
        TRUE
        | FALSE
        | NOT boolean
        | expr HASH expr
        | expr EQUALS expr
        | boolean OR boolean
        | boolean AND boolean
        | expr LESSER_THAN expr
        | expr GREATER_THAN expr
        | expr LESSER_EQUALS expr
        | expr GREATER_EQUALS expr
        | PARENTHESIS_L boolean PARENTHESIS_R
        ;

    read_call: READ IDENTIFIER SEMICOLON;
    print_call: PRINT IDENTIFIER SEMICOLON;
    affectation: IDENTIFIER AFFECTATION IDENTIFIER SEMICOLON;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}

int main() {
    return yyparse();
}
