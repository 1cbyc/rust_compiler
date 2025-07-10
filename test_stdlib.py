#!/usr/bin/env python3
"""
Standard Library Test Script
Simulates standard library functionality including I/O, collections, and macros
"""

import re
from typing import Dict, List, Optional, Union, Any

# Standard library function types
class StdLibFunction:
    def __init__(self, name: str, param_types: List[str], return_type: str, impl=None):
        self.name = name
        self.param_types = param_types
        self.return_type = return_type
        self.impl = impl
    
    def call(self, *args):
        if self.impl:
            return self.impl(*args)
        return f"{self.name}({', '.join(map(str, args))})"

class StdLibContext:
    def __init__(self):
        self.functions = {}
        self.init_stdlib()
    
    def init_stdlib(self):
        """Initialize standard library functions"""
        # Print functions
        self.register_function("print", ["&str"], "()", lambda s: f"print({s})")
        self.register_function("println", ["&str"], "()", lambda s: f"println({s})")
        
        # String functions
        self.register_function("len", ["&str"], "usize", lambda s: f"len({s})")
        self.register_function("concat", ["&str", "&str"], "String", lambda a, b: f"concat({a}, {b})")
        
        # Collection functions
        self.register_function("Vec::new", [], "Vec<T>", lambda: "Vec::new()")
        self.register_function("push", ["&mut Vec<T>", "T"], "()", lambda vec, item: f"push({vec}, {item})")
        self.register_function("get", ["&Vec<T>", "usize"], "Option<T>", lambda vec, idx: f"get({vec}, {idx})")
        
        # Error handling
        self.register_function("Result::Ok", ["T"], "Result<T, E>", lambda val: f"Result::Ok({val})")
        self.register_function("Result::Err", ["E"], "Result<T, E>", lambda err: f"Result::Err({err})")
    
    def register_function(self, name: str, param_types: List[str], return_type: str, impl=None):
        self.functions[name] = StdLibFunction(name, param_types, return_type, impl)
    
    def lookup_function(self, name: str) -> Optional[StdLibFunction]:
        return self.functions.get(name)
    
    def call_function(self, name: str, *args):
        func = self.lookup_function(name)
        if func:
            return func.call(*args)
        return f"undefined_function({name})"

# Macro system
class Macro:
    def __init__(self, name: str, pattern: str, replacement: str):
        self.name = name
        self.pattern = pattern
        self.replacement = replacement

class MacroContext:
    def __init__(self):
        self.macros = {}
        self.init_basic_macros()
    
    def init_basic_macros(self):
        """Initialize basic macros"""
        self.register_macro("println!", "println!({})", "println({})")
        self.register_macro("print!", "print!({})", "print({})")
        self.register_macro("vec!", "vec![{}]", "Vec::new()")
    
    def register_macro(self, name: str, pattern: str, replacement: str):
        self.macros[name] = Macro(name, pattern, replacement)
    
    def expand_macro(self, input_text: str) -> str:
        """Expand macros in input text"""
        result = input_text
        
        for macro in self.macros.values():
            # Simple pattern matching and replacement
            if macro.pattern in result:
                result = result.replace(macro.pattern, macro.replacement)
        
        return result

# Test functions
def test_stdlib_functions():
    """Test standard library functions"""
    stdlib = StdLibContext()
    
    print("=== Standard Library Function Test ===")
    
    # Test print functions
    print(f"print!(\"hello\"): {stdlib.call_function('print', '\"hello\"')}")
    print(f"println!(\"world\"): {stdlib.call_function('println', '\"world\"')}")
    
    # Test string functions
    print(f"len(\"test\"): {stdlib.call_function('len', '\"test\"')}")
    print(f"concat(\"a\", \"b\"): {stdlib.call_function('concat', '\"a\"', '\"b\"')}")
    
    # Test collection functions
    print(f"Vec::new(): {stdlib.call_function('Vec::new')}")
    print(f"push(vec, 42): {stdlib.call_function('push', 'vec', '42')}")
    print(f"get(vec, 0): {stdlib.call_function('get', 'vec', '0')}")
    
    # Test error handling
    print(f"Result::Ok(42): {stdlib.call_function('Result::Ok', '42')}")
    print(f"Result::Err(\"error\"): {stdlib.call_function('Result::Err', '\"error\"')}")
    
    print("=== Standard Library Function Test Complete ===")

def test_macro_system():
    """Test macro system"""
    macro_ctx = MacroContext()
    
    print("\n=== Macro System Test ===")
    
    # Test macro expansion
    test_cases = [
        "println!(\"hello\")",
        "print!(\"world\")",
        "vec![1, 2, 3]",
        "let x = 42;"
    ]
    
    for test_case in test_cases:
        expanded = macro_ctx.expand_macro(test_case)
        print(f"'{test_case}' -> '{expanded}'")
    
    print("=== Macro System Test Complete ===")

def test_stdlib_integration():
    """Test stdlib integration with sample Rust code"""
    stdlib = StdLibContext()
    macro_ctx = MacroContext()
    
    print("\n=== Standard Library Integration Test ===")
    
    # Sample Rust code with stdlib usage
    rust_code = """
    fn main() {
        println!("Hello, world!");
        let vec = vec![1, 2, 3];
        let len = len("test");
        let result = Result::Ok(42);
    }
    """
    
    print("Original Rust code:")
    print(rust_code)
    
    # Expand macros
    expanded_code = macro_ctx.expand_macro(rust_code)
    print("\nAfter macro expansion:")
    print(expanded_code)
    
    # Simulate function calls
    print("\nSimulated function calls:")
    print(f"  {stdlib.call_function('println', '\"Hello, world!\"')}")
    print(f"  {stdlib.call_function('Vec::new')}")
    print(f"  {stdlib.call_function('len', '\"test\"')}")
    print(f"  {stdlib.call_function('Result::Ok', '42')}")
    
    print("=== Standard Library Integration Test Complete ===")

if __name__ == "__main__":
    test_stdlib_functions()
    test_macro_system()
    test_stdlib_integration() 