#include "text.h"
#include "kbd.h"
#include "defs.h"
#include "syscall.h"

#ifndef IH_H
#define IH_H

void eh_c(struct cpu_state c, unsigned int i, struct stack_state s) {
  switch (i) {
  case 0x05: // #BR - Bound
    c.eax = 2; // returns an error code, indicating the bound was not in range
    if (s.cs != 0x08) { // if not called by kernel
      s.eip = prog_head -> eh_ptr; // set eip to error handler
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
