#include "checksum.h"

uint64_t checksum(const void *d, uint64_t size) {
  auto data = reinterpret_cast<const uint8_t *>(d);
  uint64_t res = 0;
  for (; size-- > 0; ++data) {
    res += *data;
  }
  return res;
}
