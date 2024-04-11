global getint

section .text

; rax = res, rcx = loop_counter, rdx = temp_char
getint:
	push	rbp						; sauvegarde le pointeur de base
	mov		rbp, rsp				; rbp = rsp

	sub		rsp, 8					; reserve 1 octets (8 bits)
	mov 	qword [rsp], 0

	mov		rax, 0					; lecture
	mov		rdi, 0					; stdin
	mov		rsi, rsp				; adresse où stocker la lecture
	mov		rdx, 1					; taille de la lecture
	syscall

	mov 	rdx, 0 
	mov     rax, 0					; rax = 0
	mov     rcx, 0					; rcx = 0


parse_int:
	cmp		byte [rsp + rcx], '0'
	jl		exit_failure			; stack[rcx] < '0'
	cmp		byte [rsp + rcx], '9'
	jg		exit_failure			; stack[rcx] > '9'

loop:
	movzx	rdx, byte [rsp + rcx]	; rdx = stack[rcx]
	
	cmp		rdx, 0
	jz		final					; stack[rcx] == '\0'
	cmp		rdx, '0'				; 
	jl		final					; stack[rcx] < '0'
	cmp		rdx, '9'				; 
	jg		final					; stack[rcx] > '9'

	imul	rax, rax, 10			; rax *= 10
	sub		rdx, '0'				; rdx = int(rdx)
	add		rax, rdx				; rax += rdx
	inc		rcx						; rcx++
	jmp		loop

final:
	mov 	rsp, rbp				; restore stack
	pop 	rbp
	ret

exit_failure:
	mov 	rsp, rbp				; restore stack
	pop 	rbp
	mov 	rax, 60					; rax = 60
	mov 	rdi, 5					; rdi = 5
	syscall
