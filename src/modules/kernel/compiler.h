#pragma once

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#define ffs(x) __builtin_ffs(x)
#define clz(x) __builtin_clz(x)
/* Count trailing zeros: fast way to get index of the first 1-bit */
#define ctz(x) __builtin_ctz(x)
