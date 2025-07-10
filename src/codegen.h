#ifndef CODEGEN_H
#define CODEGEN_H

#include "semantic.h"
#include <stdio.h>
#include <stdbool.h>

// Codegen context
typedef struct CodegenContext {
    FILE *out;
    int label_counter;
    bool had_error;
} CodegenContext;

// Codegen API
CodegenContext* codegen_context_create(FILE *out);
void codegen_context_free(CodegenContext *ctx);
void codegen_emit(CodegenContext *ctx, const char *fmt, ...);

// Main codegen function
bool codegen_generate(CodegenContext *ctx, IRNode *ir);

#endif // CODEGEN_H 