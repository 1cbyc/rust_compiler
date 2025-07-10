#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include <stddef.h>

// ast node types
typedef enum {
    NODE_PROGRAM,
    NODE_FUNCTION,
    NODE_VARIABLE_DECL,
    NODE_EXPRESSION,
    NODE_LITERAL,
    NODE_BINARY_OP,
    NODE_UNARY_OP,
    NODE_IDENTIFIER,
    NODE_FUNCTION_CALL,
    NODE_BLOCK,
    NODE_IF_STATEMENT,
    NODE_WHILE_STATEMENT,
    NODE_FOR_STATEMENT,
    NODE_RETURN_STATEMENT,
    NODE_STRUCT_DEF,
    NODE_STRUCT_FIELD,
    NODE_ENUM_DEF,
    NODE_ENUM_VARIANT,
    NODE_IMPL_BLOCK,
    NODE_TYPE_ANNOTATION
} NodeType;

// expression types
typedef enum {
    EXPR_LITERAL,
    EXPR_IDENTIFIER,
    EXPR_BINARY_OP,
    EXPR_UNARY_OP,
    EXPR_FUNCTION_CALL,
    EXPR_BLOCK
} ExprType;

// statement types
typedef enum {
    STMT_EXPRESSION,
    STMT_VARIABLE_DECL,
    STMT_FUNCTION_DECL,
    STMT_IF,
    STMT_WHILE,
    STMT_FOR,
    STMT_RETURN,
    STMT_BLOCK
} StmtType;

// binary operator types
typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_ASSIGN,
    OP_EQ,
    OP_NE,
    OP_LT,
    OP_GT,
    OP_LE,
    OP_GE,
    OP_AND,
    OP_OR,
    OP_XOR,
    OP_SHL,
    OP_SHR
} BinaryOp;

// unary operator types
typedef enum {
    UOP_PLUS,
    UOP_MINUS,
    UOP_NOT,
    UOP_DEREF,
    UOP_REF
} UnaryOp;

// ast node structure
typedef struct ASTNode {
    NodeType type;
    size_t line;
    size_t column;
    
    union {
        // program node
        struct {
            struct ASTNode **statements;
            size_t statement_count;
        } program;
        
        // function node
        struct {
            char *name;
            struct ASTNode **parameters;
            size_t param_count;
            struct ASTNode *return_type;
            struct ASTNode *body;
        } function;
        
        // variable declaration
        struct {
            char *name;
            struct ASTNode *type_annotation;
            struct ASTNode *initializer;
            bool is_mutable;
        } variable_decl;
        
        // expression node
        struct {
            ExprType expr_type;
            union {
                // literal
                struct {
                    TokenType token_type;
                    char *value;
                } literal;
                
                // identifier
                char *identifier;
                
                // binary operation
                struct {
                    BinaryOp op;
                    struct ASTNode *left;
                    struct ASTNode *right;
                } binary_op;
                
                // unary operation
                struct {
                    UnaryOp op;
                    struct ASTNode *operand;
                } unary_op;
                
                // function call
                struct {
                    char *function_name;
                    struct ASTNode **arguments;
                    size_t arg_count;
                } function_call;
                
                // block expression
                struct {
                    struct ASTNode **statements;
                    size_t statement_count;
                } block;
            } expr;
        } expression;
        
        // statement node
        struct {
            StmtType stmt_type;
            struct ASTNode *stmt;
        } statement;
        
        // if statement
        struct {
            struct ASTNode *condition;
            struct ASTNode *then_branch;
            struct ASTNode *else_branch;
        } if_stmt;
        
        // while statement
        struct {
            struct ASTNode *condition;
            struct ASTNode *body;
        } while_stmt;
        
        // for statement
        struct {
            char *iterator;
            struct ASTNode *range;
            struct ASTNode *body;
        } for_stmt;
        
        // return statement
        struct {
            struct ASTNode *value;
        } return_stmt;
        
        // struct definition
        struct {
            char *name;
            struct ASTNode **fields;
            size_t field_count;
        } struct_def;
        
        // struct field
        struct {
            char *name;
            struct ASTNode *type;
        } struct_field;
        
        // impl block
        struct {
            char *type_name;
            struct ASTNode **methods;
            size_t method_count;
        } impl_block;
        
        // type annotation
        struct {
            char *type_name;
            struct ASTNode *generic_type;
        } type_annotation;

        // enum definition
        struct {
            char *name;
            struct ASTNode **variants;
            size_t variant_count;
        } enum_def;
        
        // enum variant
        struct {
            char *name;
            struct ASTNode **fields;
            size_t field_count;
        } enum_variant;
    } data;
} ASTNode;

// parser state
typedef struct {
    Lexer *lexer;
    Token *current_token;
    Token *peek_token;
    bool had_error;
} Parser;

// function declarations
Parser* parser_init(Lexer *lexer);
void parser_free(Parser *parser);
ASTNode* parse_program(Parser *parser);
ASTNode* parse_statement(Parser *parser);
ASTNode* parse_expression(Parser *parser);
ASTNode* parse_primary_expression(Parser *parser);
ASTNode* parse_function_declaration(Parser *parser);
ASTNode* parse_variable_declaration(Parser *parser);
ASTNode* parse_if_statement(Parser *parser);
ASTNode* parse_while_statement(Parser *parser);
ASTNode* parse_for_statement(Parser *parser);
ASTNode* parse_return_statement(Parser *parser);
ASTNode* parse_struct_definition(Parser *parser);
ASTNode* parse_enum_definition(Parser *parser);
ASTNode* parse_impl_block(Parser *parser);

// utility functions
void ast_node_free(ASTNode *node);
void print_ast(ASTNode *node, int indent);
const char* node_type_to_string(NodeType type);
const char* binary_op_to_string(BinaryOp op);
const char* unary_op_to_string(UnaryOp op);

#endif // PARSER_H 