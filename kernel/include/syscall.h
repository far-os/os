#include "../syscall/string.h"
#include "../syscall/memory.h"
#include "../syscall/utilfunc.h"

#pragma once

#define SYSCALL_AMOUNT 3

void (*sys_table[SYSCALL_AMOUNT])(struct cpu_state *c, unsigned char rout) = {
  sys_0,
  sys_1,
  sys_2,
};

void syscall(struct cpu_state *c);
