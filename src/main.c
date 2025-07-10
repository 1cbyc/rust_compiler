#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"

void test_lexer(const char *source_code) {
    printf("=== testing lexer ===\n");
    printf("source code:\n%s\n\n", source_code);
    
    Lexer *lexer = lexer_init(source_code);
    if (!lexer) {
        printf("failed to initialize lexer\n");
        return;
    }
    
    printf("tokens:\n");
    Token *token;
    int token_count = 0;
    
    do {
        token = get_next_token(lexer);
        if (!token) {
            printf("failed to get token\n");
            break;
        }
        
        printf("[%d] %s", token_count++, token_type_to_string(token->type));
        if (token->value) {
            printf(": '%s'", token->value);
        }
        printf(" (line %zu, col %zu)\n", token->line, token->column);
        
        if (token->type == TOKEN_ERROR) {
            printf("lexer error: %s\n", token->value ? token->value : "unknown error");
        }
        
        token_free(token);
    } while (token->type != TOKEN_EOF);
    
    lexer_free(lexer);
    printf("\n=== end of lexer test ===\n\n");
}

void test_parser(const char *source_code) {
    printf("=== testing parser ===\n");
    printf("source code:\n%s\n\n", source_code);
    
    Lexer *lexer = lexer_init(source_code);
    if (!lexer) {
        printf("failed to initialize lexer\n");
        return;
    }
    
    Parser *parser = parser_init(lexer);
    if (!parser) {
        printf("failed to initialize parser\n");
        lexer_free(lexer);
        return;
    }
    
    ASTNode *ast = parse_program(parser);
    if (ast) {
        printf("abstract syntax tree:\n");
        print_ast(ast, 0);
        ast_node_free(ast);
    } else {
        printf("failed to parse program\n");
    }
    
    if (parser->had_error) {
        printf("parser encountered errors\n");
    }
    
    parser_free(parser);
    lexer_free(lexer);
    printf("\n=== end of parser test ===\n\n");
}

int main() {
    printf("rust compiler in c - lexer and parser test\n");
    printf("===========================================\n\n");
    
    // test 1: simple rust function
    const char *test1 = "fn main() {\n    let x = 42;\n    println!(\"hello, world!\");\n}";
    test_lexer(test1);
    test_parser(test1);
    
    // test 2: variable declarations
    const char *test2 = "let mut sum = 0;\nlet name: String = \"rust\";";
    test_lexer(test2);
    test_parser(test2);
    
    // test 3: expressions
    const char *test3 = "let result = 1 + 2 * 3;";
    test_lexer(test3);
    test_parser(test3);
    
    // test 4: control flow
    const char *test4 = "if x > 0 {\n    return x;\n} else {\n    return 0;\n}";
    test_lexer(test4);
    test_parser(test4);
    
    // test 5: while loop
    const char *test5 = "while i < 10 {\n    sum += i;\n    i += 1;\n}";
    test_lexer(test5);
    test_parser(test5);
    
    // test 6: for loop
    const char *test6 = "for i in 0..10 {\n    println!(i);\n}";
    test_lexer(test6);
    test_parser(test6);
    
    // test 7: struct definition
    const char *test7 = "struct Point {\n    x: i32,\n    y: i32,\n};";
    test_lexer(test7);
    test_parser(test7);
    
    // test 8: enum definition
    const char *test8 = "enum Option<T> {\n    Some(T),\n    None,\n};";
    test_lexer(test8);
    test_parser(test8);
    
    // test 9: impl block
    const char *test9 = "impl Point {\n    fn new(x: i32, y: i32) -> Self {\n        Point { x, y }\n    }\n}";
    test_lexer(test9);
    test_parser(test9);
    
    return 0;
} 