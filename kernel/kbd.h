#include "port.h"
#include "text.h"

#ifndef KBD_H
#define KBD_H

#define K_PORT 0x60

void read_kbd() {
  unsigned char scan = pbyte_in(K_PORT);
  write_cell(scan, get_cur(), COLOUR(BLACK, WHITE));
  set_cur(get_cur() + 1);
}

#endif
