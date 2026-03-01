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
                fprintf(ctx->stream, "        int32 V_%d%s ", i, i, (i == local_count - 1) ? "" : ",");
            fprintf(ctx->stream, "    )\n");
    }
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
