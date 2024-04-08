global putchar
section .text


; registre d'argument de fonction : rdi
putchar:
    mov rbp, rdi ; déplacement de la valeur d'entrée dans rbp

    ; convention d'appel nasm 64
    push rbp
    mov rbp, rsp
    
    mov rsi, rsp ; on met à rsi l'adresse source où est stockée le caractère
    mov rdx, 1 ; taille de la variable à écrire
    mov rax, 1 ; choix de la fonction write
    mov rdi, 1 ; sortie standard
    syscall ; appel à write

    mov rsp, rbp ; restauration de la pile
    pop rbp
    ret