#ifndef _EMITTER_H_
#define _EMITTER_H_

void emit_op(CodeGenContext *ctx, const char *opcode);
void emit_load_int(CodeGenContext *ctx, int32_t value);
// TODO: create specialized functions for variable loading and writing
// with the goal to remove all inline string literals from "top" business logic
void emit_local_var(CodeGenContext *ctx, const char *operation, int index);
void emit_branch(CodeGenContext *ctx, const char *opcode, const char *label_prefix, int label_id);
void emit_label(CodeGenContext *ctx, const char *label_prefix, int label_id);
void emit_call(CodeGenContext *ctx, const char *method_signature);

#ifdef EMITTER_IMPLEMENTATION

void emit_op(CodeGenContext *ctx, const char *opcode) { fprintf(ctx->stream, "\t%s\n", opcode); }
void emit_load_int(CodeGenContext *ctx, int32_t value) {
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

#endif // EMITTER_IMPLEMENTATION
#endif // _EMITTER_H_
