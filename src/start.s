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
extern link_loader_start
extern link_loader_end

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
tags:
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
TEXT_SCREEN_ROW equ 80 * 2 ; 80 characters wide, 2 bytes per character
TEXT_SCREEN_NUM_ROWS equ 25
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

pm32_putstrln:
  mov edx, ecx
  mov ecx, TEXT_SCREEN_ROW
  imul ecx, [current_line]
  add ecx, TEXT_SCREEN_MEMORY
  mov eax, [current_line]
  inc eax
  mov [current_line], eax
  call pm32_putstr
  ret

%ifdef DEBUG
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
%endif

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

; eax set to the greater of edx and ecx
pm32_max:
  cmp ecx, edx
  jl .lt
  mov eax, ecx
  ret
.lt:
  mov eax, edx
  ret

; e.g. 9 % 5 = 4
; ecx = 5
; edx = 9
; eax = 4 (result)
pm32_mod:
  mov eax, edx
  xor edx, edx
  div ecx
  mov eax, edx
  ret

; edx = addr
; ecx = sz
pm32_align_up:
  mov esi, ecx
  mov edi, edx
  call pm32_mod
  cmp eax, 0
  jz .aligned
  sub edi, eax
  add edi, esi
  mov eax, edi
  ret
.aligned:
  mov eax, edi
  ret

; the value of eax when we get control of the machine from grub (see 3.3)
MULTIBOOT_EXPECTED_MAGIC equ 0x36d76289

;;;;;;;;;;;;;;;;;
;; ENTRY POINT ;;
;;;;;;;;;;;;;;;;;
start:
  ; setup stack
  mov esp, stack_top

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
  mov ecx, error_wrong_magic_str
  call pm32_putstrln
  jmp hang
.correct_magic:

  ; push magic (eax) and multiboot info (ebx) for later use
  push eax
  push ebx

  mov ecx, TEXT_SCREEN_MEMORY
.clearscreen:
  mov edx, 0
  call pm32_putchar
  cmp ecx, TEXT_SCREEN_MEMORY + TEXT_SCREEN_ROW * TEXT_SCREEN_NUM_ROWS
  jne .clearscreen

%ifdef DEBUG
  ; debug: print hello world string to screen as a sanity check
  ; mov ecx, TEXT_SCREEN_MEMORY
  mov ecx, hello_world_str
  call pm32_putstrln
%endif

  ; ensure we can use long mode, and print an error otherwise
  call pm32_check_long_mode_supported

%ifdef DEBUG
  ; debug: tell the user we can use long mode
  mov ecx, long_mode_supported_str
  call pm32_putstrln

  ; debug: print out current eip
  mov ecx, instruction_ptr_str
  call pm32_putstrln
  call pm32_get_eip ; load eip into eax
  mov edx, eax
  call pm32_puthex

  ; debug: print address of multiboot information structure
  mov ecx, multiboot_info_ptr_str
  call pm32_putstrln
  pop edx
  push edx
  call pm32_puthex
%endif

  ; read and parse multiboot information, looking for modules to jump to
  ; once we have gotten into long mode. (see section 3.4.2 through 3.4.12, but
  ; specifically we are interested in modules as described by 3.4.6)
  mov esi, [esp] ; multiboot information
  add esi, 8   ; start of multiboot information tags
  xor edi, edi ; zero out module start address so we can check we actually
               ; found one later!

.parse_multiboot_header:

  mov eax, [esi] ; type
  mov ebx, [esi + 4] ; size

  cmp eax, 3 ; module tags have type = 3

  jne .not_module

  mov edx, [esi + 3 * 4]
  mov [module.mod_end], edx
  lea edx, [esi + 4 * 4]
  mov [module.string], edx

  mov edi, [esi+2*4] ; module start, store in edi so we can jump to it
                     ; once we enter 64-bit mode!
  mov [kernel_paddr], edi

%ifdef DEBUG
  mov ecx, edi_mod_start_str
  call pm32_putstrln
  mov edx, edi
  call pm32_puthex
  mov dl, ' '
  call pm32_putchar
  mov edx, [module.string]
  call pm32_putstr
%endif

  jmp .parse_multiboot_header_next

.not_module:
  cmp eax, 0
  je .parse_multiboot_header_end

.parse_multiboot_header_next:
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
  cmp dword [kernel_paddr], 0
  jne .valid_module_start_addr
  mov ecx, error_no_kernel_module_str
  call pm32_putstrln
  jmp hang
.valid_module_start_addr:

%ifdef DEBUG
  mov ecx, empty_str
  call pm32_putstrln
  mov ecx, self_start_str
  call pm32_putstrln
  mov edx, link_loader_start
  call pm32_puthex
  mov ecx, self_end_str
  call pm32_putstrln
  mov edx, link_loader_end
  call pm32_puthex

  mov ecx, empty_str
  call pm32_putstrln
  mov ecx, mod_struct_str
  call pm32_putstrln
  mov edx, [module.string]
  call pm32_putstr
  mov ecx, mod_struct_end_str
  call pm32_putstrln
  mov edx, [module.mod_end]
  call pm32_puthex
  mov ecx, empty_str
  call pm32_putstrln
%endif

  ; set up paging
  ; (see http://wiki.osdev.org/Setting_Up_Long_Mode#Setting_up_the_Paging)
  ; we start by identity mapping the first two megabytes (up to 0x200000)
  ; (TODO: we can change it to higher-half later)

  ; our page tables will be located as follows
  ; P4 / PML4T - 0x1000
  ; P3 / PDPT - 0x2000 (high) and 0x4000 (ident)
  ; P2 / PDT - 0x3000
  ; hopefully these won't clash with grub, which tends to load things
  ; much higher in the memory space

PAGE_PRESENT equ 0x1
PAGE_WRITABLE equ 0x3
PAGE_PS equ 1 << 7

; this is the offset that we will add to every absolute memory access
; once the higher half addressing is activated (equiv. to -2 GiB unsigned)
HIGH_ADDR_OFFSET equ 0xffffffff80000000

  mov ecx, link_loader_end
  mov edx, [module.mod_end]
  call pm32_max

  mov ecx, 0x200000
  mov edx, eax
  call pm32_align_up
  mov ecx, 0x200000
  cdq
  div ecx
  mov eax, 0x200

  ; Set up an identity mapping, so that later we can set up the real mapping
  ; (with the identity mapping still existing) so that we can swap the rip and
  ; rsp into the real mapping without everything exploding.
  ; map 0th entry of each of the page tables to the next table in the chain
  ; bits set: present and r/w
  ; level 2 entry has the Page Size (PS) bit set, meaning it's a flat 2 MiB
  ; We place the level 3 table at the bottom of the stack. This is the table
  ; that indexes the bits responsible for giving us an identity mapping
  ; It's the only table we have to change to get our -2 GiB mapping

  mov edi, page_table.l2
  mov esi, PAGE_PS | PAGE_WRITABLE | PAGE_PRESENT
  mov ecx, 0x8
  mul ecx
  add eax, page_table.l2
.set_l2e_next:
  mov dword [edi], esi
  add edi, 0x8
  add esi, 0x200000
  cmp edi, eax
  jl .set_l2e_next

  mov edi, page_table.l3_ident
  mov esi, page_table.l2
  or esi, PAGE_WRITABLE | PAGE_PRESENT
  mov dword [edi], esi

  mov edi, page_table.l3_ident
  or edi, PAGE_WRITABLE | PAGE_PRESENT
  mov dword [page_table.l4], edi

  mov edi, page_table.l4
  mov cr3, edi ; c3 takes the physical base of the PML4

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
  mov cr0, eax ; paging is enabled as soon as this register is set

  mov ecx, 0x200000
  mov edx, [module.mod_end]
  call pm32_align_up
  mov esi, eax

%ifdef DEBUG
  mov ecx, empty_str
  call pm32_putstrln
  mov ecx, empty_str
  call pm32_putstrln
  mov edx, esi
  call pm32_puthex

  ; debug: tell the user we've set up paging
  mov ecx, paging_set_up_str
  call pm32_putstrln
%endif

  ; So supposedly at this point we are actually in long mode, however
  ; we have one last thing to set up in the form of a global descriptor table
  ; (http://wiki.osdev.org/Setting_Up_Long_Mode#Entering_the_64-bit_Submode)
  lgdt [gdt64.pointer]         ; load the 64-bit global descriptor table.

NULL_SEG equ 0x00 ; indexes into the gdt (see label gdt64)
CODE_SEG equ 0x08
DATA_SEG equ 0x10

  mov ax, DATA_SEG ; set indexes for data segment registers to the offset of
  mov ds, ax       ; the data segment in the gdt
  mov es, ax
  mov gs, ax
  mov fs, ax
  mov ss, ax
  jmp CODE_SEG:realm64       ; set the code segment and enter 64-bit long
                             ; mode by performing a jump to realm64

hang:
  hlt ; halt the cpu
  jmp hang ; if we got a non-maskable interrupt or something like that, just
           ; continue to halt


[bits 64]

%ifdef DEBUG
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

lm64_putstrln:
  mov rdx, rcx
  mov rcx, TEXT_SCREEN_ROW
  imul rcx, [current_line]
  add rcx, TEXT_SCREEN_MEMORY
  mov rax, [current_line]
  inc rax
  mov [current_line], rax
  call lm64_putstr
  ret

lm64_puthexdigit:
  ; cannot use the alphabet used in 32-bit mode due to the fact we do not know
  ; whether to reference it with a high or physical address!
  cmp rdx, 10
  jb lm64_putdigit
  add rdx, 'a'-10
  jmp lm64_putchar
lm64_putdigit:
  add rdx, '0'
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
%endif

realm64: ; from here on we are officially (like, actually) in long mode!

  ; for higher-half mapping we need slightly more annoying arithmetic.
  ; we want to map the kernel to -2GiB so all existing addresses can be
  ; converted simply by subtracting 2GiB = 0x8000'0000 (aka. HIGH_ADDR_OFFSET)
  ; this means we want P4[0x1ff] (the final entry) to point to P3_HIGH
  ; and P3_HIGH[0x1fe] to P2_HIGH

  mov rdi, page_table.l3
  or rdi, PAGE_WRITABLE | PAGE_PRESENT
  mov qword [page_table.l4 + 8 * 0x1ff], rdi

  mov rdi, page_table.l2
  or rdi, PAGE_WRITABLE | PAGE_PRESENT
  mov qword [page_table.l3 + 8 * 0x1fe], rdi

  ; also set the direct virtual mapping

  mov rdi, page_table.l3_v
  or rdi, PAGE_WRITABLE | PAGE_PRESENT
  mov qword [page_table.l4 + 8 * 0x110], rdi

  mov rdi, page_table.l2
  or rdi, PAGE_WRITABLE | PAGE_PRESENT
  mov qword [page_table.l3_v], rdi

%ifdef DEBUG
  mov rcx, empty_str
  call lm64_putstrln
  mov rdx, rsp
  call lm64_puthex
%endif

  ; add HIGH_ADDR_OFFSET to rsp and rip
  ; we set rip by adding HIGH_ADDR_OFFSET to the below label
  ; and then jumping to it
  lea rax, [.realm64_high]
  mov rdx, HIGH_ADDR_OFFSET
  add rax, rdx
  add rsp, rdx
  jmp rax
.realm64_high:

  ; load the 64-bit global descriptor table in the right address space
  ; TODO: we should also do a long jump here to ensure it is fully loaded
  lea rax, [gdt64.pointer]
  mov rbx, HIGH_ADDR_OFFSET
  add [gdt64.pointer+2], rbx
  add rax, rbx
  lgdt [rax]

%ifdef DEBUG
  ; debug: check that hex printing works for negative numbers
  push rax
  mov rcx, empty_str
  call lm64_putstrln
  pop rax
  mov rdx, rax
  call lm64_puthex

  ; debug: check that hex printing works for "negative" numbers (msb set)
  mov rcx, empty_str
  call lm64_putstrln
  mov rdx, 0x8000000000000000
  call lm64_puthex
  mov rcx, empty_str
  call lm64_putstrln
  mov rdx, page_table.l3
  call lm64_puthex

  mov rcx, empty_str
  call lm64_putstrln
  mov edx, [rsp]
  call lm64_puthex
%endif

  ; unset identity mapping values
  mov rdi, page_table.l4
  add rdi, HIGH_ADDR_OFFSET
  mov dword [rdi], 0

  invlpg [TEXT_SCREEN_MEMORY] ; invalidate something in the old address range

  ; load our module address (as placed at kernel_paddr by us at
  ; .valid_module_start_addr)
  mov rdx, kernel_paddr
  add rdx, HIGH_ADDR_OFFSET ; convert address of kernel_paddr to vaddr
  mov eax, [rdx] ; filling eax clears top 32 bits as well
  add rax, HIGH_ADDR_OFFSET ; convert kernel_paddr itself to vaddr

  ; at this point (hopefully)
  ; [kernel_paddr] = kernel physical address (see a couple of lines above)
  ; [rsp+0] = information structure physical address
  ; [rsp+4] = multiboot2 magic value
  ; load arguments 1/2 with the magic and information structure (in the right
  ; address space of course!) and jump to the module address
  mov edi, dword [rsp+4] ; magic (also clears top 32-bits)
  mov esi, dword [rsp+0] ; info (also clears top 32-bits)
  add rsi, HIGH_ADDR_OFFSET ; convert info physical addr to virtual addr
  mov edx, link_loader_start
  mov ecx, link_loader_end
  ; ...and jump straight to it! (note that since we don't know anything about
  ; the module other than its load address, we're hoping that the module has
  ; a trampoline or equivalent at the beginning of the module that will kindly
  ; redirect us to where we actually want to go.

  call rax

  ; if we return from that then all we can do is disable interrupts and hang
  cli
  jmp hang

section .data
%ifdef DEBUG
empty_str:
  dq 0
hello_world_str:
  db "Hello World!", 0
instruction_ptr_str:
  db "Current instruction pointer is ", 0
multiboot_info_ptr_str:
  db "Multiboot information structure is at ", 0
long_mode_supported_str:
  db "Long mode seems to be supported! :)", 0
paging_set_up_str:
  db "Finished setting up 64-bit paging!", 0
edi_mod_start_str:
  db "edi (mod_start): ", 0
self_start_str:
  db "self start = ", 0
self_end_str:
  db "self end = ", 0
mod_struct_str:
  db "Found module: ", 0
mod_struct_end_str:
  db "mod_end = ", 0
%endif
hex_digit_lookup_str:
  db "0123456789abcdef", 0
error_wrong_magic_str:
  db "Error: Incorrect multiboot magic number!", 0
error_no_cpuid_str:
  db "Error: The CPUID instruction does not appear to be supported!", 0
error_no_long_mode_str:
  db "Error: Long mode does not appear to be supported!", 0
error_no_kernel_module_str:
  db "Error: No valid kernel module appears to have been loaded!", 0

current_line:
  dq 0

module:
.mod_end:
  dd 0
.string:
  dd 0

kernel_paddr:
  dd 0

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

; TODO: we should really have some kind of guard to ensure this does not
; accidentally overflow into somewhere important!
; We have 5 page tables (1 KiB each) at the top of the stack
STACK_SIZE equ 0x1000000

; reserve us some stack space
stack:
  resb STACK_SIZE
stack_top:

; page table space
; this is so we don't have to guess where we're allowed to put it
; each level of page table is 4 KiB (0x1000 B)

; page tables must be page aligned.
align 0x200000

page_table:
.l3_ident:
  resb 0x1000
.l2_v:
  resb 0x1000
.l3_v:
  resb 0x1000
.l2:
  resb 0x1000
.l3:
  resb 0x1000
.l4:
  resb 0x1000
