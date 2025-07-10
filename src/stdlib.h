#ifndef STDLIB_H
#define STDLIB_H

#include "types.h"
#include "semantic.h"
#include <stddef.h>
#include <stdbool.h>

// Standard library function types
typedef struct StdLibFunction {
    char *name;
    Type **param_types;
    size_t param_count;
    Type *return_type;
    void (*native_impl)(void); // pointer to native implementation
} StdLibFunction;

// Standard library context
typedef struct StdLibContext {
    StdLibFunction **functions;
    size_t function_count;
    size_t capacity;
} StdLibContext;

// Standard library initialization
StdLibContext* stdlib_create(void);
void stdlib_free(StdLibContext *ctx);

// Function registration
bool stdlib_register_function(StdLibContext *ctx, const char *name, 
                            Type **param_types, size_t param_count, 
                            Type *return_type, void (*impl)(void));

// Standard library functions
void stdlib_init_print_functions(StdLibContext *ctx);
void stdlib_init_string_functions(StdLibContext *ctx);
void stdlib_init_collection_functions(StdLibContext *ctx);
void stdlib_init_error_functions(StdLibContext *ctx);

// Native implementations (stubs)
void stdlib_print(void);
void stdlib_println(void);
void stdlib_read_line(void);
void stdlib_string_len(void);
void stdlib_string_concat(void);
void stdlib_vec_new(void);
void stdlib_vec_push(void);
void stdlib_vec_get(void);
void stdlib_result_ok(void);
void stdlib_result_err(void);

// Macro system (basic)
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

MacroContext* macro_context_create(void);
void macro_context_free(MacroContext *ctx);
bool macro_register(MacroContext *ctx, const char *name, const char *pattern, const char *replacement);
char* macro_expand(MacroContext *ctx, const char *input);

#endif // STDLIB_H 