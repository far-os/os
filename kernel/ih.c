#include "include/ih.h"
#include "include/text.h"
#include "include/kbd.h"
#include "include/syscall.h"
#include "include/cmos.h"
#include "include/misc.h"
#include "include/pic.h"

unsigned char quitting_prog = 0;

static inline void retto_progeh(struct stack_state *s) {
  if (s -> cs != 0x08) { // if not called by kernel
    s -> eip = prog_head -> eh_ptr; // set eip to error handler
  }
}

void eh_c(struct cpu_state c, unsigned int i, struct stack_state s) {
  switch (i) {
  case 0x05: // #BR - Bound
    c.eax = 7; // returns an error code, indicating the bound was not in range
    retto_progeh(&s);
    break;
  case 0x06: // #UD - Illegal instruction
    c.eax = 9;
    retto_progeh(&s);
    break;
  case 0x20: // timer
    if (!(++countx % 100)) {
      adv_time(curr_time);
    } // increment centisecond counter
    break;
  case 0x21: // PS/2 keyboard
    read_kbd(); // send to the keyboard
    if (quitting_prog) {
      quitting_prog = 0;
      retto_progeh(&s);
    }
    break;
  case 0x33: // SYSCALL
    syscall(&c); // tell kernel about syscall
    break;
  default:
    break;
  }

  pic_ack(i);
}
