#include "misc.h"

#pragma once

#define CEIL_DIV(a, b) ((a/b) + !!(a%b))
#define MAX(a, b) ((a > b) ? a : b)
#define MIN(a, b) ((a < b) ? a : b)

void idle();

int strlen(char *str);
void memcpy(void *src, void *dest, unsigned int amount);
void backmemcpy(void *srcend, void *destend, unsigned int amount);
void strcpy(char *src, char *dest);
unsigned char memcmp(void *src, void *dest, unsigned int amount);

extern unsigned int rip_thunk();

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


unsigned char strcmp(char *src, char *dest);

static inline void memset(void *dest, unsigned int amount, unsigned char val) {
  asm volatile ("cld\n"
                "rep stosb\n":
    : "a" (val),
      "c" (amount),
      "D" (dest)
    : "memory", "cc" );
}

#define memzero(dest, amount) memset(dest, amount, 0)

void memrev(char *src, int len, char *dest);

char nybble_to_hex(int num);
void to_hex(void *data, unsigned char i_len, char *out);
void to_filled_dec(int input, char *out, unsigned char size, char fill);
void to_dec(int input, char *out);
unsigned int to_uint(char *input);

char to_upper(char);

static inline char *strcat(char *out, char *in) {
  strcpy(in, endof(out));
  return out;
}


#define NOT_WAITING 0x00
#define HAS_INTEGER 0x10
#define AFTER_PERCENT -1
void sprintf(char *dest, const char *fmt, ...);

#define trace_ch_until(a,b) trace_ch_until_with(a,b,0)
unsigned short trace_ch_until_with(char *str, int until, int start);

unsigned char is_whitespace(char x);
