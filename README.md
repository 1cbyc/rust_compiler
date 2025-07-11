# rust compiler in c

a rust compiler implementation written in c, because why not take the challenge to the next level?

## project status

### completed
- **lexer**: complete tokenization of rust syntax
  - all rust keywords (`fn`, `let`, `mut`, `if`, `else`, `loop`, `while`, `for`, `in`, `match`, `return`, `break`, `continue`, `struct`, `enum`, `impl`, `trait`, `use`, `pub`, `crate`, `mod`, `as`, `where`, `type`, `const`, `static`, `unsafe`, `extern`, `move`, `ref`, `box`, `self`, `super`, `true`, `false`)
  - all operators (`+`, `-`, `*`, `/`, `%`, `=`, `==`, `!=`, `<`, `>`, `<=`, `>=`, `+=`, `-=`, `*=`, `/=`, `%=`, `&&`, `||`, `->`, `=>`, `<<`, `>>`, `&`, `|`, `^`, `!`, `~`)
  - all delimiters (`()`, `{}`, `[]`, `;`, `:`, `,`, `.`, `@`, `#`, `$`, `?`, `_`)
  - literals (integers, floats, strings, characters)
  - comments (single-line `//` and multi-line `/* */`)
  - proper line and column tracking for error reporting

- **parser**: complete abstract syntax tree generation
  - ast node structures defined for all rust constructs
  - complete recursive descent parser implemented
  - can parse all major rust syntax constructs:
    - variable declarations and expressions
    - function declarations and calls
    - control flow (if/else, while, for, return)
    - struct and enum definitions
    - impl blocks
    - operator precedence parsing
    - comprehensive error handling

- **type system**: complete type checking and inference
  - type representation and inference implemented
  - type checking for variables, functions, structs, enums
  - generic type support and trait bounds
  - comprehensive error reporting for type mismatches

- **semantic analysis**: complete symbol table and scope management
  - name resolution and scope management
  - semantic checks for variable/function redefinition, shadowing, and undefined references
  - ir (intermediate representation) construction from ast
  - borrow checker (basic) and ownership rules enforcement

- **code generation**: complete x86-64 assembly generation
  - code generation infrastructure targeting x86-64 assembly
  - ir-to-assembly translation for variables, constants, binary operations, and function calls
  - integration with main pipeline (lexer → parser → type checker → semantic analysis → code generation)

- **standard library**: complete rust-like standard library
  - i/o functions (print, println)
  - string manipulation functions (len, concat)
  - collection functions (Vec, push, get)
  - error handling types (Result, Option)
  - basic macro system with expansion capabilities

- **optimization**: complete compiler optimizations
  - constant folding
  - dead code elimination
  - basic inlining
  - loop optimizations
  - register allocation improvements

- **error handling & debugging**: complete error management
  - comprehensive error messages and recovery mechanisms
  - debug information generation
  - stack traces and source location tracking
  - error reporting integrated across all compiler phases

- **testing & documentation**: complete test suite and documentation
  - comprehensive unit tests for each component
  - integration tests for full pipeline
  - performance benchmarks
  - detailed documentation and user guide

- **advanced features**: complete advanced rust features
  - closures with environment management and capture mechanisms
  - iterators with adaptors and consumers
  - async/await with futures and runtime
  - unsafe blocks with safety checks
  - foreign function interface with calling conventions
  - cargo-like package management with dependency resolution

### planned
- **advanced optimizations**: advanced compiler optimizations
- **linker integration**: complete linking and executable generation
- **cross-platform support**: support for multiple target platforms
- **ide integration**: language server and ide support

## project structure

```
rust_compiler/
├── src/
│   ├── lexer.h          # lexer header with token definitions
│   ├── lexer.c          # lexer implementation
│   ├── parser.h         # parser header with ast definitions
│   ├── parser.c         # parser implementation
│   ├── types.h          # type system header
│   ├── types.c          # type system implementation
│   ├── semantic.h       # semantic analysis header
│   ├── semantic.c       # semantic analysis implementation
│   ├── codegen.h        # code generation header
│   ├── codegen.c        # code generation implementation
│   ├── stdlib.h         # standard library header
│   ├── stdlib.c         # standard library implementation
│   ├── optimizer.h      # optimization header
│   ├── optimizer.c      # optimization implementation
│   ├── error.h          # error handling header
│   ├── error.c          # error handling implementation
│   ├── closures.h       # closures header
│   ├── closures.c       # closures implementation
│   ├── iterators.h      # iterators header
│   ├── iterators.c      # iterators implementation
│   ├── async.h          # async/await header
│   ├── async.c          # async/await implementation
│   ├── unsafe.h         # unsafe blocks header
│   ├── unsafe.c         # unsafe blocks implementation
│   ├── ffi.h            # foreign function interface header
│   ├── ffi.c            # foreign function interface implementation
│   ├── package.h        # package management header
│   ├── package.c        # package management implementation
│   └── main.c           # main program and tests
├── include/
│   ├── closures.h       # closures header
│   ├── iterators.h      # iterators header
│   ├── async.h          # async/await header
│   ├── unsafe.h         # unsafe blocks header
│   ├── ffi.h            # foreign function interface header
│   └── package.h        # package management header
├── test_*.py           # python test scripts for each component
├── build.bat            # windows build script
├── makefile             # unix/linux build system
├── ROADMAP.md           # development roadmap
└── readme.md            # this file
```

## building and testing

### windows
```bash
# build the project
build.bat

# or manually with available compiler
gcc -wall -wextra -std=c99 -g src/*.c -o rust_compiler.exe
```

### unix/linux
```bash
# build the project
make

# run tests
make test

# clean build artifacts
make clean
```

### python test (no c compiler needed)
```bash
python test_lexer.py
python test_parser.py
python test_types.py
python test_semantic.py
python test_codegen.py
python test_stdlib.py
python test_optimizer.py
python test_error.py
python test_closures.py
python test_iterators.py
python test_async.py
python test_unsafe.py
python test_ffi.py
python test_package.py
```

## example output

the lexer correctly tokenizes rust code:

```rust
fn main() {
    let x = 42;
    println!("hello, world!");
}
```

produces tokens:
```
[0] fn: 'fn' (line 1, col 1)
[1] identifier: 'main' (line 1, col 4)
[2] lparen: '(' (line 1, col 8)
[3] rparen: ')' (line 1, col 9)
[4] lbrace: '{' (line 1, col 11)
[5] let: 'let' (line 2, col 5)
[6] identifier: 'x' (line 2, col 9)
[7] assign: '=' (line 2, col 11)
[8] integer_literal: '42' (line 2, col 13)
[9] semicolon: ';' (line 2, col 15)
[10] identifier: 'println' (line 3, col 5)
[11] bang: '!' (line 3, col 12)
[12] lparen: '(' (line 3, col 13)
[13] string_literal: 'hello, world!' (line 3, col 14)
[14] rparen: ')' (line 3, col 29)
[15] semicolon: ';' (line 3, col 30)
[16] rbrace: '}' (line 4, col 1)
[17] eof (line 4, col 2)
```

the parser creates an abstract syntax tree:

```
program (1 statements)
  function 'main' (0 parameters)
    block (2 statements)
      variable_decl 'x' (immutable)
        literal '42'
      expression 'println' (1 arguments)
        literal 'hello, world!'
```

## advanced features

### closures
```rust
let add = |x, y| x + y;
let result = add(5, 3);
```

### iterators
```rust
let numbers = vec![1, 2, 3, 4, 5];
let doubled: Vec<i32> = numbers.iter().map(|x| x * 2).collect();
```

### async/await
```rust
async fn fetch_data() -> String {
    // async operation
    "data".to_string()
}

async fn main() {
    let data = await fetch_data();
}
```

### unsafe blocks
```rust
unsafe {
    let ptr = std::ptr::null_mut();
    // unsafe operations
}
```

### foreign function interface
```rust
extern "C" {
    fn printf(format: *const i8, ...) -> i32;
}
```

### package management
```bash
cargo new my_project
cargo add serde
cargo build
cargo test
cargo run
```

## next steps

1. **advanced optimizations**: implement advanced compiler optimizations
2. **linker integration**: complete linking and executable generation
3. **cross-platform support**: support for multiple target platforms
4. **ide integration**: language server and ide support

## contributing

this is a learning project! feel free to contribute ideas, code, or just follow along as i build a rust compiler from scratch in c.

## license

this project is open source. see license file for details.

---

*taking my hatred for rust to a new level - building a rust compiler in c because why not?*
