#include <common/common.h>
#include <common/interrupts.h>
#include <common/multiboot2.h>

#include <apic.h>
#include <assert.h>
#include <cmos.h>
#include <idt.h>
#include <irq.h>
#include <isr.h>
#include <kernel.h>
#include <keyboard.h>
#include <log.h>
#include <pci.h>
#include <pic.h>
#include <pit.h>
#include <terminal.h>

#include <boot/multiboot_info.h>
#include <util/fixedsize_vector.h>
#include <vm/vm.h>

extern int64_t link_kern_start;
extern int64_t link_kern_end;

// entry point of 64-bit kernel proper, as jumped to from entry.s
extern "C" void kernel_main(const uint32_t multiboot_magic,
                            void *multiboot_data, void *start_mod_start,
                            void *start_mod_end) {
  terminal::init();
  klog("Welcome to os");

  assert(multiboot_magic == MULTIBOOT2_BOOTLOADER_MAGIC);

  kernel k(*static_cast<multiboot_info *>(multiboot_data), start_mod_start,
           start_mod_end,
           vm::kvaddr_to_paddr(static_cast<void *>(&link_kern_start)),
           vm::kvaddr_to_paddr(static_cast<void *>(&link_kern_end)));
  idt::init();
  isr::init();
  irq::init();
  pit::init();
  cmos::init();
  keyboard::init();
  pic::init();
  pci::init();
  apic::init();

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
