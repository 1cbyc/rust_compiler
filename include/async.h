#ifndef ASYNC_H
#define ASYNC_H

#include "parser.h"
#include "types.h"
#include "semantic.h"
#include <stddef.h>
#include <stdbool.h>

// async function state
typedef enum {
    ASYNC_STATE_PENDING,
    ASYNC_STATE_RUNNING,
    ASYNC_STATE_COMPLETED,
    ASYNC_STATE_ERROR
} AsyncState;

// future type
typedef struct FutureType {
    Type *result_type;
    bool is_async;
    struct FutureType *next;
} FutureType;

// async function ast node
typedef struct AsyncFunction {
    ASTNode base;
    char *name;
    char **parameters;
    size_t parameter_count;
    Type *return_type;
    ASTNode *body;
    bool is_async;
} AsyncFunction;

// async expression ast node
typedef struct AsyncExpr {
    ASTNode base;
    ASTNode *expression;
    bool is_await;
} AsyncExpr;

// async runtime context
typedef struct AsyncRuntime {
    size_t task_count;
    size_t max_tasks;
    void **tasks;
    AsyncState *task_states;
    void **task_results;
    size_t current_task;
} AsyncRuntime;

// async context for analysis
typedef struct AsyncContext {
    AsyncRuntime *runtime;
    size_t async_function_count;
    char **async_function_names;
    size_t name_capacity;
    bool in_async_context;
} AsyncContext;

// async function management
AsyncFunction* async_function_create(const char *name, char **params, size_t param_count, Type *return_type, ASTNode *body, bool is_async);
void async_function_free(AsyncFunction *func);
bool async_function_validate(AsyncFunction *func);

// async expression parsing
AsyncExpr* parse_async_expression(Parser *parser);
AsyncFunction* parse_async_function(Parser *parser);
ASTNode* parse_async_block(Parser *parser);

// async semantic analysis
bool analyze_async_function(SemanticContext *ctx, AsyncFunction *func, Type **type_out);
bool analyze_async_expression(SemanticContext *ctx, AsyncExpr *expr, Type **type_out);
bool check_async_context_validity(SemanticContext *ctx, AsyncFunction *func);

// async code generation
bool generate_async_function_code(CodegenContext *ctx, AsyncFunction *func);
bool generate_async_expression_code(CodegenContext *ctx, AsyncExpr *expr);
bool generate_future_code(CodegenContext *ctx, FutureType *future_type);

// async runtime management
AsyncRuntime* async_runtime_create(size_t max_tasks);
void async_runtime_free(AsyncRuntime *runtime);
bool async_runtime_add_task(AsyncRuntime *runtime, void *task);
bool async_runtime_run_tasks(AsyncRuntime *runtime);
void* async_runtime_get_result(AsyncRuntime *runtime, size_t task_id);

// async context management
AsyncContext* async_context_create(AsyncRuntime *runtime);
void async_context_free(AsyncContext *ctx);
char* async_context_generate_name(AsyncContext *ctx, const char *base);

// async optimization
bool optimize_async_functions(AsyncFunction *func);
bool eliminate_unused_async_code(AsyncFunction *func);
bool optimize_async_runtime(AsyncRuntime *runtime);

// async testing utilities
bool test_async_parsing(const char *source);
bool test_async_type_checking(const char *source);
bool test_async_code_generation(const char *source);

// built-in async types
FutureType* create_future_type(Type *result_type);
FutureType* create_async_result_type(Type *result_type);

// async utilities
bool is_async_function(const char *name);
bool is_await_expression(ASTNode *expr);
bool can_be_awaited(Type *type);

// async runtime utilities
bool generate_async_runtime_code(CodegenContext *ctx, AsyncRuntime *runtime);
bool generate_task_scheduler(CodegenContext *ctx);
bool generate_async_io_handlers(CodegenContext *ctx);

#endif // ASYNC_H 