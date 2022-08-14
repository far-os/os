#include "port.h"

#ifndef PIC_H
#define PIC_H

#define P1_B 0x20 /* base address for PIC1's io */
#define P1_COM P1_B /* PIC1 command */
#define P1_DAT (P1_COM + 1) /* PIC1 data */

#define P2_B 0xA0 /* base address for PIC2's io */
#define P2_COM P2_B /* PIC2 command */
#define P2_DAT (P2_COM + 1) /* PIC2 data */

#define P1_I 0x20 /* start of PIC1's interrupts */
#define P2_I (P1_I + 0x08) /* start of PIC2's interrupts */

void pic_init() {
  pbyte_out(P1_COM, 0x11); // init with ICW4
  idle(); // wait for the PIC to do its thing
  pbyte_out(P2_COM, 0x11); // same but PIC2
  idle();

  pbyte_out(P1_DAT, 0x20); // hey guys the PIC interrupts are here now
  idle();
  pbyte_out(P2_DAT, 0x28);
  idle();
//
  pbyte_out(P1_DAT, 4); // tell PIC1 there is a slave PIC on interrupt 2
  idle();
  pbyte_out(P2_DAT, 2); // tell slave cascabe identity
  idle();

  pbyte_out(P1_DAT, 0x01); // we're an 8086 apparently
  idle();
  pbyte_out(P2_DAT, 0x01);
  idle();

  pbyte_out(P1_DAT, 0xFF); // mask ***everything***
  idle();
  pbyte_out(P2_DAT, 0xFF);
  idle();
}

void pic_ack(unsigned int i) { // acknowledge the interrupt
  if (i < P1_I || i > (P2_I + 7)) {
    return;
  }

  if (i < P2_I) {
    pbyte_out(P1_COM, 0x20); // acknowledge
  } else {
    pbyte_out(P2_COM, 0x20);
  }
}

void irq_m_free(unsigned char line) {
  unsigned short port;
  unsigned char value;

  if (line < 0x8) {
    port = P1_DAT;
  } else {
    port = P2_DAT;
    line -= 8;
  }
  value = pbyte_in(port) & ~(1 << line);
  pbyte_out(port, value);
}

#endif
