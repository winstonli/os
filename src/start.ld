OUTPUT_FORMAT(binary)
SECTIONS
{
  . = 0x00100000;
  link_loader_start = .;

  /* Multiboot header must be in the first 4K of the kernel */
  link_multiboot_start = .;
  .multiboot : {
    *(.multiboot)
  }
  link_multiboot_end = .;

  link_text_start = .;
  .text : {
    *(.text)
  }
  link_text_end = .;

  link_data_start = .;
  .data ALIGN (0x1000) : {
    *(.rodata)
    *(.data)
  }
  link_data_end = .;

  link_bss_start = .;
  .bss : {
    sbss = .;
    *(COMMON)
    *(.bss)
    ebss = .;
  }
  link_bss_end = .;
  /* You can use extern link_kernel_end in C to find where your kernel ends */
  link_loader_end = .;
}
