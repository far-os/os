#pragma once

#define K_PORT 0x60
#define K_PORT_COM 0x64

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

extern struct keystates *keys;

extern char scan_map_en_UK[96];
extern char scan_map_en_UK_shift[96];

enum what_upd {
  CURSOR,
  COMMAND
};

void indic_light_upd(); 

void read_kbd(); 
void cpu_reset();
