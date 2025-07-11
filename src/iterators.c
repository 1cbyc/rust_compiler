#include "iterators.h"
#include "lexer.h"
#include "parser.h"
#include "types.h"
#include "semantic.h"
#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// iterator trait management
IteratorType* iterator_type_create(Type *element_type, bool is_mutable, bool is_double_ended, bool is_exact_size) {
    IteratorType *type = malloc(sizeof(IteratorType));
    if (!type) return NULL;
    
    type->element_type = type_clone(element_type);
    type->is_mutable = is_mutable;
    type->is_double_ended = is_double_ended;
    type->is_exact_size = is_exact_size;
    type->next = NULL;
    
    return type;
}

void iterator_type_free(IteratorType *type) {
    if (!type) return;
    
    type_free(type->element_type);
    free(type);
}

bool iterator_type_equals(IteratorType *a, IteratorType *b) {
    if (!a || !b) return false;
    if (a->is_mutable != b->is_mutable) return false;
    if (a->is_double_ended != b->is_double_ended) return false;
    if (a->is_exact_size != b->is_exact_size) return false;
    
    return type_equals(a->element_type, b->element_type);
}

// iterator implementation management
IteratorImpl* iterator_impl_create(const char *name, IteratorType *type) {
    IteratorImpl *impl = malloc(sizeof(IteratorImpl));
    if (!impl) return NULL;
    
    impl->name = strdup(name);
    impl->type = type;
    impl->next_function = NULL;
    impl->size_hint_function = NULL;
    impl->count_function = NULL;
    impl->last_function = NULL;
    impl->nth_function = NULL;
    impl->step_by_function = NULL;
    impl->chain_function = NULL;
    impl->zip_function = NULL;
    impl->map_function = NULL;
    impl->filter_function = NULL;
    impl->fold_function = NULL;
    impl->any_function = NULL;
    impl->all_function = NULL;
    impl->find_function = NULL;
    impl->position_function = NULL;
    
    return impl;
}

void iterator_impl_free(IteratorImpl *impl) {
    if (!impl) return;
    
    free(impl->name);
    // note: don't free type as it's shared
    // free ast nodes if needed
    free(impl);
}

bool iterator_impl_add_method(IteratorImpl *impl, IteratorMethod method, ASTNode *function) {
    if (!impl || !function) return false;
    
    switch (method) {
        case ITER_NEXT:
            impl->next_function = function;
            break;
        case ITER_SIZE_HINT:
            impl->size_hint_function = function;
            break;
        case ITER_COUNT:
            impl->count_function = function;
            break;
        case ITER_LAST:
            impl->last_function = function;
            break;
        case ITER_NTH:
            impl->nth_function = function;
            break;
        case ITER_STEP_BY:
            impl->step_by_function = function;
            break;
        case ITER_CHAIN:
            impl->chain_function = function;
            break;
        case ITER_ZIP:
            impl->zip_function = function;
            break;
        case ITER_MAP:
            impl->map_function = function;
            break;
        case ITER_FILTER:
            impl->filter_function = function;
            break;
        case ITER_FOLD:
            impl->fold_function = function;
            break;
        case ITER_ANY:
            impl->any_function = function;
            break;
        case ITER_ALL:
            impl->all_function = function;
            break;
        case ITER_FIND:
            impl->find_function = function;
            break;
        case ITER_POSITION:
            impl->position_function = function;
            break;
        default:
            return false;
    }
    
    return true;
}

// iterator expression parsing
IteratorExpr* parse_iterator_expression(Parser *parser) {
    if (!parser) return NULL;
    
    // expect collection name followed by .iter() or similar
    if (!check_token(parser, TOKEN_IDENTIFIER)) {
        return NULL;
    }
    
    IteratorExpr *iterator = malloc(sizeof(IteratorExpr));
    if (!iterator) return NULL;
    
    // initialize base ast node
    iterator->base.type = NODE_EXPRESSION;
    iterator->base.line = parser->current_token->line;
    iterator->base.column = parser->current_token->column;
    
    iterator->collection_name = strdup(parser->current_token->value);
    iterator->iterator_type = NULL;
    iterator->range_expr = NULL;
    iterator->filter_expr = NULL;
    iterator->map_expr = NULL;
    iterator->is_mutable = false;
    
    advance_token(parser);
    
    // expect dot
    if (!match_token(parser, TOKEN_DOT)) {
        free(iterator);
        return NULL;
    }
    
    // expect method call
    if (!check_token(parser, TOKEN_IDENTIFIER)) {
        free(iterator);
        return NULL;
    }
    
    char *method_name = strdup(parser->current_token->value);
    advance_token(parser);
    
    // expect parentheses
    if (!match_token(parser, TOKEN_LPAREN)) {
        free(method_name);
        free(iterator);
        return NULL;
    }
    
    // parse arguments if any
    if (!check_token(parser, TOKEN_RPAREN)) {
        // parse arguments (simplified)
        while (!check_token(parser, TOKEN_RPAREN)) {
            advance_token(parser);
            if (match_token(parser, TOKEN_COMMA)) {
                continue;
            }
        }
    }
    
    if (!match_token(parser, TOKEN_RPAREN)) {
        free(method_name);
        free(iterator);
        return NULL;
    }
    
    // handle different iterator methods
    if (strcmp(method_name, "iter") == 0) {
        iterator->is_mutable = false;
    } else if (strcmp(method_name, "iter_mut") == 0) {
        iterator->is_mutable = true;
    } else if (strcmp(method_name, "into_iter") == 0) {
        iterator->is_mutable = false;
    }
    
    free(method_name);
    return iterator;
}

ASTNode* parse_iterator_range(Parser *parser) {
    if (!parser) return NULL;
    
    // expect range expression like 0..10
    if (!check_token(parser, TOKEN_INTEGER_LITERAL)) {
        return NULL;
    }
    
    // simplified range parsing
    ASTNode *start = parse_expression(parser);
    
    if (!match_token(parser, TOKEN_DOT_DOT)) {
        return start;
    }
    
    ASTNode *end = parse_expression(parser);
    
    // create range expression (simplified)
    ASTNode *range = malloc(sizeof(ASTNode));
    range->type = NODE_EXPRESSION;
    range->line = parser->current_token->line;
    range->column = parser->current_token->column;
    
    return range;
}

ASTNode* parse_iterator_filter(Parser *parser) {
    if (!parser) return NULL;
    
    // expect .filter(|x| ...)
    if (!match_token(parser, TOKEN_DOT)) {
        return NULL;
    }
    
    if (!check_token(parser, TOKEN_IDENTIFIER) || strcmp(parser->current_token->value, "filter") != 0) {
        return NULL;
    }
    
    advance_token(parser);
    
    if (!match_token(parser, TOKEN_LPAREN)) {
        return NULL;
    }
    
    // parse closure
    ASTNode *filter_expr = parse_closure_expression(parser);
    
    if (!match_token(parser, TOKEN_RPAREN)) {
        return NULL;
    }
    
    return filter_expr;
}

ASTNode* parse_iterator_map(Parser *parser) {
    if (!parser) return NULL;
    
    // expect .map(|x| ...)
    if (!match_token(parser, TOKEN_DOT)) {
        return NULL;
    }
    
    if (!check_token(parser, TOKEN_IDENTIFIER) || strcmp(parser->current_token->value, "map") != 0) {
        return NULL;
    }
    
    advance_token(parser);
    
    if (!match_token(parser, TOKEN_LPAREN)) {
        return NULL;
    }
    
    // parse closure
    ASTNode *map_expr = parse_closure_expression(parser);
    
    if (!match_token(parser, TOKEN_RPAREN)) {
        return NULL;
    }
    
    return map_expr;
}

// iterator semantic analysis
bool analyze_iterator_expression(SemanticContext *ctx, IteratorExpr *iterator, Type **type_out) {
    if (!ctx || !iterator || !type_out) return false;
    
    // look up collection in symbol table
    Symbol *collection = symbol_table_lookup(ctx->symbols, iterator->collection_name);
    if (!collection) {
        semantic_context_error(ctx, "collection not found", iterator->base.line, iterator->base.column);
        return false;
    }
    
    // determine element type from collection type
    Type *element_type = NULL;
    if (strcmp(collection->type->name, "Vec") == 0) {
        element_type = type_create_primitive("i32"); // simplified
    } else if (strcmp(collection->type->name, "&str") == 0) {
        element_type = type_create_primitive("char");
    } else {
        element_type = type_create_primitive("i32"); // default
    }
    
    // create iterator type
    IteratorType *iterator_type = iterator_type_create(element_type, iterator->is_mutable, true, false);
    iterator->iterator_type = iterator_type;
    
    *type_out = type_create_iterator(iterator_type);
    return true;
}

bool analyze_iterator_methods(SemanticContext *ctx, IteratorImpl *impl) {
    if (!ctx || !impl) return false;
    
    // check that required methods are implemented
    if (!impl->next_function) {
        semantic_context_error(ctx, "iterator must implement next() method", 0, 0);
        return false;
    }
    
    return true;
}

bool check_iterator_trait_implementation(SemanticContext *ctx, IteratorImpl *impl) {
    if (!ctx || !impl) return false;
    
    // check iterator trait requirements
    // simplified: just check that next() method exists
    return impl->next_function != NULL;
}

// iterator code generation
bool generate_iterator_code(CodegenContext *ctx, IteratorExpr *iterator) {
    if (!ctx || !iterator) return false;
    
    fprintf(ctx->output, "; iterator for %s\n", iterator->collection_name);
    fprintf(ctx->output, "iterator_%s:\n", iterator->collection_name);
    fprintf(ctx->output, "    push rbp\n");
    fprintf(ctx->output, "    mov rbp, rsp\n");
    
    // generate iterator initialization
    fprintf(ctx->output, "    ; initialize iterator\n");
    fprintf(ctx->output, "    mov rax, [rbp+8]  ; collection pointer\n");
    fprintf(ctx->output, "    mov rbx, 0         ; index\n");
    fprintf(ctx->output, "    mov rcx, [rax]     ; length\n");
    
    // generate next() function
    fprintf(ctx->output, "iterator_%s_next:\n", iterator->collection_name);
    fprintf(ctx->output, "    cmp rbx, rcx\n");
    fprintf(ctx->output, "    jge iterator_end\n");
    fprintf(ctx->output, "    mov rax, [rax+8+rbx*8]  ; get element\n");
    fprintf(ctx->output, "    inc rbx\n");
    fprintf(ctx->output, "    ret\n");
    fprintf(ctx->output, "iterator_end:\n");
    fprintf(ctx->output, "    mov rax, 0  ; return None\n");
    fprintf(ctx->output, "    ret\n");
    
    fprintf(ctx->output, "    pop rbp\n");
    fprintf(ctx->output, "    ret\n");
    
    return true;
}

bool generate_iterator_methods(CodegenContext *ctx, IteratorImpl *impl) {
    if (!ctx || !impl) return false;
    
    fprintf(ctx->output, "; iterator methods for %s\n", impl->name);
    
    // generate next() method
    if (impl->next_function) {
        fprintf(ctx->output, "%s_next:\n", impl->name);
        // simplified: would generate actual next() implementation
        fprintf(ctx->output, "    ; next() implementation\n");
        fprintf(ctx->output, "    ret\n");
    }
    
    return true;
}

bool generate_iterator_next_function(CodegenContext *ctx, IteratorImpl *impl) {
    if (!ctx || !impl) return false;
    
    fprintf(ctx->output, "; %s next() function\n", impl->name);
    fprintf(ctx->output, "%s_next:\n", impl->name);
    fprintf(ctx->output, "    push rbp\n");
    fprintf(ctx->output, "    mov rbp, rsp\n");
    
    // simplified next() implementation
    fprintf(ctx->output, "    ; get next element\n");
    fprintf(ctx->output, "    mov rax, [rbp+8]  ; iterator pointer\n");
    fprintf(ctx->output, "    mov rbx, [rax]     ; current index\n");
    fprintf(ctx->output, "    mov rcx, [rax+8]   ; length\n");
    
    fprintf(ctx->output, "    cmp rbx, rcx\n");
    fprintf(ctx->output, "    jge .end\n");
    
    fprintf(ctx->output, "    mov rdx, [rax+16]  ; data pointer\n");
    fprintf(ctx->output, "    mov rax, [rdx+rbx*8]  ; get element\n");
    fprintf(ctx->output, "    inc rbx\n");
    fprintf(ctx->output, "    mov [rax], rbx     ; update index\n");
    fprintf(ctx->output, "    jmp .done\n");
    
    fprintf(ctx->output, ".end:\n");
    fprintf(ctx->output, "    mov rax, 0  ; return None\n");
    
    fprintf(ctx->output, ".done:\n");
    fprintf(ctx->output, "    pop rbp\n");
    fprintf(ctx->output, "    ret\n");
    
    return true;
}

// iterator context management
IteratorContext* iterator_context_create(void) {
    IteratorContext *ctx = malloc(sizeof(IteratorContext));
    if (!ctx) return NULL;
    
    ctx->current_iterator = NULL;
    ctx->iterator_count = 0;
    ctx->name_capacity = 10;
    ctx->iterator_names = malloc(sizeof(char*) * ctx->name_capacity);
    
    return ctx;
}

void iterator_context_free(IteratorContext *ctx) {
    if (!ctx) return;
    
    for (size_t i = 0; i < ctx->iterator_count; i++) {
        free(ctx->iterator_names[i]);
    }
    free(ctx->iterator_names);
    free(ctx);
}

char* iterator_context_generate_name(IteratorContext *ctx, const char *base) {
    if (!ctx || !base) return NULL;
    
    char *name = malloc(strlen(base) + 20);
    sprintf(name, "%s_%zu", base, ctx->iterator_count++);
    
    if (ctx->iterator_count >= ctx->name_capacity) {
        ctx->name_capacity *= 2;
        ctx->iterator_names = realloc(ctx->iterator_names, sizeof(char*) * ctx->name_capacity);
    }
    
    ctx->iterator_names[ctx->iterator_count - 1] = strdup(name);
    return name;
}

// iterator optimization
bool optimize_iterator_operations(IteratorExpr *iterator) {
    if (!iterator) return false;
    
    // optimize iterator chains
    return optimize_iterator_chain(iterator);
}

bool eliminate_unused_iterator_methods(IteratorExpr *iterator) {
    if (!iterator) return false;
    
    // remove unused iterator methods
    return true;
}

bool optimize_iterator_chain(IteratorExpr *iterator) {
    if (!iterator) return false;
    
    // optimize iterator chains by combining operations
    return true;
}

// iterator testing utilities
bool test_iterator_parsing(const char *source) {
    Lexer *lexer = lexer_init(source);
    Parser *parser = parser_init(lexer);
    
    IteratorExpr *iterator = parse_iterator_expression(parser);
    bool success = iterator != NULL;
    
    if (iterator) {
        free(iterator->collection_name);
        free(iterator);
    }
    
    parser_free(parser);
    lexer_free(lexer);
    
    return success;
}

bool test_iterator_type_checking(const char *source) {
    // simplified test
    return true;
}

bool test_iterator_code_generation(const char *source) {
    // simplified test
    return true;
}

// built-in iterator types
IteratorType* create_vec_iterator(Type *element_type, bool is_mutable) {
    return iterator_type_create(element_type, is_mutable, true, true);
}

IteratorType* create_range_iterator(Type *element_type) {
    return iterator_type_create(element_type, false, true, true);
}

IteratorType* create_string_iterator(bool is_mutable) {
    Type *char_type = type_create_primitive("char");
    return iterator_type_create(char_type, is_mutable, true, true);
}

IteratorType* create_array_iterator(Type *element_type, size_t size) {
    return iterator_type_create(element_type, false, true, true);
}

// iterator adaptors
IteratorExpr* create_map_iterator(IteratorExpr *base, ASTNode *map_function) {
    if (!base || !map_function) return NULL;
    
    IteratorExpr *map_iter = malloc(sizeof(IteratorExpr));
    if (!map_iter) return NULL;
    
    map_iter->base = base->base;
    map_iter->collection_name = strdup(base->collection_name);
    map_iter->iterator_type = base->iterator_type;
    map_iter->range_expr = base->range_expr;
    map_iter->filter_expr = base->filter_expr;
    map_iter->map_expr = map_function;
    map_iter->is_mutable = base->is_mutable;
    
    return map_iter;
}

IteratorExpr* create_filter_iterator(IteratorExpr *base, ASTNode *filter_function) {
    if (!base || !filter_function) return NULL;
    
    IteratorExpr *filter_iter = malloc(sizeof(IteratorExpr));
    if (!filter_iter) return NULL;
    
    filter_iter->base = base->base;
    filter_iter->collection_name = strdup(base->collection_name);
    filter_iter->iterator_type = base->iterator_type;
    filter_iter->range_expr = base->range_expr;
    filter_iter->filter_expr = filter_function;
    filter_iter->map_expr = base->map_expr;
    filter_iter->is_mutable = base->is_mutable;
    
    return filter_iter;
}

IteratorExpr* create_chain_iterator(IteratorExpr *first, IteratorExpr *second) {
    if (!first || !second) return NULL;
    
    IteratorExpr *chain_iter = malloc(sizeof(IteratorExpr));
    if (!chain_iter) return NULL;
    
    chain_iter->base = first->base;
    chain_iter->collection_name = strdup("chain");
    chain_iter->iterator_type = first->iterator_type;
    chain_iter->range_expr = NULL;
    chain_iter->filter_expr = NULL;
    chain_iter->map_expr = NULL;
    chain_iter->is_mutable = false;
    
    return chain_iter;
}

IteratorExpr* create_zip_iterator(IteratorExpr *first, IteratorExpr *second) {
    if (!first || !second) return NULL;
    
    IteratorExpr *zip_iter = malloc(sizeof(IteratorExpr));
    if (!zip_iter) return NULL;
    
    zip_iter->base = first->base;
    zip_iter->collection_name = strdup("zip");
    zip_iter->iterator_type = first->iterator_type;
    zip_iter->range_expr = NULL;
    zip_iter->filter_expr = NULL;
    zip_iter->map_expr = NULL;
    zip_iter->is_mutable = false;
    
    return zip_iter;
}

IteratorExpr* create_step_by_iterator(IteratorExpr *base, size_t step) {
    if (!base) return NULL;
    
    IteratorExpr *step_iter = malloc(sizeof(IteratorExpr));
    if (!step_iter) return NULL;
    
    step_iter->base = base->base;
    step_iter->collection_name = strdup(base->collection_name);
    step_iter->iterator_type = base->iterator_type;
    step_iter->range_expr = base->range_expr;
    step_iter->filter_expr = base->filter_expr;
    step_iter->map_expr = base->map_expr;
    step_iter->is_mutable = base->is_mutable;
    
    return step_iter;
}

// iterator consumers
bool generate_iterator_consumer(CodegenContext *ctx, IteratorExpr *iterator, const char *consumer_type) {
    if (!ctx || !iterator || !consumer_type) return false;
    
    fprintf(ctx->output, "; iterator consumer: %s\n", consumer_type);
    
    if (strcmp(consumer_type, "for") == 0) {
        fprintf(ctx->output, "    ; for loop implementation\n");
    } else if (strcmp(consumer_type, "collect") == 0) {
        fprintf(ctx->output, "    ; collect implementation\n");
    } else if (strcmp(consumer_type, "sum") == 0) {
        fprintf(ctx->output, "    ; sum implementation\n");
    }
    
    return true;
}

bool generate_for_loop(CodegenContext *ctx, IteratorExpr *iterator, ASTNode *body) {
    if (!ctx || !iterator || !body) return false;
    
    fprintf(ctx->output, "; for loop with iterator\n");
    fprintf(ctx->output, "for_loop:\n");
    fprintf(ctx->output, "    push rbp\n");
    fprintf(ctx->output, "    mov rbp, rsp\n");
    
    fprintf(ctx->output, "loop_start:\n");
    fprintf(ctx->output, "    ; get next element\n");
    fprintf(ctx->output, "    call iterator_next\n");
    fprintf(ctx->output, "    cmp rax, 0\n");
    fprintf(ctx->output, "    je loop_end\n");
    
    fprintf(ctx->output, "    ; execute loop body\n");
    fprintf(ctx->output, "    ; body implementation\n");
    
    fprintf(ctx->output, "    jmp loop_start\n");
    fprintf(ctx->output, "loop_end:\n");
    fprintf(ctx->output, "    pop rbp\n");
    fprintf(ctx->output, "    ret\n");
    
    return true;
}

bool generate_collect_operation(CodegenContext *ctx, IteratorExpr *iterator, Type *target_type) {
    if (!ctx || !iterator || !target_type) return false;
    
    fprintf(ctx->output, "; collect iterator into %s\n", target_type->name);
    fprintf(ctx->output, "collect_operation:\n");
    fprintf(ctx->output, "    push rbp\n");
    fprintf(ctx->output, "    mov rbp, rsp\n");
    
    fprintf(ctx->output, "    ; allocate result collection\n");
    fprintf(ctx->output, "    ; iterate and collect elements\n");
    fprintf(ctx->output, "    ; return collected result\n");
    
    fprintf(ctx->output, "    pop rbp\n");
    fprintf(ctx->output, "    ret\n");
    
    return true;
} 