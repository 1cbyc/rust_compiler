#ifndef UNSAFE_H
#define UNSAFE_H

#include "parser.h"
#include "types.h"
#include "semantic.h"
#include <stddef.h>
#include <stdbool.h>

// unsafe operation types
typedef enum {
    UNSAFE_DEREF_RAW_POINTER,
    UNSAFE_CALL_UNSAFE_FUNCTION,
    UNSAFE_ACCESS_UNION_FIELD,
    UNSAFE_ACCESS_STATIC_MUT,
    UNSAFE_IMPLEMENT_UNSAFE_TRAIT,
    UNSAFE_CAST_POINTER,
    UNSAFE_TRANSMUTE,
    UNSAFE_OFFSET_POINTER
} UnsafeOperation;

// unsafe block ast node
typedef struct UnsafeBlock {
    ASTNode base;
    ASTNode *body;
    UnsafeOperation *operations;
    size_t operation_count;
    bool is_unsafe_function;
} UnsafeBlock;

// unsafe expression ast node
typedef struct UnsafeExpr {
    ASTNode base;
    ASTNode *expression;
    UnsafeOperation operation;
} UnsafeExpr;

// raw pointer type
typedef struct RawPointerType {
    Type *pointee_type;
    bool is_mutable;
    size_t alignment;
} RawPointerType;

// unsafe context for analysis
typedef struct UnsafeContext {
    bool in_unsafe_block;
    size_t unsafe_operation_count;
    UnsafeOperation *operations;
    size_t operation_capacity;
    bool allow_unsafe_operations;
} UnsafeContext;

// unsafe block management
UnsafeBlock* unsafe_block_create(ASTNode *body, bool is_unsafe_function);
void unsafe_block_free(UnsafeBlock *block);
bool unsafe_block_add_operation(UnsafeBlock *block, UnsafeOperation operation);

// unsafe expression parsing
UnsafeExpr* parse_unsafe_expression(Parser *parser);
UnsafeBlock* parse_unsafe_block(Parser *parser);
ASTNode* parse_unsafe_function(Parser *parser);

// unsafe semantic analysis
bool analyze_unsafe_block(SemanticContext *ctx, UnsafeBlock *block, Type **type_out);
bool analyze_unsafe_expression(SemanticContext *ctx, UnsafeExpr *expr, Type **type_out);
bool check_unsafe_safety(SemanticContext *ctx, UnsafeBlock *block);

// unsafe code generation
bool generate_unsafe_block_code(CodegenContext *ctx, UnsafeBlock *block);
bool generate_unsafe_expression_code(CodegenContext *ctx, UnsafeExpr *expr);
bool generate_raw_pointer_code(CodegenContext *ctx, RawPointerType *pointer_type);

// unsafe context management
UnsafeContext* unsafe_context_create(void);
void unsafe_context_free(UnsafeContext *ctx);
bool unsafe_context_add_operation(UnsafeContext *ctx, UnsafeOperation operation);

// unsafe optimization
bool optimize_unsafe_operations(UnsafeBlock *block);
bool eliminate_unnecessary_unsafe(UnsafeBlock *block);
bool optimize_raw_pointer_operations(UnsafeExpr *expr);

// unsafe testing utilities
bool test_unsafe_parsing(const char *source);
bool test_unsafe_type_checking(const char *source);
bool test_unsafe_code_generation(const char *source);

// built-in unsafe types
RawPointerType* create_raw_pointer_type(Type *pointee_type, bool is_mutable);
RawPointerType* create_const_raw_pointer_type(Type *pointee_type);

// unsafe utilities
bool is_unsafe_operation(ASTNode *expr);
bool requires_unsafe_context(UnsafeOperation operation);
bool is_raw_pointer_type(Type *type);

// unsafe safety checks
bool check_pointer_validity(RawPointerType *pointer);
bool check_memory_alignment(RawPointerType *pointer, size_t offset);
bool check_unsafe_function_safety(const char *function_name);

// unsafe code generation utilities
bool generate_unsafe_runtime_checks(CodegenContext *ctx);
bool generate_memory_safety_checks(CodegenContext *ctx);
bool generate_pointer_validation(CodegenContext *ctx);

#endif // UNSAFE_H 