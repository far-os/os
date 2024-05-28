#include "text.h"
#include "defs.h"
// #include "memring.h"
// cyclic, declare what we want

void *malloc(unsigned int);
void free(void *);

enum MSG_TYPE;
void msg(enum MSG_TYPE, int, char *);

// not really using standard library - stdarg just provides platform-dependent defines 
#include <stdarg.h>

#ifndef UTIL_H
#define UTIL_H

/*void cp437() {
  // prints cp437

  write_str_at("  Codepage 437  ", POS((VGA_WIDTH - 16), (VGA_HEIGHT - 16 - 2)), COLOUR(MAGENTA, B_GREEN));
  for (int cph = 0; cph < 16; ++cph) {
    write_cell(nybble_to_hex(cph), POS((VGA_WIDTH - 16 - 1), (VGA_HEIGHT - (16 - cph))), COLOUR(RED, WHITE));
    write_cell(nybble_to_hex(cph), POS((VGA_WIDTH - (16 - cph)), (VGA_HEIGHT - 16 - 1)), COLOUR(RED, WHITE));
    for (int cpw = 0; cpw < 16; ++cpw) {
      write_cell((cph * 16) + cpw, POS((VGA_WIDTH - (16 - cpw)), (VGA_HEIGHT - (16 - cph))), COLOUR(YELLOW, B_CYAN));
    }
  }
}*/

int strlen(char *str) {
  int i = -1;
  while (str[++i]);
  return i;
}

void memcpy(void *src, void *dest, unsigned int amount) {
  asm volatile ("cld\n"
                "rep movsb\n" :
    : "c" (amount),
      "S" (src),
      "D" (dest)
    : "memory", "cc" );
    src -= amount;
}

void backmemcpy(void *srcend, void *destend, unsigned int amount) {
  asm volatile ("std\n"
                "rep movsb\n" :
    : "c" (amount),
      "S" (srcend),
      "D" (destend)
    : "memory", "cc" );
}

void strcpy(char *src, char *dest) {
  memcpy(src, dest, strlen(src));
}

unsigned char memcmp(void *src, void *dest, unsigned int amount) {
  unsigned char o;
  asm volatile ("cld\n"
                "rep cmpsb" :
      "=@ccz" (o)
    : "c" (amount),
      "S" (src),
      "D" (dest)
    : "cc" );
  return o;
}

static inline unsigned char bittest(void *src, unsigned int bit) {
  unsigned char o;
  asm volatile ("bt %2, (%1)" : 
      "=@ccc" (o)
      : "r" (src),
        "ir" (bit)
      : "cc" );
  return o;
}

static inline unsigned char bitinv(void *src, unsigned int bit) {
  unsigned char o;
  asm volatile ("btc %2, (%1)" : 
      "=@ccc" (o)
      : "r" (src),
        "ir" (bit)
      : "cc" );
  return o;
}

static inline unsigned char bitclear(void *src, unsigned int bit) {
  unsigned char o;
  asm volatile ("btr %2, (%1)" : 
      "=@ccc" (o)
      : "r" (src),
        "ir" (bit)
      : "cc" );
  return o;
}

static inline unsigned char bitset(void *src, unsigned int bit) {
  unsigned char o;
  asm volatile ("bts %2, (%1)" : 
      "=@ccc" (o)
      : "r" (src),
        "ir" (bit)
      : "cc" );
  return o;
}

unsigned char strcmp(char *src, char *dest) {
  return memcmp(src, dest, strlen(src)) && strlen(src) == strlen(dest);
}

static inline void memset(void *dest, unsigned int amount, unsigned char val) {
  asm volatile ("cld\n"
                "rep stosb\n":
    : "a" (val),
      "c" (amount),
      "D" (dest)
    : "memory", "cc" );
}

static inline void memzero(void *dest, unsigned int amount) {
  memset(dest, amount, 0);
}

void memrev(char *src, int len, char *dest) {
  for (int i = 0; i < len; ++i) {
    dest[i] = src[len - i - 1];
  }
}

char nybble_to_hex(int num) {
  int value = num;
  value &= 0x0f;
  value += 0x30;
  if (value >= 0x3a) {
    value += 0x27;
  }
  return (char) value;
}

void to_hex(void *data, unsigned char i_len, char *out) {
  char temporary;
  for (int j = 0; j < i_len; ++j) {
    temporary = ((char *) data)[((i_len-1) / 2) - (j / 2)];
    temporary >>= (!(j % 2) * 4);
    out[j] = nybble_to_hex(temporary);
  }
}

void to_filled_dec(int input, char *out, unsigned char size, char fill) {
  char * dectempbuf = malloc(size);
  memset(dectempbuf, size, fill);
  for (int i = input, j = 0; i > 0; i /= 10, ++j) {
    dectempbuf[j] = (char) (i % 10) + '0';
  }
  memrev(dectempbuf, strlen(dectempbuf), out);
  free(dectempbuf);
}

void to_dec(int input, char *out) {
  char * dectempbuf = malloc(12);
  if (!input) {
    dectempbuf[0] = '0';
  } else {
    for (int i = input, j = 0; i > 0; i /= 10, ++j) {
      dectempbuf[j] = (char) (i % 10) + '0';
    }
  }
  memrev(dectempbuf, strlen(dectempbuf), out);
  free(dectempbuf);
}

unsigned int to_uint(char *input) {
  unsigned int f = 0;
  for (int i = 0; i < strlen(input); ++i) {
    unsigned char x = input[i];
    x -= '0';
    if (x > 9) break;

    f *= 10;
    f += (unsigned int) x;
  }
  return f;
}

char *strcat(char *out, char *in) {
  strcpy(in, out + strlen(out));
  return out;
}

#define NOT_WAITING 0x00
#define HAS_INTEGER 0x10
#define AFTER_PERCENT -1
void sprintf(char *dest, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt); // second parameter is the last arg before variadic
  
  // we entrust that dest has already been zero'd
  char waiting = NOT_WAITING;
  unsigned int length_modif = 0;

  for (int fi = 0, di = 0; fmt[fi];) { // fi = fmt offset, di = dest offset
    if (waiting == NOT_WAITING) {
      if (fmt[fi] == '%') {
        waiting = AFTER_PERCENT;
        ++fi;
        continue;
      }

      // normal operation
      dest[di++] = fmt[fi++];
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
        case 'd':
          if (length_modif) {
            to_filled_dec(va_arg(args, unsigned int), dest + di, length_modif, '0');
            length_modif = 0;
          } else {
            to_dec(va_arg(args, unsigned int), dest + di);
          }
          di = strlen(dest);
          goto stop_waiting;
        case 'X':
          strcat(dest, "0x");
          di += 2;
        case 'x':
          if (length_modif == 1) {
            dest[di++] = nybble_to_hex(va_arg(args, unsigned int));
            length_modif = 0;
            goto stop_waiting;
          }

          to_hex(va_arg(args, unsigned int), length_modif ? length_modif : 8, dest + di);
          length_modif = 0;
          di = strlen(dest);
          goto stop_waiting;
        case 's':
          char *ptr = va_arg(args, char *);
          if (ptr) { // ie, not null
            if (length_modif) {
              memcpy(ptr, dest + di, length_modif);
              length_modif = 0;
            } else {
              strcpy(ptr, dest + di);
            }
            di = strlen(dest);
          }
          goto stop_waiting;
        case 'c':
          char c = va_arg(args, int);
          if (c) { dest[di++] = c; }
          goto stop_waiting;
        case '%':
          dest[di++] = '%';
          goto stop_waiting;
        default:
          char *k = "printf: unknown format %_, ignoring";
          k[24] = fmt[fi];
          msg(WARN, 1, k);

        stop_waiting:
          waiting = NOT_WAITING;
          break;
      }
      ++fi;
    }
  }
}

void to_ver_string(struct far_ver * ver, char * vbuf) {
  sprintf(vbuf, "FarOS v%d.%d.%d", ver -> major, ver -> minor, ver -> patch);
}

#endif
