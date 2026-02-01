# Bboop Compiler

A modern multi-target compiler for a simple C-like language featuring advanced code generation backends and WebAssembly integration with WasmEdge AOT optimization support.

## Language Overview
- **Variables:** `auto x;` (local), `global y;` (global) 
- **Assignment:** `x = 5;`
- **Arithmetic:** `+`, `-`, `*`, `/`, `%` with operator precedence
- **Control Flow:** `if/else`, `while` loops, `return` statements
- **Comparisons:** `==`, `!=`, `<`, `>`, `<=`, `>=`
- **Bitwise Operations:** `<<`, `>>`, `&`, `|`, `^`
- **Function Calls:** `add(x);`, `extern` declarations

## Multi-Target Architecture

The Bboop compiler features a sophisticated plugin-based target system supporting multiple architectures:

### Supported Targets
- **x86_64**: Native Linux x86-64 assembly generation
- **ARM64**: ARM AArch64 assembly generation  
- **ARM32**: ARM 32-bit assembly generation
- **WebAssembly**: Standard WASM module generation
- **WasmEdge AOT**: Optimized WebAssembly with ahead-of-time compilation

### Target Selection
```sh
./compiler -t x86_64 yourfile.b    # Default x86-64 target
./compiler -t arm64 yourfile.b     # ARM 64-bit target
./compiler -t wasm yourfile.b      # Standard WebAssembly
./compiler -t wasmedge yourfile.b  # WasmEdge AOT optimized
./compiler --list-targets          # Show all available targets
```

## WasmEdge Integration & AOT Optimization

### WasmEdge Target Features
The WasmEdge target provides advanced ahead-of-time compilation with several key optimizations:

- **AOT Pre-compilation**: Converts WebAssembly to native machine code at compile time
- **Memory Layout Optimization**: Efficient memory management for AOT execution
- **Runtime Integration**: Seamless integration with WasmEdge runtime environment
- **Performance Benchmarking**: Built-in comparison tools for optimization analysis

### Performance Comparison: AOT vs Runtime Optimization

**Bboop Compiler Optimizations:**
- Constant folding and propagation at IR level
- Dead code elimination 
- Register allocation optimization
- Loop optimization techniques

**WasmEdge AOT Optimizations:**
- LLVM-based backend optimizations
- Native code generation with aggressive inlining
- Runtime profile-guided optimization
- Advanced vectorization and parallelization

### Benchmark Results
```sh
# Run performance comparison
make benchmark
./tests/benchmark_wasmedge_aot.sh
```

The WasmEdge AOT pipeline typically shows 2-5x performance improvement over interpreted WebAssembly, while Bboop's IR-level optimizations provide competitive performance for mathematical workloads.

## Building

1. **Clone or download this repository.**
2. **Build using Makefile:**
   ```sh
   make
   ```
   This will produce the `compiler` executable in the project directory.

3. **Install WasmEdge (optional, for AOT support):**
   ```sh
   # Install WasmEdge runtime
   curl -sSf https://raw.githubusercontent.com/WasmEdge/WasmEdge/master/utils/install.sh | bash
   ```

## Usage

### Basic Compilation
```sh
./compiler yourfile.b              # Compile to x86_64 assembly
./compiler yourfile.b --print-ir   # Print intermediate representation
./compiler -t wasm yourfile.b      # Compile to WebAssembly
```

### Advanced Usage
```sh
./compiler -t wasmedge yourfile.b --asm-only  # Generate WasmEdge-optimized WAT
./compiler --help                             # Show all options
```

### Run Tests
```sh
make test                    # Run all test cases
./tests/benchmark_aot.sh     # Performance benchmarking
```

## Architecture Overview

### File Structure
```
src/
├── main.cpp              # Compiler entry point
├── Parser.cpp/.h         # Language parser
├── Tokenizer.cpp/.h      # Lexical analyzer  
├── ir.cpp                # IR generation & optimization
├── target.cpp            # Target management system
└── codegen/              # Code generation backends
    ├── x86_64_generator.cpp/.h    # x86-64 backend
    ├── arm_generator.cpp/.h       # ARM backend
    ├── wasm_generator.cpp/.h      # WebAssembly backend
    └── wasmedge_generator.cpp/.h  # WasmEdge AOT backend

include/                  # Header files
tests/                   # Test suite and benchmarks
```

### Compilation Pipeline
1. **Tokenization** → Lexical analysis
2. **Parsing** → Abstract Syntax Tree (AST) generation
3. **IR Generation** → Three-address code intermediate representation
4. **Optimization** → Multiple optimization passes
5. **Target Selection** → Backend-specific code generation
6. **Assembly/Binary** → Final executable or WebAssembly module

## Example Program
```c
main() {
    extern exit;
    auto x, y, result;
    
    x = 10;
    y = 20;
    
    if (x < y) {
        result = x * y + 5;
        while (result > 100) {
            result = result - 50;
        }
    }
    
    exit(result);
}
```

---

## Intermediate Representation (IR)

### IR Features
- **Three-address code format** for efficient optimization
- **Static Single Assignment (SSA)** form support
- **Control flow graph** construction
- **Data flow analysis** capabilities

### Current Optimizations
**Bboop Compiler IR Optimizations:**
- Constant folding and propagation
- Dead code elimination
- Common subexpression elimination
- Loop-invariant code motion
- Register allocation with graph coloring

**WasmEdge AOT Pipeline Optimizations:**
- LLVM backend optimizations (O2/O3 level)
- Aggressive function inlining
- Vectorization and loop unrolling  
- Profile-guided optimization (PGO)
- Native code generation with runtime linking

### Performance Analysis
The compiler includes built-in benchmarking tools to compare optimization effectiveness:

```sh
# Compare native vs WasmEdge AOT performance
./tests/benchmark_wasmedge_aot.sh

# Detailed timing breakdown
./tests/benchmark_heavy_detailed.sh
```

**Typical Performance Results:**
- **Native x86_64**: Baseline performance (1.0x)
- **Bboop IR Optimized**: 15-25% improvement over naive codegen
- **WebAssembly (wasmtime)**: 60-80% of native performance
- **WasmEdge AOT**: 85-95% of native performance

## Contributing

The modular architecture makes it easy to add new targets:

1. Create new generator in `src/codegen/your_target_generator.cpp`
2. Implement the `TargetGenerator` interface
3. Register target in `src/target.cpp`
4. Add build rules to `Makefile`

## License

This project is licensed under the MIT License - see the LICENCE file for details.

## Acknowledgments

- **WasmEdge Community**: For the excellent AOT compilation framework
- **LLVM Project**: For optimization techniques and IR design inspiration
- **WebAssembly Specification**: For standardized bytecode format


