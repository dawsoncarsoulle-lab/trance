#ifndef _EMITTER_CIL_H_
#define _EMITTER_CIL_H_

#include "../context.h"

void emit_op(CodeGenContext *ctx, const char *opcode) { fprintf(ctx->stream, "\t%s\n", opcode); }
void emit_load_int(CodeGenContext *ctx, int value) {
  fprintf(ctx->stream, "\tldc.i4\t%d\n", value);
}
void emit_local_var(CodeGenContext *ctx, const char *operation, int index) {
  fprintf(ctx->stream, "\t%s\t%d\n", operation, index);
}
void emit_label(CodeGenContext *ctx, const char *label_prefix, int label_id) {
  fprintf(ctx->stream, "%s_%d:\n", label_prefix, label_id);
}
void emit_branch(CodeGenContext *ctx, const char *opcode, const char *label_prefix, int label_id) {
  fprintf(ctx->stream, "\t%s %s_%d\n", opcode, label_prefix, label_id);
}
void emit_call(CodeGenContext *ctx, const char *method_signature) {
  fprintf(ctx->stream, "\tcall %s\n", method_signature);
}

#endif // _EMITTER_CIL_H_
