#pragma once

#include <common/common.h>

#include <compiler.h>
#include <log.h>

#define assert_debug(cond, msg, ...) assert(cond, msg, ##__VA_ARGS__)

#define assertf(cond, msg, ...)                                                \
  do {                                                                         \
    if (unlikely(!cond)) {                                                     \
      klog_crit("assertion failure: " #cond ", " msg, ##__VA_ARGS__);          \
      panic();                                                                 \
    }                                                                          \
  } while (false)

#define assert(cond)                                                           \
  do {                                                                         \
    if (unlikely(!cond)) {                                                     \
      klog_crit("assertion failure: " #cond);                                  \
      panic();                                                                 \
    }                                                                          \
  } while (false)

void panic() NORETURN;
