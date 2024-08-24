# Compilation

Transpiler made with [Rayancris](https://github.com/Rayancris) for the graduation year of our bachelor's degree.

Explore the world of lexical analysis with ASTs, semantic  analysis with type checker and compilation.

___

- [Compilation](#compilation)
  - [Features](#features)
  - [Download](#download)
  - [Run and compile](#run-and-compile)
  - [Example](#example)
  - [Tests](#tests)

## Features
* **Lexical analysis** using Flex
* **Synataxical analysis** using Yacc (Bison)
* Custom **semantic analysis** with symbol table and type checkers
* **I/O interactions** written in Assembly
* **Transpile** TPC into Assembly

For further explanation of the TPC language, see [this file](./doc/tpc.md).

## Download

You can simply donwload this repository using git commands

```bash
git clone https://github.com/GouruRK/compilation
cd compilation
```

## Run and compile

To compile, use the make command which produces `bin/tpcc`

```bash
make
```

Once compiled, use `./bin/tpcc --help` to display how to use it

```
Usage: ./tpcc [OPTION...] FILE
Check if syntax of given file is valid, according to the grammar defined in parser.y

With no FILE, FILE is the standard input

  -t, --tree            print abstract tree of the given file
  -s, --symbols         print associated symbol tables
  -h, --help            display this help message and exit
```

## Example

Take for example the following TPC file `example.tpc`.

```c
int main(void) {
  return 5;
}
```

The command `./bin/tpcc example.tpc` produces an assembly file called `example.asm` which can be compiled as so :


```bash
nasm -f elf64 -o example.o example.asm
gcc -o example example.o -nostartfiles -no-pie
```
to produce the executable `./example`

## Tests

A test collection of more than 430 tests have been created. This contains tests from friends on the same project.

To run them, just use `make test`
