#include "text.h"

#ifndef ERR_H
#define ERR_H

enum MSG_TYPE {
  INFO = 0, // ->
  WARN = 1, // +
  PROGERR = 2, // !
  KERNERR = 3, // !!
  PANIC = 4, // x
};

char msg_symbs[5] = {
  0x1a, // -> symbol
  '+',
  '!',
  0x13, // !! symbol
  'x',
};

void msg(enum MSG_TYPE type, int sig, char* supp) {
  unsigned char msg_style;
  if (type == INFO) {
    msg_style = COLOUR(BLACK, B_CYAN);
  } else if (type == WARN) {
    msg_style = COLOUR(BLACK, B_YELLOW);
  } else {
    msg_style = COLOUR(BLACK, B_RED);
  }

  write_cell_cur(msg_symbs[type], msg_style);
  adv_cur();
  write_str(supp, msg_style);

  if (sig != 0) {
    char *sigbuf = malloc(12);
    to_dec(sig, sigbuf);
    short pos = ((get_cur() / VGA_WIDTH) + 1) * VGA_WIDTH;
    pos -= strlen(sigbuf);
    write_str_at(sigbuf, pos, msg_style);
    free(sigbuf);
  }
}

#endif
