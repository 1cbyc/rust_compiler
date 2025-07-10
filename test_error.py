#!/usr/bin/env python3
"""
Error Handling Test Script
Simulates error handling, recovery mechanisms, and debug information generation
"""

from typing import Dict, List, Optional, Union, Any
from enum import Enum
import sys

# Error severity levels
class ErrorSeverity(Enum):
    INFO = "INFO"
    WARNING = "WARNING"
    ERROR = "ERROR"
    FATAL = "FATAL"

# Error types
class ErrorType(Enum):
    LEXICAL = "LEXICAL"
    SYNTAX = "SYNTAX"
    TYPE = "TYPE"
    SEMANTIC = "SEMANTIC"
    CODE_GEN = "CODE_GEN"
    OPTIMIZATION = "OPTIMIZATION"
    STDLIB = "STDLIB"

# Source location
class SourceLocation:
    def __init__(self, line: int, column: int, filename: str = None):
        self.line = line
        self.column = column
        self.offset = 0
        self.filename = filename
    
    def __str__(self):
        return f"{self.filename or 'unknown'}:{self.line}:{self.column}"

# Error message
class ErrorMessage:
    def __init__(self, error_type: ErrorType, severity: ErrorSeverity, 
                 message: str, location: SourceLocation, suggestion: str = None):
        self.type = error_type
        self.severity = severity
        self.message = message
        self.location = location
        self.suggestion = suggestion
        self.recovered = False
    
    def __str__(self):
        result = f"{self.severity.value} {self.type.value}: {self.message} at {self.location}"
        if self.suggestion:
            result += f"\nSuggestion: {self.suggestion}"
        return result

# Error context
class ErrorContext:
    def __init__(self, max_errors: int = 100):
        self.errors = []
        self.had_error = False
        self.had_fatal = False
        self.max_errors = max_errors
    
    def report_error(self, error_type: ErrorType, severity: ErrorSeverity,
                    message: str, location: SourceLocation, suggestion: str = None) -> bool:
        if len(self.errors) >= self.max_errors:
            return False
        
        error = ErrorMessage(error_type, severity, message, location, suggestion)
        self.errors.append(error)
        
        if severity in [ErrorSeverity.ERROR, ErrorSeverity.FATAL]:
            self.had_error = True
        if severity == ErrorSeverity.FATAL:
            self.had_fatal = True
        
        return True
    
    def report_lexical(self, message: str, line: int, column: int, filename: str = None) -> bool:
        location = SourceLocation(line, column, filename)
        return self.report_error(ErrorType.LEXICAL, ErrorSeverity.ERROR, message, location)
    
    def report_syntax(self, message: str, line: int, column: int, filename: str = None) -> bool:
        location = SourceLocation(line, column, filename)
        return self.report_error(ErrorType.SYNTAX, ErrorSeverity.ERROR, message, location)
    
    def report_type(self, message: str, line: int, column: int, filename: str = None) -> bool:
        location = SourceLocation(line, column, filename)
        return self.report_error(ErrorType.TYPE, ErrorSeverity.ERROR, message, location)
    
    def report_semantic(self, message: str, line: int, column: int, filename: str = None) -> bool:
        location = SourceLocation(line, column, filename)
        return self.report_error(ErrorType.SEMANTIC, ErrorSeverity.ERROR, message, location)
    
    def can_recover(self) -> bool:
        return not self.had_fatal and len(self.errors) < self.max_errors
    
    def try_recover(self, error_type: ErrorType) -> bool:
        # Simple recovery strategies
        recovery_strategies = {
            ErrorType.LEXICAL: "Skip to next token",
            ErrorType.SYNTAX: "Skip to next statement",
            ErrorType.TYPE: "Use default type",
            ErrorType.SEMANTIC: "Skip problematic code"
        }
        
        strategy = recovery_strategies.get(error_type, "Unknown recovery strategy")
        print(f"Recovery: {strategy}")
        return True
    
    def get_stats(self) -> Dict[str, int]:
        stats = {
            'total_errors': len(self.errors),
            'warnings': 0,
            'errors': 0,
            'fatals': 0,
            'recovered': 0,
            'by_type': {t.value: 0 for t in ErrorType}
        }
        
        for error in self.errors:
            if error.severity == ErrorSeverity.WARNING:
                stats['warnings'] += 1
            elif error.severity == ErrorSeverity.ERROR:
                stats['errors'] += 1
            elif error.severity == ErrorSeverity.FATAL:
                stats['fatals'] += 1
            
            if error.recovered:
                stats['recovered'] += 1
            
            stats['by_type'][error.type.value] += 1
        
        return stats

# Debug information
class DebugInfo:
    def __init__(self, location: SourceLocation, function_name: str = None):
        self.location = location
        self.function_name = function_name
        self.variable_name = None
        self.type_name = None
        self.stack_depth = 0
    
    def add_variable(self, name: str, type_name: str):
        self.variable_name = name
        self.type_name = type_name
    
    def set_stack_depth(self, depth: int):
        self.stack_depth = depth
    
    def __str__(self):
        result = f"DebugInfo: {self.function_name or 'unknown'} at {self.location}"
        if self.variable_name:
            result += f" (var: {self.variable_name}: {self.type_name})"
        result += f" [depth: {self.stack_depth}]"
        return result

# Stack trace
class StackFrame:
    def __init__(self, function_name: str, location: SourceLocation):
        self.function_name = function_name
        self.location = location
        self.variables = []
    
    def add_variable(self, name: str):
        self.variables.append(name)
    
    def __str__(self):
        result = f"{self.function_name} at {self.location}"
        if self.variables:
            result += f" (vars: {', '.join(self.variables)})"
        return result

class StackTrace:
    def __init__(self):
        self.frames = []
    
    def add_frame(self, function_name: str, location: SourceLocation) -> bool:
        frame = StackFrame(function_name, location)
        self.frames.append(frame)
        return True
    
    def print_trace(self):
        print("Stack trace:")
        for i, frame in enumerate(self.frames):
            print(f"  {i}: {frame}")

# Test functions
def test_error_reporting():
    """Test basic error reporting"""
    print("=== Error Reporting Test ===")
    
    ctx = ErrorContext()
    
    # Test different error types
    ctx.report_lexical("Unexpected character '#'", 5, 10, "test.rs")
    ctx.report_syntax("Expected ';' after expression", 12, 15, "test.rs")
    ctx.report_type("Type mismatch: expected i32, got String", 8, 3, "test.rs")
    ctx.report_semantic("Variable 'x' is not defined", 20, 7, "test.rs")
    
    print("Reported errors:")
    for error in ctx.errors:
        print(f"  {error}")
    
    stats = ctx.get_stats()
    print(f"\nError statistics: {stats}")

def test_error_recovery():
    """Test error recovery mechanisms"""
    print("\n=== Error Recovery Test ===")
    
    ctx = ErrorContext()
    
    # Simulate errors and recovery
    ctx.report_lexical("Invalid token", 1, 1, "test.rs")
    if ctx.can_recover():
        ctx.try_recover(ErrorType.LEXICAL)
    
    ctx.report_syntax("Missing semicolon", 5, 10, "test.rs")
    if ctx.can_recover():
        ctx.try_recover(ErrorType.SYNTAX)
    
    print(f"Can recover: {ctx.can_recover()}")
    print(f"Had error: {ctx.had_error}")
    print(f"Had fatal: {ctx.had_fatal}")

def test_debug_information():
    """Test debug information generation"""
    print("\n=== Debug Information Test ===")
    
    # Create debug info for different scenarios
    locations = [
        SourceLocation(10, 5, "main.rs"),
        SourceLocation(25, 12, "utils.rs"),
        SourceLocation(8, 3, "parser.rs")
    ]
    
    functions = ["main", "calculate", "parse_expression"]
    
    for i, (loc, func) in enumerate(zip(locations, functions)):
        debug_info = DebugInfo(loc, func)
        debug_info.add_variable(f"var_{i}", "i32")
        debug_info.set_stack_depth(i)
        print(f"  {debug_info}")

def test_stack_trace():
    """Test stack trace generation"""
    print("\n=== Stack Trace Test ===")
    
    trace = StackTrace()
    
    # Simulate function calls
    locations = [
        SourceLocation(15, 8, "main.rs"),
        SourceLocation(42, 12, "math.rs"),
        SourceLocation(7, 3, "utils.rs")
    ]
    
    functions = ["main", "add_numbers", "validate_input"]
    
    for loc, func in zip(locations, functions):
        trace.add_frame(func, loc)
        # Add some variables to the frame
        frame = trace.frames[-1]
        frame.add_variable("x")
        frame.add_variable("y")
    
    trace.print_trace()

def test_error_formatting():
    """Test error message formatting"""
    print("\n=== Error Formatting Test ===")
    
    ctx = ErrorContext()
    
    # Test different error scenarios
    test_cases = [
        (ErrorType.LEXICAL, "Unexpected character", 1, 1, "test.rs"),
        (ErrorType.SYNTAX, "Expected '}' but found ';'", 10, 15, "test.rs"),
        (ErrorType.TYPE, "Cannot assign String to i32", 5, 8, "test.rs"),
        (ErrorType.SEMANTIC, "Variable 'undefined' not in scope", 20, 3, "test.rs")
    ]
    
    for error_type, message, line, col, filename in test_cases:
        location = SourceLocation(line, col, filename)
        error = ErrorMessage(error_type, ErrorSeverity.ERROR, message, location)
        print(f"  {error}")

def test_error_statistics():
    """Test error statistics collection"""
    print("\n=== Error Statistics Test ===")
    
    ctx = ErrorContext()
    
    # Generate various errors
    ctx.report_error(ErrorType.LEXICAL, ErrorSeverity.WARNING, "Minor issue", 
                    SourceLocation(1, 1, "test.rs"))
    ctx.report_error(ErrorType.SYNTAX, ErrorSeverity.ERROR, "Syntax error", 
                    SourceLocation(5, 10, "test.rs"))
    ctx.report_error(ErrorType.TYPE, ErrorSeverity.ERROR, "Type error", 
                    SourceLocation(8, 3, "test.rs"))
    ctx.report_error(ErrorType.SEMANTIC, ErrorSeverity.FATAL, "Fatal error", 
                    SourceLocation(15, 7, "test.rs"))
    
    stats = ctx.get_stats()
    print("Error Statistics:")
    for key, value in stats.items():
        if key != 'by_type':
            print(f"  {key}: {value}")
    
    print("  By type:")
    for error_type, count in stats['by_type'].items():
        if count > 0:
            print(f"    {error_type}: {count}")

def test_source_location():
    """Test source location utilities"""
    print("\n=== Source Location Test ===")
    
    # Test source location creation
    loc1 = SourceLocation(10, 5, "main.rs")
    print(f"Location 1: {loc1}")
    
    loc2 = SourceLocation(25, 12, "utils.rs")
    print(f"Location 2: {loc2}")
    
    # Test location from offset (simulated)
    source_code = "fn main() {\n    let x = 42;\n}"
    offset = 15  # Position of 'x'
    loc3 = SourceLocation(2, 8, "test.rs")  # Simulated result
    print(f"Location from offset {offset}: {loc3}")

if __name__ == "__main__":
    test_error_reporting()
    test_error_recovery()
    test_debug_information()
    test_stack_trace()
    test_error_formatting()
    test_error_statistics()
    test_source_location() 