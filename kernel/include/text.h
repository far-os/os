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

#define COLOUR(back, fore) (unsigned char) ((back << 4) + fore)
#define POS(x, y) (short) (y) * VGA_WIDTH + (x)

#define VRAM_CTRL_PORT 0x3d4
#define VRAM_DATA_PORT 0x3d5

#define PAGE_COUNT 8
char *vram = (char *) 0xb8000;
unsigned char page = 0;
short page_curloc_cache[PAGE_COUNT]; 

#define PAGE(p) (vram + (p << 12))
#define CPAGE PAGE(page)

extern void clear_pag(unsigned char p);
extern void clear_pag_ln(unsigned char p, int lnr);
extern void scroll_pag(unsigned char p);

#define clear_scr() clear_pag(page)
#define clear_ln(lnr) clear_pag_ln(page, lnr)
#define scroll_scr() scroll_pag(page)

void set_cur(short pos);
short get_cur();
void set_page(unsigned char pg);

short ln_nr();
void line_feed();
void carriage_return();
void tab();
void v_tab();
void adv_cur();
void write_cell(char ch, short pos, unsigned char style);
void write_cell_cur(char ch, unsigned char style);
void write_str_at(char *str, short pos, unsigned char style);
void write_str(char *str, unsigned char style);
