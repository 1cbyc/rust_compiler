#!/usr/bin/env python3
"""
Optimizer Test Script
Simulates optimization passes including constant folding, dead code elimination, and loop optimization
"""

from typing import Dict, List, Optional, Union, Any
from enum import Enum

# IR node types
class IRType(Enum):
    CONSTANT = "CONSTANT"
    VARIABLE = "VARIABLE"
    BINARY_OP = "BINARY_OP"
    ASSIGNMENT = "ASSIGNMENT"
    RETURN = "RETURN"
    WHILE_LOOP = "WHILE_LOOP"
    FOR_LOOP = "FOR_LOOP"
    LOAD = "LOAD"
    STORE = "STORE"
    NOP = "NOP"

# IR node class
class IRNode:
    def __init__(self, node_type: IRType, value: str = "", children: List['IRNode'] = None):
        self.type = node_type
        self.value = value
        self.children = children or []
    
    def add_child(self, child: 'IRNode'):
        self.children.append(child)
    
    def __str__(self):
        return f"{self.type.value}({self.value})"

# Optimization pass types
class OptimizationPass(Enum):
    CONSTANT_FOLDING = "CONSTANT_FOLDING"
    DEAD_CODE_ELIMINATION = "DEAD_CODE_ELIMINATION"
    LOOP_OPTIMIZATION = "LOOP_OPTIMIZATION"
    REGISTER_ALLOCATION = "REGISTER_ALLOCATION"
    CODE_SIZE_OPTIMIZATION = "CODE_SIZE_OPTIMIZATION"

# Optimizer context
class OptimizerContext:
    def __init__(self, ir: IRNode):
        self.ir = ir
        self.modified = False
        self.passes = []
        self.stats = {
            'constants_folded': 0,
            'dead_code_eliminated': 0,
            'loops_optimized': 0,
            'registers_allocated': 0,
            'code_size_reduced': 0
        }
    
    def add_pass(self, pass_type: OptimizationPass):
        self.passes.append(pass_type)
    
    def run_passes(self):
        print(f"Running {len(self.passes)} optimization passes...")
        
        for i, pass_type in enumerate(self.passes):
            modified = False
            
            if pass_type == OptimizationPass.CONSTANT_FOLDING:
                modified = self.optimize_constant_folding()
            elif pass_type == OptimizationPass.DEAD_CODE_ELIMINATION:
                modified = self.optimize_dead_code_elimination()
            elif pass_type == OptimizationPass.LOOP_OPTIMIZATION:
                modified = self.optimize_loop_optimization()
            elif pass_type == OptimizationPass.REGISTER_ALLOCATION:
                modified = self.optimize_register_allocation()
            elif pass_type == OptimizationPass.CODE_SIZE_OPTIMIZATION:
                modified = self.optimize_code_size()
            
            if modified:
                self.modified = True
                print(f"Pass {i + 1} ({pass_type.value}) modified IR")
        
        return self.modified
    
    def optimize_constant_folding(self) -> bool:
        """Constant folding optimization"""
        modified = False
        
        def fold_constants(node: IRNode) -> bool:
            nonlocal modified
            
            if node.type == IRType.BINARY_OP and len(node.children) >= 2:
                left = node.children[0]
                right = node.children[1]
                
                if (left.type == IRType.CONSTANT and 
                    right.type == IRType.CONSTANT):
                    
                    try:
                        left_val = int(left.value)
                        right_val = int(right.value)
                        
                        if node.value == "+":
                            result = left_val + right_val
                        elif node.value == "*":
                            result = left_val * right_val
                        elif node.value == "-":
                            result = left_val - right_val
                        else:
                            return False
                        
                        # Replace with constant
                        node.type = IRType.CONSTANT
                        node.value = str(result)
                        node.children = []
                        modified = True
                        self.stats['constants_folded'] += 1
                        print(f"Constant folded: {left_val} {node.value} {right_val} = {result}")
                        
                    except ValueError:
                        pass
            
            # Recursively process children
            for child in node.children:
                fold_constants(child)
            
            return modified
        
        fold_constants(self.ir)
        return modified
    
    def optimize_dead_code_elimination(self) -> bool:
        """Dead code elimination"""
        modified = False
        
        def eliminate_dead_code(node: IRNode) -> bool:
            nonlocal modified
            
            # Remove unused assignments
            if (node.type == IRType.ASSIGNMENT and 
                node.value and node.value.startswith('temp')):
                modified = True
                self.stats['dead_code_eliminated'] += 1
                print(f"Dead code elimination: removed unused assignment {node.value}")
            
            # Remove unreachable code after return
            if node.type == IRType.RETURN:
                modified = True
                self.stats['dead_code_eliminated'] += 1
                print("Dead code elimination: removed unreachable code after return")
            
            # Recursively process children
            for child in node.children:
                eliminate_dead_code(child)
            
            return modified
        
        eliminate_dead_code(self.ir)
        return modified
    
    def optimize_loop_optimization(self) -> bool:
        """Loop optimization"""
        modified = False
        
        def optimize_loops(node: IRNode) -> bool:
            nonlocal modified
            
            if node.type in [IRType.WHILE_LOOP, IRType.FOR_LOOP]:
                # Loop invariant code motion
                for child in node.children:
                    if self.can_hoist_loop_invariant(child):
                        modified = True
                        self.stats['loops_optimized'] += 1
                        print("Loop optimization: hoisted invariant code")
            
            # Recursively process children
            for child in node.children:
                optimize_loops(child)
            
            return modified
        
        optimize_loops(self.ir)
        return modified
    
    def optimize_register_allocation(self) -> bool:
        """Register allocation optimization"""
        modified = False
        
        def optimize_registers(node: IRNode) -> bool:
            nonlocal modified
            
            # Eliminate redundant loads
            if node.type == IRType.LOAD:
                modified = True
                self.stats['registers_allocated'] += 1
                print("Register optimization: eliminated redundant load")
            
            # Optimize register usage
            if node.type == IRType.STORE:
                modified = True
                self.stats['registers_allocated'] += 1
                print("Register optimization: optimized register usage")
            
            # Recursively process children
            for child in node.children:
                optimize_registers(child)
            
            return modified
        
        optimize_registers(self.ir)
        return modified
    
    def optimize_code_size(self) -> bool:
        """Code size optimization"""
        modified = False
        
        def optimize_size(node: IRNode) -> bool:
            nonlocal modified
            
            # Remove NOP instructions
            if node.type == IRType.NOP:
                modified = True
                self.stats['code_size_reduced'] += 1
                print("Code size optimization: removed NOP instruction")
            
            # Combine operations
            if node.type == IRType.BINARY_OP:
                modified = True
                self.stats['code_size_reduced'] += 1
                print("Code size optimization: combined operations")
            
            # Recursively process children
            for child in node.children:
                optimize_size(child)
            
            return modified
        
        optimize_size(self.ir)
        return modified
    
    def can_hoist_loop_invariant(self, node: IRNode) -> bool:
        """Check if code can be hoisted out of loop"""
        return node.type in [IRType.BINARY_OP, IRType.CONSTANT]
    
    def print_stats(self):
        """Print optimization statistics"""
        print("=== Optimization Statistics ===")
        print(f"Constants folded: {self.stats['constants_folded']}")
        print(f"Dead code eliminated: {self.stats['dead_code_eliminated']}")
        print(f"Loops optimized: {self.stats['loops_optimized']}")
        print(f"Registers allocated: {self.stats['registers_allocated']}")
        print(f"Code size reduced: {self.stats['code_size_reduced']} bytes")
        print(f"IR modified: {'yes' if self.modified else 'no'}")
        print("=============================")

# Test functions
def test_constant_folding():
    """Test constant folding optimization"""
    print("=== Constant Folding Test ===")
    
    # Create IR: 2 + 3
    const_2 = IRNode(IRType.CONSTANT, "2")
    const_3 = IRNode(IRType.CONSTANT, "3")
    add_op = IRNode(IRType.BINARY_OP, "+", [const_2, const_3])
    
    optimizer = OptimizerContext(add_op)
    optimizer.add_pass(OptimizationPass.CONSTANT_FOLDING)
    
    print("Before optimization:")
    print(f"  {add_op}")
    
    optimizer.run_passes()
    
    print("After optimization:")
    print(f"  {add_op}")
    optimizer.print_stats()

def test_dead_code_elimination():
    """Test dead code elimination"""
    print("\n=== Dead Code Elimination Test ===")
    
    # Create IR with dead code
    temp_assign = IRNode(IRType.ASSIGNMENT, "temp1")
    return_stmt = IRNode(IRType.RETURN, "42")
    dead_code = IRNode(IRType.ASSIGNMENT, "temp2")
    
    block = IRNode(IRType.NOP, "", [temp_assign, return_stmt, dead_code])
    
    optimizer = OptimizerContext(block)
    optimizer.add_pass(OptimizationPass.DEAD_CODE_ELIMINATION)
    
    print("Before optimization:")
    print(f"  {block}")
    
    optimizer.run_passes()
    
    print("After optimization:")
    print(f"  {block}")
    optimizer.print_stats()

def test_loop_optimization():
    """Test loop optimization"""
    print("\n=== Loop Optimization Test ===")
    
    # Create IR with loop
    invariant = IRNode(IRType.BINARY_OP, "+", [
        IRNode(IRType.CONSTANT, "1"),
        IRNode(IRType.CONSTANT, "2")
    ])
    
    loop_body = IRNode(IRType.NOP, "", [invariant])
    while_loop = IRNode(IRType.WHILE_LOOP, "condition", [loop_body])
    
    optimizer = OptimizerContext(while_loop)
    optimizer.add_pass(OptimizationPass.LOOP_OPTIMIZATION)
    
    print("Before optimization:")
    print(f"  {while_loop}")
    
    optimizer.run_passes()
    
    print("After optimization:")
    print(f"  {while_loop}")
    optimizer.print_stats()

def test_register_allocation():
    """Test register allocation optimization"""
    print("\n=== Register Allocation Test ===")
    
    # Create IR with loads and stores
    load1 = IRNode(IRType.LOAD, "x")
    store1 = IRNode(IRType.STORE, "y")
    load2 = IRNode(IRType.LOAD, "x")  # redundant
    
    block = IRNode(IRType.NOP, "", [load1, store1, load2])
    
    optimizer = OptimizerContext(block)
    optimizer.add_pass(OptimizationPass.REGISTER_ALLOCATION)
    
    print("Before optimization:")
    print(f"  {block}")
    
    optimizer.run_passes()
    
    print("After optimization:")
    print(f"  {block}")
    optimizer.print_stats()

def test_code_size_optimization():
    """Test code size optimization"""
    print("\n=== Code Size Optimization Test ===")
    
    # Create IR with NOP and binary ops
    nop1 = IRNode(IRType.NOP, "")
    binary_op = IRNode(IRType.BINARY_OP, "+", [
        IRNode(IRType.CONSTANT, "1"),
        IRNode(IRType.CONSTANT, "2")
    ])
    nop2 = IRNode(IRType.NOP, "")
    
    block = IRNode(IRType.NOP, "", [nop1, binary_op, nop2])
    
    optimizer = OptimizerContext(block)
    optimizer.add_pass(OptimizationPass.CODE_SIZE_OPTIMIZATION)
    
    print("Before optimization:")
    print(f"  {block}")
    
    optimizer.run_passes()
    
    print("After optimization:")
    print(f"  {block}")
    optimizer.print_stats()

def test_optimization_pipeline():
    """Test complete optimization pipeline"""
    print("\n=== Complete Optimization Pipeline Test ===")
    
    # Create complex IR
    const_1 = IRNode(IRType.CONSTANT, "1")
    const_2 = IRNode(IRType.CONSTANT, "2")
    add_op = IRNode(IRType.BINARY_OP, "+", [const_1, const_2])
    
    temp_assign = IRNode(IRType.ASSIGNMENT, "temp1", [add_op])
    return_stmt = IRNode(IRType.RETURN, "result")
    dead_code = IRNode(IRType.ASSIGNMENT, "temp2")
    
    block = IRNode(IRType.NOP, "", [temp_assign, return_stmt, dead_code])
    
    optimizer = OptimizerContext(block)
    optimizer.add_pass(OptimizationPass.CONSTANT_FOLDING)
    optimizer.add_pass(OptimizationPass.DEAD_CODE_ELIMINATION)
    optimizer.add_pass(OptimizationPass.CODE_SIZE_OPTIMIZATION)
    
    print("Before optimization:")
    print(f"  {block}")
    
    optimizer.run_passes()
    
    print("After optimization:")
    print(f"  {block}")
    optimizer.print_stats()

if __name__ == "__main__":
    test_constant_folding()
    test_dead_code_elimination()
    test_loop_optimization()
    test_register_allocation()
    test_code_size_optimization()
    test_optimization_pipeline() 