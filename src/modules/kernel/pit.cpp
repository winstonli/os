#include "pit.h"
#include "assert.h"
#include "common/string.h"
#include "irq.h"
#include "registers.h"
#include "terminal.h"

#define PIT_IRQ 0

#define PIT_CHAN0_REG 0x40
#define PIT_CHAN1_REG 0x41
#define PIT_CHAN2_REG 0x42
#define PIT_CMD_REG 0x43

#define PIT_ACCESS_16BIT 0x30  // access mode: low byte then high byte

#define MAX_HANDLERS 16

enum pit_handler_type_t { PIT_HANDLER_SINGLE, PIT_HANDLER_PERIODIC };

struct pit_handler_t {
  void (*fn_ptr)(const registers_t *);
  uint64_t reload_ticks;
  uint64_t remaining_ticks;
  pit_handler_type_t type;
};

STATIC pit_handler_t handlers[MAX_HANDLERS];
STATIC uint8_t handler_count;
STATIC uint16_t pit_frequency[3];  // frequency for each channel (hz)
STATIC volatile uint64_t ticks;

static void pit_handler(const registers_t *regs) {
  ++ticks;
  for (uint8_t i = 0; i < handler_count; ++i) {
    if (--handlers[i].remaining_ticks == 0) {
      handlers[i].remaining_ticks = handlers[i].reload_ticks;
      (*handlers[i].fn_ptr)(regs);
      if (handlers[i].type == PIT_HANDLER_SINGLE) {
        // move everything back one to erase expired handler
        for (uint8_t j = i; j < handler_count - 1; ++j) {
          handlers[j] = handlers[j + 1];
        }
        --i;
        --handler_count;
      }
    }
  }
}

enum class pit_mode_t {
  TERMINAL_COUNT = 0,
  ONE_SHOT = 0x2,
  RATE_GENERATOR = 0x4,
  SQUARE_WAVE = 0x6,
  SOFTWARE_STROBE = 0x8,
  HARDWARE_STROBE = 0xa
};

#define PIT_NATIVE_FREQ_HZ 1193182

static void pit_start_counter(uint16_t freq, uint8_t counter_id,
                              pit_mode_t mode) {
  assert(freq != 0);
  assert(counter_id < 3);

  klog_debug("starting counter (id=%d, freq=%d, mode=%d)", counter_id, freq,
             mode);

  uint32_t freq_divisor_32 = PIT_NATIVE_FREQ_HZ / freq;
  uint16_t freq_divisor = MIN(freq_divisor_32, (1 << 16) - 1);

  klog_debug("frequency divisor is %d", freq_divisor);

  uint8_t cmd =
      static_cast<uint8_t>(mode) | PIT_ACCESS_16BIT | (counter_id * 0x40);

  // set pit mode, access mode and counter to be used
  out<uint8_t>(PIT_CMD_REG, cmd);

  out<uint8_t>(PIT_CHAN0_REG + counter_id, (freq_divisor & 0xff));
  out<uint8_t>(PIT_CHAN0_REG + counter_id, ((freq_divisor >> 8) & 0xff));

  pit_frequency[counter_id] = freq;
}

void pit::init() {
  irq::register_handler(PIT_IRQ, &pit_handler);

  memzero(&handlers[0], MAX_HANDLERS);
  handler_count = 0;
  memzero(&pit_frequency[0], 3);
  ticks = 0;

  pit_start_counter(1000, 0, pit_mode_t::SQUARE_WAVE);
}

static void pit_register(void (*fn_ptr)(const registers_t *),
                         uint64_t interval_ms, pit_handler_type_t type) {
  assert(handler_count < MAX_HANDLERS);

  auto &handler = handlers[handler_count];
  handler.fn_ptr = fn_ptr;
  handler.reload_ticks = (interval_ms * pit_frequency[0]) / 1000;
  handler.remaining_ticks = handler.reload_ticks;
  handler.type = type;

  handler_count++;
}

void pit::register_periodic(void (*fn_ptr)(const registers_t *),
                            uint64_t interval_ms) {
  pit_register(fn_ptr, interval_ms, PIT_HANDLER_PERIODIC);
}

void pit::register_single(void (*fn_ptr)(const registers_t *),
                          uint64_t interval_ms) {
  pit_register(fn_ptr, interval_ms, PIT_HANDLER_SINGLE);
}

void pit::busy_wait(uint64_t interval_ms) {
  auto expected_ticks = (interval_ms * pit_frequency[0]) / 1000 + ticks;
  while (ticks < expected_ticks)
    ;
}
