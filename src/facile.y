%{
    #include <stdio.h>
    #include <glib.h>
    #define CODEGEN_IMPLEMENTATION
    #include "codegen.h"

    GNode *ast_root_node = NULL;
    GHashTable *table = NULL;


    // parent_node_type found in codegen.h
    #define PARENT_(parent_node_type) \
    do { yyval.node = g_node_new(GINT_TO_POINTER(parent_node_type)); } while (0);

    #define _WITH_CHILDREN_BINARY(left, right) \
    do { g_node_append(yyval.node, left); g_node_append(yyval.node, right); } while (0);

    #define _WITH_CHILDREN_TERNARY(first, second, third) \
    do { g_node_append(yyval.node, first); g_node_append(yyval.node, second); g_node_append(yyval.node, third); } while (0);

%}

%union {
    int integer;
    char * identifier;
    GNode * node;
}

%define parse.trace

%token <integer> INTEGER;
%token <identifier> IDENTIFIER;

%token IF THEN ELSE ELSEIF END ENDIF WHILE DO ENDWHILE CONTINUE BREAK
DOUBLE_QUOTE SEMICOLON ADD SUB MUL DIV CURLY_BRACE_L CURLY_BRACE_R
PARENTHESIS_L PARENTHESIS_R AFFECTATION PRINT READ NOT AND OR GREATER_THAN
LESSER_THAN HASH EQUALS _FALSE _TRUE GREATER_EQUALS LESSER_EQUALS

// mirror node enum found in codegen.h
// syntax transformation example: identifier -> NODE_IDENTIFIER
%type<node> identifier
%type<node> number
%type<node> add
%type<node> sub
%type<node> mul
%type<node> div

%type<node> program block instruction expr boolean
%type<node> if_statement else_if_statement while_statement
%type<node> read_call print_call affectation


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
            $$ = g_node_new(GINT_TO_POINTER(NODE_BLOCK));
            if ($1 != NULL) g_node_append($$, $1);
            g_node_append($$, $2);
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
            $$ = g_node_new(GINT_TO_POINTER(NODE_NUMBER));
            g_node_append_data($$, GINT_TO_POINTER($1));
        }
        | IDENTIFIER {
            gulong value = (gulong) g_hash_table_lookup(table, $1);
            if (!value) {
                char error_msg[256];
                snprintf(error_msg, sizeof(error_msg), "Error: Undeclared variable '%s' in expression", $1);
                yyerror(error_msg);
                YYABORT;
            }
            $$ = g_node_new(GINT_TO_POINTER(NODE_IDENTIFIER));
            g_node_append_data($$, (gpointer)value);
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
        IF boolean THEN block else_if_statement if_ender {
            PARENT_(NODE_IF_STATEMENT)_WITH_CHILDREN_TERNARY($2, $4, $5);
        }
        |
        IF boolean THEN block else_if_statement ELSE block if_ender {
            GNode* current = $5;
            while (g_node_nth_child(current, 2) != NULL) {
                current = g_node_nth_child(current, 2);
            }
            g_node_append(current, $7);

            PARENT_(NODE_IF_STATEMENT)_WITH_CHILDREN_TERNARY($2, $4, $5);
        }
        ;

    else_if_statement:
        ELSEIF boolean THEN block {
            PARENT_(NODE_IF_STATEMENT)_WITH_CHILDREN_BINARY($2, $4);
        }
        |
        else_if_statement ELSEIF boolean THEN block {
            $$ = $1;
            GNode *new_if = g_node_new(GINT_TO_POINTER(NODE_IF_STATEMENT));
            g_node_append(new_if, $3);
            g_node_append(new_if, $5);
            GNode* current = $$;
            while (g_node_nth_child(current, 2) != NULL) {
                current = g_node_nth_child(current, 2);
            }
            g_node_append(current, new_if);
        }
        ;

    while_ender: END | ENDWHILE;

    while_statement: WHILE boolean DO block while_ender { PARENT_(NODE_WHILE_STATEMENT)_WITH_CHILDREN_BINARY($2, $4); };

    boolean:
        _TRUE
        | _FALSE
        | NOT boolean
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

    read_call: READ IDENTIFIER SEMICOLON {
        gulong value = (gulong) g_hash_table_lookup(table, $2);
        if (!value) {
            value = g_hash_table_size(table) + 1;
            g_hash_table_insert(table, strdup($2), (gpointer)value);
        }

        GNode *id_node = g_node_new(GINT_TO_POINTER(NODE_IDENTIFIER));
        g_node_append_data(id_node, (gpointer)value);

        $$ = g_node_new(GINT_TO_POINTER(NODE_READ));
        g_node_append($$, id_node);
    };
    print_call: PRINT expr SEMICOLON {
        $$ = g_node_new(GINT_TO_POINTER(NODE_PRINT));
        g_node_append($$, $2);
    };
    affectation: IDENTIFIER AFFECTATION expr SEMICOLON {
        gulong value = (gulong) g_hash_table_lookup(table, $1);
        if (!value) {
            value = g_hash_table_size(table) + 1;
            g_hash_table_insert(table, strdup($1), (gpointer) value);
        }
        GNode *id_node = g_node_new(GINT_TO_POINTER(NODE_IDENTIFIER));
        g_node_append_data(id_node, (gpointer)value);
        PARENT_(NODE_AFFECTATION)_WITH_CHILDREN_BINARY(id_node, $3)
    };

%%

extern char *yytext;
extern int yylineno;

void yyerror(const char *s) {
    fprintf(stderr, "Syntax Error at line %d near unexpected token: '%s'\nGiven error is : %s\n", yylineno, yytext, s);
}

int main(int argc, char * argv[]) {
    char * il_filename = "facile.il";
    if (argc == 2) il_filename = argv[1];
    table = g_hash_table_new(g_str_hash, g_str_equal);

    extern int yydebug;
    // yydebug = 1;

    if (yyparse() == 0) {
        CodeGenContext ctx = {0};
        ctx.stream = fopen(il_filename, "w");
        if (ctx.stream == NULL) {
            fprintf(stderr, "Error: Failed to open facile.il for writing.\n");
            return 1;
        }
        guint local_count = g_hash_table_size(table);
        begin_code(&ctx, local_count);
        produce_code(&ctx, ast_root_node);
        end_code(&ctx);

        fclose(ctx.stream);
        if (ast_root_node != NULL) g_node_destroy(ast_root_node);

    } else {
        printf("Compilation failed due to syntax errors.\n");
        return 1;
    }
    return 0;
}
