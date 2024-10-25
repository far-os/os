#pragma once

#define RTC_SEC  0x00
#define RTC_MIN  0x02
#define RTC_HR   0x04
#define RTC_WKDY 0x06
#define RTC_DATE 0x07
#define RTC_MON  0x08
#define RTC_YR   0x09
#define RTC_CENT 0x32

#define RTC_STAT_A 0x0a
#define RTC_STAT_B 0x0b

#define P_CMOS_ADDRESS 0x70
#define P_CMOS_DATA    0x71

#define MK_BCD(num) num = ((num) & 0x0f) + (((num) >> 4) * 10)

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
char *weekmap[7] = {
  "Sun",
  "Mon",
  "Tue",
  "Wed",
  "Thu",
  "Fri",
  "Sat"
};

static inline unsigned char get_cmos_reg(unsigned char reg);
void read_rtc(struct timestamp *ts);
void time(void *tbuf);

// AARGH
unsigned char days_per_mo[13] = {29, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
#define IS_LEAP_YR(yr) (!(yr % 4) && (!(yr % 400) || (yr % 100)))

void adv_time(struct timestamp *ts);
