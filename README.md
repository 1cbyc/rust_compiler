# Rust Compiler in C

A Rust compiler implementation written in C, because why not take the challenge to the next level? 🦀

## Project Status

### ✅ Completed
- **Lexer**: Complete tokenization of Rust syntax
  - All Rust keywords (`fn`, `let`, `mut`, `if`, `else`, `loop`, `while`, `for`, `in`, `match`, `return`, `break`, `continue`, `struct`, `enum`, `impl`, `trait`, `use`, `pub`, `crate`, `mod`, `as`, `where`, `type`, `const`, `static`, `unsafe`, `extern`, `move`, `ref`, `box`, `self`, `super`, `true`, `false`)
  - All operators (`+`, `-`, `*`, `/`, `%`, `=`, `==`, `!=`, `<`, `>`, `<=`, `>=`, `+=`, `-=`, `*=`, `/=`, `%=`, `&&`, `||`, `->`, `=>`, `<<`, `>>`, `&`, `|`, `^`, `!`, `~`)
  - All delimiters (`()`, `{}`, `[]`, `;`, `:`, `,`, `.`, `@`, `#`, `$`, `?`, `_`)
  - Literals (integers, floats, strings, characters)
  - Comments (single-line `//` and multi-line `/* */`)
  - Proper line and column tracking for error reporting

### 🚧 In Progress
- **Parser**: Converting tokens to Abstract Syntax Tree (AST)

### 📋 Planned
- **Type Checker**: Implement Rust's type system
- **Code Generator**: Generate machine code or LLVM IR
- **Standard Library**: Basic Rust std library functions
- **Error Handling**: Comprehensive error messages
- **Optimizations**: Basic compiler optimizations

## Project Structure

```
rust_compiler/
├── src/
│   ├── lexer.h          # Lexer header with token definitions
│   ├── lexer.c          # Lexer implementation
│   └── main.c           # Main program and tests
├── old-files/           # Original placeholder files
├── test_lexer.py        # Python test script for lexer
├── build.bat            # Windows build script
├── Makefile             # Unix/Linux build system
└── README.md            # This file
```

## Building and Testing

### Windows
```bash
# Build the project
build.bat

# Or manually with available compiler
gcc -Wall -Wextra -std=c99 -g src/*.c -o rust_compiler.exe
```

### Unix/Linux
```bash
# Build the project
make

# Run tests
make test

# Clean build artifacts
make clean
```

### Python Test (No C compiler needed)
```bash
python test_lexer.py
```

## Example Output

The lexer correctly tokenizes Rust code:

```rust
fn main() {
    let x = 42;
    println!("Hello, world!");
}
```

Produces tokens:
```
[0] FN: 'fn' (line 1, col 1)
[1] IDENTIFIER: 'main' (line 1, col 4)
[2] LPAREN: '(' (line 1, col 8)
[3] RPAREN: ')' (line 1, col 9)
[4] LBRACE: '{' (line 1, col 11)
[5] LET: 'let' (line 2, col 5)
[6] IDENTIFIER: 'x' (line 2, col 9)
[7] ASSIGN: '=' (line 2, col 11)
[8] INTEGER_LITERAL: '42' (line 2, col 13)
[9] SEMICOLON: ';' (line 2, col 15)
[10] IDENTIFIER: 'println' (line 3, col 5)
[11] BANG: '!' (line 3, col 12)
[12] LPAREN: '(' (line 3, col 13)
[13] STRING_LITERAL: 'Hello, world!' (line 3, col 14)
[14] RPAREN: ')' (line 3, col 29)
[15] SEMICOLON: ';' (line 3, col 30)
[16] RBRACE: '}' (line 4, col 1)
[17] EOF (line 4, col 2)
```

## Next Steps

1. **Implement Parser**: Create AST nodes and parsing logic
2. **Add Type System**: Implement Rust's type checking
3. **Code Generation**: Generate assembly or LLVM IR
4. **Standard Library**: Basic Rust std functions
5. **Error Handling**: Better error messages and recovery

## Contributing

This is a learning project! Feel free to contribute ideas, code, or just follow along as we build a Rust compiler from scratch in C.

## License

This project is open source. See LICENSE file for details.

---

*"Taking my hatred for Rust to a new level" - Building a Rust compiler in C because why not?* 🦀⚡
