#pragma once

// 1/3 of the ntsc colour burst frequency, or 1/12 of the 315/22 MHz.
// This is the frequency that the PIT runs at.
#define PIT_FREQ_HZ 1193182

#define TIMER_DPORT_C0 0x40
#define TIMER_DPORT_C1 0x41
#define TIMER_DPORT_C2 0x42
#define TIMER_CPORT 0x43

static inline unsigned short get_divisor(unsigned int freq);

void init_timer(unsigned short divisor);
