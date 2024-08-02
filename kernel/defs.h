#ifndef DEFS_H
#define DEFS_H

#define adj(x) (x) + 0x100000
#define badj(x) (x) - 0x100000
// adjusting pointers

#define NULL (void *) 0
#define endof(str) (str + strlen(str))

struct timestamp {
  unsigned char second;
  unsigned char minute;
  unsigned char hour;
  unsigned char weekday;
  unsigned char date;
  unsigned char month;
  unsigned int year;
} __attribute__((packed));

struct timestamp *curr_time = (struct timestamp *) 0xc7f0;

// centisec (100ths of sec) since load
unsigned int countx = 0;

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
  .build = 19, // minor changes + hotfixes
});

struct keystates { /* a 104-bit struct containing data */
  /*
    bit 0 set: scroll lock
    bit 1 set: num lock
    bit 2 set: caps lock
    bit 3 set: shift held
    bit 4 set: ctrl held
    bit 5 set: meta held
    bit 6 set: alt held
    bit 7 set: waiting for another scancode in a multi-scancode key
  */
  unsigned char modifs;

  /*
    the following two comprise a 96-bit value, where each bit is whether that key is held
  */
  unsigned long long int states_low;
  unsigned int states_high;
} __attribute__((packed));

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

struct sector_box { // a pointer to a place of disk
  unsigned int lba; // lba of start
  unsigned char len; // length
} __attribute__((packed));

// error message shit
enum MSG_TYPE {
  INFO = 0, // ->
  WARN = 1, // +
  PROGERR = 2, // !
  KERNERR = 3, // !!
  PANIC = 4, // x
};

char msg_symbs[5] = {
  0x1a, // -> symbol
  '+',
  '!',
  0x13, // !! symbol
  'x',
};

struct inp_strbuf {
  char *buf;
  unsigned int len;
  unsigned int ix;
};

#endif

