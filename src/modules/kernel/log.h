#pragma once

#include <terminal.h>
#include <util/string_util.h>

#define klog(msg, ...) klog_info(msg, ##__VA_ARGS__)

#define klog_debug(msg, ...)                                                   \
  klog_colour("DEBUG", terminal_colour_t::LIGHT_GREEN, msg, ##__VA_ARGS__)

#define klog_info(msg, ...)                                                    \
  klog_colour("INFO", terminal_colour_t::WHITE, msg, ##__VA_ARGS__)

#define klog_warn(msg, ...)                                                    \
  klog_colour("WARN", terminal_colour_t::MAGENTA, msg, ##__VA_ARGS__)

#define klog_err(msg, ...)                                                     \
  klog_colour("ERROR", terminal_colour_t::RED, msg, ##__VA_ARGS__)

#define klog_crit(msg, ...)                                                    \
  klog_colour("CRITICAL", terminal_colour_t::BROWN, msg, ##__VA_ARGS__)

#define klog_colour(loglevel, colour, msg, ...)                                \
  do {                                                                         \
    terminal_set_colour(terminal_colour_t::LIGHT_GRAY,                         \
                        terminal_colour_t::BLACK);                             \
    terminal_printf("[");                                                      \
    terminal_set_colour(terminal_colour_t::GREEN, terminal_colour_t::BLACK);   \
    terminal_printf("%s:%d", string_util::trim_after(__FILE__, '/'),           \
                    __LINE__);                                                 \
    terminal_set_colour(terminal_colour_t::LIGHT_GRAY,                         \
                        terminal_colour_t::BLACK);                             \
    terminal_printf("]");                                                      \
    terminal_set_colour(colour, terminal_colour_t::BLACK);                     \
    terminal_printf(" " loglevel " ");                                         \
    terminal_set_colour(terminal_colour_t::LIGHT_GRAY,                         \
                        terminal_colour_t::BLACK);                             \
    terminal_printf(msg "\n", ##__VA_ARGS__);                                  \
  } while (false)

#define klog_clear()                                                           \
  do {                                                                         \
    terminal_clear();                                                          \
  } while (false)
