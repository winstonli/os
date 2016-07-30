#include <common/common.h>
#include <common/interrupts.h>
#include <common/multiboot2.h>

#include <acpi.h>
#include <assert.h>
#include <cmos.h>
#include <idt.h>
#include <ioapic.h>
#include <irq.h>
#include <isr.h>
#include <kernel.h>
#include <keyboard.h>
#include <lapic.h>
#include <log.h>
#include <pci.h>
#include <pic.h>
#include <pit.h>
#include <terminal.h>

#include <boot/multiboot_info.h>
#include <util/emb_list.h>
#include <util/fixedsize_vector.h>
#include <vm/vm.h>

#ifdef RUN_TESTS
#include <test_main.h>
#endif

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
  acpi::init();
  lapic::init();
  ioapic::init();

  uint64_t rip = 0;
  asm volatile("leaq (%%rip), %0" : "=r"(rip));

  // terminal_printf("%s: %x\n", "Current instruction pointer is around", rip);
  // terminal_printf("%s: %x\n", "Multiboot data is at", multiboot_data);

  enable_interrupts();

#ifdef RUN_TESTS
  test_get_content(0, nullptr);
#endif

  // terminal_printf("\n");
  // terminal_printf("%s: %x\n", "link_kern_start", &link_kern_start);
  // terminal_printf("%s: %x\n", "link_kern_end", &link_kern_end);

  while (true) {
  }
}
