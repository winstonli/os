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
    PROVIDE(_before_bss = .);
    *(.bss)
    PROVIDE(_after_bss = .);
  }
  ASSERT((_before_bss == _after_bss), "Error: Position-independent and flat-binary modules cannot have .bss sections since no global offset table would be loaded and space would not be reserved by the program loader!")
  ASSERT((_before_bss == _after_bss), "       To avoid this, please ensure that any global variables are annotates with use the STATIC macro as defined in common.h, e.g.")
  ASSERT((_before_bss == _after_bss), "       STATIC int global_variable;")
  ASSERT((_before_bss == _after_bss), "       STATIC state_t state_stack[16];")
}
