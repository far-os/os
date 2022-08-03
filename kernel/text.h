#include "port.h"

#ifndef TEXT_H
#define TEXT_H

#define BLACK     0x0
#define BLUE      0x1
#define GREEN     0x2
#define CYAN      0x3
#define RED       0x4
#define MAGENTA   0x5
#define YELLOW    0x6
#define WHITE     0x7
#define B_BLACK   0x8
#define B_BLUE    0x9
#define B_GREEN   0xa
#define B_CYAN    0xb
#define B_RED     0xc
#define B_MAGENTA 0xd
#define B_YELLOW  0xe
#define B_WHITE   0xf

// in foreground colours, B stands for bright
// in background colours, B stands for blink

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

#define COLOUR(back, fore) (unsigned char) ((back << 4) + fore)
#define POS(x, y) (short) y * VGA_WIDTH + x

#define VRAM_CTRL_PORT 0x3d4
#define VRAM_DATA_PORT 0x3d5

char *vram = (char *) 0xb8000;

void set_cur(short pos) {
  pbyte_out(VRAM_CTRL_PORT, 0xe); // we are sending the high 8 bits of position
  pbyte_out(VRAM_DATA_PORT, ((pos >> 8) & 0x00ff)); // the high eight bits
  pbyte_out(VRAM_CTRL_PORT, 0xf); // we are sending the low 8 bits of position
  pbyte_out(VRAM_DATA_PORT, (pos & 0x00ff)); // the low eight bits
}

short get_cur() {
  short pos;
  pbyte_out(VRAM_CTRL_PORT, 0xe); // we are getting the high 8 bits of position
  pos |= ((short) pbyte_in(VRAM_DATA_PORT)) << 8; // the high eight bits
  pbyte_out(VRAM_CTRL_PORT, 0xf); // we are getting the low 8 bits of position
  pos |= pbyte_in(VRAM_DATA_PORT); // the low eight bits
  return pos;
}

void line_feed() {
  short i = get_cur() / 80;
  set_cur(++i * 80);
}

void write_cell(char ch, short pos, unsigned char style) {
  vram[pos * 2] = ch;
  vram[pos * 2 + 1] = style;
}

char nybble_to_hex(int num) {
  int value = num;
  value &= 0x0f;
  value += 0x30;
  if (value >= 0x3a) {
    value += 0x27;
  }
  return (char) value;
}

void write_hex(int input) {
  write_cell('0', 0, 0x2d);
  write_cell('x', 1, 0x2d);
  int temporary;
  for (int i = 28; i >= 0; i -= 4) {
    temporary = input;
    temporary >>= i;
    write_cell(nybble_to_hex(temporary), (36 - i) / 4, 0x2d);
  }
}


void write_str(char *str, short pos, unsigned char style) {
  short cur;
  if (pos == -1) {
    cur = get_cur();
  } else {
    cur = pos;
  }

  for (int i = 0; str[i] != 0; ++i) {
    if (str[i] == '\n') {
      line_feed();
      continue;
    }
    write_cell(str[i], cur + i, style);
    if (pos == -1) { set_cur(cur + i + 1); }
  }
}

void clear_scr() {
  for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT * 2; i += 2) {
    vram[i] = 0;
    vram[i+1] = COLOUR(BLACK, WHITE);
  }
}

#endif
