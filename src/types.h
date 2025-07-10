#ifndef TYPES_H
#define TYPES_H

#include "parser.h"
#include <stddef.h>
#include <stdbool.h>

// type kinds
typedef enum {
    TYPE_UNKNOWN,
    TYPE_UNIT,           // () - unit type
    TYPE_BOOL,           // bool
    TYPE_INT,            // i8, i16, i32, i64, i128, isize
    TYPE_UINT,           // u8, u16, u32, u64, u128, usize
    TYPE_FLOAT,          // f32, f64
    TYPE_CHAR,           // char
    TYPE_STRING,         // String, &str
    TYPE_ARRAY,          // [T; N]
    TYPE_SLICE,          // [T]
    TYPE_TUPLE,          // (T1, T2, ...)
    TYPE_STRUCT,         // struct types
    TYPE_ENUM,           // enum types
    TYPE_FUNCTION,       // fn(T1, T2, ...) -> T
    TYPE_REF,            // &T, &mut T
    TYPE_POINTER,        // *T, *mut T
    TYPE_GENERIC,        // T<U, V>
    TYPE_TRAIT_OBJECT    // dyn Trait
} TypeKind;

// type flags
typedef enum {
    TYPE_FLAG_NONE = 0,
    TYPE_FLAG_MUTABLE = 1 << 0,
    TYPE_FLAG_REFERENCE = 1 << 1,
    TYPE_FLAG_POINTER = 1 << 2,
    TYPE_FLAG_UNSAFE = 1 << 3,
    TYPE_FLAG_CONST = 1 << 4
} TypeFlags;

// type structure
typedef struct Type {
    TypeKind kind;
    TypeFlags flags;
    char *name;                    // type name (e.g., "i32", "String")
    size_t size;                   // size in bytes
    bool is_sized;                 // whether type has known size
    
    // for composite types
    struct Type **generic_args;     // generic type arguments
    size_t generic_arg_count;
    
    // for function types
    struct Type **param_types;      // parameter types
    size_t param_count;
    struct Type *return_type;       // return type
    
    // for struct/enum types
    char *struct_name;             // struct/enum name
    struct Type **field_types;      // field types
    char **field_names;            // field names
    size_t field_count;
    
    // for array types
    struct Type *element_type;      // element type
    size_t array_size;             // array size (0 for slices)
    
    // for reference types
    struct Type *referenced_type;   // type being referenced
    
    // for tuple types
    struct Type **tuple_types;      // tuple element types
    size_t tuple_size;
} Type;

// type environment (scope)
typedef struct TypeEnv {
    char **names;                  // variable names
    Type **types;                  // variable types
    size_t count;                  // number of variables
    size_t capacity;               // capacity of arrays
    struct TypeEnv *parent;        // parent scope
} TypeEnv;

// type inference context
typedef struct TypeContext {
    TypeEnv *env;                  // current type environment
    TypeEnv *global_env;           // global type environment
    bool had_error;                // error flag
    char *error_message;           // error message
    size_t error_line;             // error line
    size_t error_column;           // error column
} TypeContext;

// function declarations

// type creation and management
Type* type_create(TypeKind kind, const char *name);
Type* type_create_primitive(TypeKind kind, const char *name, size_t size);
Type* type_create_function(Type **param_types, size_t param_count, Type *return_type);
Type* type_create_reference(Type *referenced_type, bool is_mutable);
Type* type_create_pointer(Type *pointed_type, bool is_mutable);
Type* type_create_array(Type *element_type, size_t size);
Type* type_create_slice(Type *element_type);
Type* type_create_tuple(Type **types, size_t count);
Type* type_create_struct(const char *name, Type **field_types, char **field_names, size_t field_count);
Type* type_create_enum(const char *name, Type **variant_types, char **variant_names, size_t variant_count);
Type* type_create_generic(const char *name, Type **args, size_t arg_count);

void type_free(Type *type);
Type* type_clone(Type *type);
bool type_equals(Type *a, Type *b);
bool type_is_sized(Type *type);
bool type_is_copy(Type *type);

// type environment management
TypeEnv* type_env_create(void);
void type_env_free(TypeEnv *env);
TypeEnv* type_env_push(TypeEnv *parent);
TypeEnv* type_env_pop(TypeEnv *env);
bool type_env_insert(TypeEnv *env, const char *name, Type *type);
Type* type_env_lookup(TypeEnv *env, const char *name);
Type* type_env_lookup_global(TypeEnv *env, const char *name);

// type context management
TypeContext* type_context_create(void);
void type_context_free(TypeContext *ctx);
void type_context_error(TypeContext *ctx, const char *message, size_t line, size_t column);

// type checking functions
Type* type_check_expression(TypeContext *ctx, ASTNode *expr);
Type* type_check_statement(TypeContext *ctx, ASTNode *stmt);
Type* type_check_function(TypeContext *ctx, ASTNode *func);
Type* type_check_program(TypeContext *ctx, ASTNode *program);

// type inference functions
Type* type_infer_expression(TypeContext *ctx, ASTNode *expr);
Type* type_infer_literal(TypeContext *ctx, ASTNode *literal);
Type* type_infer_binary_op(TypeContext *ctx, ASTNode *op);
Type* type_infer_function_call(TypeContext *ctx, ASTNode *call);

// utility functions
const char* type_kind_to_string(TypeKind kind);
const char* type_to_string(Type *type);
void type_print(Type *type);
bool type_is_numeric(Type *type);
bool type_is_integer(Type *type);
bool type_is_float(Type *type);
bool type_is_boolean(Type *type);
bool type_is_unit(Type *type);
bool type_is_reference(Type *type);
bool type_is_pointer(Type *type);

// built-in types
extern Type *TYPE_UNIT;
extern Type *TYPE_BOOL;
extern Type *TYPE_I8, *TYPE_I16, *TYPE_I32, *TYPE_I64, *TYPE_ISIZE;
extern Type *TYPE_U8, *TYPE_U16, *TYPE_U32, *TYPE_U64, *TYPE_USIZE;
extern Type *TYPE_F32, *TYPE_F64;
extern Type *TYPE_CHAR;
extern Type *TYPE_STRING, *TYPE_STR;

// type initialization
void types_init(void);
void types_cleanup(void);

#endif // TYPES_H 