#ifndef ITERATORS_H
#define ITERATORS_H

#include "parser.h"
#include "types.h"
#include "semantic.h"
#include <stddef.h>
#include <stdbool.h>

// iterator trait methods
typedef enum {
    ITER_NEXT,
    ITER_SIZE_HINT,
    ITER_COUNT,
    ITER_LAST,
    ITER_NTH,
    ITER_STEP_BY,
    ITER_CHAIN,
    ITER_ZIP,
    ITER_MAP,
    ITER_FILTER,
    ITER_FOLD,
    ITER_ANY,
    ITER_ALL,
    ITER_FIND,
    ITER_POSITION
} IteratorMethod;

// iterator state
typedef enum {
    ITER_STATE_INIT,
    ITER_STATE_ACTIVE,
    ITER_STATE_EXHAUSTED,
    ITER_STATE_ERROR
} IteratorState;

// iterator type
typedef struct IteratorType {
    Type *element_type;
    bool is_mutable;
    bool is_double_ended;
    bool is_exact_size;
    struct IteratorType *next;
} IteratorType;

// iterator implementation
typedef struct IteratorImpl {
    char *name;
    IteratorType *type;
    ASTNode *next_function;
    ASTNode *size_hint_function;
    ASTNode *count_function;
    ASTNode *last_function;
    ASTNode *nth_function;
    ASTNode *step_by_function;
    ASTNode *chain_function;
    ASTNode *zip_function;
    ASTNode *map_function;
    ASTNode *filter_function;
    ASTNode *fold_function;
    ASTNode *any_function;
    ASTNode *all_function;
    ASTNode *find_function;
    ASTNode *position_function;
} IteratorImpl;

// iterator expression ast node
typedef struct IteratorExpr {
    ASTNode base;
    char *collection_name;
    IteratorType *iterator_type;
    ASTNode *range_expr;
    ASTNode *filter_expr;
    ASTNode *map_expr;
    bool is_mutable;
} IteratorExpr;

// iterator context for analysis
typedef struct IteratorContext {
    IteratorType *current_iterator;
    size_t iterator_count;
    char **iterator_names;
    size_t name_capacity;
} IteratorContext;

// iterator trait management
IteratorType* iterator_type_create(Type *element_type, bool is_mutable, bool is_double_ended, bool is_exact_size);
void iterator_type_free(IteratorType *type);
bool iterator_type_equals(IteratorType *a, IteratorType *b);

// iterator implementation management
IteratorImpl* iterator_impl_create(const char *name, IteratorType *type);
void iterator_impl_free(IteratorImpl *impl);
bool iterator_impl_add_method(IteratorImpl *impl, IteratorMethod method, ASTNode *function);

// iterator expression parsing
IteratorExpr* parse_iterator_expression(Parser *parser);
ASTNode* parse_iterator_range(Parser *parser);
ASTNode* parse_iterator_filter(Parser *parser);
ASTNode* parse_iterator_map(Parser *parser);

// iterator semantic analysis
bool analyze_iterator_expression(SemanticContext *ctx, IteratorExpr *iterator, Type **type_out);
bool analyze_iterator_methods(SemanticContext *ctx, IteratorImpl *impl);
bool check_iterator_trait_implementation(SemanticContext *ctx, IteratorImpl *impl);

// iterator code generation
bool generate_iterator_code(CodegenContext *ctx, IteratorExpr *iterator);
bool generate_iterator_methods(CodegenContext *ctx, IteratorImpl *impl);
bool generate_iterator_next_function(CodegenContext *ctx, IteratorImpl *impl);

// iterator context management
IteratorContext* iterator_context_create(void);
void iterator_context_free(IteratorContext *ctx);
char* iterator_context_generate_name(IteratorContext *ctx, const char *base);

// iterator optimization
bool optimize_iterator_operations(IteratorExpr *iterator);
bool eliminate_unused_iterator_methods(IteratorExpr *iterator);
bool optimize_iterator_chain(IteratorExpr *iterator);

// iterator testing utilities
bool test_iterator_parsing(const char *source);
bool test_iterator_type_checking(const char *source);
bool test_iterator_code_generation(const char *source);

// built-in iterator types
IteratorType* create_vec_iterator(Type *element_type, bool is_mutable);
IteratorType* create_range_iterator(Type *element_type);
IteratorType* create_string_iterator(bool is_mutable);
IteratorType* create_array_iterator(Type *element_type, size_t size);

// iterator adaptors
IteratorExpr* create_map_iterator(IteratorExpr *base, ASTNode *map_function);
IteratorExpr* create_filter_iterator(IteratorExpr *base, ASTNode *filter_function);
IteratorExpr* create_chain_iterator(IteratorExpr *first, IteratorExpr *second);
IteratorExpr* create_zip_iterator(IteratorExpr *first, IteratorExpr *second);
IteratorExpr* create_step_by_iterator(IteratorExpr *base, size_t step);

// iterator consumers
bool generate_iterator_consumer(CodegenContext *ctx, IteratorExpr *iterator, const char *consumer_type);
bool generate_for_loop(CodegenContext *ctx, IteratorExpr *iterator, ASTNode *body);
bool generate_collect_operation(CodegenContext *ctx, IteratorExpr *iterator, Type *target_type);

#endif // ITERATORS_H 