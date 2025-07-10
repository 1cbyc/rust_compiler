#include "stdlib.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Standard library context management
StdLibContext* stdlib_create(void) {
    StdLibContext *ctx = malloc(sizeof(StdLibContext));
    ctx->functions = NULL;
    ctx->function_count = 0;
    ctx->capacity = 0;
    return ctx;
}

void stdlib_free(StdLibContext *ctx) {
    if (!ctx) return;
    
    for (size_t i = 0; i < ctx->function_count; i++) {
        StdLibFunction *func = ctx->functions[i];
        free(func->name);
        free(func->param_types);
        free(func);
    }
    free(ctx->functions);
    free(ctx);
}

bool stdlib_register_function(StdLibContext *ctx, const char *name, 
                            Type **param_types, size_t param_count, 
                            Type *return_type, void (*impl)(void)) {
    if (!ctx || !name) return false;
    
    // Expand capacity if needed
    if (ctx->function_count >= ctx->capacity) {
        size_t new_capacity = ctx->capacity == 0 ? 8 : ctx->capacity * 2;
        StdLibFunction **new_functions = realloc(ctx->functions, 
                                               new_capacity * sizeof(StdLibFunction*));
        if (!new_functions) return false;
        ctx->functions = new_functions;
        ctx->capacity = new_capacity;
    }
    
    // Create function
    StdLibFunction *func = malloc(sizeof(StdLibFunction));
    func->name = strdup(name);
    func->param_types = param_types;
    func->param_count = param_count;
    func->return_type = return_type;
    func->native_impl = impl;
    
    ctx->functions[ctx->function_count++] = func;
    return true;
}

// Initialize print functions
void stdlib_init_print_functions(StdLibContext *ctx) {
    // print!(value: &str) -> ()
    Type *str_type = type_create_primitive(TYPE_STRING, "&str", 16);
    Type **print_params = malloc(sizeof(Type*));
    print_params[0] = str_type;
    stdlib_register_function(ctx, "print", print_params, 1, TYPE_UNIT, stdlib_print);
    
    // println!(value: &str) -> ()
    Type **println_params = malloc(sizeof(Type*));
    println_params[0] = str_type;
    stdlib_register_function(ctx, "println", println_params, 1, TYPE_UNIT, stdlib_println);
}

// Initialize string functions
void stdlib_init_string_functions(StdLibContext *ctx) {
    // len(s: &str) -> usize
    Type *str_type = type_create_primitive(TYPE_STRING, "&str", 16);
    Type *usize_type = type_create_primitive(TYPE_UINT, "usize", 8);
    Type **len_params = malloc(sizeof(Type*));
    len_params[0] = str_type;
    stdlib_register_function(ctx, "len", len_params, 1, usize_type, stdlib_string_len);
    
    // concat(a: &str, b: &str) -> String
    Type *string_type = type_create_primitive(TYPE_STRING, "String", 24);
    Type **concat_params = malloc(2 * sizeof(Type*));
    concat_params[0] = str_type;
    concat_params[1] = str_type;
    stdlib_register_function(ctx, "concat", concat_params, 2, string_type, stdlib_string_concat);
}

// Initialize collection functions
void stdlib_init_collection_functions(StdLibContext *ctx) {
    // Vec::new() -> Vec<T>
    Type *vec_type = type_create_generic("Vec", NULL, 0);
    stdlib_register_function(ctx, "Vec::new", NULL, 0, vec_type, stdlib_vec_new);
    
    // push(vec: &mut Vec<T>, item: T) -> ()
    Type **push_params = malloc(2 * sizeof(Type*));
    push_params[0] = vec_type;
    push_params[1] = TYPE_UNKNOWN; // generic T
    stdlib_register_function(ctx, "push", push_params, 2, TYPE_UNIT, stdlib_vec_push);
    
    // get(vec: &Vec<T>, index: usize) -> Option<T>
    Type *option_type = type_create_generic("Option", NULL, 0);
    Type **get_params = malloc(2 * sizeof(Type*));
    get_params[0] = vec_type;
    get_params[1] = type_create_primitive(TYPE_UINT, "usize", 8);
    stdlib_register_function(ctx, "get", get_params, 2, option_type, stdlib_vec_get);
}

// Initialize error handling functions
void stdlib_init_error_functions(StdLibContext *ctx) {
    // Result::Ok(value: T) -> Result<T, E>
    Type *result_type = type_create_generic("Result", NULL, 0);
    Type **ok_params = malloc(sizeof(Type*));
    ok_params[0] = TYPE_UNKNOWN; // generic T
    stdlib_register_function(ctx, "Result::Ok", ok_params, 1, result_type, stdlib_result_ok);
    
    // Result::Err(error: E) -> Result<T, E>
    Type **err_params = malloc(sizeof(Type*));
    err_params[0] = TYPE_UNKNOWN; // generic E
    stdlib_register_function(ctx, "Result::Err", err_params, 1, result_type, stdlib_result_err);
}

// Native implementations (stubs)
void stdlib_print(void) {
    // Native implementation would handle printing
    printf("print called\n");
}

void stdlib_println(void) {
    // Native implementation would handle printing with newline
    printf("println called\n");
}

void stdlib_read_line(void) {
    // Native implementation would handle reading
    printf("read_line called\n");
}

void stdlib_string_len(void) {
    // Native implementation would calculate string length
    printf("string_len called\n");
}

void stdlib_string_concat(void) {
    // Native implementation would concatenate strings
    printf("string_concat called\n");
}

void stdlib_vec_new(void) {
    // Native implementation would create new vector
    printf("vec_new called\n");
}

void stdlib_vec_push(void) {
    // Native implementation would push to vector
    printf("vec_push called\n");
}

void stdlib_vec_get(void) {
    // Native implementation would get from vector
    printf("vec_get called\n");
}

void stdlib_result_ok(void) {
    // Native implementation would create Ok result
    printf("result_ok called\n");
}

void stdlib_result_err(void) {
    // Native implementation would create Err result
    printf("result_err called\n");
}

// Macro system
MacroContext* macro_context_create(void) {
    MacroContext *ctx = malloc(sizeof(MacroContext));
    ctx->macros = NULL;
    ctx->macro_count = 0;
    ctx->capacity = 0;
    return ctx;
}

void macro_context_free(MacroContext *ctx) {
    if (!ctx) return;
    
    for (size_t i = 0; i < ctx->macro_count; i++) {
        Macro *macro = ctx->macros[i];
        free(macro->name);
        free(macro->pattern);
        free(macro->replacement);
        free(macro);
    }
    free(ctx->macros);
    free(ctx);
}

bool macro_register(MacroContext *ctx, const char *name, const char *pattern, const char *replacement) {
    if (!ctx || !name || !pattern || !replacement) return false;
    
    // Expand capacity if needed
    if (ctx->macro_count >= ctx->capacity) {
        size_t new_capacity = ctx->capacity == 0 ? 8 : ctx->capacity * 2;
        Macro **new_macros = realloc(ctx->macros, new_capacity * sizeof(Macro*));
        if (!new_macros) return false;
        ctx->macros = new_macros;
        ctx->capacity = new_capacity;
    }
    
    // Create macro
    Macro *macro = malloc(sizeof(Macro));
    macro->name = strdup(name);
    macro->pattern = strdup(pattern);
    macro->replacement = strdup(replacement);
    
    ctx->macros[ctx->macro_count++] = macro;
    return true;
}

char* macro_expand(MacroContext *ctx, const char *input) {
    if (!ctx || !input) return NULL;
    
    // Simple macro expansion (basic implementation)
    char *result = strdup(input);
    
    for (size_t i = 0; i < ctx->macro_count; i++) {
        Macro *macro = ctx->macros[i];
        // Basic string replacement (simplified)
        // In a real implementation, this would be more sophisticated
        char *pos = strstr(result, macro->pattern);
        if (pos) {
            // Replace pattern with replacement
            size_t pattern_len = strlen(macro->pattern);
            size_t replacement_len = strlen(macro->replacement);
            size_t result_len = strlen(result);
            
            char *new_result = malloc(result_len - pattern_len + replacement_len + 1);
            size_t before_len = pos - result;
            strncpy(new_result, result, before_len);
            strcpy(new_result + before_len, macro->replacement);
            strcpy(new_result + before_len + replacement_len, pos + pattern_len);
            
            free(result);
            result = new_result;
        }
    }
    
    return result;
} 