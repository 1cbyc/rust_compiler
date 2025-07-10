#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "parser.h"
#include "types.h"
#include <stddef.h>
#include <stdbool.h>

// Symbol kinds
typedef enum {
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION,
    SYMBOL_STRUCT,
    SYMBOL_ENUM,
    SYMBOL_TYPE,
    SYMBOL_CONST
} SymbolKind;

// Symbol table entry
typedef struct Symbol {
    SymbolKind kind;
    char *name;
    Type *type;
    void *ast_node; // pointer to AST node
    struct Symbol *next;
} Symbol;

// Symbol table (scope)
typedef struct SymbolTable {
    Symbol **buckets;
    size_t bucket_count;
    struct SymbolTable *parent;
} SymbolTable;

// IR node kinds
typedef enum {
    IR_NOP,
    IR_ASSIGN,
    IR_BINOP,
    IR_CALL,
    IR_JUMP,
    IR_LABEL,
    IR_RETURN,
    IR_VAR,
    IR_CONST,
    IR_BLOCK
} IRKind;

typedef struct IRNode {
    IRKind kind;
    struct IRNode **children;
    size_t child_count;
    char *name;
    Type *type;
    // ... more fields as needed ...
} IRNode;

// Semantic analysis context
typedef struct SemanticContext {
    SymbolTable *symbols;
    bool had_error;
    char *error_message;
    size_t error_line;
    size_t error_column;
} SemanticContext;

// Symbol table management
SymbolTable* symbol_table_create(SymbolTable *parent);
void symbol_table_free(SymbolTable *table);
bool symbol_table_insert(SymbolTable *table, const char *name, SymbolKind kind, Type *type, void *ast_node);
Symbol* symbol_table_lookup(SymbolTable *table, const char *name);

// IR management
IRNode* irnode_create(IRKind kind, const char *name, Type *type);
void irnode_add_child(IRNode *parent, IRNode *child);
void irnode_free(IRNode *node);

// Semantic analysis
SemanticContext* semantic_context_create(void);
void semantic_context_free(SemanticContext *ctx);
void semantic_context_error(SemanticContext *ctx, const char *message, size_t line, size_t column);

bool semantic_analyze(SemanticContext *ctx, ASTNode *program, IRNode **ir_out);

#endif // SEMANTIC_H 