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

struct far_ver * curr_ver = &((struct far_ver) {
  .major = 0,
  .minor = 0,
  .patch = 3,
  .build = 22, // minor changes + hotfixes
});

struct farb_header {
  char farb_magic[4]; // should be equal to FARb
  unsigned char jump_away[2]; // jumps to after the header
  unsigned short arch; // corresponds to the archicture
  int eh_ptr; // error handler pointer
} __attribute__((packed));

struct farb_header __seg_fs *prog_head = 0;
