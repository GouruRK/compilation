global putint
extern putchar
section .text

; rax: int à afficher 
; rbx: diviseur
; rdx: reste de la division
putint:
    ; convention d'appel AMD64
    push    rbp
    mov     rbp, rsp

    mov     rax, rdi        ; sauvegarde de l'entier à afficher
    mov     rbx, 10         ; diviseur

loop:
    cmp     rax, 0
    je      exit

    mov     rdx, 0          ; initialise le reste

    idiv    rbx             ; rax = rax // rbx, rdx = rax % rbx
    
    push    rax

    add     rdx, '0'        ; rdx += '0'
    mov     rdi, rdx        ; on affiche rax % 10
    call    putchar 

    pop     rax
    jmp     loop

exit:
    mov     rsp, rbp        ; restore pile
    pop     rbp
    ret
