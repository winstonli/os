#include <common/common.h>
#include <common/interrupts.h>
#include <common/multiboot2.h>

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

// 16-bit pointer to ebda start physical addr right-shifted 4 places
// (so shift left 4 to get actual physical addr)
#define EBDA_PTR_PADDR reinterpret_cast<void *>(0x040e)

static void *get_ebda_addr() {
  auto ebda_ptr_vaddr = vm::paddr_to_vaddr(EBDA_PTR_PADDR);
  auto ebda_paddr = *reinterpret_cast<uint16_t *>(ebda_ptr_vaddr) << 4;
  return vm::paddr_to_vaddr(reinterpret_cast<void *>(ebda_paddr));
}

static void search_for_mp(char *begin, char *end) {
  // check is 16-byte aligned
  assert(reinterpret_cast<uintptr_t>(begin) % 16 == 0);
  for (; begin < end; begin += 16) {
    if (!memcmp(begin, "_MP_", 4)) {
      klog_debug("Found MP Floating Pointer Structure at %x", begin);
    }
  }
}

static void search_for_rsdp(char *begin, char *end) {
  // check is 16-byte aligned
  assert(reinterpret_cast<uintptr_t>(begin) % 16 == 0);
  for (; begin < end; begin += 16) {
    if (!memcmp(begin, "RSD PTR ", 8)) {
      klog_debug("Found Root System Description Pointer at %x", begin);
    }
  }
}

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
  lapic::init();
  ioapic::init();

  auto ebda_addr = reinterpret_cast<char *>(get_ebda_addr());
  klog_debug("Extended BIOS data area is located at %x", ebda_addr);

  // places to search (only 16-byte boundaries):
  //  - ebda_addr to ebda_addr + 1024
  //  - 0xE0000 to 0xFFFFF

  auto bios_area_start = reinterpret_cast<char *>(
      vm::paddr_to_vaddr(reinterpret_cast<void *>(0xe0000)));
  auto bios_area_end = reinterpret_cast<char *>(
      vm::paddr_to_vaddr(reinterpret_cast<void *>(0xfffff)));

  search_for_rsdp(ebda_addr, ebda_addr + 1024);
  search_for_rsdp(bios_area_start, bios_area_end);

  search_for_mp(ebda_addr, ebda_addr + 1024);
  search_for_mp(bios_area_start, bios_area_end);

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
