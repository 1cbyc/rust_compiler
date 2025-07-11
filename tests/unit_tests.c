#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../src/lexer.h"
#include "../src/parser.h"
#include "../src/types.h"
#include "../src/semantic.h"
#include "../src/codegen.h"
#include "../src/optimizer.h"
#include "../src/stdlib.h"
#include "../src/error.h"

// Test utilities
#define TEST_ASSERT(condition, message) \
    if (!(condition)) { \
        printf("FAIL: %s\n", message); \
        return false; \
    } else { \
        printf("PASS: %s\n", message); \
    }

#define TEST_SECTION(name) printf("\n=== %s ===\n", name)

// Lexer tests
bool test_lexer_basic() {
    TEST_SECTION("Lexer Basic Tests");
    
    const char *source = "let x = 42;";
    Lexer *lexer = lexer_init(source);
    TEST_ASSERT(lexer != NULL, "Lexer initialization");
    
    Token *token = get_next_token(lexer);
    TEST_ASSERT(token != NULL, "Token retrieval");
    TEST_ASSERT(token->type == TOKEN_LET, "LET token");
    
    token = get_next_token(lexer);
    TEST_ASSERT(token != NULL, "Identifier token");
    TEST_ASSERT(token->type == TOKEN_IDENTIFIER, "IDENTIFIER token");
    TEST_ASSERT(strcmp(token->value, "x") == 0, "Identifier value");
    
    token = get_next_token(lexer);
    TEST_ASSERT(token->type == TOKEN_ASSIGN, "ASSIGN token");
    
    token = get_next_token(lexer);
    TEST_ASSERT(token->type == TOKEN_INTEGER_LITERAL, "INTEGER_LITERAL token");
    TEST_ASSERT(strcmp(token->value, "42") == 0, "Integer value");
    
    token = get_next_token(lexer);
    TEST_ASSERT(token->type == TOKEN_SEMICOLON, "SEMICOLON token");
    
    token = get_next_token(lexer);
    TEST_ASSERT(token->type == TOKEN_EOF, "EOF token");
    
    lexer_free(lexer);
    return true;
}

bool test_lexer_operators() {
    TEST_SECTION("Lexer Operator Tests");
    
    const char *source = "+ - * / % == != < > <= >=";
    Lexer *lexer = lexer_init(source);
    
    Token *token = get_next_token(lexer);
    TEST_ASSERT(token->type == TOKEN_PLUS, "PLUS operator");
    
    token = get_next_token(lexer);
    TEST_ASSERT(token->type == TOKEN_MINUS, "MINUS operator");
    
    token = get_next_token(lexer);
    TEST_ASSERT(token->type == TOKEN_STAR, "STAR operator");
    
    token = get_next_token(lexer);
    TEST_ASSERT(token->type == TOKEN_SLASH, "SLASH operator");
    
    token = get_next_token(lexer);
    TEST_ASSERT(token->type == TOKEN_PERCENT, "PERCENT operator");
    
    token = get_next_token(lexer);
    TEST_ASSERT(token->type == TOKEN_EQ, "EQ operator");
    
    token = get_next_token(lexer);
    TEST_ASSERT(token->type == TOKEN_NE, "NE operator");
    
    token = get_next_token(lexer);
    TEST_ASSERT(token->type == TOKEN_LT, "LT operator");
    
    token = get_next_token(lexer);
    TEST_ASSERT(token->type == TOKEN_GT, "GT operator");
    
    token = get_next_token(lexer);
    TEST_ASSERT(token->type == TOKEN_LE, "LE operator");
    
    token = get_next_token(lexer);
    TEST_ASSERT(token->type == TOKEN_GE, "GE operator");
    
    lexer_free(lexer);
    return true;
}

// Parser tests
bool test_parser_variable_declaration() {
    TEST_SECTION("Parser Variable Declaration Tests");
    
    const char *source = "let x = 42;";
    Lexer *lexer = lexer_init(source);
    Parser *parser = parser_init(lexer);
    
    ASTNode *ast = parse_program(parser);
    TEST_ASSERT(ast != NULL, "AST creation");
    TEST_ASSERT(ast->type == NODE_PROGRAM, "Program node");
    TEST_ASSERT(ast->data.program.statement_count == 1, "One statement");
    
    ASTNode *stmt = ast->data.program.statements[0];
    TEST_ASSERT(stmt->type == NODE_VARIABLE_DECL, "Variable declaration");
    TEST_ASSERT(strcmp(stmt->data.variable_decl.name, "x") == 0, "Variable name");
    
    ast_node_free(ast);
    parser_free(parser);
    lexer_free(lexer);
    return true;
}

bool test_parser_function_declaration() {
    TEST_SECTION("Parser Function Declaration Tests");
    
    const char *source = "fn add(x: i32, y: i32) -> i32 { return x + y; }";
    Lexer *lexer = lexer_init(source);
    Parser *parser = parser_init(lexer);
    
    ASTNode *ast = parse_program(parser);
    TEST_ASSERT(ast != NULL, "AST creation");
    
    ASTNode *stmt = ast->data.program.statements[0];
    TEST_ASSERT(stmt->type == NODE_FUNCTION_DECL, "Function declaration");
    TEST_ASSERT(strcmp(stmt->data.function.name, "add") == 0, "Function name");
    TEST_ASSERT(stmt->data.function.param_count == 2, "Two parameters");
    
    ast_node_free(ast);
    parser_free(parser);
    lexer_free(lexer);
    return true;
}

// Type checker tests
bool test_type_checker_basic() {
    TEST_SECTION("Type Checker Basic Tests");
    
    types_init();
    
    Type *int_type = type_create_primitive(TYPE_INT, "i32", 4);
    TEST_ASSERT(int_type != NULL, "Integer type creation");
    TEST_ASSERT(int_type->kind == TYPE_INT, "Type kind");
    TEST_ASSERT(strcmp(int_type->name, "i32") == 0, "Type name");
    
    Type *string_type = type_create_primitive(TYPE_STRING, "String", 24);
    TEST_ASSERT(string_type != NULL, "String type creation");
    TEST_ASSERT(string_type->kind == TYPE_STRING, "Type kind");
    
    TypeContext *ctx = type_context_create();
    TEST_ASSERT(ctx != NULL, "Type context creation");
    
    type_context_free(ctx);
    types_cleanup();
    return true;
}

// Semantic analysis tests
bool test_semantic_analysis() {
    TEST_SECTION("Semantic Analysis Tests");
    
    const char *source = "let x = 42; let y = x + 10;";
    Lexer *lexer = lexer_init(source);
    Parser *parser = parser_init(lexer);
    ASTNode *ast = parse_program(parser);
    
    ErrorContext *error_ctx = error_context_create(100);
    SemanticContext *sem_ctx = semantic_context_create(error_ctx);
    TEST_ASSERT(sem_ctx != NULL, "Semantic context creation");
    
    IRNode *ir = NULL;
    bool success = semantic_analyze(sem_ctx, ast, &ir);
    TEST_ASSERT(success, "Semantic analysis success");
    TEST_ASSERT(ir != NULL, "IR generation");
    
    if (ir) irnode_free(ir);
    semantic_context_free(sem_ctx);
    error_context_free(error_ctx);
    ast_node_free(ast);
    parser_free(parser);
    lexer_free(lexer);
    return true;
}

// Code generation tests
bool test_code_generation() {
    TEST_SECTION("Code Generation Tests");
    
    // Create a simple IR for testing
    IRNode *const_node = irnode_create(IR_CONST, "42", NULL);
    IRNode *var_node = irnode_create(IR_VAR, "x", NULL);
    IRNode *binop_node = irnode_create(IR_BINOP, "+", NULL);
    irnode_add_child(binop_node, const_node);
    irnode_add_child(binop_node, var_node);
    
    CodegenContext *codegen_ctx = codegen_context_create(stdout);
    TEST_ASSERT(codegen_ctx != NULL, "Codegen context creation");
    
    bool success = codegen_generate(codegen_ctx, binop_node);
    TEST_ASSERT(success, "Code generation success");
    
    codegen_context_free(codegen_ctx);
    irnode_free(binop_node);
    return true;
}

// Optimizer tests
bool test_optimizer() {
    TEST_SECTION("Optimizer Tests");
    
    // Create IR for constant folding test
    IRNode *const1 = irnode_create(IR_CONST, "2", NULL);
    IRNode *const2 = irnode_create(IR_CONST, "3", NULL);
    IRNode *binop = irnode_create(IR_BINOP, "+", NULL);
    irnode_add_child(binop, const1);
    irnode_add_child(binop, const2);
    
    OptimizerContext *opt_ctx = optimizer_create(binop);
    TEST_ASSERT(opt_ctx != NULL, "Optimizer context creation");
    
    optimizer_add_pass(opt_ctx, OPT_CONSTANT_FOLDING);
    bool success = optimizer_run_passes(opt_ctx);
    TEST_ASSERT(success, "Optimization success");
    
    optimizer_free(opt_ctx);
    irnode_free(binop);
    return true;
}

// Standard library tests
bool test_stdlib() {
    TEST_SECTION("Standard Library Tests");
    
    StdLibContext *stdlib_ctx = stdlib_create();
    TEST_ASSERT(stdlib_ctx != NULL, "Stdlib context creation");
    
    stdlib_init_print_functions(stdlib_ctx);
    stdlib_init_string_functions(stdlib_ctx);
    stdlib_init_collection_functions(stdlib_ctx);
    stdlib_init_error_functions(stdlib_ctx);
    
    TEST_ASSERT(stdlib_ctx->function_count > 0, "Functions registered");
    
    MacroContext *macro_ctx = macro_context_create();
    TEST_ASSERT(macro_ctx != NULL, "Macro context creation");
    
    bool macro_success = macro_register(macro_ctx, "println!", "println!({})", "println({})");
    TEST_ASSERT(macro_success, "Macro registration");
    
    char *expanded = macro_expand(macro_ctx, "println!(\"hello\")");
    TEST_ASSERT(expanded != NULL, "Macro expansion");
    
    free(expanded);
    macro_context_free(macro_ctx);
    stdlib_free(stdlib_ctx);
    return true;
}

// Error handling tests
bool test_error_handling() {
    TEST_SECTION("Error Handling Tests");
    
    ErrorContext *error_ctx = error_context_create(100);
    TEST_ASSERT(error_ctx != NULL, "Error context creation");
    
    // Test error reporting
    bool success = error_report_lexical(error_ctx, "Test error", 1, 1, "test.rs");
    TEST_ASSERT(success, "Error reporting");
    TEST_ASSERT(error_ctx->error_count == 1, "Error count");
    
    // Test debug info
    SourceLocation loc = source_location_create(10, 5, "main.rs");
    DebugInfo *debug_info = debug_info_create(loc, "test_function");
    TEST_ASSERT(debug_info != NULL, "Debug info creation");
    
    // Test stack trace
    StackTrace *trace = stack_trace_create();
    TEST_ASSERT(trace != NULL, "Stack trace creation");
    
    bool trace_success = stack_trace_add_frame(trace, "main", loc);
    TEST_ASSERT(trace_success, "Stack frame addition");
    TEST_ASSERT(trace->frame_count == 1, "Frame count");
    
    stack_trace_free(trace);
    debug_info_free(debug_info);
    error_context_free(error_ctx);
    return true;
}

// Integration tests
bool test_integration_full_pipeline() {
    TEST_SECTION("Integration Tests - Full Pipeline");
    
    const char *source = "fn main() { let x = 42; println!(\"Hello, world!\"); }";
    
    // Lexer
    Lexer *lexer = lexer_init(source);
    TEST_ASSERT(lexer != NULL, "Lexer creation");
    
    // Parser
    Parser *parser = parser_init(lexer);
    TEST_ASSERT(parser != NULL, "Parser creation");
    
    ASTNode *ast = parse_program(parser);
    TEST_ASSERT(ast != NULL, "AST creation");
    
    // Type checker
    types_init();
    TypeContext *type_ctx = type_context_create();
    TEST_ASSERT(type_ctx != NULL, "Type context creation");
    
    Type *program_type = type_check_program(type_ctx, ast);
    TEST_ASSERT(program_type != NULL, "Type checking");
    
    // Semantic analysis
    ErrorContext *error_ctx = error_context_create(100);
    SemanticContext *sem_ctx = semantic_context_create(error_ctx);
    TEST_ASSERT(sem_ctx != NULL, "Semantic context creation");
    
    IRNode *ir = NULL;
    bool sem_success = semantic_analyze(sem_ctx, ast, &ir);
    TEST_ASSERT(sem_success, "Semantic analysis");
    TEST_ASSERT(ir != NULL, "IR generation");
    
    // Code generation
    CodegenContext *codegen_ctx = codegen_context_create(stdout);
    TEST_ASSERT(codegen_ctx != NULL, "Codegen context creation");
    
    bool codegen_success = codegen_generate(codegen_ctx, ir);
    TEST_ASSERT(codegen_success, "Code generation");
    
    // Cleanup
    codegen_context_free(codegen_ctx);
    if (ir) irnode_free(ir);
    semantic_context_free(sem_ctx);
    error_context_free(error_ctx);
    type_context_free(type_ctx);
    types_cleanup();
    ast_node_free(ast);
    parser_free(parser);
    lexer_free(lexer);
    
    return true;
}

// Performance benchmarks
bool test_performance_benchmarks() {
    TEST_SECTION("Performance Benchmarks");
    
    const char *source = "fn fibonacci(n: i32) -> i32 { if n <= 1 { return n; } return fibonacci(n - 1) + fibonacci(n - 2); }";
    
    // Benchmark lexer
    clock_t start = clock();
    Lexer *lexer = lexer_init(source);
    Token *token;
    int token_count = 0;
    while ((token = get_next_token(lexer)) && token->type != TOKEN_EOF) {
        token_count++;
    }
    clock_t end = clock();
    double lexer_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Lexer performance: %d tokens in %.6f seconds\n", token_count, lexer_time);
    
    lexer_free(lexer);
    return true;
}

// Main test runner
int main() {
    printf("Running Rust Compiler Unit Tests\n");
    printf("================================\n");
    
    bool all_passed = true;
    
    // Run all tests
    all_passed &= test_lexer_basic();
    all_passed &= test_lexer_operators();
    all_passed &= test_parser_variable_declaration();
    all_passed &= test_parser_function_declaration();
    all_passed &= test_type_checker_basic();
    all_passed &= test_semantic_analysis();
    all_passed &= test_code_generation();
    all_passed &= test_optimizer();
    all_passed &= test_stdlib();
    all_passed &= test_error_handling();
    all_passed &= test_integration_full_pipeline();
    all_passed &= test_performance_benchmarks();
    
    printf("\n================================\n");
    if (all_passed) {
        printf("ALL TESTS PASSED!\n");
        return 0;
    } else {
        printf("SOME TESTS FAILED!\n");
        return 1;
    }
} 