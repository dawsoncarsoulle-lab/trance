#ifndef _CODEGEN_H_
#define _CODEGEN_H_

#include <stdio.h>
#include <assert.h>
#include <glib.h>

#ifndef PROGRAM_NAME
    #define PROGRAM_NAME "DefaultName"
#endif


typedef struct {
    FILE * stream;
    int label_count;
} CodeGenContext;

void begin_code(CodeGenContext *ctx, guint local_count);
void produce_code(CodeGenContext *ctx, GNode *node);
void end_code(CodeGenContext *ctx);

#ifdef  CODEGEN_IMPLEMENTATION

// WARNING uses CodeGenContext internals
#define BINARY_OPERATION_NODES(ctx, operation) {     \
    produce_code(ctx, g_node_nth_child(node, 0));    \
    produce_code(ctx, g_node_nth_child(node, 1));    \
    fprintf(ctx->stream, "\t%s\n", operation);        \
}

typedef enum {
    NODE_IDENTIFIER,
    NODE_NUMBER,
    NODE_BLOCK,
    NODE_INSTRUCTION,
    NODE_READ,
    NODE_PRINT,
    NODE_AFFECTATION,
    NODE_ADD,
    NODE_SUB,
    NODE_MUL,
    NODE_DIV,
    // statements
    NODE_IF_STATEMENT,
    NODE_WHILE_STATEMENT,
    // boolean
    NODE_HASH,
    NODE_EQUALS,
    NODE_OR,
    NODE_AND,
    NODE_LESSER_THAN,
    NODE_GREATER_THAN,
    NODE_LESSER_EQUALS,
    NODE_GREATER_EQUALS,
} ASTNodeType;


#define DEFAULT_MAX_STACK "8"

void begin_code(CodeGenContext *ctx, guint local_count) {
    fprintf(ctx->stream,
        ".assembly extern mscorlib {}\n"
        ".assembly " PROGRAM_NAME " {}\n"
        "\n"
        ".method static void Main()\n"
        "{\n"
        "    .entrypoint\n"
        "    .maxstack " DEFAULT_MAX_STACK "\n"
    );
    if (local_count > 0) {
            fprintf(ctx->stream, "    .locals init (\n");
            for (guint i = 0; i < local_count; i++)
                fprintf(ctx->stream, "        int32 V_%d%s ", i, (i == local_count - 1) ? "" : ",");
            fprintf(ctx->stream, "    )\n");
    }
}

/**
 * General structure : IF boolean THEN block ELSE block END
 *                     $1   $2     $3   $4    $5   $6
 *                           0           1          2
 */
void produce_if_statement(CodeGenContext *ctx, GNode *node) {
    int label_id = ctx->label_count++;
    GNode *else_node = g_node_nth_child(node, 2);

    produce_code(ctx, g_node_nth_child(node, 0));

    if (else_node) {
        fprintf(ctx->stream, "\tbrfalse ELSE_%d\n", label_id);
        produce_code(ctx, g_node_nth_child(node, 1));
        fprintf(ctx->stream, "\tbr END_%d\n"
                                   "ELSE_%d:\n", label_id, label_id);
        produce_code(ctx, else_node);
    } else {
        fprintf(ctx->stream, "\tbrfalse END_%d\n", label_id);
        produce_code(ctx, g_node_nth_child(node, 1));
    }
    fprintf(ctx->stream, "END_%d:\n", label_id);
}


/**
 * General structure : WHILE boolean DO block while_ender
 *                       $1    $2    $3  $4       $5
 *                              0         1        2
 */
void produce_while_statement(CodeGenContext *ctx, GNode *node) {
    int label_id = ctx->label_count++;

    fprintf(ctx->stream, "WHILE_%d:\n", label_id);
    produce_code(ctx, g_node_nth_child(node, 0));

    fprintf(ctx->stream, "\tbrfalse END_%d\n", label_id);
    produce_code(ctx, g_node_nth_child(node, 1));
    fprintf(ctx->stream, "\tbr WHILE_%d\n"
                            "END_%d:\n", label_id, label_id);
}

void produce_code(CodeGenContext *ctx, GNode* node) {
    if (node == NULL) return;
    ASTNodeType node_type = (ASTNodeType)GPOINTER_TO_INT(node->data);
    switch(node_type) {
        case NODE_BLOCK:
            produce_code(ctx, g_node_nth_child(node, 0));
            produce_code(ctx, g_node_nth_child(node, 1));
        break;
        case NODE_AFFECTATION:
            produce_code(ctx, g_node_nth_child(node, 1));
            fprintf(ctx->stream, "\tstloc\t%ld\n", (long)g_node_nth_child(g_node_nth_child(node, 0), 0)->data - 1);
        break;
        case NODE_ADD: BINARY_OPERATION_NODES(ctx, "add"); break;
        case NODE_SUB: BINARY_OPERATION_NODES(ctx, "sub"); break;
        case NODE_MUL: BINARY_OPERATION_NODES(ctx, "mul"); break;
        case NODE_DIV: BINARY_OPERATION_NODES(ctx, "div"); break;
        case NODE_EQUALS: BINARY_OPERATION_NODES(ctx, "ceq"); break;
        case NODE_HASH: // double negation proof since no specific instruction
            BINARY_OPERATION_NODES(ctx, "ceq");             // negation
            fprintf(ctx->stream, "\tldc.i4 0\n\tceq\n");    // negation with a nneagtive value (0)
            break;
        case NODE_LESSER_THAN: BINARY_OPERATION_NODES(ctx, "clt"); break;
        case NODE_GREATER_THAN: BINARY_OPERATION_NODES(ctx, "cgt"); break;
        case NODE_IF_STATEMENT: produce_if_statement(ctx, node); break;
        case NODE_WHILE_STATEMENT: produce_while_statement(ctx, node); break;
        case NODE_NUMBER:       fprintf(ctx->stream, "\tldc.i4\t%ld\n", (long)g_node_nth_child(node, 0)->data);      break;
        case NODE_IDENTIFIER:   fprintf(ctx->stream, "\tldloc\t%ld\n", (long)g_node_nth_child(node, 0)->data - 1);   break;
        case NODE_PRINT:
            produce_code(ctx, g_node_nth_child(node, 0));
            fprintf(ctx->stream, "\tcall void class [mscorlib]System.Console::WriteLine(int32)\n");
        break;
        case NODE_READ:
            fprintf(ctx->stream, "\tcall string class [mscorlib]System.Console::ReadLine()\n");
            fprintf(ctx->stream, "\tcall int32 int32::Parse(string)\n");
            fprintf(ctx->stream, "\tstloc\t%ld\n", (long)g_node_nth_child(g_node_nth_child(node, 0), 0)->data - 1);
        break;
        default: fprintf(ctx->stream, "ERROR: unknown token"); break;
    }
}

void end_code(CodeGenContext *ctx) {
    fprintf(ctx->stream,
        "\tret\n"
        "}\n");
}



#endif // CODEGEN_IMPLEMENTATION

#endif // _CODEGEN_H_
