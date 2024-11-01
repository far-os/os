#pragma once

#define PROG_LOC 0x100000
#define adj(x) (x) + PROG_LOC
#define badj(x) (x) - PROG_LOC
// adjusting pointers

#define NULL (void *) 0
#define endof(str) (str + strlen(str))

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
