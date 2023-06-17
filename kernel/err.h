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
}

#endif
