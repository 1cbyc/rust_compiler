#ifndef CLOSURES_H
#define CLOSURES_H

#include "parser.h"
#include "types.h"
#include "semantic.h"
#include <stddef.h>
#include <stdbool.h>

// closure capture modes
typedef enum {
    CAPTURE_BY_VALUE,
    CAPTURE_BY_REFERENCE,
    CAPTURE_BY_MOVE
} CaptureMode;

// captured variable in closure
typedef struct CapturedVariable {
    char *name;
    Type *type;
    CaptureMode mode;
    size_t offset;  // offset in closure environment
    struct CapturedVariable *next;
} CapturedVariable;

// closure environment (captured variables)
typedef struct ClosureEnvironment {
    CapturedVariable *captures;
    size_t capture_count;
    size_t total_size;  // total size of captured variables
    struct ClosureEnvironment *parent;  // for nested closures
} ClosureEnvironment;

// closure type
typedef struct ClosureType {
    Type *return_type;
    Type **parameter_types;
    size_t parameter_count;
    ClosureEnvironment *environment;
    bool is_async;
} ClosureType;

// closure expression ast node
typedef struct ClosureExpr {
    ASTNode base;
    char **parameters;
    size_t parameter_count;
    ASTNode *body;
    ClosureEnvironment *captures;
    bool is_async;
} ClosureExpr;

// closure implementation
typedef struct ClosureImpl {
    char *name;
    ClosureType *type;
    ASTNode *body;
    ClosureEnvironment *environment;
    char *generated_function_name;
} ClosureImpl;

// closure context for analysis
typedef struct ClosureContext {
    ClosureEnvironment *current_environment;
    size_t closure_count;
    char **generated_names;
    size_t name_capacity;
} ClosureContext;

// closure environment management
ClosureEnvironment* closure_environment_create(ClosureEnvironment *parent);
void closure_environment_free(ClosureEnvironment *env);
bool closure_environment_add_capture(ClosureEnvironment *env, const char *name, Type *type, CaptureMode mode);
CapturedVariable* closure_environment_find_capture(ClosureEnvironment *env, const char *name);

// closure type management
ClosureType* closure_type_create(Type *return_type, Type **params, size_t param_count, ClosureEnvironment *env);
void closure_type_free(ClosureType *type);
bool closure_type_equals(ClosureType *a, ClosureType *b);

// closure expression parsing
ClosureExpr* parse_closure_expression(Parser *parser);
ASTNode* parse_closure_body(Parser *parser);
bool parse_closure_captures(Parser *parser, ClosureEnvironment *env);

// closure semantic analysis
bool analyze_closure_expression(SemanticContext *ctx, ClosureExpr *closure, Type **type_out);
bool analyze_closure_captures(SemanticContext *ctx, ClosureExpr *closure);
bool check_closure_capture_validity(SemanticContext *ctx, ClosureExpr *closure);

// closure code generation
bool generate_closure_code(CodegenContext *ctx, ClosureExpr *closure);
bool generate_closure_environment(CodegenContext *ctx, ClosureEnvironment *env);
bool generate_closure_function(CodegenContext *ctx, ClosureImpl *impl);

// closure context management
ClosureContext* closure_context_create(void);
void closure_context_free(ClosureContext *ctx);
char* closure_context_generate_name(ClosureContext *ctx, const char *base);

// closure optimization
bool optimize_closure_captures(ClosureExpr *closure);
bool eliminate_unused_captures(ClosureExpr *closure);
bool optimize_closure_environment(ClosureEnvironment *env);

// closure testing utilities
bool test_closure_parsing(const char *source);
bool test_closure_type_checking(const char *source);
bool test_closure_code_generation(const char *source);

#endif // CLOSURES_H 