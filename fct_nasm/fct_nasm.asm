section .text
global _start
extern putchar
extern getchar
extern putint
extern getint

_start:
    mov rax, 60
    mov rdi, 0
    syscall