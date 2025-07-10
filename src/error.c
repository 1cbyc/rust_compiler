#include "error.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

// Error context management
ErrorContext* error_context_create(size_t max_errors) {
    ErrorContext *ctx = malloc(sizeof(ErrorContext));
    ctx->errors = NULL;
    ctx->error_count = 0;
    ctx->error_capacity = 0;
    ctx->had_error = false;
    ctx->had_fatal = false;
    ctx->max_errors = max_errors;
    return ctx;
}

void error_context_free(ErrorContext *ctx) {
    if (!ctx) return;
    
    for (size_t i = 0; i < ctx->error_count; i++) {
        ErrorMessage *error = ctx->errors[i];
        free(error->message);
        free(error->suggestion);
        free(error->location.filename);
        free(error);
    }
    free(ctx->errors);
    free(ctx);
}

// Error reporting
bool error_report(ErrorContext *ctx, ErrorType type, ErrorSeverity severity,
                 const char *message, SourceLocation location, const char *suggestion) {
    if (!ctx || !message) return false;
    
    // Check if we've exceeded max errors
    if (ctx->error_count >= ctx->max_errors) {
        return false;
    }
    
    // Expand capacity if needed
    if (ctx->error_count >= ctx->error_capacity) {
        size_t new_capacity = ctx->error_capacity == 0 ? 8 : ctx->error_capacity * 2;
        ErrorMessage **new_errors = realloc(ctx->errors, 
                                          new_capacity * sizeof(ErrorMessage*));
        if (!new_errors) return false;
        ctx->errors = new_errors;
        ctx->error_capacity = new_capacity;
    }
    
    // Create error message
    ErrorMessage *error = malloc(sizeof(ErrorMessage));
    error->type = type;
    error->severity = severity;
    error->message = strdup(message);
    error->location = location;
    error->suggestion = suggestion ? strdup(suggestion) : NULL;
    error->recovered = false;
    
    ctx->errors[ctx->error_count++] = error;
    
    // Update context flags
    if (severity >= ERROR_ERROR) {
        ctx->had_error = true;
    }
    if (severity == ERROR_FATAL) {
        ctx->had_fatal = true;
    }
    
    return true;
}

bool error_report_lexical(ErrorContext *ctx, const char *message, 
                         size_t line, size_t column, const char *filename) {
    SourceLocation loc = source_location_create(line, column, filename);
    return error_report(ctx, ERROR_LEXICAL, ERROR_ERROR, message, loc, NULL);
}

bool error_report_syntax(ErrorContext *ctx, const char *message,
                        size_t line, size_t column, const char *filename) {
    SourceLocation loc = source_location_create(line, column, filename);
    return error_report(ctx, ERROR_SYNTAX, ERROR_ERROR, message, loc, NULL);
}

bool error_report_type(ErrorContext *ctx, const char *message,
                      size_t line, size_t column, const char *filename) {
    SourceLocation loc = source_location_create(line, column, filename);
    return error_report(ctx, ERROR_TYPE, ERROR_ERROR, message, loc, NULL);
}

bool error_report_semantic(ErrorContext *ctx, const char *message,
                          size_t line, size_t column, const char *filename) {
    SourceLocation loc = source_location_create(line, column, filename);
    return error_report(ctx, ERROR_SEMANTIC, ERROR_ERROR, message, loc, NULL);
}

// Error recovery
bool error_can_recover(ErrorContext *ctx) {
    if (!ctx) return false;
    return !ctx->had_fatal && ctx->error_count < ctx->max_errors;
}

bool error_try_recover(ErrorContext *ctx, ErrorType type) {
    if (!ctx) return false;
    
    // Simple recovery strategies based on error type
    switch (type) {
        case ERROR_LEXICAL:
            // Skip to next token
            return true;
        case ERROR_SYNTAX:
            // Skip to next statement
            return true;
        case ERROR_TYPE:
            // Use default type or skip
            return true;
        case ERROR_SEMANTIC:
            // Skip problematic code
            return true;
        default:
            return false;
    }
}

void error_clear_recovered(ErrorContext *ctx) {
    if (!ctx) return;
    
    for (size_t i = 0; i < ctx->error_count; i++) {
        ErrorMessage *error = ctx->errors[i];
        if (error->recovered) {
            // Remove recovered errors
            free(error->message);
            free(error->suggestion);
            free(error->location.filename);
            free(error);
            
            // Shift remaining errors
            for (size_t j = i; j < ctx->error_count - 1; j++) {
                ctx->errors[j] = ctx->errors[j + 1];
            }
            ctx->error_count--;
            i--; // Recheck this index
        }
    }
}

// Debug information functions
DebugInfo* debug_info_create(SourceLocation location, const char *function_name) {
    DebugInfo *info = malloc(sizeof(DebugInfo));
    info->location = location;
    info->function_name = function_name ? strdup(function_name) : NULL;
    info->variable_name = NULL;
    info->type_name = NULL;
    info->stack_depth = 0;
    return info;
}

void debug_info_free(DebugInfo *info) {
    if (!info) return;
    free(info->function_name);
    free(info->variable_name);
    free(info->type_name);
    free(info);
}

void debug_info_add_variable(DebugInfo *info, const char *name, const char *type) {
    if (!info) return;
    free(info->variable_name);
    free(info->type_name);
    info->variable_name = name ? strdup(name) : NULL;
    info->type_name = type ? strdup(type) : NULL;
}

void debug_info_set_stack_depth(DebugInfo *info, size_t depth) {
    if (!info) return;
    info->stack_depth = depth;
}

// Stack trace functions
StackTrace* stack_trace_create(void) {
    StackTrace *trace = malloc(sizeof(StackTrace));
    trace->frames = NULL;
    trace->frame_count = 0;
    trace->frame_capacity = 0;
    return trace;
}

void stack_trace_free(StackTrace *trace) {
    if (!trace) return;
    
    for (size_t i = 0; i < trace->frame_count; i++) {
        StackFrame *frame = trace->frames[i];
        free(frame->function_name);
        free(frame->location.filename);
        for (size_t j = 0; j < frame->variable_count; j++) {
            free(frame->variables[j]);
        }
        free(frame->variables);
        free(frame);
    }
    free(trace->frames);
    free(trace);
}

bool stack_trace_add_frame(StackTrace *trace, const char *function_name, 
                          SourceLocation location) {
    if (!trace || !function_name) return false;
    
    // Expand capacity if needed
    if (trace->frame_count >= trace->frame_capacity) {
        size_t new_capacity = trace->frame_capacity == 0 ? 8 : trace->frame_capacity * 2;
        StackFrame **new_frames = realloc(trace->frames, 
                                        new_capacity * sizeof(StackFrame*));
        if (!new_frames) return false;
        trace->frames = new_frames;
        trace->frame_capacity = new_capacity;
    }
    
    // Create frame
    StackFrame *frame = malloc(sizeof(StackFrame));
    frame->function_name = strdup(function_name);
    frame->location = location;
    frame->variables = NULL;
    frame->variable_count = 0;
    
    trace->frames[trace->frame_count++] = frame;
    return true;
}

void stack_trace_print(StackTrace *trace, FILE *output) {
    if (!trace || !output) return;
    
    fprintf(output, "Stack trace:\n");
    for (size_t i = 0; i < trace->frame_count; i++) {
        StackFrame *frame = trace->frames[i];
        fprintf(output, "  %zu: %s at %s:%zu:%zu\n", 
                i, frame->function_name, 
                frame->location.filename ? frame->location.filename : "unknown",
                frame->location.line, frame->location.column);
    }
}

// Error formatting and output
void error_print_summary(ErrorContext *ctx, FILE *output) {
    if (!ctx || !output) return;
    
    ErrorStats stats;
    error_get_stats(ctx, &stats);
    
    fprintf(output, "Compilation %s\n", stats.fatals > 0 ? "failed" : "completed");
    fprintf(output, "Errors: %zu, Warnings: %zu\n", stats.errors, stats.warnings);
}

void error_print_detailed(ErrorContext *ctx, FILE *output) {
    if (!ctx || !output) return;
    
    for (size_t i = 0; i < ctx->error_count; i++) {
        ErrorMessage *error = ctx->errors[i];
        char *formatted = error_format_message(error);
        fprintf(output, "%s\n", formatted);
        free(formatted);
    }
}

char* error_format_message(ErrorMessage *error) {
    if (!error) return NULL;
    
    const char *severity_str[] = {"INFO", "WARNING", "ERROR", "FATAL"};
    const char *type_str[] = {"LEXICAL", "SYNTAX", "TYPE", "SEMANTIC", 
                              "CODE_GEN", "OPTIMIZATION", "STDLIB"};
    
    char *result = malloc(1024);
    snprintf(result, 1024, "%s %s: %s at %s:%zu:%zu", 
             severity_str[error->severity], type_str[error->type],
             error->message, 
             error->location.filename ? error->location.filename : "unknown",
             error->location.line, error->location.column);
    
    if (error->suggestion) {
        char *new_result = realloc(result, strlen(result) + strlen(error->suggestion) + 10);
        if (new_result) {
            result = new_result;
            strcat(result, "\nSuggestion: ");
            strcat(result, error->suggestion);
        }
    }
    
    return result;
}

void error_print_source_line(const char *source, SourceLocation location, FILE *output) {
    if (!source || !output) return;
    
    // Find the line
    size_t line = 1;
    size_t col = 1;
    size_t i = 0;
    
    while (i < location.offset && source[i] != '\0') {
        if (source[i] == '\n') {
            line++;
            col = 1;
        } else {
            col++;
        }
        i++;
    }
    
    // Print the line with marker
    fprintf(output, "%zu | ", line);
    
    // Find start of line
    size_t line_start = i;
    while (line_start > 0 && source[line_start - 1] != '\n') {
        line_start--;
    }
    
    // Print the line
    for (size_t j = line_start; source[j] != '\0' && source[j] != '\n'; j++) {
        fputc(source[j], output);
    }
    fprintf(output, "\n");
    
    // Print marker
    fprintf(output, "   ");
    for (size_t j = 0; j < col - 1; j++) {
        fputc(' ', output);
    }
    fprintf(output, "^\n");
}

// Source location utilities
SourceLocation source_location_create(size_t line, size_t column, const char *filename) {
    SourceLocation loc;
    loc.line = line;
    loc.column = column;
    loc.offset = 0;
    loc.filename = filename ? strdup(filename) : NULL;
    return loc;
}

SourceLocation source_location_from_offset(const char *source, size_t offset, const char *filename) {
    SourceLocation loc = {0};
    loc.filename = filename ? strdup(filename) : NULL;
    loc.offset = offset;
    
    // Calculate line and column
    size_t line = 1;
    size_t column = 1;
    
    for (size_t i = 0; i < offset && source[i] != '\0'; i++) {
        if (source[i] == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
    }
    
    loc.line = line;
    loc.column = column;
    return loc;
}

void source_location_update(SourceLocation *loc, char c) {
    if (!loc) return;
    
    loc->offset++;
    if (c == '\n') {
        loc->line++;
        loc->column = 1;
    } else {
        loc->column++;
    }
}

// Error statistics
void error_get_stats(ErrorContext *ctx, ErrorStats *stats) {
    if (!ctx || !stats) return;
    
    memset(stats, 0, sizeof(ErrorStats));
    
    for (size_t i = 0; i < ctx->error_count; i++) {
        ErrorMessage *error = ctx->errors[i];
        stats->total_errors++;
        
        switch (error->severity) {
            case ERROR_WARNING:
                stats->warnings++;
                break;
            case ERROR_ERROR:
                stats->errors++;
                break;
            case ERROR_FATAL:
                stats->fatals++;
                break;
            default:
                break;
        }
        
        if (error->recovered) {
            stats->recovered++;
        }
        
        stats->by_type[error->type]++;
    }
}

void error_print_stats(ErrorStats *stats, FILE *output) {
    if (!stats || !output) return;
    
    fprintf(output, "=== Error Statistics ===\n");
    fprintf(output, "Total errors: %zu\n", stats->total_errors);
    fprintf(output, "Warnings: %zu\n", stats->warnings);
    fprintf(output, "Errors: %zu\n", stats->errors);
    fprintf(output, "Fatals: %zu\n", stats->fatals);
    fprintf(output, "Recovered: %zu\n", stats->recovered);
    
    const char *type_names[] = {"Lexical", "Syntax", "Type", "Semantic", 
                                "CodeGen", "Optimization", "StdLib"};
    fprintf(output, "By type:\n");
    for (int i = 0; i < 7; i++) {
        if (stats->by_type[i] > 0) {
            fprintf(output, "  %s: %zu\n", type_names[i], stats->by_type[i]);
        }
    }
    fprintf(output, "=====================\n");
} 