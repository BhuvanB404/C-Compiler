# Bboop Compiler

A minimalist compiler for a simple C-like language with basic arithmetic, variables, and function calls.

## Language Overview
- **Variables:** `auto x;` (local), `global y;` (global)
- **Assignment:** `x = 5;`
- **Arithmetic:** `+`, `-`, `*`, `/` (e.g., `y = x + 2;`)
- **Function Calls:** `add(x);`

## Building

1. **Clone or download this repository.**
2. **Build using Makefile:**
   ```sh
   make
   ```
   This will produce the `compiler` executable in the project directory.

## Usage

To compile a source file and print the IR:
```sh
./compiler yourfile.b --print-ir
```

To compile and generate assembly:
```sh
./compiler yourfile.b > yourfile.asm
```

### Run Tests
To run the provided tests and print their IR:
```sh
make test
```
This will build the compiler and run it on the test files, printing the IR output for each test.

## File Structure
- `main.cpp` / `main.h` - Entry point and IR definitions
- `ir.cpp` - IR generation and optimizations
- `generator.cpp` - Assembly code generation
- `Parser.cpp` / `Parser.h` - Parser for the language
- `Tokenizer.cpp` / `Tokenizer.h` - Tokenizer/lexer
- `Makefile` - Build instructions
- `tests/` - Example and test programs

## Example Program
```
auto x;
x = 10;
auto y;
y = x + 5;
print(y);
```

---

## IR 
- Implemented three address code IR.
- Workflow: Tokenize -> Parse -> AST -> IR -> Assembly -> Linker ->.out
- Current optimizations:
  - Constant folding
  - Constant propagation


