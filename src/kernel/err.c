#include "include/err.h"
#include "include/misc.h"
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

#define LOGRING_START 0x18'000
#define LOGRING_END 0x20'000
struct logring_t logring = {
  .start = LOGRING_START,
  .end   = LOGRING_END,

  .earliest = LOGRING_START,
  .next     = LOGRING_START,
  .full     = false
};

// returns next step
const struct logring_entry * const draw_msg(const struct logring_entry * const data, bool show_more) {
  // print always, except if info, then print info if verbose enough OR showing more
  bool do_print = show_more || (data->level == INFO && xconf->verbosity < SHOW_INFO) || (data -> level != INFO);

  unsigned char msg_style;

  if (do_print) {
    if (get_cur() % VGA_WIDTH) { line_feed(); }; // start on a new line
    if (data->level == INFO) {
      msg_style = COLOUR(BLACK, B_CYAN);
    } else if (data->level == WARN) {
      msg_style = COLOUR(BLACK, B_YELLOW);
    } else {
      msg_style = COLOUR(BLACK, B_RED);
    }

    write_cell_cur(msg_symbs[data->is_panic ? PANIC : data->level], msg_style);
    adv_cur();
  }

  const struct logring_entry *traversal = data + 1;
  for (unsigned int blk = data->msg_blocks; blk > 0; blk--) {
    // if past end, rotate next back to normal
    while (traversal >= logring.end) {
      traversal -= (logring.end - logring.start);
    }

    // print one block's worth
    if (do_print) nprintf(sizeof(struct logring_entry), "%$%16s", msg_style, traversal);

    traversal++;
  }

  // if we don't want it printed, we just want to traversal it
  if (!do_print) return traversal;

  adv_cur();

  if (xconf -> verbosity >= SHOW_STACKTRACE) {
    printf(
      "%$<%p> \xae <%p> ",
      msg_style & 0xf7,
      data->traceback[0],
      data->traceback[1]
    );
  }

  if (show_more) {
    printf(
      "%$[#%u]",
      msg_style & 0xf7,
      data->timestamp
    );
  }

  if (data->sig && !(data->is_panic)) {
    char sigbuf[16] = {0}; // can't malloc, becuase malloc may call this
    to_dec(data->sig, sigbuf); // DO NOT use printf, cause that calls malloc too
    short pos = ((get_cur() / VGA_WIDTH) + 1) * VGA_WIDTH;
    pos -= strlen(sigbuf);
    write_str_at(sigbuf, pos, msg_style);
  }

  line_feed();

  return traversal;
}

struct logring_entry * logring_pushf(struct logring_entry preliminary, const char *fmt, va_list args) {
  // exactly which byte in the current block
  unsigned int unit; // set later, when checks are needed
  unsigned int blocks;

  // XXX: gcc extension to use nested function
  void logring_writechar(char ch, unsigned char style) {
    if (logring.next == logring.earliest && logring.full) {
      // traverse earliest, and advance by that many blocks
      // we don't multiply by sizeof(struct logring_entry) because pointer arithmetic does that for us
      logring.earliest += logring.earliest->msg_blocks + 1;

      // if past end, rotate earliest back to normal
      while (logring.earliest >= logring.end) {
        logring.earliest -= (logring.end - logring.start);
      }
    }

    if (unit >= LOGRING_BLOCK_SIZE) {
      logring.next++;
      blocks++;

      // if past end, rotate next back to normal
      while (logring.next >= logring.end) {
        logring.next -= (logring.end - logring.start);
        logring.full = true;
        printf("rotate!!!! \n");
      }

      unit = 0;
    }

    if (blocks < MAX_MSG_BLOCKS) {
      ((char *) logring.next)[unit] = ch;
      unit += 1;
    }

    // null term at maximum
    if (blocks == (MAX_MSG_BLOCKS - 1) && unit == (LOGRING_BLOCK_SIZE - 1)) {
      ((char *) logring.next)[unit] = 0;
    }
  }

  // this specific log
  struct logring_entry * const data = logring.next;

  // write preliminary, but including all the checks we have put in the callback
  unit = 0;
  logring_writechar(0, 0); // do all the checks

  // write next
  *logring.next = preliminary;

  unit = 16; // make .next overflow
  logring_writechar(0, 0); // do all the checks (again)

  unit = 0;
  blocks = 0;

  // write message
  vpfctprintf(&logring_writechar, fmt, 0, args); // style doesn't matter: here we've just put 0
  logring_writechar(0, 0); // write a null byte (yes, we want this included in the blocks)

  data->msg_blocks = blocks + (!!unit);
  if (unit) logring.next++;

  return data;
}

/// formatted message
void msg(enum MSG_TYPE type, enum ERRSIG sig, const char* supp, ...) {
  va_list args;
  va_start(args, supp); // second parameter is the last arg before variadic

  struct logring_entry ent = {
    .sig = sig,
    .level = type,
    .is_panic = type == PANIC,
    .timestamp = uptime,
    .traceback = {
      __builtin_return_address(0),
      __builtin_return_address(1)
    },
  }; // we don't know msg_bytes.

  const struct logring_entry * const data = logring_pushf(ent, supp, args);

  draw_msg(data, false);

  if (type == PANIC) {
    asm volatile("cli; hlt"); // STOP NOW
  }

  va_end(args); // no longer need args, get out!
}
