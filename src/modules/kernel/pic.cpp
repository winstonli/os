#include "pic.h"
#include "common/common.h"

#include <stdint.h>

#define PIC1 0x20 /* IO base address for master PIC */
#define PIC2 0xA0 /* IO base address for slave PIC */

#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)

#define PIC_EOI 0x20 /* End-of-interrupt command code */

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

#define PIC2_OFFSET (PIC1_OFFSET + 0x8)  // new offset for slave

// see http://wiki.osdev.org/PIC#Initialisation
// and http://stackoverflow.com/questions/282983/setting-up-irq-mapping
void pic::init() {
  unsigned char a1, a2;

  a1 = in<uint8_t>(PIC1_DATA);  // save masks
  a2 = in<uint8_t>(PIC2_DATA);

  // starts the initialization sequence (in cascade mode)
  out<uint8_t>(PIC1_COMMAND, ICW1_INIT + ICW1_ICW4);
  io_wait();
  out<uint8_t>(PIC2_COMMAND, ICW1_INIT + ICW1_ICW4);
  io_wait();
  out<uint8_t>(PIC1_DATA, PIC1_OFFSET);  // ICW2: Master PIC vector offset
  io_wait();
  out<uint8_t>(PIC2_DATA, PIC2_OFFSET);  // ICW2: Slave PIC vector offset
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

  out<uint8_t>(PIC1_DATA, a1);  // restore saved masks.
  out<uint8_t>(PIC2_DATA, a2);
}

// see http://wiki.osdev.org/8259_PIC#End_of_Interrupt
void pic::send_eoi(uint8_t irq) {
  if (irq >= 8) {
    // TODO: we do not handle the case of a spurious irq, in which case we must
    // NOT send an eoi to the slave (but still send one to the master, see
    // http://wiki.osdev.org/8259_PIC#Handling_Spurious_IRQs
    out<uint8_t>(PIC2_COMMAND, PIC_EOI);
  }

  out<uint8_t>(PIC1_COMMAND, PIC_EOI);
}
