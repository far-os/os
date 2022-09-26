#include "../text.h"
#include "../defs.h"

#ifndef SYS_STRING_H
#define SYS_STRING_H

// String Service 
void sys_0(struct cpu_state *c, unsigned char rout) {
  switch (rout) {

  // Move cursor forward
  case 0x00:
    adv_cur();
    break;

  // Get cursor position
  case 0x01:
    unsigned short x = get_cur();
    c -> ecx &= (unsigned int) x;
    break;

  // Set cursor position
  case 0x02:
    set_cur(c -> ecx & 0xffff);
    break;

  // Write Character at Cursor
  case 0x03:
    char ch, fmt;
    asm volatile ("movb %%ch, %%dl" : "=d" (ch), "=c" (fmt) : "c" (c -> ecx));
    write_cell_cur(ch, fmt); // ch, cl
    break;

  // Write String at Cursor
  case 0x04:
    fmt = c -> ecx & 0xff;
    void *address = c -> esi;
    address += 0x100000; // adjustment
    write_str(address, fmt);
    break;
  default:
    break;
  }
}

#endif
