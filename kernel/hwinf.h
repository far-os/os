#include "util.h"

#ifndef HWINF_H
#define HWINF_H

struct hwinf {
  unsigned char bios_disk; // 0xcc00
  unsigned int cpuid_leaves; // 0xcc01
  char vendor[12]; // 0xcc05
} __attribute__((packed));

struct hwinf *hardware = (struct hwinf *) 0xcc00;

extern char check_cpuid_avail();

void query_cpuid() {
  if (!check_cpuid_avail()) { // is cpuid unavailable?
    hardware -> cpuid_leaves = 0; // if so make available leaves 0
    return;
  }

  unsigned int vendbuf[3];
  
  asm volatile ("cpuid"
    : "=a" (hardware -> cpuid_leaves),
      "=b" (vendbuf[0]),
      "=d" (vendbuf[1]),
      "=c" (vendbuf[2])
    : "a" (0));

  memcpy(vendbuf, &(hardware -> vendor), 12);
}

#endif
