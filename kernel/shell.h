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
  char *headbuf = "FarSH. (c) 2022.\n"; // the underscores are placeholder for the memcpy
  write_str(headbuf, COLOUR(BLUE, B_RED));
//  write_hex(buf, -1);
  
  comupd();

  while (1) {
    asm volatile ("sti; hlt");
  }
}

#define OUT_LEN VGA_WIDTH
char outbuf[OUT_LEN];

void shexec() {
  memzero(outbuf, OUT_LEN);
  if (strcmp(combuf, "info")) {
    strcpy("FarOS v0.0.1.\n\tVol. label \"________________\"", outbuf);
    memcpy(&(csdfs -> label), outbuf + 27, 16); // memcpy because we need to control the length
  } else if (strcmp(combuf, "reset")) {
    cpu_reset();
  } else if (strcmp(combuf, "clear")) {
    clear_scr();
    set_cur(POS(0, 0));
    return;
  } else {
    strcpy(combuf, outbuf);
  }
  write_str(outbuf, COLOUR(RED, B_YELLOW));
  line_feed();
}

void comupd() {
  if (strlen(combuf) >= COM_LEN) {
    write_str(" - FATAL: Command too long\n", COLOUR(BLACK, B_RED));
    memzero(combuf, COM_LEN);
  }

  int comlen = strlen(combuf);

  switch (combuf[comlen - 1]) {
  case '\b':
    combuf[comlen - 2] = '\0';
    combuf[comlen - 1] = '\0';
    break;
  case '\n':
    line_feed();
    combuf[comlen - 1] = '\0';

    shexec();
    memzero(combuf, COM_LEN);
    break;
  }

  char printbuf[20] = "\r!> ";
  strcpy(combuf, printbuf + 4);
  write_str(printbuf, COLOUR(BLACK, WHITE));
}

#endif
