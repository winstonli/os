global start
global _start
extern kernel_main ; Defined in main.cpp

; Multiboot Header Definitions
MAGIC equ 0xe85250d6
ARCHITECTURE equ 0 ; 0 for 32-bit protected, 4 for 32-bit MIPS
HEADER_LENGTH equ (multiboot_header_end-multiboot_header)
CHECKSUM equ -(MAGIC + ARCHITECTURE + HEADER_LENGTH)


section .multiboot
align 8, db 0 ; pad with 0s to align to 64-bit boundary
multiboot_header:
    dd MAGIC
    dd ARCHITECTURE
    dd HEADER_LENGTH
    dd CHECKSUM
.tags:
multiboot_header_end:

section .text

STACKSIZE equ 0x4000

start:
_start:
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
