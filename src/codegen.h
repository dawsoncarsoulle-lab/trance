#ifndef _CODEGEN_H_
#define _CODEGEN_H_

#include "ast.h"
#include <stdio.h>

#ifndef PROGRAM_NAME
#define PROGRAM_NAME "DefaultName"
#endif

typedef struct {
  FILE *stream;
  int label_count;
  int current_loop_label;
} CodeGenContext;

void begin_code(CodeGenContext *ctx, int local_count);
void produce_code(CodeGenContext *ctx, FacileNode *node);
void end_code(CodeGenContext *ctx);

#ifdef CODEGEN_IMPLEMENTATION

#define EMITTER_IMPLEMENTATION
#include "emitter.h"

// WARNING: uses emit_op from emitter.h
#define BINARY_OPERATION_NODES(ctx, node, operation)                                               \
  do {                                                                                             \
    produce_code(ctx, (node)->children[0]);                                                        \
    produce_code(ctx, (node)->children[1]);                                                        \
    emit_op(ctx, operation);                                                                       \
  } while (0)

typedef enum {
  NODE_BLOCK,
  // statements/instructions
  NODE_READ,
  NODE_PRINT,
  NODE_AFFECTATION,
  NODE_IF_STATEMENT,
  NODE_WHILE_STATEMENT,
  NODE_BREAK,
  NODE_CONTINUE,
  // expression
  NODE_NUMBER,
  NODE_IDENTIFIER,
  NODE_ADD,
  NODE_SUB,
  NODE_MUL,
  NODE_DIV,
  // boolean
  NODE_TRUE,
  NODE_FALSE,
  NODE_NOT,
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

void begin_code(CodeGenContext *ctx, int local_count) {
  fprintf(ctx->stream, ".assembly extern mscorlib {}\n"
                       ".assembly " PROGRAM_NAME " {}\n"
                       "\n"
                       ".method static void Main()\n"
                       "{\n"
                       "    .entrypoint\n"
                       "    .maxstack " DEFAULT_MAX_STACK "\n");
  if (local_count > 0) {
    fprintf(ctx->stream, "    .locals init ( ");
    for (int i = 0; i < local_count; i++)
      fprintf(ctx->stream, "        int32 V_%d%s ", i, (i == local_count - 1) ? "" : ",");
    fprintf(ctx->stream, "    )\n");
  }
}

/**
 * General structure : IF boolean THEN block ELSE block END
 *                     $1   $2     $3   $4    $5   $6
 *                           0           1          2
 */
void produce_if_statement(CodeGenContext *ctx, FacileNode *node) {
  int label_id = ctx->label_count++;
  FacileNode *else_node = node->children[2];

  produce_code(ctx, node->children[0]);

  if (else_node) {
    emit_branch(ctx, "brfalse", "ELSE", label_id);
    produce_code(ctx, node->children[1]);
    emit_branch(ctx, "br", "END", label_id);

    emit_label(ctx, "ELSE", label_id);
    produce_code(ctx, else_node);
  } else {
    emit_branch(ctx, "brfalse", "END", label_id);
    produce_code(ctx, node->children[1]);
  }
  emit_label(ctx, "END", label_id);
}

/**
 * General structure : WHILE boolean DO block while_ender
 *                       $1    $2    $3  $4       $5
 *                              0         1        2
 */
void produce_while_statement(CodeGenContext *ctx, FacileNode *node) {
  int label_id = ctx->label_count++;
  int prev_loop_label = ctx->current_loop_label;

  ctx->current_loop_label = label_id;

  emit_label(ctx, "WHILE", label_id);
  produce_code(ctx, node->children[0]);
  emit_branch(ctx, "brfalse", "END", label_id);

  produce_code(ctx, node->children[1]);
  emit_branch(ctx, "br", "WHILE", label_id);

  emit_label(ctx, "END", label_id);
  ctx->current_loop_label = prev_loop_label;
}

void produce_code(CodeGenContext *ctx, FacileNode *node) {
  if (node == NULL)
    return;

  switch (node->type) {
  case NODE_BLOCK:
    produce_code(ctx, node->children[0]);
    produce_code(ctx, node->children[1]);
    break;
  case NODE_AFFECTATION:
    produce_code(ctx, node->children[1]);
    emit_local_var(ctx, "stloc", node->children[0]->data);
    break;
  case NODE_ADD   : BINARY_OPERATION_NODES(ctx, node, "add"); break;
  case NODE_SUB   : BINARY_OPERATION_NODES(ctx, node, "sub"); break;
  case NODE_MUL   : BINARY_OPERATION_NODES(ctx, node, "mul"); break;
  case NODE_DIV   : BINARY_OPERATION_NODES(ctx, node, "div"); break;
  case NODE_EQUALS: BINARY_OPERATION_NODES(ctx, node, "ceq"); break;
  case NODE_HASH:                             // double negation proof since no specific instruction
    BINARY_OPERATION_NODES(ctx, node, "ceq"); // initial negation
    // combo negation 2
    emit_load_int(ctx, 0);
    emit_op(ctx, "ceq");
    break;
  case NODE_TRUE : emit_load_int(ctx, 1); break;
  case NODE_FALSE: emit_load_int(ctx, 0); break;
  case NODE_NOT:
    produce_code(ctx, node->children[0]);
    emit_load_int(ctx, 0);
    emit_op(ctx, "ceq");
    break;
  case NODE_AND        : BINARY_OPERATION_NODES(ctx, node, "and"); break;
  case NODE_OR         : BINARY_OPERATION_NODES(ctx, node, "or"); break;
  case NODE_LESSER_THAN: BINARY_OPERATION_NODES(ctx, node, "clt"); break;
  case NODE_LESSER_EQUALS:
    BINARY_OPERATION_NODES(ctx, node, "cgt");
    emit_load_int(ctx, 0);
    emit_op(ctx, "ceq");
    break;
  case NODE_GREATER_THAN: BINARY_OPERATION_NODES(ctx, node, "cgt"); break;
  case NODE_GREATER_EQUALS:
    BINARY_OPERATION_NODES(ctx, node, "clt");
    emit_load_int(ctx, 0);
    emit_op(ctx, "ceq");
    break;
  case NODE_IF_STATEMENT   : produce_if_statement(ctx, node); break;
  case NODE_WHILE_STATEMENT: produce_while_statement(ctx, node); break;
  case NODE_NUMBER         : emit_load_int(ctx, node->data); break;
  case NODE_IDENTIFIER     : emit_local_var(ctx, "ldloc", node->data); break;
  case NODE_PRINT:
    produce_code(ctx, node->children[0]);
    emit_call(ctx, "void class [mscorlib]System.Console::WriteLine(int32)");
    break;
  case NODE_READ:
    emit_call(ctx, "string class [mscorlib]System.Console::ReadLine()");
    emit_call(ctx, "int32 int32::Parse(string)");
    emit_local_var(ctx, "stloc", node->children[0]->data);
    break;
  case NODE_BREAK   : emit_branch(ctx, "br", "END", ctx->current_loop_label); break;
  case NODE_CONTINUE: emit_branch(ctx, "br", "WHILE", ctx->current_loop_label); break;
  default           : fprintf(ctx->stream, "ERROR: unknown token"); break;
  }
}

void end_code(CodeGenContext *ctx) {
  fprintf(ctx->stream, "\tret\n"
                       "}\n");
}

#endif // CODEGEN_IMPLEMENTATION

#endif // _CODEGEN_H_
