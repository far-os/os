#include "util.h"

#ifndef HWINF_H
#define HWINF_H

struct drv_param {
  unsigned short block_size; // size of the parameter block, 1ah for v1.0, 1eh for v2.0, 42h for v3.0
  unsigned short i_flags; // information flags

  unsigned int c_count; // cylinder count
  unsigned int h_count; // head count
  unsigned int s_per_track; // sectors per track
  unsigned long long int s_count; // total sector count
  unsigned short b_per_sect; // bytes per sector
  
  // v2.0 onwards
  unsigned int edd_config; // edd config parameters
  
  // v3.0 onwards
  unsigned short bedd_sig; // signature of 0xbedd to show presence of device path
  unsigned char dev_path_len; // length of device path block: should be 24h
  
  char resvd_0[3]; // reserved

  char host_bus[4]; // null terminated bus type: "ISA\0" or "PCI\0"
  char itrf_type[8]; // null terminated interface type: "ATA\0", "ATAPI\0", "SCSI\0", "USB\0", "1394\0", or "FIBRE\0"

  unsigned char itrf_path[8]; // 8 byte block: depends on the contents of host_bus
  unsigned char dev_path[8]; // 8 byte block: depends on the contents of itrf_type
  
  char resvd_1; // reserved
  
  unsigned char checksum; // checksum: the 2's complement of the 8-bit sum of all the bytes from 1eh through 40h.
                          // thus, the 8-bit sum of all bytes 1eh through 41h is 0. (including the checksum)
} __attribute__((packed));

struct hwinf {
  unsigned char bios_disk; // 0xcc00
  struct drv_param boot_disk_p; // 0xcc01
  
  unsigned int cpuid_leaves; // 0xcc43
  unsigned int cpuid_ext_leaves; // 0xcc47

  char vendor[12]; // 0xcc4b
  char brand[48]; // 0xcc57

  unsigned char c_family; // 0xcc87
  unsigned char c_model; // 0xcc88
  unsigned char c_stepping; // 0xcc89

  unsigned int apic_pc_brand; // 0xcc8a

  unsigned int f_flags_edx; // 0xcc8e
  unsigned int f_flags_ecx; // 0xcc92
                            // 0xcc96
} __attribute__((packed));

struct hwinf *hardware = (struct hwinf *) 0xcc00;

extern char check_cpuid_avail();

static inline int chk_cflag (unsigned char feat) {
  return bittest(&(hardware -> f_flags_edx), feat);
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
      "=d" (hardware -> f_flags_edx),
      "=c" (hardware -> f_flags_ecx)
    : "a" (0x00000001));

  hardware -> c_stepping = fms & 0xf; // fms[3:0]
  hardware -> c_model = ((fms >> 12) & 0xf0) | ((fms >> 4) & 0x0f); // fms[19:16] : fms[7:4]
  hardware -> c_family = ((fms >> 20) & 0xff) + ((fms >> 8) & 0x0f); // fms[27:20] + fms[11:8]
  
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
