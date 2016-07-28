#include "pci.h"
#include "common/common.h"
#include "log.h"

#define PCI_VENDOR_ID_OFFSET 0x0
#define PCI_DEVICE_ID_OFFSET 0x2
#define PCI_COMMAND_OFFSET 0x4
#define PCI_STATUS_OFFSET 0x6
#define PCI_REVISION_ID_OFFSET 0x8
#define PCI_PROG_IF_OFFSET 0x8
#define PCI_REVISION_ID_MASK 0x00ff
#define PCI_PROG_IF_MASK 0xff00
#define PCI_SUBCLASS_OFFSET 0xa
#define PCI_CLASS_CODE_OFFSET 0xa
#define PCI_SUBCLASS_MASK 0x00ff
#define PCI_CLASS_CODE_MASK 0xff00

#define PCI_HEADER_TYPE_OFFSET 0xe
#define PCI_HEADER_TYPE_MASK 0x00ff

#define PCI_SECONDARY_BUS_OFFSET 0x18
#define PCI_SECONDARY_BUS_MASK 0xff00

// see http://wiki.osdev.org/PCI#Configuration_Space_Access_Mechanism_.231
uint16_t pci_config_read_word(uint8_t bus, uint8_t device, uint8_t function,
                              uint8_t offset) {
  uint32_t lbus = (uint32_t)bus;
  uint32_t ldevice = (uint32_t)device;
  uint32_t lfunction = (uint32_t)function;

  // create configuration address as per Figure 1
  auto address = (uint32_t)((lbus << 16) | (ldevice << 11) | (lfunction << 8) |
                            (offset & 0xfc) | ((uint32_t)0x80000000));

  // write out the address */
  out<uint32_t>(0xCF8, address);
  // read in the data
  // (offset & 2) * 8) = 0 will choose the first word of the 32 bits register
  return in<uint32_t>(0xCFC) >> ((offset & 2) * 8);
}

uint16_t pci_get_vendor_id(uint8_t bus, uint8_t device, uint8_t function) {
  return pci_config_read_word(bus, device, function, PCI_VENDOR_ID_OFFSET);
}

uint8_t pci_get_base_class(uint8_t bus, uint8_t device, uint8_t function) {
  auto c = pci_config_read_word(bus, device, function, PCI_CLASS_CODE_OFFSET);
  return c & PCI_CLASS_CODE_MASK;
}

uint8_t pci_get_subclass(uint8_t bus, uint8_t device, uint8_t function) {
  auto c = pci_config_read_word(bus, device, function, PCI_SUBCLASS_OFFSET);
  return c & PCI_SUBCLASS_MASK;
}

uint8_t pci_get_header_type(uint8_t bus, uint8_t device, uint8_t function) {
  auto c = pci_config_read_word(bus, device, function, PCI_HEADER_TYPE_OFFSET);
  return c & PCI_HEADER_TYPE_MASK;
}

uint8_t pci_get_secondary_bus(uint8_t bus, uint8_t device, uint8_t function) {
  auto c =
      pci_config_read_word(bus, device, function, PCI_SECONDARY_BUS_OFFSET);
  return c & PCI_SECONDARY_BUS_MASK;
}

static void pci_check_bus(uint8_t bus);

static void pci_check_function(uint8_t bus, uint8_t device, uint8_t function) {
  uint8_t base_class = pci_get_base_class(bus, device, function);
  uint8_t subclass = pci_get_subclass(bus, device, function);
  if ((base_class == 0x06) && (subclass == 0x04)) {
    uint8_t secondary_bus = pci_get_secondary_bus(bus, device, function);
    pci_check_bus(secondary_bus);
  }
}

static void pci_check_device(uint8_t bus, uint8_t device) {
  auto vendor_id = pci_get_vendor_id(bus, device, 0);
  if (vendor_id == 0xFFFF) {
    // device doesn't exist
    return;
  }
  pci_check_function(bus, device, 0);
  auto header_type = pci_get_header_type(bus, device, 0);
  if ((header_type & 0x80) != 0) {
    // it is a multi-function device, so check remaining functions
    for (uint8_t function = 1; function < 8; function++) {
      if (pci_get_vendor_id(bus, device, function) != 0xFFFF) {
        pci_check_function(bus, device, function);
      }
    }
  }
}

static void pci_check_bus(uint8_t bus) {
  for (uint8_t device = 0; device < 32; device++) {
    pci_check_device(bus, device);
  }
}

static void pci_check_all_buses() {
  auto header_type = pci_get_header_type(0, 0, 0);
  if ((header_type & 0x80) == 0) {
    // single host controller
    pci_check_bus(0);
  } else {
    // multiple host controllers
    for (uint8_t function = 0; function < 8; function++) {
      if (pci_get_vendor_id(0, 0, function) != 0xFFFF) {
        break;
      }
      auto bus = function;
      pci_check_bus(bus);
    }
  }
}

void pci::init() {
  klog_debug("Scanning for PCI devices...");
  pci_check_all_buses();
}
