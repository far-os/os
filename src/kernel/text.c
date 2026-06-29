#include "include/text.h"
#include "include/kappldr.h"
#include "include/port.h"
#include "include/util.h"

volatile struct char_packet *vram = (struct char_packet *) 0xb8000;
unsigned char page = 0;
curpos_t page_curloc_cache[PAGE_COUNT];
unsigned char is_split = 0;

#pragma GCC push_options
#pragma GCC optimize "O3"

void set_cur(curpos_t pos) {
  if (pos >= (VGA_WIDTH * VP_HEIGHT)) {
    pos -= VGA_WIDTH;
    scroll_scr();
  }

  pos += page << 11;
  pbyte_out(VRAM_CTRL_PORT, 0xe); // we are sending the high 8 bits of position
  pbyte_out(VRAM_DATA_PORT, ((pos >> 8) & 0x00ff)); // the high eight bits
  pbyte_out(VRAM_CTRL_PORT, 0xf); // we are sending the low 8 bits of position
  pbyte_out(VRAM_DATA_PORT, (pos & 0x00ff)); // the low eight bits
}

curpos_t get_cur() {
  curpos_t pos = 0;
  pbyte_out(VRAM_CTRL_PORT, 0xe); // we are getting the high 8 bits of position
  pos |= ((curpos_t) pbyte_in(VRAM_DATA_PORT)) << 8; // the high eight bits
  pbyte_out(VRAM_CTRL_PORT, 0xf); // we are getting the low 8 bits of position
  pos |= pbyte_in(VRAM_DATA_PORT); // the low eight bits
  return pos & 0x7ff;
}

void cur_off() {
  pbyte_out(VRAM_CTRL_PORT, 0xa);
  pbyte_out(VRAM_DATA_PORT, 0x20); // off
}

void cur_on_with(unsigned char start_scl, unsigned char end_scl) {
  pbyte_out(VRAM_CTRL_PORT, 0xa);
  pbyte_out(VRAM_DATA_PORT, (pbyte_in(VRAM_DATA_PORT) & 0xc0) | start_scl);
  pbyte_out(VRAM_CTRL_PORT, 0xb);
  pbyte_out(VRAM_DATA_PORT, (pbyte_in(VRAM_DATA_PORT) & 0xe0) | end_scl);
}

void set_page(unsigned char pg) {
  page_curloc_cache[page] = get_cur();
  page = pg;
  set_cur(page_curloc_cache[pg]);

  if (pg == 0 && is_split) { return; }

  pbyte_out(VRAM_CTRL_PORT, 0xc); // we are sending the high 8 bits of position
  pbyte_out(VRAM_DATA_PORT, ((pg << 3) & 0x00ff)); // the high eight bits
  // always zero, no point
  //pbyte_out(VRAM_CTRL_PORT, 0xd); // we are sending the low 8 bits of position
  //pbyte_out(VRAM_DATA_PORT, ((pos << 11) & 0x00ff)); // the low eight bits
  idle();
}

void split_scr(app_handle app) {
  // ok so the line compare register is quite compicated
  // bits 7:0 => r18h[7:0], bit 8 => r07h[4], bit 9 => r09h[6]
  // this shows the scanline (pixel vertically) at which to reset to offset 0 of memory

  // scanline 0 is used to unsplit, we want 0 if 0, 1 otherwise
  is_split = !!(app && app_db[app]);

  short scanline = is_split ? 200 : 0;

  pbyte_out(VRAM_CTRL_PORT, 0x18); // 7:0
  pbyte_out(VRAM_DATA_PORT, scanline & 0xff);

  pbyte_out(VRAM_CTRL_PORT, 0x7); // 8 
  unsigned char old = pbyte_in(VRAM_DATA_PORT);
  old &= ~((~scanline & 0x100) >> 4); // 8 - 4 = 4
  pbyte_out(VRAM_DATA_PORT, old);

  pbyte_out(VRAM_CTRL_PORT, 0x9); // 9 
  old = pbyte_in(VRAM_DATA_PORT);
  old &= ~((~scanline & 0x200) >> 3); // 9 - 6 = 3
  pbyte_out(VRAM_DATA_PORT, old);
}
#pragma GCC pop_options

void paint_row(unsigned char colour) {
  int ln = ln_nr();
  for (int k = 0; k < VGA_WIDTH; ++k) {
    CPAGE[POS(k, ln)].style &= 0xf;
    CPAGE[POS(k, ln)].style |= colour << 4;
  }
}

curpos_t ln_nr() {
  return get_cur() / VGA_WIDTH;
}

curpos_t write_cell_packet(curpos_t pos, struct char_packet pack, bool advanced) {
  // if we're not advanced, we ignore all the case arms, and jump straight to the default behaviour
  if (!advanced) goto plain_putch;

  switch (pack.ch) {
  case '\n':
    pos = (pos / VGA_WIDTH + 1) * VGA_WIDTH;
    break;
  case '\t':
    pos = ((pos / 8) + 1) * 8;
    break;
  case '\r':
    pos = (pos / VGA_WIDTH) * VGA_WIDTH;
    break;
  case '\v':
    pos += VGA_WIDTH;
    break;
  case '\b':
    pos--; // backspace
    break;
  plain_putch:
  default:
    CPAGE[pos++] = pack;
    break;
  }

  return pos;
}

void adv_cur_by(curpos_t n) {
  curpos_t cur = get_cur();
  cur += n;
  set_cur(cur);
}

void write_str_at(const char *str, curpos_t pos, unsigned char style) {
  bool is_cur = (pos == -1); // is cursor
  if (is_cur) pos = get_cur();

  for (unsigned int i = 0; str[i] != 0; ++i) {
    // write changed pos to running pos
    pos = write_advanced_cell(str[i], pos, style);
  }

  // if cursor, set cursor to final position. avoids having to constantly set the cursor after every char
  if (is_cur) set_cur(pos);
}

// uses different logic to above. TODO: fix with generics for inp_strbuf
void write_cell_into(struct inp_strbuf *dest, char ch, unsigned char style) {
  dest->buf[dest->ix * 2] = ch;
  dest->buf[dest->ix * 2 + 1] = style;
  dest->ix++;
}

void write_str_into(struct inp_strbuf *dest, const char *str, unsigned char style) {
  for (int i = 0; str[i] != 0; ++i) {
    write_cell_into(dest, str[i], style);
  }
}
