#include "text.h"
#include "defs.h"
// #include "memring.h"
// cyclic, declare what we want

void *malloc(unsigned int);
void free(void *, unsigned int);

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
  int i = 0;
  for (; str[i] != 0; ++i);
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
  free(dectempbuf, size);
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
  free(dectempbuf, 12);
}

void to_ver_string(short int ver, char * vbuf) {
  strcpy("FarOS v_._._\0\0", vbuf);
  to_dec((ver >> 8) & 0xf, vbuf + 7);
  to_dec((ver >> 4) & 0xf, vbuf + 9);
  to_dec(ver & 0xf, vbuf + 11);
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

#endif
