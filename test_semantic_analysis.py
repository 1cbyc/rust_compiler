#!/usr/bin/env python3
"""
Semantic Analysis & IR Test Script
Simulates semantic analysis and IR generation functionality
"""

import re
from typing import Dict, List, Optional, Union, Any

# Symbol kinds
class SymbolKind:
    VARIABLE = "variable"
    FUNCTION = "function"
    STRUCT = "struct"
    ENUM = "enum"
    TYPE = "type"
    CONST = "const"

# IR node kinds
class IRKind:
    NOP = "nop"
    ASSIGN = "assign"
    BINOP = "binop"
    CALL = "call"
    JUMP = "jump"
    LABEL = "label"
    RETURN = "return"
    VAR = "var"
    CONST = "const"
    BLOCK = "block"

class Symbol:
    def __init__(self, kind: str, name: str, type_obj=None, ast_node=None):
        self.kind = kind
        self.name = name
        self.type = type_obj
        self.ast_node = ast_node

class SymbolTable:
    def __init__(self, parent=None):
        self.symbols = {}
        self.parent = parent
    
    def insert(self, name: str, kind: str, type_obj=None, ast_node=None) -> bool:
        self.symbols[name] = Symbol(kind, name, type_obj, ast_node)
        return True
    
    def lookup(self, name: str) -> Optional[Symbol]:
        if name in self.symbols:
            return self.symbols[name]
        if self.parent:
            return self.parent.lookup(name)
        return None

class IRNode:
    def __init__(self, kind: str, name: str = None, type_obj=None):
        self.kind = kind
        self.name = name
        self.type = type_obj
        self.children = []
    
    def add_child(self, child):
        self.children.append(child)
    
    def __str__(self):
        return f"{self.kind}({self.name or ''})"

class SemanticContext:
    def __init__(self):
        self.symbols = SymbolTable()
        self.had_error = False
        self.error_message = ""
        self.error_line = 0
        self.error_column = 0
    
    def error(self, message: str, line: int = 0, column: int = 0):
        self.had_error = True
        self.error_message = message
        self.error_line = line
        self.error_column = column
        print(f"Semantic error at line {line}, column {column}: {message}")

class SemanticAnalyzer:
    def __init__(self):
        self.ctx = SemanticContext()
    
    def analyze_variable_declaration(self, name: str, type_annot: str = None, initializer: str = None) -> IRNode:
        """Analyze variable declaration"""
        # Check for shadowing
        existing = self.ctx.symbols.lookup(name)
        if existing:
            self.ctx.error(f"Variable '{name}' shadows existing declaration")
        
        # Insert into symbol table
        self.ctx.symbols.insert(name, SymbolKind.VARIABLE, type_annot)
        
        # Create IR node
        node = IRNode(IRKind.ASSIGN, name, type_annot)
        if initializer:
            init_node = IRNode(IRKind.CONST, initializer)
            node.add_child(init_node)
        
        return node
    
    def analyze_function_declaration(self, name: str, params: List[str], return_type: str = None) -> IRNode:
        """Analyze function declaration"""
        # Check for redefinition
        existing = self.ctx.symbols.lookup(name)
        if existing:
            self.ctx.error(f"Function '{name}' redefined")
        
        # Insert function into symbol table
        self.ctx.symbols.insert(name, SymbolKind.FUNCTION, return_type)
        
        # Create new scope for function body
        old_symbols = self.ctx.symbols
        self.ctx.symbols = SymbolTable(old_symbols)
        
        # Add parameters to scope
        for param in params:
            if ':' in param:
                param_name, param_type = param.split(':', 1)
                self.ctx.symbols.insert(param_name.strip(), SymbolKind.VARIABLE, param_type.strip())
        
        # Create function IR node
        node = IRNode(IRKind.BLOCK, name, return_type)
        
        # Restore scope
        self.ctx.symbols = old_symbols
        
        return node
    
    def analyze_struct_declaration(self, name: str, fields: List[str]) -> IRNode:
        """Analyze struct declaration"""
        # Check for redefinition
        existing = self.ctx.symbols.lookup(name)
        if existing:
            self.ctx.error(f"Struct '{name}' redefined")
        
        # Insert struct into symbol table
        self.ctx.symbols.insert(name, SymbolKind.STRUCT, name)
        
        return IRNode(IRKind.NOP, name, name)
    
    def analyze_enum_declaration(self, name: str, variants: List[str]) -> IRNode:
        """Analyze enum declaration"""
        # Check for redefinition
        existing = self.ctx.symbols.lookup(name)
        if existing:
            self.ctx.error(f"Enum '{name}' redefined")
        
        # Insert enum into symbol table
        self.ctx.symbols.insert(name, SymbolKind.ENUM, name)
        
        return IRNode(IRKind.NOP, name, name)
    
    def analyze_expression(self, expr: str) -> IRNode:
        """Analyze expression"""
        # Variable reference
        if re.match(r'^[a-zA-Z_][a-zA-Z0-9_]*$', expr):
            sym = self.ctx.symbols.lookup(expr)
            if not sym:
                self.ctx.error(f"Undefined variable '{expr}'")
                return IRNode(IRKind.VAR, expr)
            return IRNode(IRKind.VAR, expr, sym.type)
        
        # Literal
        if re.match(r'^[0-9]+$', expr) or re.match(r'^"[^"]*"$', expr):
            return IRNode(IRKind.CONST, expr)
        
        # Function call
        m = re.match(r'(\w+)\(([^)]*)\)', expr)
        if m:
            func_name, args_str = m.groups()
            sym = self.ctx.symbols.lookup(func_name)
            if not sym or sym.kind != SymbolKind.FUNCTION:
                self.ctx.error(f"Undefined function '{func_name}'")
                return IRNode(IRKind.CALL, func_name)
            
            node = IRNode(IRKind.CALL, func_name, sym.type)
            args = [arg.strip() for arg in args_str.split(',') if arg.strip()]
            for arg in args:
                arg_node = self.analyze_expression(arg)
                node.add_child(arg_node)
            return node
        
        # Binary operation (simplified)
        if any(op in expr for op in ['+', '-', '*', '/', '==', '!=', '<', '>']):
            node = IRNode(IRKind.BINOP, None)
            # Simplified - in real implementation would parse the expression
            return node
        
        return IRNode(IRKind.CONST, expr)
    
    def analyze_program(self, source_code: str) -> Optional[IRNode]:
        """Analyze a complete program"""
        print(f"Semantic analysis of program:\n{source_code}\n")
        
        lines = source_code.split('\n')
        root = IRNode(IRKind.BLOCK, "program")
        
        for line in lines:
            line = line.strip()
            if not line or line.startswith('//'):
                continue
            
            # Variable declaration
            if line.startswith('let '):
                match = re.match(r'let\s+(?:mut\s+)?(\w+)(?:\s*:\s*([^=]+))?\s*=\s*(.+)', line)
                if match:
                    name, type_annot, initializer = match.groups()
                    node = self.analyze_variable_declaration(name, type_annot, initializer)
                    root.add_child(node)
            
            # Function declaration
            elif line.startswith('fn '):
                match = re.match(r'fn\s+(\w+)\s*\(([^)]*)\)\s*(?:->\s*([^{]+))?\s*{', line)
                if match:
                    name, params_str, return_type = match.groups()
                    params = [p.strip() for p in params_str.split(',') if p.strip()]
                    node = self.analyze_function_declaration(name, params, return_type)
                    root.add_child(node)
            
            # Struct declaration
            elif line.startswith('struct '):
                match = re.match(r'struct\s+(\w+)\s*{(.+)}', line)
                if match:
                    name, fields_str = match.groups()
                    fields = [f.strip() for f in fields_str.split(',') if f.strip()]
                    node = self.analyze_struct_declaration(name, fields)
                    root.add_child(node)
            
            # Enum declaration
            elif line.startswith('enum '):
                match = re.match(r'enum\s+(\w+)\s*{(.+)}', line)
                if match:
                    name, variants_str = match.groups()
                    variants = [v.strip() for v in variants_str.split(',') if v.strip()]
                    node = self.analyze_enum_declaration(name, variants)
                    root.add_child(node)
            
            # Expression
            else:
                node = self.analyze_expression(line)
                if node:
                    root.add_child(node)
        
        if self.ctx.had_error:
            print("Semantic analysis failed with errors")
            return None
        else:
            print(f"Semantic analysis successful, IR generated with {len(root.children)} children")
            return root

def test_semantic_analysis():
    """Test semantic analysis with various Rust code examples"""
    analyzer = SemanticAnalyzer()
    
    test_cases = [
        # Test 1: Variable declarations and shadowing
        """
        let x = 42
        let x = 100
        """,
        
        # Test 2: Function declarations
        """
        fn add(x: i32, y: i32) -> i32 {
        }
        fn add(x: i32, y: i32) -> i32 {
        }
        """,
        
        # Test 3: Struct and enum declarations
        """
        struct Point { x: i32, y: i32 }
        enum Option { Some, None }
        """,
        
        # Test 4: Variable references
        """
        let x = 42
        let y = x
        let z = undefined_var
        """,
        
        # Test 5: Function calls
        """
        fn print(x: i32) {
        }
        print(42)
        undefined_func()
        """,
        
        # Test 6: Complex program
        """
        struct Point { x: i32, y: i32 }
        fn create_point(x: i32, y: i32) -> Point {
        }
        let p = create_point(1, 2)
        """
    ]
    
    print("=== Semantic Analysis Test ===\n")
    
    for i, test_case in enumerate(test_cases, 1):
        print(f"Test {i}:")
        analyzer = SemanticAnalyzer()  # Reset for each test
        result = analyzer.analyze_program(test_case)
        print(f"Result: {'SUCCESS' if result else 'FAILED'}")
        print("-" * 50)
    
    print("\n=== Semantic Analysis Test Complete ===")

if __name__ == "__main__":
    test_semantic_analysis() 