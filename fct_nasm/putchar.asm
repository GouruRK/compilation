global putchar
section .text


; registre d'argument de fonction : rdi
putchar:
    ; convention d'appel AMD64
    push rbp
    mov rbp, rsp
    
    push rdi        ; on met sur la pile le caractère à afficher

    mov rsi, rsp    ; on met à rsi l'adresse source où est stockée le caractère
    mov rdx, 1      ; taille de la variable à écrire
    mov rax, 1      ; choix de la fonction write
    mov rdi, 1      ; stdout
    syscall         ; appel à write

    mov rsp, rbp    ; restauration de la pile
    pop rbp
    
    ret
