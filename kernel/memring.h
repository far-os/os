#include "text.h"
#include "util.h"

#ifndef MEMRING_H
#define MEMRING_H

#define MEMBLK_SIZE 16
#define MEMRING_LOC 0x12000
#define MEM_LOC (MEMRING_LOC * MEMBLK_SIZE)
#define MEMRING_LEN 1024 // 16k of mem free

#define FREE 0x00
#define BLK_START 0x01 // 0b01
#define IN_USE 0x02 // 0b10
#define BLK_END 0x04 // 0b100

// the memring sits at 0x12_000 whereas the memory it points to sits at 0x120_000 (<< 4)
// each byte in the memring points to a 16 byte block

unsigned char * memring = (unsigned char *) MEMRING_LOC;

void mem_init() {
  memzero(MEMRING_LOC, MEMRING_LEN * MEMBLK_SIZE);
}

void * malloc(unsigned int len) {
  unsigned int blocks = (len / MEMBLK_SIZE) + !!(len % MEMBLK_SIZE); // amount of blocks taken up
  unsigned int run = 0;
  for (; run < MEMRING_LEN; ++run) { // loop through all memrings
    if (memring[run]) { continue; }

    else {
      char not_blank = 0;
      for (int i = 0; i < blocks; ++i) {
        not_blank |= memring[run + i]; // if any one of them are not zero, not blank will not be zero
      }
      if (!not_blank) { break; }
    }
  }
  
  memset(&(memring[run]), blocks, IN_USE);
  memring[run] |= BLK_START;
  memring[run + blocks - 1] |= BLK_END;
  
  void * ptr = (void *) MEM_LOC + (run * MEMBLK_SIZE);
  memzero(ptr, blocks * MEMBLK_SIZE);
  return ptr;
}

void free(void * ptr) {
  unsigned int blocks = 1; // amount of blocks taken up
  for (unsigned char * ptr_lc = (unsigned int) ptr / MEMBLK_SIZE; !(*(ptr_lc++) & BLK_END);)
    ++blocks;

  memzero((unsigned int) ptr / MEMBLK_SIZE, blocks);
}

#endif
