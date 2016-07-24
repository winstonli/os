[bits 64]

global halt

section .text

halt:
  hlt
  jmp halt
