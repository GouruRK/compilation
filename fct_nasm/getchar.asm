global getchar
section .text
; Breif 
;   read a character from stdin
; Arguments
;   N/A
; Returns
;   the read character

; Used registers
; rax: contains the function return's value
getchar:
    ; AMD64 call conventions
    push    rbp
    mov     rbp, rsp       
    
    sub     rsp, 8
    mov     qword [rsp], 0

    mov     rax, 0          ; read
    mov     rdi, 0          ; stdin
    mov     rsi, rsp        ; return address
    mov     rdx, 1          ; size of the lecture (in bytes)
    syscall
    
    pop     rax

    mov     rsp, rbp
    pop     rbp
    ret
