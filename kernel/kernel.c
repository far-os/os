#include "text.h"

void main() {
//  clear_scr();

  set_cur(POS(0, 0));
  write_str("Welcome to the Kernel!\n", -1, COLOUR(CYAN, B_YELLOW));
  cp437();
}
