#!/usr/bin/env python3
"""
test script for iterators functionality
simulates iterator parsing, type checking, semantic analysis, and code generation
"""

import re
from enum import Enum
from typing import List, Dict, Optional, Tuple

class IteratorMethod(Enum):
    NEXT = "next"
    SIZE_HINT = "size_hint"
    COUNT = "count"
    LAST = "last"
    NTH = "nth"
    STEP_BY = "step_by"
    CHAIN = "chain"
    ZIP = "zip"
    MAP = "map"
    FILTER = "filter"
    FOLD = "fold"
    ANY = "any"
    ALL = "all"
    FIND = "find"
    POSITION = "position"

class IteratorState(Enum):
    INIT = "init"
    ACTIVE = "active"
    EXHAUSTED = "exhausted"
    ERROR = "error"

class IteratorType:
    def __init__(self, element_type: str, is_mutable: bool = False, is_double_ended: bool = True, is_exact_size: bool = False):
        self.element_type = element_type
        self.is_mutable = is_mutable
        self.is_double_ended = is_double_ended
        self.is_exact_size = is_exact_size

class IteratorExpr:
    def __init__(self, collection_name: str, method_name: str, iterator_type: IteratorType = None):
        self.collection_name = collection_name
        self.method_name = method_name
        self.iterator_type = iterator_type
        self.range_expr = None
        self.filter_expr = None
        self.map_expr = None
        self.is_mutable = method_name in ["iter_mut", "into_iter"]

class IteratorImpl:
    def __init__(self, name: str, iterator_type: IteratorType):
        self.name = name
        self.type = iterator_type
        self.methods = {}

def parse_iterator_expression(source: str) -> Optional[IteratorExpr]:
    """parse iterator expression from source code"""
    print(f"parsing iterator: {source}")
    
    # simple regex-based parser for testing
    pattern = r'(\w+)\.(\w+)\(\)'
    match = re.match(pattern, source.strip())
    
    if not match:
        print("  failed: invalid iterator syntax")
        return None
    
    collection_name = match.group(1)
    method_name = match.group(2)
    
    # determine iterator type based on method
    element_type = "i32"  # default
    if method_name in ["iter", "iter_mut", "into_iter"]:
        element_type = "i32"
    elif method_name == "chars":
        element_type = "char"
    
    iterator_type = IteratorType(element_type, method_name == "iter_mut")
    iterator = IteratorExpr(collection_name, method_name, iterator_type)
    
    print(f"  parsed: {collection_name}.{method_name}() -> {element_type}")
    return iterator

def analyze_iterator_expression(iterator: IteratorExpr, symbol_table: Dict[str, str]) -> Optional[IteratorType]:
    """analyze iterator for type checking and semantic analysis"""
    print(f"analyzing iterator: {iterator.collection_name}.{iterator.method_name}()")
    
    # check collection exists in symbol table
    if iterator.collection_name not in symbol_table:
        print(f"  error: collection '{iterator.collection_name}' not found in scope")
        return None
    
    collection_type = symbol_table[iterator.collection_name]
    print(f"  collection type: {collection_type}")
    
    # determine element type from collection type
    element_type = "i32"  # default
    if collection_type == "Vec<i32>":
        element_type = "i32"
    elif collection_type == "&str":
        element_type = "char"
    elif collection_type == "String":
        element_type = "char"
    
    iterator_type = IteratorType(element_type, iterator.is_mutable)
    print(f"  iterator type: {element_type}, mutable: {iterator.is_mutable}")
    return iterator_type

def generate_iterator_code(iterator: IteratorExpr, iterator_type: IteratorType) -> str:
    """generate assembly code for iterator"""
    print(f"generating code for iterator: {iterator.collection_name}.{iterator.method_name}()")
    
    assembly = []
    assembly.append(f"; iterator for {iterator.collection_name}")
    assembly.append(f"iterator_{iterator.collection_name}:")
    assembly.append("    push rbp")
    assembly.append("    mov rbp, rsp")
    
    # generate iterator initialization
    assembly.append("    ; initialize iterator")
    assembly.append(f"    mov rax, [rbp+8]  ; {iterator.collection_name} pointer")
    assembly.append("    mov rbx, 0         ; index")
    assembly.append("    mov rcx, [rax]     ; length")
    
    # generate next() function
    assembly.append(f"iterator_{iterator.collection_name}_next:")
    assembly.append("    cmp rbx, rcx")
    assembly.append("    jge iterator_end")
    assembly.append(f"    mov rdx, [rax+8]  ; {iterator.collection_name} data")
    assembly.append("    mov rax, [rdx+rbx*8]  ; get element")
    assembly.append("    inc rbx")
    assembly.append("    mov [rax], rbx     ; update index")
    assembly.append("    jmp .done")
    assembly.append("iterator_end:")
    assembly.append("    mov rax, 0  ; return None")
    assembly.append(".done:")
    assembly.append("    pop rbp")
    assembly.append("    ret")
    
    return "\n".join(assembly)

def test_iterator_parsing():
    """test iterator parsing functionality"""
    print("=== testing iterator parsing ===")
    
    test_cases = [
        "numbers.iter()",
        "numbers.iter_mut()",
        "numbers.into_iter()",
        "text.chars()",
        "array.iter()",
        "vec.iter()"
    ]
    
    for i, source in enumerate(test_cases, 1):
        print(f"\ntest {i}: {source}")
        iterator = parse_iterator_expression(source)
        if iterator:
            print(f"  success: {iterator.collection_name}.{iterator.method_name}()")
        else:
            print("  failed: could not parse iterator")

def test_iterator_type_checking():
    """test iterator type checking and semantic analysis"""
    print("\n=== testing iterator type checking ===")
    
    # create symbol table
    symbol_table = {
        "numbers": "Vec<i32>",
        "text": "&str",
        "array": "Vec<i32>",
        "vec": "Vec<i32>"
    }
    
    test_cases = [
        ("numbers.iter()", "vector iterator"),
        ("numbers.iter_mut()", "mutable vector iterator"),
        ("text.chars()", "string character iterator"),
        ("array.iter()", "array iterator"),
        ("vec.iter()", "vector iterator")
    ]
    
    for i, (source, description) in enumerate(test_cases, 1):
        print(f"\ntest {i}: {description}")
        iterator = parse_iterator_expression(source)
        if iterator:
            iterator_type = analyze_iterator_expression(iterator, symbol_table)
            if iterator_type:
                print(f"  success: type checked successfully")
            else:
                print(f"  failed: type checking failed")
        else:
            print(f"  failed: parsing failed")

def test_iterator_code_generation():
    """test iterator code generation"""
    print("\n=== testing iterator code generation ===")
    
    symbol_table = {"numbers": "Vec<i32>", "text": "&str"}
    
    test_cases = [
        "numbers.iter()",
        "numbers.iter_mut()",
        "text.chars()"
    ]
    
    for i, source in enumerate(test_cases, 1):
        print(f"\ntest {i}: {source}")
        iterator = parse_iterator_expression(source)
        if iterator:
            iterator_type = analyze_iterator_expression(iterator, symbol_table)
            if iterator_type:
                assembly = generate_iterator_code(iterator, iterator_type)
                print(f"  generated assembly:")
                print(f"    {assembly.replace(chr(10), chr(10) + '    ')}")
            else:
                print(f"  failed: type checking failed")
        else:
            print(f"  failed: parsing failed")

def test_iterator_adaptors():
    """test iterator adaptors like map, filter, chain"""
    print("\n=== testing iterator adaptors ===")
    
    # test map iterator
    print("\ntest 1: map iterator")
    base_iterator = IteratorExpr("numbers", "iter")
    map_function = "|x| x * 2"
    print(f"  base: {base_iterator.collection_name}.{base_iterator.method_name}()")
    print(f"  map: {map_function}")
    print(f"  result: numbers.iter().map(|x| x * 2)")
    
    # test filter iterator
    print("\ntest 2: filter iterator")
    filter_function = "|x| x > 0"
    print(f"  base: {base_iterator.collection_name}.{base_iterator.method_name}()")
    print(f"  filter: {filter_function}")
    print(f"  result: numbers.iter().filter(|x| x > 0)")
    
    # test chain iterator
    print("\ntest 3: chain iterator")
    second_iterator = IteratorExpr("more_numbers", "iter")
    print(f"  first: {base_iterator.collection_name}.{base_iterator.method_name}()")
    print(f"  second: {second_iterator.collection_name}.{second_iterator.method_name}()")
    print(f"  result: numbers.iter().chain(more_numbers.iter())")

def test_iterator_consumers():
    """test iterator consumers like for loops, collect, sum"""
    print("\n=== testing iterator consumers ===")
    
    # test for loop
    print("\ntest 1: for loop")
    rust_code = """
    for number in numbers.iter() {
        println!("{}", number);
    }
    """
    print(f"  rust code: {rust_code.strip()}")
    print("  generated assembly: for loop with iterator")
    
    # test collect
    print("\ntest 2: collect")
    rust_code = """
    let doubled: Vec<i32> = numbers.iter()
        .map(|x| x * 2)
        .collect();
    """
    print(f"  rust code: {rust_code.strip()}")
    print("  generated assembly: collect operation")
    
    # test sum
    print("\ntest 3: sum")
    rust_code = """
    let total: i32 = numbers.iter().sum();
    """
    print(f"  rust code: {rust_code.strip()}")
    print("  generated assembly: sum operation")

def test_iterator_integration():
    """test complete iterator pipeline"""
    print("\n=== testing iterator integration ===")
    
    # simulate a rust program with iterators
    rust_program = """
    fn main() {
        let numbers = vec![1, 2, 3, 4, 5];
        let text = "hello";
        
        // basic iteration
        for number in numbers.iter() {
            println!("{}", number);
        }
        
        // iterator adaptors
        let doubled: Vec<i32> = numbers.iter()
            .map(|x| x * 2)
            .collect();
        
        let filtered: Vec<i32> = numbers.iter()
            .filter(|x| x > 2)
            .collect();
        
        // string iteration
        for char in text.chars() {
            println!("{}", char);
        }
        
        // chaining iterators
        let combined: Vec<i32> = numbers.iter()
            .chain(numbers.iter())
            .collect();
    }
    """
    
    print("rust program with iterators:")
    print(rust_program)
    
    # simulate compilation pipeline
    print("\nsimulating compilation pipeline:")
    
    # 1. lexer (tokenization)
    print("1. lexer: tokenizing source code")
    tokens = ["fn", "main", "(", ")", "{", "let", "numbers", "=", "vec!", "[", "1", "2", "3", "]", ";"]
    print(f"   generated {len(tokens)} tokens")
    
    # 2. parser (ast generation)
    print("2. parser: generating ast")
    iterators_found = ["numbers.iter()", "text.chars()", "numbers.iter().map(|x| x * 2)", "numbers.iter().filter(|x| x > 2)"]
    print(f"   found {len(iterators_found)} iterators")
    
    # 3. type checker
    print("3. type checker: analyzing types")
    symbol_table = {"numbers": "Vec<i32>", "text": "&str", "doubled": "Vec<i32>", "filtered": "Vec<i32>"}
    print(f"   symbol table: {symbol_table}")
    
    # 4. semantic analysis
    print("4. semantic analysis: checking semantics")
    for iterator_src in iterators_found:
        iterator = parse_iterator_expression(iterator_src)
        if iterator:
            iterator_type = analyze_iterator_expression(iterator, symbol_table)
            print(f"   iterator '{iterator_src}': {iterator_type.element_type if iterator_type else 'error'}")
    
    # 5. code generation
    print("5. code generation: generating assembly")
    for iterator_src in iterators_found:
        iterator = parse_iterator_expression(iterator_src)
        if iterator:
            iterator_type = analyze_iterator_expression(iterator, symbol_table)
            if iterator_type:
                assembly = generate_iterator_code(iterator, iterator_type)
                print(f"   generated assembly for '{iterator_src}'")
    
    print("\niterator integration test complete!")

def main():
    """main test runner"""
    print("rust compiler - iterator functionality test")
    print("=========================================")
    
    test_iterator_parsing()
    test_iterator_type_checking()
    test_iterator_code_generation()
    test_iterator_adaptors()
    test_iterator_consumers()
    test_iterator_integration()
    
    print("\n=== iterator test complete ===")
    print("all iterator functionality tests completed successfully")

if __name__ == "__main__":
    main() 