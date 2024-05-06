; registre d'argument de fonction : rdi
putchar:
    ; convention d'appel AMD64
    push    rbp
    mov     rbp, rsp
    
    ; on met sur la pile le caractère a afficher
    push rdi

    ; Appel systeme d'affichage
    mov rdi, 1      ; stdout
    mov rax, 1      ; stdout
    mov rsi, rsp    ; charactere a afficher
    mov rdx, 1      ; taille de la variable à écrire
    syscall         ; appel à write

    ; restauration de la pile
    mov rsp, rbp
    pop rbp
    
    ret
