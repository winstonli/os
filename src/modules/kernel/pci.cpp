#include "pci.h"
#include "common/common.h"
#include "log.h"

#define PCI_VENDOR_ID_OFFSET 0x0
#define PCI_DEVICE_ID_OFFSET 0x2
#define PCI_COMMAND_OFFSET 0x4
#define PCI_STATUS_OFFSET 0x6
#define PCI_REVISION_ID_OFFSET 0x8
#define PCI_REVISION_ID_MASK 0x00ff
#define PCI_REVISION_ID_SHIFT 0
#define PCI_PROG_IF_OFFSET 0x8
#define PCI_PROG_IF_MASK 0xff00
#define PCI_PROG_IF_SHIFT 8
#define PCI_SUBCLASS_OFFSET 0xa
#define PCI_SUBCLASS_MASK 0x00ff
#define PCI_SUBCLASS_SHIFT 0
#define PCI_CLASS_CODE_OFFSET 0xa
#define PCI_CLASS_CODE_MASK 0xff00
#define PCI_CLASS_CODE_SHIFT 8

#define PCI_HEADER_TYPE_OFFSET 0xe
#define PCI_HEADER_TYPE_MASK 0x00ff
#define PCI_HEADER_TYPE_SHIFT 0

#define PCI_SECONDARY_BUS_OFFSET 0x18
#define PCI_SECONDARY_BUS_MASK 0xff00
#define PCI_SECONDARY_BUS_SHIFT 8

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

uint8_t pci_get_prog_if(uint8_t bus, uint8_t device, uint8_t function) {
  auto c = pci_config_read_word(bus, device, function, PCI_PROG_IF_OFFSET);
  return (c & PCI_PROG_IF_MASK) >> PCI_PROG_IF_SHIFT;
}

uint8_t pci_get_base_class(uint8_t bus, uint8_t device, uint8_t function) {
  auto c = pci_config_read_word(bus, device, function, PCI_CLASS_CODE_OFFSET);
  return (c & PCI_CLASS_CODE_MASK) >> PCI_CLASS_CODE_SHIFT;
}

uint8_t pci_get_subclass(uint8_t bus, uint8_t device, uint8_t function) {
  auto c = pci_config_read_word(bus, device, function, PCI_SUBCLASS_OFFSET);
  return (c & PCI_SUBCLASS_MASK) >> PCI_SUBCLASS_SHIFT;
}

uint8_t pci_get_header_type(uint8_t bus, uint8_t device, uint8_t function) {
  auto c = pci_config_read_word(bus, device, function, PCI_HEADER_TYPE_OFFSET);
  return (c & PCI_HEADER_TYPE_MASK) >> PCI_HEADER_TYPE_SHIFT;
}

uint8_t pci_get_secondary_bus(uint8_t bus, uint8_t device, uint8_t function) {
  auto c =
      pci_config_read_word(bus, device, function, PCI_SECONDARY_BUS_OFFSET);
  return (c & PCI_SECONDARY_BUS_MASK) >> PCI_SECONDARY_BUS_SHIFT;
}

static const char* pci_get_device_type_str(uint8_t base_class, uint8_t subclass,
                                           uint8_t prog_if);
static void pci_check_bus(uint8_t bus);

static void pci_check_function(uint8_t bus, uint8_t device, uint8_t function) {
  auto base_class = pci_get_base_class(bus, device, function);
  auto subclass = pci_get_subclass(bus, device, function);
  auto prog_if = pci_get_prog_if(bus, device, function);
  auto device_type_str = pci_get_device_type_str(base_class, subclass, prog_if);
  klog_debug("Found device (code %x/%x/%x) (%s)", base_class, subclass, prog_if,
             device_type_str);
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

const char* pci_get_device_type_str(uint8_t base_class, uint8_t subclass,
                                    uint8_t prog_if) {
  uint16_t combined16 = (static_cast<uint16_t>(base_class) << 8) | subclass;
  uint32_t combined32 = (static_cast<uint32_t>(combined16) << 8) | prog_if;
  switch (combined32) {
    case 0x000100:
      return "VGA-Compatible Device";
    case 0x010601:
      return "SATA (AHCI 1.0)";
    case 0x020000:
      return "Ethernet Controller";
    case 0x030000:
      return "VGA-Compatible Controller";
    case 0x060000:
      return "Host Bridge";
    case 0x060100:
      return "ISA Bridge";
    case 0x060400:
    case 0x060401:
      return "PCI-PCI Bridge";
    case 0x0c0300:
      return "USB Universal Host Controller";
    case 0x0c0310:
      return "USB Open Host Controller";
    case 0x0c0320:
      return "USB2 Host Controller";
    case 0x0c0330:
      return "USB3 XHCI Controller";
    case 0x0c0500:
      return "System Management Bus";
  }
  switch (combined16) {
    case 0x0101:
      return "IDE Controller";
    case 0x0106:
      return "SATA";
    case 0x0c03:
      return "USB Controller";
  }
  switch (base_class) {
    case 0x01:
      return "Mass Storage Controller";
    case 0x02:
      return "Network Controller";
    case 0x03:
      return "Display Controller";
    case 0x04:
      return "Multimedia Device";
    case 0x05:
      return "Memory Controller";
    case 0x06:
      return "Bridge Device";
    case 0x07:
      return "Communications Device";
    case 0x08:
      return "System Peripheral";
    case 0x09:
      return "Input Controller";
    case 0x0a:
      return "Docking Station";
    case 0x0b:
      return "Co-Processor";
    case 0x0d:
      return "Wireless Controller";
  }
  return "Unknown";
}
