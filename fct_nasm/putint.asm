
section .text
global my_putint

my_putint:
    push rbp
    mov rbp, rsp
    sub rsp, 16

    mov rax, rdi ; stockage du paramètre
    mov r9, 15 ; compteur
    mov r10, 10 ; division par 10

boucle:
    cmp rax, 0
    jg sauvegarde
    jle affichage

sauvegarde:
    mov rdx, 0
    idiv r10
    mov [rsp + r9], dl + '0' ; sauvegarde du chiffre
    dec r9
    jmp boucle


affichage:
    mov r11, 15
    sub r11, r9
    mov rax, 1 
    mov rdi, 1
    mov rsi, [rsp + r9]
    mov rdx, r11
    syscall
    mov rsp, rbp
    pop rbp
    ret