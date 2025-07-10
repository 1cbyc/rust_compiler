#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"

void test_lexer(const char *source_code) {
    printf("=== Testing Lexer ===\n");
    printf("Source code:\n%s\n\n", source_code);
    
    Lexer *lexer = lexer_init(source_code);
    if (!lexer) {
        printf("Failed to initialize lexer\n");
        return;
    }
    
    printf("Tokens:\n");
    Token *token;
    int token_count = 0;
    
    do {
        token = get_next_token(lexer);
        if (!token) {
            printf("Failed to get token\n");
            break;
        }
        
        printf("[%d] %s", token_count++, token_type_to_string(token->type));
        if (token->value) {
            printf(": '%s'", token->value);
        }
        printf(" (line %zu, col %zu)\n", token->line, token->column);
        
        if (token->type == TOKEN_ERROR) {
            printf("Lexer error: %s\n", token->value ? token->value : "Unknown error");
        }
        
        token_free(token);
    } while (token->type != TOKEN_EOF);
    
    lexer_free(lexer);
    printf("\n=== End of Lexer Test ===\n\n");
}

int main() {
    printf("Rust Compiler in C - Lexer Test\n");
    printf("================================\n\n");
    
    // Test 1: Simple Rust function
    const char *test1 = "fn main() {\n    let x = 42;\n    println!(\"Hello, world!\");\n}";
    test_lexer(test1);
    
    // Test 2: More complex Rust code
    const char *test2 = "struct Point {\n    x: i32,\n    y: i32,\n}\n\nimpl Point {\n    fn new(x: i32, y: i32) -> Self {\n        Point { x, y }\n    }\n}";
    test_lexer(test2);
    
    // Test 3: Comments and operators
    const char *test3 = "// This is a comment\nlet mut sum = 0;\nfor i in 0..10 {\n    sum += i; // Add to sum\n}";
    test_lexer(test3);
    
    return 0;
} 