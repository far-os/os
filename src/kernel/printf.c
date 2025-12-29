#include "include/text.h"
#include "include/util.h"
#include "include/memring.h"
#include "include/err.h"
#include "include/printf.h"

// not really using standard library - stdarg just provides platform-dependent defines
#include <stdarg.h>

// returns number of characters moved
unsigned int callback_str(putch_callback put, char *str, unsigned char style, unsigned int lim) {
  unsigned int count = 0;
  while (str[count]) {
    put(str[count], style);
    count++;

    if (lim && count > lim) {
      break;
    }
  }

  return count;
}

// thin wrapper
void printf(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt); // second parameter is the last arg before variadic

  vpfctprintf(&write_advanced_cell_cur, fmt, COLOUR(BLACK, WHITE), args);

  va_end(args);
}

// thin wrapper
void sprintf(char *dest, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt); // second parameter is the last arg before variadic

  int di = 0;
  // XXX: gcc extension to use nested functions
  void cb(char ch, unsigned char _) {
    dest[di++] = ch;
  }

  vpfctprintf(&cb, fmt, 0, args); // style doesn't matter: here we've just put 0

  va_end(args);
}

void snprintf(char *dest, unsigned int n, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt); // second parameter is the last arg before variadic

  int di = 0;
  // XXX: gcc extension to use nested functions
  void cb(char ch, unsigned char _) {
    if (di < n) dest[di++] = ch;
  }

  vpfctprintf(&cb, fmt, 0, args); // style doesn't matter: here we've just put 0

  va_end(args);
}

// va_list painted function printf. nobody should really be using this
void vpfctprintf(putch_callback put, const char *fmt, unsigned char start_style, va_list args) {
  // we trust that dest has already been zero'd
  char waiting = NOT_WAITING;
  unsigned int length_modif = 0;

  unsigned char style = start_style;
  int fi;

  char *convbuf = NULL;

  for (fi = 0; fmt[fi];) { // fi = fmt index
    if (waiting == NOT_WAITING) {
      if (fmt[fi] == '%') {
        waiting = AFTER_PERCENT;
        ++fi;
        continue;
      }

      // normal operation
      put(fmt[fi++], style);
      continue;
    } else {
      switch (fmt[fi]) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          waiting = HAS_INTEGER;
          length_modif *= 10;
          length_modif += fmt[fi] - '0';
          break;
        case 'b':
          // NOTE: lesson learnt here. ALWAYS give va_arg unsigned int, nothing shorter, because otherwise gcc will emit ud2 and will only tell you in a pathetic little "note:"
          callback_str(put, va_arg(args, unsigned int) ? "true" : "false", style, 0);
          goto stop_waiting;
        case 'B':
          callback_str(put, va_arg(args, unsigned int) ? "yes" : "no", style, 0);
          goto stop_waiting;
        case 'd':
          if (length_modif) {
            convbuf = malloc(length_modif + 1);
            to_filled_dec(va_arg(args, unsigned int), convbuf, length_modif, '0');
          } else {
            convbuf = malloc(16);
            to_dec(va_arg(args, unsigned int), convbuf);
          }

          // convbuf is guaranteed to be defined.
          callback_str(put, convbuf, style, 0);
          goto stop_waiting;
        case 'l': // no filled variant, historical accident again
          convbuf = malloc(32); // is freed later
          l_to_dec(va_arg(args, unsigned int), convbuf);

          callback_str(put, convbuf, style, 0);
          goto stop_waiting;
        case 'X':
          callback_str(put, "0x", style, 0);
        case 'x':
          if (length_modif == 1) {
            put(nybble_to_hex(va_arg(args, unsigned int)), style);
            goto stop_waiting;
          }

          convbuf = malloc(length_modif ? length_modif : 8);
          to_hex(va_arg(args, unsigned int), length_modif ? length_modif : 8, convbuf);
          callback_str(put, convbuf, style, 0);

          goto stop_waiting;
        case 'p':
          int val = va_arg(args, unsigned int);

          convbuf = malloc(8);
          to_hex(&val, 8, convbuf);
          callback_str(put, convbuf, style, 0);

          goto stop_waiting;
        case 's':
          char *ptr = va_arg(args, char *);
          if (ptr) { // ie, not null
            if (length_modif) { // if we have limited ourselves
              callback_str(put, ptr, style, length_modif);
            } else {
              callback_str(put, ptr, style, 0);
            }
          }
          goto stop_waiting;
        case 'c':
          char c = va_arg(args, int);
          if (c) { put(c, style); }
          goto stop_waiting;
        case '$': // style
          if (length_modif) {
            style = length_modif & 0xff;
          } else {
            style = va_arg(args, unsigned int) & 0xff;
          }
          goto stop_waiting;
        case '%':
          put('%', style);
          goto stop_waiting;
        default:
          char *k = "printf: unknown format %_, ignoring";
          k[24] = fmt[fi];
          msg(WARN, E_PROG, k);

        stop_waiting:
          if (convbuf) { // free our conversion buffer if it's been used
            free(convbuf);
            convbuf = NULL;
          }

          length_modif = 0;
          waiting = NOT_WAITING;
          break;
      }
      ++fi;
    }
  }
}
