#include "common/common.h"
#include "common/interrupts.h"
#include "common/multiboot2.h"
#include "idt.h"
#include "irq.h"
#include "isr.h"
#include "pic.h"
#include "terminal.h"
#include <boot/multiboot_info.h>
#include <log.h>
#include <vm/page_table.h>

// entry point of 64-bit kernel proper, as jumped to from entry.s
extern "C" void kernel_main(const uint32_t multiboot_magic,
                            void *multiboot_data) {
  terminal_init();

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

  terminal_push_cursor_state(0, 19, terminal_colour_t::GREEN,
                             terminal_colour_t::BLACK);
  klog("Welcome to os blah blah blah blah blah blah blah blah "
       "blahhhhhhh...george you've done this line wrapping beautifully");
  klog_debug("Hello debug");
  klog_warn("hello warn");
  klog_err("Hello error");
  klog_crit("Hello crit");
  multiboot_info::init(multiboot_data);
  page_table::init();
  idt_init();
  isr_init();
  irq_init();
  pic_init();
  uint64_t rip = 0;
  asm volatile("leaq (%%rip), %0" : "=r"(rip));

  // terminal_printf("%s: %x\n", "Current instruction pointer is around", rip);
  // terminal_printf("%s: %x\n", "Multiboot data is at", multiboot_data);

  out<uint8_t>(0x21, 0xfd);
  out<uint8_t>(0xa1, 0xff);

  enable_interrupts();

  for (int i = -11; i < 11; ++i) {
    // terminal_printf("%d,", i);
  }

  // terminal_printf("\n");
  // terminal_printf("%s: %x\n", "link_kern_start", &link_kern_start);
  // terminal_printf("%s: %x\n", "link_kern_end", &link_kern_end);

  while (true) {
  }
}
