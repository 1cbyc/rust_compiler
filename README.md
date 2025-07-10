# rust compiler in c

A rust compiler implementation written in c, because why not take the challenge to the next level?

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

### planned
- **type checker**: implement rust's type system
- **code generator**: generate machine code or llvm ir
- **standard library**: basic rust std library functions
- **error handling**: comprehensive error messages
- **optimizations**: basic compiler optimizations

## project structure

```
rust_compiler/
├── src/
│   ├── lexer.h          # lexer header with token definitions
│   ├── lexer.c          # lexer implementation
│   ├── parser.h         # parser header with ast definitions
│   ├── parser.c         # parser implementation
│   └── main.c           # main program and tests
├── test_lexer.py        # python test script for lexer
├── build.bat            # windows build script
├── makefile             # unix/linux build system
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

## next steps

1. **implement type system**: add rust's type checking and inference
2. **code generation**: generate assembly or llvm ir
3. **standard library**: basic rust std functions
4. **error handling**: better error messages and recovery

## contributing

this is a learning project! feel free to contribute ideas, code, or just follow along as we build a rust compiler from scratch in c.

## license

this project is open source. see license file for details.

---

*taking my hatred for rust to a new level - building a rust compiler in c because why not?*
