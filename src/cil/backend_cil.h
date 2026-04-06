#ifndef _BACKEND_CIL_H_
#define _BACKEND_CIL_H_

#include "emitter_cil.h"

#ifndef COMPILER_IMPLEMENTATION
static void produce_code(CodeGenContext *ctx, FacileNode *node);
#endif

#ifndef PROGRAM_NAME
#define PROGRAM_NAME "DefaultName"
#endif

#define DEFAULT_MAX_STACK "8"

void facile_begin_program(CodeGenContext *ctx, int local_count) {
  fprintf(ctx->stream, ".assembly extern mscorlib {}\n"
                       ".assembly " PROGRAM_NAME " {}\n"
                       "\n"
                       ".method static void Main()\n"
                       "{\n"
                       "    .entrypoint\n"
                       "    .maxstack " DEFAULT_MAX_STACK "\n");
  if (shlen(ctx->table) > 0) {
    fprintf(ctx->stream, "    .locals init (\n");

    for (int i = 0; i < shlen(ctx->table); i++) {
      const char *t = (ctx->table[i].type == T_STR) ? "string" : "int32";
      fprintf(ctx->stream, "      [%d] %s %s%s\n", i, t, ctx->table[i].key,
              (i == shlen(ctx->table) - 1) ? "" : ",");
    }
    fprintf(ctx->stream, "    )\n");
  }
}

void facile_end_program(CodeGenContext *ctx) {
  fprintf(ctx->stream, "\tret\n"
                       "}\n");
}

// statements/instructions
void facile_produce_block(CodeGenContext *ctx, FacileNode *node) {
  produce_code(ctx, node->children[0]);
  produce_code(ctx, node->children[1]);
}

void facile_produce_affectation(CodeGenContext *ctx, FacileNode *node) {
  produce_code(ctx, node->children[1]);
  emit_local_var(ctx, "stloc", node->children[0]->data);
}

/**
 * General structure : IF boolean THEN block ELSE block END
 *                     $1   $2     $3   $4    $5   $6
 *                           0           1          2
 */
void facile_produce_if(CodeGenContext *ctx, FacileNode *node) {
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
void facile_produce_while(CodeGenContext *ctx, FacileNode *node) {
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

void facile_produce_break(CodeGenContext *ctx, FacileNode *node) {
  emit_branch(ctx, "br", "END", ctx->current_loop_label);
}
void facile_produce_continue(CodeGenContext *ctx, FacileNode *node) {
  emit_branch(ctx, "br", "WHILE", ctx->current_loop_label);
}

void facile_produce_print(CodeGenContext *ctx, FacileNode *node) {
  produce_code(ctx, node->children[0]);
  if (node->children[0]->evaluated_type == T_INT)
    emit_call(ctx, "void class [mscorlib]System.Console::WriteLine(int32)");
  else
    emit_call(ctx, "void class [mscorlib]System.Console::WriteLine(string)");
}
void facile_produce_read(CodeGenContext *ctx, FacileNode *node) {
  emit_call(ctx, "string class [mscorlib]System.Console::ReadLine()");

  if (node->children[0]->evaluated_type == T_INT)
    emit_call(ctx, "int32 int32::Parse(string)");

  emit_local_var(ctx, "stloc", node->children[0]->data);
}

void facile_produce_string_literal(CodeGenContext *ctx, FacileNode *node) {
  fprintf(ctx->stream, "\tldstr \"%s\"\n", node->string_lit);
}

#define BINARY_OPERATION_NODES(ctx, node, operation)                                               \
  do {                                                                                             \
    produce_code(ctx, (node)->children[0]);                                                        \
    produce_code(ctx, (node)->children[1]);                                                        \
    emit_op(ctx, operation);                                                                       \
  } while (0)

// boolean
void facile_produce_true(CodeGenContext *ctx, FacileNode *node) { emit_load_int(ctx, 1); }
void facile_produce_false(CodeGenContext *ctx, FacileNode *node) { emit_load_int(ctx, 0); }
void facile_produce_hash(CodeGenContext *ctx, FacileNode *node) {
  BINARY_OPERATION_NODES(ctx, node, "ceq"); // initial negation
  emit_load_int(ctx, 0);
  emit_op(ctx, "ceq"); // combo negation 2
}
void facile_produce_add(CodeGenContext *ctx, FacileNode *node) {
  BINARY_OPERATION_NODES(ctx, node, "add");
}
void facile_produce_sub(CodeGenContext *ctx, FacileNode *node) {
  BINARY_OPERATION_NODES(ctx, node, "sub");
}
void facile_produce_mul(CodeGenContext *ctx, FacileNode *node) {
  BINARY_OPERATION_NODES(ctx, node, "mul");
}
void facile_produce_div(CodeGenContext *ctx, FacileNode *node) {
  BINARY_OPERATION_NODES(ctx, node, "div");
}
void facile_produce_number(CodeGenContext *ctx, FacileNode *node) {
  emit_load_int(ctx, node->data);
}
void facile_produce_identifier(CodeGenContext *ctx, FacileNode *node) {
  emit_local_var(ctx, "ldloc", node->data);
}
void facile_produce_not(CodeGenContext *ctx, FacileNode *node) {
  produce_code(ctx, node->children[0]);
  emit_load_int(ctx, 0);
  emit_op(ctx, "ceq");
}
void facile_produce_and(CodeGenContext *ctx, FacileNode *node) {
  BINARY_OPERATION_NODES(ctx, node, "and");
}
void facile_produce_or(CodeGenContext *ctx, FacileNode *node) {
  BINARY_OPERATION_NODES(ctx, node, "or");
}
void facile_produce_eq(CodeGenContext *ctx, FacileNode *node) {
  BINARY_OPERATION_NODES(ctx, node, "ceq");
}
void facile_produce_ge(CodeGenContext *ctx, FacileNode *node) {
  BINARY_OPERATION_NODES(ctx, node, "clt");
  emit_load_int(ctx, 0);
  emit_op(ctx, "ceq");
}
void facile_produce_gt(CodeGenContext *ctx, FacileNode *node) {
  BINARY_OPERATION_NODES(ctx, node, "cgt");
}
void facile_produce_le(CodeGenContext *ctx, FacileNode *node) {
  BINARY_OPERATION_NODES(ctx, node, "cgt");
  emit_load_int(ctx, 0);
  emit_op(ctx, "ceq");
}
void facile_produce_lt(CodeGenContext *ctx, FacileNode *node) {
  BINARY_OPERATION_NODES(ctx, node, "clt");
}

#endif // _BACKEND_CIL_H_
