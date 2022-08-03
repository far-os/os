#include "text.h"
#include "util.h"

struct idt_entry {
  unsigned short offset_high;
  unsigned char flags;
  unsigned char reserved;
  unsigned short segment;
  unsigned short offset_low;
} __attribute__((packed));

void main() {
//  clear_scr();

  set_cur(POS(0, 0));
  write_str("Welcome to the Kernel!\n", -1, COLOUR(CYAN, B_YELLOW));
  cp437();
}
