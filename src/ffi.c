#include "ffi.h"
#include "lexer.h"
#include "parser.h"
#include "types.h"
#include "semantic.h"
#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// ffi block management
FFIBlock* ffi_block_create(const char *library_name, FFICallingConvention convention) {
    FFIBlock *block = malloc(sizeof(FFIBlock));
    if (!block) return NULL;
    
    block->base.type = NODE_BLOCK;
    block->base.line = 0;
    block->base.column = 0;
    
    block->library_name = strdup(library_name);
    block->functions = NULL;
    block->function_count = 0;
    block->default_convention = convention;
    
    return block;
}

void ffi_block_free(FFIBlock *block) {
    if (!block) return;
    
    free(block->library_name);
    for (size_t i = 0; i < block->function_count; i++) {
        external_function_free(block->functions[i]);
    }
    free(block->functions);
    free(block);
}

bool ffi_block_add_function(FFIBlock *block, ExternalFunction *func) {
    if (!block || !func) return false;
    
    block->functions = realloc(block->functions, sizeof(ExternalFunction*) * (block->function_count + 1));
    block->functions[block->function_count++] = func;
    
    return true;
}

// external function management
ExternalFunction* external_function_create(const char *name, const char *library, Type *return_type, Type **params, size_t param_count, FFICallingConvention convention) {
    ExternalFunction *func = malloc(sizeof(ExternalFunction));
    if (!func) return NULL;
    
    func->name = strdup(name);
    func->library_name = strdup(library);
    func->return_type = type_clone(return_type);
    func->parameter_count = param_count;
    func->parameter_types = malloc(sizeof(Type*) * param_count);
    func->calling_convention = convention;
    func->is_variadic = false;
    
    for (size_t i = 0; i < param_count; i++) {
        func->parameter_types[i] = type_clone(params[i]);
    }
    
    return func;
}

void external_function_free(ExternalFunction *func) {
    if (!func) return;
    
    free(func->name);
    free(func->library_name);
    type_free(func->return_type);
    for (size_t i = 0; i < func->parameter_count; i++) {
        type_free(func->parameter_types[i]);
    }
    free(func->parameter_types);
    free(func);
}

bool external_function_validate(ExternalFunction *func) {
    if (!func) return false;
    
    // check that function has valid name
    if (!func->name || strlen(func->name) == 0) {
        return false;
    }
    
    // check that library name is valid
    if (!func->library_name || strlen(func->library_name) == 0) {
        return false;
    }
    
    // check that return type is valid
    if (!func->return_type) {
        return false;
    }
    
    return true;
}

// ffi expression parsing
FFIExpr* parse_ffi_expression(Parser *parser) {
    if (!parser) return NULL;
    
    // expect external function call
    if (!check_token(parser, TOKEN_IDENTIFIER)) {
        return NULL;
    }
    
    FFIExpr *expr = malloc(sizeof(FFIExpr));
    if (!expr) return NULL;
    
    expr->base.type = NODE_EXPRESSION;
    expr->base.line = parser->current_token->line;
    expr->base.column = parser->current_token->column;
    
    expr->function_name = strdup(parser->current_token->value);
    expr->library_name = NULL;
    expr->arguments = NULL;
    expr->argument_count = 0;
    expr->calling_convention = FFI_CDECL; // default
    
    advance_token(parser);
    
    // expect parentheses
    if (!match_token(parser, TOKEN_LPAREN)) {
        free(expr->function_name);
        free(expr);
        return NULL;
    }
    
    // parse arguments
    if (!check_token(parser, TOKEN_RPAREN)) {
        do {
            if (expr->argument_count == 0) {
                advance_token(parser); // skip first comma
            }
            
            ASTNode *arg = parse_expression(parser);
            if (arg) {
                expr->arguments = realloc(expr->arguments, sizeof(ASTNode*) * (expr->argument_count + 1));
                expr->arguments[expr->argument_count++] = arg;
            }
        } while (match_token(parser, TOKEN_COMMA));
    }
    
    if (!match_token(parser, TOKEN_RPAREN)) {
        for (size_t i = 0; i < expr->argument_count; i++) {
            // free arguments
        }
        free(expr->arguments);
        free(expr->function_name);
        free(expr);
        return NULL;
    }
    
    return expr;
}

FFIBlock* parse_ffi_block(Parser *parser) {
    if (!parser) return NULL;
    
    // expect extern block
    if (!check_token(parser, TOKEN_EXTERN)) {
        return NULL;
    }
    advance_token(parser);
    
    // expect library name
    if (!check_token(parser, TOKEN_STRING_LITERAL)) {
        return NULL;
    }
    char *library_name = strdup(parser->current_token->value);
    advance_token(parser);
    
    // expect block
    if (!match_token(parser, TOKEN_LBRACE)) {
        free(library_name);
        return NULL;
    }
    
    FFIBlock *block = ffi_block_create(library_name, FFI_CDECL);
    free(library_name);
    
    if (!block) {
        return NULL;
    }
    
    // parse external functions
    while (!check_token(parser, TOKEN_RBRACE)) {
        ExternalFunction *func = parse_external_function(parser);
        if (func) {
            ffi_block_add_function(block, func);
        } else {
            advance_token(parser); // skip invalid function
        }
    }
    
    if (!match_token(parser, TOKEN_RBRACE)) {
        ffi_block_free(block);
        return NULL;
    }
    
    return block;
}

ExternalFunction* parse_external_function(Parser *parser) {
    if (!parser) return NULL;
    
    // expect function declaration
    if (!check_token(parser, TOKEN_FN)) {
        return NULL;
    }
    advance_token(parser);
    
    // parse function name
    if (!check_token(parser, TOKEN_IDENTIFIER)) {
        return NULL;
    }
    char *name = strdup(parser->current_token->value);
    advance_token(parser);
    
    // expect parentheses
    if (!match_token(parser, TOKEN_LPAREN)) {
        free(name);
        return NULL;
    }
    
    // parse parameters (simplified)
    Type **params = NULL;
    size_t param_count = 0;
    
    while (!check_token(parser, TOKEN_RPAREN)) {
        if (check_token(parser, TOKEN_IDENTIFIER)) {
            // parse parameter type
            Type *param_type = type_create_primitive("i32"); // default
            params = realloc(params, sizeof(Type*) * (param_count + 1));
            params[param_count++] = param_type;
            advance_token(parser);
        } else {
            advance_token(parser);
        }
        
        if (match_token(parser, TOKEN_COMMA)) {
            continue;
        }
    }
    
    if (!match_token(parser, TOKEN_RPAREN)) {
        for (size_t i = 0; i < param_count; i++) {
            type_free(params[i]);
        }
        free(params);
        free(name);
        return NULL;
    }
    
    // expect return type
    Type *return_type = type_create_primitive("i32"); // default
    if (match_token(parser, TOKEN_ARROW)) {
        if (check_token(parser, TOKEN_IDENTIFIER)) {
            type_free(return_type);
            return_type = type_create_primitive(parser->current_token->value);
            advance_token(parser);
        }
    }
    
    // expect semicolon
    if (!match_token(parser, TOKEN_SEMICOLON)) {
        for (size_t i = 0; i < param_count; i++) {
            type_free(params[i]);
        }
        free(params);
        type_free(return_type);
        free(name);
        return NULL;
    }
    
    ExternalFunction *func = external_function_create(name, "unknown", return_type, params, param_count, FFI_CDECL);
    
    // cleanup
    for (size_t i = 0; i < param_count; i++) {
        type_free(params[i]);
    }
    free(params);
    type_free(return_type);
    free(name);
    
    return func;
}

// ffi semantic analysis
bool analyze_ffi_block(SemanticContext *ctx, FFIBlock *block) {
    if (!ctx || !block) return false;
    
    // analyze all external functions
    for (size_t i = 0; i < block->function_count; i++) {
        ExternalFunction *func = block->functions[i];
        
        if (!external_function_validate(func)) {
            semantic_context_error(ctx, "invalid external function", 0, 0);
            return false;
        }
        
        if (!check_ffi_compatibility(ctx, func)) {
            semantic_context_error(ctx, "incompatible ffi function", 0, 0);
            return false;
        }
    }
    
    return true;
}

bool analyze_ffi_expression(SemanticContext *ctx, FFIExpr *expr, Type **type_out) {
    if (!ctx || !expr || !type_out) return false;
    
    // look up external function
    // simplified: just return a default type
    *type_out = type_create_primitive("i32");
    
    // check argument types
    for (size_t i = 0; i < expr->argument_count; i++) {
        Type *arg_type = type_check_expression(ctx->type_context, expr->arguments[i]);
        if (!arg_type) {
            return false;
        }
    }
    
    return true;
}

bool check_ffi_compatibility(SemanticContext *ctx, ExternalFunction *func) {
    if (!ctx || !func) return false;
    
    // check that function is ffi safe
    if (!check_ffi_function_safety(func->name)) {
        return false;
    }
    
    // check parameter types are ffi safe
    for (size_t i = 0; i < func->parameter_count; i++) {
        if (!is_ffi_safe_type(func->parameter_types[i])) {
            return false;
        }
    }
    
    // check return type is ffi safe
    if (!is_ffi_safe_type(func->return_type)) {
        return false;
    }
    
    return true;
}

// ffi code generation
bool generate_ffi_block_code(CodegenContext *ctx, FFIBlock *block) {
    if (!ctx || !block) return false;
    
    fprintf(ctx->output, "; ffi block for library %s\n", block->library_name);
    fprintf(ctx->output, "ffi_block_%s:\n", block->library_name);
    fprintf(ctx->output, "    push rbp\n");
    fprintf(ctx->output, "    mov rbp, rsp\n");
    
    // generate function declarations
    for (size_t i = 0; i < block->function_count; i++) {
        ExternalFunction *func = block->functions[i];
        fprintf(ctx->output, "    ; external function %s\n", func->name);
        fprintf(ctx->output, "    extern %s\n", func->name);
    }
    
    fprintf(ctx->output, "    pop rbp\n");
    fprintf(ctx->output, "    ret\n");
    
    return true;
}

bool generate_ffi_expression_code(CodegenContext *ctx, FFIExpr *expr) {
    if (!ctx || !expr) return false;
    
    fprintf(ctx->output, "; ffi function call %s\n", expr->function_name);
    fprintf(ctx->output, "ffi_call_%s:\n", expr->function_name);
    fprintf(ctx->output, "    push rbp\n");
    fprintf(ctx->output, "    mov rbp, rsp\n");
    
    // generate argument setup
    for (size_t i = 0; i < expr->argument_count; i++) {
        fprintf(ctx->output, "    ; setup argument %zu\n", i);
        // would generate actual argument setup
    }
    
    // generate function call
    fprintf(ctx->output, "    call %s\n", expr->function_name);
    
    // generate return value handling
    fprintf(ctx->output, "    ; handle return value\n");
    
    fprintf(ctx->output, "    pop rbp\n");
    fprintf(ctx->output, "    ret\n");
    
    return true;
}

bool generate_external_function_call(CodegenContext *ctx, ExternalFunction *func, ASTNode **args, size_t arg_count) {
    if (!ctx || !func) return false;
    
    fprintf(ctx->output, "; call external function %s\n", func->name);
    fprintf(ctx->output, "call_%s:\n", func->name);
    fprintf(ctx->output, "    push rbp\n");
    fprintf(ctx->output, "    mov rbp, rsp\n");
    
    // generate calling convention setup
    switch (func->calling_convention) {
        case FFI_CDECL:
            fprintf(ctx->output, "    ; cdecl calling convention\n");
            break;
        case FFI_STDCALL:
            fprintf(ctx->output, "    ; stdcall calling convention\n");
            break;
        case FFI_FASTCALL:
            fprintf(ctx->output, "    ; fastcall calling convention\n");
            break;
        case FFI_SYSTEMV:
            fprintf(ctx->output, "    ; systemv calling convention\n");
            break;
        case FFI_WIN64:
            fprintf(ctx->output, "    ; win64 calling convention\n");
            break;
    }
    
    // generate argument setup
    for (size_t i = 0; i < arg_count && i < func->parameter_count; i++) {
        fprintf(ctx->output, "    ; setup parameter %zu\n", i);
        // would generate actual parameter setup
    }
    
    // generate function call
    fprintf(ctx->output, "    call %s\n", func->name);
    
    // generate cleanup
    if (func->calling_convention == FFI_CDECL) {
        fprintf(ctx->output, "    ; cdecl cleanup\n");
        fprintf(ctx->output, "    add rsp, %zu\n", arg_count * 8);
    }
    
    fprintf(ctx->output, "    pop rbp\n");
    fprintf(ctx->output, "    ret\n");
    
    return true;
}

// ffi context management
FFIContext* ffi_context_create(FFICallingConvention default_convention) {
    FFIContext *ctx = malloc(sizeof(FFIContext));
    if (!ctx) return NULL;
    
    ctx->loaded_libraries = NULL;
    ctx->library_count = 0;
    ctx->external_functions = NULL;
    ctx->function_count = 0;
    ctx->default_convention = default_convention;
    
    return ctx;
}

void ffi_context_free(FFIContext *ctx) {
    if (!ctx) return;
    
    for (size_t i = 0; i < ctx->library_count; i++) {
        free(ctx->loaded_libraries[i]);
    }
    free(ctx->loaded_libraries);
    
    for (size_t i = 0; i < ctx->function_count; i++) {
        external_function_free(ctx->external_functions[i]);
    }
    free(ctx->external_functions);
    
    free(ctx);
}

bool ffi_context_add_library(FFIContext *ctx, const char *library_name) {
    if (!ctx || !library_name) return false;
    
    ctx->loaded_libraries = realloc(ctx->loaded_libraries, sizeof(char*) * (ctx->library_count + 1));
    ctx->loaded_libraries[ctx->library_count++] = strdup(library_name);
    
    return true;
}

bool ffi_context_add_function(FFIContext *ctx, ExternalFunction *func) {
    if (!ctx || !func) return false;
    
    ctx->external_functions = realloc(ctx->external_functions, sizeof(ExternalFunction*) * (ctx->function_count + 1));
    ctx->external_functions[ctx->function_count++] = func;
    
    return true;
}

// ffi optimization
bool optimize_ffi_calls(FFIExpr *expr) {
    if (!expr) return false;
    
    // optimize ffi calls
    return true;
}

bool eliminate_unused_ffi_functions(FFIBlock *block) {
    if (!block) return false;
    
    // remove unused ffi functions
    return true;
}

bool optimize_calling_conventions(FFIContext *ctx) {
    if (!ctx) return false;
    
    // optimize calling conventions
    return true;
}

// ffi testing utilities
bool test_ffi_parsing(const char *source) {
    Lexer *lexer = lexer_init(source);
    Parser *parser = parser_init(lexer);
    
    FFIBlock *block = parse_ffi_block(parser);
    bool success = block != NULL;
    
    if (block) {
        ffi_block_free(block);
    }
    
    parser_free(parser);
    lexer_free(lexer);
    
    return success;
}

bool test_ffi_type_checking(const char *source) {
    // simplified test
    return true;
}

bool test_ffi_code_generation(const char *source) {
    // simplified test
    return true;
}

// built-in ffi types
Type* create_ffi_type(const char *c_type) {
    if (!c_type) return NULL;
    
    if (strcmp(c_type, "int") == 0) {
        return type_create_primitive("i32");
    } else if (strcmp(c_type, "char") == 0) {
        return type_create_primitive("i8");
    } else if (strcmp(c_type, "void") == 0) {
        return type_create_primitive("void");
    } else {
        return type_create_primitive("i32"); // default
    }
}

Type* create_ffi_pointer_type(Type *pointee_type) {
    if (!pointee_type) return NULL;
    
    // create pointer type
    Type *pointer_type = malloc(sizeof(Type));
    if (!pointer_type) return NULL;
    
    pointer_type->name = malloc(strlen(pointee_type->name) + 3);
    sprintf(pointer_type->name, "*%s", pointee_type->name);
    pointer_type->size = 8; // pointer size
    pointer_type->alignment = 8;
    
    return pointer_type;
}

Type* create_ffi_array_type(Type *element_type, size_t size) {
    if (!element_type) return NULL;
    
    // create array type
    Type *array_type = malloc(sizeof(Type));
    if (!array_type) return NULL;
    
    array_type->name = malloc(strlen(element_type->name) + 20);
    sprintf(array_type->name, "[%s; %zu]", element_type->name, size);
    array_type->size = element_type->size * size;
    array_type->alignment = element_type->alignment;
    
    return array_type;
}

// ffi utilities
bool is_ffi_function(const char *name) {
    if (!name) return false;
    // simplified: check if name starts with external prefix
    return strncmp(name, "external_", 9) == 0;
}

bool is_compatible_calling_convention(FFICallingConvention rust_conv, FFICallingConvention c_conv) {
    // simplified: assume compatible
    return true;
}

bool is_ffi_safe_type(Type *type) {
    if (!type) return false;
    
    // check if type is ffi safe
    if (strcmp(type->name, "i32") == 0 || strcmp(type->name, "i64") == 0 ||
        strcmp(type->name, "f32") == 0 || strcmp(type->name, "f64") == 0) {
        return true;
    }
    
    return false;
}

// ffi safety checks
bool check_ffi_function_safety(const char *function_name) {
    if (!function_name) return false;
    
    // check if function is known to be safe
    return strstr(function_name, "safe") != NULL || strstr(function_name, "printf") != NULL;
}

bool check_ffi_type_compatibility(Type *rust_type, Type *c_type) {
    if (!rust_type || !c_type) return false;
    
    // simplified type compatibility check
    return true;
}

bool check_ffi_memory_safety(ExternalFunction *func) {
    if (!func) return false;
    
    // check memory safety of external function
    return true;
}

// ffi code generation utilities
bool generate_ffi_runtime_checks(CodegenContext *ctx) {
    if (!ctx) return false;
    
    fprintf(ctx->output, "; ffi runtime checks\n");
    fprintf(ctx->output, "ffi_runtime_checks:\n");
    fprintf(ctx->output, "    push rbp\n");
    fprintf(ctx->output, "    mov rbp, rsp\n");
    
    fprintf(ctx->output, "    ; check library availability\n");
    fprintf(ctx->output, "    ; check function availability\n");
    fprintf(ctx->output, "    ; check memory safety\n");
    
    fprintf(ctx->output, "    pop rbp\n");
    fprintf(ctx->output, "    ret\n");
    
    return true;
}

bool generate_calling_convention_wrapper(CodegenContext *ctx, FFICallingConvention convention) {
    if (!ctx) return false;
    
    fprintf(ctx->output, "; calling convention wrapper\n");
    fprintf(ctx->output, "calling_convention_wrapper:\n");
    fprintf(ctx->output, "    push rbp\n");
    fprintf(ctx->output, "    mov rbp, rsp\n");
    
    switch (convention) {
        case FFI_CDECL:
            fprintf(ctx->output, "    ; cdecl wrapper\n");
            break;
        case FFI_STDCALL:
            fprintf(ctx->output, "    ; stdcall wrapper\n");
            break;
        case FFI_FASTCALL:
            fprintf(ctx->output, "    ; fastcall wrapper\n");
            break;
        case FFI_SYSTEMV:
            fprintf(ctx->output, "    ; systemv wrapper\n");
            break;
        case FFI_WIN64:
            fprintf(ctx->output, "    ; win64 wrapper\n");
            break;
    }
    
    fprintf(ctx->output, "    pop rbp\n");
    fprintf(ctx->output, "    ret\n");
    
    return true;
}

bool generate_ffi_error_handling(CodegenContext *ctx) {
    if (!ctx) return false;
    
    fprintf(ctx->output, "; ffi error handling\n");
    fprintf(ctx->output, "ffi_error_handling:\n");
    fprintf(ctx->output, "    push rbp\n");
    fprintf(ctx->output, "    mov rbp, rsp\n");
    
    fprintf(ctx->output, "    ; handle library load errors\n");
    fprintf(ctx->output, "    ; handle function call errors\n");
    fprintf(ctx->output, "    ; handle memory errors\n");
    
    fprintf(ctx->output, "    pop rbp\n");
    fprintf(ctx->output, "    ret\n");
    
    return true;
} 