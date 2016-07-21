#include "common/common.h"
#include "common/interrupts.h"
#include "common/multiboot2.h"
#include "interrupt.h"
#include "pic.h"
#include "terminal.h"

// entry point of 64-bit kernel proper, as jumped to from entry.s
extern "C" void kernel_main(const uint32_t multiboot_magic,
                            void *multiboot_data) {
  terminal_init();
  pic_init();

  terminal_push_cursor_state(79, 24, terminal_colour_t::WHITE,
                             terminal_colour_t::RED);
  terminal_putchar('.');
  terminal_pop_cursor_state();

  terminal_push_cursor_state(4, 10, terminal_colour_t::RED,
                             terminal_colour_t::BLACK);
  if (multiboot_magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
    terminal_printf("Expected magic value of %x but got %x instead!\n",
                    MULTIBOOT2_BOOTLOADER_MAGIC, multiboot_magic);
  }
  terminal_pop_cursor_state();

  terminal_push_cursor_state(0, 10, terminal_colour_t::GREEN,
                             terminal_colour_t::BLACK);
  uint64_t rip = 0;
  asm volatile("leaq (%%rip), %0" : "=r"(rip));

  terminal_printf("%s: %x\n", "Current instruction pointer is around", rip);
  terminal_printf("%s: %x\n", "Multiboot data is at", multiboot_data);

  init_interrupt_descriptor_table();

  out<uint8_t>(0x21, 0xff);
  out<uint8_t>(0xa1, 0xff);

  enable_interrupts();

  for (int i = -11; i < 11; ++i) {
    terminal_printf("%d,", i);
  }
  while (true) {
    for (int j = 0; j < 10; ++j) {
      for (int i = 0; i < 2000000; ++i) {
      }
      terminal_putchar('.');
    }
    terminal_putchar('X');
  }
}
