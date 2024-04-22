global getint
extern getchar
section .text

; Registres
; r12: flags si negatif
; rax: valeur de retour de getchar
; r13: résultat
getint:
    ; conventions d'appels AMD64 
    push    rbp             ; sauvegarde le pointeur de base
    mov     rbp, rsp        ; rbp = rsp

    call    getchar         ; récupère le premier character

    mov     r13, 0          ; Initialise le résultat
    mov     r12, 0          ; Initialise les flags
    
    cmp     rax, '-'
    jne     convert         ; rax n'est pas '-', c'est donc un nombre. On passe directement à la conversion 
    mov     r12, 1          ; r12 possède le flag 'nombre negatif' 

loop_label:
    call    getchar        ; récupère le prochain character

    ; On vérifie si le charactère est un chiffre/la saisie est terminée
    cmp     rax, 0
    je      final          ; stack[rcx] == '\0'
    cmp     rax, '0'       ; 
    jl      final          ; stack[rcx] < '0'
    cmp     rax, '9'       ; 
    jg      final          ; stack[rcx] > '9'

convert:

    ; On ajoute rax dans le résultat 
    imul    r13, 10        ; r13 *= 10
    sub     rax, '0'       ; rax = int(rax)
    add     r13, rax       ; r13 += rax
    jmp     loop_label

final:
    mov     rax, r13       ; on met le résultat dans le registre de retour

    mov     rsp, rbp       ; restore stack
    pop     rbp
    
    cmp     r13, 0
    je      exit_failure
    ret

exit_failure:
    mov     rax, 60        ; rax = 60
    mov     rdi, 5         ; rdi = 5
    syscall
