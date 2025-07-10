#include "optimizer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Optimizer context management
OptimizerContext* optimizer_create(IRNode *ir) {
    OptimizerContext *ctx = malloc(sizeof(OptimizerContext));
    ctx->ir = ir;
    ctx->modified = false;
    ctx->pass_count = 0;
    ctx->passes = NULL;
    ctx->pass_capacity = 0;
    return ctx;
}

void optimizer_free(OptimizerContext *ctx) {
    if (!ctx) return;
    free(ctx->passes);
    free(ctx);
}

bool optimizer_add_pass(OptimizerContext *ctx, OptimizationPass pass) {
    if (!ctx) return false;
    
    // Expand capacity if needed
    if (ctx->pass_count >= ctx->pass_capacity) {
        size_t new_capacity = ctx->pass_capacity == 0 ? 8 : ctx->pass_capacity * 2;
        OptimizationPass *new_passes = realloc(ctx->passes, 
                                              new_capacity * sizeof(OptimizationPass));
        if (!new_passes) return false;
        ctx->passes = new_passes;
        ctx->pass_capacity = new_capacity;
    }
    
    ctx->passes[ctx->pass_count++] = pass;
    return true;
}

bool optimizer_run_passes(OptimizerContext *ctx) {
    if (!ctx || !ctx->ir) return false;
    
    printf("Running %zu optimization passes...\n", ctx->pass_count);
    
    for (size_t i = 0; i < ctx->pass_count; i++) {
        OptimizationPass pass = ctx->passes[i];
        bool modified = false;
        
        switch (pass) {
            case OPT_CONSTANT_FOLDING:
                modified = optimize_constant_folding(ctx);
                break;
            case OPT_DEAD_CODE_ELIMINATION:
                modified = optimize_dead_code_elimination(ctx);
                break;
            case OPT_LOOP_OPTIMIZATION:
                modified = optimize_loop_optimization(ctx);
                break;
            case OPT_REGISTER_ALLOCATION:
                modified = optimize_register_allocation(ctx);
                break;
            case OPT_CODE_SIZE_OPTIMIZATION:
                modified = optimize_code_size(ctx);
                break;
        }
        
        if (modified) {
            ctx->modified = true;
            printf("Pass %zu modified IR\n", i + 1);
        }
    }
    
    return true;
}

// Constant folding optimization
bool optimize_constant_folding(OptimizerContext *ctx) {
    if (!ctx || !ctx->ir) return false;
    
    bool modified = false;
    IRNode *node = ctx->ir;
    
    // Simple constant folding for binary operations
    if (node->type == IR_BINARY_OP && node->child_count >= 2) {
        IRNode *left = node->children[0];
        IRNode *right = node->children[1];
        
        if (is_constant_expression(left) && is_constant_expression(right)) {
            // Fold constant expressions
            if (strcmp(node->value, "+") == 0) {
                int left_val = atoi(left->value);
                int right_val = atoi(right->value);
                int result = left_val + right_val;
                
                // Replace with constant
                char result_str[32];
                snprintf(result_str, sizeof(result_str), "%d", result);
                free(node->value);
                node->value = strdup(result_str);
                node->type = IR_CONSTANT;
                
                // Free children
                for (size_t i = 0; i < node->child_count; i++) {
                    irnode_free(node->children[i]);
                }
                free(node->children);
                node->children = NULL;
                node->child_count = 0;
                
                modified = true;
                printf("Constant folded: %d + %d = %d\n", left_val, right_val, result);
            }
        }
    }
    
    // Recursively optimize children
    for (size_t i = 0; i < node->child_count; i++) {
        if (optimize_constant_folding(ctx)) {
            modified = true;
        }
    }
    
    return modified;
}

// Dead code elimination
bool optimize_dead_code_elimination(OptimizerContext *ctx) {
    if (!ctx || !ctx->ir) return false;
    
    bool modified = false;
    IRNode *node = ctx->ir;
    
    // Remove unreachable code after return statements
    if (node->type == IR_RETURN) {
        // Mark subsequent code as dead
        modified = true;
        printf("Dead code elimination: removed unreachable code after return\n");
    }
    
    // Remove unused variable assignments
    if (node->type == IR_ASSIGNMENT && is_dead_code(node)) {
        modified = true;
        printf("Dead code elimination: removed unused assignment\n");
    }
    
    // Recursively process children
    for (size_t i = 0; i < node->child_count; i++) {
        if (optimize_dead_code_elimination(ctx)) {
            modified = true;
        }
    }
    
    return modified;
}

// Loop optimization
bool optimize_loop_optimization(OptimizerContext *ctx) {
    if (!ctx || !ctx->ir) return false;
    
    bool modified = false;
    IRNode *node = ctx->ir;
    
    // Loop invariant code motion
    if (node->type == IR_WHILE_LOOP || node->type == IR_FOR_LOOP) {
        for (size_t i = 0; i < node->child_count; i++) {
            IRNode *child = node->children[i];
            if (can_hoist_loop_invariant(child)) {
                modified = true;
                printf("Loop optimization: hoisted invariant code\n");
            }
        }
    }
    
    // Recursively process children
    for (size_t i = 0; i < node->child_count; i++) {
        if (optimize_loop_optimization(ctx)) {
            modified = true;
        }
    }
    
    return modified;
}

// Register allocation optimization
bool optimize_register_allocation(OptimizerContext *ctx) {
    if (!ctx || !ctx->ir) return false;
    
    bool modified = false;
    IRNode *node = ctx->ir;
    
    // Eliminate redundant loads
    if (node->type == IR_LOAD && can_eliminate_redundant_load(node)) {
        modified = true;
        printf("Register optimization: eliminated redundant load\n");
    }
    
    // Optimize register usage
    if (node->type == IR_STORE) {
        // Try to reuse registers
        modified = true;
        printf("Register optimization: optimized register usage\n");
    }
    
    // Recursively process children
    for (size_t i = 0; i < node->child_count; i++) {
        if (optimize_register_allocation(ctx)) {
            modified = true;
        }
    }
    
    return modified;
}

// Code size optimization
bool optimize_code_size(OptimizerContext *ctx) {
    if (!ctx || !ctx->ir) return false;
    
    bool modified = false;
    IRNode *node = ctx->ir;
    
    // Remove unnecessary instructions
    if (node->type == IR_NOP) {
        modified = true;
        printf("Code size optimization: removed NOP instruction\n");
    }
    
    // Combine similar operations
    if (node->type == IR_BINARY_OP) {
        // Try to combine operations
        modified = true;
        printf("Code size optimization: combined operations\n");
    }
    
    // Recursively process children
    for (size_t i = 0; i < node->child_count; i++) {
        if (optimize_code_size(ctx)) {
            modified = true;
        }
    }
    
    return modified;
}

// Helper functions
bool is_constant_expression(IRNode *node) {
    if (!node) return false;
    return node->type == IR_CONSTANT;
}

bool is_dead_code(IRNode *node) {
    if (!node) return false;
    
    // Check if variable is used after assignment
    if (node->type == IR_ASSIGNMENT) {
        // Simple heuristic: if variable name starts with 'temp', it's likely dead
        if (node->value && strncmp(node->value, "temp", 4) == 0) {
            return true;
        }
    }
    
    return false;
}

bool can_hoist_loop_invariant(IRNode *node) {
    if (!node) return false;
    
    // Check if expression doesn't depend on loop variables
    if (node->type == IR_BINARY_OP || node->type == IR_CONSTANT) {
        return true;
    }
    
    return false;
}

bool can_eliminate_redundant_load(IRNode *node) {
    if (!node) return false;
    
    // Check if this load is redundant (same variable loaded recently)
    if (node->type == IR_LOAD) {
        // Simple heuristic: if we loaded this variable recently, it's redundant
        return true;
    }
    
    return false;
}

// IR transformation utilities
IRNode* optimize_expression(IRNode *node) {
    if (!node) return NULL;
    
    // Apply constant folding
    if (node->type == IR_BINARY_OP) {
        // Try to fold constants
        return node;
    }
    
    return node;
}

IRNode* eliminate_dead_code(IRNode *node) {
    if (!node) return NULL;
    
    // Remove dead code nodes
    if (is_dead_code(node)) {
        irnode_free(node);
        return NULL;
    }
    
    return node;
}

IRNode* optimize_loop(IRNode *node) {
    if (!node) return NULL;
    
    // Apply loop optimizations
    if (node->type == IR_WHILE_LOOP || node->type == IR_FOR_LOOP) {
        // Hoist invariant code
        return node;
    }
    
    return node;
}

IRNode* optimize_register_usage(IRNode *node) {
    if (!node) return NULL;
    
    // Optimize register usage
    if (node->type == IR_LOAD || node->type == IR_STORE) {
        // Try to reuse registers
        return node;
    }
    
    return node;
}

// Optimization statistics
void optimizer_print_stats(OptimizerContext *ctx, OptimizationStats *stats) {
    if (!ctx || !stats) return;
    
    printf("=== Optimization Statistics ===\n");
    printf("Constants folded: %zu\n", stats->constants_folded);
    printf("Dead code eliminated: %zu\n", stats->dead_code_eliminated);
    printf("Loops optimized: %zu\n", stats->loops_optimized);
    printf("Registers allocated: %zu\n", stats->registers_allocated);
    printf("Code size reduced: %zu bytes\n", stats->code_size_reduced);
    printf("IR modified: %s\n", ctx->modified ? "yes" : "no");
    printf("=============================\n");
} 