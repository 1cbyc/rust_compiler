#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// built-in types
Type *TYPE_UNIT;
Type *TYPE_BOOL;
Type *TYPE_I8, *TYPE_I16, *TYPE_I32, *TYPE_I64, *TYPE_ISIZE;
Type *TYPE_U8, *TYPE_U16, *TYPE_U32, *TYPE_U64, *TYPE_USIZE;
Type *TYPE_F32, *TYPE_F64;
Type *TYPE_CHAR;
Type *TYPE_STRING, *TYPE_STR;

// type creation functions
Type* type_create(TypeKind kind, const char *name) {
    Type *type = malloc(sizeof(Type));
    if (!type) return NULL;
    
    type->kind = kind;
    type->flags = TYPE_FLAG_NONE;
    type->name = name ? strdup(name) : NULL;
    type->size = 0;
    type->is_sized = true;
    
    // initialize all pointers to NULL
    type->generic_args = NULL;
    type->generic_arg_count = 0;
    type->param_types = NULL;
    type->param_count = 0;
    type->return_type = NULL;
    type->struct_name = NULL;
    type->field_types = NULL;
    type->field_names = NULL;
    type->field_count = 0;
    type->element_type = NULL;
    type->array_size = 0;
    type->referenced_type = NULL;
    type->tuple_types = NULL;
    type->tuple_size = 0;
    
    return type;
}

Type* type_create_primitive(TypeKind kind, const char *name, size_t size) {
    Type *type = type_create(kind, name);
    if (!type) return NULL;
    
    type->size = size;
    type->is_sized = true;
    
    return type;
}

Type* type_create_function(Type **param_types, size_t param_count, Type *return_type) {
    Type *type = type_create(TYPE_FUNCTION, "function");
    if (!type) return NULL;
    
    type->param_types = param_types;
    type->param_count = param_count;
    type->return_type = return_type;
    type->is_sized = false; // function types are unsized
    
    return type;
}

Type* type_create_reference(Type *referenced_type, bool is_mutable) {
    Type *type = type_create(TYPE_REF, "reference");
    if (!type) return NULL;
    
    type->referenced_type = referenced_type;
    type->flags = TYPE_FLAG_REFERENCE;
    if (is_mutable) {
        type->flags |= TYPE_FLAG_MUTABLE;
    }
    type->is_sized = true; // references are always sized
    
    return type;
}

Type* type_create_pointer(Type *pointed_type, bool is_mutable) {
    Type *type = type_create(TYPE_POINTER, "pointer");
    if (!type) return NULL;
    
    type->referenced_type = pointed_type;
    type->flags = TYPE_FLAG_POINTER;
    if (is_mutable) {
        type->flags |= TYPE_FLAG_MUTABLE;
    }
    type->is_sized = true; // pointers are always sized
    
    return type;
}

Type* type_create_array(Type *element_type, size_t size) {
    Type *type = type_create(TYPE_ARRAY, "array");
    if (!type) return NULL;
    
    type->element_type = element_type;
    type->array_size = size;
    type->is_sized = true;
    type->size = element_type ? element_type->size * size : 0;
    
    return type;
}

Type* type_create_slice(Type *element_type) {
    Type *type = type_create(TYPE_SLICE, "slice");
    if (!type) return NULL;
    
    type->element_type = element_type;
    type->array_size = 0; // slices are unsized
    type->is_sized = false;
    
    return type;
}

Type* type_create_tuple(Type **types, size_t count) {
    Type *type = type_create(TYPE_TUPLE, "tuple");
    if (!type) return NULL;
    
    type->tuple_types = types;
    type->tuple_size = count;
    type->is_sized = true;
    
    // calculate total size
    size_t total_size = 0;
    for (size_t i = 0; i < count; i++) {
        if (types[i] && types[i]->is_sized) {
            total_size += types[i]->size;
        }
    }
    type->size = total_size;
    
    return type;
}

Type* type_create_struct(const char *name, Type **field_types, char **field_names, size_t field_count) {
    Type *type = type_create(TYPE_STRUCT, "struct");
    if (!type) return NULL;
    
    type->struct_name = strdup(name);
    type->field_types = field_types;
    type->field_names = field_names;
    type->field_count = field_count;
    type->is_sized = true;
    
    // calculate total size
    size_t total_size = 0;
    for (size_t i = 0; i < field_count; i++) {
        if (field_types[i] && field_types[i]->is_sized) {
            total_size += field_types[i]->size;
        }
    }
    type->size = total_size;
    
    return type;
}

Type* type_create_enum(const char *name, Type **variant_types, char **variant_names, size_t variant_count) {
    Type *type = type_create(TYPE_ENUM, "enum");
    if (!type) return NULL;
    
    type->struct_name = strdup(name);
    type->field_types = variant_types;
    type->field_names = variant_names;
    type->field_count = variant_count;
    type->is_sized = true;
    
    // enum size is the size of the largest variant
    size_t max_size = 0;
    for (size_t i = 0; i < variant_count; i++) {
        if (variant_types[i] && variant_types[i]->is_sized && variant_types[i]->size > max_size) {
            max_size = variant_types[i]->size;
        }
    }
    type->size = max_size;
    
    return type;
}

Type* type_create_generic(const char *name, Type **args, size_t arg_count) {
    Type *type = type_create(TYPE_GENERIC, name);
    if (!type) return NULL;
    
    type->generic_args = args;
    type->generic_arg_count = arg_count;
    
    return type;
}

// type management functions
void type_free(Type *type) {
    if (!type) return;
    
    free(type->name);
    free(type->struct_name);
    
    // free generic args
    for (size_t i = 0; i < type->generic_arg_count; i++) {
        type_free(type->generic_args[i]);
    }
    free(type->generic_args);
    
    // free param types
    for (size_t i = 0; i < type->param_count; i++) {
        type_free(type->param_types[i]);
    }
    free(type->param_types);
    
    // free return type
    type_free(type->return_type);
    
    // free field types and names
    for (size_t i = 0; i < type->field_count; i++) {
        type_free(type->field_types[i]);
        free(type->field_names[i]);
    }
    free(type->field_types);
    free(type->field_names);
    
    // free element type
    type_free(type->element_type);
    
    // free referenced type
    type_free(type->referenced_type);
    
    // free tuple types
    for (size_t i = 0; i < type->tuple_size; i++) {
        type_free(type->tuple_types[i]);
    }
    free(type->tuple_types);
    
    free(type);
}

Type* type_clone(Type *type) {
    if (!type) return NULL;
    
    Type *clone = type_create(type->kind, type->name);
    if (!clone) return NULL;
    
    clone->flags = type->flags;
    clone->size = type->size;
    clone->is_sized = type->is_sized;
    
    // clone generic args
    if (type->generic_arg_count > 0) {
        clone->generic_args = malloc(type->generic_arg_count * sizeof(Type*));
        for (size_t i = 0; i < type->generic_arg_count; i++) {
            clone->generic_args[i] = type_clone(type->generic_args[i]);
        }
        clone->generic_arg_count = type->generic_arg_count;
    }
    
    // clone param types
    if (type->param_count > 0) {
        clone->param_types = malloc(type->param_count * sizeof(Type*));
        for (size_t i = 0; i < type->param_count; i++) {
            clone->param_types[i] = type_clone(type->param_types[i]);
        }
        clone->param_count = type->param_count;
    }
    
    // clone return type
    clone->return_type = type_clone(type->return_type);
    
    // clone struct name
    if (type->struct_name) {
        clone->struct_name = strdup(type->struct_name);
    }
    
    // clone field types and names
    if (type->field_count > 0) {
        clone->field_types = malloc(type->field_count * sizeof(Type*));
        clone->field_names = malloc(type->field_count * sizeof(char*));
        for (size_t i = 0; i < type->field_count; i++) {
            clone->field_types[i] = type_clone(type->field_types[i]);
            clone->field_names[i] = strdup(type->field_names[i]);
        }
        clone->field_count = type->field_count;
    }
    
    // clone element type
    clone->element_type = type_clone(type->element_type);
    clone->array_size = type->array_size;
    
    // clone referenced type
    clone->referenced_type = type_clone(type->referenced_type);
    
    // clone tuple types
    if (type->tuple_size > 0) {
        clone->tuple_types = malloc(type->tuple_size * sizeof(Type*));
        for (size_t i = 0; i < type->tuple_size; i++) {
            clone->tuple_types[i] = type_clone(type->tuple_types[i]);
        }
        clone->tuple_size = type->tuple_size;
    }
    
    return clone;
}

bool type_equals(Type *a, Type *b) {
    if (!a || !b) return a == b;
    
    if (a->kind != b->kind) return false;
    if (a->flags != b->flags) return false;
    
    switch (a->kind) {
        case TYPE_UNIT:
        case TYPE_BOOL:
        case TYPE_INT:
        case TYPE_UINT:
        case TYPE_FLOAT:
        case TYPE_CHAR:
            return strcmp(a->name, b->name) == 0;
            
        case TYPE_FUNCTION:
            if (a->param_count != b->param_count) return false;
            if (!type_equals(a->return_type, b->return_type)) return false;
            for (size_t i = 0; i < a->param_count; i++) {
                if (!type_equals(a->param_types[i], b->param_types[i])) return false;
            }
            return true;
            
        case TYPE_REF:
        case TYPE_POINTER:
            return type_equals(a->referenced_type, b->referenced_type);
            
        case TYPE_ARRAY:
        case TYPE_SLICE:
            if (a->array_size != b->array_size) return false;
            return type_equals(a->element_type, b->element_type);
            
        case TYPE_TUPLE:
            if (a->tuple_size != b->tuple_size) return false;
            for (size_t i = 0; i < a->tuple_size; i++) {
                if (!type_equals(a->tuple_types[i], b->tuple_types[i])) return false;
            }
            return true;
            
        case TYPE_STRUCT:
        case TYPE_ENUM:
            return strcmp(a->struct_name, b->struct_name) == 0;
            
        default:
            return false;
    }
}

bool type_is_sized(Type *type) {
    return type && type->is_sized;
}

bool type_is_copy(Type *type) {
    if (!type) return false;
    
    switch (type->kind) {
        case TYPE_UNIT:
        case TYPE_BOOL:
        case TYPE_INT:
        case TYPE_UINT:
        case TYPE_FLOAT:
        case TYPE_CHAR:
            return true;
            
        case TYPE_REF:
        case TYPE_POINTER:
            return true;
            
        case TYPE_ARRAY:
            return type_is_copy(type->element_type);
            
        case TYPE_TUPLE:
            for (size_t i = 0; i < type->tuple_size; i++) {
                if (!type_is_copy(type->tuple_types[i])) return false;
            }
            return true;
            
        default:
            return false;
    }
}

// type environment management
TypeEnv* type_env_create(void) {
    TypeEnv *env = malloc(sizeof(TypeEnv));
    if (!env) return NULL;
    
    env->names = NULL;
    env->types = NULL;
    env->count = 0;
    env->capacity = 0;
    env->parent = NULL;
    
    return env;
}

void type_env_free(TypeEnv *env) {
    if (!env) return;
    
    for (size_t i = 0; i < env->count; i++) {
        free(env->names[i]);
        type_free(env->types[i]);
    }
    free(env->names);
    free(env->types);
    free(env);
}

TypeEnv* type_env_push(TypeEnv *parent) {
    TypeEnv *env = type_env_create();
    if (!env) return NULL;
    
    env->parent = parent;
    return env;
}

TypeEnv* type_env_pop(TypeEnv *env) {
    if (!env) return NULL;
    
    TypeEnv *parent = env->parent;
    type_env_free(env);
    return parent;
}

bool type_env_insert(TypeEnv *env, const char *name, Type *type) {
    if (!env || !name || !type) return false;
    
    // check if we need to expand capacity
    if (env->count >= env->capacity) {
        size_t new_capacity = env->capacity == 0 ? 8 : env->capacity * 2;
        char **new_names = realloc(env->names, new_capacity * sizeof(char*));
        Type **new_types = realloc(env->types, new_capacity * sizeof(Type*));
        
        if (!new_names || !new_types) return false;
        
        env->names = new_names;
        env->types = new_types;
        env->capacity = new_capacity;
    }
    
    env->names[env->count] = strdup(name);
    env->types[env->count] = type_clone(type);
    env->count++;
    
    return true;
}

Type* type_env_lookup(TypeEnv *env, const char *name) {
    if (!env || !name) return NULL;
    
    // search current scope
    for (size_t i = 0; i < env->count; i++) {
        if (strcmp(env->names[i], name) == 0) {
            return env->types[i];
        }
    }
    
    // search parent scope
    if (env->parent) {
        return type_env_lookup(env->parent, name);
    }
    
    return NULL;
}

Type* type_env_lookup_global(TypeEnv *env, const char *name) {
    if (!env || !name) return NULL;
    
    // search all scopes
    while (env) {
        for (size_t i = 0; i < env->count; i++) {
            if (strcmp(env->names[i], name) == 0) {
                return env->types[i];
            }
        }
        env = env->parent;
    }
    
    return NULL;
}

// type context management
TypeContext* type_context_create(void) {
    TypeContext *ctx = malloc(sizeof(TypeContext));
    if (!ctx) return NULL;
    
    ctx->env = type_env_create();
    ctx->global_env = type_env_create();
    ctx->had_error = false;
    ctx->error_message = NULL;
    ctx->error_line = 0;
    ctx->error_column = 0;
    
    return ctx;
}

void type_context_free(TypeContext *ctx) {
    if (!ctx) return;
    
    type_env_free(ctx->env);
    type_env_free(ctx->global_env);
    free(ctx->error_message);
    free(ctx);
}

void type_context_error(TypeContext *ctx, const char *message, size_t line, size_t column) {
    if (!ctx) return;
    
    ctx->had_error = true;
    free(ctx->error_message);
    ctx->error_message = strdup(message);
    ctx->error_line = line;
    ctx->error_column = column;
    
    // fprintf(stderr, "type error at line %zu, column %zu: %s\n", line, column, message);
}

// utility functions
const char* type_kind_to_string(TypeKind kind) {
    switch (kind) {
        case TYPE_UNKNOWN: return "unknown";
        case TYPE_UNIT: return "unit";
        case TYPE_BOOL: return "bool";
        case TYPE_INT: return "int";
        case TYPE_UINT: return "uint";
        case TYPE_FLOAT: return "float";
        case TYPE_CHAR: return "char";
        case TYPE_STRING: return "string";
        case TYPE_ARRAY: return "array";
        case TYPE_SLICE: return "slice";
        case TYPE_TUPLE: return "tuple";
        case TYPE_STRUCT: return "struct";
        case TYPE_ENUM: return "enum";
        case TYPE_FUNCTION: return "function";
        case TYPE_REF: return "reference";
        case TYPE_POINTER: return "pointer";
        case TYPE_GENERIC: return "generic";
        case TYPE_TRAIT_OBJECT: return "trait_object";
        default: return "unknown";
    }
}

const char* type_to_string(Type *type) {
    if (!type) return "unknown";
    
    static char buffer[256];
    snprintf(buffer, sizeof(buffer), "%s", type->name ? type->name : type_kind_to_string(type->kind));
    return buffer;
}

void type_print(Type *type) {
    if (!type) {
        printf("unknown");
        return;
    }
    
    printf("%s", type_to_string(type));
}

bool type_is_numeric(Type *type) {
    return type && (type->kind == TYPE_INT || type->kind == TYPE_UINT || type->kind == TYPE_FLOAT);
}

bool type_is_integer(Type *type) {
    return type && (type->kind == TYPE_INT || type->kind == TYPE_UINT);
}

bool type_is_float(Type *type) {
    return type && type->kind == TYPE_FLOAT;
}

bool type_is_boolean(Type *type) {
    return type && type->kind == TYPE_BOOL;
}

bool type_is_unit(Type *type) {
    return type && type->kind == TYPE_UNIT;
}

bool type_is_reference(Type *type) {
    return type && type->kind == TYPE_REF;
}

bool type_is_pointer(Type *type) {
    return type && type->kind == TYPE_POINTER;
}

// type initialization
void types_init(void) {
    // initialize built-in types
    TYPE_UNIT = type_create_primitive(TYPE_UNIT, "()", 0);
    TYPE_BOOL = type_create_primitive(TYPE_BOOL, "bool", 1);
    
    TYPE_I8 = type_create_primitive(TYPE_INT, "i8", 1);
    TYPE_I16 = type_create_primitive(TYPE_INT, "i16", 2);
    TYPE_I32 = type_create_primitive(TYPE_INT, "i32", 4);
    TYPE_I64 = type_create_primitive(TYPE_INT, "i64", 8);
    TYPE_ISIZE = type_create_primitive(TYPE_INT, "isize", 8);
    
    TYPE_U8 = type_create_primitive(TYPE_UINT, "u8", 1);
    TYPE_U16 = type_create_primitive(TYPE_UINT, "u16", 2);
    TYPE_U32 = type_create_primitive(TYPE_UINT, "u32", 4);
    TYPE_U64 = type_create_primitive(TYPE_UINT, "u64", 8);
    TYPE_USIZE = type_create_primitive(TYPE_UINT, "usize", 8);
    
    TYPE_F32 = type_create_primitive(TYPE_FLOAT, "f32", 4);
    TYPE_F64 = type_create_primitive(TYPE_FLOAT, "f64", 8);
    
    TYPE_CHAR = type_create_primitive(TYPE_CHAR, "char", 4);
    TYPE_STRING = type_create_primitive(TYPE_STRING, "String", 24);
    TYPE_STR = type_create_primitive(TYPE_STRING, "&str", 16);
}

void types_cleanup(void) {
    type_free(TYPE_UNIT);
    type_free(TYPE_BOOL);
    type_free(TYPE_I8);
    type_free(TYPE_I16);
    type_free(TYPE_I32);
    type_free(TYPE_I64);
    type_free(TYPE_ISIZE);
    type_free(TYPE_U8);
    type_free(TYPE_U16);
    type_free(TYPE_U32);
    type_free(TYPE_U64);
    type_free(TYPE_USIZE);
    type_free(TYPE_F32);
    type_free(TYPE_F64);
    type_free(TYPE_CHAR);
    type_free(TYPE_STRING);
    type_free(TYPE_STR);
} 