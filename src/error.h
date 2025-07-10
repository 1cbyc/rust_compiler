#ifndef ERROR_H
#define ERROR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

// Error severity levels
typedef enum ErrorSeverity {
    ERROR_INFO,
    ERROR_WARNING,
    ERROR_ERROR,
    ERROR_FATAL
} ErrorSeverity;

// Error types
typedef enum ErrorType {
    ERROR_LEXICAL,
    ERROR_SYNTAX,
    ERROR_TYPE,
    ERROR_SEMANTIC,
    ERROR_CODE_GEN,
    ERROR_OPTIMIZATION,
    ERROR_STDLIB
} ErrorType;

// Source location information
typedef struct SourceLocation {
    size_t line;
    size_t column;
    size_t offset;
    char *filename;
} SourceLocation;

// Error message structure
typedef struct ErrorMessage {
    ErrorType type;
    ErrorSeverity severity;
    char *message;
    SourceLocation location;
    char *suggestion;
    bool recovered;
} ErrorMessage;

// Error context for recovery
typedef struct ErrorContext {
    ErrorMessage **errors;
    size_t error_count;
    size_t error_capacity;
    bool had_error;
    bool had_fatal;
    size_t max_errors;
} ErrorContext;

// Debug information
typedef struct DebugInfo {
    SourceLocation location;
    char *function_name;
    char *variable_name;
    char *type_name;
    size_t stack_depth;
} DebugInfo;

// Error handling functions
ErrorContext* error_context_create(size_t max_errors);
void error_context_free(ErrorContext *ctx);

// Error reporting
bool error_report(ErrorContext *ctx, ErrorType type, ErrorSeverity severity,
                 const char *message, SourceLocation location, const char *suggestion);
bool error_report_lexical(ErrorContext *ctx, const char *message, 
                         size_t line, size_t column, const char *filename);
bool error_report_syntax(ErrorContext *ctx, const char *message,
                        size_t line, size_t column, const char *filename);
bool error_report_type(ErrorContext *ctx, const char *message,
                      size_t line, size_t column, const char *filename);
bool error_report_semantic(ErrorContext *ctx, const char *message,
                          size_t line, size_t column, const char *filename);

// Error recovery
bool error_can_recover(ErrorContext *ctx);
bool error_try_recover(ErrorContext *ctx, ErrorType type);
void error_clear_recovered(ErrorContext *ctx);

// Debug information functions
DebugInfo* debug_info_create(SourceLocation location, const char *function_name);
void debug_info_free(DebugInfo *info);
void debug_info_add_variable(DebugInfo *info, const char *name, const char *type);
void debug_info_set_stack_depth(DebugInfo *info, size_t depth);

// Stack trace functions
typedef struct StackFrame {
    char *function_name;
    SourceLocation location;
    char **variables;
    size_t variable_count;
} StackFrame;

typedef struct StackTrace {
    StackFrame **frames;
    size_t frame_count;
    size_t frame_capacity;
} StackTrace;

StackTrace* stack_trace_create(void);
void stack_trace_free(StackTrace *trace);
bool stack_trace_add_frame(StackTrace *trace, const char *function_name, 
                          SourceLocation location);
void stack_trace_print(StackTrace *trace, FILE *output);

// Error formatting and output
void error_print_summary(ErrorContext *ctx, FILE *output);
void error_print_detailed(ErrorContext *ctx, FILE *output);
char* error_format_message(ErrorMessage *error);
void error_print_source_line(const char *source, SourceLocation location, FILE *output);

// Source location utilities
SourceLocation source_location_create(size_t line, size_t column, const char *filename);
SourceLocation source_location_from_offset(const char *source, size_t offset, const char *filename);
void source_location_update(SourceLocation *loc, char c);

// Error statistics
typedef struct ErrorStats {
    size_t total_errors;
    size_t warnings;
    size_t errors;
    size_t fatals;
    size_t recovered;
    size_t by_type[7]; // indexed by ErrorType
} ErrorStats;

void error_get_stats(ErrorContext *ctx, ErrorStats *stats);
void error_print_stats(ErrorStats *stats, FILE *output);

#endif // ERROR_H 