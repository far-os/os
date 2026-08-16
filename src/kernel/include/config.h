#pragma once

#include "fs.h"

#define CONFIG_MAGIC 0xc091fa2b

#define NO_VERBOSE 0
#define SHOW_INFO 1 // show msg(INFO) calls
#define SHOW_STACKTRACE 2 // show stacktraces in all msg() calls

// XCONFIG.QI file
struct xconfig_qi {
  unsigned int qi_magic; // should be 0xc091fa2b
  unsigned char verbosity; // see above
  dosdate_t motd_last_shown_at; // message of the day in dosdate
} __attribute__((packed));

// this is in misc.c, because like i dont feel like creating a one-line c file
// future me, this was foreshadowing
extern struct xconfig_qi *xconf;

#define WRITE_CONF() write_file("xconfig.qi", xconf, sizeof(struct xconfig_qi))

#define BOCHS_DEBUGGER asm volatile ("xchgw %bx, %bx")
