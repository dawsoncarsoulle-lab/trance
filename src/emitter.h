#ifndef _EMITTER_H_
#define _EMITTER_H_

#include "context.h"

void emit_op(CodeGenContext *ctx, const char *opcode);
void emit_load_int(CodeGenContext *ctx, int value);
// TODO: create specialized functions for variable loading and writing
// with the goal to remove all inline string literals from "top" business logic
void emit_local_var(CodeGenContext *ctx, const char *operation, int index);
void emit_branch(CodeGenContext *ctx, const char *opcode, const char *label_prefix, int label_id);
void emit_label(CodeGenContext *ctx, const char *label_prefix, int label_id);
void emit_call(CodeGenContext *ctx, const char *method_signature);

#endif // _EMITTER_H_
