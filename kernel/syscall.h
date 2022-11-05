#include "defs.h"
#include "syscall/string.h"

#ifndef SYSCALL_H
#define SYSCALL_H

#define SYSCALL_AMOUNT 1

struct cpu_state;

void (*sys_table[SYSCALL_AMOUNT])(struct cpu_state *c, unsigned char rout) = {
  sys_0
};

void syscall(struct cpu_state *c) {
  unsigned char serv, rout;
  asm volatile ("movb %%ch, %%dl" : "=d" (serv), "=c" (rout) : "c" (c -> eax));

  (*sys_table[serv])(c, rout);

}

#endif
