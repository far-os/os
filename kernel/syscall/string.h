#include "../text.h"
#include "../util.h"
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
    c -> ecx = (unsigned int) x;
    break;

  // Set cursor position
  case 0x02:
    set_cur(c -> ecx & 0xffff);
    break;

  // Insert special character
  case 0x03:
    void *xf = &(c -> edx);
    if (bittest(xf, 0)) line_feed();
    if (bittest(xf, 1)) carriage_return();
    if (bittest(xf, 2)) tab();
    break;


  // Write character at cursor
  case 0x04:
    char ch, fmt;
    asm volatile ("movb %%ch, %%dl" : "=d" (ch), "=c" (fmt) : "c" (c -> ebx));
    write_cell_cur(ch, fmt); // ch, cl
    break;

  // Write string at cursor
  case 0x05:
    fmt = c -> ebx & 0xff;
    void *address = c -> esi;
    write_str(adj(address), fmt);
    break;

  // Clear Screen
  case 0x06:
    clear_scr();
    break;

  // Clear specific line of screen
  case 0x07:
    clear_ln(c -> edx & 0xff);
    break;

  // Scroll entire screen
  case 0x08:
    scroll_scr();
    break;
  
  // Write character at specific location
  case 0x09:
    asm volatile ("movb %%ch, %%dl" : "=d" (ch), "=c" (fmt) : "c" (c -> ebx));
    write_cell(ch, c -> ecx & 0xffff, fmt); // ch, cl
    break;

  // Write string at specific location
  case 0x0a:
    fmt = c -> ebx & 0xff;
    address = c -> esi;
    write_str_at(adj(address), c -> ecx & 0xffff, fmt);

  default:
    break;
  }
}

#endif
