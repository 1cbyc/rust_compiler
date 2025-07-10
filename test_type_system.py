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
        
        # User-defined struct/enum
        t = self.ctx.global_env.lookup(annotation)
        if t:
            return t
        
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
        
        # Enum variant (e.g., Option::Some)
        m = re.match(r'(\w+)::(\w+)', initializer)
        if m:
            enum_name, variant = m.groups()
            enum_type = self.ctx.global_env.lookup(enum_name)
            if enum_type and enum_type.kind == TypeKind.ENUM:
                return enum_type
            self.ctx.error(f"Unknown enum or variant: {initializer}")
            return None
        
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
    
    def check_struct_declaration(self, name: str, fields: List[str]) -> Type:
        """Check struct declaration"""
        field_types = []
        field_names = []
        for field in fields:
            if ':' in field:
                fname, ftype = field.split(':', 1)
                field_names.append(fname.strip())
                t = self.parse_type_annotation(ftype.strip())
                if t:
                    field_types.append(t)
                else:
                    self.ctx.error(f"Invalid struct field type: {ftype}")
                    return None
        struct_type = Type(TypeKind.STRUCT, name)
        struct_type.field_names = field_names
        struct_type.field_types = field_types
        self.ctx.global_env.insert(name, struct_type)
        return struct_type

    def check_enum_declaration(self, name: str, variants: List[str]) -> Type:
        """Check enum declaration"""
        variant_names = []
        for v in variants:
            v = v.strip()
            if v:
                variant_names.append(v)
        enum_type = Type(TypeKind.ENUM, name)
        enum_type.field_names = variant_names
        self.ctx.global_env.insert(name, enum_type)
        return enum_type

    def check_function_call(self, name: str, args: List[str]) -> Optional[Type]:
        """Check function call type"""
        func_type = self.ctx.global_env.lookup(name)
        if not func_type or func_type.kind != TypeKind.FUNCTION:
            self.ctx.error(f"Undefined function: {name}")
            return None
        if len(args) != len(func_type.param_types):
            self.ctx.error(f"Function argument count mismatch for {name}")
            return None
        for i, arg in enumerate(args):
            arg_type = self.infer_initializer_type(arg)
            if not arg_type or not type_equals(arg_type, func_type.param_types[i]):
                self.ctx.error(f"Function argument type mismatch for {name} at position {i+1}")
                return None
        return func_type.return_type

    def check_program(self, source_code: str) -> Optional[Type]:
        print(f"Type checking program:\n{source_code}\n")
        lines = source_code.split('\n')
        program_type = TYPE_UNIT
        
        for line in lines:
            line = line.strip()
            if not line or line.startswith('//'):
                continue
            
            # Skip block syntax lines
            if line in ['{', '}', '} else {'] or line.startswith('}') or line.endswith('{'):
                continue
                
            # Skip return statements (they're handled in control flow)
            if line.startswith('return '):
                continue
                
            # Struct declaration
            if line.startswith('struct '):
                m = re.match(r'struct\s+(\w+)\s*{(.+)}', line)
                if m:
                    name, fields_str = m.groups()
                    fields = [f.strip() for f in fields_str.split(',') if f.strip()]
                    t = self.check_struct_declaration(name, fields)
                    if t:
                        program_type = t
                continue
                
            # Enum declaration
            if line.startswith('enum '):
                m = re.match(r'enum\s+(\w+)\s*{(.+)}', line)
                if m:
                    name, variants_str = m.groups()
                    variants = [v.strip() for v in variants_str.split(',') if v.strip()]
                    t = self.check_enum_declaration(name, variants)
                    if t:
                        program_type = t
                continue
                
            # Function call (very simple simulation: name(args))
            m = re.match(r'(\w+)\(([^)]*)\);', line)
            if m:
                fname, args_str = m.groups()
                args = [a.strip() for a in args_str.split(',') if a.strip()]
                t = self.check_function_call(fname, args)
                if t:
                    program_type = t
                continue
                
            # Variable declaration
            if line.startswith('let '):
                match = re.match(r'let\s+(?:mut\s+)?(\w+)(?:\s*:\s*([^=]+))?\s*=\s*(.+)', line)
                if match:
                    name, type_annot, initializer = match.groups()
                    type_obj = self.check_variable_declaration(name, type_annot, initializer)
                    if type_obj:
                        program_type = type_obj
                continue
                
            # Function declaration
            if line.startswith('fn '):
                match = re.match(r'fn\s+(\w+)\s*\(([^)]*)\)\s*(?:->\s*([^{]+))?\s*{', line)
                if match:
                    name, params_str, return_type = match.groups()
                    params = [p.strip() for p in params_str.split(',') if p.strip()]
                    type_obj = self.check_function_declaration(name, params, return_type, "")
                    if type_obj:
                        program_type = type_obj
                continue
                
            # Expression (only if it looks like a simple expression)
            if re.match(r'^[a-zA-Z_][a-zA-Z0-9_]*$', line) or re.match(r'^[0-9]+$', line) or re.match(r'^"[^"]*"$', line):
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
    checker = TypeChecker()
    test_cases = [
        # Test 1: Simple variable declarations
        """
        let x = 42
        let y = 100
        let name = "rust"
        """,
        
        # Test 2: Expressions
        """
        let result = 1 + 2 * 3
        let comparison = x > 0
        let logical = true && false
        """,
        
        # Test 3: Function declaration
        """
        fn add(x: i32, y: i32) -> i32 {
        }
        """,
        
        # Test 4: Control flow (simplified)
        """
        let x = 10
        if x > 0 {
        } else {
        }
        """,
        
        # Test 5: Type annotations
        """
        let mut sum = 0
        let name = "rust"
        let flag = true
        """,
        
        # Test 6: Struct and function call
        """
        struct Point { x: i32, y: i32 }
        fn make_point(x: i32, y: i32) -> Point {
        }
        make_point(1, 2)
        """,
        
        # Test 7: Enum
        """
        enum Option { Some, None }
        Option::Some
        """,
        
        # Test 8: Struct as type annotation and return type
        """
        struct Point { x: i32, y: i32 }
        fn make_point(x: i32, y: i32) -> Point {
        }
        let p = make_point(1, 2)
        """,
        
        # Test 9: Enum variant usage
        """
        enum Option { Some, None }
        let a = Option::Some
        """,
        
        # Test 10: Generics (simulated)
        """
        struct Wrapper { value: i32 }
        fn wrap(x: i32) -> Wrapper {
        }
        wrap(42)
        """
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