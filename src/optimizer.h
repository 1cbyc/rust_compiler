#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "semantic.h"
#include "codegen.h"
#include <stdbool.h>
#include <stddef.h>

// Optimization pass types
typedef enum OptimizationPass {
    OPT_CONSTANT_FOLDING,
    OPT_DEAD_CODE_ELIMINATION,
    OPT_LOOP_OPTIMIZATION,
    OPT_REGISTER_ALLOCATION,
    OPT_CODE_SIZE_OPTIMIZATION
} OptimizationPass;

// Optimization context
typedef struct OptimizerContext {
    IRNode *ir;
    bool modified;
    size_t pass_count;
    OptimizationPass *passes;
    size_t pass_capacity;
} OptimizerContext;

// Optimization pass function type
typedef bool (*OptimizationPassFunc)(OptimizerContext *ctx);

// Optimizer initialization
OptimizerContext* optimizer_create(IRNode *ir);
void optimizer_free(OptimizerContext *ctx);

// Optimization pass registration
bool optimizer_add_pass(OptimizerContext *ctx, OptimizationPass pass);
bool optimizer_run_passes(OptimizerContext *ctx);

// Individual optimization passes
bool optimize_constant_folding(OptimizerContext *ctx);
bool optimize_dead_code_elimination(OptimizerContext *ctx);
bool optimize_loop_optimization(OptimizerContext *ctx);
bool optimize_register_allocation(OptimizerContext *ctx);
bool optimize_code_size(OptimizerContext *ctx);

// Helper functions for optimizations
bool is_constant_expression(IRNode *node);
bool is_dead_code(IRNode *node);
bool can_hoist_loop_invariant(IRNode *node);
bool can_eliminate_redundant_load(IRNode *node);

// IR transformation utilities
IRNode* optimize_expression(IRNode *node);
IRNode* eliminate_dead_code(IRNode *node);
IRNode* optimize_loop(IRNode *node);
IRNode* optimize_register_usage(IRNode *node);

// Optimization statistics
typedef struct OptimizationStats {
    size_t constants_folded;
    size_t dead_code_eliminated;
    size_t loops_optimized;
    size_t registers_allocated;
    size_t code_size_reduced;
} OptimizationStats;

void optimizer_print_stats(OptimizerContext *ctx, OptimizationStats *stats);

#endif // OPTIMIZER_H 