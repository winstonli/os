[bits 64]
extern get_content

section .text
start:
  call get_content
  mov [0xb8000+160*24], eax
  ret
