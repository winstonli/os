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
STATIC void (*handlers[MAX_HANDLERS])(const registers_t *);
STATIC uint8_t handler_count;

static void pit_handler(const registers_t *regs) {
  terminal_putchar('.');

  for (uint8_t i = 0; i < handler_count; ++i) {
    handlers[i](regs);
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

  out<uint8_t>(PIT_CHAN0_REG, static_cast<uint8_t>(freq_divisor & 0xff));
  out<uint8_t>(PIT_CHAN0_REG, static_cast<uint8_t>((freq_divisor >> 8) & 0xff));
}

void pit::init() {
  irq::register_handler(PIT_IRQ, &pit_handler);

  memzero(&handlers[0], MAX_HANDLERS);
  handler_count = 0;

  pit_start_counter(10, 0, pit_mode_t::SQUARE_WAVE);
}

void pit::register_periodic(void (*handler)(const registers_t *)) {
  assert(handler_count < MAX_HANDLERS);
  handlers[handler_count++] = handler;
}
