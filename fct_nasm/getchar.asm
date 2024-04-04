section .text
global my_getchar

; registre de retour de fonction : rax
my_getchar:
    ; convention d'appel nasm 64
    push rbp
    mov rbp, rsp
    
    mov rsi, rsp ; on met à rsi l'adresse destination où sera stockée le caractère
    mov rdx, 1 ; taille du caractère
    mov rax, 0 ; choix de read
    mov rdi, 0 ; entrée standard
    syscall ; appel à read

    mov al, byte [rsp] ; renvoie du caractère

    mov rsp, rbp ; restauration de la pile
    pop rbp
    ret