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

extern void clear_scr();
extern void clear_ln(int lnr);
extern void scroll_scr();

#pragma GCC push_options
#pragma GCC optimize "O3"

void set_cur(short pos) {
  if (pos >= (VGA_WIDTH * VGA_HEIGHT)) {
    pos -= VGA_WIDTH;
    scroll_scr();
  }
  pbyte_out(VRAM_CTRL_PORT, 0xe); // we are sending the high 8 bits of position
  pbyte_out(VRAM_DATA_PORT, ((pos >> 8) & 0x00ff)); // the high eight bits
  pbyte_out(VRAM_CTRL_PORT, 0xf); // we are sending the low 8 bits of position
  pbyte_out(VRAM_DATA_PORT, (pos & 0x00ff)); // the low eight bits
}

short get_cur() {
  short pos = 0;
  pbyte_out(VRAM_CTRL_PORT, 0xe); // we are getting the high 8 bits of position
  pos |= ((short) pbyte_in(VRAM_DATA_PORT)) << 8; // the high eight bits
  pbyte_out(VRAM_CTRL_PORT, 0xf); // we are getting the low 8 bits of position
  pos |= pbyte_in(VRAM_DATA_PORT); // the low eight bits
  return pos;
}

#pragma GCC pop_options

void line_feed() {
  short i = get_cur() / VGA_WIDTH;
  set_cur(++i * VGA_WIDTH);
}

void carriage_return() {
  short i = get_cur() / VGA_WIDTH;
  set_cur(i * VGA_WIDTH);
}

void tab() {
  short i = get_cur();
  set_cur(i + 8);
}

void write_cell(char ch, short pos, unsigned char style) {
  vram[pos * 2] = ch;
  vram[pos * 2 + 1] = style;
}

void adv_cur() {
  short cur = get_cur();
  ++cur;
  set_cur(cur);
}


void write_cell_cur(char ch, unsigned char style) {
  short cur = get_cur();
  write_cell(ch, cur, style);
  adv_cur();
}

void write_str(char *str, unsigned char style) {
  for (int i = 0; str[i] != 0; ++i) {
    switch (str[i]) {
    case '\n':
      line_feed();
      break;
    case '\t':
      tab();
      break;
    case '\r':
      carriage_return();
      break;
    default:
      write_cell_cur(str[i], style);
      break;
    }
  }
}

void write_str_at(char *str, short pos, unsigned char style) {
  short x = get_cur();
  set_cur(pos);
  write_str(str, style);
  set_cur(x);
}

/*void clear_scr() {
  asm volatile ("cld\n" "rep stosw" :
  : "a" ((short) (COLOUR(BLACK, WHITE) << 8)),
    "c" ((int) VGA_WIDTH * VGA_HEIGHT),
    "D" ((int) vram)
  : "memory");
}*/


#endif
