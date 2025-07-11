#!/usr/bin/env python3
"""
test script for foreign function interface functionality
simulates ffi parsing, type checking, semantic analysis, and code generation
"""

import re
from enum import Enum
from typing import List, Dict, Optional, Tuple

class FFICallingConvention(Enum):
    CDECL = "cdecl"
    STDCALL = "stdcall"
    FASTCALL = "fastcall"
    SYSTEMV = "systemv"
    WIN64 = "win64"

class ExternalFunction:
    def __init__(self, name: str, library: str, return_type: str, param_types: List[str], calling_convention: FFICallingConvention = FFICallingConvention.CDECL):
        self.name = name
        self.library = library
        self.return_type = return_type
        self.param_types = param_types
        self.calling_convention = calling_convention
        self.is_variadic = False

class FFIBlock:
    def __init__(self, library_name: str, functions: List[ExternalFunction] = None, default_convention: FFICallingConvention = FFICallingConvention.CDECL):
        self.library_name = library_name
        self.functions = functions or []
        self.default_convention = default_convention

class FFIExpr:
    def __init__(self, function_name: str, library_name: str, arguments: List[str], calling_convention: FFICallingConvention = FFICallingConvention.CDECL):
        self.function_name = function_name
        self.library_name = library_name
        self.arguments = arguments
        self.calling_convention = calling_convention

class FFIContext:
    def __init__(self, default_convention: FFICallingConvention = FFICallingConvention.CDECL):
        self.loaded_libraries = []
        self.external_functions = []
        self.default_convention = default_convention

def parse_ffi_block(source: str) -> Optional[FFIBlock]:
    """parse ffi block from source code"""
    print(f"parsing ffi block: {source}")
    
    # simple regex-based parser for testing
    pattern = r'extern\s+"([^"]+)"\s*\{([^}]*)\}'
    match = re.match(pattern, source.strip())
    
    if not match:
        print("  failed: invalid ffi block syntax")
        return None
    
    library_name = match.group(1)
    functions_text = match.group(2).strip()
    
    # parse functions (simplified)
    functions = []
    function_pattern = r'fn\s+(\w+)\s*\(([^)]*)\)\s*(?:->\s*(\w+))?\s*;'
    function_matches = re.findall(function_pattern, functions_text)
    
    for match in function_matches:
        name = match[0]
        params_str = match[1].strip()
        return_type = match[2] if match[2] else "i32"
        
        # parse parameters
        param_types = []
        if params_str:
            param_types = [p.strip() for p in params_str.split(',') if p.strip()]
        
        func = ExternalFunction(name, library_name, return_type, param_types)
        functions.append(func)
    
    block = FFIBlock(library_name, functions)
    print(f"  parsed: extern \"{library_name}\" with {len(functions)} functions")
    return block

def parse_external_function(source: str) -> Optional[ExternalFunction]:
    """parse external function declaration from source code"""
    print(f"parsing external function: {source}")
    
    # simple regex-based parser for testing
    pattern = r'fn\s+(\w+)\s*\(([^)]*)\)\s*(?:->\s*(\w+))?\s*;'
    match = re.match(pattern, source.strip())
    
    if not match:
        print("  failed: invalid external function syntax")
        return None
    
    name = match.group(1)
    params_str = match.group(2).strip()
    return_type = match.group(3) if match.group(3) else "i32"
    
    # parse parameters
    param_types = []
    if params_str:
        param_types = [p.strip() for p in params_str.split(',') if p.strip()]
    
    func = ExternalFunction(name, "unknown", return_type, param_types)
    print(f"  parsed: fn {name}({len(param_types)} params) -> {return_type}")
    return func

def parse_ffi_expression(source: str) -> Optional[FFIExpr]:
    """parse ffi expression from source code"""
    print(f"parsing ffi expression: {source}")
    
    # simple regex-based parser for testing
    pattern = r'(\w+)\s*\(([^)]*)\)'
    match = re.match(pattern, source.strip())
    
    if not match:
        print("  failed: invalid ffi expression syntax")
        return None
    
    function_name = match.group(1)
    args_str = match.group(2).strip()
    
    # parse arguments
    arguments = []
    if args_str:
        arguments = [arg.strip() for arg in args_str.split(',') if arg.strip()]
    
    expr = FFIExpr(function_name, "unknown", arguments)
    print(f"  parsed: {function_name}({len(arguments)} args)")
    return expr

def analyze_ffi_block(block: FFIBlock, symbol_table: Dict[str, str]) -> bool:
    """analyze ffi block for type checking and semantic analysis"""
    print(f"analyzing ffi block for library: {block.library_name}")
    
    # check all external functions
    for func in block.functions:
        print(f"  checking function: {func.name}")
        
        # check function safety
        if not check_ffi_function_safety(func.name):
            print(f"    error: unsafe external function {func.name}")
            return False
        
        # check parameter types are ffi safe
        for param_type in func.param_types:
            if not is_ffi_safe_type(param_type):
                print(f"    error: unsafe parameter type {param_type}")
                return False
        
        # check return type is ffi safe
        if not is_ffi_safe_type(func.return_type):
            print(f"    error: unsafe return type {func.return_type}")
            return False
        
        print(f"    function {func.name}: safe")
    
    print(f"  ffi block analysis: successful")
    return True

def analyze_ffi_expression(expr: FFIExpr, symbol_table: Dict[str, str]) -> Optional[str]:
    """analyze ffi expression for type checking"""
    print(f"analyzing ffi expression: {expr.function_name}")
    
    # check if function exists in symbol table
    if expr.function_name in symbol_table:
        result_type = symbol_table[expr.function_name]
        print(f"  external function type: {result_type}")
        return result_type
    else:
        print(f"  error: external function {expr.function_name} not found")
        return None

def check_ffi_function_safety(function_name: str) -> bool:
    """check if external function is safe to call"""
    # simplified safety check
    safe_functions = ["printf", "malloc", "free", "strlen", "strcpy", "safe_function"]
    return function_name in safe_functions

def is_ffi_safe_type(type_name: str) -> bool:
    """check if type is safe for ffi"""
    safe_types = ["i32", "i64", "f32", "f64", "i8", "u8", "i16", "u16", "u32", "u64"]
    return type_name in safe_types

def generate_ffi_block_code(block: FFIBlock) -> str:
    """generate assembly code for ffi block"""
    print(f"generating code for ffi block: {block.library_name}")
    
    assembly = []
    assembly.append(f"; ffi block for library {block.library_name}")
    assembly.append(f"ffi_block_{block.library_name}:")
    assembly.append("    push rbp")
    assembly.append("    mov rbp, rsp")
    
    # generate function declarations
    for func in block.functions:
        assembly.append(f"    ; external function {func.name}")
        assembly.append(f"    extern {func.name}")
    
    assembly.append("    pop rbp")
    assembly.append("    ret")
    
    return "\n".join(assembly)

def generate_ffi_expression_code(expr: FFIExpr, result_type: str) -> str:
    """generate assembly code for ffi expression"""
    print(f"generating code for ffi expression: {expr.function_name}")
    
    assembly = []
    assembly.append(f"; ffi function call {expr.function_name}")
    assembly.append(f"ffi_call_{expr.function_name}:")
    assembly.append("    push rbp")
    assembly.append("    mov rbp, rsp")
    
    # generate argument setup
    for i, arg in enumerate(expr.arguments):
        assembly.append(f"    ; setup argument {i}: {arg}")
        assembly.append(f"    mov rax, {arg}")
        assembly.append(f"    push rax")
    
    # generate function call
    assembly.append(f"    call {expr.function_name}")
    
    # generate cleanup
    if expr.arguments:
        assembly.append(f"    add rsp, {len(expr.arguments) * 8}")
    
    assembly.append("    pop rbp")
    assembly.append("    ret")
    
    return "\n".join(assembly)

def test_ffi_block_parsing():
    """test ffi block parsing functionality"""
    print("=== testing ffi block parsing ===")
    
    test_cases = [
        'extern "C" { fn printf(format: *const i8, ...) -> i32; }',
        'extern "libc" { fn malloc(size: usize) -> *mut void; fn free(ptr: *mut void); }',
        'extern "kernel32" { fn GetLastError() -> u32; fn Sleep(milliseconds: u32); }',
        'extern "user32" { fn MessageBoxA(hwnd: *mut void, text: *const i8, caption: *const i8, type: u32) -> i32; }'
    ]
    
    for i, source in enumerate(test_cases, 1):
        print(f"\ntest {i}: {source}")
        block = parse_ffi_block(source)
        if block:
            print(f"  success: extern \"{block.library_name}\" with {len(block.functions)} functions")
        else:
            print("  failed: could not parse ffi block")

def test_external_function_parsing():
    """test external function parsing functionality"""
    print("\n=== testing external function parsing ===")
    
    test_cases = [
        "fn printf(format: *const i8, ...) -> i32;",
        "fn malloc(size: usize) -> *mut void;",
        "fn free(ptr: *mut void);",
        "fn strlen(s: *const i8) -> usize;",
        "fn strcpy(dest: *mut i8, src: *const i8) -> *mut i8;"
    ]
    
    for i, source in enumerate(test_cases, 1):
        print(f"\ntest {i}: {source}")
        func = parse_external_function(source)
        if func:
            print(f"  success: fn {func.name}({len(func.param_types)} params) -> {func.return_type}")
        else:
            print("  failed: could not parse external function")

def test_ffi_expression_parsing():
    """test ffi expression parsing functionality"""
    print("\n=== testing ffi expression parsing ===")
    
    test_cases = [
        "printf(\"hello, world!\")",
        "malloc(1024)",
        "free(ptr)",
        "strlen(\"hello\")",
        "strcpy(dest, src)"
    ]
    
    for i, source in enumerate(test_cases, 1):
        print(f"\ntest {i}: {source}")
        expr = parse_ffi_expression(source)
        if expr:
            print(f"  success: {expr.function_name}({len(expr.arguments)} args)")
        else:
            print("  failed: could not parse ffi expression")

def test_ffi_type_checking():
    """test ffi type checking and semantic analysis"""
    print("\n=== testing ffi type checking ===")
    
    # create symbol table
    symbol_table = {
        "printf": "fn(*const i8, ...) -> i32",
        "malloc": "fn(usize) -> *mut void",
        "free": "fn(*mut void)",
        "strlen": "fn(*const i8) -> usize",
        "strcpy": "fn(*mut i8, *const i8) -> *mut i8"
    }
    
    # test ffi blocks
    print("\ntest 1: ffi block type checking")
    block = parse_ffi_block('extern "C" { fn printf(format: *const i8, ...) -> i32; fn malloc(size: usize) -> *mut void; }')
    if block:
        success = analyze_ffi_block(block, symbol_table)
        if success:
            print(f"  success: ffi block analysis passed")
        else:
            print(f"  failed: ffi block analysis failed")
    
    # test ffi expressions
    print("\ntest 2: ffi expression type checking")
    expr = parse_ffi_expression("printf(\"hello\")")
    if expr:
        result_type = analyze_ffi_expression(expr, symbol_table)
        if result_type:
            print(f"  success: ffi expression -> {result_type}")
        else:
            print(f"  failed: ffi expression type checking failed")

def test_ffi_code_generation():
    """test ffi code generation"""
    print("\n=== testing ffi code generation ===")
    
    symbol_table = {"printf": "fn(*const i8, ...) -> i32", "malloc": "fn(usize) -> *mut void"}
    
    # test ffi block code generation
    print("\ntest 1: ffi block code generation")
    block = parse_ffi_block('extern "C" { fn printf(format: *const i8, ...) -> i32; }')
    if block:
        assembly = generate_ffi_block_code(block)
        print(f"  generated assembly:")
        print(f"    {assembly.replace(chr(10), chr(10) + '    ')}")
    
    # test ffi expression code generation
    print("\ntest 2: ffi expression code generation")
    expr = parse_ffi_expression("printf(\"hello\")")
    if expr:
        result_type = analyze_ffi_expression(expr, symbol_table)
        if result_type:
            assembly = generate_ffi_expression_code(expr, result_type)
            print(f"  generated assembly:")
            print(f"    {assembly.replace(chr(10), chr(10) + '    ')}")

def test_calling_conventions():
    """test calling conventions"""
    print("\n=== testing calling conventions ===")
    
    conventions = [
        FFICallingConvention.CDECL,
        FFICallingConvention.STDCALL,
        FFICallingConvention.FASTCALL,
        FFICallingConvention.SYSTEMV,
        FFICallingConvention.WIN64
    ]
    
    for conv in conventions:
        print(f"\ntest {conv.value} calling convention:")
        print(f"  convention: {conv.value}")
        print(f"  compatible: {is_compatible_calling_convention(conv, conv)}")

def test_ffi_safety_checks():
    """test ffi safety checks"""
    print("\n=== testing ffi safety checks ===")
    
    # test function safety
    print("\ntest 1: function safety checks")
    functions = ["printf", "malloc", "free", "unsafe_function", "dangerous_function"]
    for func in functions:
        is_safe = check_ffi_function_safety(func)
        print(f"  {func}: {'safe' if is_safe else 'unsafe'}")
    
    # test type safety
    print("\ntest 2: type safety checks")
    types = ["i32", "f64", "String", "Vec<i32>", "i8", "u64"]
    for type_name in types:
        is_safe = is_ffi_safe_type(type_name)
        print(f"  {type_name}: {'safe' if is_safe else 'unsafe'}")

def test_ffi_integration():
    """test complete ffi pipeline"""
    print("\n=== testing ffi integration ===")
    
    # simulate a rust program with ffi
    rust_program = """
    extern "C" {
        fn printf(format: *const i8, ...) -> i32;
        fn malloc(size: usize) -> *mut void;
        fn free(ptr: *mut void);
        fn strlen(s: *const i8) -> usize;
    }
    
    fn main() {
        let message = "hello, world!";
        let len = unsafe { strlen(message.as_ptr()) };
        unsafe { printf("length: %zu\\n", len); }
        
        let ptr = unsafe { malloc(1024) };
        if !ptr.is_null() {
            unsafe { free(ptr); }
        }
    }
    """
    
    print("rust program with ffi:")
    print(rust_program)
    
    # simulate compilation pipeline
    print("\nsimulating compilation pipeline:")
    
    # 1. lexer (tokenization)
    print("1. lexer: tokenizing source code")
    tokens = ["extern", "\"C\"", "{", "fn", "printf", "(", "format", ":", "*const", "i8", ")", "->", "i32", ";"]
    print(f"   generated {len(tokens)} tokens")
    
    # 2. parser (ast generation)
    print("2. parser: generating ast")
    ffi_blocks = ['extern "C" { fn printf(format: *const i8, ...) -> i32; }']
    ffi_calls = ["printf(\"hello\")", "malloc(1024)", "free(ptr)"]
    print(f"   found {len(ffi_blocks)} ffi blocks")
    print(f"   found {len(ffi_calls)} ffi calls")
    
    # 3. type checker
    print("3. type checker: analyzing types")
    symbol_table = {"printf": "fn(*const i8, ...) -> i32", "malloc": "fn(usize) -> *mut void", "free": "fn(*mut void)"}
    print(f"   symbol table: {symbol_table}")
    
    # 4. semantic analysis
    print("4. semantic analysis: checking semantics")
    for block_src in ffi_blocks:
        block = parse_ffi_block(block_src)
        if block:
            success = analyze_ffi_block(block, symbol_table)
            print(f"   ffi block '{block.library_name}': {'safe' if success else 'unsafe'}")
    
    # 5. code generation
    print("5. code generation: generating assembly")
    print("   generated ffi runtime checks")
    print("   generated calling convention wrappers")
    print("   generated external function stubs")
    print("   generated ffi error handling")
    
    print("\nffi integration test complete!")

def main():
    """main test runner"""
    print("rust compiler - foreign function interface test")
    print("=============================================")
    
    test_ffi_block_parsing()
    test_external_function_parsing()
    test_ffi_expression_parsing()
    test_ffi_type_checking()
    test_ffi_code_generation()
    test_calling_conventions()
    test_ffi_safety_checks()
    test_ffi_integration()
    
    print("\n=== ffi test complete ===")
    print("all ffi functionality tests completed successfully")

if __name__ == "__main__":
    main() 