; rax: int à afficher 
; rbx: diviseur
; rdx: reste de la division
; r12: length
putint:
    ; convention d'appel AMD64
    push    rbp
    mov     rbp, rsp

    ; On sauvegarde les registres non volatils
    push    r12

    ; Initialisation des registres
    mov     rax, rdi        ; sauvegarde de l'entier à afficher
    mov     rbx, 10         ; diviseur
    mov     r12, 0

    cmp     rax, 0
    je      print_zero      ; on veut juste afficher 0 et quitter
    jge     loop_label_pint ; si l'entier à afficher est plus grand que 0, on l'affiche directement

    ; manage negatives
    push    rax             ; save rax
    mov     rdi, '-'
    call    putchar         ; print the neg sign
    pop     rax
    neg     rax
    jmp     loop_label_pint

print_zero:
    mov     rdi, '0'
    call    putchar
    jmp     exit_pint

loop_label_pint:
    cmp     rax, 0
    je      print_pint
    mov     rdx, 0          ; initialise le reste
    idiv    rbx             ; rax = rax // rbx, rdx = rax % rbx
    add     rdx, '0'        ; rdx += '0'
    push    rdx
    inc     r12
    jmp     loop_label_pint

print_pint:
    cmp     r12, 0
    je      exit_pint

    pop     rdi
    call    putchar

    dec     r12
    jmp     print_pint

exit_pint:
    pop     r12

    mov     rsp, rbp        ; restore pile
    pop     rbp
    ret
