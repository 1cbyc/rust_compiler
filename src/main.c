#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"
#include "types.h"
#include "semantic.h"
#include "codegen.h"
#include "stdlib.h"
#include "optimizer.h"

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

void test_type_checker(const char *source_code) {
    printf("=== testing type checker ===\n");
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
    if (!ast) {
        printf("failed to parse program\n");
        parser_free(parser);
        lexer_free(lexer);
        return;
    }
    
    TypeContext *ctx = type_context_create();
    if (!ctx) {
        printf("failed to create type context\n");
        ast_node_free(ast);
        parser_free(parser);
        lexer_free(lexer);
        return;
    }
    
    Type *program_type = type_check_program(ctx, ast);
    if (program_type) {
        printf("program type: %s\n", type_to_string(program_type));
    } else {
        printf("type checking failed\n");
    }
    
    if (ctx->had_error) {
        printf("type checker encountered errors\n");
    }
    
    type_context_free(ctx);
    ast_node_free(ast);
    parser_free(parser);
    lexer_free(lexer);
    printf("\n=== end of type checker test ===\n\n");
}

void test_semantic_analysis(const char *source_code) {
    printf("=== testing semantic analysis ===\n");
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
    if (!ast) {
        printf("failed to parse program\n");
        parser_free(parser);
        lexer_free(lexer);
        return;
    }
    
    SemanticContext *ctx = semantic_context_create();
    if (!ctx) {
        printf("failed to create semantic context\n");
        ast_node_free(ast);
        parser_free(parser);
        lexer_free(lexer);
        return;
    }
    
    IRNode *ir = NULL;
    bool success = semantic_analyze(ctx, ast, &ir);
    
    if (success && ir) {
        printf("semantic analysis successful\n");
        printf("IR generated with %zu children\n", ir->child_count);
    } else {
        printf("semantic analysis failed\n");
    }
    
    if (ctx->had_error) {
        printf("semantic analysis encountered errors\n");
    }
    
    if (ir) irnode_free(ir);
    semantic_context_free(ctx);
    ast_node_free(ast);
    parser_free(parser);
    lexer_free(lexer);
    printf("\n=== end of semantic analysis test ===\n\n");
}

void test_code_generation(const char *source_code) {
    printf("=== testing code generation ===\n");
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
    if (!ast) {
        printf("failed to parse program\n");
        parser_free(parser);
        lexer_free(lexer);
        return;
    }
    
    SemanticContext *sem_ctx = semantic_context_create();
    if (!sem_ctx) {
        printf("failed to create semantic context\n");
        ast_node_free(ast);
        parser_free(parser);
        lexer_free(lexer);
        return;
    }
    
    IRNode *ir = NULL;
    bool sem_success = semantic_analyze(sem_ctx, ast, &ir);
    
    if (!sem_success || !ir) {
        printf("semantic analysis failed, skipping code generation\n");
        semantic_context_free(sem_ctx);
        ast_node_free(ast);
        parser_free(parser);
        lexer_free(lexer);
        return;
    }
    
    // Generate assembly
    CodegenContext *codegen_ctx = codegen_context_create(stdout);
    if (!codegen_ctx) {
        printf("failed to create codegen context\n");
        irnode_free(ir);
        semantic_context_free(sem_ctx);
        ast_node_free(ast);
        parser_free(parser);
        lexer_free(lexer);
        return;
    }
    
    bool codegen_success = codegen_generate(codegen_ctx, ir);
    
    if (codegen_success) {
        printf("\ncode generation successful\n");
    } else {
        printf("\ncode generation failed\n");
    }
    
    codegen_context_free(codegen_ctx);
    irnode_free(ir);
    semantic_context_free(sem_ctx);
    ast_node_free(ast);
    parser_free(parser);
    lexer_free(lexer);
    printf("\n=== end of code generation test ===\n\n");
}

void test_stdlib(const char *source_code) {
    printf("=== testing standard library ===\n");
    printf("source code:\n%s\n\n", source_code);
    
    // Initialize standard library
    StdLibContext *stdlib_ctx = stdlib_create();
    if (!stdlib_ctx) {
        printf("failed to create stdlib context\n");
        return;
    }
    
    // Initialize stdlib functions
    stdlib_init_print_functions(stdlib_ctx);
    stdlib_init_string_functions(stdlib_ctx);
    stdlib_init_collection_functions(stdlib_ctx);
    stdlib_init_error_functions(stdlib_ctx);
    
    printf("registered %zu standard library functions\n", stdlib_ctx->function_count);
    
    // Test macro system
    MacroContext *macro_ctx = macro_context_create();
    if (!macro_ctx) {
        printf("failed to create macro context\n");
        stdlib_free(stdlib_ctx);
        return;
    }
    
    // Register basic macros
    macro_register(macro_ctx, "println!", "println!({})", "println({})");
    macro_register(macro_ctx, "print!", "print!({})", "print({})");
    macro_register(macro_ctx, "vec!", "vec![{}]", "Vec::new()");
    
    printf("registered %zu macros\n", macro_ctx->macro_count);
    
    // Test macro expansion
    char *expanded = macro_expand(macro_ctx, source_code);
    if (expanded) {
        printf("macro expansion result:\n%s\n", expanded);
        free(expanded);
    }
    
    macro_context_free(macro_ctx);
    stdlib_free(stdlib_ctx);
    printf("\n=== end of standard library test ===\n\n");
}

void test_optimization(const char *source_code) {
    printf("=== testing optimization ===\n");
    printf("source code:\n%s\n\n", source_code);
    
    // Create a simple IR for optimization testing
    IRNode *const_1 = irnode_create(IR_CONSTANT, "1");
    IRNode *const_2 = irnode_create(IR_CONSTANT, "2");
    IRNode *add_op = irnode_create(IR_BINARY_OP, "+");
    irnode_add_child(add_op, const_1);
    irnode_add_child(add_op, const_2);
    
    IRNode *temp_assign = irnode_create(IR_ASSIGNMENT, "temp1");
    irnode_add_child(temp_assign, add_op);
    
    IRNode *return_stmt = irnode_create(IR_RETURN, "result");
    IRNode *dead_code = irnode_create(IR_ASSIGNMENT, "temp2");
    
    IRNode *block = irnode_create(IR_NOP, "");
    irnode_add_child(block, temp_assign);
    irnode_add_child(block, return_stmt);
    irnode_add_child(block, dead_code);
    
    // Create optimizer context
    OptimizerContext *opt_ctx = optimizer_create(block);
    if (!opt_ctx) {
        printf("failed to create optimizer context\n");
        irnode_free(block);
        return;
    }
    
    // Add optimization passes
    optimizer_add_pass(opt_ctx, OPT_CONSTANT_FOLDING);
    optimizer_add_pass(opt_ctx, OPT_DEAD_CODE_ELIMINATION);
    optimizer_add_pass(opt_ctx, OPT_CODE_SIZE_OPTIMIZATION);
    
    printf("Running optimization with %zu passes...\n", opt_ctx->pass_count);
    
    // Run optimization passes
    bool success = optimizer_run_passes(opt_ctx);
    
    if (success) {
        printf("optimization completed successfully\n");
        printf("IR modified: %s\n", opt_ctx->modified ? "yes" : "no");
    } else {
        printf("optimization failed\n");
    }
    
    // Print optimization statistics
    OptimizationStats stats = {0};
    optimizer_print_stats(opt_ctx, &stats);
    
    optimizer_free(opt_ctx);
    irnode_free(block);
    printf("\n=== end of optimization test ===\n\n");
}

int main() {
    printf("rust compiler in c - lexer, parser, type checker, semantic analysis, and code generation test\n");
    printf("==========================================================================================\n\n");
    
    // initialize type system
    types_init();
    
    // test 1: simple rust function
    const char *test1 = "fn main() {\n    let x = 42;\n    println!(\"hello, world!\");\n}";
    test_lexer(test1);
    test_parser(test1);
    test_type_checker(test1);
    test_semantic_analysis(test1);
    test_code_generation(test1);
    test_stdlib(test1);
    test_optimization(test1);
    
    // test 2: variable declarations
    const char *test2 = "let mut sum = 0;\nlet name: String = \"rust\";";
    test_lexer(test2);
    test_parser(test2);
    test_type_checker(test2);
    test_semantic_analysis(test2);
    test_code_generation(test2);
    
    // test 3: expressions
    const char *test3 = "let result = 1 + 2 * 3;";
    test_lexer(test3);
    test_parser(test3);
    test_type_checker(test3);
    test_semantic_analysis(test3);
    test_code_generation(test3);
    
    // test 4: control flow
    const char *test4 = "if x > 0 {\n    return x;\n} else {\n    return 0;\n}";
    test_lexer(test4);
    test_parser(test4);
    test_type_checker(test4);
    test_semantic_analysis(test4);
    test_code_generation(test4);
    
    // test 5: while loop
    const char *test5 = "while i < 10 {\n    sum += i;\n    i += 1;\n}";
    test_lexer(test5);
    test_parser(test5);
    test_type_checker(test5);
    test_semantic_analysis(test5);
    test_code_generation(test5);
    
    // test 6: for loop
    const char *test6 = "for i in 0..10 {\n    println!(i);\n}";
    test_lexer(test6);
    test_parser(test6);
    test_type_checker(test6);
    test_semantic_analysis(test6);
    test_code_generation(test6);
    
    // test 7: struct definition
    const char *test7 = "struct Point {\n    x: i32,\n    y: i32,\n};";
    test_lexer(test7);
    test_parser(test7);
    test_type_checker(test7);
    test_semantic_analysis(test7);
    test_code_generation(test7);
    
    // test 8: enum definition
    const char *test8 = "enum Option<T> {\n    Some(T),\n    None,\n};";
    test_lexer(test8);
    test_parser(test8);
    test_type_checker(test8);
    test_semantic_analysis(test8);
    test_code_generation(test8);
    
    // test 9: impl block
    const char *test9 = "impl Point {\n    fn new(x: i32, y: i32) -> Self {\n        Point { x, y }\n    }\n}";
    test_lexer(test9);
    test_parser(test9);
    test_type_checker(test9);
    test_semantic_analysis(test9);
    test_code_generation(test9);
    
    // cleanup
    types_cleanup();
    
    return 0;
} 