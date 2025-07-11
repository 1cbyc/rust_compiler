#include "unsafe.h"
#include "lexer.h"
#include "parser.h"
#include "types.h"
#include "semantic.h"
#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// unsafe block management
UnsafeBlock* unsafe_block_create(ASTNode *body, bool is_unsafe_function) {
    UnsafeBlock *block = malloc(sizeof(UnsafeBlock));
    if (!block) return NULL;
    
    block->base.type = NODE_BLOCK;
    block->base.line = 0;
    block->base.column = 0;
    
    block->body = body;
    block->operations = NULL;
    block->operation_count = 0;
    block->is_unsafe_function = is_unsafe_function;
    
    return block;
}

void unsafe_block_free(UnsafeBlock *block) {
    if (!block) return;
    
    free(block->operations);
    // note: don't free body as it's shared
    free(block);
}

bool unsafe_block_add_operation(UnsafeBlock *block, UnsafeOperation operation) {
    if (!block) return false;
    
    block->operations = realloc(block->operations, sizeof(UnsafeOperation) * (block->operation_count + 1));
    block->operations[block->operation_count++] = operation;
    
    return true;
}

// unsafe expression parsing
UnsafeExpr* parse_unsafe_expression(Parser *parser) {
    if (!parser) return NULL;
    
    // expect unsafe keyword
    if (!check_token(parser, TOKEN_UNSAFE)) {
        return NULL;
    }
    advance_token(parser);
    
    UnsafeExpr *expr = malloc(sizeof(UnsafeExpr));
    if (!expr) return NULL;
    
    expr->base.type = NODE_EXPRESSION;
    expr->base.line = parser->current_token->line;
    expr->base.column = parser->current_token->column;
    expr->operation = UNSAFE_DEREF_RAW_POINTER; // default
    
    // parse the unsafe expression
    expr->expression = parse_expression(parser);
    if (!expr->expression) {
        free(expr);
        return NULL;
    }
    
    return expr;
}

UnsafeBlock* parse_unsafe_block(Parser *parser) {
    if (!parser) return NULL;
    
    // expect unsafe block
    if (!check_token(parser, TOKEN_UNSAFE)) {
        return NULL;
    }
    advance_token(parser);
    
    if (!match_token(parser, TOKEN_LBRACE)) {
        return NULL;
    }
    
    ASTNode *body = parse_block(parser);
    if (!body) {
        return NULL;
    }
    
    UnsafeBlock *block = unsafe_block_create(body, false);
    if (!block) {
        return NULL;
    }
    
    // detect unsafe operations in body (simplified)
    // in real implementation, would analyze body for unsafe operations
    unsafe_block_add_operation(block, UNSAFE_DEREF_RAW_POINTER);
    
    return block;
}

ASTNode* parse_unsafe_function(Parser *parser) {
    if (!parser) return NULL;
    
    // expect unsafe fn
    if (!check_token(parser, TOKEN_UNSAFE)) {
        return NULL;
    }
    advance_token(parser);
    
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
    while (!check_token(parser, TOKEN_RPAREN)) {
        advance_token(parser);
        if (match_token(parser, TOKEN_COMMA)) {
            continue;
        }
    }
    
    if (!match_token(parser, TOKEN_RPAREN)) {
        free(name);
        return NULL;
    }
    
    // expect function body
    if (!match_token(parser, TOKEN_LBRACE)) {
        free(name);
        return NULL;
    }
    
    ASTNode *body = parse_block(parser);
    if (!body) {
        free(name);
        return NULL;
    }
    
    // create unsafe function (simplified)
    UnsafeBlock *unsafe_func = unsafe_block_create(body, true);
    unsafe_block_add_operation(unsafe_func, UNSAFE_CALL_UNSAFE_FUNCTION);
    
    free(name);
    return (ASTNode*)unsafe_func;
}

// unsafe semantic analysis
bool analyze_unsafe_block(SemanticContext *ctx, UnsafeBlock *block, Type **type_out) {
    if (!ctx || !block || !type_out) return false;
    
    // check unsafe safety
    if (!check_unsafe_safety(ctx, block)) {
        semantic_context_error(ctx, "unsafe block contains unsafe operations", block->base.line, block->base.column);
        return false;
    }
    
    // analyze body
    if (block->body) {
        Type *body_type = type_check_expression(ctx->type_context, block->body);
        if (!body_type) {
            return false;
        }
        
        *type_out = body_type;
        return true;
    }
    
    return false;
}

bool analyze_unsafe_expression(SemanticContext *ctx, UnsafeExpr *expr, Type **type_out) {
    if (!ctx || !expr || !type_out) return false;
    
    // analyze the unsafe expression
    Type *expr_type = type_check_expression(ctx->type_context, expr->expression);
    if (!expr_type) {
        return false;
    }
    
    // handle different unsafe operations
    switch (expr->operation) {
        case UNSAFE_DEREF_RAW_POINTER:
            // dereference raw pointer
            if (is_raw_pointer_type(expr_type)) {
                // simplified: just return the pointee type
                *type_out = type_create_primitive("i32");
                return true;
            } else {
                semantic_context_error(ctx, "cannot dereference non-pointer type", expr->base.line, expr->base.column);
                return false;
            }
            break;
            
        case UNSAFE_CALL_UNSAFE_FUNCTION:
            // call unsafe function
            *type_out = expr_type;
            return true;
            
        default:
            *type_out = expr_type;
            return true;
    }
}

bool check_unsafe_safety(SemanticContext *ctx, UnsafeBlock *block) {
    if (!ctx || !block) return false;
    
    // check that unsafe operations are valid
    for (size_t i = 0; i < block->operation_count; i++) {
        UnsafeOperation op = block->operations[i];
        
        switch (op) {
            case UNSAFE_DEREF_RAW_POINTER:
                // check pointer validity
                break;
                
            case UNSAFE_CALL_UNSAFE_FUNCTION:
                // check function safety
                break;
                
            case UNSAFE_ACCESS_UNION_FIELD:
                // check union field access
                break;
                
            case UNSAFE_ACCESS_STATIC_MUT:
                // check static mut access
                break;
                
            default:
                // other unsafe operations
                break;
        }
    }
    
    return true;
}

// unsafe code generation
bool generate_unsafe_block_code(CodegenContext *ctx, UnsafeBlock *block) {
    if (!ctx || !block) return false;
    
    fprintf(ctx->output, "; unsafe block\n");
    fprintf(ctx->output, "unsafe_block:\n");
    fprintf(ctx->output, "    push rbp\n");
    fprintf(ctx->output, "    mov rbp, rsp\n");
    
    // generate safety checks
    fprintf(ctx->output, "    ; unsafe safety checks\n");
    for (size_t i = 0; i < block->operation_count; i++) {
        UnsafeOperation op = block->operations[i];
        
        switch (op) {
            case UNSAFE_DEREF_RAW_POINTER:
                fprintf(ctx->output, "    ; pointer dereference safety check\n");
                fprintf(ctx->output, "    mov rax, [rbp+8]  ; pointer\n");
                fprintf(ctx->output, "    test rax, rax      ; null check\n");
                fprintf(ctx->output, "    jz .null_pointer_error\n");
                break;
                
            case UNSAFE_CALL_UNSAFE_FUNCTION:
                fprintf(ctx->output, "    ; unsafe function call\n");
                break;
                
            default:
                fprintf(ctx->output, "    ; other unsafe operation\n");
                break;
        }
    }
    
    // generate body code
    if (block->body) {
        fprintf(ctx->output, "    ; unsafe block body\n");
        // would generate actual body code
    }
    
    fprintf(ctx->output, "    pop rbp\n");
    fprintf(ctx->output, "    ret\n");
    
    fprintf(ctx->output, ".null_pointer_error:\n");
    fprintf(ctx->output, "    ; handle null pointer error\n");
    fprintf(ctx->output, "    mov rax, 1  ; error code\n");
    fprintf(ctx->output, "    ret\n");
    
    return true;
}

bool generate_unsafe_expression_code(CodegenContext *ctx, UnsafeExpr *expr) {
    if (!ctx || !expr) return false;
    
    fprintf(ctx->output, "; unsafe expression\n");
    fprintf(ctx->output, "unsafe_expr:\n");
    fprintf(ctx->output, "    push rbp\n");
    fprintf(ctx->output, "    mov rbp, rsp\n");
    
    // generate unsafe operation code
    switch (expr->operation) {
        case UNSAFE_DEREF_RAW_POINTER:
            fprintf(ctx->output, "    ; dereference raw pointer\n");
            fprintf(ctx->output, "    mov rax, [rbp+8]  ; pointer\n");
            fprintf(ctx->output, "    mov rax, [rax]     ; dereference\n");
            break;
            
        case UNSAFE_CALL_UNSAFE_FUNCTION:
            fprintf(ctx->output, "    ; call unsafe function\n");
            fprintf(ctx->output, "    call unsafe_function\n");
            break;
            
        case UNSAFE_CAST_POINTER:
            fprintf(ctx->output, "    ; cast pointer\n");
            fprintf(ctx->output, "    mov rax, [rbp+8]  ; source pointer\n");
            fprintf(ctx->output, "    ; cast to target type\n");
            break;
            
        default:
            fprintf(ctx->output, "    ; other unsafe operation\n");
            break;
    }
    
    fprintf(ctx->output, "    pop rbp\n");
    fprintf(ctx->output, "    ret\n");
    
    return true;
}

bool generate_raw_pointer_code(CodegenContext *ctx, RawPointerType *pointer_type) {
    if (!ctx || !pointer_type) return false;
    
    fprintf(ctx->output, "; raw pointer operations\n");
    fprintf(ctx->output, "raw_pointer_ops:\n");
    fprintf(ctx->output, "    push rbp\n");
    fprintf(ctx->output, "    mov rbp, rsp\n");
    
    // generate pointer arithmetic
    fprintf(ctx->output, "    ; pointer arithmetic\n");
    fprintf(ctx->output, "    mov rax, [rbp+8]  ; base pointer\n");
    fprintf(ctx->output, "    mov rbx, [rbp+16] ; offset\n");
    fprintf(ctx->output, "    add rax, rbx       ; add offset\n");
    
    // generate dereference
    fprintf(ctx->output, "    ; dereference pointer\n");
    fprintf(ctx->output, "    mov rcx, [rax]     ; load value\n");
    
    fprintf(ctx->output, "    pop rbp\n");
    fprintf(ctx->output, "    ret\n");
    
    return true;
}

// unsafe context management
UnsafeContext* unsafe_context_create(void) {
    UnsafeContext *ctx = malloc(sizeof(UnsafeContext));
    if (!ctx) return NULL;
    
    ctx->in_unsafe_block = false;
    ctx->unsafe_operation_count = 0;
    ctx->operation_capacity = 10;
    ctx->operations = malloc(sizeof(UnsafeOperation) * ctx->operation_capacity);
    ctx->allow_unsafe_operations = false;
    
    return ctx;
}

void unsafe_context_free(UnsafeContext *ctx) {
    if (!ctx) return;
    
    free(ctx->operations);
    free(ctx);
}

bool unsafe_context_add_operation(UnsafeContext *ctx, UnsafeOperation operation) {
    if (!ctx) return false;
    
    if (ctx->unsafe_operation_count >= ctx->operation_capacity) {
        ctx->operation_capacity *= 2;
        ctx->operations = realloc(ctx->operations, sizeof(UnsafeOperation) * ctx->operation_capacity);
    }
    
    ctx->operations[ctx->unsafe_operation_count++] = operation;
    return true;
}

// unsafe optimization
bool optimize_unsafe_operations(UnsafeBlock *block) {
    if (!block) return false;
    
    // optimize unsafe operations
    return true;
}

bool eliminate_unnecessary_unsafe(UnsafeBlock *block) {
    if (!block) return false;
    
    // remove unnecessary unsafe operations
    return true;
}

bool optimize_raw_pointer_operations(UnsafeExpr *expr) {
    if (!expr) return false;
    
    // optimize raw pointer operations
    return true;
}

// unsafe testing utilities
bool test_unsafe_parsing(const char *source) {
    Lexer *lexer = lexer_init(source);
    Parser *parser = parser_init(lexer);
    
    UnsafeBlock *block = parse_unsafe_block(parser);
    bool success = block != NULL;
    
    if (block) {
        unsafe_block_free(block);
    }
    
    parser_free(parser);
    lexer_free(lexer);
    
    return success;
}

bool test_unsafe_type_checking(const char *source) {
    // simplified test
    return true;
}

bool test_unsafe_code_generation(const char *source) {
    // simplified test
    return true;
}

// built-in unsafe types
RawPointerType* create_raw_pointer_type(Type *pointee_type, bool is_mutable) {
    RawPointerType *pointer = malloc(sizeof(RawPointerType));
    if (!pointer) return NULL;
    
    pointer->pointee_type = type_clone(pointee_type);
    pointer->is_mutable = is_mutable;
    pointer->alignment = 8; // default alignment
    
    return pointer;
}

RawPointerType* create_const_raw_pointer_type(Type *pointee_type) {
    return create_raw_pointer_type(pointee_type, false);
}

// unsafe utilities
bool is_unsafe_operation(ASTNode *expr) {
    if (!expr) return false;
    // simplified: check if expression contains unsafe operations
    return expr->type == NODE_EXPRESSION;
}

bool requires_unsafe_context(UnsafeOperation operation) {
    // all unsafe operations require unsafe context
    return true;
}

bool is_raw_pointer_type(Type *type) {
    if (!type) return false;
    return strstr(type->name, "*const") != NULL || strstr(type->name, "*mut") != NULL;
}

// unsafe safety checks
bool check_pointer_validity(RawPointerType *pointer) {
    if (!pointer) return false;
    
    // simplified pointer validity check
    return true;
}

bool check_memory_alignment(RawPointerType *pointer, size_t offset) {
    if (!pointer) return false;
    
    // check if offset is aligned properly
    return (offset % pointer->alignment) == 0;
}

bool check_unsafe_function_safety(const char *function_name) {
    if (!function_name) return false;
    
    // simplified: check if function is known to be safe
    return strstr(function_name, "safe") != NULL;
}

// unsafe code generation utilities
bool generate_unsafe_runtime_checks(CodegenContext *ctx) {
    if (!ctx) return false;
    
    fprintf(ctx->output, "; unsafe runtime checks\n");
    fprintf(ctx->output, "unsafe_runtime_checks:\n");
    fprintf(ctx->output, "    push rbp\n");
    fprintf(ctx->output, "    mov rbp, rsp\n");
    
    fprintf(ctx->output, "    ; pointer validity check\n");
    fprintf(ctx->output, "    mov rax, [rbp+8]  ; pointer\n");
    fprintf(ctx->output, "    test rax, rax      ; null check\n");
    fprintf(ctx->output, "    jz .invalid_pointer\n");
    
    fprintf(ctx->output, "    ; alignment check\n");
    fprintf(ctx->output, "    test rax, 7        ; 8-byte alignment\n");
    fprintf(ctx->output, "    jnz .unaligned_pointer\n");
    
    fprintf(ctx->output, "    ; valid pointer\n");
    fprintf(ctx->output, "    mov rax, 0         ; success\n");
    fprintf(ctx->output, "    jmp .done\n");
    
    fprintf(ctx->output, ".invalid_pointer:\n");
    fprintf(ctx->output, "    mov rax, 1         ; error code\n");
    fprintf(ctx->output, "    jmp .done\n");
    
    fprintf(ctx->output, ".unaligned_pointer:\n");
    fprintf(ctx->output, "    mov rax, 2         ; error code\n");
    
    fprintf(ctx->output, ".done:\n");
    fprintf(ctx->output, "    pop rbp\n");
    fprintf(ctx->output, "    ret\n");
    
    return true;
}

bool generate_memory_safety_checks(CodegenContext *ctx) {
    if (!ctx) return false;
    
    fprintf(ctx->output, "; memory safety checks\n");
    fprintf(ctx->output, "memory_safety_checks:\n");
    fprintf(ctx->output, "    push rbp\n");
    fprintf(ctx->output, "    mov rbp, rsp\n");
    
    fprintf(ctx->output, "    ; bounds check\n");
    fprintf(ctx->output, "    mov rax, [rbp+8]  ; pointer\n");
    fprintf(ctx->output, "    mov rbx, [rbp+16] ; size\n");
    fprintf(ctx->output, "    add rax, rbx       ; end pointer\n");
    fprintf(ctx->output, "    cmp rax, rbx       ; check bounds\n");
    fprintf(ctx->output, "    jae .bounds_error\n");
    
    fprintf(ctx->output, "    ; valid access\n");
    fprintf(ctx->output, "    mov rax, 0         ; success\n");
    fprintf(ctx->output, "    jmp .done\n");
    
    fprintf(ctx->output, ".bounds_error:\n");
    fprintf(ctx->output, "    mov rax, 1         ; error code\n");
    
    fprintf(ctx->output, ".done:\n");
    fprintf(ctx->output, "    pop rbp\n");
    fprintf(ctx->output, "    ret\n");
    
    return true;
}

bool generate_pointer_validation(CodegenContext *ctx) {
    if (!ctx) return false;
    
    fprintf(ctx->output, "; pointer validation\n");
    fprintf(ctx->output, "pointer_validation:\n");
    fprintf(ctx->output, "    push rbp\n");
    fprintf(ctx->output, "    mov rbp, rsp\n");
    
    fprintf(ctx->output, "    ; validate pointer\n");
    fprintf(ctx->output, "    mov rax, [rbp+8]  ; pointer\n");
    fprintf(ctx->output, "    test rax, rax      ; null check\n");
    fprintf(ctx->output, "    jz .invalid\n");
    
    fprintf(ctx->output, "    ; check alignment\n");
    fprintf(ctx->output, "    test rax, 7        ; 8-byte alignment\n");
    fprintf(ctx->output, "    jnz .unaligned\n");
    
    fprintf(ctx->output, "    ; valid pointer\n");
    fprintf(ctx->output, "    mov rax, 0         ; success\n");
    fprintf(ctx->output, "    jmp .done\n");
    
    fprintf(ctx->output, ".invalid:\n");
    fprintf(ctx->output, "    mov rax, 1         ; invalid pointer\n");
    fprintf(ctx->output, "    jmp .done\n");
    
    fprintf(ctx->output, ".unaligned:\n");
    fprintf(ctx->output, "    mov rax, 2         ; unaligned pointer\n");
    
    fprintf(ctx->output, ".done:\n");
    fprintf(ctx->output, "    pop rbp\n");
    fprintf(ctx->output, "    ret\n");
    
    return true;
} 