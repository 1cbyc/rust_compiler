// lexer.h
typedef enum {
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_KEYWORD,
    TOKEN_OPERATOR,
    // Add other token types as needed
} TokenType;

typedef struct {
    TokenType type;
    char *value;
} Token;

Token *get_next_token(const char *source_code);
