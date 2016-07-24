[bits 64]

; convenience functions to push and pop all general purpose registers
; (note how the order of pushes determines the order of structure members
; for registers_t, but otherwise the order is arbitrary)
; TODO: we are not saving r8-r16
; TODO: we are not saving sse/mmx/x87/avx registers
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

; load a new idt table to the cpu
global idt_flush
idt_flush:
   lidt [rdi] ; load the idt pointer from first argument
   ret

; common code for handling isr/irq
%macro common 1
  extern %1_handler
  %1_common:
    push_all ; save all the registers

    mov rax, ds ; save the data segment descriptor
    push rax

    mov ax, 0x10 ; load the kernel data segment descriptor
    mov ds, ax   ; TODO: only fs/gs are actually used in 64-bit mode?
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov rdi, rsp ; move pointer to saved registers to rdi (first argument to
                 ; function) so we can easily inspect registers from C++ land.

    call %1_handler ; call {isr, irq}_handler

    pop rbx ; reload the original data descriptor
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx

    pop_all
    add rsp, 16 ; pop the pushed error code and int no
    sti ; re-enable interrupts (takes effect after next instruction)
    iretq ; return back to interrupted code (note q suffix, which nasm really
          ; doesn't feel like adding implicitly for us even in 64-bit mode)
%endmacro

; define the two common handlers
common isr
common irq

; define exception handler for exceptions with no error code
%macro ISR_NOERRCODE 1
  global isr%1
  isr%1:
    cli
    push byte 0 ; push dummy error code to allow us to use common code
    push byte %1 ; push int no and jump to common code
    jmp isr_common
%endmacro

; define exception handler for exceptions with a cpu-pushed error code
%macro ISR_ERRCODE 1
  global isr%1
  isr%1:
    cli
    push byte %1 ; error code is pushed by cpu, so just need int no
    jmp isr_common
%endmacro

; define interrupt request handler (software/pic etc...-generated interrupts)
%macro IRQ 2
  global irq%1
  irq%1:
    cli
    push byte 0 ; no cpu-pushed error code
    push byte %2 ; push int no and jump
    jmp irq_common
%endmacro

ISR_NOERRCODE 0  ; divide by zero error (rip points to div/idiv instruction)
ISR_NOERRCODE 1  ; debug exception
ISR_NOERRCODE 2  ; non-maskable interrupt (nmi)
ISR_NOERRCODE 3  ; breakpoint exception
ISR_NOERRCODE 4  ; overflow exception
ISR_NOERRCODE 5  ; bound range exceeded (see x86 'bound' instruction)
ISR_NOERRCODE 6  ; invalid opcode
ISR_NOERRCODE 7  ; device not available (e.g. fpu disabled)
ISR_ERRCODE   8  ; double fault (exception occurrred trying to handle an
                 ; existing exception, rip is undefined)
ISR_NOERRCODE 9  ; formerly: coprocessor segment overrun, no longer used.
ISR_ERRCODE   10 ; invalid task state segment (err code is selector index)
ISR_ERRCODE   11 ; segment not present
ISR_ERRCODE   12 ; stack-segment fault
ISR_ERRCODE   13 ; general protection fault (gpf) (err is selector index,
                 ; rip is faulting instruction)
ISR_ERRCODE   14 ; page fault (rip is faulting instruction)
ISR_NOERRCODE 15 ; reserved
ISR_NOERRCODE 16 ; x87 floating-point exception
ISR_ERRCODE   17 ; alignment check
ISR_NOERRCODE 18 ; machine check
ISR_NOERRCODE 19 ; virtualisation exception
ISR_NOERRCODE 20 ; reserved
ISR_NOERRCODE 21 ; reserved
ISR_NOERRCODE 22 ; reserved
ISR_NOERRCODE 23 ; reserved
ISR_NOERRCODE 24 ; reserved
ISR_NOERRCODE 25 ; reserved
ISR_NOERRCODE 26 ; reserved
ISR_NOERRCODE 27 ; reserved
ISR_NOERRCODE 28 ; reserved
ISR_NOERRCODE 29 ; reserved
ISR_ERRCODE   30 ; security exception
ISR_NOERRCODE 31 ; reserved

; assumes pic is remapped to 0x20 onwards (see pic.h)
IRQ 0, 32 ; assume PIC1_OFFSET is 32=0x20
IRQ 1, 33
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40 ; assume PIC2_OFFSET is 40=0x28
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47
