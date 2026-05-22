#pragma once

#include "misc.h"

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
  unsigned char mem_ents; // 0xcc01
  struct drv_param boot_disk_p; // 0xcc02

  unsigned int cpuid_leaves; // 0xcc44
  unsigned int cpuid_ext_leaves; // 0xcc48

  char vendor[12]; // 0xcc4c
  char brand[48]; // 0xcc58

  unsigned char c_family; // 0xcc88
  unsigned char c_model; // 0xcc89
  unsigned char c_stepping; // 0xcc8a

  unsigned int apic_pc_brand; // 0xcc8b

  unsigned int f_flags_edx; // 0xcc8f
  unsigned int f_flags_ecx; // 0xcc93
                            // 0xcc97
} __attribute__((packed));

enum mem_entry_type {
  USABLE = 1,
  RESERVED,
  ACPI_RECLAIMABLE,
  ACPI_NVS,
  BAD,
};

struct mem_entry {
  unsigned long long base;
  unsigned long long len;
  enum mem_entry_type type;
  unsigned int ignore: 1; // this is terrible, because ignore is if this bit is low
  unsigned int non_volatile: 1;
  unsigned int : 0; // force pad out
} __attribute__((packed));

extern struct mem_entry* mem_table;
extern struct hwinf *hardware;

extern bool check_cpuid_avail();
extern unsigned int total_bytes_mem_of_type(enum mem_entry_type);

void query_cpuid();
