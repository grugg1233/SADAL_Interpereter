# SADAL_Interpereter
Built a lexical analyzer, parser, and interpereter for a Simple ADA like language. Project created using C++. Part of course work.
# SADAL Interpreter

A recursive-descent interpreter for the Simple Ada-Like (SADAL) language, implemented in C++. This project includes a lexer, parser, and interpreter capable of processing SADAL programs with basic control structures, arithmetic operations, and I/O statements.

## Features
- **Lexical Analysis**: Tokenizes SADAL source code, handling identifiers, keywords, numbers, operators, and punctuation.
- **Parsing**: Recursive-descent parser implementing SADAL grammar, including:
  - Procedure declarations and bodies
  - Variable declarations and types
  - Control structures (`if`, `while`, `for`)
  - Arithmetic and string operations
  - Input/Output statements
- **Interpretation**: Executes parsed SADAL programs, maintaining symbol tables for variables and procedures.
- **Error Handling**: Reports lexical, syntactic, and semantic errors with line numbers for easier debugging.

## Getting Started

### Prerequisites
- C++11/13 compatible compiler (e.g., `g++`)

### Installation & Compilation

1. **Clone the repository**:
   ```bash
   git clone https://github.com/grugg1233/SADAL_Interpereter.git
   cd SADAL-Interpreter
