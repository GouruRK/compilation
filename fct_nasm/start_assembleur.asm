section .text
global _start
extern show_registers
extern my_putchar
extern my_getchar
extern my_putint

_start:
    mov r11, rsp ; Début alignement de la pile
    sub rsp, 8 
    and rsp, -16 ; Masque
    mov qword [rsp], r11 ; Fin alignement de la pile

    ; test putchar
    mov dil, 'A'
    call my_putchar
    mov dil, 10 ; saut de ligne
    call my_putchar

    ; test getchar
    call my_getchar
    mov rbx, [rax]
    mov dil, bl
    call my_putchar
    mov dil, 10 ; saut de ligne
    call my_putchar

    ; test putint
    mov dil, 2
    call my_putint
    mov dil, 10 ; saut de ligne
    call my_putchar
    
    mov rax, 60
    mov rdi, 0
    syscall