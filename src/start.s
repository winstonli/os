; This file holds data that identifies us as multiboot-compilant
; and enables grub to load this file and switch to us after nicely putting us
; in 32-bit protected mode. We load the actual 64-bit kernel proper as a
; multiboot module, allowing us to strictly work with 64-bit code in the
; kernel itself so only this file has to deal with the additional complexity
; of handling both modes.
[bits 32]
extern link_text_start ; all defined by linker script of `filename.ld`,
extern link_data_end
extern link_bss_end

; references to multiboot spec refer to version 1.6:
; http://download-mirror.savannah.gnu.org/releases/grub/phcoder/multiboot.pdf

; multiboot header definitions (see section 3.1.1/3.1.2 of multiboot spec)
MAGIC equ 0xe85250d6
ARCHITECTURE equ 0 ; 0 for 32-bit protected, 4 for 32-bit MIPS
HEADER_LENGTH equ (multiboot_header_end-multiboot_header)
CHECKSUM equ -(MAGIC + ARCHITECTURE + HEADER_LENGTH)

; tags allow us to request and supply certain pieces of information about our
; system from and to grub (see sections 3.1.3 through 3.1.9)
TAG_TYPE_NULL equ 0  ; null tag type to signify end of tag list
TAG_TYPE_INFORMATION_REQUEST equ 1 ; request information (e.g. memory map
                                   ; or loaded modules), however grub does
                                   ; this without asking
TAG_TYPE_ADDRESS equ 2  ; specify where to load this file into physical memory
TAG_TYPE_ENTRY equ 3 ; specify entry point for our program (i.e. `start` label)
TAG_TYPE_FLAGS equ 4 
TAG_TYPE_FRAMEBUFFER equ 5  ; allows us to set the graphics mode
TAG_TYPE_MODULE_ALIGNMENT equ 6 ; requst that modules are loaded aligned to
                                ; a page boundary, however grub does this
                                ; without asking

; specifies whether the bootloader should fail if it does not support a tag
TAG_REQUIRED equ 0
TAG_OPTIONAL equ 1

; some useful definitions that we'll use in the tag definitions
HEADER_ADDR equ multiboot_header ; the physical address of the multiboot header
LOAD_START_ADDR equ link_text_start ; the address of the start of .text/.data
LOAD_END_ADDR equ link_data_end ; the address of the end of .text/.data
BSS_END_ADDR equ link_bss_end ; the address of the end of .bss

; alignment of various structures in multiboot structures
; section 3.1.3 _incorrectly_ specifies that structures are padded to u_virt
; size (the size of the virtual address space, i.e. 32-bits/4-bytes in 32-bit
; protected mode). It is actually 8 bytes regardless of the underlying arch.
; (see http://forum.osdev.org/viewtopic.php?f=1&t=27602 if interested)
MULTIBOOT_ALIGNMENT equ 8



section .multiboot
align MULTIBOOT_ALIGNMENT, db 0 ; pad with 0s to align to 64-bit boundary
multiboot_header:
  dd MAGIC ; magic number so grub2 will find us
  dd ARCHITECTURE ; x86, 32-bit protected mode
  dd HEADER_LENGTH
  dd CHECKSUM ; must be 0 when above three fields are added to this
              ; i.e. MAGIC + ARCHITECTURE + HEADER_LENGTH + CHECKSUM = 0

; we should technically have an information request here for modules, but
; since grub provides it regardless we omit it for now. (similarly we may
; also want to have aligned modules, unsure)
.tags:
align MULTIBOOT_ALIGNMENT, db 0
.addr_tag:  ; specify important addresses so grub loads us correctly (3.1.5)
  dw TAG_TYPE_ADDRESS
  dw TAG_OPTIONAL
  dd (.addr_tag_end - .addr_tag)
  dd HEADER_ADDR
  dd LOAD_START_ADDR
  dd LOAD_END_ADDR
  dd BSS_END_ADDR
.addr_tag_end:
align MULTIBOOT_ALIGNMENT, db 0
.entry_tag:  ; specify entry point of our program (`start` symbol, 3.1.6)
  dw TAG_TYPE_ENTRY
  dw TAG_OPTIONAL
  dd (.entry_tag_end - .entry_tag)
  dd start ; entry point!
.entry_tag_end:
align MULTIBOOT_ALIGNMENT, db 0
; Makes us a nice screen to work with, but 0x8b000 is easier for debugging
; purposes for the time being, so we'll uncomment this later if we want it.
; .framebuffer_tag:  ; 3.1.8
;   dw TAG_TYPE_FRAMEBUFFER
;   dw TAG_OPTIONAL
;   dd (.framebuffer_tag_end - .framebuffer_tag)
;   dd 1024
;   dd 768
;   dd 32
; .framebuffer_tag_end:
; align MULTIBOOT_ALIGNMENT, db 0
.null_tag:
  dw TAG_TYPE_NULL ; this lets the bootloader know that we have run out of tags
  dw 0             ; (final paragraph of 3.1.3)
  dd (.null_tag_end - .null_tag)
.null_tag_end:
.tags_end:
multiboot_header_end:



section .text

TEXT_SCREEN_MEMORY equ 0xb8000 ; address of the text screen video memory
                               ; (80x25 w/ one byte each for colour and ascii)
TEXT_SCREEN_ROW equ 80*2 ; 80 characters wide, 2 bytes per character
TEXT_WHITE_ON_BLACK equ 0x07

; note on calling conventions:
; for this file we utilise our own arbitrary calling convention of passing
; arguments 1 and 2 in ecx/rcx and edx/rdx respectively and returning any
; resulting values in eax/rax (similar to 32-bit ms/gnu fastcall convention)
; (for more info on calling conventions, although not especially relevant, see
; http://www.agner.org/optimize/calling_conventions.pdf)

; print the character in dl to the screen at position given by ecx,
; then increment ecx in preparation for next character
pm32_putchar:
  mov dh, TEXT_WHITE_ON_BLACK
  mov [ecx], dx
  add ecx, 2
  ret

; print the \0-terminated string given by edx to the screen at position given
; by ecx, returning edx pointing to the end of the string and ecx pointing to
; the screen at the position after the final character
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

; print the character corresponding to the value of dl in hexadecimal
; assumes that dl is in the range [0-15] inclusive, see putchar
pm32_puthexdigit:
  mov dl, [hex_digit_lookup_str + edx]
  jmp pm32_putchar

; writes a 32-bit hexadecimal number of the screen at ecx with a '0x' prefix
pm32_puthex:
  push edx
  ; draw '0x' prefix
  mov dl, '0'
  call pm32_putchar
  mov dl, 'x'
  call pm32_putchar
  pop edx
  ; if we are dealing with a value that is exactly 0, we can just print that
  ; (which avoids accidentally printing "0x" by itself when trimming leading
  ; zeros)
  cmp edx, 0
  jne .not_zero
  mov dl, '0'
  call pm32_putchar
  ret
.not_zero:
  push eax ; caller-saved eax
  mov eax, ecx ; need to move ecx and eax around since only cl can be used
               ; for shifts.
  mov cl, 32 ; how much do we need to shift to get the value of the next digit
  mov ch, 0 ; have we written a non-zero char yet?
.puthex:
  sub cl, 4
  push edx ; save the original number for the next comparison

  shr edx, cl ; shift right and "and" with 0xf to get least-significant-digit
  and edx, 0xf

  cmp edx, 0x0 ; if its 0, check if we've already had a nonzero character
  je .skip_digit_maybe
  jmp .no_skip_digit
.skip_digit_maybe:
  cmp ch, 0
  je .skip_digit ; we haven't seen a non-zero character yet, so skip
.no_skip_digit:
  mov ch, 1 ; we've now seen a non-zero character if we hadn't already

  ; since we want ecx to hold the screen ptr when calling puthexdigit, we swap
  ; eax and ecx, swapping them back again at the end so we can use it again on
  ; the next iteration
  xchg eax, ecx
  call pm32_puthexdigit
  xchg eax, ecx

.skip_digit:
  pop edx
  cmp cl, 0 ; are we done yet?
  jnz .puthex

  ; reset the registers as we saw them and exit
  mov ecx, eax
  pop eax
  ret

; get the value of the instruction pointer in a convoluted way, since x86
; doesn't let us do `mov eax, eip`
; (see http://f.osdev.org/viewtopic.php?f=13&t=18936)
pm32_get_eip:
  mov eax, [esp]
  ret

; check if cpuid is supported by attempting to flip the id bit (bit 21) in
; the flags register. if we can flip it, cpuid is available.
; (see http://wiki.osdev.org/Setting_Up_Long_Mode#Detection_of_CPUID)
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
  jmp hang

; check long mode is supported by first checking if we support cpuid, then
; checking if the function to check if long mode is supported is supported,
; then finally checking if long mode is supported :)
; (see http://wiki.osdev.org/Setting_Up_Long_Mode#x86_or_x86-64)
pm32_check_long_mode_supported:
  call pm32_check_cpuid_supported

  mov eax, 0x80000000
  cpuid
  cmp eax, 0x80000001
  jb .nolongmode

  mov eax, 0x80000001
  cpuid
  test edx, 1 << 29 ; test if the lm-bit, which is bit 29, is set
  jz .nolongmode
  ret
.nolongmode:
  mov ecx, TEXT_SCREEN_MEMORY
  mov edx, error_no_long_mode_str
  call pm32_putstr
  jmp hang


; TODO: we should really have some kind of guard to ensure this does not
; accidentally overflow into somewhere important!
STACK_SIZE equ 0x4000

; the value of eax when we get control of the machine from grub (see 3.3)
MULTIBOOT_EXPECTED_MAGIC equ 0x36d76289

;;;;;;;;;;;;;;;;;
;; ENTRY POINT ;;
;;;;;;;;;;;;;;;;;
start:
  mov esp, stack+STACK_SIZE ; setup stack

  ; state of the machine at this point: (see section 3.3 for more details)
  ; eax contains multiboot magic value (MULTIBOOT_EXPECTED_MAGIC)
  ; ebx contains the physical address of the multiboot information structure
  ; segment registers set to offset 0, limit 0xffffffff
  ; a20 enabled
  ; cr0: bit 0 (protected mode) set, bit 31 (paging) unset
  ; gdtr - must not load segment registers until is set by os (see gdt64)
  ; idtr - must leave interrupts disabled until the os sets up an idt


  ; check that we got the right magic value, otherwise nobody knows!
  cmp eax, MULTIBOOT_EXPECTED_MAGIC
  je .correct_magic
  mov ecx, TEXT_SCREEN_MEMORY
  mov edx, error_wrong_magic_str
  call pm32_putstr
  jmp hang
.correct_magic:

  push ebx ; multiboot info, for later use

  ; debug: print hello world string to screen as a sanity check
  mov ecx, TEXT_SCREEN_MEMORY
  mov edx, hello_world_str
  call pm32_putstr

  ; ensure we can use long mode, and print an error otherwise
  call pm32_check_long_mode_supported

  ; debug: tell the user we can use long mode
  mov ecx, TEXT_SCREEN_MEMORY+TEXT_SCREEN_ROW
  mov edx, long_mode_supported_str
  call pm32_putstr

  ; debug: print out current eip
  mov ecx, TEXT_SCREEN_MEMORY+2*TEXT_SCREEN_ROW
  mov edx, instruction_ptr_str
  call pm32_putstr
  call pm32_get_eip ; load eip into eax
  mov edx, eax
  call pm32_puthex

  ; debug: print address of multiboot information structure
  mov ecx, TEXT_SCREEN_MEMORY+3*TEXT_SCREEN_ROW
  mov edx, multiboot_info_ptr_str
  call pm32_putstr
  pop edx
  push edx
  call pm32_puthex

  ; read and parse multiboot information, looking for modules to jump to
  ; once we have gotten into long mode. (see section 3.4.2 through 3.4.12, but
  ; specifically we are interested in modules as described by 3.4.6)
  mov ecx, TEXT_SCREEN_MEMORY+4*TEXT_SCREEN_ROW
  pop edx ; multiboot information
  push edx
  lea esi, [edx+8] ; start of multiboot information tags
  xor edi, edi ; zero out module start address so we can check we actually
               ; found one later!
.parse_multiboot_header:
  mov dl, 't'
  call pm32_putchar
  mov edx, [esi] ; type
  mov eax, edx
  call pm32_puthex ; print type of tag
  mov dl, ':'
  mov ebx, [esi+4] ; size

  cmp eax, 3 ; module tags have type = 3
  jne .not_module
  lea edx, [esi+4*4] ; module path (utf-8, \0-terminated string)
  call pm32_putstr
  mov dl, '@'
  call pm32_putchar
  mov edx, [esi+2*4] ; module start
  mov edi, edx       ; store module start in edi so we can jump to it
                     ; once we enter 64-bit mode!
  call pm32_puthex
  mov dl, ' '
  call pm32_putchar
  jmp .parse_multiboot_header_next

.not_module:
  cmp eax, 0
  je .parse_multiboot_header_end
.parse_multiboot_header_next:
  mov dl, ' '
  call pm32_putchar
  add esi, ebx
.parse_multiboot_header_next2:
  ; increment until we are aligned according to MULTIBOOT_ALIGNMENT,
  ; then jump back and continue
  test esi, MULTIBOOT_ALIGNMENT-1
  jz .parse_multiboot_header
  inc esi
  jmp .parse_multiboot_header_next2
.parse_multiboot_header_end:

  ; check that we found a good start address
  cmp edi, 0
  jne .valid_module_start_addr
  mov ecx, TEXT_SCREEN_MEMORY+6*TEXT_SCREEN_ROW
  mov edx, error_no_kernel_module_str
  call pm32_putstr
  jmp hang
.valid_module_start_addr:
  push edi ; push module start address

  ; set up paging
  ; (see http://wiki.osdev.org/Setting_Up_Long_Mode#Setting_up_the_Paging)
  ; we start by identity mapping the first two megabytes (up to 0x200000)
  ; (TODO: we can change it to higher-half later)

  ; our page tables will be located as follows:
  ; P4 / PML4T - 0x1000
  ; P3 / PDPT - 0x2000
  ; P2 / PDT - 0x3000
  ; P1 / PT - 0x4000
  ; hopefully these won't clash with grub, which tends to load things
  ; much higher in the memory space

  mov edi, 0x1000
  mov cr3, edi ; set the location of P4 to 0x1000

  xor eax, eax
  mov ecx, 4096
  rep stosd ; zero the first 4096 dwords of memory (i.e. 4096*4 = 0x4000 bytes)

  mov edi, cr3
  ; the trailing 3s here indicate that the age is present and r/w
  mov dword [edi], 0x2003      ; map 0th entry of P4 to 0x2000
  add edi, 0x1000
  mov dword [edi], 0x3003      ; map 0th entry of P3 to 0x3000
  add edi, 0x1000
  mov dword [edi], 0x4003      ; map 0th entry of P2 to 0x4000
  add edi, 0x1000

  ; finally, identity map all 512 entries of P1 at 0x4000!
  mov ebx, 0x3 ; start at address 0, with flags as above
  mov ecx, 512

.set_entry:
  mov dword [edi], ebx ; actually set the entry
  add ebx, 0x1000      ; iterate to next address (4kb pages = 0x1000)
  add edi, 8           ; iterate to next entry in P1 (size of ptr)
  loop .set_entry      ; decrement and check ecx

  ; now we set up PAE (physical address extension) by setting bit 5
  ; of control register 4
  mov eax, cr4
  or eax, 1 << 5
  mov cr4, eax
  
  ; FINALLY we are ready to enter long mode!
  mov ecx, 0xc0000080          ; 0xc0000080 is the efer msr (extended feature
                               ; enable register, model specific register)
  rdmsr                        ; read from model-specific register
  or eax, 1 << 8               ; set the lm-bit (long-mode) which is bit 8.
  wrmsr                        ; write to model-specific register

  ; enable paging by setting bit 31 of control register 0
  mov eax, cr0
  or eax, 1 << 31
  mov cr0, eax

  ; debug: tell the user we've set up paging
  mov ecx, TEXT_SCREEN_MEMORY+6*TEXT_SCREEN_ROW
  mov edx, paging_set_up_str
  call pm32_putstr

  ; So supposedly at this point we are actually in long mode, however
  ; we have one last thing to set up in the form of a global descriptor table
  ; (http://wiki.osdev.org/Setting_Up_Long_Mode#Entering_the_64-bit_Submode)
  lgdt [gdt64.pointer]         ; load the 64-bit global descriptor table.
  jmp gdt64.code:realm64       ; set the code segment and enter 64-bit long
                               ; mode by performing a far jump to realm64

hang:
  hlt ; halt the cpu
  jmp hang ; if we got a non-maskable interrupt or something like that, just
           ; continue to halt


[bits 64]

; these functions are all just 64-bit versions of those defined above
; (i.e. for documentation, see s/lm64/pm32/)

lm64_putchar:
  mov dh, 0x07
  mov [rcx], dx
  add rcx, 2
  ret

lm64_putstr:
  cmp byte [rdx], 0
  je .done
  push rdx
  mov dl, [rdx]
  call lm64_putchar
  pop rdx
  add rdx, 1
  jmp lm64_putstr
.done:
  ret


lm64_puthexdigit:
  mov dl, [hex_digit_lookup_str + rdx]
  jmp lm64_putchar

lm64_puthex:
  push rdx
  mov dl, '0'
  call lm64_putchar
  mov dl, 'x'
  call lm64_putchar
  pop rdx
  cmp rdx, 0
  jne .not_zero
  mov dl, '0'
  call lm64_putchar
  ret
.not_zero:
  push rax
  mov rax, rcx
  mov cl, 64
  mov ch, 0 ; have we written a non-zero char yet?
.puthex:
  sub cl, 4
  push rdx
  shr rdx, cl
  and rdx, 0xf
  cmp rdx, 0x0
  je .skip_digit_maybe
  jmp .no_skip_digit
.skip_digit_maybe:
  cmp ch, 0
  je .skip_digit
.no_skip_digit:
  mov ch, 1
  xchg rax, rcx
  call lm64_puthexdigit
  xchg rax, rcx
.skip_digit:
  pop rdx
  cmp cl, 0
  jnz .puthex
  mov rcx, rax
  pop rax
  ret

realm64: ; from here on we are officially (like, actually) in long mode!

  ; debug: load our module address (as placed on the stack by us at
  ; .valid_module_start_addr), print it out...
  xor rax, rax
  mov eax, [rsp]
  mov ecx, TEXT_SCREEN_MEMORY + 7*TEXT_SCREEN_ROW
  mov rdx, rax
  call lm64_puthex
  ; ...and jump straight to it! (note that since we don't know anything about
  ; the module other than its load address, we're hoping that the module has
  ; a trampoline or equivalent at the beginning of the module that will kindly
  ; redirect us to where we actually want to go.
  call rax

  ; if we return from that then all we can do is disable interrupts and hang
  cli
  jmp hang

section .data
hello_world_str: db "Hello World!", 0
instruction_ptr_str: db "Current instruction pointer is ", 0
multiboot_info_ptr_str: db "Multiboot information structure is at ", 0
long_mode_supported_str: db "Long mode seems to be supported! :)", 0
paging_set_up_str: db "Finished setting up 64-bit paging!", 0
error_wrong_magic_str: db "Error: Incorrect multiboot magic number!", 0
error_no_cpuid_str: db "Error: The CPUID instruction does not appear to be supported!", 0
error_no_long_mode_str: db "Error: Long mode does not appear to be supported!", 0
error_no_kernel_module_str: db "Error: No valid kernel module appears to have been loaded!", 0
hex_digit_lookup_str: db "0123456789abcdef", 0


; global descriptor table
; (see http://wiki.osdev.org/Setting_Up_Long_Mode#Entering_the_64-bit_Submode)
; (see http://wiki.osdev.org/Global_Descriptor_Table)

gdt64:                ; global descriptor table (64-bit).
.null: equ $ - gdt64  ; the null descriptor.
  dw 0                ; limit (low).
  dw 0                ; base (low).
  db 0                ; base (middle)
  db 0                ; access.
  db 0                ; granularity.
  db 0                ; base (high).
.code: equ $ - gdt64  ; the code descriptor.
  dw 0                ; limit (low).
  dw 0                ; base (low).
  db 0                ; base (middle)
  db 10011010b        ; access (exec/read).
  db 00100000b        ; granularity.
  db 0                ; base (high).
.data: equ $ - gdt64  ; the data descriptor.
  dw 0                ; limit (low).
  dw 0                ; base (low).
  db 0                ; base (middle)
  db 10010010b        ; access (read/write).
  db 00000000b        ; granularity.
  db 0                ; base (high).
.pointer:             ; the gdt-pointer.
  dw $ - gdt64 - 1    ; limit.
  dq gdt64            ; base.


; note: ordinarily since we're loading a flat binary, having a .bss section is
; a recipe for disaster since grub won't load it and we'll be accidentally
; destroying important bits of memory (probably the kernel), however since we
; specify the bss section of the loader explicitly in the address tag of the
; multiboot header, grub should reserve us space for it despite it not actually
; being in the file at all
section .bss
; reserve us some stack space
stack: resb STACK_SIZE
