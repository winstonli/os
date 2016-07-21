[bits 64]

%macro push_all 0
      push rax
      push rcx
      push rdx
      push rbx
      push rbp
      push rsi
      push rdi
%endmacro

%macro pop_all 0
      pop rdi
      pop rsi
      pop rbp
      pop rbx
      pop rdx
      pop rcx
      pop rax
%endmacro

global idt_flush
idt_flush:
   lidt [rdi] ; load the idt pointer from first argument
   ret

; interrupt handler (in C code)
extern isr_handler

; This is our common ISR stub. It saves the processor state, sets
; up for kernel mode segments, calls the C-level fault handler,
; and finally restores the stack frame.
isr_common:
    push_all

    mov rax, ds ; save the data segment descriptor
    push rax

    mov ax, 0x10 ; load the kernel data segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call isr_handler

    pop rbx ; reload the original data descriptor
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx

    pop_all
    add rsp, 16+3*8 ; pop the pushed error code and isr number
    sti
    iretq

%macro ISR_NOERRCODE 1
  global isr%1
  isr%1:
    cli
    push byte 0 
    push byte 0x14
    push byte 0x15
    push byte 0x16
    push byte %1
    jmp isr_common
%endmacro

%macro ISR_ERRCODE 1
  global isr%1
  isr%1:
    cli
    push byte %1
    push byte 0x11
    push byte 0x12
    push byte 0x13
    jmp isr_common
%endmacro

ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_ERRCODE   17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_ERRCODE   30
ISR_NOERRCODE 31
