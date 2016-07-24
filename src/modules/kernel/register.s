[bits 64]

global get_cr3

section .text

get_cr3:
mov qword rax, cr3
ret
