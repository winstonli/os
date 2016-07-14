OUTPUT_FORMAT(binary)
SECTIONS
{
  .text : {
    *(.text)
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
