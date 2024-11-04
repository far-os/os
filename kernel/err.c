#include "include/err.h"
#include "include/text.h"
#include "include/memring.h"
#include "include/util.h"

char msg_symbs[5] = {
  0x1a, // -> symbol
  '+',
  '!',
  0x13, // !! symbol
  0xad, // ¡ symbol
};

void msg(enum MSG_TYPE type, enum ERRSIG sig, char* supp) {
  if (get_cur() % VGA_WIDTH) { line_feed(); }; // start on a new line
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

  if (sig && (type != PANIC)) {
    char *sigbuf = malloc(12);
    to_dec(sig, sigbuf);
    short pos = ((get_cur() / VGA_WIDTH) + 1) * VGA_WIDTH;
    pos -= strlen(sigbuf);
    write_str_at(sigbuf, pos, msg_style);
    free(sigbuf);
  }

  line_feed();

  if (type == PANIC) {
    asm volatile("cli; hlt"); // STOP NOW
  }
}
