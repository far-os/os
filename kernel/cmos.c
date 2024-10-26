#include "include/cmos.h"
#include "include/err.h"
#include "include/util.h"
#include "include/port.h"
#include "include/memring.h"

static inline unsigned char get_cmos_reg(unsigned char reg) {
  pbyte_out(P_CMOS_ADDRESS, reg | 0x80);
  return pbyte_in(P_CMOS_DATA);
}

void read_rtc(struct timestamp *ts) {
  // update in progress flag
//  while (get_cmos_reg(RTC_STAT_A) | 0x80);

  ts -> second  = get_cmos_reg(RTC_SEC );
  ts -> minute  = get_cmos_reg(RTC_MIN );
  ts -> hour    = get_cmos_reg(RTC_HR );
  unsigned char pm_flag = bitclear(&(ts -> hour), 7); // only relevant if 12hour: takes the most signifcant bit
  ts -> weekday = get_cmos_reg(RTC_WKDY);
  ts -> date    = get_cmos_reg(RTC_DATE);
  ts -> month   = get_cmos_reg(RTC_MON );
  ts -> year    = (unsigned int) get_cmos_reg(RTC_YR);
  unsigned char century = get_cmos_reg(RTC_CENT);

  if (!ts -> weekday || ts -> weekday > 7) {
    msg(INFO, E_TIME, "Invalid weekday");
    line_feed();
    ts -> weekday = 0;
  }
  
  // bcd mode - if bit 2 of B is clear we are in bcd mode.
  if (!(get_cmos_reg(RTC_STAT_B) & 0x04)) {
    MK_BCD(ts -> second);
    MK_BCD(ts -> minute);
    MK_BCD(ts -> hour);
    MK_BCD(ts -> date);
    MK_BCD(ts -> month);
    MK_BCD(ts -> year);
    MK_BCD(century);
  }

  // 12/24h - if bit 1 set we are in 24h mode.
  if (!(get_cmos_reg(RTC_STAT_B) & 0x02) && pm_flag) {
    ts -> hour = (ts -> hour + 12) % 24;
  }

  if (century < 19) {
    msg(INFO, E_TIME, "Invalid century or before 1900 - assuming year 20xx");
    line_feed();
    century = 20;
  }

  ts -> year += century * 100;
}

void time(void *tbuf) {
  // get two readings - check if they are equal. a very naive method.
  struct timestamp *temp = malloc(sizeof(struct timestamp));
  struct timestamp *next = malloc(sizeof(struct timestamp));

  read_rtc(temp);
  read_rtc(next);

  while (!memcmp(temp, next, sizeof(struct timestamp))) {
    free(temp);
    temp = next;
    next = malloc(sizeof(struct timestamp));
    read_rtc(next);
  }

  free(temp);
  memcpy(next, tbuf, sizeof(struct timestamp));
  free(next);
}

// using dumb almightily horrid macro, see header file
void adv_time(struct timestamp *ts) {
  // unholy horrors
  if (++ts -> second >= 60) {
    ts -> second = 0;
    if (++ts -> minute >= 60) {
      ts -> minute = 0;
      if (++ts -> hour >= 24) {
        ts -> hour = 0;
        if (ts -> weekday) {
          ts -> weekday = (ts -> weekday % 7) + 1;
        }
        if (++ts -> date > days_per_mo[ts -> month == 2 ? !IS_LEAP_YR(ts -> year) * 2 : ts -> month]) {
          ts -> date = 1;
          if (++ts -> month > 12) {
            ts -> month = 1;
            ++ts -> year;
          }
        }
      }
    }
  }
}
