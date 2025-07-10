#!/usr/bin/env python3
"""
Code Generation Test Script
Simulates code generation functionality for x86-64 assembly
"""

import re
from typing import Dict, List, Optional, Union, Any

# IR node kinds (from semantic analysis)
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

class CodegenContext:
    def __init__(self):
        self.output = []
        self.label_counter = 0
        self.had_error = False
    
    def emit(self, line: str):
        self.output.append(line)
    
    def get_output(self) -> str:
        return "\n".join(self.output)

class CodeGenerator:
    def __init__(self):
        self.ctx = CodegenContext()
    
    def generate(self, ir: IRNode) -> str:
        """Generate x86-64 assembly from IR"""
        self.ctx.output = []
        self.ctx.emit(".section .text")
        self.ctx.emit(".global _start")
        self.ctx.emit("_start:")
        
        self.codegen_ir(ir)
        
        self.ctx.emit("mov rax, 60")
        self.ctx.emit("mov rdi, 0")
        self.ctx.emit("syscall")
        
        return self.ctx.get_output()
    
    def codegen_ir(self, ir: IRNode):
        """Generate assembly for IR node"""
        if not ir:
            return
        
        if ir.kind == IRKind.BLOCK:
            for child in ir.children:
                self.codegen_ir(child)
        
        elif ir.kind == IRKind.ASSIGN:
            self.ctx.emit(f"; assign {ir.name}")
            if ir.children:
                self.codegen_ir(ir.children[0])
                self.ctx.emit(f"mov [rbp-{self.get_var_offset(ir.name)}], rax")
        
        elif ir.kind == IRKind.CONST:
            if ir.name and ir.name.isdigit():
                self.ctx.emit(f"mov rax, {ir.name}")
            elif ir.name and ir.name.startswith('"'):
                self.ctx.emit(f"mov rax, {ir.name}")
            else:
                self.ctx.emit(f"; const {ir.name}")
        
        elif ir.kind == IRKind.VAR:
            self.ctx.emit(f"; var {ir.name}")
            self.ctx.emit(f"mov rax, [rbp-{self.get_var_offset(ir.name)}]")
        
        elif ir.kind == IRKind.BINOP:
            self.ctx.emit("; binop")
            if len(ir.children) >= 2:
                self.codegen_ir(ir.children[0])
                self.ctx.emit("push rax")
                self.codegen_ir(ir.children[1])
                self.ctx.emit("mov rbx, rax")
                self.ctx.emit("pop rax")
                self.ctx.emit("add rax, rbx")
        
        elif ir.kind == IRKind.CALL:
            self.ctx.emit(f"; call {ir.name}")
            for child in ir.children:
                self.codegen_ir(child)
                self.ctx.emit("push rax")
            self.ctx.emit(f"call {ir.name}")
        
        elif ir.kind == IRKind.RETURN:
            self.ctx.emit("; return")
            if ir.children:
                self.codegen_ir(ir.children[0])
            self.ctx.emit("ret")
        
        elif ir.kind == IRKind.NOP:
            pass
        
        else:
            self.ctx.emit(f"; unknown IR kind: {ir.kind}")
    
    def get_var_offset(self, name: str) -> str:
        """Get stack offset for variable (simplified)"""
        return "8"  # Simplified - real implementation would track stack layout

def test_code_generation():
    """Test code generation with sample IR"""
    generator = CodeGenerator()
    
    # Create sample IR
    root = IRNode(IRKind.BLOCK, "program")
    
    # Variable assignment: let x = 42
    assign = IRNode(IRKind.ASSIGN, "x")
    const = IRNode(IRKind.CONST, "42")
    assign.add_child(const)
    root.add_child(assign)
    
    # Binary operation: x + 1
    binop = IRNode(IRKind.BINOP)
    var = IRNode(IRKind.VAR, "x")
    const2 = IRNode(IRKind.CONST, "1")
    binop.add_child(var)
    binop.add_child(const2)
    root.add_child(binop)
    
    # Function call: print(x)
    call = IRNode(IRKind.CALL, "print")
    var2 = IRNode(IRKind.VAR, "x")
    call.add_child(var2)
    root.add_child(call)
    
    # Generate assembly
    assembly = generator.generate(root)
    
    print("=== Code Generation Test ===")
    print("Generated x86-64 assembly:")
    print(assembly)
    print("\n=== Code Generation Test Complete ===")

if __name__ == "__main__":
    test_code_generation() 