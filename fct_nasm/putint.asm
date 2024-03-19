section .text
global my_putint

my_putint:
    add rdi, "0"
    push rdi
    mov rsi, rsp
    mov rdx, 1
    mov rax, 1
    mov rdi, 1
    syscall
    pop rdi
    ret