#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>

// Token types for Rust syntax
typedef enum {
    // Literals
    TOKEN_IDENTIFIER,
    TOKEN_INTEGER_LITERAL,
    TOKEN_FLOAT_LITERAL,
    TOKEN_STRING_LITERAL,
    TOKEN_CHAR_LITERAL,
    TOKEN_BOOL_LITERAL,
    
    // Keywords
    TOKEN_FN,
    TOKEN_LET,
    TOKEN_MUT,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_LOOP,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_IN,
    TOKEN_MATCH,
    TOKEN_RETURN,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
    TOKEN_STRUCT,
    TOKEN_ENUM,
    TOKEN_IMPL,
    TOKEN_TRAIT,
    TOKEN_USE,
    TOKEN_PUB,
    TOKEN_CRATE,
    TOKEN_MOD,
    TOKEN_AS,
    TOKEN_WHERE,
    TOKEN_TYPE,
    TOKEN_CONST,
    TOKEN_STATIC,
    TOKEN_UNSAFE,
    TOKEN_EXTERN,
    TOKEN_MOVE,
    TOKEN_REF,
    TOKEN_BOX,
    TOKEN_SELF,
    TOKEN_SUPER,
    TOKEN_TRUE,
    TOKEN_FALSE,
    
    // Operators
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_PERCENT,
    TOKEN_ASSIGN,
    TOKEN_PLUS_ASSIGN,
    TOKEN_MINUS_ASSIGN,
    TOKEN_STAR_ASSIGN,
    TOKEN_SLASH_ASSIGN,
    TOKEN_PERCENT_ASSIGN,
    TOKEN_EQ,
    TOKEN_NE,
    TOKEN_LT,
    TOKEN_GT,
    TOKEN_LE,
    TOKEN_GE,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT,
    TOKEN_AND_AND,
    TOKEN_OR_OR,
    TOKEN_XOR,
    TOKEN_SHL,
    TOKEN_SHR,
    TOKEN_SHL_ASSIGN,
    TOKEN_SHR_ASSIGN,
    TOKEN_AND_ASSIGN,
    TOKEN_OR_ASSIGN,
    TOKEN_XOR_ASSIGN,
    
    // Delimiters
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_SEMICOLON,
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_ARROW,
    TOKEN_FAT_ARROW,
    TOKEN_AT,
    TOKEN_HASH,
    TOKEN_DOLLAR,
    TOKEN_QUESTION,
    TOKEN_BANG,
    TOKEN_TILDE,
    TOKEN_UNDERSCORE,
    
    // Special
    TOKEN_EOF,
    TOKEN_ERROR
} TokenType;

typedef struct {
    TokenType type;
    char *value;
    size_t line;
    size_t column;
} Token;

// Lexer state
typedef struct {
    const char *source;
    size_t position;
    size_t line;
    size_t column;
} Lexer;

// Function declarations
Lexer* lexer_init(const char *source);
void lexer_free(Lexer *lexer);
Token* get_next_token(Lexer *lexer);
void token_free(Token *token);
const char* token_type_to_string(TokenType type);

#endif // LEXER_H 