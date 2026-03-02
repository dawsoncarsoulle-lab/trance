%{
    #include <stdio.h>
    #include <glib.h>
    #define CODEGEN_IMPLEMENTATION
    #include "codegen.h"

    GNode *ast_root_node = NULL;
    GHashTable *table = NULL;
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
        | BREAK SEMICOLON
        | CONTINUE SEMICOLON
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
        | expr ADD expr     { $$ = g_node_new(GINT_TO_POINTER(NODE_ADD)); g_node_append($$, $1); g_node_append($$, $3); }
        | expr SUB expr     { $$ = g_node_new(GINT_TO_POINTER(NODE_SUB)); g_node_append($$, $1); g_node_append($$, $3); }
        | expr MUL expr     { $$ = g_node_new(GINT_TO_POINTER(NODE_MUL)); g_node_append($$, $1); g_node_append($$, $3); }
        | expr DIV expr     { $$ = g_node_new(GINT_TO_POINTER(NODE_DIV)); g_node_append($$, $1); g_node_append($$, $3); }
        | PARENTHESIS_L expr PARENTHESIS_R { $$ = $2; }
        ;

    if_ender: END | ENDIF;

    if_statement:
        IF boolean THEN block if_ender {
            $$ = g_node_new(GINT_TO_POINTER(NODE_IF_STATEMENT));
            g_node_append($$, $2);
            g_node_append($$, $4);
        }
        | IF boolean THEN block ELSE block if_ender {
            $$ = g_node_new(GINT_TO_POINTER(NODE_IF_STATEMENT));
                g_node_append($$, $2);
                g_node_append($$, $4);
                g_node_append($$, $6);
        }
        | IF boolean THEN else_if_statement if_ender
        | IF boolean THEN else_if_statement ELSE block if_ender
        ;

    else_if_statement:
        ELSEIF boolean THEN block
        | else_if_statement ELSEIF boolean THEN block
        ;

    while_ender: END | ENDWHILE;

    while_statement: WHILE boolean DO block while_ender {
        $$ = g_node_new(GINT_TO_POINTER(NODE_WHILE_STATEMENT));
        g_node_append($$, $2);
        g_node_append($$, $4);
    };

    boolean:
        _TRUE
        | _FALSE
        | NOT boolean
        | expr HASH expr {
            $$ = g_node_new(GINT_TO_POINTER(NODE_HASH));
            g_node_append($$, $1);
            g_node_append($$, $3);
        }
        | expr EQUALS expr {
            $$ = g_node_new(GINT_TO_POINTER(NODE_EQUALS));
            g_node_append($$, $1);
            g_node_append($$, $3);
        }
        | boolean OR boolean
        | boolean AND boolean
        | expr LESSER_THAN expr {
            $$ = g_node_new(GINT_TO_POINTER(NODE_LESSER_THAN));
            g_node_append($$, $1);
            g_node_append($$, $3);
        }
        | expr GREATER_THAN expr  {
            $$ = g_node_new(GINT_TO_POINTER(NODE_GREATER_THAN));
            g_node_append($$, $1);
            g_node_append($$, $3);
        }
        | expr LESSER_EQUALS expr
        | expr GREATER_EQUALS expr
        | PARENTHESIS_L boolean PARENTHESIS_R
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
        $$ = g_node_new(GINT_TO_POINTER(NODE_AFFECTATION));
        g_node_append($$, id_node);
        g_node_append($$, $3);
    };

%%

extern char *yytext;
extern int yylineno;

void yyerror(const char *s) {
    fprintf(stderr, "Syntax Error at line %d near unexpected token: '%s'\nGiven error is : %s\n", yylineno, yytext, s);
}

int main() {

    table = g_hash_table_new(g_str_hash, g_str_equal);

    extern int yydebug;
    // yydebug = 1;

    if (yyparse() == 0) {
        CodeGenContext ctx = {0};
        ctx.stream = fopen("facile.il", "w");
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
