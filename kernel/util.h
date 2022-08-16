#include "text.h"

#ifndef UTIL_H
#define UTIL_H

/*void cp437() {
  // prints cp437

  write_str_at("  Codepage 437  ", POS((VGA_WIDTH - 16), (VGA_HEIGHT - 16 - 2)), COLOUR(MAGENTA, B_GREEN));
  for (int cph = 0; cph < 16; ++cph) {
    write_cell(nybble_to_hex(cph), POS((VGA_WIDTH - 16 - 1), (VGA_HEIGHT - (16 - cph))), COLOUR(RED, WHITE));
    write_cell(nybble_to_hex(cph), POS((VGA_WIDTH - (16 - cph)), (VGA_HEIGHT - 16 - 1)), COLOUR(RED, WHITE));
    for (int cpw = 0; cpw < 16; ++cpw) {
      write_cell((cph * 16) + cpw, POS((VGA_WIDTH - (16 - cpw)), (VGA_HEIGHT - (16 - cph))), COLOUR(YELLOW, B_CYAN));
    }
  }
}*/

void memcpy(void *src, void *dest, unsigned int amount) {
  asm volatile ("cld\n"
                "rep movsb\n" :
    : "c" (amount),
      "S" (src),
      "D" (dest)
    : "memory" );
    src -= amount;
}

void strcpy(char *src, char *dest) {
  memcpy(src, dest, strlen(src));
}

unsigned char memcmp(void *src, void *dest, unsigned int amount) {
  unsigned char o;
  asm volatile ("cld\n"
                "rep cmpsb" :
      "=@ccz" (o)
    : "c" (amount),
      "S" (src),
      "D" (dest)
    : "cc" );
  return o;
}

unsigned char strcmp(char *src, char *dest) {
  return memcmp(src, dest, strlen(src)) && strlen(src) == strlen(dest);
}

void memzero(void *dest, unsigned int amount) {
  asm volatile ("cld\n"
                "rep stosb\n":
    : "a" ((unsigned char) 0),
      "c" (amount),
      "D" (dest)
    : "memory" );
}

int strlen(char *str) {
  int i = 0;
  for (; str[i] != 0; ++i);
  return i;
}

#endif
