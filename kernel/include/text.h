#pragma once

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

// viewport
#define VP_HEIGHT (is_split ? 12 : VGA_HEIGHT)

// tab is 8 spaces. i know literally noone programs with 8 space tabs, but noone uses actual tab characters anymore unless you're charles babbage himself etching code in stone
#define TAB_WIDTH 8

#define COLOUR(back, fore) (unsigned char) ((back << 4) + fore)
#define POS(x, y) ((short) (y) * VGA_WIDTH + (x))

#define VRAM_CTRL_PORT 0x3d4
#define VRAM_DATA_PORT 0x3d5

// control flags for k_apps
#define NEEDS_ENTER_CTRLCODE 0x01

#define QUEUE_LEN 8

// to future me: forint symbol that i like to notate function keys is \237
#define CTRL_BASE 0x0
#define CTRL(LET) (CTRL_BASE + (#LET[0] - 0x40))
#define FN_BASE 0x20
#define F(n) (FN_BASE + n)
#define SHIFT_FN_BASE 0x2c
#define SHIFT_F(n) (SHIFT_FN_BASE + n)
#define ALT_BASE 0x40
#define ALT(LET) (ALT_BASE + (#LET[0] - 0x40))

enum ctrl_char {
  NO_CTRL = 0,
  // ^A = 1 ... ^Z = 26
 // CTRL_C = 3, // ctrl_c
//  CTRL_S = 19, // etc.
  ENTER = 29, // a slot for enter, can either be used like this or have a new line sent
  BACKSPACE = 30, // backspace, it's not a real character (i know it doesnt map to ascii)
  DEL = 31,
  ESC = 32,
  // F1 -> F12 take up 0x21-2c (0x20 + n)
  // _F1 -> _F12 take up 0x2d-38 (0x2c + n)
  UP = 0x39,
  DOWN = 0x3a,
  LEFT = 0x3b,
  RIGHT = 0x3c,
  PGUP = 0x3d,
  PGDOWN = 0x3e,
  HOME = 0x3f,
  END = 0x40,
  // alt+A = 0x41 ... alt+Z = 0x5a
};

#define PAGE_COUNT 8
// TODO: fix maybe
#ifdef __cplusplus
extern struct f_videobuf vram;
#endif
extern short page_curloc_cache[PAGE_COUNT]; 


// PAGOFF is character offset, PAGE is memory offset
#define PAGOFF(p) (p << 11)
//#define CPAGOFF PAGOFF(vram.page)
#define PAGE(p) (vram.buf + (p << 12))
//#define CPAGE PAGE(vram.page)
//#define addr_of_loc(pos) CPAGE + (pos * 2)

extern void clear_pag(unsigned char p);
extern void clear_pag_ln(unsigned char p, int lnr);
extern void scroll_pag(unsigned char p);

int ln_nr();
void clear_scr();
void clear_ln(int lnr);
void scroll_scr();

void set_cur(short pos);
short get_cur();
void adv_cur_by(short n);
#define adv_cur() adv_cur_by(1)

void cur_off();
void cur_on_with(unsigned char, unsigned char);

#define cur_on() cur_on_with(14, 15);

void set_page(unsigned char pg);

void paint_row(unsigned char);

extern unsigned char is_split;
void split_scr(int);

//short ln_nr();
void line_feed();
void carriage_return();
void tab();
void v_tab();

//#define adv_cur() adv_cur_by(1)

void write_cell(char ch, short pos, unsigned char style);
void write_cell_cur(char ch, unsigned char style);
void write_str_at(char *str, short pos, unsigned char style);
void write_str(char *str, unsigned char style);
void write_str_into(struct f_videobuf dest, char *str, unsigned char style);
void write_cell_into(struct f_videobuf dest, char ch, unsigned char style);
