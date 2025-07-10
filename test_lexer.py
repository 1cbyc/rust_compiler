#!/usr/bin/env python3
"""
Test script for the Rust lexer logic
This simulates the C lexer behavior in Python for testing
"""

import re
from enum import Enum
from typing import List, Tuple, Optional

class TokenType(Enum):
    # Literals
    IDENTIFIER = "IDENTIFIER"
    INTEGER_LITERAL = "INTEGER_LITERAL"
    FLOAT_LITERAL = "FLOAT_LITERAL"
    STRING_LITERAL = "STRING_LITERAL"
    CHAR_LITERAL = "CHAR_LITERAL"
    BOOL_LITERAL = "BOOL_LITERAL"
    
    # Keywords
    FN = "FN"
    LET = "LET"
    MUT = "MUT"
    IF = "IF"
    ELSE = "ELSE"
    LOOP = "LOOP"
    WHILE = "WHILE"
    FOR = "FOR"
    IN = "IN"
    MATCH = "MATCH"
    RETURN = "RETURN"
    BREAK = "BREAK"
    CONTINUE = "CONTINUE"
    STRUCT = "STRUCT"
    ENUM = "ENUM"
    IMPL = "IMPL"
    TRAIT = "TRAIT"
    USE = "USE"
    PUB = "PUB"
    CRATE = "CRATE"
    MOD = "MOD"
    AS = "AS"
    WHERE = "WHERE"
    TYPE = "TYPE"
    CONST = "CONST"
    STATIC = "STATIC"
    UNSAFE = "UNSAFE"
    EXTERN = "EXTERN"
    MOVE = "MOVE"
    REF = "REF"
    BOX = "BOX"
    SELF = "SELF"
    SUPER = "SUPER"
    TRUE = "TRUE"
    FALSE = "FALSE"
    
    # Operators
    PLUS = "PLUS"
    MINUS = "MINUS"
    STAR = "STAR"
    SLASH = "SLASH"
    PERCENT = "PERCENT"
    ASSIGN = "ASSIGN"
    PLUS_ASSIGN = "PLUS_ASSIGN"
    MINUS_ASSIGN = "MINUS_ASSIGN"
    STAR_ASSIGN = "STAR_ASSIGN"
    SLASH_ASSIGN = "SLASH_ASSIGN"
    PERCENT_ASSIGN = "PERCENT_ASSIGN"
    EQ = "EQ"
    NE = "NE"
    LT = "LT"
    GT = "GT"
    LE = "LE"
    GE = "GE"
    AND = "AND"
    OR = "OR"
    NOT = "NOT"
    AND_AND = "AND_AND"
    OR_OR = "OR_OR"
    XOR = "XOR"
    SHL = "SHL"
    SHR = "SHR"
    
    # Delimiters
    LPAREN = "LPAREN"
    RPAREN = "RPAREN"
    LBRACE = "LBRACE"
    RBRACE = "RBRACE"
    LBRACKET = "LBRACKET"
    RBRACKET = "RBRACKET"
    SEMICOLON = "SEMICOLON"
    COLON = "COLON"
    COMMA = "COMMA"
    DOT = "DOT"
    ARROW = "ARROW"
    FAT_ARROW = "FAT_ARROW"
    AT = "AT"
    HASH = "HASH"
    DOLLAR = "DOLLAR"
    QUESTION = "QUESTION"
    BANG = "BANG"
    TILDE = "TILDE"
    UNDERSCORE = "UNDERSCORE"
    
    # Special
    EOF = "EOF"
    ERROR = "ERROR"

class Token:
    def __init__(self, token_type: TokenType, value: str, line: int, column: int):
        self.type = token_type
        self.value = value
        self.line = line
        self.column = column
    
    def __str__(self):
        result = f"{self.type.value}"
        if self.value:
            result += f": '{self.value}'"
        result += f" (line {self.line}, col {self.column})"
        return result

class PythonLexer:
    def __init__(self, source: str):
        self.source = source
        self.position = 0
        self.line = 1
        self.column = 1
        
        # Keywords mapping
        self.keywords = {
            "fn": TokenType.FN,
            "let": TokenType.LET,
            "mut": TokenType.MUT,
            "if": TokenType.IF,
            "else": TokenType.ELSE,
            "loop": TokenType.LOOP,
            "while": TokenType.WHILE,
            "for": TokenType.FOR,
            "in": TokenType.IN,
            "match": TokenType.MATCH,
            "return": TokenType.RETURN,
            "break": TokenType.BREAK,
            "continue": TokenType.CONTINUE,
            "struct": TokenType.STRUCT,
            "enum": TokenType.ENUM,
            "impl": TokenType.IMPL,
            "trait": TokenType.TRAIT,
            "use": TokenType.USE,
            "pub": TokenType.PUB,
            "crate": TokenType.CRATE,
            "mod": TokenType.MOD,
            "as": TokenType.AS,
            "where": TokenType.WHERE,
            "type": TokenType.TYPE,
            "const": TokenType.CONST,
            "static": TokenType.STATIC,
            "unsafe": TokenType.UNSAFE,
            "extern": TokenType.EXTERN,
            "move": TokenType.MOVE,
            "ref": TokenType.REF,
            "box": TokenType.BOX,
            "self": TokenType.SELF,
            "super": TokenType.SUPER,
            "true": TokenType.TRUE,
            "false": TokenType.FALSE,
        }
    
    def peek(self) -> str:
        if self.position >= len(self.source):
            return '\0'
        return self.source[self.position]
    
    def peek_next(self) -> str:
        if self.position + 1 >= len(self.source):
            return '\0'
        return self.source[self.position + 1]
    
    def advance(self) -> str:
        if self.position >= len(self.source):
            return '\0'
        
        c = self.source[self.position]
        if c == '\n':
            self.line += 1
            self.column = 1
        else:
            self.column += 1
        self.position += 1
        return c
    
    def skip_whitespace(self):
        while self.peek() and self.peek().isspace():
            self.advance()
    
    def skip_comment(self):
        if self.peek() == '/' and self.peek_next() == '/':
            # Single line comment
            while self.peek() and self.peek() != '\n':
                self.advance()
        elif self.peek() == '/' and self.peek_next() == '*':
            # Multi-line comment
            self.advance()  # consume '/'
            self.advance()  # consume '*'
            while self.peek():
                if self.peek() == '*' and self.peek_next() == '/':
                    self.advance()  # consume '*'
                    self.advance()  # consume '/'
                    break
                self.advance()
    
    def read_identifier(self) -> Token:
        start_line = self.line
        start_column = self.column
        start_pos = self.position
        
        while self.peek() and (self.peek().isalnum() or self.peek() == '_'):
            self.advance()
        
        value = self.source[start_pos:self.position]
        
        # Check if it's a keyword
        if value in self.keywords:
            return Token(self.keywords[value], value, start_line, start_column)
        
        return Token(TokenType.IDENTIFIER, value, start_line, start_column)
    
    def read_number(self) -> Token:
        start_line = self.line
        start_column = self.column
        start_pos = self.position
        is_float = False
        
        # Read integer part
        while self.peek() and self.peek().isdigit():
            self.advance()
        
        # Check for decimal point
        if self.peek() == '.':
            is_float = True
            self.advance()
            
            # Read fractional part
            while self.peek() and self.peek().isdigit():
                self.advance()
        
        # Check for exponent
        if self.peek() in ['e', 'E']:
            is_float = True
            self.advance()
            
            # Optional sign
            if self.peek() in ['+', '-']:
                self.advance()
            
            # Exponent digits
            while self.peek() and self.peek().isdigit():
                self.advance()
        
        value = self.source[start_pos:self.position]
        token_type = TokenType.FLOAT_LITERAL if is_float else TokenType.INTEGER_LITERAL
        return Token(token_type, value, start_line, start_column)
    
    def read_string(self) -> Token:
        start_line = self.line
        start_column = self.column
        
        self.advance()  # consume opening quote
        
        start_pos = self.position
        while self.peek() and self.peek() != '"':
            if self.peek() == '\\':
                self.advance()  # consume backslash
            self.advance()
        
        if not self.peek():
            return Token(TokenType.ERROR, "Unterminated string", start_line, start_column)
        
        value = self.source[start_pos:self.position]
        self.advance()  # consume closing quote
        
        return Token(TokenType.STRING_LITERAL, value, start_line, start_column)
    
    def get_next_token(self) -> Token:
        self.skip_whitespace()
        self.skip_comment()
        self.skip_whitespace()
        
        if not self.peek() or self.peek() == '\0':
            return Token(TokenType.EOF, None, self.line, self.column)
        
        c = self.peek()
        current_line = self.line
        current_column = self.column
        
        # Identifiers and keywords
        if c.isalpha() or c == '_':
            return self.read_identifier()
        
        # Numbers
        if c.isdigit():
            return self.read_number()
        
        # Strings
        if c == '"':
            return self.read_string()
        
        # Two-character operators
        if c == '=' and self.peek_next() == '=':
            self.advance()
            self.advance()
            return Token(TokenType.EQ, "==", current_line, current_column)
        if c == '!' and self.peek_next() == '=':
            self.advance()
            self.advance()
            return Token(TokenType.NE, "!=", current_line, current_column)
        if c == '<' and self.peek_next() == '=':
            self.advance()
            self.advance()
            return Token(TokenType.LE, "<=", current_line, current_column)
        if c == '>' and self.peek_next() == '=':
            self.advance()
            self.advance()
            return Token(TokenType.GE, ">=", current_line, current_column)
        if c == '+' and self.peek_next() == '=':
            self.advance()
            self.advance()
            return Token(TokenType.PLUS_ASSIGN, "+=", current_line, current_column)
        if c == '-' and self.peek_next() == '=':
            self.advance()
            self.advance()
            return Token(TokenType.MINUS_ASSIGN, "-=", current_line, current_column)
        if c == '*' and self.peek_next() == '=':
            self.advance()
            self.advance()
            return Token(TokenType.STAR_ASSIGN, "*=", current_line, current_column)
        if c == '/' and self.peek_next() == '=':
            self.advance()
            self.advance()
            return Token(TokenType.SLASH_ASSIGN, "/=", current_line, current_column)
        if c == '%' and self.peek_next() == '=':
            self.advance()
            self.advance()
            return Token(TokenType.PERCENT_ASSIGN, "%=", current_line, current_column)
        if c == '&' and self.peek_next() == '&':
            self.advance()
            self.advance()
            return Token(TokenType.AND_AND, "&&", current_line, current_column)
        if c == '|' and self.peek_next() == '|':
            self.advance()
            self.advance()
            return Token(TokenType.OR_OR, "||", current_line, current_column)
        if c == '-' and self.peek_next() == '>':
            self.advance()
            self.advance()
            return Token(TokenType.ARROW, "->", current_line, current_column)
        if c == '=' and self.peek_next() == '>':
            self.advance()
            self.advance()
            return Token(TokenType.FAT_ARROW, "=>", current_line, current_column)
        if c == '<' and self.peek_next() == '<':
            self.advance()
            self.advance()
            return Token(TokenType.SHL, "<<", current_line, current_column)
        if c == '>' and self.peek_next() == '>':
            self.advance()
            self.advance()
            return Token(TokenType.SHR, ">>", current_line, current_column)
        
        # Single character tokens
        self.advance()
        
        token_map = {
            '(': TokenType.LPAREN,
            ')': TokenType.RPAREN,
            '{': TokenType.LBRACE,
            '}': TokenType.RBRACE,
            '[': TokenType.LBRACKET,
            ']': TokenType.RBRACKET,
            ';': TokenType.SEMICOLON,
            ':': TokenType.COLON,
            ',': TokenType.COMMA,
            '.': TokenType.DOT,
            '@': TokenType.AT,
            '#': TokenType.HASH,
            '$': TokenType.DOLLAR,
            '?': TokenType.QUESTION,
            '!': TokenType.BANG,
            '~': TokenType.TILDE,
            '_': TokenType.UNDERSCORE,
            '+': TokenType.PLUS,
            '-': TokenType.MINUS,
            '*': TokenType.STAR,
            '/': TokenType.SLASH,
            '%': TokenType.PERCENT,
            '=': TokenType.ASSIGN,
            '<': TokenType.LT,
            '>': TokenType.GT,
            '&': TokenType.AND,
            '|': TokenType.OR,
            '^': TokenType.XOR,
        }
        
        if c in token_map:
            return Token(token_map[c], c, current_line, current_column)
        
        return Token(TokenType.ERROR, c, current_line, current_column)

def test_lexer(source_code: str):
    print("=== Testing Lexer ===")
    print(f"Source code:\n{source_code}\n")
    
    lexer = PythonLexer(source_code)
    tokens = []
    token_count = 0
    
    while True:
        token = lexer.get_next_token()
        tokens.append(token)
        
        print(f"[{token_count}] {token}")
        token_count += 1
        
        if token.type == TokenType.ERROR:
            print(f"Lexer error: {token.value}")
        
        if token.type == TokenType.EOF:
            break
    
    print("\n=== End of Lexer Test ===\n")
    return tokens

def main():
    print("Rust Compiler in C - Lexer Test (Python Simulation)")
    print("===================================================\n")
    
    # Test 1: Simple Rust function
    test1 = """fn main() {
    let x = 42;
    println!("Hello, world!");
}"""
    test_lexer(test1)
    
    # Test 2: More complex Rust code
    test2 = """struct Point {
    x: i32,
    y: i32,
}

impl Point {
    fn new(x: i32, y: i32) -> Self {
        Point { x, y }
    }
}"""
    test_lexer(test2)
    
    # Test 3: Comments and operators
    test3 = """// This is a comment
let mut sum = 0;
for i in 0..10 {
    sum += i; // Add to sum
}"""
    test_lexer(test3)

if __name__ == "__main__":
    main() 