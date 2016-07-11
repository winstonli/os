global loader
extern kernel_main ; Defined in main.cpp

; Multiboot Header
FLAGS equ 0
MAGIC equ 0xe85250d6
CHECKSUM equ -(MAGIC + FLAGS)

section .mbheader
align 4
MultiBootHeader:
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .text

STACKSIZE equ 0x4000

loader:
;    mov esp, stack+STACKSIZE ; Stack setup
;    push eax ; Multiboot magic
;    push ebx ; Miltiboot info
;
;    call kernel_main ; Call C kernel

    cli

hang:
    hlt
    jmp hang

;section .bss
;align 4
;stack:
;    resb STACKSIZE
