#include "checksum.h"

uint64_t checksum(const uint8_t *data, uint64_t size) {
  uint64_t res = 0;
  for (; size-- > 0; ++data) {
    res += *data;
  }
  return res;
}
