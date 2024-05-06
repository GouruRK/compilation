; Brief: getint
;    lit sur l'entrée standard un nombre 
; Arguments:
;   aucun
; retourne:
;   un int

; Registres utilisés
; r12: flag si le nombre entré est négatif
; r13: stocke le résultat temporaire
; rax: valeurs de retour de 'getchar' et de 'getint'
getint:
    ; conventions d'appels AMD64 
    push    rbp             ; sauvegarde le pointeur de base
    mov     rbp, rsp        ; rbp = rsp

    ; On sauvegarde les registres non volatils
    push    r12
    push    r13

    call    getchar         ; récupère le premier character

    mov     r13, 0          ; Initialise le résultat
    mov     r12, 0          ; Initialise les flags
    
    cmp     rax, '-'
    jne     check_gint      ; rax n'est pas '-', c'est donc un nombre. On passe directement à la conversion 
    mov     r12, 1          ; r12 possède le flag 'nombre negatif' 

next_char_gint:
    call    getchar         ; récupère le prochain character dans rax

check_gint:
    ; On vérifie si le charactère est un chiffre/la saisie est terminée
    cmp     rax, 0
    je      final_gint     ; rax == '\0'
    cmp     rax, '0'       ; 
    jl      final_gint     ; rax < '0'
    cmp     rax, '9'       ; 
    jg      final_gint     ; rax > '9'

convert_gint: 
    ; Conversion du 'char' rax en int et ajout au résultat

    imul    r13, 10        ; r13 *= 10
    sub     rax, '0'       ; rax = int(rax)
    add     r13, rax       ; r13 += rax
    jmp     next_char_gint

final_gint:
    ; Prépare la sortie de fonction
    mov     rax, r13       ; on met le résultat dans le registre de retour

    cmp     r12, 1         ; on regarde si le flag 'neg' sur r12 est renseigné
    jne     quit_gint      ; pas négatif, on quitte normalement
    neg     rax            ; c'est négatif, on lui applique la fonction 'neg'

quit_gint:
    ; Détermine quelle sortie appeler en cas d'erreur et de sortie correcte
    cmp     r13, 0
    je      exit_failure_gint
    
    ; On restaure les registres non volatils
    pop     r13
    pop     r12

    mov     rsp, rbp       ; restore stack
    pop     rbp
    ret

exit_failure_gint:
    ; Sortie en cas d'erreur
    mov     rax, 60        ; rax = 60
    mov     rdi, 5         ; rdi = 5
    syscall
