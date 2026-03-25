#ifndef _CODEGEN_H_
#define _CODEGEN_H_

#include <assert.h>
#include <glib.h>
#include <stdio.h>

#ifndef PROGRAM_NAME
#define PROGRAM_NAME "DefaultName"
#endif

typedef struct {
  FILE *stream;
  int label_count;
  int current_loop_label;
} CodeGenContext;

#define EMITTER_IMPLEMENTATION
#include "emitter.h"

void begin_code(CodeGenContext *ctx, guint local_count);
void produce_code(CodeGenContext *ctx, GNode *node);
void end_code(CodeGenContext *ctx);

#ifdef CODEGEN_IMPLEMENTATION

// WARNING: uses emit_op from emitter.h
#define BINARY_OPERATION_NODES(ctx, operation)                                 \
  {                                                                            \
    produce_code(ctx, g_node_nth_child(node, 0));                              \
    produce_code(ctx, g_node_nth_child(node, 1));                              \
    emit_op(ctx, operation);                                                   \
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
  NODE_BREAK,
  NODE_CONTINUE,
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
  fprintf(ctx->stream, ".assembly extern mscorlib {}\n"
                       ".assembly " PROGRAM_NAME " {}\n"
                       "\n"
                       ".method static void Main()\n"
                       "{\n"
                       "    .entrypoint\n"
                       "    .maxstack " DEFAULT_MAX_STACK "\n");
  if (local_count > 0) {
    fprintf(ctx->stream, "    .locals init (\n");
    for (guint i = 0; i < local_count; i++)
      fprintf(ctx->stream, "        int32 V_%d%s ", i,
              (i == local_count - 1) ? "" : ",");
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

  int prev_loop_label = ctx->current_loop_label;
  ctx->current_loop_label = label_id;
  emit_label(ctx, "WHILE", label_id);
  produce_code(ctx, g_node_nth_child(node, 0));
  emit_branch(ctx, "brfalse", "END", label_id);
  produce_code(ctx, g_node_nth_child(node, 1));
  emit_branch(ctx, "br", "WHILE", label_id);
  emit_label(ctx, "END", label_id);
  ctx->current_loop_label = prev_loop_label;
}

// variables loaded in are offset by 1 (NULL + 1), this gets original index
#define GET_VAR_INDEX(node) (GPOINTER_TO_INT((node)->data) - 1)

void produce_code(CodeGenContext *ctx, GNode *node) {
  if (node == NULL)
    return;
  ASTNodeType node_type = (ASTNodeType)GPOINTER_TO_INT(node->data);
  switch (node_type) {
  case NODE_BLOCK:
    produce_code(ctx, g_node_nth_child(node, 0));
    produce_code(ctx, g_node_nth_child(node, 1));
    break;
  case NODE_AFFECTATION:
    produce_code(ctx, g_node_nth_child(node, 1));
    emit_local_var(
        ctx, "stloc",
        GET_VAR_INDEX(g_node_nth_child(g_node_nth_child(node, 0), 0)));
    break;
  case NODE_ADD:
    BINARY_OPERATION_NODES(ctx, "add");
    break;
  case NODE_SUB:
    BINARY_OPERATION_NODES(ctx, "sub");
    break;
  case NODE_MUL:
    BINARY_OPERATION_NODES(ctx, "mul");
    break;
  case NODE_DIV:
    BINARY_OPERATION_NODES(ctx, "div");
    break;
  case NODE_EQUALS:
    BINARY_OPERATION_NODES(ctx, "ceq");
    break;
  case NODE_HASH: // double negation proof since no specific instruction
    BINARY_OPERATION_NODES(ctx, "ceq"); // initial negation
    // combo negation 2
    emit_load_int(ctx, 0);
    emit_op(ctx, "ceq");
    break;
  case NODE_LESSER_THAN:
    BINARY_OPERATION_NODES(ctx, "clt");
    break;
  case NODE_GREATER_THAN:
    BINARY_OPERATION_NODES(ctx, "cgt");
    break;
  case NODE_IF_STATEMENT:
    produce_if_statement(ctx, node);
    break;
  case NODE_WHILE_STATEMENT:
    produce_while_statement(ctx, node);
    break;
  case NODE_NUMBER:
    emit_load_int(ctx, (int32_t)g_node_nth_child(node, 0)->data);
    break;
  case NODE_IDENTIFIER:
    emit_local_var(ctx, "ldloc", GET_VAR_INDEX(g_node_nth_child(node, 0)));
    break;
  case NODE_PRINT:
    produce_code(ctx, g_node_nth_child(node, 0));
    emit_call(ctx, "void class [mscorlib]System.Console::WriteLine(int32)");
    break;
  case NODE_READ:
    emit_call(ctx, "string class [mscorlib]System.Console::ReadLine()");
    emit_call(ctx, "int32 int32::Parse(string)");
    emit_local_var(
        ctx, "stloc",
        GET_VAR_INDEX(g_node_nth_child(g_node_nth_child(node, 0), 0)));
    break;
  case NODE_IF_STATEMENT:
    produce_if_statement(ctx, node);
    break;
  case NODE_WHILE_STATEMENT:
    produce_while_statement(ctx, node);
    break;
  case NODE_BREAK:
    emit_branch(ctx, "br", "END", ctx->current_loop_label);
    break;
  case NODE_CONTINUE:
    emit_branch(ctx, "br", "WHILE", ctx->current_loop_label);
    break;
  default:
    fprintf(ctx->stream, "ERROR: unknown token");
    break;
  }
}

void end_code(CodeGenContext *ctx) {
  fprintf(ctx->stream, "\tret\n"
                       "}\n");
}

#endif // CODEGEN_IMPLEMENTATION

#endif // _CODEGEN_H_
