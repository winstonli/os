#include "cmos.h"
#include "pit.h"
#include "registers.h"
#include "terminal.h"

// see http://wiki.osdev.org/CMOS#Reading_All_RTC_Time_and_Date_Registers

#define CMOS_ADDRESS 0x70
#define CMOS_DATA 0x71

#define CMOS_SECS_REG 0x00
#define CMOS_MINS_REG 0x02
#define CMOS_HOURS_REG 0x04
#define CMOS_WEEKDAY_REG 0x06
#define CMOS_DAY_REG 0x07
#define CMOS_MONTH_REG 0x08
#define CMOS_YEAR_REG 0x09

#define CMOS_STATUS_REG_A 0x0a
#define CMOS_STATUS_REG_B 0x0b

#define CMOS_BCD_MASK 0x04     // datetime is encoded using bcd
#define CMOS_12HOUR_MASK 0x02  // datetime is encoded using 12-hour time
#define CMOS_UPDATE_IN_PROGRESS_MASK 0x80  // bit 7

struct cmos_date_t {
  uint8_t second;
  uint8_t minute;
  uint8_t hour;
  uint8_t weekday;
  uint8_t day;
  uint8_t month;
  uint16_t year;

  bool operator==(const cmos_date_t &right) const {
    return second == right.second && minute == right.minute &&
           hour == right.hour && weekday == right.weekday && day == right.day &&
           month == right.month && year == right.year;
  }

  bool operator!=(const cmos_date_t &right) const { return !(*this == right); }
};

static uint8_t read_rtc_reg(uint8_t reg) {
  out<uint8_t>(CMOS_ADDRESS, reg);
  return in<uint8_t>(CMOS_DATA);
}

static bool is_update_in_progress() {
  return read_rtc_reg(CMOS_STATUS_REG_A) & CMOS_UPDATE_IN_PROGRESS_MASK;
}

cmos_date_t read_rtc_unchecked() {
  cmos_date_t ret;
  ret.second = read_rtc_reg(CMOS_SECS_REG);
  ret.minute = read_rtc_reg(CMOS_MINS_REG);
  ret.hour = read_rtc_reg(CMOS_HOURS_REG);
  ret.weekday = read_rtc_reg(CMOS_WEEKDAY_REG);
  ret.day = read_rtc_reg(CMOS_DAY_REG);
  ret.month = read_rtc_reg(CMOS_MONTH_REG);
  ret.year = read_rtc_reg(CMOS_YEAR_REG);
  return ret;
}

uint8_t convert_bcd(uint8_t x) { return (x & 0x0F) + ((x / 16) * 10); }

cmos_date_t read_rtc() {
  while (is_update_in_progress())
    ;  // wait until update has finished
  cmos_date_t datetime = read_rtc_unchecked();
  cmos_date_t next_datetime;
  do {
    next_datetime = read_rtc_unchecked();
  } while (next_datetime != datetime);  // repeat until settles

  auto status_b = read_rtc_reg(CMOS_STATUS_REG_B);

  if (!(status_b & CMOS_BCD_MASK)) {
    // rtc is encoded using bcd rather than binary, so convert
    datetime.second = convert_bcd(datetime.second);
    datetime.minute = convert_bcd(datetime.minute);
    datetime.hour = convert_bcd(datetime.hour) | (datetime.hour & 0x80);
    datetime.weekday = convert_bcd(datetime.weekday);
    datetime.day = convert_bcd(datetime.day);
    datetime.month = convert_bcd(datetime.month);
    datetime.year = convert_bcd(datetime.year);
  }

  // if cmos is using 12-hour time and hour has bit 7 set, then add 12
  if (!(status_b & CMOS_12HOUR_MASK) && (datetime.hour & 0x80)) {
    datetime.hour = ((datetime.hour & 0x7F) + 12) % 24;
  }

  return datetime;
}

void cmos_handler(const registers_t *regs UNUSED) {
  auto datetime = read_rtc();
  terminal_push_cursor_state(0, 0, terminal_colour_t::BLACK,
                             terminal_colour_t::WHITE);
  terminal_printf("%02d:%02d:%02d %02d/%02d/%02d\n", datetime.hour,
                  datetime.minute, datetime.second, datetime.day,
                  datetime.month, datetime.year);
  terminal_pop_cursor_state();
}

void cmos::init() { pit::register_periodic(&cmos_handler); }
