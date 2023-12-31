#include "../util.h"

#ifndef SYS_UTILFUNC_H
#define SYS_UTILFUNC_H

// utilfunc Service
void sys_2(struct cpu_state *c, unsigned char rout) {
  switch (rout) {
  // itoa
  case 0x00:
    to_dec(c -> edx, adj(c -> edi));
    break;

  default:
    break;
  }
}

#endif
