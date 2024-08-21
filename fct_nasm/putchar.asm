global putchar
section .text

; Brief:
;   print in stdout the given character
; Arguments
;   character to print
; Returns
;   N/A

; Used registers
; rdi: argument that contains the character to print
putchar:
    ; AMD64 call conventions
    push rbp        ; save back stack pointer
    mov rbp, rsp    ; rbp = rsp
    
    push rdi        ; pushing the character to print on the stack

    mov rsi, rsp    ; rsi contains the address of the character to print
    mov rdx, 1      ; size to print (in bytes)
    mov rax, 1      ; choose the write function
    mov rdi, 1      ; stdout
    syscall

    mov rsp, rbp    ; putting the stack back
    pop rbp
    
    ret
