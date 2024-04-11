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
    mov     rbx, 10

    cmp     rax, 0          ; si rax negatif
    jg      loop            ; rax non negatif, on passe directemnt à la loop
    
    mov     rdi, '-'        ; on affiche le signe moins
    call    putchar     
    neg     rax             ; rax positif

loop:
	cmp     rax, 0
    je      exit

    idiv	bx        	    ; rax = rax / bx, rdx = rax % bx
	
    add		rdx, '0'		; rdx += '0'
    mov     rdi, rdx        ; on affiche rax % 10
    call    putchar 

	jmp		loop

exit:
	mov 	rsp, rbp		; restore pile
	pop 	rbp
	ret
