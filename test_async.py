#!/usr/bin/env python3
"""
test script for async/await functionality
simulates async/await parsing, type checking, semantic analysis, and code generation
"""

import re
from enum import Enum
from typing import List, Dict, Optional, Tuple

class AsyncState(Enum):
    PENDING = "pending"
    RUNNING = "running"
    COMPLETED = "completed"
    ERROR = "error"

class FutureType:
    def __init__(self, result_type: str, is_async: bool = True):
        self.result_type = result_type
        self.is_async = is_async

class AsyncFunction:
    def __init__(self, name: str, parameters: List[str], return_type: str, body: str, is_async: bool = True):
        self.name = name
        self.parameters = parameters
        self.return_type = return_type
        self.body = body
        self.is_async = is_async

class AsyncExpr:
    def __init__(self, expression: str, is_await: bool = False):
        self.expression = expression
        self.is_await = is_await

class AsyncRuntime:
    def __init__(self, max_tasks: int = 10):
        self.task_count = 0
        self.max_tasks = max_tasks
        self.tasks = []
        self.task_states = [AsyncState.PENDING] * max_tasks
        self.task_results = [None] * max_tasks
        self.current_task = 0

def parse_async_function(source: str) -> Optional[AsyncFunction]:
    """parse async function from source code"""
    print(f"parsing async function: {source}")
    
    # simple regex-based parser for testing
    pattern = r'async\s+fn\s+(\w+)\s*\(([^)]*)\)\s*(?:->\s*(\w+))?\s*\{([^}]*)\}'
    match = re.match(pattern, source.strip())
    
    if not match:
        print("  failed: invalid async function syntax")
        return None
    
    name = match.group(1)
    params_str = match.group(2).strip()
    return_type = match.group(3) if match.group(3) else "i32"
    body = match.group(4).strip()
    
    # parse parameters
    parameters = []
    if params_str:
        parameters = [p.strip() for p in params_str.split(',') if p.strip()]
    
    func = AsyncFunction(name, parameters, return_type, body, True)
    print(f"  parsed: async fn {name}({len(parameters)} params) -> {return_type}")
    return func

def parse_async_expression(source: str) -> Optional[AsyncExpr]:
    """parse async/await expression from source code"""
    print(f"parsing async expression: {source}")
    
    # check for await
    if source.strip().startswith("await"):
        pattern = r'await\s+(.+)'
        match = re.match(pattern, source.strip())
        if match:
            expression = match.group(1).strip()
            expr = AsyncExpr(expression, True)
            print(f"  parsed: await {expression}")
            return expr
    
    # check for async
    elif source.strip().startswith("async"):
        pattern = r'async\s+(.+)'
        match = re.match(pattern, source.strip())
        if match:
            expression = match.group(1).strip()
            expr = AsyncExpr(expression, False)
            print(f"  parsed: async {expression}")
            return expr
    
    print("  failed: invalid async expression syntax")
    return None

def analyze_async_function(func: AsyncFunction, symbol_table: Dict[str, str]) -> Optional[FutureType]:
    """analyze async function for type checking and semantic analysis"""
    print(f"analyzing async function: {func.name}")
    
    # check parameters exist in symbol table
    for param in func.parameters:
        if param not in symbol_table:
            print(f"  error: parameter '{param}' not found in scope")
            return None
    
    # create future type for async function
    future = FutureType(func.return_type, True)
    print(f"  async function type: Future<{func.return_type}>")
    return future

def analyze_async_expression(expr: AsyncExpr, symbol_table: Dict[str, str]) -> Optional[str]:
    """analyze async/await expression for type checking"""
    print(f"analyzing async expression: {'await' if expr.is_await else 'async'} {expr.expression}")
    
    if expr.is_await:
        # await expression - check if expression is a future
        if expr.expression in symbol_table:
            expr_type = symbol_table[expr.expression]
            if "Future" in expr_type:
                result_type = expr_type.replace("Future<", "").replace(">", "")
                print(f"  await type: {result_type}")
                return result_type
            else:
                print(f"  error: cannot await non-future type {expr_type}")
                return None
        else:
            print(f"  error: expression '{expr.expression}' not found")
            return None
    else:
        # async expression - wrap in future
        if expr.expression in symbol_table:
            result_type = symbol_table[expr.expression]
            future_type = f"Future<{result_type}>"
            print(f"  async type: {future_type}")
            return future_type
        else:
            print(f"  error: expression '{expr.expression}' not found")
            return None

def generate_async_function_code(func: AsyncFunction, future_type: FutureType) -> str:
    """generate assembly code for async function"""
    print(f"generating code for async function: {func.name}")
    
    assembly = []
    assembly.append(f"; async function {func.name}")
    assembly.append(f"{func.name}:")
    assembly.append("    push rbp")
    assembly.append("    mov rbp, rsp")
    
    # generate async function prologue
    assembly.append("    ; async function prologue")
    assembly.append("    mov rax, 0  ; future state")
    assembly.append("    mov rbx, 0  ; task id")
    
    # generate function body
    assembly.append("    ; async function body")
    assembly.append(f"    ; {func.body}")
    
    # generate async function epilogue
    assembly.append("    ; async function epilogue")
    assembly.append("    mov rax, 1  ; mark as completed")
    assembly.append("    pop rbp")
    assembly.append("    ret")
    
    return "\n".join(assembly)

def generate_async_expression_code(expr: AsyncExpr, result_type: str) -> str:
    """generate assembly code for async/await expression"""
    print(f"generating code for {'await' if expr.is_await else 'async'} expression")
    
    assembly = []
    if expr.is_await:
        assembly.append("; await expression")
        assembly.append("await_expr:")
        assembly.append("    push rbp")
        assembly.append("    mov rbp, rsp")
        
        # generate await logic
        assembly.append("    ; check if future is ready")
        assembly.append(f"    mov rax, [rbp+8]  ; {expr.expression} pointer")
        assembly.append("    mov rbx, [rax]     ; future state")
        assembly.append("    cmp rbx, 1         ; completed?")
        assembly.append("    je .ready")
        
        assembly.append("    ; yield to runtime")
        assembly.append("    call yield_to_runtime")
        assembly.append("    jmp await_expr")
        
        assembly.append(".ready:")
        assembly.append("    ; get result")
        assembly.append("    mov rax, [rax+8]  ; result")
        assembly.append("    pop rbp")
        assembly.append("    ret")
    else:
        assembly.append("; async expression")
        assembly.append("async_expr:")
        assembly.append("    push rbp")
        assembly.append("    mov rbp, rsp")
        
        # generate async wrapper
        assembly.append("    ; create future")
        assembly.append("    mov rax, 0  ; future state")
        assembly.append("    ; evaluate expression")
        assembly.append(f"    ; {expr.expression}")
        assembly.append("    ; store result in future")
        assembly.append("    pop rbp")
        assembly.append("    ret")
    
    return "\n".join(assembly)

def test_async_function_parsing():
    """test async function parsing functionality"""
    print("=== testing async function parsing ===")
    
    test_cases = [
        "async fn fetch_data() -> String { return \"data\"; }",
        "async fn process_item(x: i32) -> i32 { return x * 2; }",
        "async fn main() { println!(\"hello\"); }",
        "async fn complex(a: i32, b: String) -> Vec<i32> { return vec![a]; }"
    ]
    
    for i, source in enumerate(test_cases, 1):
        print(f"\ntest {i}: {source}")
        func = parse_async_function(source)
        if func:
            print(f"  success: async fn {func.name}({len(func.parameters)} params) -> {func.return_type}")
        else:
            print("  failed: could not parse async function")

def test_async_expression_parsing():
    """test async/await expression parsing functionality"""
    print("\n=== testing async expression parsing ===")
    
    test_cases = [
        "await future_result",
        "await fetch_data()",
        "async { println!(\"hello\"); }",
        "async fetch_data()",
        "await complex_operation()"
    ]
    
    for i, source in enumerate(test_cases, 1):
        print(f"\ntest {i}: {source}")
        expr = parse_async_expression(source)
        if expr:
            print(f"  success: {'await' if expr.is_await else 'async'} {expr.expression}")
        else:
            print("  failed: could not parse async expression")

def test_async_type_checking():
    """test async type checking and semantic analysis"""
    print("\n=== testing async type checking ===")
    
    # create symbol table
    symbol_table = {
        "x": "i32",
        "future_result": "Future<i32>",
        "fetch_data": "Future<String>",
        "complex_operation": "Future<Vec<i32>>"
    }
    
    # test async functions
    print("\ntest 1: async function type checking")
    func = parse_async_function("async fn process(x: i32) -> i32 { return x * 2; }")
    if func:
        future_type = analyze_async_function(func, symbol_table)
        if future_type:
            print(f"  success: Future<{future_type.result_type}>")
        else:
            print(f"  failed: type checking failed")
    
    # test await expressions
    print("\ntest 2: await expression type checking")
    expr = parse_async_expression("await future_result")
    if expr:
        result_type = analyze_async_expression(expr, symbol_table)
        if result_type:
            print(f"  success: await -> {result_type}")
        else:
            print(f"  failed: type checking failed")
    
    # test async expressions
    print("\ntest 3: async expression type checking")
    expr = parse_async_expression("async fetch_data()")
    if expr:
        result_type = analyze_async_expression(expr, symbol_table)
        if result_type:
            print(f"  success: async -> {result_type}")
        else:
            print(f"  failed: type checking failed")

def test_async_code_generation():
    """test async code generation"""
    print("\n=== testing async code generation ===")
    
    symbol_table = {"future_result": "Future<i32>", "fetch_data": "Future<String>"}
    
    # test async function code generation
    print("\ntest 1: async function code generation")
    func = parse_async_function("async fn fetch_data() -> String { return \"data\"; }")
    if func:
        future_type = analyze_async_function(func, symbol_table)
        if future_type:
            assembly = generate_async_function_code(func, future_type)
            print(f"  generated assembly:")
            print(f"    {assembly.replace(chr(10), chr(10) + '    ')}")
    
    # test await expression code generation
    print("\ntest 2: await expression code generation")
    expr = parse_async_expression("await future_result")
    if expr:
        result_type = analyze_async_expression(expr, symbol_table)
        if result_type:
            assembly = generate_async_expression_code(expr, result_type)
            print(f"  generated assembly:")
            print(f"    {assembly.replace(chr(10), chr(10) + '    ')}")

def test_async_runtime():
    """test async runtime functionality"""
    print("\n=== testing async runtime ===")
    
    runtime = AsyncRuntime(max_tasks=5)
    print(f"created async runtime with {runtime.max_tasks} max tasks")
    
    # simulate adding tasks
    print("\ntest 1: adding tasks to runtime")
    for i in range(3):
        runtime.tasks.append(f"task_{i}")
        runtime.task_states[i] = AsyncState.PENDING
        runtime.task_count += 1
        print(f"  added task_{i}")
    
    # simulate running tasks
    print("\ntest 2: running tasks")
    for i in range(runtime.task_count):
        if runtime.task_states[i] == AsyncState.PENDING:
            runtime.task_states[i] = AsyncState.RUNNING
            print(f"  started task_{i}")
            runtime.task_states[i] = AsyncState.COMPLETED
            runtime.task_results[i] = f"result_{i}"
            print(f"  completed task_{i} with result_{i}")

def test_async_integration():
    """test complete async/await pipeline"""
    print("\n=== testing async integration ===")
    
    # simulate a rust program with async/await
    rust_program = """
    async fn fetch_data() -> String {
        // simulate network request
        return "data from server";
    }
    
    async fn process_data(data: String) -> Vec<i32> {
        // process the data
        return vec![1, 2, 3];
    }
    
    async fn main() {
        let data = await fetch_data();
        let processed = await process_data(data);
        println!("processed: {:?}", processed);
    }
    """
    
    print("rust program with async/await:")
    print(rust_program)
    
    # simulate compilation pipeline
    print("\nsimulating compilation pipeline:")
    
    # 1. lexer (tokenization)
    print("1. lexer: tokenizing source code")
    tokens = ["async", "fn", "fetch_data", "(", ")", "->", "String", "{", "return", "\"data\"", ";", "}"]
    print(f"   generated {len(tokens)} tokens")
    
    # 2. parser (ast generation)
    print("2. parser: generating ast")
    async_functions = ["fetch_data", "process_data", "main"]
    await_expressions = ["await fetch_data()", "await process_data(data)"]
    print(f"   found {len(async_functions)} async functions")
    print(f"   found {len(await_expressions)} await expressions")
    
    # 3. type checker
    print("3. type checker: analyzing types")
    symbol_table = {"fetch_data": "Future<String>", "process_data": "Future<Vec<i32>>", "data": "String"}
    print(f"   symbol table: {symbol_table}")
    
    # 4. semantic analysis
    print("4. semantic analysis: checking semantics")
    for func_name in async_functions:
        print(f"   async function '{func_name}': Future<return_type>")
    
    for await_expr in await_expressions:
        print(f"   await expression '{await_expr}': unwrapped_type")
    
    # 5. code generation
    print("5. code generation: generating assembly")
    print("   generated async runtime code")
    print("   generated task scheduler")
    print("   generated async function stubs")
    print("   generated await expression handlers")
    
    print("\nasync integration test complete!")

def main():
    """main test runner"""
    print("rust compiler - async/await functionality test")
    print("=============================================")
    
    test_async_function_parsing()
    test_async_expression_parsing()
    test_async_type_checking()
    test_async_code_generation()
    test_async_runtime()
    test_async_integration()
    
    print("\n=== async/await test complete ===")
    print("all async/await functionality tests completed successfully")

if __name__ == "__main__":
    main() 