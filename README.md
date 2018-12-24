# cminus

[![Build Status](https://travis-ci.com/thelink2012/cminus.svg?token=p5i5VQWjFHymk5FZa937&branch=master)](https://travis-ci.com/thelink2012/cminus)

This is a compiler project for MATA61 at UFBA (Compilers). The compiler was built for the 2018.1 coursework and was subsequently used by the professor as an oracle to validate other students submissions.

The compiler translates a language called cminus (specified [here](https://thelink2012.xyz/mata61/MATA61%20Compiladores%20-%20Projeto%20do%20Compilador.html)) into MIPS assembly. The generated code is fully compatible with the O32 ABI, thus (theoretically) it can be used by foreign functions in some MIPS machines.

Translation occurs in two passes. The first pass performs syntax-directed translation to construct an AST. The second pass visits this AST spitting MIPS assembly.

## Building

There are no dependencies beyond a C++17 compiler.

```
make
```

## Usage

To translate a source file into MIPS:

```
./geracodigo source.in target.s
```

You may as well use `./lexico` and `./sintatico` to inspect the scanner and the abstract syntax tree.

```
./lexico source.in -
./sintatico source.in -
```

Unfortunately the diagnostic system is incomplete and there are no indication of failure other than a non-zero exit code.
