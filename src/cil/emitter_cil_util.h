#ifdef CODEGEN_IMPLEMENTATION

static inline void _emit_op(CodeGenContext *ctx, const char *opcode) {
  fprintf(ctx->stream, "\t%s\n", opcode);
}
static inline void _emit_call(CodeGenContext *ctx, const char *method_signature) {
  fprintf(ctx->stream, "\tcall %s\n", method_signature);
}
static inline void _emit_raw_label(CodeGenContext *ctx, const char *prefix, int label_id) {
  fprintf(ctx->stream, "%s_%d:\n", prefix, label_id);
}
static inline void _emit_raw_branch(CodeGenContext *ctx, const char *op, const char *prefix,
                                    int label_id) {
  fprintf(ctx->stream, "\t%s %s_%d\n", op, prefix, label_id);
}

#endif
