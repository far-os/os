#include "text.h"
#include "kbd.h"
#include "defs.h"
#include "syscall.h"

#ifndef IH_H
#define IH_H

void eh_c(struct cpu_state c, unsigned int i) {
  if (i < 0x20) {
    return; // general protection fault? sounds like a skill issue
  }

  switch (i) {
  case 0x20:
    countx++;
    break;
  case 0x21:
    read_kbd();
    break;
  case 0x33:
    syscall(&c);
    break;
  default:
//    write_hex(0x12ae0000 | i, VGA_WIDTH - 10);
    break;
  }

  pic_ack(i);


  //asm("cli"); // no more interrupts
  //asm("hlt"); // adios
}

#endif
