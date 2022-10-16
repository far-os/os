#include "util.h"

#ifndef TIMER_H
#define TIMER_H

// 1/3 of the ntsc colour burst frequency, or 1/12 of the 315/22 MHz.
// This is the frequency that the PIT runs at.
#define PIT_FREQ_HZ 1193182

#define TIMER_DPORT_C0 0x40
#define TIMER_DPORT_C1 0x41
#define TIMER_DPORT_C2 0x42
#define TIMER_CPORT 0x43

// get the divisor - the amount the time needs to count down from - from a number in hz
static inline unsigned short get_divisor(int freq) {
  return (unsigned short) (PIT_FREQ_HZ / freq);
}

void init_timer(unsigned short divisor) {
  // com[7:6] - channel
  // com[5:4] - r/w mode
    // 01 - LSB
    // 10 - MSB
    // 11 - both
  // com[3:1] - mode
  // com[0] - binary/bcd
  pbyte_out(0x43, 0b00110110);

  idle();
  pbyte_out(0x40, divisor & 0xff);
  idle();
  pbyte_out(0x40, (divisor >> 8) & 0xff);
}

#endif
