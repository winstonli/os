#include "ioapic.h"
#include "log.h"

void ioapic::init() {
  klog_debug("Initialising IO APIC");
  // TODO: we cannot actually initialise this until we know the base address
  // of the io apic, which we must retreive from the acpi tables
}
