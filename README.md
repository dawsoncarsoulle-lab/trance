# Facile Compiler

A lightweight compiler written in C that translates the Facile programming language into Common Intermediate Language (CIL) for the .NET/Mono runtime.

## Architecture

* **Frontend:** Flex (`facile.lex`) for lexical analysis and Bison (`facile.y`) for parsing.
* **Intermediate Representation:** Custom Abstract Syntax Tree (AST).
* **Backend:** Stack-based CIL code generation (`src/cil/`).

## Dependencies

* GCC (or any ANSI C compiler)
* Flex
* Bison
* Make

## Build Instructions

Compile the project from the src directory:

```bash
make
```

This generates the compiler executable at `build/facile`.

## Usage

Pass a `.facile` source file to the compiler to generate a `.il` CIL file:

```bash
./build/facile path/to/source.facile
```

## Contributing

To use the project's formatting, run the pre-commit hooks found in the scripts from the root of the project:

```bash
make
```
