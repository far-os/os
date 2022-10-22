#include "text.h"
#include "kbd.h"
#include "defs.h"
#include "syscall.h"

#ifndef IH_H
#define IH_H

unsigned char __seg_fs *prog_ip; // program instruction pointer

void eh_c(struct cpu_state c, unsigned int i, struct stack_state s) {
  switch (i) {
  case 0x05: // #BR - Bound
    c.eax = 2; // returns an error code, indicating the bound was not in range
    if (s.cs != 0x08) { // if not called by kernel
      prog_ip = s.eip; // set program instruction pointer to eip

      prog_ip[0] = 0xc9; // leave = 0xc9 - clean up
      prog_ip[1] = 0xcb; // retf = 0xcb - makes code return with error
    }
    break;
  case 0x20: // timer
    countx++; // increment millisecond counter
    break;
  case 0x21: // PS/2 keyboard
    read_kbd(); // send to the keyboard
    break;
  case 0x33: // SYSCALL
    syscall(&c); // tell kernel about syscall
    break;
  default:
    break;
  }

  pic_ack(i);

}

#endif
