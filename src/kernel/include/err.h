#include "sched.h"
#include "misc.h"
#pragma once

// error message
enum MSG_TYPE {
  INFO    = 0, // ->
  WARN    = 1, // +
  PROGERR = 2, // !
  KERNERR = 3, // !!
  PANIC   = 4, // x
  // PANIC is special. it's not a real log level. it's technically considered a KERNERR with a special flag set to halt (i.e. bit LOGRING_LEVEL_BITS), thus leaving 2 bits for the rest of the log levels.
  // NOTE: before idiot me does something again, they must be sequential for msg_symbs[]
}; // takes two bits, hence LOGRING_LEVEL_BITS = 2

#define LOGRING_LEVEL_BITS 2
#define LOGRING_SIG_BITS  14
// signal + level combined are an unsigned short.
// 2 bits are used (explained above), so 14 bits are left, hence maximum signal of 16383

// LOGRING_SIG_BITS maximum
enum ERRSIG {
  NONE = 0,
  E_PROG = 1,
  E_EXTERN = 2,
  E_NOFILE = 3,
  E_NOSTORAGE = 4,
  E_BADADDR = 5,
  E_ARGS = 6,
  E_BOUND = 7,
  E_MATHS = 8,
  E_ILLEGAL = 9,
  E_SUICIDE = 10,
  E_UNKENTITY = 11,
  E_TIME = 12,
  E_UNSUPPORT = 13,
  E_BINLOAD = 15,
  E_CONFIG = 18,
  E_BUFOVERFLOW = 23,
  E_HANDLEALLOC = 28
};

// symbol table for signal types
extern char msg_symbs[5];

// how deep the traceback goes
#define TRACEBACK_N 2
struct logring_entry { // this is exactly one block in size
  tick_t timestamp; // current uptime when time happened
  void *traceback[TRACEBACK_N]; // traceback, in steps (from closest and furthest from current execution)

  unsigned short sig   : 14; // can't use enum ERRSIG because it must be the same enum for both fields
  unsigned short level : 2; // 29 + 3 = 32

  // next eight bits is flags
  unsigned char is_panic : 1;
  unsigned char : 0;

  unsigned char msg_blocks; // length of message in blocks (including null terminating)
  char msg[]; // flexible array member (c99+). not included in sizeof()
};

#define MAX_MSG_BLOCKS 255
#define LOGRING_BLOCK_SIZE sizeof(struct logring_entry)

// circular list, hence ring. the items are variable size, so it's a little more complicated than an ordinary circular list, as we have more pointers.
// list is segmented into blocks, of the exact size of struct logring_entry. the string is divided into blocks like this as well, and is padded with nulls if too large.
/*
  consider this example list, where each number is a block, and different numbers mean different blocks, and dashes are blank spaces:
  start > [1111122222223344--] < end
           ^               ^
           |               - next
           - earliest

  when a new value, say 555 needs to be pushed, it won't fit at the end contiguously, but that doesn't matter cause we know where it wraps. we have to invalidate each block as we write to it

  start > [511112222222334455] < end
            ^   ^
            |   |
            |   - earliest
            - next

  block 1 is no longer valid, despite half of it being there, so in logring_pushf() it is invalidated by traversing it.
*/
struct logring_t {
  // const pointers, where the address itself can't be changed
  struct logring_entry * const start; // beginning of block
  struct logring_entry * const end;   // end of block

  struct logring_entry * earliest; // earliest valid address
  struct logring_entry * next; // next available space.

  bool full;
};
extern struct logring_t logring;

#define logring_init() memzero(logring.start, (logring.end - logring.start))

void msg(enum MSG_TYPE type, enum ERRSIG sig, const char* supp, ...);
struct logring_entry * draw_msg(const struct logring_entry * const data, bool show_more);
