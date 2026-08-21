#include "include/util.h"
#include "include/err.h"
#include "include/misc.h"
#include "include/text.h"
#include "include/stdlib.h"
#include "include/memring.h"
#include "include/port.h"

// xnor fibonacci lfsr with taps (32, 30, 26, 25)
uint32_t lfsr() {
  static uint32_t state;

  unsigned _scratch;

  asm volatile ("testb $0xc5, %b1; setpe %b0; shrdl $1, %0, %1" :
      "=&r" (_scratch),
      "+r" (state)
    :
    : "cc");

  return state;
}

void idle() {
  pbyte_out(0x80, 0x0); // just passing the time (1-4 microseconds)
}

void memrev(const char *src, int len, char *dest) {
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

void to_hex(const void *data, unsigned char i_len, char *out) {
  char temporary;
  int z = i_len & 1;
  for (int j = z; j < (i_len + z); ++j) {
    temporary = ((char *) data)[((i_len-1) / 2) - (j / 2)];
    temporary >>= (!(j % 2) * 4);
    out[j - z] = nybble_to_hex(temporary);
  }
}

void to_filled_dec(int input, char *out, unsigned char size, char fill) {
  memset(out, fill, size);
  for (int i = input, j = size - 1; i > 0; i /= 10, --j) {
    out[j] = (char) (i % 10) + '0';
  }
  //memrev(dectempbuf, strlen(dectempbuf), out);
  //free(dectempbuf);
}

// so that the long long division doesn't try to use glibc
#pragma GCC push_options
#pragma GCC optimize "O3"

// signed. remember.
void l_to_dec(long long input, char *out) {
  char dectempbuf[21] = {0};
  unsigned char negative = 0;
  if (input < 0) {
    negative = 1;
    input = -input;
  }

  if (!input) {
    dectempbuf[0] = '0';
  } else {
    for (long long i = input, j = 0; i > 0; i /= 10, ++j) {
      dectempbuf[j] = (char) (i % 10) + '0';
    }
  }

  // the end of the reversed number is its beginning, so that's where the minus goes
  if (negative) *endof(dectempbuf) = '-';
  memrev(dectempbuf, strlen(dectempbuf), out);
//  free(dectempbuf);
}

#pragma GCC pop_options

// historical accident
void to_dec(int input, char *out) {
  l_to_dec((long long) input, out);
}

unsigned int to_uint(const char *input) {
  unsigned int f = 0;
  for (size_t i = 0; i < strlen(input); ++i) {
    unsigned char x = input[i];
    x -= '0';
    if (x > 9) {
      if (i == 0) f = -1;
      break;
    }

    f *= 10;
    f += (unsigned int) x;
  }
  return f;
}

unsigned short trace_ch_until_with(const char *str, int until, int start) {
  unsigned short int runx = start;
  for (int ii = 0; ii < until; ii++) {
    switch (str[ii]) {
    case '\n':
      runx /= POS(0, 1);
      runx++;
      runx *= POS(0, 1);
      break;
    case '\t':
      runx /= 8;
      runx++;
      runx *= 8;
      break;
    case '\b':
      runx--;
      break;
    default:
      runx++;
    }
  }
  return runx;
}

bool is_whitespace(char x) {
  return (x == '\0' || x == ' ' || x == '\xff');
}

char to_upper(char x) {
  if (x >= 'a' && x <= 'z') {
    return x - 0x20;
  } else {
    return x;
  }
}
