#include "misc.h"

#pragma once

#define CEIL_DIV(a, b) ((a/b) + !!(a%b))
#define MAX(a, b) ((a > b) ? a : b)
#define MIN(a, b) ((a < b) ? a : b)

extern void idle();

extern unsigned int rip_thunk();

static inline unsigned char bittest(const void *src, unsigned int bit) {
  unsigned char o;
  asm volatile ("bt %2, (%1)" :
      "=@ccc" (o)
      : "r" (src),
        "ir" (bit)
      : "cc" );
  return o;
}

static inline unsigned char bitinv(const void *src, unsigned int bit) {
  unsigned char o;
  asm volatile ("btc %2, (%1)" :
      "=@ccc" (o)
      : "r" (src),
        "ir" (bit)
      : "cc" );
  return o;
}

static inline unsigned char bitclear(const void *src, unsigned int bit) {
  unsigned char o;
  asm volatile ("btr %2, (%1)" :
      "=@ccc" (o)
      : "r" (src),
        "ir" (bit)
      : "cc" );
  return o;
}

static inline unsigned char bitset(const void *src, unsigned int bit) {
  unsigned char o;
  asm volatile ("bts %2, (%1)" :
      "=@ccc" (o)
      : "r" (src),
        "ir" (bit)
      : "cc" );
  return o;
}

void memrev(const char *src, int len, char *dest);

char nybble_to_hex(int num);
void to_hex(const void *data, unsigned char i_len, char *out);
void to_filled_dec(int input, char *out, unsigned char size, char fill);
void l_to_dec(long long int input, char *out);
void to_dec(int input, char *out);
unsigned int to_uint(const char *input);

char to_upper(char);


#define trace_ch_until(a,b) trace_ch_until_with(a,b,0)
unsigned short trace_ch_until_with(const char *str, int until, int start);

unsigned char is_whitespace(char x);
