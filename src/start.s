global start
global _start
extern kernel_main ; Defined in main.cpp

; Multiboot Header Definitions
MAGIC equ 0xe85250d6
ARCHITECTURE equ 0 ; 0 for 32-bit protected, 4 for 32-bit MIPS
HEADER_LENGTH equ (multiboot_header_end-multiboot_header)
CHECKSUM equ -(MAGIC + ARCHITECTURE + HEADER_LENGTH)

TAG_TYPE_NULL equ 0
TAG_TYPE_INFORMATION_REQUEST equ 1
TAG_TYPE_ADDRESS equ 2
TAG_TYPE_ENTRY equ 3
TAG_TYPE_FLAGS equ 4
TAG_TYPE_FRAMEBUFFER equ 5
TAG_TYPE_MODULE_ALIGNMENT equ 6

TAG_REQUIRED equ 0
TAG_OPTIONAL equ 1

HEADER_ADDR equ multiboot_header ; TODO hardcoded magic, get from linker script
LOAD_ADDR equ start ; TODO this could be wrong!
LOAD_END_ADDR equ 0 ; TODO
BSS_ADDR equ 0 ; TODO



section .multiboot
align 8, db 0 ; pad with 0s to align to 64-bit boundary
multiboot_header:
  dd MAGIC
  dd ARCHITECTURE
  dd HEADER_LENGTH
  dd CHECKSUM
.tags:
align 8, db 0 ; pad with 0s to align to 64-bit boundary
.addr_tag:
  dw TAG_TYPE_ADDRESS
  dw TAG_OPTIONAL
  dd (.addr_tag_end - .addr_tag)
  dd HEADER_ADDR
  dd LOAD_ADDR
  dd LOAD_END_ADDR
  dd BSS_ADDR
.addr_tag_end:
align 8, db 0 ; pad with 0s to align to 64-bit boundary
.entry_tag:
  dw TAG_TYPE_ENTRY
  dw TAG_OPTIONAL
  dd (.entry_tag_end - .entry_tag)
  dd start
.entry_tag_end:
align 8, db 0 ; pad with 0s to align to 64-bit boundary
.framebuffer_tag:
  dw TAG_TYPE_FRAMEBUFFER
  dw TAG_OPTIONAL
  dd (.framebuffer_tag_end - .framebuffer_tag)
  dd 1024
  dd 768
  dd 32
.framebuffer_tag_end:
align 8, db 0 ; pad with 0s to align to 64-bit boundary
.null_tag:
  dw TAG_TYPE_NULL
  dw 0
  dd (.null_tag_end - .null_tag)
.null_tag_end:
.tags_end:
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
