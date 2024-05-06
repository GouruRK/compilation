; registre de retour de fonction : rax
getchar:
    ; convention d'appel AMD 64
    push    rbp
    mov     rbp, rsp       
    
    ; On réserve dans la pile un octet pour stocker la valeur
    sub     rsp, 8
    mov     qword [rsp], 0

    ; Appel systeme pour récupérer le prochain charactere
    mov     rax, 0          ; read
    mov     rdi, 0          ; stdin
    mov     rsi, rsp        ; adresse de retour
    mov     rdx, 1          ; taille de la lecture
    syscall
    
    ; On met le charactere recupere dans rax
    pop     rax

    ; Restitution de la pile
    mov     rsp, rbp
    pop     rbp
    
    ret
