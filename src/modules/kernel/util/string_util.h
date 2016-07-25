#pragma once

namespace string_util {
// finds the first occurrence of ch in str, returning a pointer to this
// occurrence or to the end of the string if not found
const char *find(const char *str, char ch);

// finds the first occurrence of ch in str, returning a pointer to the
// character _after_ the occurrence
// if ch == '\0', then this is equivalent to find
const char *find_after(const char *str, char ch);

// finds the last occurrence of ch in str, returning a pointer to this
// occurrence or to the end of the string if not found
const char *find_last(const char *str, char ch);

// finds the last occurrence of ch in str, returning a pointer to the
// character _after_ the occurrence
// if ch == '\0', then this is equivalent to find
const char *find_last_after(const char *str, char ch);
}
