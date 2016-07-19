; we're now in 64-bit only land! yay!

; since we only know the address of the module loaded rather than any symbols
; within it, the best we can do is to jump to the front of the module (i.e.
; here) and hope that whoever wrote it had the good sense to stick a trampoline
; to the real code here (which we did)!
[bits 64]
extern kernel_main ; defined in main.cpp
extern enable_interrupts ; defined in main.cpp
global entry

section .text
; not particularly exciting, just jump to the kernel proper!
entry:
  jmp kernel_main
