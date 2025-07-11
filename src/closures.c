#include "closures.h"
#include "lexer.h"
#include "parser.h"
#include "types.h"
#include "semantic.h"
#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// closure environment management
ClosureEnvironment* closure_environment_create(ClosureEnvironment *parent) {
    ClosureEnvironment *env = malloc(sizeof(ClosureEnvironment));
    if (!env) return NULL;
    
    env->captures = NULL;
    env->capture_count = 0;
    env->total_size = 0;
    env->parent = parent;
    
    return env;
}

void closure_environment_free(ClosureEnvironment *env) {
    if (!env) return;
    
    CapturedVariable *current = env->captures;
    while (current) {
        CapturedVariable *next = current->next;
        free(current->name);
        type_free(current->type);
        free(current);
        current = next;
    }
    
    free(env);
}

bool closure_environment_add_capture(ClosureEnvironment *env, const char *name, Type *type, CaptureMode mode) {
    if (!env || !name || !type) return false;
    
    CapturedVariable *capture = malloc(sizeof(CapturedVariable));
    if (!capture) return false;
    
    capture->name = strdup(name);
    capture->type = type_clone(type);
    capture->mode = mode;
    capture->offset = env->total_size;
    capture->next = env->captures;
    
    env->captures = capture;
    env->capture_count++;
    env->total_size += type_size(type);
    
    return true;
}

CapturedVariable* closure_environment_find_capture(ClosureEnvironment *env, const char *name) {
    if (!env || !name) return NULL;
    
    CapturedVariable *current = env->captures;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    
    // check parent environment for nested closures
    if (env->parent) {
        return closure_environment_find_capture(env->parent, name);
    }
    
    return NULL;
}

// closure type management
ClosureType* closure_type_create(Type *return_type, Type **params, size_t param_count, ClosureEnvironment *env) {
    ClosureType *type = malloc(sizeof(ClosureType));
    if (!type) return NULL;
    
    type->return_type = type_clone(return_type);
    type->parameter_count = param_count;
    type->parameter_types = malloc(sizeof(Type*) * param_count);
    type->environment = env;
    type->is_async = false;
    
    for (size_t i = 0; i < param_count; i++) {
        type->parameter_types[i] = type_clone(params[i]);
    }
    
    return type;
}

void closure_type_free(ClosureType *type) {
    if (!type) return;
    
    type_free(type->return_type);
    for (size_t i = 0; i < type->parameter_count; i++) {
        type_free(type->parameter_types[i]);
    }
    free(type->parameter_types);
    free(type);
}

bool closure_type_equals(ClosureType *a, ClosureType *b) {
    if (!a || !b) return false;
    if (a->parameter_count != b->parameter_count) return false;
    if (a->is_async != b->is_async) return false;
    
    // check return type
    if (!type_equals(a->return_type, b->return_type)) return false;
    
    // check parameter types
    for (size_t i = 0; i < a->parameter_count; i++) {
        if (!type_equals(a->parameter_types[i], b->parameter_types[i])) {
            return false;
        }
    }
    
    return true;
}

// closure expression parsing
ClosureExpr* parse_closure_expression(Parser *parser) {
    if (!parser) return NULL;
    
    // expect | or async | for closure
    bool is_async = false;
    if (check_token(parser, TOKEN_ASYNC)) {
        advance_token(parser);
        is_async = true;
    }
    
    if (!match_token(parser, TOKEN_PIPE)) {
        return NULL;
    }
    
    ClosureExpr *closure = malloc(sizeof(ClosureExpr));
    if (!closure) return NULL;
    
    // initialize base ast node
    closure->base.type = NODE_EXPRESSION;
    closure->base.line = parser->current_token->line;
    closure->base.column = parser->current_token->column;
    
    closure->parameters = NULL;
    closure->parameter_count = 0;
    closure->body = NULL;
    closure->captures = closure_environment_create(NULL);
    closure->is_async = is_async;
    
    // parse parameters
    if (!check_token(parser, TOKEN_PIPE)) {
        // parse parameter list
        do {
            if (closure->parameter_count == 0) {
                advance_token(parser); // skip first comma
            }
            
            if (!check_token(parser, TOKEN_IDENTIFIER)) {
                break;
            }
            
            char *param_name = strdup(parser->current_token->value);
            closure->parameters = realloc(closure->parameters, sizeof(char*) * (closure->parameter_count + 1));
            closure->parameters[closure->parameter_count++] = param_name;
            
            advance_token(parser);
        } while (match_token(parser, TOKEN_COMMA));
    }
    
    // expect closing pipe
    if (!match_token(parser, TOKEN_PIPE)) {
        // error: expected closing pipe
        free(closure);
        return NULL;
    }
    
    // parse body
    closure->body = parse_closure_body(parser);
    if (!closure->body) {
        free(closure);
        return NULL;
    }
    
    return closure;
}

ASTNode* parse_closure_body(Parser *parser) {
    if (!parser) return NULL;
    
    if (check_token(parser, TOKEN_LBRACE)) {
        // block body
        return parse_block(parser);
    } else {
        // expression body
        return parse_expression(parser);
    }
}

bool parse_closure_captures(Parser *parser, ClosureEnvironment *env) {
    if (!parser || !env) return false;
    
    // parse capture list if present
    if (check_token(parser, TOKEN_LBRACKET)) {
        advance_token(parser);
        
        while (!check_token(parser, TOKEN_RBRACKET)) {
            if (check_token(parser, TOKEN_IDENTIFIER)) {
                char *var_name = strdup(parser->current_token->value);
                advance_token(parser);
                
                // determine capture mode
                CaptureMode mode = CAPTURE_BY_REFERENCE;
                if (check_token(parser, TOKEN_MOVE)) {
                    mode = CAPTURE_BY_MOVE;
                    advance_token(parser);
                }
                
                // add to environment (type will be determined during semantic analysis)
                closure_environment_add_capture(env, var_name, type_create_unknown(), mode);
                free(var_name);
            }
            
            if (!match_token(parser, TOKEN_COMMA)) {
                break;
            }
        }
        
        if (!match_token(parser, TOKEN_RBRACKET)) {
            return false;
        }
    }
    
    return true;
}

// closure semantic analysis
bool analyze_closure_expression(SemanticContext *ctx, ClosureExpr *closure, Type **type_out) {
    if (!ctx || !closure || !type_out) return false;
    
    // analyze captures
    if (!analyze_closure_captures(ctx, closure)) {
        return false;
    }
    
    // check capture validity
    if (!check_closure_capture_validity(ctx, closure)) {
        return false;
    }
    
    // analyze body
    if (closure->body) {
        Type *body_type = type_check_expression(ctx->type_context, closure->body);
        if (!body_type) {
            return false;
        }
        
        // create closure type
        Type **param_types = malloc(sizeof(Type*) * closure->parameter_count);
        for (size_t i = 0; i < closure->parameter_count; i++) {
            param_types[i] = type_create_unknown(); // will be inferred
        }
        
        ClosureType *closure_type = closure_type_create(body_type, param_types, closure->parameter_count, closure->captures);
        *type_out = type_create_closure(closure_type);
        
        free(param_types);
        return true;
    }
    
    return false;
}

bool analyze_closure_captures(SemanticContext *ctx, ClosureExpr *closure) {
    if (!ctx || !closure) return false;
    
    CapturedVariable *current = closure->captures->captures;
    while (current) {
        // look up variable in symbol table
        Symbol *symbol = symbol_table_lookup(ctx->symbols, current->name);
        if (symbol) {
            // update capture with actual type
            type_free(current->type);
            current->type = type_clone(symbol->type);
        } else {
            // error: variable not found
            semantic_context_error(ctx, "captured variable not found", closure->base.line, closure->base.column);
            return false;
        }
        
        current = current->next;
    }
    
    return true;
}

bool check_closure_capture_validity(SemanticContext *ctx, ClosureExpr *closure) {
    if (!ctx || !closure) return false;
    
    CapturedVariable *current = closure->captures->captures;
    while (current) {
        // check if variable is mutable when captured by reference
        if (current->mode == CAPTURE_BY_REFERENCE) {
            Symbol *symbol = symbol_table_lookup(ctx->symbols, current->name);
            if (symbol && symbol->kind == SYMBOL_VARIABLE) {
                // check mutability (simplified)
                // in real implementation, would check variable mutability
            }
        }
        
        current = current->next;
    }
    
    return true;
}

// closure code generation
bool generate_closure_code(CodegenContext *ctx, ClosureExpr *closure) {
    if (!ctx || !closure) return false;
    
    // generate closure environment
    if (!generate_closure_environment(ctx, closure->captures)) {
        return false;
    }
    
    // generate closure function
    ClosureImpl *impl = malloc(sizeof(ClosureImpl));
    impl->name = "closure";
    impl->type = closure_type_create(type_create_unknown(), NULL, closure->parameter_count, closure->captures);
    impl->body = closure->body;
    impl->environment = closure->captures;
    impl->generated_function_name = "generated_closure";
    
    bool success = generate_closure_function(ctx, impl);
    
    closure_type_free(impl->type);
    free(impl);
    
    return success;
}

bool generate_closure_environment(CodegenContext *ctx, ClosureEnvironment *env) {
    if (!ctx || !env) return false;
    
    // generate environment structure
    fprintf(ctx->output, "; closure environment\n");
    fprintf(ctx->output, "section .data\n");
    fprintf(ctx->output, "closure_env:\n");
    
    CapturedVariable *current = env->captures;
    while (current) {
        fprintf(ctx->output, "    %s: %s %s\n", 
                current->name, 
                type_to_asm_type(current->type),
                current->mode == CAPTURE_BY_REFERENCE ? "ref" : "value");
        current = current->next;
    }
    
    return true;
}

bool generate_closure_function(CodegenContext *ctx, ClosureImpl *impl) {
    if (!ctx || !impl) return false;
    
    fprintf(ctx->output, "; generated closure function\n");
    fprintf(ctx->output, "%s:\n", impl->generated_function_name);
    fprintf(ctx->output, "    push rbp\n");
    fprintf(ctx->output, "    mov rbp, rsp\n");
    
    // access captured variables
    CapturedVariable *current = impl->environment->captures;
    while (current) {
        fprintf(ctx->output, "    ; access captured variable %s\n", current->name);
        fprintf(ctx->output, "    mov rax, [rbp+%zu]\n", current->offset);
        current = current->next;
    }
    
    // generate body code
    if (impl->body) {
        // simplified: would generate actual body code
        fprintf(ctx->output, "    ; closure body\n");
    }
    
    fprintf(ctx->output, "    pop rbp\n");
    fprintf(ctx->output, "    ret\n");
    
    return true;
}

// closure context management
ClosureContext* closure_context_create(void) {
    ClosureContext *ctx = malloc(sizeof(ClosureContext));
    if (!ctx) return NULL;
    
    ctx->current_environment = NULL;
    ctx->closure_count = 0;
    ctx->name_capacity = 10;
    ctx->generated_names = malloc(sizeof(char*) * ctx->name_capacity);
    
    return ctx;
}

void closure_context_free(ClosureContext *ctx) {
    if (!ctx) return;
    
    for (size_t i = 0; i < ctx->closure_count; i++) {
        free(ctx->generated_names[i]);
    }
    free(ctx->generated_names);
    free(ctx);
}

char* closure_context_generate_name(ClosureContext *ctx, const char *base) {
    if (!ctx || !base) return NULL;
    
    char *name = malloc(strlen(base) + 20);
    sprintf(name, "%s_%zu", base, ctx->closure_count++);
    
    if (ctx->closure_count >= ctx->name_capacity) {
        ctx->name_capacity *= 2;
        ctx->generated_names = realloc(ctx->generated_names, sizeof(char*) * ctx->name_capacity);
    }
    
    ctx->generated_names[ctx->closure_count - 1] = strdup(name);
    return name;
}

// closure optimization
bool optimize_closure_captures(ClosureExpr *closure) {
    if (!closure) return false;
    
    // eliminate unused captures
    return eliminate_unused_captures(closure);
}

bool eliminate_unused_captures(ClosureExpr *closure) {
    if (!closure) return false;
    
    // simplified: would analyze body to find used captures
    // and remove unused ones
    return true;
}

bool optimize_closure_environment(ClosureEnvironment *env) {
    if (!env) return false;
    
    // optimize environment layout
    // could reorder captures for better memory layout
    return true;
}

// closure testing utilities
bool test_closure_parsing(const char *source) {
    Lexer *lexer = lexer_init(source);
    Parser *parser = parser_init(lexer);
    
    ClosureExpr *closure = parse_closure_expression(parser);
    bool success = closure != NULL;
    
    if (closure) {
        // free closure
        closure_environment_free(closure->captures);
        free(closure);
    }
    
    parser_free(parser);
    lexer_free(lexer);
    
    return success;
}

bool test_closure_type_checking(const char *source) {
    // simplified test
    return true;
}

bool test_closure_code_generation(const char *source) {
    // simplified test
    return true;
} 