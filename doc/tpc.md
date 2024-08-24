Description of TPC

- [Types](#types)
- [Variable and function](#variable-and-function)
  - [Arrays](#arrays)
  - [Built-in](#built-in)
- [Operators](#operators)
- [Conditionnal structures](#conditionnal-structures)


TPC is a sub language of C. A TPC program must be contains in a `.tpc` file and is a row/sequel of functions.

The program *must* include a `main` function that returns a int and take no parameters.

```c
int main(void) {
    return 0;
}
```
*basic TPC program*

## Types

The two types of the language are `int` and `char`, and their variants as static arrays. The `void` key-word can be used for a function which does not take any parameters or a function which does not return any value.

## Variable and function

A function is divided in two parts : one to declare local variables and the function's body. Before creating functions, global variables can be declared.

A global variable cannot have the same name as a function, and a local variable cannot have the same name as a parameter.

```c
int var1, var2, array1[20];

int function(int integer, char character, char array[]) {
    int a;
    int b;

    // instructions there

    return 0;
}
```
*correct TPC programm with variable declaration*

### Arrays

Array are statics and only in one-dimension. When created, their size must be known and being a non-zero integer

> [!CAUTION]
> The following syntax is therefore forbidden
> ```c
> int array[];
> int array[0];
> int array[2 - 1];
> ```

However, when giving an array as a parameter, the size must be left blank

```c
int array[20];

void function(char arr[]) {
    int a[1];

    a[0] = 5;
    return a[0];
}
```
*Correct way to declare and use arrays in TPC*

### Built-in

Some functions are already provided and does not require the user to code them. Theses functions are I/O interactions and have the following signature :

```c
char getchar(void);
int getint(void);
void putchar(char);
void putint(int);
```

> [!NOTE]
> Note that declaring functions as so is invalid in TPC. There is no way to declare functions signature yet

No function or global variable can have the same name as theses functions.

## Operators

All TPC operator are the same as in the C language

Operations:
* `+`, `-`, `/`, `*`, `%`, `&&`, `||`
* `+`, `-` can also be used as unary operator
* `!` is an unary operator

Compares:
* `==`, `!=`, `>`, `<`, `<=`, `>=`

## Conditionnal structures

The conditionnal structures are the `if`, `if/else` and `while` statement and must used the following syntax

```c
if (condition) {
    // if true
}

if (condition) {
    // if true
} else {
    // if false
}

while (condition) {
    // while true
}
```

> [!WARNING]
> Unlike C, array's address cannot be used as a boolean expression
