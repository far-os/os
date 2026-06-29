#pragma once

// TODO: remove!

#define PROG_LOC ((void *) 0x100000)

// pointer arithmetic with void ptrs is technically undefined behaviour, as we don't know the width of void
#define adj(x) ((void *) ((unsigned int) x + (unsigned int) PROG_LOC))
#define badj(x) ((void *) ((unsigned int) x - (unsigned int) PROG_LOC))
// adjusting pointers

// c++ has a funny relationship with null, in that it doesn't like casts to void*
// however, straightup 0 is fine
#ifdef __cplusplus
#define NULL 0
#else
#define NULL (void *) 0
#endif

/*
typedef unsigned int size_t;
typedef int ssize_t;
*/

#define endof(str) (str + strlen(str))

#include <stdbool.h>

// where the kernel is stored
extern void *kernel;

struct far_ver {
  unsigned char major;
  unsigned char minor;
  unsigned char patch;
  unsigned char build;
} __attribute__((packed));

// current vrsion
extern struct far_ver * curr_ver;

struct farb_header {
  char farb_magic[4]; // should be equal to FARb
  unsigned char jump_away[2]; // jumps to after the header
  unsigned short arch; // corresponds to the archicture
  int eh_ptr; // error handler pointer
} __attribute__((packed));

// c++ seems to hate __seg_fs
#ifndef __cplusplus
extern struct farb_header __seg_fs *prog_head;
#endif
