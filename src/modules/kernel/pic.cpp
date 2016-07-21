#include "pic.h"
#include "common/common.h"
#include "common/stdint.h"

/* reinitialize the PIC controllers, giving them specified vector offsets
   rather than 8h and 70h, as configured by default */

#define ICW1_ICW4 0x01      /* ICW4 (not) needed */
#define ICW1_SINGLE 0x02    /* Single (cascade) mode */
#define ICW1_INTERVAL4 0x04 /* Call address interval 4 (8) */
#define ICW1_LEVEL 0x08     /* Level triggered (edge) mode */
#define ICW1_INIT 0x10      /* Initialization - required! */

#define ICW4_8086 0x01       /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO 0x02       /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE 0x08  /* Buffered mode/slave */
#define ICW4_BUF_MASTER 0x0C /* Buffered mode/master */
#define ICW4_SFNM 0x10       /* Special fully nested (not) */

#define PIC1_OFFSET 0x20 // new offset for master (exceptions end at 0x1f)
#define PIC2_OFFSET 0x28 // new offset for slave

void pic_init() {
  unsigned char a1, a2;

  a1 = in<uint8_t>(PIC1_DATA); // save masks
  a2 = in<uint8_t>(PIC2_DATA);

  // starts the initialization sequence (in cascade mode)
  out<uint8_t>(PIC1_COMMAND, ICW1_INIT + ICW1_ICW4);
  io_wait();
  out<uint8_t>(PIC2_COMMAND, ICW1_INIT + ICW1_ICW4);
  io_wait();
  out<uint8_t>(PIC1_DATA, PIC1_OFFSET); // ICW2: Master PIC vector offset
  io_wait();
  out<uint8_t>(PIC2_DATA, PIC2_OFFSET); // ICW2: Slave PIC vector offset
  io_wait();
  // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
  out<uint8_t>(PIC1_DATA, 4);
  io_wait();
  // ICW3: tell Slave PIC its cascade identity (0000 0010)
  out<uint8_t>(PIC2_DATA, 2);
  io_wait();

  out<uint8_t>(PIC1_DATA, ICW4_8086);
  io_wait();
  out<uint8_t>(PIC2_DATA, ICW4_8086);
  io_wait();

  out<uint8_t>(PIC1_DATA, a1); // restore saved masks.
  out<uint8_t>(PIC2_DATA, a2);
}