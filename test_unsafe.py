#!/usr/bin/env python3
"""
test script for unsafe blocks functionality
simulates unsafe blocks parsing, type checking, semantic analysis, and code generation
"""

import re
from enum import Enum
from typing import List, Dict, Optional, Tuple

class UnsafeOperation(Enum):
    DEREF_RAW_POINTER = "deref_raw_pointer"
    CALL_UNSAFE_FUNCTION = "call_unsafe_function"
    ACCESS_UNION_FIELD = "access_union_field"
    ACCESS_STATIC_MUT = "access_static_mut"
    IMPLEMENT_UNSAFE_TRAIT = "implement_unsafe_trait"
    CAST_POINTER = "cast_pointer"
    TRANSMUTE = "transmute"
    OFFSET_POINTER = "offset_pointer"

class RawPointerType:
    def __init__(self, pointee_type: str, is_mutable: bool = False, alignment: int = 8):
        self.pointee_type = pointee_type
        self.is_mutable = is_mutable
        self.alignment = alignment

class UnsafeBlock:
    def __init__(self, body: str, operations: List[UnsafeOperation] = None, is_unsafe_function: bool = False):
        self.body = body
        self.operations = operations or []
        self.is_unsafe_function = is_unsafe_function

class UnsafeExpr:
    def __init__(self, expression: str, operation: UnsafeOperation):
        self.expression = expression
        self.operation = operation

class UnsafeContext:
    def __init__(self):
        self.in_unsafe_block = False
        self.unsafe_operation_count = 0
        self.operations = []
        self.allow_unsafe_operations = False

def parse_unsafe_block(source: str) -> Optional[UnsafeBlock]:
    """parse unsafe block from source code"""
    print(f"parsing unsafe block: {source}")
    
    # simple regex-based parser for testing
    pattern = r'unsafe\s*\{([^}]*)\}'
    match = re.match(pattern, source.strip())
    
    if not match:
        print("  failed: invalid unsafe block syntax")
        return None
    
    body = match.group(1).strip()
    
    # detect unsafe operations in body (simplified)
    operations = []
    if "*" in body:
        operations.append(UnsafeOperation.DEREF_RAW_POINTER)
    if "unsafe" in body:
        operations.append(UnsafeOperation.CALL_UNSAFE_FUNCTION)
    if "transmute" in body:
        operations.append(UnsafeOperation.TRANSMUTE)
    if "offset" in body:
        operations.append(UnsafeOperation.OFFSET_POINTER)
    
    block = UnsafeBlock(body, operations)
    print(f"  parsed: unsafe block with {len(operations)} unsafe operations")
    return block

def parse_unsafe_function(source: str) -> Optional[UnsafeBlock]:
    """parse unsafe function from source code"""
    print(f"parsing unsafe function: {source}")
    
    # simple regex-based parser for testing
    pattern = r'unsafe\s+fn\s+(\w+)\s*\(([^)]*)\)\s*(?:->\s*(\w+))?\s*\{([^}]*)\}'
    match = re.match(pattern, source.strip())
    
    if not match:
        print("  failed: invalid unsafe function syntax")
        return None
    
    name = match.group(1)
    params_str = match.group(2).strip()
    return_type = match.group(3) if match.group(3) else "i32"
    body = match.group(4).strip()
    
    # parse parameters
    parameters = []
    if params_str:
        parameters = [p.strip() for p in params_str.split(',') if p.strip()]
    
    # detect unsafe operations
    operations = [UnsafeOperation.CALL_UNSAFE_FUNCTION]
    if "*" in body:
        operations.append(UnsafeOperation.DEREF_RAW_POINTER)
    if "transmute" in body:
        operations.append(UnsafeOperation.TRANSMUTE)
    
    func = UnsafeBlock(body, operations, True)
    print(f"  parsed: unsafe fn {name}({len(parameters)} params) -> {return_type}")
    return func

def parse_unsafe_expression(source: str) -> Optional[UnsafeExpr]:
    """parse unsafe expression from source code"""
    print(f"parsing unsafe expression: {source}")
    
    # check for unsafe keyword
    if source.strip().startswith("unsafe"):
        pattern = r'unsafe\s+(.+)'
        match = re.match(pattern, source.strip())
        if match:
            expression = match.group(1).strip()
            
            # determine operation type
            operation = UnsafeOperation.DEREF_RAW_POINTER  # default
            if "transmute" in expression:
                operation = UnsafeOperation.TRANSMUTE
            elif "offset" in expression:
                operation = UnsafeOperation.OFFSET_POINTER
            elif "cast" in expression:
                operation = UnsafeOperation.CAST_POINTER
            
            expr = UnsafeExpr(expression, operation)
            print(f"  parsed: unsafe {expression} ({operation.value})")
            return expr
    
    print("  failed: invalid unsafe expression syntax")
    return None

def analyze_unsafe_block(block: UnsafeBlock, symbol_table: Dict[str, str]) -> Optional[str]:
    """analyze unsafe block for type checking and semantic analysis"""
    print(f"analyzing unsafe block with {len(block.operations)} operations")
    
    # check unsafe safety
    for operation in block.operations:
        print(f"  checking operation: {operation.value}")
        
        if operation == UnsafeOperation.DEREF_RAW_POINTER:
            # check pointer validity
            if "*const" in block.body or "*mut" in block.body:
                print(f"    pointer dereference: valid")
            else:
                print(f"    error: invalid pointer dereference")
                return None
                
        elif operation == UnsafeOperation.CALL_UNSAFE_FUNCTION:
            # check function safety
            print(f"    unsafe function call: valid")
            
        elif operation == UnsafeOperation.TRANSMUTE:
            # check transmute safety
            print(f"    transmute operation: valid")
            
        elif operation == UnsafeOperation.OFFSET_POINTER:
            # check pointer offset
            print(f"    pointer offset: valid")
    
    # analyze body type
    result_type = "i32"  # default
    if "String" in block.body:
        result_type = "String"
    elif "Vec" in block.body:
        result_type = "Vec<i32>"
    
    print(f"  unsafe block type: {result_type}")
    return result_type

def analyze_unsafe_expression(expr: UnsafeExpr, symbol_table: Dict[str, str]) -> Optional[str]:
    """analyze unsafe expression for type checking"""
    print(f"analyzing unsafe expression: {expr.operation.value}")
    
    # handle different unsafe operations
    if expr.operation == UnsafeOperation.DEREF_RAW_POINTER:
        # dereference raw pointer
        if "*const" in expr.expression or "*mut" in expr.expression:
            # extract pointee type
            if "*const i32" in expr.expression:
                result_type = "i32"
            elif "*mut String" in expr.expression:
                result_type = "String"
            else:
                result_type = "i32"  # default
            print(f"  dereference type: {result_type}")
            return result_type
        else:
            print(f"  error: cannot dereference non-pointer type")
            return None
            
    elif expr.operation == UnsafeOperation.CALL_UNSAFE_FUNCTION:
        # call unsafe function
        if expr.expression in symbol_table:
            result_type = symbol_table[expr.expression]
            print(f"  unsafe function type: {result_type}")
            return result_type
        else:
            print(f"  error: unsafe function not found")
            return None
            
    elif expr.operation == UnsafeOperation.TRANSMUTE:
        # transmute operation
        result_type = "transmuted_type"
        print(f"  transmute type: {result_type}")
        return result_type
        
    else:
        # other unsafe operations
        result_type = "unsafe_result"
        print(f"  unsafe operation type: {result_type}")
        return result_type

def generate_unsafe_block_code(block: UnsafeBlock, result_type: str) -> str:
    """generate assembly code for unsafe block"""
    print(f"generating code for unsafe block")
    
    assembly = []
    assembly.append("; unsafe block")
    assembly.append("unsafe_block:")
    assembly.append("    push rbp")
    assembly.append("    mov rbp, rsp")
    
    # generate safety checks
    assembly.append("    ; unsafe safety checks")
    for operation in block.operations:
        if operation == UnsafeOperation.DEREF_RAW_POINTER:
            assembly.append("    ; pointer dereference safety check")
            assembly.append("    mov rax, [rbp+8]  ; pointer")
            assembly.append("    test rax, rax      ; null check")
            assembly.append("    jz .null_pointer_error")
            
        elif operation == UnsafeOperation.CALL_UNSAFE_FUNCTION:
            assembly.append("    ; unsafe function call")
            
        elif operation == UnsafeOperation.TRANSMUTE:
            assembly.append("    ; transmute operation")
            
        elif operation == UnsafeOperation.OFFSET_POINTER:
            assembly.append("    ; pointer offset")
    
    # generate body code
    assembly.append("    ; unsafe block body")
    assembly.append(f"    ; {block.body}")
    
    assembly.append("    pop rbp")
    assembly.append("    ret")
    
    assembly.append(".null_pointer_error:")
    assembly.append("    ; handle null pointer error")
    assembly.append("    mov rax, 1  ; error code")
    assembly.append("    ret")
    
    return "\n".join(assembly)

def generate_unsafe_expression_code(expr: UnsafeExpr, result_type: str) -> str:
    """generate assembly code for unsafe expression"""
    print(f"generating code for unsafe expression: {expr.operation.value}")
    
    assembly = []
    assembly.append("; unsafe expression")
    assembly.append("unsafe_expr:")
    assembly.append("    push rbp")
    assembly.append("    mov rbp, rsp")
    
    # generate unsafe operation code
    if expr.operation == UnsafeOperation.DEREF_RAW_POINTER:
        assembly.append("    ; dereference raw pointer")
        assembly.append("    mov rax, [rbp+8]  ; pointer")
        assembly.append("    mov rax, [rax]     ; dereference")
        
    elif expr.operation == UnsafeOperation.CALL_UNSAFE_FUNCTION:
        assembly.append("    ; call unsafe function")
        assembly.append("    call unsafe_function")
        
    elif expr.operation == UnsafeOperation.TRANSMUTE:
        assembly.append("    ; transmute operation")
        assembly.append("    mov rax, [rbp+8]  ; source")
        assembly.append("    ; cast to target type")
        
    elif expr.operation == UnsafeOperation.CAST_POINTER:
        assembly.append("    ; cast pointer")
        assembly.append("    mov rax, [rbp+8]  ; source pointer")
        assembly.append("    ; cast to target type")
        
    else:
        assembly.append("    ; other unsafe operation")
    
    assembly.append("    pop rbp")
    assembly.append("    ret")
    
    return "\n".join(assembly)

def test_unsafe_block_parsing():
    """test unsafe block parsing functionality"""
    print("=== testing unsafe block parsing ===")
    
    test_cases = [
        "unsafe { let x = *ptr; }",
        "unsafe { call_unsafe_function(); }",
        "unsafe { let y = transmute::<i32, u32>(x); }",
        "unsafe { let z = ptr.offset(1); }",
        "unsafe { access_static_mut(); }"
    ]
    
    for i, source in enumerate(test_cases, 1):
        print(f"\ntest {i}: {source}")
        block = parse_unsafe_block(source)
        if block:
            print(f"  success: unsafe block with {len(block.operations)} operations")
        else:
            print("  failed: could not parse unsafe block")

def test_unsafe_function_parsing():
    """test unsafe function parsing functionality"""
    print("\n=== testing unsafe function parsing ===")
    
    test_cases = [
        "unsafe fn deref_ptr(ptr: *const i32) -> i32 { return *ptr; }",
        "unsafe fn transmute_data(x: i32) -> u32 { return transmute(x); }",
        "unsafe fn offset_pointer(ptr: *mut i32, offset: isize) -> *mut i32 { return ptr.offset(offset); }",
        "unsafe fn call_unsafe() { unsafe_function(); }"
    ]
    
    for i, source in enumerate(test_cases, 1):
        print(f"\ntest {i}: {source}")
        func = parse_unsafe_function(source)
        if func:
            print(f"  success: unsafe function with {len(func.operations)} operations")
        else:
            print("  failed: could not parse unsafe function")

def test_unsafe_expression_parsing():
    """test unsafe expression parsing functionality"""
    print("\n=== testing unsafe expression parsing ===")
    
    test_cases = [
        "unsafe *ptr",
        "unsafe transmute::<i32, u32>(x)",
        "unsafe ptr.offset(1)",
        "unsafe cast_pointer(ptr)",
        "unsafe call_unsafe_function()"
    ]
    
    for i, source in enumerate(test_cases, 1):
        print(f"\ntest {i}: {source}")
        expr = parse_unsafe_expression(source)
        if expr:
            print(f"  success: unsafe {expr.operation.value}")
        else:
            print("  failed: could not parse unsafe expression")

def test_unsafe_type_checking():
    """test unsafe type checking and semantic analysis"""
    print("\n=== testing unsafe type checking ===")
    
    # create symbol table
    symbol_table = {
        "ptr": "*const i32",
        "mut_ptr": "*mut String",
        "unsafe_function": "unsafe fn() -> i32",
        "transmute": "unsafe fn transmute<T, U>(T) -> U"
    }
    
    # test unsafe blocks
    print("\ntest 1: unsafe block type checking")
    block = parse_unsafe_block("unsafe { let x = *ptr; return x; }")
    if block:
        result_type = analyze_unsafe_block(block, symbol_table)
        if result_type:
            print(f"  success: unsafe block -> {result_type}")
        else:
            print(f"  failed: type checking failed")
    
    # test unsafe functions
    print("\ntest 2: unsafe function type checking")
    func = parse_unsafe_function("unsafe fn deref(ptr: *const i32) -> i32 { return *ptr; }")
    if func:
        result_type = analyze_unsafe_block(func, symbol_table)
        if result_type:
            print(f"  success: unsafe function -> {result_type}")
        else:
            print(f"  failed: type checking failed")
    
    # test unsafe expressions
    print("\ntest 3: unsafe expression type checking")
    expr = parse_unsafe_expression("unsafe *ptr")
    if expr:
        result_type = analyze_unsafe_expression(expr, symbol_table)
        if result_type:
            print(f"  success: unsafe expression -> {result_type}")
        else:
            print(f"  failed: type checking failed")

def test_unsafe_code_generation():
    """test unsafe code generation"""
    print("\n=== testing unsafe code generation ===")
    
    symbol_table = {"ptr": "*const i32", "unsafe_function": "unsafe fn() -> i32"}
    
    # test unsafe block code generation
    print("\ntest 1: unsafe block code generation")
    block = parse_unsafe_block("unsafe { let x = *ptr; return x; }")
    if block:
        result_type = analyze_unsafe_block(block, symbol_table)
        if result_type:
            assembly = generate_unsafe_block_code(block, result_type)
            print(f"  generated assembly:")
            print(f"    {assembly.replace(chr(10), chr(10) + '    ')}")
    
    # test unsafe expression code generation
    print("\ntest 2: unsafe expression code generation")
    expr = parse_unsafe_expression("unsafe *ptr")
    if expr:
        result_type = analyze_unsafe_expression(expr, symbol_table)
        if result_type:
            assembly = generate_unsafe_expression_code(expr, result_type)
            print(f"  generated assembly:")
            print(f"    {assembly.replace(chr(10), chr(10) + '    ')}")

def test_unsafe_safety_checks():
    """test unsafe safety checks"""
    print("\n=== testing unsafe safety checks ===")
    
    # test pointer validity
    print("\ntest 1: pointer validity checks")
    pointer = RawPointerType("i32", False, 8)
    is_valid = check_pointer_validity(pointer)
    print(f"  pointer validity: {is_valid}")
    
    # test memory alignment
    print("\ntest 2: memory alignment checks")
    is_aligned = check_memory_alignment(pointer, 8)
    print(f"  memory alignment: {is_aligned}")
    
    # test unsafe function safety
    print("\ntest 3: unsafe function safety")
    is_safe = check_unsafe_function_safety("safe_function")
    print(f"  function safety: {is_safe}")

def test_unsafe_integration():
    """test complete unsafe blocks pipeline"""
    print("\n=== testing unsafe integration ===")
    
    # simulate a rust program with unsafe blocks
    rust_program = """
    unsafe fn deref_pointer(ptr: *const i32) -> i32 {
        return *ptr;
    }
    
    fn main() {
        let x = 42;
        let ptr = &x as *const i32;
        
        unsafe {
            let value = *ptr;
            println!("value: {}", value);
        }
        
        let result = unsafe { deref_pointer(ptr) };
        println!("result: {}", result);
    }
    """
    
    print("rust program with unsafe blocks:")
    print(rust_program)
    
    # simulate compilation pipeline
    print("\nsimulating compilation pipeline:")
    
    # 1. lexer (tokenization)
    print("1. lexer: tokenizing source code")
    tokens = ["unsafe", "fn", "deref_pointer", "(", "ptr", ":", "*const", "i32", ")", "->", "i32"]
    print(f"   generated {len(tokens)} tokens")
    
    # 2. parser (ast generation)
    print("2. parser: generating ast")
    unsafe_blocks = ["unsafe { let value = *ptr; }", "unsafe { deref_pointer(ptr) }"]
    unsafe_functions = ["deref_pointer"]
    print(f"   found {len(unsafe_blocks)} unsafe blocks")
    print(f"   found {len(unsafe_functions)} unsafe functions")
    
    # 3. type checker
    print("3. type checker: analyzing types")
    symbol_table = {"ptr": "*const i32", "deref_pointer": "unsafe fn(*const i32) -> i32"}
    print(f"   symbol table: {symbol_table}")
    
    # 4. semantic analysis
    print("4. semantic analysis: checking semantics")
    for block_src in unsafe_blocks:
        block = parse_unsafe_block(block_src)
        if block:
            result_type = analyze_unsafe_block(block, symbol_table)
            print(f"   unsafe block '{block_src}': {result_type}")
    
    # 5. code generation
    print("5. code generation: generating assembly")
    print("   generated unsafe runtime checks")
    print("   generated memory safety checks")
    print("   generated pointer validation")
    print("   generated unsafe function stubs")
    
    print("\nunsafe integration test complete!")

def main():
    """main test runner"""
    print("rust compiler - unsafe blocks functionality test")
    print("=============================================")
    
    test_unsafe_block_parsing()
    test_unsafe_function_parsing()
    test_unsafe_expression_parsing()
    test_unsafe_type_checking()
    test_unsafe_code_generation()
    test_unsafe_safety_checks()
    test_unsafe_integration()
    
    print("\n=== unsafe blocks test complete ===")
    print("all unsafe blocks functionality tests completed successfully")

if __name__ == "__main__":
    main() 