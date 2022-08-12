#include "text.h"
#include "kbd.h"
#include "util.h"

#ifndef SHELL_H
#define SHELL_H

void shell() {
  set_cur(POS(0, 1)); // new line
  char *buf = "FarSH. Volume \"________________\"\n"; // the underscores are placeholder for the memcpy
  memcpy((void *) 0x19fc4, buf + 15, 16); // memcpy the vol label to the string
  write_str(buf, -1, COLOUR(BLUE, B_RED));  

  write_str("!> ", -1, COLOUR(BLACK, WHITE));
}

#endif
