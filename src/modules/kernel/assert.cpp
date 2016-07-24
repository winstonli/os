#include "assert.h"

#include <halt.h>

void panic() {
  klog_crit("KERNEL PANIC");
  halt();
}
