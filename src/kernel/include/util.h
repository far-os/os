#include "misc.h"
#include "stdlib.h"

#pragma once

#define CEIL_DIV(a, b) ((a/b) + !!(a%b))
#define MAX(a, b) ((a > b) ? a : b)
#define MIN(a, b) ((a < b) ? a : b)

extern void idle();

extern unsigned int rip_thunk();

uint32_t lfsr();

static inline bool bittest(const void *src, uint32_t bit) {
  bool o;
  asm volatile ("btl %2, (%1)" :
      "=@ccc" (o)
      : "r" (src),
        "ir" (bit)
      : "cc" );
  return o;
}

static inline bool bitinv(const void *src, uint32_t bit) {
  bool o;
  asm volatile ("btcl %2, (%1)" :
      "=@ccc" (o)
      : "r" (src),
        "ir" (bit)
      : "cc" );
  return o;
}

static inline bool bitclear(const void *src, uint32_t bit) {
  bool o;
  asm volatile ("btrl %2, (%1)" :
      "=@ccc" (o)
      : "r" (src),
        "ir" (bit)
      : "cc" );
  return o;
}

static inline bool bitset(const void *src, uint32_t bit) {
  bool o;
  asm volatile ("btsl %2, (%1)" :
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

bool is_whitespace(char x);
