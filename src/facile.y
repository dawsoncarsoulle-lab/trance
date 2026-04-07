%{
    #include <stdio.h>

    #define STB_DS_IMPLEMENTATION
    #include "../include/stb_ds.h"

    #define CODEGEN_IMPLEMENTATION
    #define BACKEND_LANGUAGE__CIL
    #include "codegen.h"

    int yylex(void);
    void yyerror(const char *s);

    #define DEFAULT_TYPE T_INT
    FacileNode *ast_root_node = NULL;
    FacileSymbol *table = NULL;


    // parent_node_type found in codegen.h
    #define PARENT_(parent_node_type) do { yyval.node = facile_create_node(parent_node_type); } while (0);

    #define _WITH_CHILD(child_node) \
        do { facile_node_add_child(yyval.node, child_node); } while (0);

    #define _WITH_CHILDREN_BINARY(left, right) \
        do { facile_node_add_child(yyval.node, left); facile_node_add_child(yyval.node, right); } while (0);
    #define _WITH_CHILDREN_TERNARY(first, second, third) \
        do { facile_node_add_child(yyval.node, first); facile_node_add_child(yyval.node, second); facile_node_add_child(yyval.node, third); } while (0);

    #define FATAL_ERROR(error_message) do { yyerror(error_message); YYABORT; } while(0)
    #define FATAL_ERROR_DYNAMIC(error_message_format, ...) do {                                             \
        char error_message_buffer[256];                                                                     \
        snprintf(error_message_buffer, sizeof(error_message_buffer), error_message_format, ##__VA_ARGS__);  \
        yyerror(error_message_buffer);                                                                      \
        YYABORT;                                                                                            \
    } while(0)

    int facile_symbol_declaration(char *name, DataType type) {
        if (shgeti(table, name) != -1) return -1; // case exists already

        int id = shlen(table);
        shput(table, name, id);
        shgetp(table, name)->type = (int)type;

        return id;
    }

    int facile_symbol_lookup(char *name) {
        ptrdiff_t idx = shgeti(table, name);
        free(name);
        if (idx == -1) return -1;

        return table[idx].value;
    }

    FacileNode* facile_resolve_identifier_node(char *name, DataType type) {
        ptrdiff_t idx = shgeti(table, name);

        int id;
        if (idx == -1)
            id = facile_symbol_declaration(name, type);
        else {
            type = (DataType)table[idx].type;
            id = facile_symbol_lookup(name);
        }

        FacileNode *id_node = facile_create_node(NODE_IDENTIFIER);
        id_node->data = id;
        id_node->evaluated_type = type;

        return id_node;
    }
    %}

%union {
    int integer;
    char * identifier;
    FacileNode * node;
    DataType data_type;
}

%define parse.trace

%token <integer> INTEGER;
%token <identifier> IDENTIFIER;
%token <identifier> STRING_LITERAL

%token IF THEN ELSE ELSEIF END ENDIF WHILE DO ENDWHILE CONTINUE BREAK SEMICOLON
PARENTHESIS_L PARENTHESIS_R AFFECTATION PRINT READ NOT AND OR GREATER_THAN
LESSER_THAN HASH EQUALS _FALSE _TRUE GREATER_EQUALS LESSER_EQUALS ADD SUB MUL DIV
TYPE_INTEGER TYPE_STRING COLON

// mirror node enum found in ast.h
// syntax transformation example: identifier -> NODE_IDENTIFIER
%type<node> program block instruction expr boolean
%type<node> if_statement else_if_statement while_statement
%type<node> read_call print_call affectation
%type<data_type> type_spec // exception


%left OR
%left AND
%left EQUALS HASH
%left LESSER_THAN GREATER_THAN LESSER_EQUALS GREATER_EQUALS
%left ADD SUB
%left MUL DIV
%right NOT

%%
    /* The initial block is the one from which the program is created and is thus the root, see pre-code section */
    program: block { ast_root_node = $1; };

    block:
        { $$ = NULL; }
        | block instruction {
            $$ = facile_create_node(NODE_BLOCK);
            if ($1 != NULL) facile_node_add_child($$, $1);
            facile_node_add_child($$, $2);
        }
        ;

    instruction:
        read_call
        | print_call
        | affectation
        | if_statement
        | while_statement
        | BREAK SEMICOLON { PARENT_(NODE_BREAK); }
        | CONTINUE SEMICOLON { PARENT_(NODE_CONTINUE); }
        ;

    expr:
        INTEGER {
            $$ = facile_create_node(NODE_NUMBER);
            $$->data = $1;
        }
        | IDENTIFIER {
            ptrdiff_t idx = shgeti(table, $1);
            if (idx == -1) FATAL_ERROR_DYNAMIC("Undeclared variable '%s' in expression", $1);

            DataType type = (DataType)table[idx].type;
            int id = facile_symbol_lookup($1);

            $$ = facile_create_node(NODE_IDENTIFIER);
            $$->data = id;
            $$->evaluated_type = type;
        }
        | STRING_LITERAL {
                $$ = facile_create_node(NODE_STRING_LITERAL);
                $$->string_lit = $1;
                $$->evaluated_type = T_STR;
        }
        | expr ADD expr     { PARENT_(NODE_ADD)_WITH_CHILDREN_BINARY($1, $3); }
        | expr SUB expr     { PARENT_(NODE_SUB)_WITH_CHILDREN_BINARY($1, $3); }
        | expr MUL expr     { PARENT_(NODE_MUL)_WITH_CHILDREN_BINARY($1, $3); }
        | expr DIV expr     { PARENT_(NODE_DIV)_WITH_CHILDREN_BINARY($1, $3); }
        | PARENTHESIS_L expr PARENTHESIS_R { $$ = $2; }
        ;

    if_ender: END | ENDIF;

    if_statement:
        IF boolean THEN block if_ender { PARENT_(NODE_IF_STATEMENT)_WITH_CHILDREN_BINARY($2, $4); }
        |
        IF boolean THEN block ELSE block if_ender { PARENT_(NODE_IF_STATEMENT)_WITH_CHILDREN_TERNARY($2, $4, $6); }
        |
        IF boolean THEN block else_if_statement if_ender { PARENT_(NODE_IF_STATEMENT)_WITH_CHILDREN_TERNARY($2, $4, $5); }
        |
        IF boolean THEN block else_if_statement ELSE block if_ender {
            FacileNode* current = $5;
            while (current->children[2] != NULL) current = current->children[2];
            facile_node_add_child(current, $7);

            PARENT_(NODE_IF_STATEMENT)_WITH_CHILDREN_TERNARY($2, $4, $5);        }
        ;

    else_if_statement:
        ELSEIF boolean THEN block {
            PARENT_(NODE_IF_STATEMENT)_WITH_CHILDREN_BINARY($2, $4);
        }
        |
        else_if_statement ELSEIF boolean THEN block {
            $$ = $1;
            FacileNode* new_if = facile_create_node(NODE_IF_STATEMENT);
            facile_node_add_child(new_if, $3);
            facile_node_add_child(new_if, $5);

            FacileNode* current = $$;
            while (current->children[2] != NULL) current = current->children[2];
            facile_node_add_child(current, new_if);
        }
        ;

    while_ender: END | ENDWHILE;

    while_statement: WHILE boolean DO block while_ender { PARENT_(NODE_WHILE_STATEMENT)_WITH_CHILDREN_BINARY($2, $4); };

    boolean:
        _TRUE                       { PARENT_(NODE_TRUE)    }
        | _FALSE                    { PARENT_(NODE_FALSE)   }
        | NOT boolean               { PARENT_(NODE_NOT)             _WITH_CHILD($2);               }
        | expr HASH expr            { PARENT_(NODE_HASH)            _WITH_CHILDREN_BINARY($1, $3); }
        | expr EQUALS expr          { PARENT_(NODE_EQUALS)          _WITH_CHILDREN_BINARY($1, $3); }
        | boolean OR boolean        { PARENT_(NODE_OR)              _WITH_CHILDREN_BINARY($1, $3); }
        | boolean AND boolean       { PARENT_(NODE_AND)             _WITH_CHILDREN_BINARY($1, $3); }
        | expr LESSER_THAN expr     { PARENT_(NODE_LESSER_THAN)     _WITH_CHILDREN_BINARY($1, $3); }
        | expr GREATER_THAN expr    { PARENT_(NODE_GREATER_THAN)    _WITH_CHILDREN_BINARY($1, $3); }
        | expr LESSER_EQUALS expr   { PARENT_(NODE_LESSER_EQUALS)   _WITH_CHILDREN_BINARY($1, $3); }
        | expr GREATER_EQUALS expr  { PARENT_(NODE_GREATER_EQUALS)  _WITH_CHILDREN_BINARY($1, $3); }
        | PARENTHESIS_L boolean PARENTHESIS_R { $$ = $2; }
        ;

    // WARNING: types are inherently unsafe since runtime checks don't exist
    read_call: READ IDENTIFIER SEMICOLON {
            FacileNode* id_node = facile_resolve_identifier_node($2, DEFAULT_TYPE);
            PARENT_(NODE_READ)_WITH_CHILD(id_node);
        }
        | READ IDENTIFIER COLON type_spec SEMICOLON {
            int id = facile_symbol_declaration($2, $4);

            FacileNode* id_node = facile_create_node(NODE_IDENTIFIER);
            id_node->data = id;
            id_node->evaluated_type = $4;

            PARENT_(NODE_READ)_WITH_CHILD(id_node);
        };
    print_call: PRINT expr SEMICOLON { PARENT_(NODE_PRINT)_WITH_CHILD($2); };
    affectation:
        IDENTIFIER AFFECTATION expr SEMICOLON {
            FacileNode* id_node = facile_resolve_identifier_node($1, $3->evaluated_type);

            if (id_node->evaluated_type != $3->evaluated_type) FATAL_ERROR("Type mismatch during affectation");

            PARENT_(NODE_AFFECTATION)_WITH_CHILDREN_BINARY(id_node, $3);
        }
        | IDENTIFIER COLON type_spec AFFECTATION expr SEMICOLON {
            if ($3 != $5->evaluated_type) FATAL_ERROR("Type mismatch during affectation");

            int id = facile_symbol_declaration($1, $3);
            if (id == -1) FATAL_ERROR("Variable re-declaration is not allowed");

            FacileNode* id_node = facile_create_node(NODE_IDENTIFIER);
            id_node->data = id;
            id_node->evaluated_type = $3;

            PARENT_(NODE_AFFECTATION)_WITH_CHILDREN_BINARY(id_node, $5);
        }
        ;

    type_spec:
        TYPE_INTEGER { $$ = T_INT; }
        | TYPE_STRING { $$ = T_STR; }
        ;
%%

extern char *yytext;
extern int yylineno;

void yyerror(const char *s) {
    fprintf(stderr, "Syntax Error at line %d near unexpected token: '%s' -> %s\n", yylineno, yytext, s);
}


#include <ctype.h>
#define FACILE_FILE_EXTENSION ".facile"

int main(int argc, char * argv[]) {
    if (argc != 2) return EXIT_FAILURE;

    char *path = argv[1];
    int filename_length = strlen(path);

    for (int i = sizeof(FACILE_FILE_EXTENSION) - 2, j = filename_length - 1; i >= 0; i--, j--)
        if (j < 0 || path[j] != FACILE_FILE_EXTENSION[i]) {
            fprintf(stderr, "input is not a .facile file\n");
            return EXIT_FAILURE;
        }

    int start = filename_length - 1;
    while (start >= 0 && path[start] != '/')  start--;
    start++;

    if (!isalpha(path[start]) && path[start] != '_') {
        fprintf(stderr, "name must start with letter or underscore\n");
        return EXIT_FAILURE;
    }
    for (int i = start + 1; i < filename_length - 7; i++)
        if (!isalnum(path[i]) && path[i] != '_') {
            fprintf(stderr, "name contains invalid characters\n");
            return EXIT_FAILURE;
    }

    char output_filename[256];
    int base_len = filename_length - 7 - start;
    snprintf(output_filename, sizeof(output_filename), "%.*s.il", base_len, &path[start]);

    extern int yydebug;
    extern FILE *yyin;

    yyin = fopen(path, "r");
    if (!yyin) {
        fprintf(stderr, "Error: Failed to open %s\n", path);
        return EXIT_FAILURE;
    }

    if (yyparse() == 0) {
        CodeGenContext ctx = {0};
        ctx.stream = fopen(output_filename, "w");
        ctx.table = table;
        if (ctx.stream == NULL) {
            fprintf(stderr, "Error: Failed to open facile.il for writing.\n");
            return EXIT_FAILURE;
        }
        int local_count = shlen(table);

        facile_begin_program(&ctx, local_count);
        produce_code(&ctx, ast_root_node);
        facile_end_program(&ctx);

        fclose(ctx.stream);
        if (ast_root_node != NULL) facile_node_free(ast_root_node);

    } else {
        printf("Compilation failed due to syntax errors.\n");
        return EXIT_FAILURE;
    }
    shfree(table);
    fclose(yyin);
    return EXIT_SUCCESS;
}
