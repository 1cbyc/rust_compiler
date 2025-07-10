#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Keyword mapping
typedef struct {
    const char *keyword;
    TokenType type;
} Keyword;

static const Keyword keywords[] = {
    {"fn", TOKEN_FN},
    {"let", TOKEN_LET},
    {"mut", TOKEN_MUT},
    {"if", TOKEN_IF},
    {"else", TOKEN_ELSE},
    {"loop", TOKEN_LOOP},
    {"while", TOKEN_WHILE},
    {"for", TOKEN_FOR},
    {"in", TOKEN_IN},
    {"match", TOKEN_MATCH},
    {"return", TOKEN_RETURN},
    {"break", TOKEN_BREAK},
    {"continue", TOKEN_CONTINUE},
    {"struct", TOKEN_STRUCT},
    {"enum", TOKEN_ENUM},
    {"impl", TOKEN_IMPL},
    {"trait", TOKEN_TRAIT},
    {"use", TOKEN_USE},
    {"pub", TOKEN_PUB},
    {"crate", TOKEN_CRATE},
    {"mod", TOKEN_MOD},
    {"as", TOKEN_AS},
    {"where", TOKEN_WHERE},
    {"type", TOKEN_TYPE},
    {"const", TOKEN_CONST},
    {"static", TOKEN_STATIC},
    {"unsafe", TOKEN_UNSAFE},
    {"extern", TOKEN_EXTERN},
    {"move", TOKEN_MOVE},
    {"ref", TOKEN_REF},
    {"box", TOKEN_BOX},
    {"self", TOKEN_SELF},
    {"super", TOKEN_SUPER},
    {"true", TOKEN_TRUE},
    {"false", TOKEN_FALSE},
    {NULL, TOKEN_ERROR}
};

Lexer* lexer_init(const char *source) {
    Lexer *lexer = malloc(sizeof(Lexer));
    if (!lexer) return NULL;
    
    lexer->source = source;
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
    
    return lexer;
}

void lexer_free(Lexer *lexer) {
    if (lexer) {
        free((void*)lexer);
    }
}

static char peek_char(Lexer *lexer) {
    return lexer->source[lexer->position];
}

static char peek_next_char(Lexer *lexer) {
    return lexer->source[lexer->position + 1];
}

static char advance_char(Lexer *lexer) {
    char c = peek_char(lexer);
    if (c == '\n') {
        lexer->line++;
        lexer->column = 1;
    } else {
        lexer->column++;
    }
    lexer->position++;
    return c;
}

static void skip_whitespace(Lexer *lexer) {
    while (isspace(peek_char(lexer))) {
        advance_char(lexer);
    }
}

static void skip_comment(Lexer *lexer) {
    char c = peek_char(lexer);
    if (c == '/' && peek_next_char(lexer) == '/') {
        // Single line comment
        while (peek_char(lexer) != '\n' && peek_char(lexer) != '\0') {
            advance_char(lexer);
        }
    } else if (c == '/' && peek_next_char(lexer) == '*') {
        // Multi-line comment
        advance_char(lexer); // consume '/'
        advance_char(lexer); // consume '*'
        while (peek_char(lexer) != '\0') {
            if (peek_char(lexer) == '*' && peek_next_char(lexer) == '/') {
                advance_char(lexer); // consume '*'
                advance_char(lexer); // consume '/'
                break;
            }
            advance_char(lexer);
        }
    }
}

static Token* create_token(TokenType type, const char *value, size_t line, size_t column) {
    Token *token = malloc(sizeof(Token));
    if (!token) return NULL;
    
    token->type = type;
    token->value = value ? strdup(value) : NULL;
    token->line = line;
    token->column = column;
    
    return token;
}

static Token* read_identifier(Lexer *lexer) {
    size_t start = lexer->position;
    size_t start_line = lexer->line;
    size_t start_column = lexer->column;
    
    while (isalnum(peek_char(lexer)) || peek_char(lexer) == '_') {
        advance_char(lexer);
    }
    
    size_t length = lexer->position - start;
    char *value = malloc(length + 1);
    strncpy(value, lexer->source + start, length);
    value[length] = '\0';
    
    // Check if it's a keyword
    for (int i = 0; keywords[i].keyword != NULL; i++) {
        if (strcmp(value, keywords[i].keyword) == 0) {
            free(value);
            return create_token(keywords[i].type, keywords[i].keyword, start_line, start_column);
        }
    }
    
    return create_token(TOKEN_IDENTIFIER, value, start_line, start_column);
}

static Token* read_number(Lexer *lexer) {
    size_t start = lexer->position;
    size_t start_line = lexer->line;
    size_t start_column = lexer->column;
    bool is_float = false;
    
    // Read integer part
    while (isdigit(peek_char(lexer))) {
        advance_char(lexer);
    }
    
    // Check for decimal point
    if (peek_char(lexer) == '.') {
        is_float = true;
        advance_char(lexer);
        
        // Read fractional part
        while (isdigit(peek_char(lexer))) {
            advance_char(lexer);
        }
    }
    
    // Check for exponent
    if (peek_char(lexer) == 'e' || peek_char(lexer) == 'E') {
        is_float = true;
        advance_char(lexer);
        
        // Optional sign
        if (peek_char(lexer) == '+' || peek_char(lexer) == '-') {
            advance_char(lexer);
        }
        
        // Exponent digits
        while (isdigit(peek_char(lexer))) {
            advance_char(lexer);
        }
    }
    
    size_t length = lexer->position - start;
    char *value = malloc(length + 1);
    strncpy(value, lexer->source + start, length);
    value[length] = '\0';
    
    TokenType type = is_float ? TOKEN_FLOAT_LITERAL : TOKEN_INTEGER_LITERAL;
    return create_token(type, value, start_line, start_column);
}

static Token* read_string(Lexer *lexer) {
    size_t start_line = lexer->line;
    size_t start_column = lexer->column;
    
    advance_char(lexer); // consume opening quote
    
    size_t start = lexer->position;
    while (peek_char(lexer) != '"' && peek_char(lexer) != '\0') {
        if (peek_char(lexer) == '\\') {
            advance_char(lexer); // consume backslash
        }
        advance_char(lexer);
    }
    
    if (peek_char(lexer) == '\0') {
        return create_token(TOKEN_ERROR, "Unterminated string", start_line, start_column);
    }
    
    size_t length = lexer->position - start;
    char *value = malloc(length + 1);
    strncpy(value, lexer->source + start, length);
    value[length] = '\0';
    
    advance_char(lexer); // consume closing quote
    
    return create_token(TOKEN_STRING_LITERAL, value, start_line, start_column);
}

static Token* read_char(Lexer *lexer) {
    size_t start_line = lexer->line;
    size_t start_column = lexer->column;
    
    advance_char(lexer); // consume opening quote
    
    if (peek_char(lexer) == '\\') {
        advance_char(lexer); // consume backslash
        advance_char(lexer); // consume escaped char
    } else {
        advance_char(lexer); // consume char
    }
    
    if (peek_char(lexer) != '\'') {
        return create_token(TOKEN_ERROR, "Unterminated char", start_line, start_column);
    }
    
    advance_char(lexer); // consume closing quote
    
    return create_token(TOKEN_CHAR_LITERAL, "'", start_line, start_column);
}

Token* get_next_token(Lexer *lexer) {
    if (!lexer) return NULL;
    
    skip_whitespace(lexer);
    skip_comment(lexer);
    skip_whitespace(lexer);
    
    if (peek_char(lexer) == '\0') {
        return create_token(TOKEN_EOF, NULL, lexer->line, lexer->column);
    }
    
    char c = peek_char(lexer);
    size_t current_line = lexer->line;
    size_t current_column = lexer->column;
    
    // Identifiers and keywords
    if (isalpha(c) || c == '_') {
        return read_identifier(lexer);
    }
    
    // Numbers
    if (isdigit(c)) {
        return read_number(lexer);
    }
    
    // Strings
    if (c == '"') {
        return read_string(lexer);
    }
    
    // Characters
    if (c == '\'') {
        return read_char(lexer);
    }
    
    // Two-character operators
    if (c == '=' && peek_next_char(lexer) == '=') {
        advance_char(lexer);
        advance_char(lexer);
        return create_token(TOKEN_EQ, "==", current_line, current_column);
    }
    if (c == '!' && peek_next_char(lexer) == '=') {
        advance_char(lexer);
        advance_char(lexer);
        return create_token(TOKEN_NE, "!=", current_line, current_column);
    }
    if (c == '<' && peek_next_char(lexer) == '=') {
        advance_char(lexer);
        advance_char(lexer);
        return create_token(TOKEN_LE, "<=", current_line, current_column);
    }
    if (c == '>' && peek_next_char(lexer) == '=') {
        advance_char(lexer);
        advance_char(lexer);
        return create_token(TOKEN_GE, ">=", current_line, current_column);
    }
    if (c == '+' && peek_next_char(lexer) == '=') {
        advance_char(lexer);
        advance_char(lexer);
        return create_token(TOKEN_PLUS_ASSIGN, "+=", current_line, current_column);
    }
    if (c == '-' && peek_next_char(lexer) == '=') {
        advance_char(lexer);
        advance_char(lexer);
        return create_token(TOKEN_MINUS_ASSIGN, "-=", current_line, current_column);
    }
    if (c == '*' && peek_next_char(lexer) == '=') {
        advance_char(lexer);
        advance_char(lexer);
        return create_token(TOKEN_STAR_ASSIGN, "*=", current_line, current_column);
    }
    if (c == '/' && peek_next_char(lexer) == '=') {
        advance_char(lexer);
        advance_char(lexer);
        return create_token(TOKEN_SLASH_ASSIGN, "/=", current_line, current_column);
    }
    if (c == '%' && peek_next_char(lexer) == '=') {
        advance_char(lexer);
        advance_char(lexer);
        return create_token(TOKEN_PERCENT_ASSIGN, "%=", current_line, current_column);
    }
    if (c == '&' && peek_next_char(lexer) == '&') {
        advance_char(lexer);
        advance_char(lexer);
        return create_token(TOKEN_AND_AND, "&&", current_line, current_column);
    }
    if (c == '|' && peek_next_char(lexer) == '|') {
        advance_char(lexer);
        advance_char(lexer);
        return create_token(TOKEN_OR_OR, "||", current_line, current_column);
    }
    if (c == '-' && peek_next_char(lexer) == '>') {
        advance_char(lexer);
        advance_char(lexer);
        return create_token(TOKEN_ARROW, "->", current_line, current_column);
    }
    if (c == '=' && peek_next_char(lexer) == '>') {
        advance_char(lexer);
        advance_char(lexer);
        return create_token(TOKEN_FAT_ARROW, "=>", current_line, current_column);
    }
    if (c == '<' && peek_next_char(lexer) == '<') {
        advance_char(lexer);
        advance_char(lexer);
        return create_token(TOKEN_SHL, "<<", current_line, current_column);
    }
    if (c == '>' && peek_next_char(lexer) == '>') {
        advance_char(lexer);
        advance_char(lexer);
        return create_token(TOKEN_SHR, ">>", current_line, current_column);
    }
    
    // Single character tokens
    advance_char(lexer);
    char token_str[2] = {c, '\0'};
    
    switch (c) {
        case '(': return create_token(TOKEN_LPAREN, "(", current_line, current_column);
        case ')': return create_token(TOKEN_RPAREN, ")", current_line, current_column);
        case '{': return create_token(TOKEN_LBRACE, "{", current_line, current_column);
        case '}': return create_token(TOKEN_RBRACE, "}", current_line, current_column);
        case '[': return create_token(TOKEN_LBRACKET, "[", current_line, current_column);
        case ']': return create_token(TOKEN_RBRACKET, "]", current_line, current_column);
        case ';': return create_token(TOKEN_SEMICOLON, ";", current_line, current_column);
        case ':': return create_token(TOKEN_COLON, ":", current_line, current_column);
        case ',': return create_token(TOKEN_COMMA, ",", current_line, current_column);
        case '.': return create_token(TOKEN_DOT, ".", current_line, current_column);
        case '@': return create_token(TOKEN_AT, "@", current_line, current_column);
        case '#': return create_token(TOKEN_HASH, "#", current_line, current_column);
        case '$': return create_token(TOKEN_DOLLAR, "$", current_line, current_column);
        case '?': return create_token(TOKEN_QUESTION, "?", current_line, current_column);
        case '!': return create_token(TOKEN_BANG, "!", current_line, current_column);
        case '~': return create_token(TOKEN_TILDE, "~", current_line, current_column);
        case '_': return create_token(TOKEN_UNDERSCORE, "_", current_line, current_column);
        case '+': return create_token(TOKEN_PLUS, "+", current_line, current_column);
        case '-': return create_token(TOKEN_MINUS, "-", current_line, current_column);
        case '*': return create_token(TOKEN_STAR, "*", current_line, current_column);
        case '/': return create_token(TOKEN_SLASH, "/", current_line, current_column);
        case '%': return create_token(TOKEN_PERCENT, "%", current_line, current_column);
        case '=': return create_token(TOKEN_ASSIGN, "=", current_line, current_column);
        case '<': return create_token(TOKEN_LT, "<", current_line, current_column);
        case '>': return create_token(TOKEN_GT, ">", current_line, current_column);
        case '&': return create_token(TOKEN_AND, "&", current_line, current_column);
        case '|': return create_token(TOKEN_OR, "|", current_line, current_column);
        case '^': return create_token(TOKEN_XOR, "^", current_line, current_column);
        default:
            return create_token(TOKEN_ERROR, token_str, current_line, current_column);
    }
}

void token_free(Token *token) {
    if (token) {
        if (token->value) {
            free(token->value);
        }
        free(token);
    }
}

const char* token_type_to_string(TokenType type) {
    switch (type) {
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_INTEGER_LITERAL: return "INTEGER_LITERAL";
        case TOKEN_FLOAT_LITERAL: return "FLOAT_LITERAL";
        case TOKEN_STRING_LITERAL: return "STRING_LITERAL";
        case TOKEN_CHAR_LITERAL: return "CHAR_LITERAL";
        case TOKEN_BOOL_LITERAL: return "BOOL_LITERAL";
        case TOKEN_FN: return "FN";
        case TOKEN_LET: return "LET";
        case TOKEN_MUT: return "MUT";
        case TOKEN_IF: return "IF";
        case TOKEN_ELSE: return "ELSE";
        case TOKEN_LOOP: return "LOOP";
        case TOKEN_WHILE: return "WHILE";
        case TOKEN_FOR: return "FOR";
        case TOKEN_IN: return "IN";
        case TOKEN_MATCH: return "MATCH";
        case TOKEN_RETURN: return "RETURN";
        case TOKEN_BREAK: return "BREAK";
        case TOKEN_CONTINUE: return "CONTINUE";
        case TOKEN_STRUCT: return "STRUCT";
        case TOKEN_ENUM: return "ENUM";
        case TOKEN_IMPL: return "IMPL";
        case TOKEN_TRAIT: return "TRAIT";
        case TOKEN_USE: return "USE";
        case TOKEN_PUB: return "PUB";
        case TOKEN_CRATE: return "CRATE";
        case TOKEN_MOD: return "MOD";
        case TOKEN_AS: return "AS";
        case TOKEN_WHERE: return "WHERE";
        case TOKEN_TYPE: return "TYPE";
        case TOKEN_CONST: return "CONST";
        case TOKEN_STATIC: return "STATIC";
        case TOKEN_UNSAFE: return "UNSAFE";
        case TOKEN_EXTERN: return "EXTERN";
        case TOKEN_MOVE: return "MOVE";
        case TOKEN_REF: return "REF";
        case TOKEN_BOX: return "BOX";
        case TOKEN_SELF: return "SELF";
        case TOKEN_SUPER: return "SUPER";
        case TOKEN_TRUE: return "TRUE";
        case TOKEN_FALSE: return "FALSE";
        case TOKEN_PLUS: return "PLUS";
        case TOKEN_MINUS: return "MINUS";
        case TOKEN_STAR: return "STAR";
        case TOKEN_SLASH: return "SLASH";
        case TOKEN_PERCENT: return "PERCENT";
        case TOKEN_ASSIGN: return "ASSIGN";
        case TOKEN_PLUS_ASSIGN: return "PLUS_ASSIGN";
        case TOKEN_MINUS_ASSIGN: return "MINUS_ASSIGN";
        case TOKEN_STAR_ASSIGN: return "STAR_ASSIGN";
        case TOKEN_SLASH_ASSIGN: return "SLASH_ASSIGN";
        case TOKEN_PERCENT_ASSIGN: return "PERCENT_ASSIGN";
        case TOKEN_EQ: return "EQ";
        case TOKEN_NE: return "NE";
        case TOKEN_LT: return "LT";
        case TOKEN_GT: return "GT";
        case TOKEN_LE: return "LE";
        case TOKEN_GE: return "GE";
        case TOKEN_AND: return "AND";
        case TOKEN_OR: return "OR";
        case TOKEN_NOT: return "NOT";
        case TOKEN_AND_AND: return "AND_AND";
        case TOKEN_OR_OR: return "OR_OR";
        case TOKEN_XOR: return "XOR";
        case TOKEN_SHL: return "SHL";
        case TOKEN_SHR: return "SHR";
        case TOKEN_SHL_ASSIGN: return "SHL_ASSIGN";
        case TOKEN_SHR_ASSIGN: return "SHR_ASSIGN";
        case TOKEN_AND_ASSIGN: return "AND_ASSIGN";
        case TOKEN_OR_ASSIGN: return "OR_ASSIGN";
        case TOKEN_XOR_ASSIGN: return "XOR_ASSIGN";
        case TOKEN_LPAREN: return "LPAREN";
        case TOKEN_RPAREN: return "RPAREN";
        case TOKEN_LBRACE: return "LBRACE";
        case TOKEN_RBRACE: return "RBRACE";
        case TOKEN_LBRACKET: return "LBRACKET";
        case TOKEN_RBRACKET: return "RBRACKET";
        case TOKEN_SEMICOLON: return "SEMICOLON";
        case TOKEN_COLON: return "COLON";
        case TOKEN_COMMA: return "COMMA";
        case TOKEN_DOT: return "DOT";
        case TOKEN_ARROW: return "ARROW";
        case TOKEN_FAT_ARROW: return "FAT_ARROW";
        case TOKEN_AT: return "AT";
        case TOKEN_HASH: return "HASH";
        case TOKEN_DOLLAR: return "DOLLAR";
        case TOKEN_QUESTION: return "QUESTION";
        case TOKEN_BANG: return "BANG";
        case TOKEN_TILDE: return "TILDE";
        case TOKEN_UNDERSCORE: return "UNDERSCORE";
        case TOKEN_EOF: return "EOF";
        case TOKEN_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
} 