#!/bin/bash

echo "Building Rust Compiler in C..."

# Check for different C compilers
if command -v gcc &> /dev/null; then
    echo "Using GCC..."
    gcc -Wall -Wextra -std=c99 -g src/*.c -o rust_compiler
elif command -v clang &> /dev/null; then
    echo "Using Clang..."
    clang -Wall -Wextra -std=c99 -g src/*.c -o rust_compiler
elif command -v tcc &> /dev/null; then
    echo "Using TinyCC..."
    tcc -Wall -g src/*.c -o rust_compiler
else
    echo "No C compiler found!"
    echo "Please install one of the following:"
    echo "- GCC"
    echo "- Clang"
    echo "- TinyCC"
    exit 1
fi

if [ -f rust_compiler ]; then
    echo "Build successful! Run with: ./rust_compiler"
else
    echo "Build failed!"
    exit 1
fi 