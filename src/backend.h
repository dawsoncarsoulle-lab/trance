#ifndef _BACKEND_H_
#define _BACKEND_H_

#include "ast.h"
#include "context.h"

// "environment" setup
void facile_begin_program(CodeGenContext *ctx, int local_count);
void facile_end_program(CodeGenContext *ctx);

// statements/instructions
void facile_produce_block(CodeGenContext *ctx, FacileNode *node);
void facile_produce_affectation(CodeGenContext *ctx, FacileNode *node);
void facile_produce_if(CodeGenContext *ctx, FacileNode *node);
void facile_produce_while(CodeGenContext *ctx, FacileNode *node);
void facile_produce_break(CodeGenContext *ctx, FacileNode *node);
void facile_produce_continue(CodeGenContext *ctx, FacileNode *node);
void facile_produce_print(CodeGenContext *ctx, FacileNode *node);
void facile_produce_read(CodeGenContext *ctx, FacileNode *node);
void facile_produce_string_literal(CodeGenContext *ctx, FacileNode *node);

// boolean
void facile_produce_add(CodeGenContext *ctx, FacileNode *node);
void facile_produce_sub(CodeGenContext *ctx, FacileNode *node);
void facile_produce_mul(CodeGenContext *ctx, FacileNode *node);
void facile_produce_div(CodeGenContext *ctx, FacileNode *node);
void facile_produce_number(CodeGenContext *ctx, FacileNode *node);
void facile_produce_identifier(CodeGenContext *ctx, FacileNode *node);
void facile_produce_true(CodeGenContext *ctx, FacileNode *node);
void facile_produce_false(CodeGenContext *ctx, FacileNode *node);
void facile_produce_not(CodeGenContext *ctx, FacileNode *node);
void facile_produce_hash(CodeGenContext *ctx, FacileNode *node);
void facile_produce_and(CodeGenContext *ctx, FacileNode *node);
void facile_produce_or(CodeGenContext *ctx, FacileNode *node);
void facile_produce_eq(CodeGenContext *ctx, FacileNode *node);
void facile_produce_ge(CodeGenContext *ctx, FacileNode *node);
void facile_produce_gt(CodeGenContext *ctx, FacileNode *node);
void facile_produce_le(CodeGenContext *ctx, FacileNode *node);
void facile_produce_lt(CodeGenContext *ctx, FacileNode *node);

#endif // _BACKEND_H_
