OUTPUT_FORMAT(binary)
SECTIONS
{
  .text : {
    *(.text)
  }

  .data : {
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
