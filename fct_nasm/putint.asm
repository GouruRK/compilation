global my_putint

section .text

; rax = N, rcx = loop_counter, r8 = 1er char,
my_putint:
	push	rbp					    
	mov		rbp, rsp				; rbp = rsp
	sub		rsp, 16					; réserve 16 bytes

	mov 	QWORD [rsp], 0 			;
	mov 	QWORD [rsp + 8], 0 		; on met à 0 la pile

	movsx	rax, edi				; rax = DWORD(rdi)
	mov		rcx, 15					; start compteur boucle
	xor		r8, r8					; r8 = 0
	mov		rbx, 10					; rbx = 10

	test	rax, rax				;
	jns		loop					; if rax >= 0

negative:
	mov		r8b, '-'				; r8 = '-'
	neg		rax						; rax = -rax

loop:
	xor		rdx, rdx				; rx = 0
	div		rbx						; rax = rax / 10, rdx = rax % 10
	add		rdx, '0'				; rdx += '0'
	mov		BYTE [rsp + rcx], dl	; pile[rcx] = dl
	dec		rcx						; rcx--
	test	rax, rax				; 
	jnz		loop					; if rax < 0

write:
	mov		BYTE [rsp + rcx], r8b	;
	mov		rax, 1					; rax = 1
	mov		rdi, 1					; rdi = 1
	mov		rsi, rsp				; rsi = pointeur sur la pile
	mov		rdx, 16					; rdx = 16
	syscall

	mov 	rsp, rbp				; restore pile
	pop 	rbp
	ret