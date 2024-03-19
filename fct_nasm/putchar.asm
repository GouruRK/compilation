section .text
global my_putchar

my_putchar:
    push rdi
    mov rsi, rsp
    mov rdx, 1
    mov rax, 1
    mov rdi, 1
    syscall
    pop rdi
    ret