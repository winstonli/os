global start
global _start
extern kernel_main ; Defined in main.cpp
extern link_text_start
extern link_data_end
extern link_bss_end

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

HEADER_ADDR equ multiboot_header ; the physical address of the multiboot header
LOAD_START_ADDR equ link_text_start ; the address of the start of .text/.data
LOAD_END_ADDR equ link_data_end ; the address of the end of .text/.data
BSS_END_ADDR equ link_bss_end ; the address of the end of .bss



section .multiboot
align 8, db 0 ; pad with 0s to align to 64-bit boundary
multiboot_header:
  dd MAGIC ; magic number so grub2 will find us
  dd ARCHITECTURE ; x86, 32-bit protected mode
  dd HEADER_LENGTH
  dd CHECKSUM ; must be 0 when above three fields are added

.tags:
align 8, db 0
.addr_tag:
  dw TAG_TYPE_ADDRESS
  dw TAG_OPTIONAL
  dd (.addr_tag_end - .addr_tag)
  dd HEADER_ADDR
  dd LOAD_START_ADDR
  dd LOAD_END_ADDR
  dd BSS_END_ADDR
.addr_tag_end:
align 8, db 0
.entry_tag:
  dw TAG_TYPE_ENTRY
  dw TAG_OPTIONAL
  dd (.entry_tag_end - .entry_tag)
  dd start
.entry_tag_end:
align 8, db 0
; Makes us a nice screen to work with, but 0x8b000 is easier for debugging
; purposes for the time being, so we'll uncomment this later if we want it.
; .framebuffer_tag:
;   dw TAG_TYPE_FRAMEBUFFER
;   dw TAG_OPTIONAL
;   dd (.framebuffer_tag_end - .framebuffer_tag)
;   dd 1024
;   dd 768
;   dd 32
; .framebuffer_tag_end:
; align 8, db 0
.null_tag:
  dw TAG_TYPE_NULL
  dw 0
  dd (.null_tag_end - .null_tag)
.null_tag_end:
.tags_end:
multiboot_header_end:



section .text

TEXT_SCREEN_MEMORY equ 0xb8000
TEXT_SCREEN_ROW equ 80*2 ; 80 characters wide, 2 bytes per character

pm32_putchar:
  mov dh, 0x07
  mov [ecx], dx
  add ecx, 2
  ret

pm32_putstr:
  cmp byte [edx], 0
  je .done
  push edx
  mov dl, [edx]
  call pm32_putchar
  pop edx
  add edx, 1
  jmp pm32_putstr
.done:
  ret

; check if cpuid is supported by attempting to flip the id bit (bit 21) in
; the flags register. if we can flip it, cpuid is available.
pm32_check_cpuid_supported:
  ; copy flags in to eax via stack
  pushfd
  pop eax

  mov ecx, eax

  ; flip the id bit
  xor eax, 1 << 21

  ; copy eax to flags via the stack
  push eax
  popfd

  ; copy flags back to eax (with the flipped bit if cpuid is supported)
  pushfd
  pop eax

  ; restore flags from the old version stored in ecx (i.e. flipping the id bit
  ; back if it was ever flipped).
  push ecx
  popfd

  ; compare eax and ecx. if they are equal then that means the bit wasn't
  ; flipped, and cpuid isn't supported.
  cmp eax, ecx
  je .nocpuid
  ret
.nocpuid:
  mov ecx, TEXT_SCREEN_MEMORY
  mov edx, error_no_cpuid_str
  call pm32_putstr

pm32_check_long_mode_supported:
  call pm32_check_cpuid_supported

  mov eax, 0x80000000    ; set the a-register to 0x80000000.
  cpuid                  ; cpu identification.
  cmp eax, 0x80000001    ; compare the a-register with 0x80000001.
  jb .nolongmode         ; it is less, there is no long mode.

  mov eax, 0x80000001    ; set the a-register to 0x80000001.
  cpuid                  ; cpu identification.
  test edx, 1 << 29      ; test if the lm-bit, which is bit 29, is set in the d-register.
  jz .nolongmode         ; they aren't, there is no long mode.
  ret
.nolongmode:
  mov ecx, TEXT_SCREEN_MEMORY
  mov edx, error_no_long_mode_str
  call pm32_putstr


start: ; ENTRY POINT
_start:
  mov esp, stack+STACKSIZE ; Stack setup
  push eax ; Multiboot magic
  push ebx ; Miltiboot info

  cli

  mov ecx, TEXT_SCREEN_MEMORY
  mov edx, hello_world_str
  call pm32_putstr

  call pm32_check_long_mode_supported

  mov ecx, TEXT_SCREEN_MEMORY+TEXT_SCREEN_ROW
  mov edx, long_mode_supported_str
  call pm32_putstr

hang:
  hlt
  jmp hang



section .data
hello_world_str: db "Hello World!", 0
long_mode_supported_str: db "Long mode seems to be supported! :)", 0
error_no_cpuid_str: db "Error: The CPUID instruction does not appear to be supported!", 0
error_no_long_mode_str: db "Error: Long mode does not appear to be supported!", 0



section .bss
align 4

STACKSIZE equ 0x4000

stack: resb STACKSIZE
