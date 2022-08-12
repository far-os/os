#include "text.h"
#include "kbd.h"

#ifndef IH_H
#define IH_H

/*struct cpu_state {
    unsigned int eax;
    unsigned int ecx;
    unsigned int edx;
    unsigned int ebx;
    unsigned int esp;
    unsigned int ebp;
    unsigned int esi;
    unsigned int edi;
} __attribute__((packed)); // registers as returned from pushad

struct stack_state {
    unsigned int error_code;
    unsigned int eip;
    unsigned int cs;
    unsigned int eflags;
} __attribute__((packed)); // stack */

void eh_c(unsigned int i) {
  if (i < 0x20) {
    return; // general protection fault? sounds like a skill issue
  }

  switch (i) {
  case 0x21:
    read_kbd();
    break;
  default:
//    write_hex(0x12ae0000 | i, VGA_WIDTH - 10);
    break;
  }

  //asm("cli"); // no more interrupts
  //asm("hlt"); // adios
}

#endif
