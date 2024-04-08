global getint

section .text

; rax = res, rcx = loop_counter, rdx = temp_char
getint:
	push	rbp						; sauvegarde le pointeur de base
	mov		rbp, rsp				; rbp = rsp

	sub		rsp, 16					; reserve 16 bytes

	xor		rax, rax				; rax = 0
	xor		rdi, rdi				; rdi = 0
	mov		rsi, rsp				; rsi = &stack
	mov		rdx, 16					; rdx = 16
	syscall

	test	rax, rax
	js		exit_failure			; read < 0

parse_int:
	xor     rax, rax				; rax = 0
	xor     rcx, rcx				; rcx = 0
	cmp		byte [rsp + rcx], '0'
	jl		exit_failure			; stack[rcx] < '0'
	cmp		byte [rsp + rcx], '9'
	jg		exit_failure			; stack[rcx] > '9'

loop:
	movzx	rdx, byte [rsp + rcx]	; rdx = stack[rcx]
	
	test	rdx, rdx
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
