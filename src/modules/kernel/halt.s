[bits 64]

global halt

section .text

halt:
  cli ; if we've halted, we don't want to trigger any more interrupts!
.halt:
  hlt
  jmp .halt
