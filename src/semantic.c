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
SemanticContext* semantic_context_create(ErrorContext *error_ctx) {
    SemanticContext *ctx = malloc(sizeof(SemanticContext));
    ctx->symbols = symbol_table_create(NULL);
    ctx->had_error = false;
    ctx->error_message = NULL;
    ctx->error_line = 0;
    ctx->error_column = 0;
    ctx->error_ctx = error_ctx;
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
    error_report_semantic(ctx->error_ctx, message, line, column);
}

// Semantic analysis functions
static IRNode* analyze_expression(SemanticContext *ctx, ASTNode *expr);
static IRNode* analyze_statement(SemanticContext *ctx, ASTNode *stmt);
static IRNode* analyze_variable_declaration(SemanticContext *ctx, ASTNode *decl);
static IRNode* analyze_function_declaration(SemanticContext *ctx, ASTNode *func);
static IRNode* analyze_struct_declaration(SemanticContext *ctx, ASTNode *struct_def);
static IRNode* analyze_enum_declaration(SemanticContext *ctx, ASTNode *enum_def);

static IRNode* analyze_expression(SemanticContext *ctx, ASTNode *expr) {
    if (!expr || expr->type != NODE_EXPRESSION) return NULL;
    
    switch (expr->data.expression.expr_type) {
        case EXPR_LITERAL: {
            IRNode *node = irnode_create(IR_CONST, expr->data.expression.expr.literal.value, NULL);
            return node;
        }
        case EXPR_IDENTIFIER: {
            char *name = expr->data.expression.expr.identifier;
            Symbol *sym = symbol_table_lookup(ctx->symbols, name);
            if (!sym) {
                semantic_context_error(ctx, "undefined variable", expr->line, expr->column);
                return NULL;
            }
            IRNode *node = irnode_create(IR_VAR, name, sym->type);
            return node;
        }
        case EXPR_BINARY_OP: {
            IRNode *left = analyze_expression(ctx, expr->data.expression.expr.binary_op.left);
            IRNode *right = analyze_expression(ctx, expr->data.expression.expr.binary_op.right);
            if (!left || !right) return NULL;
            
            IRNode *node = irnode_create(IR_BINOP, NULL, NULL);
            irnode_add_child(node, left);
            irnode_add_child(node, right);
            return node;
        }
        case EXPR_FUNCTION_CALL: {
            char *name = expr->data.expression.expr.function_call.function_name;
            Symbol *sym = symbol_table_lookup(ctx->symbols, name);
            if (!sym || sym->kind != SYMBOL_FUNCTION) {
                semantic_context_error(ctx, "undefined function", expr->line, expr->column);
                return NULL;
            }
            
            IRNode *node = irnode_create(IR_CALL, name, sym->type);
            for (size_t i = 0; i < expr->data.expression.expr.function_call.arg_count; i++) {
                IRNode *arg = analyze_expression(ctx, expr->data.expression.expr.function_call.arguments[i]);
                if (arg) irnode_add_child(node, arg);
            }
            return node;
        }
        default:
            semantic_context_error(ctx, "unsupported expression type", expr->line, expr->column);
            return NULL;
    }
}

static IRNode* analyze_variable_declaration(SemanticContext *ctx, ASTNode *decl) {
    if (!decl || decl->type != NODE_VARIABLE_DECL) return NULL;
    
    char *name = decl->data.variable_decl.name;
    Type *type = decl->data.variable_decl.type_annotation;
    ASTNode *initializer = decl->data.variable_decl.initializer;
    
    // Check for shadowing
    Symbol *existing = symbol_table_lookup(ctx->symbols, name);
    if (existing) {
        semantic_context_error(ctx, "variable shadows existing declaration", decl->line, decl->column);
    }
    
    // Insert into symbol table
    symbol_table_insert(ctx->symbols, name, SYMBOL_VARIABLE, type, decl);
    
    // Create IR node
    IRNode *node = irnode_create(IR_ASSIGN, name, type);
    if (initializer) {
        IRNode *init_node = analyze_expression(ctx, initializer);
        if (init_node) irnode_add_child(node, init_node);
    }
    
    return node;
}

static IRNode* analyze_function_declaration(SemanticContext *ctx, ASTNode *func) {
    if (!func || func->type != NODE_FUNCTION) return NULL;
    
    char *name = func->data.function.name;
    Type *return_type = func->data.function.return_type;
    
    // Check for redefinition
    Symbol *existing = symbol_table_lookup(ctx->symbols, name);
    if (existing) {
        semantic_context_error(ctx, "function redefined", func->line, func->column);
    }
    
    // Insert function into symbol table
    symbol_table_insert(ctx->symbols, name, SYMBOL_FUNCTION, return_type, func);
    
    // Create new scope for function body
    SymbolTable *old_symbols = ctx->symbols;
    ctx->symbols = symbol_table_create(old_symbols);
    
    // Add parameters to scope
    for (size_t i = 0; i < func->data.function.param_count; i++) {
        ASTNode *param = func->data.function.parameters[i];
        if (param && param->type == NODE_VARIABLE_DECL) {
            char *param_name = param->data.variable_decl.name;
            Type *param_type = param->data.variable_decl.type_annotation;
            symbol_table_insert(ctx->symbols, param_name, SYMBOL_VARIABLE, param_type, param);
        }
    }
    
    // Analyze function body
    IRNode *body_node = NULL;
    if (func->data.function.body) {
        body_node = analyze_statement(ctx, func->data.function.body);
    }
    
    // Restore scope
    ctx->symbols = old_symbols;
    
    // Create function IR node
    IRNode *node = irnode_create(IR_BLOCK, name, return_type);
    if (body_node) irnode_add_child(node, body_node);
    
    return node;
}

static IRNode* analyze_struct_declaration(SemanticContext *ctx, ASTNode *struct_def) {
    if (!struct_def || struct_def->type != NODE_STRUCT) return NULL;
    
    char *name = struct_def->data.struct_def.name;
    
    // Check for redefinition
    Symbol *existing = symbol_table_lookup(ctx->symbols, name);
    if (existing) {
        semantic_context_error(ctx, "struct redefined", struct_def->line, struct_def->column);
    }
    
    // Create struct type (simplified)
    Type *struct_type = type_create_struct(name, NULL, NULL, 0);
    symbol_table_insert(ctx->symbols, name, SYMBOL_STRUCT, struct_type, struct_def);
    
    return irnode_create(IR_NOP, name, struct_type);
}

static IRNode* analyze_enum_declaration(SemanticContext *ctx, ASTNode *enum_def) {
    if (!enum_def || enum_def->type != NODE_ENUM) return NULL;
    
    char *name = enum_def->data.enum_def.name;
    
    // Check for redefinition
    Symbol *existing = symbol_table_lookup(ctx->symbols, name);
    if (existing) {
        semantic_context_error(ctx, "enum redefined", enum_def->line, enum_def->column);
    }
    
    // Create enum type (simplified)
    Type *enum_type = type_create_enum(name, NULL, NULL, 0);
    symbol_table_insert(ctx->symbols, name, SYMBOL_ENUM, enum_type, enum_def);
    
    return irnode_create(IR_NOP, name, enum_type);
}

static IRNode* analyze_statement(SemanticContext *ctx, ASTNode *stmt) {
    if (!stmt) return NULL;
    
    switch (stmt->type) {
        case NODE_VARIABLE_DECL:
            return analyze_variable_declaration(ctx, stmt);
        case NODE_FUNCTION:
            return analyze_function_declaration(ctx, stmt);
        case NODE_STRUCT:
            return analyze_struct_declaration(ctx, stmt);
        case NODE_ENUM:
            return analyze_enum_declaration(ctx, stmt);
        case NODE_EXPRESSION:
            return analyze_expression(ctx, stmt);
        case NODE_RETURN_STATEMENT: {
            IRNode *node = irnode_create(IR_RETURN, NULL, NULL);
            if (stmt->data.return_stmt.value) {
                IRNode *value = analyze_expression(ctx, stmt->data.return_stmt.value);
                if (value) irnode_add_child(node, value);
            }
            return node;
        }
        default:
            semantic_context_error(ctx, "unsupported statement type", stmt->line, stmt->column);
            return NULL;
    }
}

// Main semantic analysis function
bool semantic_analyze(SemanticContext *ctx, ASTNode *program, IRNode **ir_out) {
    if (!ctx || !program || program->type != NODE_PROGRAM) {
        return false;
    }
    
    IRNode *root = irnode_create(IR_BLOCK, "program", NULL);
    
    for (size_t i = 0; i < program->data.program.statement_count; i++) {
        ASTNode *stmt = program->data.program.statements[i];
        IRNode *stmt_ir = analyze_statement(ctx, stmt);
        if (stmt_ir) {
            irnode_add_child(root, stmt_ir);
        }
    }
    
    *ir_out = root;
    return !ctx->had_error;
} 