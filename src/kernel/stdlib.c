#include "include/stdlib.h"
#include "include/util.h"

void *memcpy(void *dest, const void *src, size_t n) {
  // do nothing if 0: don't fail even if one of dest and src is NULL
  if (n == 0) return dest;

  asm volatile ("cld\n"
                "rep movsb\n" :
    : "c" (n),
      "S" (src),
      "D" (dest)
    : "memory", "cc" );

  return dest;
}

// helper function for memmove
void _backmemcpy(void *destend, const void *srcend, size_t n) {
  // do nothing if 0: don't fail even if one of dest and src is NULL
  if (n == 0) return;

  asm volatile ("std\n"
                "rep movsb\n" :
    : "c" (n),
      "S" (srcend),
      "D" (destend)
    : "memory", "cc" );
}

void *memmove(void *dest, const void *src, size_t n) {
  // do nothing if 0: don't fail even if one of dest and src is NULL
  if (n == 0) return dest;

  if (dest <= src) {
    // if dest is before src, then forward will work.
    memcpy(dest, src, n);
  } else {
    // if dest is after src, then backward will work.
    _backmemcpy((void *) ((uintptr_t) dest) + n, (const void *) ((uintptr_t) src) + n, n);
  }

  return dest;
}

void *memset(void *dest, int val, size_t n) {
  // do nothing if 0: don't fail even if one of dest and src is NULL
  if (n == 0) return dest;

  asm volatile ("cld\n"
                "rep stosb\n":
    : "a" (val),
      "c" (n), // this automatically truncates val to uint8_t
      "D" (dest)
    : "memory", "cc" );
  return dest;
}


int memcmp(const void *lhs, const void *rhs, size_t n) {
  // flags.
  bool carry;
  bool zero;

  asm volatile ("cld\n"
                "repe cmpsb" :
      "=@ccc" (carry),
      "=@ccz" (zero)
    : "c" (n),
      "S" (lhs),
      "D" (rhs)
    : "cc" );

  // returns: negative if lhs < rhs, zero if lhs = rhs, and positive if lhs > rhs
  // comparison is all unsigned.
  // basically sgn(lhs - rhs), extracted from flags
  if (zero) { // lhs equals rhs
    return 0;
  } else if (carry) { // lhs below rhs
    return -1;
  } else { // lhs above rhs
    return 1;
  }
}

// compare two regions of memory.
// returns: zero if lhs = rhs, and non-zero if lhs != rhs.
// this is a superset of memcpy, so is aliased to it.
int bcmp(const void *lhs, const void *rhs, size_t n) __attribute__((alias("memcmp")));

// get length of string.
size_t strlen(const char *s) {
  int i = -1;
  while (s[++i]);
  return i;
}

char *strcpy(char *dest, const char *src) {
  memcpy(dest, src, strlen(src) + 1); // include null terminator
  return dest;
}

int strcmp(const char *dest, const char *src) {
  // compare including the null terminator. this handles the length.
  return memcmp(dest, src, strlen(dest) + 1);
}

int strncmp(const char *dest, const char *src, size_t n) {
  // compare including the null terminator. this handles the length.
  return memcmp(dest, src, MIN(n, strlen(dest) + 1));
}

char *strcat(char *dest, const char *src) {
  strcpy(endof(dest), src);
  return dest;
}
