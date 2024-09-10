typedef enum {
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_KEYWORD,
    TOKEN_OPERATOR,
    //  i am out of ideas at the moment for the token types i will need, will search online for more if i see fit
} TokenType;

typedef struct {
    TokenType type;
    char *value;
} Token;

Token *get_next_token(const char *source_code);
//  there you have it, my hate is guaranteed