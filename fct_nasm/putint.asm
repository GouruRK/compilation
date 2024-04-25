global putint
extern putchar
section .text

; rax: int à afficher 
; rbx: diviseur
; rdx: reste de la division
; r12: length
putint:
    ; convention d'appel AMD64
    push    rbp
    mov     rbp, rsp

    mov     rax, rdi        ; sauvegarde de l'entier à afficher
    mov     rbx, 10         ; diviseur
    mov     r12, 0

loop:
    cmp     rax, 0
    je      print

    mov     rdx, 0          ; initialise le reste

    idiv    rbx             ; rax = rax // rbx, rdx = rax % rbx
    
    add     rdx, '0'        ; rdx += '0'
    
    push    rdx

    inc     r12

    jmp     loop

print:

    cmp     r12, 0
    je      exit

    pop     rdi
    call    putchar

    dec     r12
    jmp     print

exit:
    mov     rsp, rbp        ; restore pile
    pop     rbp
    ret