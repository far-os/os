#include "include/ih.h"
#include "include/syscall.h"

void syscall(struct cpu_state *c) {
  unsigned char serv, rout;
  asm volatile ("movb %%ch, %%dl" : "=d" (serv), "=c" (rout) : "c" (c -> eax));

  (*sys_table[serv])(c, rout);

}
