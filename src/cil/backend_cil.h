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
  emit_assembly_header(ctx, PROGRAM_NAME);
  emit_method_main_start(ctx, DEFAULT_MAX_STACK);

  if (local_count > 0) {
    emit_locals_init_start(ctx);

    for (int i = 0; i < local_count; i++) {
      const char *type = (ctx->table[i].type == T_STR) ? "string" : "int32";
      int is_last = (i == local_count - 1);
      emit_local_variable_decl(ctx, i, type, ctx->table[i].key, is_last);
    }

    emit_locals_init_end(ctx);
  }
}

void facile_end_program(CodeGenContext *ctx) { emit_method_main_end(ctx); }

// statements/instructions
void facile_produce_block(CodeGenContext *ctx, FacileNode *node) {
  produce_code(ctx, node->children[0]);
  produce_code(ctx, node->children[1]);
}

void facile_produce_affectation(CodeGenContext *ctx, FacileNode *node) {
  produce_code(ctx, node->children[1]);
  emit_store_local(ctx, node->children[0]->data);
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
    emit_branch_false_else(ctx, label_id);
    produce_code(ctx, node->children[1]);
    emit_jump_end(ctx, label_id);

    emit_label_else(ctx, label_id);
    produce_code(ctx, else_node);
  } else {
    emit_branch_false_end(ctx, label_id);
    produce_code(ctx, node->children[1]);
  }
  emit_label_end(ctx, label_id);
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

  emit_label_while(ctx, label_id);
  produce_code(ctx, node->children[0]);
  emit_branch_false_end(ctx, label_id);

  produce_code(ctx, node->children[1]);
  emit_jump_while(ctx, label_id);

  emit_label_end(ctx, label_id);
  ctx->current_loop_label = prev_loop_label;
}

void facile_produce_break(CodeGenContext *ctx, FacileNode *node) {
  emit_jump_end(ctx, ctx->current_loop_label);
}
void facile_produce_continue(CodeGenContext *ctx, FacileNode *node) {
  emit_jump_while(ctx, ctx->current_loop_label);
}

void facile_produce_print(CodeGenContext *ctx, FacileNode *node) {
  produce_code(ctx, node->children[0]);
  if (node->children[0]->evaluated_type == T_INT)
    emit_sys_print_int(ctx);
  else
    emit_sys_print_string(ctx);
}
void facile_produce_read(CodeGenContext *ctx, FacileNode *node) {
  emit_sys_read_line(ctx);

  if (node->children[0]->evaluated_type == T_INT)
    emit_sys_parse_int(ctx);

  emit_store_local(ctx, node->children[0]->data);
}

void facile_produce_string_literal(CodeGenContext *ctx, FacileNode *node) {
  emit_string_literal(ctx, node->string_lit);
}

#define BINARY_OPERATION_NODES(ctx, node, emitter_function)                                        \
  do {                                                                                             \
    produce_code(ctx, (node)->children[0]);                                                        \
    produce_code(ctx, (node)->children[1]);                                                        \
    emitter_function(ctx);                                                                         \
  } while (0)

// boolean
void facile_produce_true(CodeGenContext *ctx, FacileNode *node) { emit_load_int(ctx, 1); }
void facile_produce_false(CodeGenContext *ctx, FacileNode *node) { emit_load_int(ctx, 0); }
void facile_produce_hash(CodeGenContext *ctx, FacileNode *node) {
  BINARY_OPERATION_NODES(ctx, node, emit_ceq); // initial negation
  emit_load_int(ctx, 0);
  emit_ceq(ctx); // combo negation 2
}
void facile_produce_add(CodeGenContext *ctx, FacileNode *node) {
  BINARY_OPERATION_NODES(ctx, node, emit_add);
}
void facile_produce_sub(CodeGenContext *ctx, FacileNode *node) {
  BINARY_OPERATION_NODES(ctx, node, emit_sub);
}
void facile_produce_mul(CodeGenContext *ctx, FacileNode *node) {
  BINARY_OPERATION_NODES(ctx, node, emit_mul);
}
void facile_produce_div(CodeGenContext *ctx, FacileNode *node) {
  BINARY_OPERATION_NODES(ctx, node, emit_div);
}
void facile_produce_number(CodeGenContext *ctx, FacileNode *node) {
  emit_load_int(ctx, node->data);
}
void facile_produce_identifier(CodeGenContext *ctx, FacileNode *node) {
  emit_load_local(ctx, node->data);
}
void facile_produce_not(CodeGenContext *ctx, FacileNode *node) {
  produce_code(ctx, node->children[0]);
  emit_load_int(ctx, 0);
  emit_ceq(ctx);
}
void facile_produce_and(CodeGenContext *ctx, FacileNode *node) {
  BINARY_OPERATION_NODES(ctx, node, emit_and);
}
void facile_produce_or(CodeGenContext *ctx, FacileNode *node) {
  BINARY_OPERATION_NODES(ctx, node, emit_or);
}
void facile_produce_eq(CodeGenContext *ctx, FacileNode *node) {
  BINARY_OPERATION_NODES(ctx, node, emit_ceq);
}
void facile_produce_ge(CodeGenContext *ctx, FacileNode *node) {
  BINARY_OPERATION_NODES(ctx, node, emit_clt);
  emit_load_int(ctx, 0);
  emit_ceq(ctx);
}
void facile_produce_gt(CodeGenContext *ctx, FacileNode *node) {
  BINARY_OPERATION_NODES(ctx, node, emit_cgt);
}
void facile_produce_le(CodeGenContext *ctx, FacileNode *node) {
  BINARY_OPERATION_NODES(ctx, node, emit_cgt);
  emit_load_int(ctx, 0);
  emit_ceq(ctx);
}
void facile_produce_lt(CodeGenContext *ctx, FacileNode *node) {
  BINARY_OPERATION_NODES(ctx, node, emit_clt);
}

#endif // _BACKEND_CIL_H_
