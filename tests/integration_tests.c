#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "../src/lexer.h"
#include "../src/parser.h"
#include "../src/types.h"
#include "../src/semantic.h"
#include "../src/codegen.h"
#include "../src/optimizer.h"
#include "../src/stdlib.h"
#include "../src/error.h"

#define TEST_ASSERT(condition, message) \
    if (!(condition)) { \
        printf("FAIL: %s\n", message); \
        return false; \
    } else { \
        printf("PASS: %s\n", message); \
    }

#define TEST_SECTION(name) printf("\n=== %s ===\n", name)

// Test cases
const char *test_cases[] = {
    // Basic functionality
    "fn main() { let x = 42; println!(\"Hello, world!\"); }",
    
    // Variable declarations
    "let mut sum = 0;\nlet name: String = \"rust\";",
    
    // Expressions
    "let result = 1 + 2 * 3;",
    
    // Control flow
    "if x > 0 {\n    return x;\n} else {\n    return 0;\n}",
    
    // Loops
    "while i < 10 {\n    sum += i;\n    i += 1;\n}",
    
    // Functions
    "fn add(x: i32, y: i32) -> i32 {\n    return x + y;\n}",
    
    // Structs
    "struct Point {\n    x: i32,\n    y: i32,\n};",
    
    // Enums
    "enum Option<T> {\n    Some(T),\n    None,\n};",
    
    // Impl blocks
    "impl Point {\n    fn new(x: i32, y: i32) -> Self {\n        Point { x, y }\n    }\n}",
    
    // Error cases
    "let x = ;",  // Syntax error
    "let x: UndefinedType = 42;",  // Type error
    "fn main() { undefined_variable; }",  // Semantic error
    
    NULL
};

bool test_compilation_pipeline(const char *source, const char *test_name) {
    printf("\n--- Testing: %s ---\n", test_name);
    
    // Create error context
    ErrorContext *error_ctx = error_context_create(100);
    
    // Lexer
    Lexer *lexer = lexer_init(source);
    if (!lexer) {
        printf("FAIL: Lexer initialization failed\n");
        error_context_free(error_ctx);
        return false;
    }
    
    // Parser
    Parser *parser = parser_init(lexer);
    if (!parser) {
        printf("FAIL: Parser initialization failed\n");
        lexer_free(lexer);
        error_context_free(error_ctx);
        return false;
    }
    
    ASTNode *ast = parse_program(parser);
    if (!ast) {
        printf("FAIL: AST creation failed\n");
        parser_free(parser);
        lexer_free(lexer);
        error_context_free(error_ctx);
        return false;
    }
    
    // Type checker
    types_init();
    TypeContext *type_ctx = type_context_create();
    if (!type_ctx) {
        printf("FAIL: Type context creation failed\n");
        ast_node_free(ast);
        parser_free(parser);
        lexer_free(lexer);
        error_context_free(error_ctx);
        return false;
    }
    
    Type *program_type = type_check_program(type_ctx, ast);
    if (!program_type && !error_ctx->had_error) {
        printf("FAIL: Type checking failed\n");
        type_context_free(type_ctx);
        ast_node_free(ast);
        parser_free(parser);
        lexer_free(lexer);
        error_context_free(error_ctx);
        return false;
    }
    
    // Semantic analysis
    SemanticContext *sem_ctx = semantic_context_create(error_ctx);
    if (!sem_ctx) {
        printf("FAIL: Semantic context creation failed\n");
        type_context_free(type_ctx);
        ast_node_free(ast);
        parser_free(parser);
        lexer_free(lexer);
        error_context_free(error_ctx);
        return false;
    }
    
    IRNode *ir = NULL;
    bool sem_success = semantic_analyze(sem_ctx, ast, &ir);
    if (!sem_success && !error_ctx->had_error) {
        printf("FAIL: Semantic analysis failed\n");
        semantic_context_free(sem_ctx);
        type_context_free(type_ctx);
        ast_node_free(ast);
        parser_free(parser);
        lexer_free(lexer);
        error_context_free(error_ctx);
        return false;
    }
    
    // Optimization (if IR was generated)
    if (ir) {
        OptimizerContext *opt_ctx = optimizer_create(ir);
        if (opt_ctx) {
            optimizer_add_pass(opt_ctx, OPT_CONSTANT_FOLDING);
            optimizer_add_pass(opt_ctx, OPT_DEAD_CODE_ELIMINATION);
            optimizer_run_passes(opt_ctx);
            optimizer_free(opt_ctx);
        }
    }
    
    // Code generation (if IR was generated)
    if (ir) {
        CodegenContext *codegen_ctx = codegen_context_create(stdout);
        if (codegen_ctx) {
            bool codegen_success = codegen_generate(codegen_ctx, ir);
            if (!codegen_success) {
                printf("WARNING: Code generation failed\n");
            }
            codegen_context_free(codegen_ctx);
        }
    }
    
    // Print error summary
    if (error_ctx->error_count > 0) {
        printf("Errors reported: %zu\n", error_ctx->error_count);
        error_print_summary(error_ctx, stdout);
    }
    
    // Cleanup
    if (ir) irnode_free(ir);
    semantic_context_free(sem_ctx);
    type_context_free(type_ctx);
    types_cleanup();
    ast_node_free(ast);
    parser_free(parser);
    lexer_free(lexer);
    error_context_free(error_ctx);
    
    printf("PASS: Pipeline completed\n");
    return true;
}

bool test_error_recovery() {
    TEST_SECTION("Error Recovery Tests");
    
    const char *error_source = "let x = ; let y = 42;";  // Syntax error in first statement
    
    ErrorContext *error_ctx = error_context_create(100);
    Lexer *lexer = lexer_init(error_source);
    Parser *parser = parser_init(lexer);
    
    ASTNode *ast = parse_program(parser);
    
    // Should be able to recover and continue parsing
    TEST_ASSERT(ast != NULL, "AST creation despite errors");
    TEST_ASSERT(error_ctx->error_count > 0, "Errors reported");
    TEST_ASSERT(error_can_recover(error_ctx), "Error recovery possible");
    
    ast_node_free(ast);
    parser_free(parser);
    lexer_free(lexer);
    error_context_free(error_ctx);
    
    return true;
}

bool test_optimization_integration() {
    TEST_SECTION("Optimization Integration Tests");
    
    const char *source = "let x = 2 + 3; let y = x * 4;";
    
    ErrorContext *error_ctx = error_context_create(100);
    Lexer *lexer = lexer_init(source);
    Parser *parser = parser_init(lexer);
    ASTNode *ast = parse_program(parser);
    
    types_init();
    TypeContext *type_ctx = type_context_create();
    Type *program_type = type_check_program(type_ctx, ast);
    
    SemanticContext *sem_ctx = semantic_context_create(error_ctx);
    IRNode *ir = NULL;
    bool sem_success = semantic_analyze(sem_ctx, ast, &ir);
    
    if (ir) {
        OptimizerContext *opt_ctx = optimizer_create(ir);
        optimizer_add_pass(opt_ctx, OPT_CONSTANT_FOLDING);
        optimizer_add_pass(opt_ctx, OPT_DEAD_CODE_ELIMINATION);
        bool opt_success = optimizer_run_passes(opt_ctx);
        
        TEST_ASSERT(opt_success, "Optimization integration");
        
        optimizer_free(opt_ctx);
        irnode_free(ir);
    }
    
    semantic_context_free(sem_ctx);
    type_context_free(type_ctx);
    types_cleanup();
    ast_node_free(ast);
    parser_free(parser);
    lexer_free(lexer);
    error_context_free(error_ctx);
    
    return true;
}

bool test_stdlib_integration() {
    TEST_SECTION("Standard Library Integration Tests");
    
    const char *source = "fn main() { println!(\"Hello\"); let len = len(\"test\"); }";
    
    ErrorContext *error_ctx = error_context_create(100);
    Lexer *lexer = lexer_init(source);
    Parser *parser = parser_init(lexer);
    ASTNode *ast = parse_program(parser);
    
    // Initialize stdlib
    StdLibContext *stdlib_ctx = stdlib_create();
    stdlib_init_print_functions(stdlib_ctx);
    stdlib_init_string_functions(stdlib_ctx);
    
    TEST_ASSERT(stdlib_ctx->function_count > 0, "Stdlib functions registered");
    
    // Test macro expansion
    MacroContext *macro_ctx = macro_context_create();
    macro_register(macro_ctx, "println!", "println!({})", "println({})");
    
    char *expanded = macro_expand(macro_ctx, "println!(\"Hello\")");
    TEST_ASSERT(expanded != NULL, "Macro expansion");
    
    free(expanded);
    macro_context_free(macro_ctx);
    stdlib_free(stdlib_ctx);
    
    ast_node_free(ast);
    parser_free(parser);
    lexer_free(lexer);
    error_context_free(error_ctx);
    
    return true;
}

bool test_performance_benchmarks() {
    TEST_SECTION("Performance Benchmarks");
    
    const char *complex_source = 
        "fn fibonacci(n: i32) -> i32 {\n"
        "    if n <= 1 {\n"
        "        return n;\n"
        "    }\n"
        "    return fibonacci(n - 1) + fibonacci(n - 2);\n"
        "}\n"
        "fn main() {\n"
        "    let result = fibonacci(10);\n"
        "    println!(\"Result: {}\", result);\n"
        "}\n";
    
    clock_t start = clock();
    
    ErrorContext *error_ctx = error_context_create(100);
    Lexer *lexer = lexer_init(complex_source);
    Parser *parser = parser_init(lexer);
    ASTNode *ast = parse_program(parser);
    
    types_init();
    TypeContext *type_ctx = type_context_create();
    Type *program_type = type_check_program(type_ctx, ast);
    
    SemanticContext *sem_ctx = semantic_context_create(error_ctx);
    IRNode *ir = NULL;
    bool sem_success = semantic_analyze(sem_ctx, ast, &ir);
    
    if (ir) {
        OptimizerContext *opt_ctx = optimizer_create(ir);
        optimizer_add_pass(opt_ctx, OPT_CONSTANT_FOLDING);
        optimizer_add_pass(opt_ctx, OPT_DEAD_CODE_ELIMINATION);
        optimizer_run_passes(opt_ctx);
        
        CodegenContext *codegen_ctx = codegen_context_create(stdout);
        codegen_generate(codegen_ctx, ir);
        
        codegen_context_free(codegen_ctx);
        optimizer_free(opt_ctx);
        irnode_free(ir);
    }
    
    clock_t end = clock();
    double total_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("Performance: Full pipeline completed in %.6f seconds\n", total_time);
    
    semantic_context_free(sem_ctx);
    type_context_free(type_ctx);
    types_cleanup();
    ast_node_free(ast);
    parser_free(parser);
    lexer_free(lexer);
    error_context_free(error_ctx);
    
    return true;
}

int main() {
    printf("Running Rust Compiler Integration Tests\n");
    printf("======================================\n");
    
    bool all_passed = true;
    
    // Test compilation pipeline with various inputs
    const char *test_names[] = {
        "Basic Function",
        "Variable Declarations", 
        "Expressions",
        "Control Flow",
        "Loops",
        "Functions",
        "Structs",
        "Enums",
        "Impl Blocks",
        "Syntax Error",
        "Type Error",
        "Semantic Error"
    };
    
    for (int i = 0; test_cases[i] != NULL; i++) {
        all_passed &= test_compilation_pipeline(test_cases[i], test_names[i]);
    }
    
    // Run specialized integration tests
    all_passed &= test_error_recovery();
    all_passed &= test_optimization_integration();
    all_passed &= test_stdlib_integration();
    all_passed &= test_performance_benchmarks();
    
    printf("\n======================================\n");
    if (all_passed) {
        printf("ALL INTEGRATION TESTS PASSED!\n");
        return 0;
    } else {
        printf("SOME INTEGRATION TESTS FAILED!\n");
        return 1;
    }
} 