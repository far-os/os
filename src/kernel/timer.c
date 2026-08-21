#include "include/timer.h"
#include "include/port.h"
#include "include/util.h"
#include "include/cmos.h"
#include "include/hwinf.h"
#include "include/sched.h"

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

void timer_ticked() {
  ++uptime;
  // will work both if it zero and if it is resuming
  if ((uptime >= next_calc) && (next_calc != 0 || tsc_per_tick == 0)) {
    // if tsc supported
    if (hardware -> f_flags_edx & CPUID_01H_EDX_TSC) {
      calc_tsc_per_tick(TICK_COUNT_TSC_LOG2);
    }
  }

  if (!(uptime % 100)) {
    adv_time(curr_time);
  } // increment centisecond counter
  tick_focused_timed_invoke(); // tick program if available

  // tick rng
  lfsr();
}
