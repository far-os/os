extern "C" {
#include "include/text.h"
#include "include/kappldr.h"
#include "include/port.h"
#include "include/util.h"
}
#include "include/textbufs.hh"

// shoehorning
struct f_videobuf vram(0xb8000, VGA_WIDTH * VGA_HEIGHT * PAGE_COUNT);

short page_curloc_cache[PAGE_COUNT]; 
unsigned char is_split = 0;

// ok so c++ is stupid. this is 22yo bug: https://lists.debian.org/debian-gcc/2003/07/msg00057.html
void* __dso_handle = (void*) &__dso_handle;
void* __cxa_atexit = (void*) &__dso_handle;

// ok so funny story - with no optimisations, there were inexplicable crashes originating from these functions.
// so "what if i optimise everything" caused the crashes to come from elsewhere. and eventually after much trial and error, we've reached this.
// only optimising these functions in particular, as if unoptimised gcc has a personal vendetta against manipulating the vga buffer
// don't touch it, it works, and i would like it to work for the foreseeable future
#pragma GCC push_options
#pragma GCC optimize "O3"

void set_cur(short pos) {
  if (pos >= (VGA_WIDTH * VP_HEIGHT)) {
    pos -= VGA_WIDTH;
    scroll_pag(vram.page);
  }

  pos += vram.page << 11;
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
  page_curloc_cache[vram.page] = get_cur();
  vram.page = pg;
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

// end of the compiler's bullshit (one would hope)
#pragma GCC pop_options

// - functions to interface with plebeian c code

// thunked away using write_str. done for a good reason that i'll almost certainly forget in 5 minutes, so here:
// all these functions depend on the actual cursor, which ofc IS NOT (i cant stress this enough) `vram.ix'. `vram.ix' IS THE LAST USED LOCATION AND SHOULD NOT BE TRUSTED. THIS IS TO PREVENT EXCESSIVE I/O WRITES.
// this is the only (easy) way to update vram.ix without much repetition
void line_feed()        { write_str("\n", 0); }
void carriage_return()  { write_str("\r", 0); }
void tab()              { write_str("\t", 0); }
void v_tab()            { write_str("\v", 0); }
void adv_cur_by(short n) { set_cur(get_cur() + n); }

int ln_nr() { return get_cur() / VGA_WIDTH; }

void clear_scr() { clear_pag(vram.page); }
void clear_ln(int lnr) { clear_pag_ln(vram.page, lnr); }
void scroll_scr() { scroll_pag(vram.page); }

void write_cell(char ch, short pos, unsigned char style) {
  vram.ix = pos;
  write_cell_into(vram, ch, style);
}

void write_cell_cur(char ch, unsigned char style) {
  vram.ix = get_cur();
  write_cell_into(vram, ch, style);
  set_cur(vram.ix);
}

void write_str_at(char *str, short pos, unsigned char style) {
  vram.ix = pos;
  write_str_into(vram, str, style);
}

// mostly duplicate code
void write_str(char *str, unsigned char style) {
  vram.ix = get_cur();
  write_str_into(vram, str, style);
  set_cur(vram.ix);
}

// - end of plebeian c interface

void write_cell_into(struct f_videobuf dest, char ch, unsigned char style) {
  unsigned int *x = (unsigned int *)0xb8000;
  x[0] = (unsigned int) &(*dest) + 1;
  *dest = (struct fchar) {
    .ch = ch,
    .fmt = style
  };
  dest.ix++;
}

// now why should i write into an input
void write_str_into(struct f_videobuf dest, char *str, unsigned char style) {
  for (int i = 0; str[i] != 0; ++i) {
    switch (str[i]) {
    case '\n':
      dest.line_feed();
      break;
    case '\t':
      dest.tab();
      break;
    case '\r':
      dest.carriage_return();
      break;
    case '\v':
      dest.v_tab();
      break;
    default:
      write_cell_into(dest, str[i], style);
      break;
    }
  }
}
