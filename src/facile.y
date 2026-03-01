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


%left OR
%left AND
%left EQUALS HASH
%left LESSER_THAN GREATER_THAN LESSER_EQUALS GREATER_EQUALS
%left ADD SUB
%left MUL DIV
%right NOT

%%
    /*
        base "nothing" case handled by program reader parent
    */
    program: block
        ;
    block:
        | block instruction
        ;

    instruction:
        read_call
        | print_call
        | affectation
        | if_statement
        | while_statement
        | BREAK SEMICOLON
        | CONTINUE SEMICOLON
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
        IF boolean THEN block END
        | IF boolean THEN block ENDIF
        | IF boolean THEN block ELSE block END
        | IF boolean THEN block ELSE block ENDIF
        | IF boolean THEN else_if_statement END
        | IF boolean THEN else_if_statement ENDIF
        | IF boolean THEN else_if_statement ELSE block END
        | IF boolean THEN else_if_statement ELSE block ENDIF
        ;

    else_if_statement:
        ELSEIF boolean THEN block
        | else_if_statement ELSEIF boolean THEN block
        ;


    while_statement:
        WHILE boolean DO block END
        | WHILE boolean DO block ENDWHILE
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
    affectation: IDENTIFIER AFFECTATION expr SEMICOLON;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}

int main() {
    return yyparse();
}
