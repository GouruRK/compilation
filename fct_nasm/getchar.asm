global getchar
section .text
; registre de retour de fonction : rax
getchar:
    ; convention d'appel AMD 64
    push    rbp
    mov     rbp, rsp       
    
    sub     rsp, 8
    mov     qword [rsp], 0

    mov     rax, 0          ; read
    mov     rdi, 0          ; stdin
    mov     rsi, rsp        ; adresse de retour
    mov     rdx, 1          ; taille de la lecture
    syscall
    
    pop     rax

    mov     rsp, rbp
    pop     rbp
    ret
