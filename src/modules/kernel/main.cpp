#include <common/common.h>
#include <common/interrupts.h>
#include <common/multiboot2.h>

#include <assert.h>
#include <cmos.h>
#include <idt.h>
#include <irq.h>
#include <isr.h>
#include <keyboard.h>
#include <log.h>
#include <pic.h>
#include <pit.h>
#include <terminal.h>

#include <boot/multiboot_info.h>
#include <util/fixedsize_vector.h>
#include <vm/vm.h>

// entry point of 64-bit kernel proper, as jumped to from entry.s
extern "C" void kernel_main(const uint32_t multiboot_magic,
                            void *multiboot_data, void *start_mod_start,
                            void *start_mod_end) {
  terminal::init();

  assert(multiboot_magic == MULTIBOOT2_BOOTLOADER_MAGIC);

  terminal::push_cursor_state(0, 19, terminal::colour_t::GREEN);
  klog(
      "Welcome to os blah blah blah blah blah blah blah blah "
      "blahhhhhhh...george you've done this line wrapping beautifully");
  klog_debug("Hello debug");
  klog_warn("hello warn");
  klog_err("Hello error");
  klog_crit("Hello crit");
  vm::init(*static_cast<multiboot_info *>(multiboot_data), start_mod_start,
           start_mod_end);
  idt::init();
  isr::init();
  irq::init();
  pit::init();
  cmos::init();
  keyboard::init();
  pic::init();
  uint64_t rip = 0;
  asm volatile("leaq (%%rip), %0" : "=r"(rip));

  // terminal_printf("%s: %x\n", "Current instruction pointer is around", rip);
  // terminal_printf("%s: %x\n", "Multiboot data is at", multiboot_data);

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
