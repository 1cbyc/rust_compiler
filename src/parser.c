#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// parser initialization
Parser* parser_init(Lexer *lexer) {
    Parser *parser = malloc(sizeof(Parser));
    if (!parser) return NULL;
    
    parser->lexer = lexer;
    parser->current_token = NULL;
    parser->peek_token = NULL;
    parser->had_error = false;
    
    // get first two tokens
    parser->current_token = get_next_token(lexer);
    parser->peek_token = get_next_token(lexer);
    
    return parser;
}

void parser_free(Parser *parser) {
    if (parser) {
        if (parser->current_token) token_free(parser->current_token);
        if (parser->peek_token) token_free(parser->peek_token);
        free(parser);
    }
}

// token advancement
static void advance_token(Parser *parser) {
    if (parser->current_token) token_free(parser->current_token);
    parser->current_token = parser->peek_token;
    parser->peek_token = get_next_token(parser->lexer);
}

// token checking utilities
static bool check_token(Parser *parser, TokenType type) {
    return parser->current_token && parser->current_token->type == type;
}

static bool check_peek(Parser *parser, TokenType type) {
    return parser->peek_token && parser->peek_token->type == type;
}

static bool match_token(Parser *parser, TokenType type) {
    if (check_token(parser, type)) {
        advance_token(parser);
        return true;
    }
    return false;
}

static void expect_token(Parser *parser, TokenType type, const char *error_msg) {
    if (check_token(parser, type)) {
        advance_token(parser);
    } else {
        error_report_syntax(parser->lexer, parser->current_token->line, parser->current_token->column, parser->lexer->filename, error_msg);
        parser->had_error = true;
    }
}

// ast node creation utilities
static ASTNode* create_node(NodeType type, size_t line, size_t column) {
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->type = type;
    node->line = line;
    node->column = column;
    
    return node;
}

static ASTNode* create_literal_node(TokenType token_type, char *value, size_t line, size_t column) {
    ASTNode *node = create_node(NODE_EXPRESSION, line, column);
    if (!node) return NULL;
    
    node->data.expression.expr_type = EXPR_LITERAL;
    node->data.expression.expr.literal.token_type = token_type;
    node->data.expression.expr.literal.value = value ? strdup(value) : NULL;
    
    return node;
}

static ASTNode* create_identifier_node(char *name, size_t line, size_t column) {
    ASTNode *node = create_node(NODE_EXPRESSION, line, column);
    if (!node) return NULL;
    
    node->data.expression.expr_type = EXPR_IDENTIFIER;
    node->data.expression.expr.identifier = strdup(name);
    
    return node;
}

static ASTNode* create_binary_op_node(BinaryOp op, ASTNode *left, ASTNode *right, size_t line, size_t column) {
    ASTNode *node = create_node(NODE_EXPRESSION, line, column);
    if (!node) return NULL;
    
    node->data.expression.expr_type = EXPR_BINARY_OP;
    node->data.expression.expr.binary_op.op = op;
    node->data.expression.expr.binary_op.left = left;
    node->data.expression.expr.binary_op.right = right;
    
    return node;
}

// operator precedence mapping
static int get_precedence(TokenType type) {
    switch (type) {
        case TOKEN_STAR:
        case TOKEN_SLASH:
        case TOKEN_PERCENT:
            return 7;
        case TOKEN_PLUS:
        case TOKEN_MINUS:
            return 6;
        case TOKEN_SHL:
        case TOKEN_SHR:
            return 5;
        case TOKEN_LT:
        case TOKEN_GT:
        case TOKEN_LE:
        case TOKEN_GE:
            return 4;
        case TOKEN_EQ:
        case TOKEN_NE:
            return 3;
        case TOKEN_AND:
        case TOKEN_AND_AND:
            return 2;
        case TOKEN_OR:
        case TOKEN_OR_OR:
        case TOKEN_XOR:
            return 1;
        case TOKEN_ASSIGN:
        case TOKEN_PLUS_ASSIGN:
        case TOKEN_MINUS_ASSIGN:
        case TOKEN_STAR_ASSIGN:
        case TOKEN_SLASH_ASSIGN:
        case TOKEN_PERCENT_ASSIGN:
            return 0;
        default:
            return -1;
    }
}

static BinaryOp token_to_binary_op(TokenType type) {
    switch (type) {
        case TOKEN_PLUS: return OP_ADD;
        case TOKEN_MINUS: return OP_SUB;
        case TOKEN_STAR: return OP_MUL;
        case TOKEN_SLASH: return OP_DIV;
        case TOKEN_PERCENT: return OP_MOD;
        case TOKEN_ASSIGN: return OP_ASSIGN;
        case TOKEN_EQ: return OP_EQ;
        case TOKEN_NE: return OP_NE;
        case TOKEN_LT: return OP_LT;
        case TOKEN_GT: return OP_GT;
        case TOKEN_LE: return OP_LE;
        case TOKEN_GE: return OP_GE;
        case TOKEN_AND: return OP_AND;
        case TOKEN_OR: return OP_OR;
        case TOKEN_XOR: return OP_XOR;
        case TOKEN_SHL: return OP_SHL;
        case TOKEN_SHR: return OP_SHR;
        default: return OP_ADD; // fallback
    }
}

static ASTNode* parse_function_call(Parser *parser, char *function_name) {
    size_t line = parser->current_token->line;
    size_t column = parser->current_token->column;
    
    expect_token(parser, TOKEN_LPAREN, "expected '(' after function name");
    
    ASTNode **arguments = NULL;
    size_t arg_count = 0;
    
    while (!check_token(parser, TOKEN_RPAREN) && !check_token(parser, TOKEN_EOF)) {
        if (arg_count > 0) {
            expect_token(parser, TOKEN_COMMA, "expected ',' between arguments");
        }
        
        ASTNode *arg = parse_expression(parser);
        if (arg) {
            arguments = realloc(arguments, (arg_count + 1) * sizeof(ASTNode*));
            arguments[arg_count++] = arg;
        }
    }
    
    expect_token(parser, TOKEN_RPAREN, "expected ')' after function arguments");
    
    ASTNode *node = create_node(NODE_EXPRESSION, line, column);
    if (!node) {
        for (size_t i = 0; i < arg_count; i++) {
            ast_node_free(arguments[i]);
        }
        free(arguments);
        return NULL;
    }
    
    node->data.expression.expr_type = EXPR_FUNCTION_CALL;
    node->data.expression.expr.function_call.function_name = function_name;
    node->data.expression.expr.function_call.arguments = arguments;
    node->data.expression.expr.function_call.arg_count = arg_count;
    
    return node;
}

// expression parsing
ASTNode* parse_primary_expression(Parser *parser) {
    if (!parser->current_token) return NULL;
    
    size_t line = parser->current_token->line;
    size_t column = parser->current_token->column;
    
    switch (parser->current_token->type) {
        case TOKEN_INTEGER_LITERAL:
        case TOKEN_FLOAT_LITERAL:
        case TOKEN_STRING_LITERAL:
        case TOKEN_CHAR_LITERAL:
        case TOKEN_TRUE:
        case TOKEN_FALSE: {
            TokenType token_type = parser->current_token->type;
            char *value = parser->current_token->value ? strdup(parser->current_token->value) : NULL;
            advance_token(parser);
            return create_literal_node(token_type, value, line, column);
        }
        
        case TOKEN_IDENTIFIER: {
            char *name = strdup(parser->current_token->value);
            advance_token(parser);
            
            // check if this is a function call
            if (check_token(parser, TOKEN_LPAREN)) {
                return parse_function_call(parser, name);
            }
            
            return create_identifier_node(name, line, column);
        }
        
        case TOKEN_LPAREN: {
            advance_token(parser); // consume '('
            ASTNode *expr = parse_expression(parser);
            expect_token(parser, TOKEN_RPAREN, "expected ')' after expression");
            return expr;
        }
        
        default: {
            error_report_syntax(parser->lexer, line, column, parser->lexer->filename, "unexpected token");
            parser->had_error = true;
            advance_token(parser); // skip error token
            return NULL;
        }
    }
}

ASTNode* parse_expression(Parser *parser) {
    return parse_expression_precedence(parser, 0);
}

static ASTNode* parse_expression_precedence(Parser *parser, int precedence) {
    ASTNode *left = parse_primary_expression(parser);
    if (!left) return NULL;
    
    while (parser->current_token && 
           get_precedence(parser->current_token->type) >= precedence) {
        TokenType op_type = parser->current_token->type;
        int op_precedence = get_precedence(op_type);
        
        if (op_precedence < precedence) break;
        
        advance_token(parser); // consume operator
        
        ASTNode *right = parse_expression_precedence(parser, op_precedence + 1);
        if (!right) {
            ast_node_free(left);
            return NULL;
        }
        
        BinaryOp op = token_to_binary_op(op_type);
        ASTNode *new_left = create_binary_op_node(op, left, right, left->line, left->column);
        if (!new_left) {
            ast_node_free(left);
            ast_node_free(right);
            return NULL;
        }
        
        left = new_left;
    }
    
    return left;
}

// statement parsing
ASTNode* parse_statement(Parser *parser) {
    if (!parser->current_token) return NULL;
    
    switch (parser->current_token->type) {
        case TOKEN_FN:
            return parse_function_declaration(parser);
        case TOKEN_LET:
            return parse_variable_declaration(parser);
        case TOKEN_IF:
            return parse_if_statement(parser);
        case TOKEN_WHILE:
            return parse_while_statement(parser);
        case TOKEN_FOR:
            return parse_for_statement(parser);
        case TOKEN_RETURN:
            return parse_return_statement(parser);
        case TOKEN_STRUCT:
            return parse_struct_definition(parser);
        case TOKEN_IMPL:
            return parse_impl_block(parser);
        case TOKEN_ENUM:
            return parse_enum_definition(parser);
        default:
            // expression statement
            return parse_expression(parser);
    }
}

ASTNode* parse_variable_declaration(Parser *parser) {
    size_t line = parser->current_token->line;
    size_t column = parser->current_token->column;
    
    expect_token(parser, TOKEN_LET, "expected 'let'");
    
    bool is_mutable = false;
    if (check_token(parser, TOKEN_MUT)) {
        is_mutable = true;
        advance_token(parser);
    }
    
    if (!check_token(parser, TOKEN_IDENTIFIER)) {
        error_report_syntax(parser->lexer, line, column, parser->lexer->filename, "expected identifier after 'let'");
        parser->had_error = true;
        return NULL;
    }
    
    char *name = strdup(parser->current_token->value);
    advance_token(parser);
    
    ASTNode *type_annotation = NULL;
    if (check_token(parser, TOKEN_COLON)) {
        advance_token(parser);
        // simple type parsing for now
        if (check_token(parser, TOKEN_IDENTIFIER)) {
            type_annotation = create_identifier_node(parser->current_token->value, 
                                                  parser->current_token->line, 
                                                  parser->current_token->column);
            advance_token(parser);
        }
    }
    
    ASTNode *initializer = NULL;
    if (check_token(parser, TOKEN_ASSIGN)) {
        advance_token(parser);
        initializer = parse_expression(parser);
    }
    
    expect_token(parser, TOKEN_SEMICOLON, "expected ';' after variable declaration");
    
    ASTNode *node = create_node(NODE_VARIABLE_DECL, line, column);
    if (!node) return NULL;
    
    node->data.variable_decl.name = name;
    node->data.variable_decl.type_annotation = type_annotation;
    node->data.variable_decl.initializer = initializer;
    node->data.variable_decl.is_mutable = is_mutable;
    
    return node;
}

ASTNode* parse_function_declaration(Parser *parser) {
    size_t line = parser->current_token->line;
    size_t column = parser->current_token->column;
    
    expect_token(parser, TOKEN_FN, "expected 'fn'");
    
    if (!check_token(parser, TOKEN_IDENTIFIER)) {
        error_report_syntax(parser->lexer, line, column, parser->lexer->filename, "expected function name");
        parser->had_error = true;
        return NULL;
    }
    
    char *name = strdup(parser->current_token->value);
    advance_token(parser);
    
    expect_token(parser, TOKEN_LPAREN, "expected '(' after function name");
    
    // parse parameters (simplified for now)
    ASTNode **parameters = NULL;
    size_t param_count = 0;
    
    while (!check_token(parser, TOKEN_RPAREN) && !check_token(parser, TOKEN_EOF)) {
        if (param_count > 0) {
            expect_token(parser, TOKEN_COMMA, "expected ',' between parameters");
        }
        
        if (check_token(parser, TOKEN_IDENTIFIER)) {
            char *param_name = strdup(parser->current_token->value);
            advance_token(parser);
            
            expect_token(parser, TOKEN_COLON, "expected ':' after parameter name");
            
            if (check_token(parser, TOKEN_IDENTIFIER)) {
                ASTNode *param_type = create_identifier_node(parser->current_token->value,
                                                           parser->current_token->line,
                                                           parser->current_token->column);
                advance_token(parser);
                
                // create parameter node (simplified)
                ASTNode *param = create_node(NODE_VARIABLE_DECL, line, column);
                param->data.variable_decl.name = param_name;
                param->data.variable_decl.type_annotation = param_type;
                
                parameters = realloc(parameters, (param_count + 1) * sizeof(ASTNode*));
                parameters[param_count++] = param;
            }
        }
    }
    
    expect_token(parser, TOKEN_RPAREN, "expected ')' after parameters");
    
    // parse return type
    ASTNode *return_type = NULL;
    if (check_token(parser, TOKEN_ARROW)) {
        advance_token(parser);
        if (check_token(parser, TOKEN_IDENTIFIER)) {
            return_type = create_identifier_node(parser->current_token->value,
                                              parser->current_token->line,
                                              parser->current_token->column);
            advance_token(parser);
        }
    }
    
    // parse function body
    expect_token(parser, TOKEN_LBRACE, "expected '{' for function body");
    
    ASTNode *body = parse_block(parser);
    
    ASTNode *node = create_node(NODE_FUNCTION, line, column);
    if (!node) return NULL;
    
    node->data.function.name = name;
    node->data.function.parameters = parameters;
    node->data.function.param_count = param_count;
    node->data.function.return_type = return_type;
    node->data.function.body = body;
    
    return node;
}

static ASTNode* parse_block(Parser *parser) {
    ASTNode **statements = NULL;
    size_t statement_count = 0;
    
    while (!check_token(parser, TOKEN_RBRACE) && !check_token(parser, TOKEN_EOF)) {
        ASTNode *stmt = parse_statement(parser);
        if (stmt) {
            statements = realloc(statements, (statement_count + 1) * sizeof(ASTNode*));
            statements[statement_count++] = stmt;
        }
    }
    
    expect_token(parser, TOKEN_RBRACE, "expected '}' to close block");
    
    ASTNode *node = create_node(NODE_BLOCK, parser->current_token->line, parser->current_token->column);
    if (!node) return NULL;
    
    node->data.program.statements = statements;
    node->data.program.statement_count = statement_count;
    
    return node;
}

ASTNode* parse_if_statement(Parser *parser) {
    size_t line = parser->current_token->line;
    size_t column = parser->current_token->column;
    
    expect_token(parser, TOKEN_IF, "expected 'if'");
    expect_token(parser, TOKEN_LPAREN, "expected '(' after 'if'");
    
    ASTNode *condition = parse_expression(parser);
    if (!condition) {
        error_report_syntax(parser->lexer, line, column, parser->lexer->filename, "expected condition in if statement");
        parser->had_error = true;
        return NULL;
    }
    
    expect_token(parser, TOKEN_RPAREN, "expected ')' after if condition");
    
    ASTNode *then_branch = parse_statement(parser);
    if (!then_branch) {
        ast_node_free(condition);
        return NULL;
    }
    
    ASTNode *else_branch = NULL;
    if (check_token(parser, TOKEN_ELSE)) {
        advance_token(parser); // consume 'else'
        else_branch = parse_statement(parser);
    }
    
    ASTNode *node = create_node(NODE_IF_STATEMENT, line, column);
    if (!node) {
        ast_node_free(condition);
        ast_node_free(then_branch);
        ast_node_free(else_branch);
        return NULL;
    }
    
    node->data.if_stmt.condition = condition;
    node->data.if_stmt.then_branch = then_branch;
    node->data.if_stmt.else_branch = else_branch;
    
    return node;
}

ASTNode* parse_while_statement(Parser *parser) {
    size_t line = parser->current_token->line;
    size_t column = parser->current_token->column;
    
    expect_token(parser, TOKEN_WHILE, "expected 'while'");
    expect_token(parser, TOKEN_LPAREN, "expected '(' after 'while'");
    
    ASTNode *condition = parse_expression(parser);
    if (!condition) {
        error_report_syntax(parser->lexer, line, column, parser->lexer->filename, "expected condition in while statement");
        parser->had_error = true;
        return NULL;
    }
    
    expect_token(parser, TOKEN_RPAREN, "expected ')' after while condition");
    
    ASTNode *body = parse_statement(parser);
    if (!body) {
        ast_node_free(condition);
        return NULL;
    }
    
    ASTNode *node = create_node(NODE_WHILE_STATEMENT, line, column);
    if (!node) {
        ast_node_free(condition);
        ast_node_free(body);
        return NULL;
    }
    
    node->data.while_stmt.condition = condition;
    node->data.while_stmt.body = body;
    
    return node;
}

ASTNode* parse_for_statement(Parser *parser) {
    size_t line = parser->current_token->line;
    size_t column = parser->current_token->column;
    
    expect_token(parser, TOKEN_FOR, "expected 'for'");
    
    if (!check_token(parser, TOKEN_IDENTIFIER)) {
        error_report_syntax(parser->lexer, line, column, parser->lexer->filename, "expected iterator name after 'for'");
        parser->had_error = true;
        return NULL;
    }
    
    char *iterator = strdup(parser->current_token->value);
    advance_token(parser);
    
    expect_token(parser, TOKEN_IN, "expected 'in' after iterator name");
    
    ASTNode *range = parse_expression(parser);
    if (!range) {
        free(iterator);
        return NULL;
    }
    
    ASTNode *body = parse_statement(parser);
    if (!body) {
        free(iterator);
        ast_node_free(range);
        return NULL;
    }
    
    ASTNode *node = create_node(NODE_FOR_STATEMENT, line, column);
    if (!node) {
        free(iterator);
        ast_node_free(range);
        ast_node_free(body);
        return NULL;
    }
    
    node->data.for_stmt.iterator = iterator;
    node->data.for_stmt.range = range;
    node->data.for_stmt.body = body;
    
    return node;
}

ASTNode* parse_return_statement(Parser *parser) {
    size_t line = parser->current_token->line;
    size_t column = parser->current_token->column;
    
    expect_token(parser, TOKEN_RETURN, "expected 'return'");
    
    ASTNode *value = NULL;
    if (!check_token(parser, TOKEN_SEMICOLON)) {
        value = parse_expression(parser);
    }
    
    expect_token(parser, TOKEN_SEMICOLON, "expected ';' after return statement");
    
    ASTNode *node = create_node(NODE_RETURN_STATEMENT, line, column);
    if (!node) {
        ast_node_free(value);
        return NULL;
    }
    
    node->data.return_stmt.value = value;
    
    return node;
}

ASTNode* parse_struct_definition(Parser *parser) {
    size_t line = parser->current_token->line;
    size_t column = parser->current_token->column;
    
    expect_token(parser, TOKEN_STRUCT, "expected 'struct'");
    
    if (!check_token(parser, TOKEN_IDENTIFIER)) {
        error_report_syntax(parser->lexer, line, column, parser->lexer->filename, "expected struct name");
        parser->had_error = true;
        return NULL;
    }
    
    char *name = strdup(parser->current_token->value);
    advance_token(parser);
    
    expect_token(parser, TOKEN_LBRACE, "expected '{' after struct name");
    
    ASTNode **fields = NULL;
    size_t field_count = 0;
    
    while (!check_token(parser, TOKEN_RBRACE) && !check_token(parser, TOKEN_EOF)) {
        if (field_count > 0) {
            expect_token(parser, TOKEN_COMMA, "expected ',' between struct fields");
        }
        
        if (check_token(parser, TOKEN_IDENTIFIER)) {
            char *field_name = strdup(parser->current_token->value);
            advance_token(parser);
            
            expect_token(parser, TOKEN_COLON, "expected ':' after field name");
            
            if (check_token(parser, TOKEN_IDENTIFIER)) {
                ASTNode *field_type = create_identifier_node(parser->current_token->value,
                                                           parser->current_token->line,
                                                           parser->current_token->column);
                advance_token(parser);
                
                ASTNode *field = create_node(NODE_STRUCT_FIELD, line, column);
                if (field) {
                    field->data.struct_field.name = field_name;
                    field->data.struct_field.type = field_type;
                    
                    fields = realloc(fields, (field_count + 1) * sizeof(ASTNode*));
                    fields[field_count++] = field;
                }
            }
        }
    }
    
    expect_token(parser, TOKEN_RBRACE, "expected '}' to close struct definition");
    expect_token(parser, TOKEN_SEMICOLON, "expected ';' after struct definition");
    
    ASTNode *node = create_node(NODE_STRUCT_DEF, line, column);
    if (!node) {
        free(name);
        for (size_t i = 0; i < field_count; i++) {
            ast_node_free(fields[i]);
        }
        free(fields);
        return NULL;
    }
    
    node->data.struct_def.name = name;
    node->data.struct_def.fields = fields;
    node->data.struct_def.field_count = field_count;
    
    return node;
}

ASTNode* parse_impl_block(Parser *parser) {
    size_t line = parser->current_token->line;
    size_t column = parser->current_token->column;
    
    expect_token(parser, TOKEN_IMPL, "expected 'impl'");
    
    if (!check_token(parser, TOKEN_IDENTIFIER)) {
        error_report_syntax(parser->lexer, line, column, parser->lexer->filename, "expected type name after 'impl'");
        parser->had_error = true;
        return NULL;
    }
    
    char *type_name = strdup(parser->current_token->value);
    advance_token(parser);
    
    expect_token(parser, TOKEN_LBRACE, "expected '{' after impl type name");
    
    ASTNode **methods = NULL;
    size_t method_count = 0;
    
    while (!check_token(parser, TOKEN_RBRACE) && !check_token(parser, TOKEN_EOF)) {
        if (check_token(parser, TOKEN_FN)) {
            ASTNode *method = parse_function_declaration(parser);
            if (method) {
                methods = realloc(methods, (method_count + 1) * sizeof(ASTNode*));
                methods[method_count++] = method;
            }
        } else {
            advance_token(parser); // skip unexpected token
        }
    }
    
    expect_token(parser, TOKEN_RBRACE, "expected '}' to close impl block");
    
    ASTNode *node = create_node(NODE_IMPL_BLOCK, line, column);
    if (!node) {
        free(type_name);
        for (size_t i = 0; i < method_count; i++) {
            ast_node_free(methods[i]);
        }
        free(methods);
        return NULL;
    }
    
    node->data.impl_block.type_name = type_name;
    node->data.impl_block.methods = methods;
    node->data.impl_block.method_count = method_count;
    
    return node;
}

ASTNode* parse_enum_definition(Parser *parser) {
    size_t line = parser->current_token->line;
    size_t column = parser->current_token->column;
    
    expect_token(parser, TOKEN_ENUM, "expected 'enum'");
    
    if (!check_token(parser, TOKEN_IDENTIFIER)) {
        error_report_syntax(parser->lexer, line, column, parser->lexer->filename, "expected enum name");
        parser->had_error = true;
        return NULL;
    }
    
    char *name = strdup(parser->current_token->value);
    advance_token(parser);
    
    expect_token(parser, TOKEN_LBRACE, "expected '{' after enum name");
    
    ASTNode **variants = NULL;
    size_t variant_count = 0;
    
    while (!check_token(parser, TOKEN_RBRACE) && !check_token(parser, TOKEN_EOF)) {
        if (variant_count > 0) {
            expect_token(parser, TOKEN_COMMA, "expected ',' between enum variants");
        }
        
        if (check_token(parser, TOKEN_IDENTIFIER)) {
            char *variant_name = strdup(parser->current_token->value);
            advance_token(parser);
            
            ASTNode *variant = create_node(NODE_ENUM_VARIANT, line, column);
            if (variant) {
                variant->data.enum_variant.name = variant_name;
                variant->data.enum_variant.fields = NULL;
                variant->data.enum_variant.field_count = 0;
                
                // handle tuple-like variants: `Variant(Type1, Type2)`
                if (check_token(parser, TOKEN_LPAREN)) {
                    advance_token(parser);
                    
                    ASTNode **fields = NULL;
                    size_t field_count = 0;
                    
                    while (!check_token(parser, TOKEN_RPAREN) && !check_token(parser, TOKEN_EOF)) {
                        if (field_count > 0) {
                            expect_token(parser, TOKEN_COMMA, "expected ',' between variant fields");
                        }
                        
                        if (check_token(parser, TOKEN_IDENTIFIER)) {
                            ASTNode *field_type = create_identifier_node(parser->current_token->value,
                                                                       parser->current_token->line,
                                                                       parser->current_token->column);
                            advance_token(parser);
                            
                            fields = realloc(fields, (field_count + 1) * sizeof(ASTNode*));
                            fields[field_count++] = field_type;
                        }
                    }
                    
                    expect_token(parser, TOKEN_RPAREN, "expected ')' to close variant fields");
                    variant->data.enum_variant.fields = fields;
                    variant->data.enum_variant.field_count = field_count;
                }
                
                variants = realloc(variants, (variant_count + 1) * sizeof(ASTNode*));
                variants[variant_count++] = variant;
            }
        }
    }
    
    expect_token(parser, TOKEN_RBRACE, "expected '}' to close enum definition");
    expect_token(parser, TOKEN_SEMICOLON, "expected ';' after enum definition");
    
    ASTNode *node = create_node(NODE_ENUM_DEF, line, column);
    if (!node) {
        free(name);
        for (size_t i = 0; i < variant_count; i++) {
            ast_node_free(variants[i]);
        }
        free(variants);
        return NULL;
    }
    
    node->data.enum_def.name = name;
    node->data.enum_def.variants = variants;
    node->data.enum_def.variant_count = variant_count;
    
    return node;
}

// program parsing
ASTNode* parse_program(Parser *parser) {
    ASTNode **statements = NULL;
    size_t statement_count = 0;
    
    while (!check_token(parser, TOKEN_EOF)) {
        ASTNode *stmt = parse_statement(parser);
        if (stmt) {
            statements = realloc(statements, (statement_count + 1) * sizeof(ASTNode*));
            statements[statement_count++] = stmt;
        }
    }
    
    ASTNode *node = create_node(NODE_PROGRAM, 1, 1);
    if (!node) return NULL;
    
    node->data.program.statements = statements;
    node->data.program.statement_count = statement_count;
    
    return node;
}

// utility functions
void ast_node_free(ASTNode *node) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_PROGRAM:
            for (size_t i = 0; i < node->data.program.statement_count; i++) {
                ast_node_free(node->data.program.statements[i]);
            }
            free(node->data.program.statements);
            break;
            
        case NODE_FUNCTION:
            free(node->data.function.name);
            for (size_t i = 0; i < node->data.function.param_count; i++) {
                ast_node_free(node->data.function.parameters[i]);
            }
            free(node->data.function.parameters);
            ast_node_free(node->data.function.return_type);
            ast_node_free(node->data.function.body);
            break;
            
        case NODE_VARIABLE_DECL:
            free(node->data.variable_decl.name);
            ast_node_free(node->data.variable_decl.type_annotation);
            ast_node_free(node->data.variable_decl.initializer);
            break;
            
        case NODE_EXPRESSION:
            switch (node->data.expression.expr_type) {
                case EXPR_LITERAL:
                    free(node->data.expression.expr.literal.value);
                    break;
                case EXPR_IDENTIFIER:
                    free(node->data.expression.expr.identifier);
                    break;
                case EXPR_BINARY_OP:
                    ast_node_free(node->data.expression.expr.binary_op.left);
                    ast_node_free(node->data.expression.expr.binary_op.right);
                    break;
                case EXPR_UNARY_OP:
                    ast_node_free(node->data.expression.expr.unary_op.operand);
                    break;
                case EXPR_FUNCTION_CALL:
                    free(node->data.expression.expr.function_call.function_name);
                    for (size_t i = 0; i < node->data.expression.expr.function_call.arg_count; i++) {
                        ast_node_free(node->data.expression.expr.function_call.arguments[i]);
                    }
                    free(node->data.expression.expr.function_call.arguments);
                    break;
                case EXPR_BLOCK:
                    for (size_t i = 0; i < node->data.expression.expr.block.statement_count; i++) {
                        ast_node_free(node->data.expression.expr.block.statements[i]);
                    }
                    free(node->data.expression.expr.block.statements);
                    break;
            }
            break;
            
        case NODE_IF_STATEMENT:
            ast_node_free(node->data.if_stmt.condition);
            ast_node_free(node->data.if_stmt.then_branch);
            ast_node_free(node->data.if_stmt.else_branch);
            break;
            
        case NODE_WHILE_STATEMENT:
            ast_node_free(node->data.while_stmt.condition);
            ast_node_free(node->data.while_stmt.body);
            break;
            
        case NODE_FOR_STATEMENT:
            free(node->data.for_stmt.iterator);
            ast_node_free(node->data.for_stmt.range);
            ast_node_free(node->data.for_stmt.body);
            break;
            
        case NODE_RETURN_STATEMENT:
            ast_node_free(node->data.return_stmt.value);
            break;
            
        case NODE_STRUCT_DEF:
            free(node->data.struct_def.name);
            for (size_t i = 0; i < node->data.struct_def.field_count; i++) {
                ast_node_free(node->data.struct_def.fields[i]);
            }
            free(node->data.struct_def.fields);
            break;
            
        case NODE_STRUCT_FIELD:
            free(node->data.struct_field.name);
            ast_node_free(node->data.struct_field.type);
            break;
            
        case NODE_ENUM_DEF:
            free(node->data.enum_def.name);
            for (size_t i = 0; i < node->data.enum_def.variant_count; i++) {
                ast_node_free(node->data.enum_def.variants[i]);
            }
            free(node->data.enum_def.variants);
            break;
            
        case NODE_ENUM_VARIANT:
            free(node->data.enum_variant.name);
            for (size_t i = 0; i < node->data.enum_variant.field_count; i++) {
                ast_node_free(node->data.enum_variant.fields[i]);
            }
            free(node->data.enum_variant.fields);
            break;
            
        case NODE_IMPL_BLOCK:
            free(node->data.impl_block.type_name);
            for (size_t i = 0; i < node->data.impl_block.method_count; i++) {
                ast_node_free(node->data.impl_block.methods[i]);
            }
            free(node->data.impl_block.methods);
            break;
            
        case NODE_TYPE_ANNOTATION:
            free(node->data.type_annotation.type_name);
            ast_node_free(node->data.type_annotation.generic_type);
            break;
            
        default:
            break;
    }
    
    free(node);
}

const char* node_type_to_string(NodeType type) {
    switch (type) {
        case NODE_PROGRAM: return "PROGRAM";
        case NODE_FUNCTION: return "FUNCTION";
        case NODE_VARIABLE_DECL: return "VARIABLE_DECL";
        case NODE_EXPRESSION: return "EXPRESSION";
        case NODE_LITERAL: return "LITERAL";
        case NODE_BINARY_OP: return "BINARY_OP";
        case NODE_UNARY_OP: return "UNARY_OP";
        case NODE_IDENTIFIER: return "IDENTIFIER";
        case NODE_FUNCTION_CALL: return "FUNCTION_CALL";
        case NODE_BLOCK: return "BLOCK";
        case NODE_IF_STATEMENT: return "IF_STATEMENT";
        case NODE_WHILE_STATEMENT: return "WHILE_STATEMENT";
        case NODE_FOR_STATEMENT: return "FOR_STATEMENT";
        case NODE_RETURN_STATEMENT: return "RETURN_STATEMENT";
        case NODE_STRUCT_DEF: return "STRUCT_DEF";
        case NODE_STRUCT_FIELD: return "STRUCT_FIELD";
        case NODE_IMPL_BLOCK: return "IMPL_BLOCK";
        case NODE_TYPE_ANNOTATION: return "TYPE_ANNOTATION";
        default: return "UNKNOWN";
    }
}

const char* binary_op_to_string(BinaryOp op) {
    switch (op) {
        case OP_ADD: return "+";
        case OP_SUB: return "-";
        case OP_MUL: return "*";
        case OP_DIV: return "/";
        case OP_MOD: return "%";
        case OP_ASSIGN: return "=";
        case OP_EQ: return "==";
        case OP_NE: return "!=";
        case OP_LT: return "<";
        case OP_GT: return ">";
        case OP_LE: return "<=";
        case OP_GE: return ">=";
        case OP_AND: return "&";
        case OP_OR: return "|";
        case OP_XOR: return "^";
        case OP_SHL: return "<<";
        case OP_SHR: return ">>";
        default: return "?";
    }
}

const char* unary_op_to_string(UnaryOp op) {
    switch (op) {
        case UOP_PLUS: return "+";
        case UOP_MINUS: return "-";
        case UOP_NOT: return "!";
        case UOP_DEREF: return "*";
        case UOP_REF: return "&";
        default: return "?";
    }
}

void print_ast(ASTNode *node, int indent) {
    if (!node) return;
    
    for (int i = 0; i < indent; i++) printf("  ");
    printf("%s", node_type_to_string(node->type));
    
    switch (node->type) {
        case NODE_PROGRAM:
            printf(" (%zu statements)\n", node->data.program.statement_count);
            for (size_t i = 0; i < node->data.program.statement_count; i++) {
                print_ast(node->data.program.statements[i], indent + 1);
            }
            break;
            
        case NODE_FUNCTION:
            printf(" '%s' (%zu parameters)\n", node->data.function.name, node->data.function.param_count);
            for (size_t i = 0; i < node->data.function.param_count; i++) {
                print_ast(node->data.function.parameters[i], indent + 1);
            }
            if (node->data.function.return_type) {
                for (int i = 0; i < indent + 1; i++) printf("  ");
                printf("return type: ");
                print_ast(node->data.function.return_type, 0);
            }
            print_ast(node->data.function.body, indent + 1);
            break;
            
        case NODE_VARIABLE_DECL:
            printf(" '%s' %s\n", node->data.variable_decl.name, 
                   node->data.variable_decl.is_mutable ? "(mutable)" : "(immutable)");
            if (node->data.variable_decl.type_annotation) {
                for (int i = 0; i < indent + 1; i++) printf("  ");
                printf("type: ");
                print_ast(node->data.variable_decl.type_annotation, 0);
            }
            if (node->data.variable_decl.initializer) {
                print_ast(node->data.variable_decl.initializer, indent + 1);
            }
            break;
            
        case NODE_EXPRESSION:
            switch (node->data.expression.expr_type) {
                case EXPR_LITERAL:
                    printf(" '%s'\n", node->data.expression.expr.literal.value ? 
                           node->data.expression.expr.literal.value : "null");
                    break;
                case EXPR_IDENTIFIER:
                    printf(" '%s'\n", node->data.expression.expr.identifier);
                    break;
                case EXPR_BINARY_OP:
                    printf(" %s\n", binary_op_to_string(node->data.expression.expr.binary_op.op));
                    print_ast(node->data.expression.expr.binary_op.left, indent + 1);
                    print_ast(node->data.expression.expr.binary_op.right, indent + 1);
                    break;
                case EXPR_FUNCTION_CALL:
                    printf(" '%s' (%zu arguments)\n", node->data.expression.expr.function_call.function_name,
                           node->data.expression.expr.function_call.arg_count);
                    for (size_t i = 0; i < node->data.expression.expr.function_call.arg_count; i++) {
                        print_ast(node->data.expression.expr.function_call.arguments[i], indent + 1);
                    }
                    break;
                default:
                    printf("\n");
                    break;
            }
            break;
            
        case NODE_IF_STATEMENT:
            printf("\n");
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("condition: ");
            print_ast(node->data.if_stmt.condition, 0);
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("then: ");
            print_ast(node->data.if_stmt.then_branch, 0);
            if (node->data.if_stmt.else_branch) {
                for (int i = 0; i < indent + 1; i++) printf("  ");
                printf("else: ");
                print_ast(node->data.if_stmt.else_branch, 0);
            }
            break;
            
        case NODE_WHILE_STATEMENT:
            printf("\n");
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("condition: ");
            print_ast(node->data.while_stmt.condition, 0);
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("body: ");
            print_ast(node->data.while_stmt.body, 0);
            break;
            
        case NODE_FOR_STATEMENT:
            printf(" '%s'\n", node->data.for_stmt.iterator);
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("range: ");
            print_ast(node->data.for_stmt.range, 0);
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("body: ");
            print_ast(node->data.for_stmt.body, 0);
            break;
            
        case NODE_RETURN_STATEMENT:
            if (node->data.return_stmt.value) {
                printf("\n");
                print_ast(node->data.return_stmt.value, indent + 1);
            } else {
                printf(" (void)\n");
            }
            break;
            
        case NODE_STRUCT_DEF:
            printf(" '%s' (%zu fields)\n", node->data.struct_def.name, node->data.struct_def.field_count);
            for (size_t i = 0; i < node->data.struct_def.field_count; i++) {
                print_ast(node->data.struct_def.fields[i], indent + 1);
            }
            break;
            
        case NODE_STRUCT_FIELD:
            printf(" '%s': ", node->data.struct_field.name);
            print_ast(node->data.struct_field.type, 0);
            break;
            
        case NODE_ENUM_DEF:
            printf(" '%s' (%zu variants)\n", node->data.enum_def.name, node->data.enum_def.variant_count);
            for (size_t i = 0; i < node->data.enum_def.variant_count; i++) {
                print_ast(node->data.enum_def.variants[i], indent + 1);
            }
            break;
            
        case NODE_ENUM_VARIANT:
            printf(" '%s' (%zu fields)\n", node->data.enum_variant.name, node->data.enum_variant.field_count);
            for (size_t i = 0; i < node->data.enum_variant.field_count; i++) {
                print_ast(node->data.enum_variant.fields[i], indent + 1);
            }
            break;
            
        case NODE_IMPL_BLOCK:
            printf(" '%s' (%zu methods)\n", node->data.impl_block.type_name, node->data.impl_block.method_count);
            for (size_t i = 0; i < node->data.impl_block.method_count; i++) {
                print_ast(node->data.impl_block.methods[i], indent + 1);
            }
            break;
            
        default:
            printf("\n");
            break;
    }
} 