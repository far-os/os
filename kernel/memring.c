#include "include/memring.h"
#include "include/text.h"
#include "include/util.h"
#include "include/err.h"

unsigned char * memring = (unsigned char *) MEMRING_LOC;

void mem_init() {
  memzero(MEMRING_LOC, MEMRING_LEN * MEMBLK_SIZE);
}

// checks if an address is a valid memring address (i.e. not out of bounds)
static inline char is_memring(void *ptr) {
  unsigned int addr = ((unsigned int) ptr / MEMBLK_SIZE) - MEMRING_LOC;
  return addr < MEMRING_LEN;
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
  if (!(is_memring(ptr))) {
    msg(KERNERR, E_BADADDR, "free: invalid address\n");
    return;
  }

  unsigned int blocks = 1; // amount of blocks taken up
  for (unsigned char * ptr_lc = (unsigned int) ptr / MEMBLK_SIZE; !(*(ptr_lc++) & BLK_END);)
    ++blocks;

  memzero((unsigned int) ptr / MEMBLK_SIZE, blocks);
}

void * realloc(void *ptr, unsigned int len) {
  // if (!(is_memring(ptr))) {
  unsigned int blocks = 1; // amount of blocks taken up
  for (unsigned char * ptr_lc = (unsigned int) ptr / MEMBLK_SIZE; !(*(ptr_lc++) & BLK_END);)
    ++blocks;

  unsigned int new_blk = (len / MEMBLK_SIZE) + !!(len % MEMBLK_SIZE); // new block count

  unsigned int offset = (unsigned int) (ptr - MEM_LOC) / MEMBLK_SIZE;

  if (!(is_memring(ptr))) goto create_new;

  if (blocks == new_blk) return ptr;
  else if (blocks > new_blk) {
    memring[offset + new_blk - 1] |= BLK_END; // add the end where it needs to be
    memzero(&memring[offset + new_blk], blocks - new_blk); // truncate
    return ptr;
  } else { // grow
    char not_blank = 0;
    for (int i = blocks; i < new_blk; ++i) {
      not_blank |= memring[offset + i]; // if any one of them are not zero, not blank will not be zero
    }
    if (not_blank) { // can we just naively extend
      // can't naively extend
    create_new:
      void *x = malloc(len);
      memcpy(ptr, x, blocks * MEMBLK_SIZE);
      free(ptr);
      return x;
    } else { // naive extension
      memring[offset + blocks - 1] &= ~BLK_END; // delete the end
      memset(&(memring[offset + blocks]), new_blk - blocks, IN_USE);
      memring[offset + new_blk - 1] |= BLK_END; // new end
      return ptr;
    }
  }
}
