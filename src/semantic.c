#include "semantic.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Symbol table management
SymbolTable* symbol_table_create(SymbolTable *parent) {
    SymbolTable *table = malloc(sizeof(SymbolTable));
    table->bucket_count = 64;
    table->buckets = calloc(table->bucket_count, sizeof(Symbol*));
    table->parent = parent;
    return table;
}

void symbol_table_free(SymbolTable *table) {
    if (!table) return;
    for (size_t i = 0; i < table->bucket_count; i++) {
        Symbol *sym = table->buckets[i];
        while (sym) {
            Symbol *next = sym->next;
            free(sym->name);
            free(sym);
            sym = next;
        }
    }
    free(table->buckets);
    free(table);
}

static size_t hash_symbol(const char *name, size_t bucket_count) {
    size_t hash = 5381;
    for (const char *p = name; *p; p++) {
        hash = ((hash << 5) + hash) + (unsigned char)(*p);
    }
    return hash % bucket_count;
}

bool symbol_table_insert(SymbolTable *table, const char *name, SymbolKind kind, Type *type, void *ast_node) {
    if (!table || !name) return false;
    size_t idx = hash_symbol(name, table->bucket_count);
    Symbol *sym = malloc(sizeof(Symbol));
    sym->kind = kind;
    sym->name = strdup(name);
    sym->type = type;
    sym->ast_node = ast_node;
    sym->next = table->buckets[idx];
    table->buckets[idx] = sym;
    return true;
}

Symbol* symbol_table_lookup(SymbolTable *table, const char *name) {
    for (; table; table = table->parent) {
        size_t idx = hash_symbol(name, table->bucket_count);
        for (Symbol *sym = table->buckets[idx]; sym; sym = sym->next) {
            if (strcmp(sym->name, name) == 0) {
                return sym;
            }
        }
    }
    return NULL;
}

// IR management
IRNode* irnode_create(IRKind kind, const char *name, Type *type) {
    IRNode *node = malloc(sizeof(IRNode));
    node->kind = kind;
    node->name = name ? strdup(name) : NULL;
    node->type = type;
    node->children = NULL;
    node->child_count = 0;
    return node;
}

void irnode_add_child(IRNode *parent, IRNode *child) {
    if (!parent || !child) return;
    parent->children = realloc(parent->children, sizeof(IRNode*) * (parent->child_count + 1));
    parent->children[parent->child_count++] = child;
}

void irnode_free(IRNode *node) {
    if (!node) return;
    for (size_t i = 0; i < node->child_count; i++) {
        irnode_free(node->children[i]);
    }
    free(node->children);
    free(node->name);
    free(node);
}

// Semantic analysis context
SemanticContext* semantic_context_create(void) {
    SemanticContext *ctx = malloc(sizeof(SemanticContext));
    ctx->symbols = symbol_table_create(NULL);
    ctx->had_error = false;
    ctx->error_message = NULL;
    ctx->error_line = 0;
    ctx->error_column = 0;
    return ctx;
}

void semantic_context_free(SemanticContext *ctx) {
    if (!ctx) return;
    symbol_table_free(ctx->symbols);
    free(ctx->error_message);
    free(ctx);
}

void semantic_context_error(SemanticContext *ctx, const char *message, size_t line, size_t column) {
    if (!ctx) return;
    ctx->had_error = true;
    free(ctx->error_message);
    ctx->error_message = strdup(message);
    ctx->error_line = line;
    ctx->error_column = column;
    fprintf(stderr, "semantic error at line %zu, column %zu: %s\n", line, column, message);
}

// Semantic analysis stub
bool semantic_analyze(SemanticContext *ctx, ASTNode *program, IRNode **ir_out) {
    // TODO: Implement semantic analysis and IR generation
    (void)ctx; (void)program; (void)ir_out;
    return true;
} 