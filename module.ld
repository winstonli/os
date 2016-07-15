OUTPUT_FORMAT(binary)
SECTIONS
{
  .text : {
    *(.text)
    *(.rodata)
    *(.data)
  }
}
