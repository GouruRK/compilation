global putint
extern putchar
section .text

; Brief
;   print on stdin a signed number
; Arguments
;   number to print
; Returns
;   N/A

; Used registers
; r12: length
; rax: integier to print
; rbx: dividend
; rdx: quotient
putint:
    ; AMD64 call conventions
    push    rbp             ; save the back stack pointer
    mov     rbp, rsp        ; rbp = rsp

    mov     rax, rdi        ; save the integer to print
    mov     rbx, 10         ; diviseur
    mov     r12, 0

    cmp     rax, 0
    jge     loop_label      ; if the integer is positive, skip the next lines
    
    mov     rdi, '-'        ; integer is negative, print the negative sign
    call    putchar

loop_label:
    cmp     rax, 0
    je      print

    mov     rdx, 0          ; initialise quotient

    idiv    rbx             ; rax = rax // rbx, rdx = rax % rbx
    
    add     rdx, '0'        ; rdx += '0'
    
    push    rdx

    inc     r12

    jmp     loop_label

print:

    cmp     r12, 0
    je      exit

    pop     rdi
    call    putchar

    dec     r12
    jmp     print

exit:
    mov     rsp, rbp        ; restore stack
    pop     rbp
    ret
