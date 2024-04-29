global getint
extern getchar
section .text

; Brief:
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

    call    getchar         ; récupère le premier character

    mov     r13, 0          ; Initialise le résultat
    mov     r12, 0          ; Initialise les flags
    
    cmp     rax, '-'
    jne     check           ; rax n'est pas '-', c'est donc un nombre. On passe directement à la conversion 
    mov     r12, 1          ; r12 possède le flag 'nombre negatif' 

get_next_char:
    call    getchar         ; récupère le prochain character dans rax

check:
    ; On vérifie si le charactère est un chiffre/la saisie est terminée

    cmp     rax, 0
    je      final          ; rax == '\0'
    cmp     rax, '0'       ; 
    jl      final          ; rax < '0'
    cmp     rax, '9'       ; 
    jg      final          ; rax > '9'

convert: 
    ; Conversion du 'char' rax en int et ajout au résultat

    imul    r13, 10        ; r13 *= 10
    sub     rax, '0'       ; rax = int(rax)
    add     r13, rax       ; r13 += rax
    jmp     get_next_char

final:
    ; Prépare la sortie de fonction

    mov     rax, r13       ; on met le résultat dans le registre de retour

    cmp     r12, 1         ; on regarde si le flag 'neg' sur r12 est renseigné
    jne     quit           ; pas négatif, on quitte normalement
    neg     rax            ; c'est négatif, on lui applique la fonction 'neg'

quit:
    ; Détermine quelle sortie appeler en cas d'erreur et de sortie correcte

    mov     rsp, rbp       ; restore stack
    pop     rbp
    
    cmp     r13, 0
    je      exit_failure
    ret

exit_failure:
    ; Sortie en cas d'erreur

    mov     rax, 60        ; rax = 60
    mov     rdi, 5         ; rdi = 5
    syscall
