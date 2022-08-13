#include "text.h"
// #include "kbd.h"
#include "util.h"
#include "fs.h"

#ifndef SHELL_H
#define SHELL_H

#define COM_LEN 16

char combuf[COM_LEN]; 

void shell() {
//  set_cur(POS(0, 1)); // new line
  char *headbuf = "FarSH\nVolume label \"________________\"\n"; // the underscores are placeholder for the memcpy
  memcpy(&(csdfs -> label), headbuf + 20, 16); // memcpy the vol label to the string
  write_str(headbuf, COLOUR(BLUE, B_RED));
//  write_hex(buf, -1);
  
  comupd();
}

void comupd() {
  if (strlen(combuf) == COM_LEN) {
    write_str("Command too long\n", COLOUR(BLACK, B_RED));
  }

  char printbuf[20] = "\r!> ";
  memcpy(combuf, printbuf + 4, strlen(combuf));
  write_str(printbuf, COLOUR(BLACK, WHITE));
}

#endif
