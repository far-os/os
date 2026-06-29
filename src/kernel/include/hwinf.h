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

  bool has_sse; // 0xcc97
                // 0xcc98 (or 0xcc9b? who knows?)
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

// helper macros
#define REG_EAX 0
#define REG_ECX 1
#define REG_EDX 2
#define REG_EBX 3

// there is absolutely no such thing as arithmetic shift left
// so we have to manually do this to keep the "sign"
#define CPUID_LEAF(num, reg) ((num & 0x80'00'00'00) | (num << 2) | reg)

// cpu feature flags
enum cpuid_01h_edx {
  CPUID_01H_EDX_FPU   = (1 <<  0), // is there an x87
  CPUID_01H_EDX_VME   = (1 <<  1), // v8086 enhancements
  CPUID_01H_EDX_DE    = (1 <<  2), // debugging extensions
  CPUID_01H_EDX_PSE   = (1 <<  3), // big pages
  CPUID_01H_EDX_TSC   = (1 <<  4), // time stamp counter
  CPUID_01H_EDX_MSR   = (1 <<  5), // RDMSR and WRMSR
  CPUID_01H_EDX_PAE   = (1 <<  6), // physical address extension
  CPUID_01H_EDX_MCE   = (1 <<  7), // machine check exception
  CPUID_01H_EDX_CX8   = (1 <<  8), // CMPXCHG8
  CPUID_01H_EDX_APIC  = (1 <<  9), // on-chip APIC
  /* reserved */
  CPUID_01H_EDX_SEP   = (1 << 11), // SYSENTER and SYSEXIT
  CPUID_01H_EDX_MTRR  = (1 << 12), // memory type range registers
  CPUID_01H_EDX_PGE   = (1 << 13), // page global bit
  CPUID_01H_EDX_MCA   = (1 << 14), // machine check architecture
  CPUID_01H_EDX_CMOV  = (1 << 15), // CMOVcc and FCOM{I,V}
  CPUID_01H_EDX_PAT   = (1 << 16), // page attribute table
  CPUID_01H_EDX_PSE36 = (1 << 17), // 36bit page size extension
  CPUID_01H_EDX_PSN   = (1 << 18), // processor serial number is enabled (most often isn't, smth smth security)
  CPUID_01H_EDX_CFLSH = (1 << 19), // CLFLUSH
  /* reserved */
  CPUID_01H_EDX_DS    = (1 << 21), // debug store
  CPUID_01H_EDX_ACPI  = (1 << 22), // ACPI temperature and clock facilities
  CPUID_01H_EDX_MMX   = (1 << 23), // MMX technology
  CPUID_01H_EDX_FXSR  = (1 << 24), // FXSAVE and FXRSTOR
  CPUID_01H_EDX_SSE   = (1 << 25), // SSE instruction set
  CPUID_01H_EDX_SSE2  = (1 << 26), // SSE2 instruction set
  CPUID_01H_EDX_SS    = (1 << 27), // self-snoop
  CPUID_01H_EDX_HTT   = (1 << 28), // max APIC IDs field is valid
  CPUID_01H_EDX_TM    = (1 << 29), // thermal monitor
  /* reserved */
  CPUID_01H_EDX_PBE   = (1 << 31)  // pending break enable
};
