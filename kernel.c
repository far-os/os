#include "text.h"

void main() {
  set_cur(POS(13, 8));
  write_str("Hello World!", -1, COLOUR(BLUE, B_YELLOW)); // x at cursor position, blue back yellow fore and blinking

  // prints cp437
  write_str("  Codepage 437  ", POS((VGA_WIDTH - 16), (VGA_HEIGHT - 16 - 1)), COLOUR(MAGENTA, B_GREEN));
  for (int cph = 0; cph < 16; ++cph) {
    for (int cpw = 0; cpw < 16; ++cpw) {
      write_cell((cph * 16) + cpw, POS((VGA_WIDTH - (16 - cpw)), (VGA_HEIGHT - (16 - cph))), COLOUR(YELLOW, B_CYAN));
    }
  }
}
