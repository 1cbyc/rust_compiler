# Rust Compiler User Guide

## Table of Contents

1. [Installation](#installation)
2. [Quick Start](#quick-start)
3. [Basic Usage](#basic-usage)
4. [Advanced Features](#advanced-features)
5. [Error Handling](#error-handling)
6. [Optimization](#optimization)
7. [Standard Library](#standard-library)
8. [Troubleshooting](#troubleshooting)
9. [Best Practices](#best-practices)

## Installation

### Prerequisites

- C compiler (GCC, Clang, or MSVC)
- Make or CMake
- Python 3.6+ (for test scripts)

### Building from Source

```bash
# Clone the repository
git clone https://github.com/your-repo/rust_compiler.git
cd rust_compiler

# Build the compiler
make

# Run tests
make test

# Install (optional)
make install
```

### Windows Build

```cmd
# Using Visual Studio
nmake /f Makefile.win

# Using MinGW
mingw32-make
```

## Quick Start

### Hello World

Create a file `hello.rs`:

```rust
fn main() {
    println!("Hello, world!");
}
```

Compile and run:

```bash
# Compile to assembly
./rust_compiler hello.rs -o hello.s

# Assemble and link
gcc hello.s -o hello

# Run
./hello
```

### Basic Function

```rust
fn add(x: i32, y: i32) -> i32 {
    return x + y;
}

fn main() {
    let result = add(5, 3);
    println!("5 + 3 = {}", result);
}
```

## Basic Usage

### Command Line Options

```bash
rust_compiler [options] <source_file>

Options:
  -o <output>     Output file (default: stdout)
  -S              Generate assembly only
  -O0             No optimization
  -O1             Basic optimization
  -O2             Full optimization
  -v              Verbose output
  --help          Show help
```

### Supported Rust Features

#### Variable Declarations

```rust
let x = 42;                    // Type inference
let mut y = 10;               // Mutable variable
let name: String = "Rust";    // Type annotation
```

#### Functions

```rust
fn greet(name: &str) -> &str {
    return "Hello, ";
}

fn main() {
    let message = greet("World");
    println!("{}{}", message, "World!");
}
```

#### Control Flow

```rust
fn absolute(x: i32) -> i32 {
    if x >= 0 {
        return x;
    } else {
        return -x;
    }
}

fn factorial(n: i32) -> i32 {
    let mut result = 1;
    let mut i = 1;
    
    while i <= n {
        result = result * i;
        i = i + 1;
    }
    
    return result;
}
```

#### Structs

```rust
struct Point {
    x: i32,
    y: i32,
}

impl Point {
    fn new(x: i32, y: i32) -> Self {
        Point { x, y }
    }
    
    fn distance(&self, other: &Point) -> f64 {
        let dx = self.x - other.x;
        let dy = self.y - other.y;
        return (dx * dx + dy * dy) as f64;
    }
}
```

#### Enums

```rust
enum Option<T> {
    Some(T),
    None,
}

fn safe_divide(a: i32, b: i32) -> Option<i32> {
    if b == 0 {
        return Option::None;
    } else {
        return Option::Some(a / b);
    }
}
```

## Advanced Features

### Macros

```rust
// Using standard library macros
println!("Hello, {}", "world");
print!("Value: {}", 42);

// Custom macro-like functions
let numbers = vec![1, 2, 3, 4, 5];
```

### Error Handling

```rust
fn read_number() -> Result<i32, String> {
    // Simulate reading a number
    let input = "42";
    
    match input.parse::<i32>() {
        Ok(n) => Result::Ok(n),
        Err(_) => Result::Err("Invalid number".to_string()),
    }
}
```

### Collections

```rust
fn process_numbers() {
    let mut numbers = Vec::new();
    
    // Add elements
    numbers.push(1);
    numbers.push(2);
    numbers.push(3);
    
    // Access elements
    let first = numbers.get(0);
    let length = numbers.len();
    
    // Iterate
    for i in 0..numbers.len() {
        println!("Number: {}", numbers.get(i));
    }
}
```

## Error Handling

### Understanding Error Messages

The compiler provides detailed error messages with source locations:

```
ERROR SYNTAX: Expected ';' after expression at test.rs:5:10
  |
5 | let x = 42
  |          ^
  |
Suggestion: Add semicolon after expression
```

### Common Error Types

#### Lexical Errors
- **Unexpected character**: Invalid characters in source code
- **Unterminated string**: Missing closing quote
- **Invalid number**: Malformed numeric literals

#### Syntax Errors
- **Missing semicolon**: Statements must end with semicolon
- **Unmatched brackets**: Missing closing parentheses/braces
- **Invalid expression**: Malformed expressions

#### Type Errors
- **Type mismatch**: Incompatible types in operations
- **Undefined type**: Using unknown types
- **Invalid operation**: Operations not supported for types

#### Semantic Errors
- **Undefined variable**: Using undeclared variables
- **Undefined function**: Calling unknown functions
- **Variable redefinition**: Declaring same variable twice

### Error Recovery

The compiler attempts to recover from errors and continue parsing:

```rust
let x = ;  // Syntax error
let y = 42; // This will still be parsed
```

### Debugging Tips

1. **Check line numbers**: Error messages include exact line and column
2. **Read suggestions**: Error messages often include fix suggestions
3. **Use verbose mode**: `-v` flag provides more detailed output
4. **Check error summary**: Final output shows error statistics

## Optimization

### Optimization Levels

```bash
# No optimization (fastest compilation)
rust_compiler -O0 program.rs

# Basic optimization
rust_compiler -O1 program.rs

# Full optimization (slowest compilation)
rust_compiler -O2 program.rs
```

### Optimization Passes

The compiler includes several optimization passes:

1. **Constant Folding**: Evaluates constant expressions at compile time
2. **Dead Code Elimination**: Removes unreachable code
3. **Loop Optimization**: Optimizes loop structures
4. **Register Allocation**: Optimizes register usage
5. **Code Size Optimization**: Reduces generated code size

### Performance Tips

1. **Use appropriate optimization level**: Balance compilation speed vs. runtime performance
2. **Profile your code**: Identify bottlenecks before optimizing
3. **Consider algorithm choice**: Better algorithms often beat micro-optimizations
4. **Use standard library**: Built-in functions are often optimized

## Standard Library

### Available Functions

#### I/O Functions
```rust
println!("Hello, world!");     // Print with newline
print!("Value: {}", 42);       // Print without newline
```

#### String Functions
```rust
let text = "Hello, Rust!";
let length = len(text);        // Get string length
let combined = concat("Hello", " World"); // Concatenate strings
```

#### Collection Functions
```rust
let numbers = Vec::new();      // Create empty vector
numbers.push(42);              // Add element
let value = numbers.get(0);    // Get element
```

#### Error Handling
```rust
let result = Result::Ok(42);   // Create Ok result
let error = Result::Err("error"); // Create Err result
```

### Using Macros

```rust
// Print macros
println!("Value: {}", 42);
print!("No newline");

// Vector macro
let numbers = vec![1, 2, 3, 4, 5];
```

## Troubleshooting

### Common Issues

#### Compilation Errors

**Problem**: `fatal error: cannot open source file`
**Solution**: Check file path and permissions

**Problem**: `syntax error: unexpected token`
**Solution**: Check for missing semicolons, brackets, or invalid syntax

**Problem**: `type error: incompatible types`
**Solution**: Ensure types match in operations and assignments

#### Runtime Errors

**Problem**: Segmentation fault
**Solution**: Check for null pointer access or array bounds

**Problem**: Incorrect output
**Solution**: Verify logic and check for off-by-one errors

#### Performance Issues

**Problem**: Slow compilation
**Solution**: Use lower optimization level or reduce code complexity

**Problem**: Large executable
**Solution**: Enable code size optimization

### Debugging Techniques

1. **Add debug prints**: Use `println!` to trace execution
2. **Check intermediate output**: Use `-S` to see generated assembly
3. **Use verbose mode**: `-v` provides detailed compilation info
4. **Examine error context**: Error messages include stack traces

### Getting Help

1. **Check error messages**: They often contain solutions
2. **Review documentation**: See API reference for details
3. **Run tests**: Verify your setup with test cases
4. **Report issues**: Include error messages and minimal examples

## Best Practices

### Code Organization

1. **Use meaningful names**: Choose descriptive variable and function names
2. **Keep functions small**: Break complex logic into smaller functions
3. **Add comments**: Document complex algorithms and assumptions
4. **Use consistent formatting**: Follow Rust style guidelines

### Error Handling

1. **Check return values**: Always verify function return codes
2. **Use Result types**: Return Result for functions that can fail
3. **Provide meaningful errors**: Include context in error messages
4. **Handle edge cases**: Consider boundary conditions and error states

### Performance

1. **Profile first**: Measure before optimizing
2. **Use appropriate data structures**: Choose efficient containers
3. **Avoid unnecessary allocations**: Reuse objects when possible
4. **Consider algorithm complexity**: Choose O(n) over O(n²) when possible

### Testing

1. **Write unit tests**: Test individual functions
2. **Test edge cases**: Include boundary conditions
3. **Test error conditions**: Verify error handling
4. **Use integration tests**: Test complete workflows

### Memory Management

1. **Free resources**: Always call cleanup functions
2. **Check for null**: Verify pointers before use
3. **Avoid memory leaks**: Use appropriate data structures
4. **Consider ownership**: Understand who owns each resource

## Examples

### Complete Programs

#### Calculator
```rust
fn add(x: i32, y: i32) -> i32 { return x + y; }
fn subtract(x: i32, y: i32) -> i32 { return x - y; }
fn multiply(x: i32, y: i32) -> i32 { return x * y; }
fn divide(x: i32, y: i32) -> i32 { return x / y; }

fn main() {
    let a = 10;
    let b = 5;
    
    println!("{} + {} = {}", a, b, add(a, b));
    println!("{} - {} = {}", a, b, subtract(a, b));
    println!("{} * {} = {}", a, b, multiply(a, b));
    println!("{} / {} = {}", a, b, divide(a, b));
}
```

#### Vector Operations
```rust
fn sum_vector(numbers: &Vec<i32>) -> i32 {
    let mut sum = 0;
    for i in 0..numbers.len() {
        sum = sum + numbers.get(i);
    }
    return sum;
}

fn main() {
    let mut numbers = Vec::new();
    numbers.push(1);
    numbers.push(2);
    numbers.push(3);
    numbers.push(4);
    numbers.push(5);
    
    let total = sum_vector(&numbers);
    println!("Sum: {}", total);
}
```

#### Error Handling
```rust
fn safe_divide(a: i32, b: i32) -> Result<i32, String> {
    if b == 0 {
        return Result::Err("Division by zero".to_string());
    } else {
        return Result::Ok(a / b);
    }
}

fn main() {
    let result = safe_divide(10, 2);
    match result {
        Result::Ok(value) => println!("Result: {}", value),
        Result::Err(error) => println!("Error: {}", error),
    }
    
    let error_result = safe_divide(10, 0);
    match error_result {
        Result::Ok(value) => println!("Result: {}", value),
        Result::Err(error) => println!("Error: {}", error),
    }
}
```

## Conclusion

This Rust compiler provides a solid foundation for compiling Rust-like code to assembly. While it doesn't implement the full Rust language, it demonstrates the key concepts of compiler construction and can be extended with additional features.

For more information, see the API Reference and test examples in the source code. 