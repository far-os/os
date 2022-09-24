#include "text.h"
#include "kbd.h"

#ifndef IH_H
#define IH_H

struct cpu_state {
    unsigned int edi;
    unsigned int esi;
    unsigned int ebp;
    unsigned int esp;
    unsigned int ebx;
    unsigned int edx;
    unsigned int ecx;
    unsigned int eax;

//    unsigned int ss;
    unsigned int es;
    unsigned int ds;
} __attribute__((packed)); // registers as returned from pushad

/*struct stack_state {
    unsigned int error_code;
    unsigned int eip;
    unsigned int cs;
    unsigned int eflags;
} __attribute__((packed)); // stack */

void eh_c(struct cpu_state c, unsigned int i) {
  if (i < 0x20) {
    return; // general protection fault? sounds like a skill issue
  }

  switch (i) {
  case 0x21:
    read_kbd();
    break;
  case 0x33:
    switch (c.eax & 0xFFFF) {
    case 0x0000:
      char ch, fmt;
      asm volatile ("movb %%ch, %%dl" : "=d" (ch), "=c" (fmt) : "c" (c.ecx));
      write_cell_cur(ch, fmt); // ch, cl
      break;
    default:
      break;
    }
  default:
//    write_hex(0x12ae0000 | i, VGA_WIDTH - 10);
    break;
  }

  pic_ack(i);


  //asm("cli"); // no more interrupts
  //asm("hlt"); // adios
}

#endif
