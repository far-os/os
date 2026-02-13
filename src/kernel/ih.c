#include "include/ih.h"
#include "include/err.h"
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

// there was an exception.
// means loads ebx as jump to if fail, IF AND ONLY IF KERNEL.
static inline void jump_if_fail(struct cpu_state * c, struct stack_state *s) {
  if (s -> cs == 0x08) { // if called by kernel
    s -> eip = c -> ebx;
  }
}

void eh_c(struct cpu_state c, unsigned int i, struct stack_state s) {
  switch (i) {
  case 0x00: // #DE - Divide by zero error
    // TODO: make better.
    if (s.cs == 0x08) { // if from kernel (oh no very very bad)
      msg(PANIC, E_MATHS, "Divide by zero in kernel at %p", s.eip);
    } else {
      c.eax = E_MATHS;
    }
    break;
  case 0x05: // #BR - Bound
    c.eax = E_BOUND; // returns an error code, indicating the bound was not in range
    retto_progeh(&s);
    jump_if_fail(&c, &s);
    break;
  case 0x06: // #UD - Illegal instruction
    if (s.cs == 0x08) { // if from kernel (oh no very very bad)
      msg(PANIC, E_ILLEGAL, "Illegal instruction in kernel at %p", s.eip);
    } else {
      c.eax = E_ILLEGAL;
    }
    retto_progeh(&s);
    break;
  case 0x20: // timer
    if (!(++uptime % 100)) {
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
