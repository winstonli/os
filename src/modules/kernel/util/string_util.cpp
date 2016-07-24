#include "string_util.h"

const char *string_util::trim_after(const char *str, char ch) {
  const char *ret = str;
  for (; *str != '\0'; ++str) {
    if (*str == ch) {
      ret = str;
    }
  }
  return ret + 1;
}
