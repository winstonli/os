#include "string_util.h"

const char *string_util::find(const char *str, char ch) {
  for (; *str != '\0' && *str != ch; ++str)
    ;
  return str;
}

const char *string_util::find_after(const char *str, char ch) {
  auto ret = find(str, ch);
  return *ret == '\0' ? ret : ret + 1;
}

const char *string_util::find_last(const char *str, char ch) {
  auto ret = find_after(str, ch);
  auto next = ret;
  while (*next != '\0') {
    ret = next;
    next = find_after(ret, ch);
  }
  return ret;
}

const char *string_util::find_last_after(const char *str, char ch) {
  auto ret = find_last(str, ch);
  return *ret == '\0' ? ret : ret + 1;
}
