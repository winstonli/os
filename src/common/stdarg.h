#pragma once

// conveniently gcc and clang use the same names for va_list related builtin
// functions and types, so we just steal them here rather than needing to do
// some preprocessor magic

typedef __builtin_va_list va_list;

#define va_start(ap, param) __builtin_va_start(ap, param)
#define va_end(ap) __builtin_va_end(ap)
#define va_arg(ap, type) __builtin_va_arg(ap, type)
