#!/usr/bin/env python3
"""
Type System Test Script
Simulates the type system functionality to validate our implementation
"""

import re
from typing import Dict, List, Optional, Union, Any

# Type system simulation
class TypeKind:
    UNKNOWN = "unknown"
    UNIT = "unit"
    BOOL = "bool"
    INT = "int"
    UINT = "uint"
    FLOAT = "float"
    CHAR = "char"
    STRING = "string"
    ARRAY = "array"
    SLICE = "slice"
    TUPLE = "tuple"
    STRUCT = "struct"
    ENUM = "enum"
    FUNCTION = "function"
    REF = "reference"
    POINTER = "pointer"
    GENERIC = "generic"

class Type:
    def __init__(self, kind: str, name: str = None, size: int = 0):
        self.kind = kind
        self.name = name or kind
        self.size = size
        self.is_sized = True
        self.flags = 0
        self.generic_args = []
        self.param_types = []
        self.return_type = None
        self.element_type = None
        self.referenced_type = None
        self.tuple_types = []
        self.field_types = []
        self.field_names = []
    
    def __str__(self):
        return self.name
    
    def __eq__(self, other):
        if not isinstance(other, Type):
            return False
        return self.kind == other.kind and self.name == other.name

# Built-in types
TYPE_UNIT = Type(TypeKind.UNIT, "()", 0)
TYPE_BOOL = Type(TypeKind.BOOL, "bool", 1)
TYPE_I32 = Type(TypeKind.INT, "i32", 4)
TYPE_I64 = Type(TypeKind.INT, "i64", 8)
TYPE_U32 = Type(TypeKind.UINT, "u32", 4)
TYPE_U64 = Type(TypeKind.UINT, "u64", 8)
TYPE_F64 = Type(TypeKind.FLOAT, "f64", 8)
TYPE_CHAR = Type(TypeKind.CHAR, "char", 4)
TYPE_STR = Type(TypeKind.STRING, "&str", 16)
TYPE_STRING = Type(TypeKind.STRING, "String", 24)

class TypeEnvironment:
    def __init__(self, parent=None):
        self.variables = {}
        self.parent = parent
    
    def insert(self, name: str, type_obj: Type) -> bool:
        self.variables[name] = type_obj
        return True
    
    def lookup(self, name: str) -> Optional[Type]:
        if name in self.variables:
            return self.variables[name]
        if self.parent:
            return self.parent.lookup(name)
        return None
    
    def push(self):
        return TypeEnvironment(self)
    
    def pop(self):
        return self.parent

class TypeContext:
    def __init__(self):
        self.env = TypeEnvironment()
        self.global_env = TypeEnvironment()
        self.had_error = False
        self.error_message = ""
        self.error_line = 0
        self.error_column = 0
    
    def error(self, message: str, line: int = 0, column: int = 0):
        self.had_error = True
        self.error_message = message
        self.error_line = line
        self.error_column = column
        print(f"Type error at line {line}, column {column}: {message}")

def type_is_numeric(type_obj: Type) -> bool:
    return type_obj.kind in [TypeKind.INT, TypeKind.UINT, TypeKind.FLOAT]

def type_is_boolean(type_obj: Type) -> bool:
    return type_obj.kind == TypeKind.BOOL

def type_equals(a: Type, b: Type) -> bool:
    return a == b

class TypeChecker:
    def __init__(self):
        self.ctx = TypeContext()
    
    def check_literal(self, value: str, token_type: str) -> Type:
        """Infer type from literal value"""
        if token_type == "integer":
            # Simple heuristic: if fits in i32, use i32, otherwise i64
            try:
                num = int(value)
                if -2147483648 <= num <= 2147483647:
                    return TYPE_I32
                else:
                    return TYPE_I64
            except ValueError:
                return TYPE_I64
        elif token_type == "float":
            return TYPE_F64
        elif token_type == "string":
            return TYPE_STR
        elif token_type == "char":
            return TYPE_CHAR
        elif token_type in ["true", "false"]:
            return TYPE_BOOL
        else:
            self.ctx.error(f"Unknown literal type: {token_type}")
            return None
    
    def check_binary_op(self, op: str, left_type: Type, right_type: Type) -> Type:
        """Check binary operation types"""
        if op in ["+", "-", "*", "/", "%"]:
            # Arithmetic operations
            if type_is_numeric(left_type) and type_is_numeric(right_type):
                if left_type.kind == TypeKind.FLOAT or right_type.kind == TypeKind.FLOAT:
                    return TYPE_F64
                else:
                    return TYPE_I32
            else:
                self.ctx.error(f"Arithmetic operation requires numeric types, got {left_type} and {right_type}")
                return None
        elif op in ["==", "!=", "<", ">", "<=", ">="]:
            # Comparison operations
            if type_is_numeric(left_type) and type_is_numeric(right_type):
                return TYPE_BOOL
            else:
                self.ctx.error(f"Comparison operation requires numeric types, got {left_type} and {right_type}")
                return None
        elif op in ["&&", "||"]:
            # Logical operations
            if type_is_boolean(left_type) and type_is_boolean(right_type):
                return TYPE_BOOL
            else:
                self.ctx.error(f"Logical operation requires boolean types, got {left_type} and {right_type}")
                return None
        elif op == "=":
            # Assignment
            if type_equals(left_type, right_type):
                return left_type
            else:
                self.ctx.error(f"Assignment types must match, got {left_type} and {right_type}")
                return None
        else:
            self.ctx.error(f"Unsupported binary operation: {op}")
            return None
    
    def check_variable_declaration(self, name: str, type_annotation: str, initializer: str) -> Type:
        """Check variable declaration"""
        if type_annotation:
            # Parse type annotation
            type_obj = self.parse_type_annotation(type_annotation)
            if not type_obj:
                self.ctx.error(f"Invalid type annotation: {type_annotation}")
                return None
        else:
            # Infer type from initializer
            type_obj = self.infer_initializer_type(initializer)
            if not type_obj:
                self.ctx.error(f"Cannot infer type for variable {name}")
                return None
        
        # Insert into environment
        self.ctx.env.insert(name, type_obj)
        return type_obj
    
    def parse_type_annotation(self, annotation: str) -> Optional[Type]:
        """Parse type annotation string"""
        annotation = annotation.strip()
        
        # Built-in types
        if annotation == "i32": return TYPE_I32
        elif annotation == "i64": return TYPE_I64
        elif annotation == "u32": return TYPE_U32
        elif annotation == "u64": return TYPE_U64
        elif annotation == "f64": return TYPE_F64
        elif annotation == "bool": return TYPE_BOOL
        elif annotation == "char": return TYPE_CHAR
        elif annotation == "String": return TYPE_STRING
        elif annotation == "&str": return TYPE_STR
        elif annotation == "()": return TYPE_UNIT
        
        # Reference types
        if annotation.startswith("&"):
            inner_type = self.parse_type_annotation(annotation[1:])
            if inner_type:
                ref_type = Type(TypeKind.REF, f"&{inner_type.name}")
                ref_type.referenced_type = inner_type
                return ref_type
        
        # Array types
        if annotation.startswith("[") and annotation.endswith("]"):
            # Simple array parsing - could be enhanced
            return Type(TypeKind.ARRAY, annotation)
        
        return None
    
    def infer_initializer_type(self, initializer: str) -> Optional[Type]:
        """Infer type from initializer expression"""
        initializer = initializer.strip()
        
        # Integer literal
        if re.match(r'^-?\d+$', initializer):
            num = int(initializer)
            if -2147483648 <= num <= 2147483647:
                return TYPE_I32
            else:
                return TYPE_I64
        
        # Float literal
        if re.match(r'^-?\d+\.\d+$', initializer):
            return TYPE_F64
        
        # String literal
        if initializer.startswith('"') and initializer.endswith('"'):
            return TYPE_STR
        
        # Character literal
        if initializer.startswith("'") and initializer.endswith("'"):
            return TYPE_CHAR
        
        # Boolean literal
        if initializer in ["true", "false"]:
            return TYPE_BOOL
        
        # Variable reference
        if re.match(r'^[a-zA-Z_][a-zA-Z0-9_]*$', initializer):
            ref_type = self.ctx.env.lookup(initializer)
            if ref_type is None:
                self.ctx.error(f"Undefined variable referenced: {initializer}")
            return ref_type
        
        # Binary expression (simplified)
        if any(op in initializer for op in ["+", "-", "*", "/", "==", "!=", "<", ">", "&&", "||"]):
            # This is a simplified approach - real implementation would parse the expression
            if "&&" in initializer or "||" in initializer:
                return TYPE_BOOL
            elif any(op in initializer for op in ["==", "!=", "<", ">", "<=", ">="]):
                return TYPE_BOOL
            else:
                return TYPE_I32  # Default to i32 for arithmetic expressions
        
        self.ctx.error(f"Could not infer type for initializer: {initializer}")
        return None
    
    def check_function_declaration(self, name: str, params: List[str], return_type: str, body: str) -> Type:
        """Check function declaration"""
        # Parse parameter types
        param_types = []
        for param in params:
            if ":" in param:
                param_name, param_type = param.split(":", 1)
                type_obj = self.parse_type_annotation(param_type.strip())
                if type_obj:
                    param_types.append(type_obj)
                else:
                    self.ctx.error(f"Invalid parameter type: {param_type}")
                    return None
            else:
                # Default to i32 for untyped parameters
                param_types.append(TYPE_I32)
        
        # Parse return type
        return_type_obj = None
        if return_type and return_type != "()":
            return_type_obj = self.parse_type_annotation(return_type)
            if not return_type_obj:
                self.ctx.error(f"Invalid return type: {return_type}")
                return None
        else:
            return_type_obj = TYPE_UNIT
        
        # Create function type
        func_type = Type(TypeKind.FUNCTION, f"fn({', '.join(str(t) for t in param_types)}) -> {return_type_obj}")
        func_type.param_types = param_types
        func_type.return_type = return_type_obj
        
        # Insert into global environment
        self.ctx.global_env.insert(name, func_type)
        
        return func_type
    
    def check_program(self, source_code: str) -> Optional[Type]:
        """Check a complete program"""
        print(f"Type checking program:\n{source_code}\n")
        
        # Simple parsing simulation
        lines = source_code.split('\n')
        program_type = TYPE_UNIT
        
        for line in lines:
            line = line.strip()
            if not line or line.startswith('//'):
                continue
            
            # Variable declaration
            if line.startswith('let '):
                match = re.match(r'let\s+(?:mut\s+)?(\w+)(?:\s*:\s*([^=]+))?\s*=\s*(.+)', line)
                if match:
                    name, type_annot, initializer = match.groups()
                    type_obj = self.check_variable_declaration(name, type_annot, initializer)
                    if type_obj:
                        program_type = type_obj
            
            # Function declaration
            elif line.startswith('fn '):
                # Simplified function parsing
                match = re.match(r'fn\s+(\w+)\s*\(([^)]*)\)\s*(?:->\s*([^{]+))?\s*{', line)
                if match:
                    name, params_str, return_type = match.groups()
                    params = [p.strip() for p in params_str.split(',') if p.strip()]
                    type_obj = self.check_function_declaration(name, params, return_type, "")
                    if type_obj:
                        program_type = type_obj
            
            # Expression
            else:
                # Try to infer type from expression
                type_obj = self.infer_initializer_type(line)
                if type_obj:
                    program_type = type_obj
        
        if self.ctx.had_error:
            print("Type checking failed with errors")
            return None
        else:
            print(f"Program type: {program_type}")
            return program_type

def test_type_system():
    """Test the type system with various Rust code examples"""
    checker = TypeChecker()
    
    test_cases = [
        # Test 1: Simple variable declarations
        """
        let x = 42;
        let y: i64 = 100;
        let name = "rust";
        """,
        
        # Test 2: Expressions
        """
        let result = 1 + 2 * 3;
        let comparison = x > 0;
        let logical = true && false;
        """,
        
        # Test 3: Function declaration
        """
        fn add(x: i32, y: i32) -> i32 {
            x + y
        }
        """,
        
        # Test 4: Control flow
        """
        let x = 10;
        if x > 0 {
            return x;
        } else {
            return 0;
        }
        """,
        
        # Test 5: Type annotations
        """
        let mut sum: i32 = 0;
        let name: String = "rust";
        let flag: bool = true;
        """,
    ]
    
    print("=== Type System Test ===\n")
    
    for i, test_case in enumerate(test_cases, 1):
        print(f"Test {i}:")
        checker = TypeChecker()  # Reset for each test
        result = checker.check_program(test_case)
        print(f"Result: {'SUCCESS' if result else 'FAILED'}")
        print("-" * 50)
    
    print("\n=== Type System Test Complete ===")

if __name__ == "__main__":
    test_type_system() 