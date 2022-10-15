#include "util.h"

#ifndef HWINF_H
#define HWINF_H

struct hwinf {
  unsigned char bios_disk; // 0xcc00
  unsigned int cpuid_leaves; // 0xcc01
  char vendor[12]; // 0xcc05

  unsigned char c_family; // 0xcc11
  unsigned char c_model; // 0xcc12
  unsigned char c_stepping; // 0xcc13

  unsigned int apic_pc_brand; // 0xcc14

  unsigned long long int f_flags; // 0xcc18
                                  // 0xcc20
} __attribute__((packed));

struct hwinf *hardware = (struct hwinf *) 0xcc00;

extern char check_cpuid_avail();

static inline int chk_fflag (unsigned char f_flag) {
  return bittest(&(hardware -> f_flags), f_flag);
}

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
  
  unsigned int fms;
  asm volatile ("cpuid"
    : "=a" (fms),
      "=b" (hardware -> apic_pc_brand),
      "=d" (vendbuf[0]),
      "=c" (vendbuf[1])
    : "a" (1));

  hardware -> c_stepping = fms & 0xf; // fms[3:0]
  hardware -> c_model = ((fms >> 12) & 0xf0) | ((fms >> 4) & 0x0f); // fms[19:16] : fms[7:4]
  hardware -> c_family = ((fms >> 20) & 0xff) + ((fms >> 8) & 0x0f); // fms[27:20] + fms[11:8]
  
}

#endif
