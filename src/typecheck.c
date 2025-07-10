#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// type checking functions
Type* type_check_expression(TypeContext *ctx, ASTNode *expr) {
    if (!ctx || !expr) return NULL;
    
    switch (expr->type) {
        case NODE_EXPRESSION:
            return type_infer_expression(ctx, expr);
            
        default:
            type_context_error(ctx, "unexpected expression type", expr->line, expr->column);
            return NULL;
    }
}

Type* type_check_statement(TypeContext *ctx, ASTNode *stmt) {
    if (!ctx || !stmt) return NULL;
    
    switch (stmt->type) {
        case NODE_VARIABLE_DECL:
            return type_check_variable_declaration(ctx, stmt);
            
        case NODE_FUNCTION:
            return type_check_function_declaration(ctx, stmt);
            
        case NODE_IF_STATEMENT:
            return type_check_if_statement(ctx, stmt);
            
        case NODE_WHILE_STATEMENT:
            return type_check_while_statement(ctx, stmt);
            
        case NODE_FOR_STATEMENT:
            return type_check_for_statement(ctx, stmt);
            
        case NODE_RETURN_STATEMENT:
            return type_check_return_statement(ctx, stmt);
            
        case NODE_EXPRESSION:
            return type_check_expression(ctx, stmt);
            
        default:
            type_context_error(ctx, "unexpected statement type", stmt->line, stmt->column);
            return NULL;
    }
}

Type* type_check_program(TypeContext *ctx, ASTNode *program) {
    if (!ctx || !program || program->type != NODE_PROGRAM) {
        return NULL;
    }
    
    Type *result_type = TYPE_UNIT;
    
    for (size_t i = 0; i < program->data.program.statement_count; i++) {
        ASTNode *stmt = program->data.program.statements[i];
        Type *stmt_type = type_check_statement(ctx, stmt);
        
        if (!stmt_type) {
            return NULL;
        }
        
        // last statement's type becomes the program's type
        if (i == program->data.program.statement_count - 1) {
            result_type = stmt_type;
        }
    }
    
    return result_type;
}

// specific type checking functions
Type* type_check_variable_declaration(TypeContext *ctx, ASTNode *decl) {
    if (!ctx || !decl || decl->type != NODE_VARIABLE_DECL) {
        return NULL;
    }
    
    char *name = decl->data.variable_decl.name;
    Type *annotated_type = decl->data.variable_decl.type_annotation;
    ASTNode *initializer = decl->data.variable_decl.initializer;
    
    Type *inferred_type = NULL;
    
    if (initializer) {
        inferred_type = type_infer_expression(ctx, initializer);
        if (!inferred_type) {
            type_context_error(ctx, "cannot infer type for variable", decl->line, decl->column);
            return NULL;
        }
    }
    
    Type *final_type = annotated_type ? annotated_type : inferred_type;
    if (!final_type) {
        type_context_error(ctx, "variable must have a type annotation or initializer", decl->line, decl->column);
        return NULL;
    }
    
    // insert into type environment
    if (!type_env_insert(ctx->env, name, final_type)) {
        type_context_error(ctx, "failed to insert variable into type environment", decl->line, decl->column);
        return NULL;
    }
    
    return final_type;
}

Type* type_check_function_declaration(TypeContext *ctx, ASTNode *func) {
    if (!ctx || !func || func->type != NODE_FUNCTION) {
        return NULL;
    }
    
    char *name = func->data.function.name;
    ASTNode **params = func->data.function.parameters;
    size_t param_count = func->data.function.param_count;
    Type *return_type = func->data.function.return_type;
    ASTNode *body = func->data.function.body;
    
    // create parameter types
    Type **param_types = malloc(param_count * sizeof(Type*));
    for (size_t i = 0; i < param_count; i++) {
        if (params[i] && params[i]->type == NODE_VARIABLE_DECL) {
            param_types[i] = params[i]->data.variable_decl.type_annotation;
        } else {
            param_types[i] = TYPE_UNKNOWN;
        }
    }
    
    // create function type
    Type *func_type = type_create_function(param_types, param_count, return_type);
    
    // insert function into global environment
    type_env_insert(ctx->global_env, name, func_type);
    
    // create new scope for function body
    TypeEnv *old_env = ctx->env;
    ctx->env = type_env_push(old_env);
    
    // insert parameters into scope
    for (size_t i = 0; i < param_count; i++) {
        if (params[i] && params[i]->type == NODE_VARIABLE_DECL) {
            char *param_name = params[i]->data.variable_decl.name;
            Type *param_type = params[i]->data.variable_decl.type_annotation;
            type_env_insert(ctx->env, param_name, param_type);
        }
    }
    
    // type check function body
    Type *body_type = TYPE_UNIT;
    if (body) {
        body_type = type_check_statement(ctx, body);
    }
    
    // check return type compatibility
    if (return_type && body_type && !type_equals(return_type, body_type)) {
        type_context_error(ctx, "function return type does not match body type", func->line, func->column);
    }
    
    // restore environment
    ctx->env = old_env;
    
    return func_type;
}

Type* type_check_if_statement(TypeContext *ctx, ASTNode *stmt) {
    if (!ctx || !stmt || stmt->type != NODE_IF_STATEMENT) {
        return NULL;
    }
    
    // type check condition
    Type *condition_type = type_infer_expression(ctx, stmt->data.if_stmt.condition);
    if (!condition_type) {
        type_context_error(ctx, "cannot infer type for if condition", stmt->line, stmt->column);
        return NULL;
    }
    
    if (!type_is_boolean(condition_type)) {
        type_context_error(ctx, "if condition must be boolean", stmt->line, stmt->column);
        return NULL;
    }
    
    // type check then branch
    Type *then_type = type_check_statement(ctx, stmt->data.if_stmt.then_branch);
    if (!then_type) {
        return NULL;
    }
    
    // type check else branch if present
    Type *else_type = NULL;
    if (stmt->data.if_stmt.else_branch) {
        else_type = type_check_statement(ctx, stmt->data.if_stmt.else_branch);
        if (!else_type) {
            return NULL;
        }
        
        // both branches should have the same type
        if (!type_equals(then_type, else_type)) {
            type_context_error(ctx, "if and else branches must have the same type", stmt->line, stmt->column);
            return NULL;
        }
    }
    
    return then_type;
}

Type* type_check_while_statement(TypeContext *ctx, ASTNode *stmt) {
    if (!ctx || !stmt || stmt->type != NODE_WHILE_STATEMENT) {
        return NULL;
    }
    
    // type check condition
    Type *condition_type = type_infer_expression(ctx, stmt->data.while_stmt.condition);
    if (!condition_type) {
        type_context_error(ctx, "cannot infer type for while condition", stmt->line, stmt->column);
        return NULL;
    }
    
    if (!type_is_boolean(condition_type)) {
        type_context_error(ctx, "while condition must be boolean", stmt->line, stmt->column);
        return NULL;
    }
    
    // type check body
    Type *body_type = type_check_statement(ctx, stmt->data.while_stmt.body);
    if (!body_type) {
        return NULL;
    }
    
    return TYPE_UNIT; // while loops always return unit
}

Type* type_check_for_statement(TypeContext *ctx, ASTNode *stmt) {
    if (!ctx || !stmt || stmt->type != NODE_FOR_STATEMENT) {
        return NULL;
    }
    
    char *iterator = stmt->data.for_stmt.iterator;
    ASTNode *range = stmt->data.for_stmt.range;
    ASTNode *body = stmt->data.for_stmt.body;
    
    // type check range expression
    Type *range_type = type_infer_expression(ctx, range);
    if (!range_type) {
        type_context_error(ctx, "cannot infer type for for loop range", stmt->line, stmt->column);
        return NULL;
    }
    
    // create new scope for loop body
    TypeEnv *old_env = ctx->env;
    ctx->env = type_env_push(old_env);
    
    // insert iterator into scope (assuming it's an integer)
    type_env_insert(ctx->env, iterator, TYPE_USIZE);
    
    // type check body
    Type *body_type = type_check_statement(ctx, body);
    if (!body_type) {
        ctx->env = old_env;
        return NULL;
    }
    
    // restore environment
    ctx->env = old_env;
    
    return TYPE_UNIT; // for loops always return unit
}

Type* type_check_return_statement(TypeContext *ctx, ASTNode *stmt) {
    if (!ctx || !stmt || stmt->type != NODE_RETURN_STATEMENT) {
        return NULL;
    }
    
    ASTNode *value = stmt->data.return_stmt.value;
    
    if (value) {
        Type *value_type = type_infer_expression(ctx, value);
        if (!value_type) {
            type_context_error(ctx, "cannot infer type for return value", stmt->line, stmt->column);
            return NULL;
        }
        return value_type;
    } else {
        return TYPE_UNIT;
    }
}

// type inference functions
Type* type_infer_expression(TypeContext *ctx, ASTNode *expr) {
    if (!ctx || !expr || expr->type != NODE_EXPRESSION) {
        return NULL;
    }
    
    switch (expr->data.expression.expr_type) {
        case EXPR_LITERAL:
            return type_infer_literal(ctx, expr);
            
        case EXPR_IDENTIFIER:
            return type_infer_identifier(ctx, expr);
            
        case EXPR_BINARY_OP:
            return type_infer_binary_op(ctx, expr);
            
        case EXPR_FUNCTION_CALL:
            return type_infer_function_call(ctx, expr);
            
        default:
            type_context_error(ctx, "unsupported expression type for inference", expr->line, expr->column);
            return NULL;
    }
}

Type* type_infer_literal(TypeContext *ctx, ASTNode *literal) {
    if (!ctx || !literal) return NULL;
    
    TokenType token_type = literal->data.expression.expr.literal.token_type;
    char *value = literal->data.expression.expr.literal.value;
    
    switch (token_type) {
        case TOKEN_INTEGER_LITERAL:
            // simple heuristic: if it fits in i32, use i32, otherwise i64
            if (value) {
                long long num = strtoll(value, NULL, 10);
                if (num >= INT_MIN && num <= INT_MAX) {
                    return TYPE_I32;
                } else {
                    return TYPE_I64;
                }
            }
            return TYPE_I32;
            
        case TOKEN_FLOAT_LITERAL:
            return TYPE_F64;
            
        case TOKEN_STRING_LITERAL:
            return TYPE_STR;
            
        case TOKEN_CHAR_LITERAL:
            return TYPE_CHAR;
            
        case TOKEN_TRUE:
        case TOKEN_FALSE:
            return TYPE_BOOL;
            
        default:
            type_context_error(ctx, "unknown literal type", literal->line, literal->column);
            return NULL;
    }
}

Type* type_infer_identifier(TypeContext *ctx, ASTNode *expr) {
    if (!ctx || !expr) return NULL;
    
    char *name = expr->data.expression.expr.identifier;
    Type *type = type_env_lookup(ctx->env, name);
    
    if (!type) {
        type_context_error(ctx, "undefined variable", expr->line, expr->column);
        return NULL;
    }
    
    return type;
}

Type* type_infer_binary_op(TypeContext *ctx, ASTNode *expr) {
    if (!ctx || !expr) return NULL;
    
    BinaryOp op = expr->data.expression.expr.binary_op.op;
    ASTNode *left = expr->data.expression.expr.binary_op.left;
    ASTNode *right = expr->data.expression.expr.binary_op.right;
    
    Type *left_type = type_infer_expression(ctx, left);
    Type *right_type = type_infer_expression(ctx, right);
    
    if (!left_type || !right_type) {
        return NULL;
    }
    
    switch (op) {
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
        case OP_MOD:
            // arithmetic operations
            if (type_is_numeric(left_type) && type_is_numeric(right_type)) {
                // promote to the larger type
                if (type_is_float(left_type) || type_is_float(right_type)) {
                    return TYPE_F64;
                } else {
                    return TYPE_I32; // default to i32 for integer operations
                }
            } else {
                type_context_error(ctx, "arithmetic operation requires numeric types", expr->line, expr->column);
                return NULL;
            }
            
        case OP_EQ:
        case OP_NE:
        case OP_LT:
        case OP_GT:
        case OP_LE:
        case OP_GE:
            // comparison operations
            if (type_is_numeric(left_type) && type_is_numeric(right_type)) {
                return TYPE_BOOL;
            } else {
                type_context_error(ctx, "comparison operation requires numeric types", expr->line, expr->column);
                return NULL;
            }
            
        case OP_AND:
        case OP_OR:
            // logical operations
            if (type_is_boolean(left_type) && type_is_boolean(right_type)) {
                return TYPE_BOOL;
            } else {
                type_context_error(ctx, "logical operation requires boolean types", expr->line, expr->column);
                return NULL;
            }
            
        case OP_ASSIGN:
            // assignment
            if (type_equals(left_type, right_type)) {
                return left_type;
            } else {
                type_context_error(ctx, "assignment types must match", expr->line, expr->column);
                return NULL;
            }
            
        default:
            type_context_error(ctx, "unsupported binary operation", expr->line, expr->column);
            return NULL;
    }
}

Type* type_infer_function_call(TypeContext *ctx, ASTNode *expr) {
    if (!ctx || !expr) return NULL;
    
    char *function_name = expr->data.expression.expr.function_call.function_name;
    ASTNode **arguments = expr->data.expression.expr.function_call.arguments;
    size_t arg_count = expr->data.expression.expr.function_call.arg_count;
    
    // lookup function type
    Type *func_type = type_env_lookup_global(ctx, function_name);
    if (!func_type || func_type->kind != TYPE_FUNCTION) {
        type_context_error(ctx, "undefined function", expr->line, expr->column);
        return NULL;
    }
    
    // check argument count
    if (arg_count != func_type->param_count) {
        type_context_error(ctx, "function argument count mismatch", expr->line, expr->column);
        return NULL;
    }
    
    // check argument types
    for (size_t i = 0; i < arg_count; i++) {
        Type *arg_type = type_infer_expression(ctx, arguments[i]);
        Type *expected_type = func_type->param_types[i];
        
        if (!arg_type) {
            return NULL;
        }
        
        if (!type_equals(arg_type, expected_type)) {
            type_context_error(ctx, "function argument type mismatch", expr->line, expr->column);
            return NULL;
        }
    }
    
    return func_type->return_type;
} 