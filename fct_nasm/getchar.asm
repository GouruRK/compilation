section .text
global my_getchar
my_getchar:
    push rdi
    mov rsi, rsp
    mov rdx, 1
    mov rax, 0
    mov rdi, 0
    syscall
    mov rax, rsp
    pop rdi
    ret