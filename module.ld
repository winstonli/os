OUTPUT_FORMAT(binary)
SECTIONS
{
  .text : {
    *(.text)
  }

  .data ALIGN (0x1000) : {
    *(.rodata)
    *(.data)
  }

  .bss : {
    sbss = .;
    *(COMMON)
    *(.bss)
    ebss = .;
  }
}
