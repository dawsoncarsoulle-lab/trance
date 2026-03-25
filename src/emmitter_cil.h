#ifndef _EMITTER_CIL_H_
#define _EMITTER_CIL_H_

#include <glib>
#include "ast.h"
/**
 * Emitter CIL
 *
 * Emitter for the CIL language, needing forward declaration of the produce code function to properly "fill in" the functions call inside constructs
 * and possibly raw emitters
 *
 */

void produce_code(CodeGenContext *ctx, GNode* node);


/**
 * Emitters
 *
 * Emitters containing specialized IR names, these are to be defined before functions like begin, produce and end ##code.
 */

void emit_op(CodeGenContext *ctx, const char *opcode) { fprintf(ctx->stream, "\t%s\n", opcode); }
void emit_load_int(CodeGenContext *ctx, int32_t value) { fprintf(ctx->stream, "\tldc.i4\t%d\n", value); }
void emit_local_var(CodeGenContext *ctx, const char *operation, int index) { fprintf(ctx->stream, "\t%s\t%d\n", operation, index); }
void emit_label(CodeGenContext *ctx, const char *label_prefix, int label_id) { fprintf(ctx->stream, "%s_%d:\n", label_prefix, label_id); }
void emit_branch(CodeGenContext *ctx, const char *opcode, const char *label_prefix, int label_id) { fprintf(ctx->stream, "\t%s %s_%d\n", opcode, label_prefix, label_id); }
void emit_call(CodeGenContext *ctx, const char *method_signature) { fprintf(ctx->stream, "\tcall %s\n", method_signature); }



/**
 * Constructs
 *
 * Constructs are language-defined sequences or tokens that need to be taken care of by a specific function
 * they use the  Emitters and are defined after
 *
 */


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
        emit_branch(ctx, "brfalse", "ELSE", label_id);
        produce_code(ctx, g_node_nth_child(node, 1));
        emit_branch(ctx, "br", "END", label_id);

        emit_label(ctx, "ELSE", label_id);
        produce_code(ctx, else_node);
    } else {
        emit_branch(ctx, "brfalse", "END", label_id);
        produce_code(ctx, g_node_nth_child(node, 1));
    }
    emit_label(ctx, "END", label_id);
}


/**
 * General structure : WHILE boolean DO block while_ender
 *                       $1    $2    $3  $4       $5
 *                              0         1        2
 */
void produce_while_statement(CodeGenContext *ctx, GNode *node) {
    int label_id = ctx->label_count++;

    emit_label(ctx, "WHILE", label_id);
    produce_code(ctx, g_node_nth_child(node, 0));
    emit_branch(ctx, "brfalse", "END", label_id);

    produce_code(ctx, g_node_nth_child(node, 1));

    emit_branch(ctx, "br", "WHILE", label_id);
    emit_label(ctx, "END", label_id);
}



/**
 * Pillars
 *
 * The pillars of building is setting up the IR context ( here the main and assembly and module ), producing the code for each element in the AST
 * and then ending
 */

#ifndef PROGRAM_NAME
    #define PROGRAM_NAME "DefaultName"
#endif

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

// WARNING: uses emit_op
#define BINARY_OPERATION_NODES(ctx, operation) {     \
    produce_code(ctx, g_node_nth_child(node, 0));    \
    produce_code(ctx, g_node_nth_child(node, 1));    \
    emit_op(ctx, operation);        \
}

// variables loaded in are offset by 1 (NULL + 1), this gets original index
#define GET_VAR_INDEX(node) (GPOINTER_TO_INT((node)->data) - 1)

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
            emit_local_var(ctx, "stloc", GET_VAR_INDEX(g_node_nth_child(g_node_nth_child(node, 0), 0)));
            break;
        case NODE_ADD: BINARY_OPERATION_NODES(ctx, "add"); break;
        case NODE_SUB: BINARY_OPERATION_NODES(ctx, "sub"); break;
        case NODE_MUL: BINARY_OPERATION_NODES(ctx, "mul"); break;
        case NODE_DIV: BINARY_OPERATION_NODES(ctx, "div"); break;
        case NODE_EQUALS: BINARY_OPERATION_NODES(ctx, "ceq"); break;
        case NODE_HASH: // double negation proof since no specific instruction
            BINARY_OPERATION_NODES(ctx, "ceq"); // initial negation
            // combo negation 2
            emit_load_int(ctx, 0);
            emit_op(ctx, "ceq");
            break;
        case NODE_LESSER_THAN: BINARY_OPERATION_NODES(ctx, "clt"); break;
        case NODE_GREATER_THAN: BINARY_OPERATION_NODES(ctx, "cgt"); break;
        case NODE_IF_STATEMENT:     produce_if_statement(ctx, node);    break;
        case NODE_WHILE_STATEMENT:  produce_while_statement(ctx, node); break;
        case NODE_NUMBER:       emit_load_int(ctx, (int32_t)g_node_nth_child(node, 0)->data);              break;
        case NODE_IDENTIFIER:   emit_local_var(ctx, "ldloc", GET_VAR_INDEX(g_node_nth_child(node, 0)));    break;
        case NODE_PRINT:
            produce_code(ctx, g_node_nth_child(node, 0));
            emit_call(ctx, "void class [mscorlib]System.Console::WriteLine(int32)");
            break;
        case NODE_READ:
            emit_call(ctx, "string class [mscorlib]System.Console::ReadLine()");
            emit_call(ctx, "int32 int32::Parse(string)");
            emit_local_var(ctx, "stloc", GET_VAR_INDEX(g_node_nth_child(g_node_nth_child(node, 0), 0)));
            break;
        default: fprintf(ctx->stream, "ERROR: unknown token"); break;
    }
}

void end_code(CodeGenContext *ctx) {
    fprintf(ctx->stream,
        "\tret\n"
        "}\n");
}


#endif // _EMITTER_CIL_H_
