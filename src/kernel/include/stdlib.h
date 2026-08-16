// basic posix stdlib functions
#pragma once

// uint*_t, bool, etc
#include <stdint.h>
#include <stdbool.h>

// stddef takes care of size_t
#include <stddef.h>

/* begin rust core dependencies */

// copy memory from one location to another; returns destination
void *memcpy(void *dest, const void *src, size_t n);

// ditto, but behave properly if regions overlap; returns destination
void *memmove(void *dest, const void *src, size_t n);

// sets each byte in region to the same value.
// takes an int, but this is automatically truncated to unsigned char
void *memset(void *dest, int val, size_t n);

// compares two regions of memory.
// return value =0 if equal, <0 if lhs<rhs and >0 if lhs>rhs.
int memcmp(const void *lhs, const void *rhs, size_t n);

// compares two regions of memory.
// return value =0 is equal, and !=0 if unequal.
// just aliased to memcmp for ease of use
int bcmp(const void *lhs, const void *rhs, size_t n);

// get length of c-string s.
size_t strlen(const char *s);

/* end rust core dependencies */

#define memzero(dest, amount) memset(dest, 0, amount)

void backmemcpy(void *dest, const void *src, size_t n);
// string bestions of above
char *strcpy(char *dest, const char *src);
int strcmp(const char *dest, const char *src);
int strncmp(const char *dest, const char *src, size_t n);

char *strcat(char *dest, const char *src);
