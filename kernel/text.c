#include "include/text.h"
#include "include/port.h"
#include "include/util.h"

char *vram = (char *) 0xb8000;
unsigned char page = 0;
short page_curloc_cache[PAGE_COUNT]; 

#pragma GCC push_options
#pragma GCC optimize "O3"

void set_cur(short pos) {
  if (pos >= (VGA_WIDTH * VGA_HEIGHT)) {
    pos -= VGA_WIDTH;
    scroll_scr();
  }

  pos += page << 11;
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
  return pos & 0x7ff;
}

void set_page(unsigned char pg) {
  page_curloc_cache[page] = get_cur();
  page = pg;
  set_cur(page_curloc_cache[page]);

  pbyte_out(VRAM_CTRL_PORT, 0xc); // we are sending the high 8 bits of position
  pbyte_out(VRAM_DATA_PORT, ((pg << 3) & 0x00ff)); // the high eight bits
  // always zero, no point
  //pbyte_out(VRAM_CTRL_PORT, 0xd); // we are sending the low 8 bits of position
  //pbyte_out(VRAM_DATA_PORT, ((pos << 11) & 0x00ff)); // the low eight bits
  idle();
}
#pragma GCC pop_options

short ln_nr() {
  return get_cur() / VGA_WIDTH;
}

void line_feed() {
  short i = ln_nr();
  set_cur(++i * VGA_WIDTH);
}

void carriage_return() {
  short i = ln_nr();
  set_cur(i * VGA_WIDTH);
}

void tab() {
  short i = get_cur() / 8;
  set_cur((i + 1) * 8);
}

void v_tab() {
  short i = get_cur();
  set_cur(i + VGA_WIDTH);
}

void write_cell(char ch, short pos, unsigned char style) {
  CPAGE[pos * 2] = ch;
  CPAGE[pos * 2 + 1] = style;
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

void write_str_at(char *str, short pos, unsigned char style) {
  for (short i = 0; str[i] != 0; ++i) {
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
    case '\v':
      v_tab();
      break;
    default:
      write_cell(str[i], pos + i, style);
      break;
    }
  }
}

// mostly duplicate code
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
    case '\v':
      v_tab();
      break;
    default:
      write_cell_cur(str[i], style);
      break;
    }
  }
}

void write_cell_into(struct inp_strbuf *dest, char ch, unsigned char style) {
  dest->buf[dest->ix * 2] = ch;
  dest->buf[dest->ix * 2 + 1] = style;
  dest->ix++;
}

void write_str_into(struct inp_strbuf *dest, char *str, unsigned char style) {
  for (int i = 0; str[i] != 0; ++i) {
    write_cell_into(dest, str[i], style);
  }
}
