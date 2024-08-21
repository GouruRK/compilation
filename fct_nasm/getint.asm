global getint
extern getchar
section .text

; Brief:
;    read a signed number from stdin
; Arguments
;   N/A
; Returns
;   integer

; Used registers
; r12: flag that indicate if the number is negative
; r13: store temporary results
; rax: output values of 'getchar' and 'getint' function

getint:
    ; AMD64 call conventions
    push    rbp             ; save back stack pointer
    mov     rbp, rsp        ; rbp = rsp

    call    getchar         ; get the first character

    mov     r13, 0          ; Initialise result
    mov     r12, 0          ; Initialise flag
    
    cmp     rax, '-'        ; check whether the first character is a negative
                            ; sign
    
    ; check if the first given character is a negative sign. If so, sets flag to
    ; 'r12' that the number is indeed negative. If not, the first character must
    ; be a number, and we need to check if it really is before adding it to the
    ; result

    jne     check           ; not a negative sign, jump to check
    mov     r12, 1          ; sets 'r12' flag 

get_next_char:
    call    getchar         ; get the next character

check:
    ; check if the input is valid

    cmp     rax, 0
    je      final          ; rax == '\0'
    cmp     rax, '0'       ; 
    jl      final          ; rax < '0'
    cmp     rax, '9'       ; 
    jg      final          ; rax > '9'

convert: 
    ; Convert the input into decimal number

    imul    r13, 10        ; r13 *= 10
    sub     rax, '0'       ; rax = int(rax)
    add     r13, rax       ; r13 += rax
    jmp     get_next_char

final:
    ; Prepare function exit

    mov     rax, r13       ; put the result in the exit register

    cmp     r12, 1         ; check if the negative flat is up
    jne     quit           ; not up, exit normaly
    neg     rax            ; is negative

quit:
    ; Determine how exiting the function: by triggering an error or normaly

    mov     rsp, rbp       ; restore stack
    pop     rbp
    
    cmp     r13, 0
    je      exit_failure
    ret

exit_failure:
    ; In case of error

    mov     rax, 60        ; rax = 60
    mov     rdi, 5         ; rdi = 5
    syscall
