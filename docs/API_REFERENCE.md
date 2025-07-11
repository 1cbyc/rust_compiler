# Rust Compiler API Reference

## Overview

This document provides a comprehensive API reference for the Rust compiler implementation in C. The compiler consists of several phases that transform Rust source code into executable assembly.

## Table of Contents

1. [Lexer API](#lexer-api)
2. [Parser API](#parser-api)
3. [Type System API](#type-system-api)
4. [Semantic Analysis API](#semantic-analysis-api)
5. [Code Generation API](#code-generation-api)
6. [Optimizer API](#optimizer-api)
7. [Standard Library API](#standard-library-api)
8. [Error Handling API](#error-handling-api)

## Lexer API

### Data Structures

```c
typedef struct Token {
    TokenType type;
    char *value;
    size_t line;
    size_t column;
} Token;

typedef struct Lexer {
    const char *source;
    size_t position;
    size_t line;
    size_t column;
    char *filename;
} Lexer;
```

### Functions

#### `Lexer* lexer_init(const char *source)`
Initializes a new lexer with the given source code.

**Parameters:**
- `source`: The source code to tokenize

**Returns:** A pointer to the initialized lexer, or NULL on failure

#### `Token* get_next_token(Lexer *lexer)`
Retrieves the next token from the source code.

**Parameters:**
- `lexer`: The lexer instance

**Returns:** A pointer to the next token, or NULL on error

#### `void lexer_free(Lexer *lexer)`
Frees the lexer and all associated resources.

**Parameters:**
- `lexer`: The lexer to free

### Token Types

```c
typedef enum TokenType {
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_INTEGER_LITERAL,
    TOKEN_FLOAT_LITERAL,
    TOKEN_STRING_LITERAL,
    TOKEN_CHAR_LITERAL,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_LET,
    TOKEN_MUT,
    TOKEN_FN,
    TOKEN_RETURN,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_IN,
    TOKEN_STRUCT,
    TOKEN_ENUM,
    TOKEN_IMPL,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_PERCENT,
    TOKEN_ASSIGN,
    TOKEN_EQ,
    TOKEN_NE,
    TOKEN_LT,
    TOKEN_GT,
    TOKEN_LE,
    TOKEN_GE,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_XOR,
    TOKEN_SHL,
    TOKEN_SHR,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_SEMICOLON,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_COLON,
    TOKEN_ARROW,
    TOKEN_RANGE,
    TOKEN_RANGE_INCLUSIVE
} TokenType;
```

## Parser API

### Data Structures

```c
typedef struct Parser {
    Lexer *lexer;
    Token *current_token;
    Token *peek_token;
    bool had_error;
} Parser;

typedef struct ASTNode {
    NodeType type;
    size_t line;
    size_t column;
    union {
        struct {
            ASTNode **statements;
            size_t statement_count;
        } program;
        struct {
            char *name;
            Parameter *parameters;
            size_t param_count;
            Type *return_type;
            ASTNode *body;
        } function;
        struct {
            char *name;
            Type *type_annotation;
            ASTNode *initializer;
            bool is_mutable;
        } variable_decl;
        struct {
            ExpressionType expr_type;
            union {
                struct {
                    TokenType token_type;
                    char *value;
                } literal;
                char *identifier;
                struct {
                    BinaryOp op;
                    ASTNode *left;
                    ASTNode *right;
                } binary_op;
                struct {
                    char *function_name;
                    ASTNode **arguments;
                    size_t arg_count;
                } function_call;
            } expr;
        } expression;
        // ... other node types
    } data;
} ASTNode;
```

### Functions

#### `Parser* parser_init(Lexer *lexer)`
Initializes a new parser with the given lexer.

**Parameters:**
- `lexer`: The lexer to use for tokenization

**Returns:** A pointer to the initialized parser, or NULL on failure

#### `ASTNode* parse_program(Parser *parser)`
Parses a complete program and returns the AST.

**Parameters:**
- `parser`: The parser instance

**Returns:** A pointer to the program AST, or NULL on error

#### `void parser_free(Parser *parser)`
Frees the parser and all associated resources.

**Parameters:**
- `parser`: The parser to free

#### `void ast_node_free(ASTNode *node)`
Frees an AST node and all its children.

**Parameters:**
- `node`: The AST node to free

## Type System API

### Data Structures

```c
typedef struct Type {
    TypeKind kind;
    char *name;
    size_t size;
    struct Type **generic_params;
    size_t generic_param_count;
    struct Type *base_type;
} Type;

typedef struct TypeContext {
    Type **types;
    size_t type_count;
    size_t type_capacity;
    bool had_error;
    char *error_message;
    size_t error_line;
    size_t error_column;
} TypeContext;
```

### Functions

#### `void types_init(void)`
Initializes the type system.

#### `void types_cleanup(void)`
Cleans up the type system.

#### `TypeContext* type_context_create(void)`
Creates a new type context.

**Returns:** A pointer to the type context, or NULL on failure

#### `Type* type_check_program(TypeContext *ctx, ASTNode *program)`
Performs type checking on a program AST.

**Parameters:**
- `ctx`: The type context
- `program`: The program AST

**Returns:** The program's type, or NULL on error

#### `void type_context_free(TypeContext *ctx)`
Frees a type context.

**Parameters:**
- `ctx`: The type context to free

## Semantic Analysis API

### Data Structures

```c
typedef struct Symbol {
    SymbolKind kind;
    char *name;
    Type *type;
    void *ast_node;
    struct Symbol *next;
} Symbol;

typedef struct SymbolTable {
    Symbol **buckets;
    size_t bucket_count;
    struct SymbolTable *parent;
} SymbolTable;

typedef struct SemanticContext {
    SymbolTable *symbols;
    bool had_error;
    char *error_message;
    size_t error_line;
    size_t error_column;
    ErrorContext *error_ctx;
} SemanticContext;

typedef struct IRNode {
    IRKind kind;
    char *name;
    Type *type;
    struct IRNode **children;
    size_t child_count;
} IRNode;
```

### Functions

#### `SemanticContext* semantic_context_create(ErrorContext *error_ctx)`
Creates a new semantic analysis context.

**Parameters:**
- `error_ctx`: The error context for reporting errors

**Returns:** A pointer to the semantic context, or NULL on failure

#### `bool semantic_analyze(SemanticContext *ctx, ASTNode *program, IRNode **ir_out)`
Performs semantic analysis on a program AST.

**Parameters:**
- `ctx`: The semantic context
- `program`: The program AST
- `ir_out`: Output parameter for the generated IR

**Returns:** true on success, false on error

#### `void semantic_context_free(SemanticContext *ctx)`
Frees a semantic context.

**Parameters:**
- `ctx`: The semantic context to free

## Code Generation API

### Data Structures

```c
typedef struct CodegenContext {
    FILE *output;
    size_t label_counter;
    size_t temp_counter;
    char **strings;
    size_t string_count;
    size_t string_capacity;
} CodegenContext;
```

### Functions

#### `CodegenContext* codegen_context_create(FILE *output)`
Creates a new code generation context.

**Parameters:**
- `output`: The output file for generated assembly

**Returns:** A pointer to the codegen context, or NULL on failure

#### `bool codegen_generate(CodegenContext *ctx, IRNode *ir)`
Generates assembly code from IR.

**Parameters:**
- `ctx`: The codegen context
- `ir`: The IR to generate code for

**Returns:** true on success, false on error

#### `void codegen_context_free(CodegenContext *ctx)`
Frees a codegen context.

**Parameters:**
- `ctx`: The codegen context to free

## Optimizer API

### Data Structures

```c
typedef enum OptimizationPass {
    OPT_CONSTANT_FOLDING,
    OPT_DEAD_CODE_ELIMINATION,
    OPT_LOOP_OPTIMIZATION,
    OPT_REGISTER_ALLOCATION,
    OPT_CODE_SIZE_OPTIMIZATION
} OptimizationPass;

typedef struct OptimizerContext {
    IRNode *ir;
    bool modified;
    size_t pass_count;
    OptimizationPass *passes;
    size_t pass_capacity;
} OptimizerContext;
```

### Functions

#### `OptimizerContext* optimizer_create(IRNode *ir)`
Creates a new optimizer context.

**Parameters:**
- `ir`: The IR to optimize

**Returns:** A pointer to the optimizer context, or NULL on failure

#### `bool optimizer_add_pass(OptimizerContext *ctx, OptimizationPass pass)`
Adds an optimization pass to the optimizer.

**Parameters:**
- `ctx`: The optimizer context
- `pass`: The optimization pass to add

**Returns:** true on success, false on error

#### `bool optimizer_run_passes(OptimizerContext *ctx)`
Runs all registered optimization passes.

**Parameters:**
- `ctx`: The optimizer context

**Returns:** true on success, false on error

#### `void optimizer_free(OptimizerContext *ctx)`
Frees an optimizer context.

**Parameters:**
- `ctx`: The optimizer context to free

## Standard Library API

### Data Structures

```c
typedef struct StdLibFunction {
    char *name;
    Type **param_types;
    size_t param_count;
    Type *return_type;
    void (*native_impl)(void);
} StdLibFunction;

typedef struct StdLibContext {
    StdLibFunction **functions;
    size_t function_count;
    size_t capacity;
} StdLibContext;

typedef struct Macro {
    char *name;
    char *pattern;
    char *replacement;
} Macro;

typedef struct MacroContext {
    Macro **macros;
    size_t macro_count;
    size_t capacity;
} MacroContext;
```

### Functions

#### `StdLibContext* stdlib_create(void)`
Creates a new standard library context.

**Returns:** A pointer to the stdlib context, or NULL on failure

#### `bool stdlib_register_function(StdLibContext *ctx, const char *name, Type **param_types, size_t param_count, Type *return_type, void (*impl)(void))`
Registers a function in the standard library.

**Parameters:**
- `ctx`: The stdlib context
- `name`: Function name
- `param_types`: Parameter types
- `param_count`: Number of parameters
- `return_type`: Return type
- `impl`: Native implementation

**Returns:** true on success, false on error

#### `MacroContext* macro_context_create(void)`
Creates a new macro context.

**Returns:** A pointer to the macro context, or NULL on failure

#### `bool macro_register(MacroContext *ctx, const char *name, const char *pattern, const char *replacement)`
Registers a macro.

**Parameters:**
- `ctx`: The macro context
- `name`: Macro name
- `pattern`: Pattern to match
- `replacement`: Replacement text

**Returns:** true on success, false on error

#### `char* macro_expand(MacroContext *ctx, const char *input)`
Expands macros in input text.

**Parameters:**
- `ctx`: The macro context
- `input`: Input text

**Returns:** Expanded text, or NULL on error

## Error Handling API

### Data Structures

```c
typedef enum ErrorSeverity {
    ERROR_INFO,
    ERROR_WARNING,
    ERROR_ERROR,
    ERROR_FATAL
} ErrorSeverity;

typedef enum ErrorType {
    ERROR_LEXICAL,
    ERROR_SYNTAX,
    ERROR_TYPE,
    ERROR_SEMANTIC,
    ERROR_CODE_GEN,
    ERROR_OPTIMIZATION,
    ERROR_STDLIB
} ErrorType;

typedef struct SourceLocation {
    size_t line;
    size_t column;
    size_t offset;
    char *filename;
} SourceLocation;

typedef struct ErrorMessage {
    ErrorType type;
    ErrorSeverity severity;
    char *message;
    SourceLocation location;
    char *suggestion;
    bool recovered;
} ErrorMessage;

typedef struct ErrorContext {
    ErrorMessage **errors;
    size_t error_count;
    size_t error_capacity;
    bool had_error;
    bool had_fatal;
    size_t max_errors;
} ErrorContext;
```

### Functions

#### `ErrorContext* error_context_create(size_t max_errors)`
Creates a new error context.

**Parameters:**
- `max_errors`: Maximum number of errors to collect

**Returns:** A pointer to the error context, or NULL on failure

#### `bool error_report_lexical(ErrorContext *ctx, const char *message, size_t line, size_t column, const char *filename)`
Reports a lexical error.

**Parameters:**
- `ctx`: The error context
- `message`: Error message
- `line`: Line number
- `column`: Column number
- `filename`: Source filename

**Returns:** true on success, false on error

#### `bool error_report_syntax(ErrorContext *ctx, const char *message, size_t line, size_t column, const char *filename)`
Reports a syntax error.

**Parameters:**
- `ctx`: The error context
- `message`: Error message
- `line`: Line number
- `column`: Column number
- `filename`: Source filename

**Returns:** true on success, false on error

#### `bool error_report_type(ErrorContext *ctx, const char *message, size_t line, size_t column, const char *filename)`
Reports a type error.

**Parameters:**
- `ctx`: The error context
- `message`: Error message
- `line`: Line number
- `column`: Column number
- `filename`: Source filename

**Returns:** true on success, false on error

#### `bool error_report_semantic(ErrorContext *ctx, const char *message, size_t line, size_t column, const char *filename)`
Reports a semantic error.

**Parameters:**
- `ctx`: The error context
- `message`: Error message
- `line`: Line number
- `column`: Column number
- `filename`: Source filename

**Returns:** true on success, false on error

#### `void error_print_summary(ErrorContext *ctx, FILE *output)`
Prints an error summary.

**Parameters:**
- `ctx`: The error context
- `output`: Output file

#### `void error_print_detailed(ErrorContext *ctx, FILE *output)`
Prints detailed error information.

**Parameters:**
- `ctx`: The error context
- `output`: Output file

#### `void error_context_free(ErrorContext *ctx)`
Frees an error context.

**Parameters:**
- `ctx`: The error context to free

## Usage Examples

### Basic Compilation Pipeline

```c
#include "lexer.h"
#include "parser.h"
#include "types.h"
#include "semantic.h"
#include "codegen.h"
#include "optimizer.h"
#include "error.h"

int main() {
    const char *source = "fn main() { println!(\"Hello, world!\"); }";
    
    // Create error context
    ErrorContext *error_ctx = error_context_create(100);
    
    // Lexical analysis
    Lexer *lexer = lexer_init(source);
    if (!lexer) {
        printf("Failed to initialize lexer\n");
        return 1;
    }
    
    // Syntax analysis
    Parser *parser = parser_init(lexer);
    if (!parser) {
        printf("Failed to initialize parser\n");
        lexer_free(lexer);
        return 1;
    }
    
    ASTNode *ast = parse_program(parser);
    if (!ast) {
        printf("Failed to parse program\n");
        parser_free(parser);
        lexer_free(lexer);
        return 1;
    }
    
    // Type checking
    types_init();
    TypeContext *type_ctx = type_context_create();
    Type *program_type = type_check_program(type_ctx, ast);
    
    // Semantic analysis
    SemanticContext *sem_ctx = semantic_context_create(error_ctx);
    IRNode *ir = NULL;
    bool sem_success = semantic_analyze(sem_ctx, ast, &ir);
    
    // Optimization
    if (ir) {
        OptimizerContext *opt_ctx = optimizer_create(ir);
        optimizer_add_pass(opt_ctx, OPT_CONSTANT_FOLDING);
        optimizer_run_passes(opt_ctx);
        optimizer_free(opt_ctx);
    }
    
    // Code generation
    if (ir) {
        CodegenContext *codegen_ctx = codegen_context_create(stdout);
        codegen_generate(codegen_ctx, ir);
        codegen_context_free(codegen_ctx);
    }
    
    // Print error summary
    error_print_summary(error_ctx, stdout);
    
    // Cleanup
    if (ir) irnode_free(ir);
    semantic_context_free(sem_ctx);
    type_context_free(type_ctx);
    types_cleanup();
    ast_node_free(ast);
    parser_free(parser);
    lexer_free(lexer);
    error_context_free(error_ctx);
    
    return 0;
}
```

### Error Handling Example

```c
#include "error.h"

int main() {
    ErrorContext *error_ctx = error_context_create(100);
    
    // Report various types of errors
    error_report_lexical(error_ctx, "Unexpected character '#'", 5, 10, "test.rs");
    error_report_syntax(error_ctx, "Expected ';' after expression", 12, 15, "test.rs");
    error_report_type(error_ctx, "Type mismatch: expected i32, got String", 8, 3, "test.rs");
    error_report_semantic(error_ctx, "Variable 'x' is not defined", 20, 7, "test.rs");
    
    // Print error information
    printf("Total errors: %zu\n", error_ctx->error_count);
    error_print_summary(error_ctx, stdout);
    error_print_detailed(error_ctx, stdout);
    
    error_context_free(error_ctx);
    return 0;
}
```

## Error Codes and Messages

The compiler uses standardized error messages and codes for different types of errors:

### Lexical Errors
- Unexpected character
- Unterminated string literal
- Invalid number format

### Syntax Errors
- Expected token
- Unexpected token
- Missing semicolon
- Unmatched parentheses/braces

### Type Errors
- Type mismatch
- Undefined type
- Invalid operation for type

### Semantic Errors
- Undefined variable
- Undefined function
- Variable redefinition
- Invalid assignment

## Performance Considerations

- The lexer processes tokens incrementally
- The parser uses recursive descent with error recovery
- Type checking is performed bottom-up
- Semantic analysis builds symbol tables efficiently
- Optimization passes can be configured for different levels
- Error contexts limit memory usage by capping error count

## Memory Management

All data structures use dynamic memory allocation. It's important to free resources properly:

1. Always call the corresponding `*_free()` function for each allocated structure
2. Free child structures before parent structures
3. Use error contexts to limit memory usage during error conditions
4. Consider using memory pools for frequently allocated structures

## Thread Safety

The current implementation is not thread-safe. For multi-threaded usage:

1. Create separate contexts for each thread
2. Don't share contexts between threads
3. Consider adding mutex protection for shared resources
4. Use thread-local storage for temporary data 