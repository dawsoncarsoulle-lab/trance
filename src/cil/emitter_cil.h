#ifndef _EMITTER_CIL_H_
#define _EMITTER_CIL_H_

#include "../context.h"
#include "emitter_cil_util.h"

static inline void emit_load_int(CodeGenContext *ctx, int value) {
  fprintf(ctx->stream, "\tldc.i4\t%d\n", value);
}
static inline void emit_string_literal(CodeGenContext *ctx, const char *str) {
  fprintf(ctx->stream, "\tldstr \"%s\"\n", str);
}

#define EMIT_INDEX_OPCODE(ctx, base_opcode, index)                                                 \
  do {                                                                                             \
    if (index >= 0 && index <= 3)                                                                  \
      fprintf((ctx)->stream, "\t%s.%d\n", base_opcode, index);                                     \
    else if (index <= 255)                                                                         \
      fprintf((ctx)->stream, "\t%s.s\t%d\n", base_opcode, index);                                  \
    else                                                                                           \
      fprintf((ctx)->stream, "\t%s\t%d\n", base_opcode, index);                                    \
  } while (0)

static inline void emit_load_local(CodeGenContext *ctx, int index) {
  EMIT_INDEX_OPCODE(ctx, "ldloc", index);
}
static inline void emit_store_local(CodeGenContext *ctx, int index) {
  EMIT_INDEX_OPCODE(ctx, "stloc", index);
}

// uses inner function in emitter_cil_util.h
#define BINARY_EMITTER_DECL(operation)                                                             \
  static inline void emit_##operation(CodeGenContext *ctx) { _emit_op(ctx, #operation); }
// boolean
BINARY_EMITTER_DECL(add)
BINARY_EMITTER_DECL(sub)
BINARY_EMITTER_DECL(mul)
BINARY_EMITTER_DECL(div)
BINARY_EMITTER_DECL(ceq)
BINARY_EMITTER_DECL(cgt)
BINARY_EMITTER_DECL(clt)
BINARY_EMITTER_DECL(and)
BINARY_EMITTER_DECL(or)
#undef BINARY_EMITTER_DECL

// system utilities
static inline void emit_sys_print_int(CodeGenContext *ctx) {
  _emit_call(ctx, "void class [mscorlib]System.Console::WriteLine(int32)");
}
static inline void emit_sys_print_string(CodeGenContext *ctx) {
  _emit_call(ctx, "void class [mscorlib]System.Console::WriteLine(string)");
}
static inline void emit_sys_read_line(CodeGenContext *ctx) {
  _emit_call(ctx, "string class [mscorlib]System.Console::ReadLine()");
}
static inline void emit_sys_parse_int(CodeGenContext *ctx) {
  _emit_call(ctx, "int32 int32::Parse(string)");
}

static inline void emit_label_while(CodeGenContext *ctx, int label_id) {
  _emit_raw_label(ctx, "WHILE", label_id);
}
static inline void emit_label_else(CodeGenContext *ctx, int label_id) {
  _emit_raw_label(ctx, "ELSE", label_id);
}
static inline void emit_label_end(CodeGenContext *ctx, int label_id) {
  _emit_raw_label(ctx, "END", label_id);
}

static inline void emit_jump_while(CodeGenContext *ctx, int label_id) {
  _emit_raw_branch(ctx, "br", "WHILE", label_id);
}
static inline void emit_jump_end(CodeGenContext *ctx, int label_id) {
  _emit_raw_branch(ctx, "br", "END", label_id);
}

static inline void emit_branch_false_else(CodeGenContext *ctx, int label_id) {
  _emit_raw_branch(ctx, "brfalse", "ELSE", label_id);
}
static inline void emit_branch_false_end(CodeGenContext *ctx, int label_id) {
  _emit_raw_branch(ctx, "brfalse", "END", label_id);
}

// begin code section
static inline void emit_assembly_header(CodeGenContext *ctx, const char *name) {
  fprintf(ctx->stream, ".assembly extern mscorlib {}\n");
  fprintf(ctx->stream, ".assembly %s {}\n\n", name);
}
static inline void emit_method_main_start(CodeGenContext *ctx, const char *max_stack) {
  fprintf(ctx->stream, ".method static void Main()\n{\n");
  fprintf(ctx->stream, "    .entrypoint\n");
  fprintf(ctx->stream, "    .maxstack %s\n", max_stack);
}
static inline void emit_locals_init_start(CodeGenContext *ctx) {
  fprintf(ctx->stream, "    .locals init (\n");
}
static inline void emit_local_variable_decl(CodeGenContext *ctx, int index, const char *type,
                                            const char *name, int is_last) {
  fprintf(ctx->stream, "      [%d] %s %s%s\n", index, type, name, is_last ? "" : ",");
}
static inline void emit_locals_init_end(CodeGenContext *ctx) { fprintf(ctx->stream, "    )\n"); }

// end code section
static inline void emit_method_main_end(CodeGenContext *ctx) { fprintf(ctx->stream, "\tret\n}\n"); }

#endif // _EMITTER_CIL_H_
