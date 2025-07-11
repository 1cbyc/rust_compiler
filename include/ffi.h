#ifndef FFI_H
#define FFI_H

#include "parser.h"
#include "types.h"
#include "semantic.h"
#include <stddef.h>
#include <stdbool.h>

// calling conventions
typedef enum {
    FFI_CDECL,
    FFI_STDCALL,
    FFI_FASTCALL,
    FFI_SYSTEMV,
    FFI_WIN64
} FFICallingConvention;

// external function declaration
typedef struct ExternalFunction {
    char *name;
    char *library_name;
    Type *return_type;
    Type **parameter_types;
    size_t parameter_count;
    FFICallingConvention calling_convention;
    bool is_variadic;
} ExternalFunction;

// ffi block ast node
typedef struct FFIBlock {
    ASTNode base;
    char *library_name;
    ExternalFunction **functions;
    size_t function_count;
    FFICallingConvention default_convention;
} FFIBlock;

// ffi expression ast node
typedef struct FFIExpr {
    ASTNode base;
    char *function_name;
    char *library_name;
    ASTNode **arguments;
    size_t argument_count;
    FFICallingConvention calling_convention;
} FFIExpr;

// ffi context for analysis
typedef struct FFIContext {
    char **loaded_libraries;
    size_t library_count;
    ExternalFunction **external_functions;
    size_t function_count;
    FFICallingConvention default_convention;
} FFIContext;

// ffi block management
FFIBlock* ffi_block_create(const char *library_name, FFICallingConvention convention);
void ffi_block_free(FFIBlock *block);
bool ffi_block_add_function(FFIBlock *block, ExternalFunction *func);

// external function management
ExternalFunction* external_function_create(const char *name, const char *library, Type *return_type, Type **params, size_t param_count, FFICallingConvention convention);
void external_function_free(ExternalFunction *func);
bool external_function_validate(ExternalFunction *func);

// ffi expression parsing
FFIExpr* parse_ffi_expression(Parser *parser);
FFIBlock* parse_ffi_block(Parser *parser);
ExternalFunction* parse_external_function(Parser *parser);

// ffi semantic analysis
bool analyze_ffi_block(SemanticContext *ctx, FFIBlock *block);
bool analyze_ffi_expression(SemanticContext *ctx, FFIExpr *expr, Type **type_out);
bool check_ffi_compatibility(SemanticContext *ctx, ExternalFunction *func);

// ffi code generation
bool generate_ffi_block_code(CodegenContext *ctx, FFIBlock *block);
bool generate_ffi_expression_code(CodegenContext *ctx, FFIExpr *expr);
bool generate_external_function_call(CodegenContext *ctx, ExternalFunction *func, ASTNode **args, size_t arg_count);

// ffi context management
FFIContext* ffi_context_create(FFICallingConvention default_convention);
void ffi_context_free(FFIContext *ctx);
bool ffi_context_add_library(FFIContext *ctx, const char *library_name);
bool ffi_context_add_function(FFIContext *ctx, ExternalFunction *func);

// ffi optimization
bool optimize_ffi_calls(FFIExpr *expr);
bool eliminate_unused_ffi_functions(FFIBlock *block);
bool optimize_calling_conventions(FFIContext *ctx);

// ffi testing utilities
bool test_ffi_parsing(const char *source);
bool test_ffi_type_checking(const char *source);
bool test_ffi_code_generation(const char *source);

// built-in ffi types
Type* create_ffi_type(const char *c_type);
Type* create_ffi_pointer_type(Type *pointee_type);
Type* create_ffi_array_type(Type *element_type, size_t size);

// ffi utilities
bool is_ffi_function(const char *name);
bool is_compatible_calling_convention(FFICallingConvention rust_conv, FFICallingConvention c_conv);
bool is_ffi_safe_type(Type *type);

// ffi safety checks
bool check_ffi_function_safety(const char *function_name);
bool check_ffi_type_compatibility(Type *rust_type, Type *c_type);
bool check_ffi_memory_safety(ExternalFunction *func);

// ffi code generation utilities
bool generate_ffi_runtime_checks(CodegenContext *ctx);
bool generate_calling_convention_wrapper(CodegenContext *ctx, FFICallingConvention convention);
bool generate_ffi_error_handling(CodegenContext *ctx);

#endif // FFI_H 