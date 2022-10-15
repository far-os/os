#include "util.h"

#ifndef HWINF_H
#define HWINF_H

struct hwinf {
  unsigned char bios_disk; // 0xcc00
  unsigned int cpuid_leaves; // 0xcc01
  unsigned int cpuid_ext_leaves; // 0xcc05

  char vendor[12]; // 0xcc09
  char brand[48]; // 0xcc15

  unsigned char c_family; // 0xcc45
  unsigned char c_model; // 0xcc46
  unsigned char c_stepping; // 0xcc47

  unsigned int apic_pc_brand; // 0xcc48

  unsigned long long int f_flags; // 0xcc4c
                                  // 0xcc54
} __attribute__((packed));

struct hwinf *hardware = (struct hwinf *) 0xcc00;

extern char check_cpuid_avail();

static inline int chk_cflag (unsigned char f_flag) {
  return bittest(&(hardware -> f_flags), f_flag);
}

void query_cpuid() {
  if (!check_cpuid_avail()) { // is cpuid unavailable?
    hardware -> cpuid_leaves = 0; // if so make available leaves 0
    return;
  }

  unsigned int vendbuf[3];

  // get vendor, and amount of valid functions
  asm volatile ("cpuid"
    : "=a" (hardware -> cpuid_leaves),
      "=b" (vendbuf[0]),
      "=d" (vendbuf[1]),
      "=c" (vendbuf[2])
    : "a" (0x00000000));

  memcpy(vendbuf, &(hardware -> vendor), 12);

  // amount of valid extended functions
  asm volatile ("cpuid"
    : "=a" (hardware -> cpuid_ext_leaves)
    : "a" (0x80000000)
    : "ebx", "ecx", "edx");

  // get family/model/steping and feature flags
  unsigned int fms;
  asm volatile ("cpuid"
    : "=a" (fms),
      "=b" (hardware -> apic_pc_brand),
      "=d" (vendbuf[0]),
      "=c" (vendbuf[1])
    : "a" (0x00000001));

  hardware -> c_stepping = fms & 0xf; // fms[3:0]
  hardware -> c_model = ((fms >> 12) & 0xf0) | ((fms >> 4) & 0x0f); // fms[19:16] : fms[7:4]
  hardware -> c_family = ((fms >> 20) & 0xff) + ((fms >> 8) & 0x0f); // fms[27:20] + fms[11:8]
  
  memcpy(vendbuf, &(hardware -> f_flags), 8);

  if ((hardware -> cpuid_ext_leaves & 0xff) >= 0x04) {
    int brandbuf[12];
    for (int i = 0; i < 12; i += 4) {
      asm volatile ("cpuid"
        : "=a" (brandbuf[i]),
          "=b" (brandbuf[i + 1]),
          "=c" (brandbuf[i + 2]),
          "=d" (brandbuf[i + 3])
        : "a" (i / 4 + 0x80000002));
    }
    memcpy(brandbuf, &(hardware -> brand), 48);
  }
}

#endif
