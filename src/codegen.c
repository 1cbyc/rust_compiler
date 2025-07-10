#include "codegen.h"
#include <stdlib.h>
#include <stdarg.h>

CodegenContext* codegen_context_create(FILE *out) {
    CodegenContext *ctx = malloc(sizeof(CodegenContext));
    ctx->out = out;
    ctx->label_counter = 0;
    ctx->had_error = false;
    return ctx;
}

void codegen_context_free(CodegenContext *ctx) {
    if (!ctx) return;
    free(ctx);
}

void codegen_emit(CodegenContext *ctx, const char *fmt, ...) {
    if (!ctx || !ctx->out) return;
    va_list args;
    va_start(args, fmt);
    vfprintf(ctx->out, fmt, args);
    va_end(args);
}

// Forward declaration
static void codegen_ir(CodegenContext *ctx, IRNode *ir);

// Main codegen function
bool codegen_generate(CodegenContext *ctx, IRNode *ir) {
    if (!ctx || !ir) return false;
    codegen_emit(ctx, ".section .text\n.global _start\n_start:\n");
    codegen_ir(ctx, ir);
    codegen_emit(ctx, "mov rax, 60\nmov rdi, 0\nsyscall\n"); // exit(0)
    return !ctx->had_error;
}

// IR to x86-64 assembly (stub)
static void codegen_ir(CodegenContext *ctx, IRNode *ir) {
    if (!ir) return;
    switch (ir->kind) {
        case IR_BLOCK:
            for (size_t i = 0; i < ir->child_count; i++) {
                codegen_ir(ctx, ir->children[i]);
            }
            break;
        case IR_ASSIGN:
            codegen_emit(ctx, "; assign %s\n", ir->name);
            break;
        case IR_CONST:
            codegen_emit(ctx, "; const %s\n", ir->name);
            break;
        case IR_VAR:
            codegen_emit(ctx, "; var %s\n", ir->name);
            break;
        case IR_BINOP:
            codegen_emit(ctx, "; binop\n");
            break;
        case IR_CALL:
            codegen_emit(ctx, "; call %s\n", ir->name);
            break;
        case IR_RETURN:
            codegen_emit(ctx, "; return\n");
            break;
        case IR_NOP:
            break;
        default:
            codegen_emit(ctx, "; unknown IR kind\n");
            break;
    }
} 