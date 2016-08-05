#pragma once

#include <stdint.h>

namespace pci {

struct device_info_t {
  uint8_t bus;
  uint8_t device;
  uint8_t function;
  uint8_t class_code;
  uint8_t subclass;
  uint8_t prog_if;
};

class device_handler {
public:
  virtual void init_device(const device_info_t &info) = 0;
};

void init();

// pci::init must be called first
void register_handler(device_handler *handler);

// scan for any pci devices, calling the registered handlers in the process
void scan();

}
