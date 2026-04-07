#ifndef _CODEGEN_H_
#define _CODEGEN_H_

#include "backend.h"

void produce_code(CodeGenContext *ctx, FacileNode *node) {
  if (node == NULL)
    return;

  switch (node->type) {
  case NODE_BLOCK: facile_produce_block(ctx, node); break;
  // instructions/statements
  case NODE_AFFECTATION    : facile_produce_affectation(ctx, node); break;
  case NODE_IF_STATEMENT   : facile_produce_if(ctx, node); break;
  case NODE_WHILE_STATEMENT: facile_produce_while(ctx, node); break;
  case NODE_BREAK          : facile_produce_break(ctx, node); break;
  case NODE_CONTINUE       : facile_produce_continue(ctx, node); break;
  case NODE_PRINT          : facile_produce_print(ctx, node); break;
  case NODE_READ           : facile_produce_read(ctx, node); break;
  // expressions
  case NODE_ADD           : facile_produce_add(ctx, node); break;
  case NODE_SUB           : facile_produce_sub(ctx, node); break;
  case NODE_MUL           : facile_produce_mul(ctx, node); break;
  case NODE_DIV           : facile_produce_div(ctx, node); break;
  case NODE_NUMBER        : facile_produce_number(ctx, node); break;
  case NODE_IDENTIFIER    : facile_produce_identifier(ctx, node); break;
  case NODE_STRING_LITERAL: facile_produce_string_literal(ctx, node); break;
  // boolean
  case NODE_TRUE          : facile_produce_true(ctx, node); break;
  case NODE_FALSE         : facile_produce_false(ctx, node); break;
  case NODE_NOT           : facile_produce_not(ctx, node); break;
  case NODE_HASH          : facile_produce_hash(ctx, node); break;
  case NODE_EQUALS        : facile_produce_eq(ctx, node); break;
  case NODE_OR            : facile_produce_or(ctx, node); break;
  case NODE_AND           : facile_produce_and(ctx, node); break;
  case NODE_LESSER_THAN   : facile_produce_lt(ctx, node); break;
  case NODE_GREATER_THAN  : facile_produce_gt(ctx, node); break;
  case NODE_LESSER_EQUALS : facile_produce_le(ctx, node); break;
  case NODE_GREATER_EQUALS: facile_produce_ge(ctx, node); break;

  default: fprintf(stderr, "ERROR: unknown AST node type\n"); break;
  }
}

#ifdef CODEGEN_IMPLEMENTATION
#ifdef BACKEND_LANGUAGE__CIL
#include "cil/backend_cil.h"
#endif

#endif // CODEGEN_IMPLEMENTATION
#endif // _CODEGEN_H_
