#include "../util.h"
#include "../defs.h"

#ifndef SYS_UTIL_H
#define SYS_UTIL_H

void sys_1(struct cpu_state *c, unsigned char rout) {
  switch (rout) {
  case 0x00:
    unsigned char x = c -> ebx & 0xff;

    unsigned char *chptr = &(c -> ebx);
    chptr[1] = nybble_to_hex(x);

    break;

  default:
    break;
  }
}

#endif
