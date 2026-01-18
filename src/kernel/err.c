#include "include/err.h"
#include "include/text.h"
#include "include/memring.h"
#include "include/util.h"
#include "include/config.h"

#include "include/printf.h"

// not really using standard library - stdarg just provides platform-dependent defines
#include <stdarg.h>

char msg_symbs[5] = {
  0x1a, // -> symbol
  '+',
  '!',
  0x13, // !! symbol
  0xad, // ¡ symbol
};

/// formatted message
void msg(enum MSG_TYPE type, enum ERRSIG sig, const char* supp, ...) {
  va_list args;
  va_start(args, supp); // second parameter is the last arg before variadic

  // not verbose enough
  if (type == INFO && xconf->verbosity < SHOW_INFO) return;

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

  // this is technically a formatted msg, so we need raw access
  vpfctprintf(&write_advanced_cell_cur, supp, msg_style, args);
  va_end(args); // no longer need args, get out!
  adv_cur();

  if (xconf -> verbosity >= SHOW_STACKTRACE) {
    printf(
      "%$<%p> \xae <%p>",
      msg_style & 0xf7,
      __builtin_return_address(0),
      __builtin_return_address(1)
    );
  }

  if (sig && (type != PANIC)) {
    char sigbuf[16] = {0}; // can't malloc, becuase malloc may call this
    to_dec(sig, sigbuf); // DO NOT use printf, cause that calls malloc too
    short pos = ((get_cur() / VGA_WIDTH) + 1) * VGA_WIDTH;
    pos -= strlen(sigbuf);
    write_str_at(sigbuf, pos, msg_style);
  }

  line_feed();

  if (type == PANIC) {
    asm volatile("cli; hlt"); // STOP NOW
  }
}
