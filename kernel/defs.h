#ifndef DEFS_H
#define DEFS_H

#define adj(x) x + 0x100000
// adjusting pointers

unsigned int countx = 0;

struct cpu_state {
  unsigned int edi;
  unsigned int esi;
  unsigned int ebp;
  unsigned int esp;
  unsigned int ebx;
  unsigned int edx;
  unsigned int ecx;
  unsigned int eax;

//  unsigned int ss;
  unsigned int es;
  unsigned int ds;
} __attribute__((packed)); // registers as returned from pushad

struct stack_state {
  unsigned int error_code;
  unsigned int eip;
  unsigned int cs;
  unsigned int eflags;
} __attribute__((packed)); // stack

struct farb_header {
  char farb_magic[4]; // should be equal to FARb
  unsigned char jump_away[2]; // jumps to after the header
  unsigned short arch; // corresponds to the archicture
  int eh_ptr; // error handler pointer
} __attribute__((packed));

struct farb_header __seg_fs *prog_head = 0;

#endif

