#include "port.h"
#include "text.h"
#include "util.h"
#include "kapps/kshell.h" //cyclic include

#ifndef KBD_H
#define KBD_H

#define K_PORT 0x60
#define K_PORT_COM 0x64

// already decl. in shell
struct keystates *keys = &((struct keystates) { .states_high = 0x0, .states_low = 0x0, .modifs = 0b00000000 });

char scan_map_en_UK[96] = { // scancode map for UK keyboard.
  '\0',
  '\x1b', // ESC
  '1',
  '2',
  '3',
  '4',
  '5',
  '6',
  '7',
  '8',
  '9',
  '0',
  '-',
  '=',
  '\b', // BACK
  '\t', // TAB
  'q',
  'w',
  'e',
  'r',
  't',
  'y',
  'u',
  'i',
  'o',
  'p',
  '[',
  ']',
  '\n', // ENTER
  '\0', // LCTRL
  'a',
  's',
  'd',
  'f',
  'g',
  'h',
  'j',
  'k',
  'l',
  ';',
  '\'',
  '`',
  '\0', // LSHIFT
  '#',
  'z',
  'x',
  'c',
  'v',
  'b',
  'n',
  'm',
  ',',
  '.',
  '/',
  '\0', // RSHIFT
  '*', // KEYPAD
  '\0', // LALT
  ' ',
  '\0',
  '\0', // TODO: F1
  '\0', // TODO: F2
  '\0', // TODO: F3
  '\0', // TODO: F4
  '\0', // TODO: F5
  '\0', // TODO: F6
  '\0', // TODO: F7
  '\0', // TODO: F8
  '\0', // TODO: F9
  '\0', // shift+f10 => clrscr
  '\0', // NUMLCK
  '\0', // SCRLLCK
  '7', // KEYPAD
  '8', // KEYPAD
  '9', // KEYPAD
  '-', // KEYPAD
  '4', // KEYPAD
  '5', // KEYPAD
  '6', // KEYPAD
  '+', // KEYPAD
  '1', // KEYPAD
  '2', // KEYPAD
  '3', // KEYPAD
  '0', // KEYPAD
  '.', // KEYPAD
  '\0', // 0x54
  '\0', // 0x55
  '\\',
  '\0', // TODO: F11
  '\0' // TODO: F12
};

char scan_map_en_UK_shift[96] = { // scancode map for UK keyboard.
  '\0',
  '\x1b',
  '!',
  '"',
  0x9c, // £ in cp437
  '$',
  '%',
  '^',
  '&',
  '*',
  '(',
  ')',
  '_',
  '+',
  '\b', // BACK
  '\t', // TAB
  'Q',
  'W',
  'E',
  'R',
  'T',
  'Y',
  'U',
  'I',
  'O',
  'P',
  '{',
  '}',
  '\n', // ENTER
  '\0', // LCTRL
  'A',
  'S',
  'D',
  'F',
  'G',
  'H',
  'J',
  'K',
  'L',
  ':',
  '@',
  0xaa, // ¬ in cp437
  '\0', // LSHIFT
  '~',
  'Z',
  'X',
  'C',
  'V',
  'B',
  'N',
  'M',
  '<',
  '>',
  '?',
  '\0', // RSHIFT
  '*', // KEYPAD
  '\0', // LALT
  ' ',
  '\0',
  '\0', // TODO: F1
  '\0', // TODO: F2
  '\0', // TODO: F3
  '\0', // TODO: F4
  '\0', // TODO: F5
  '\0', // TODO: F6
  '\0', // TODO: F7
  '\0', // TODO: F8
  '\0', // TODO: F9
  '\0', // F10: shift+f10 to clear screen
  '\0', // NUMLCK
  '\0', // SCRLLCK
  '7', // KEYPAD
  '8', // KEYPAD
  '9', // KEYPAD
  '-', // KEYPAD
  '4', // KEYPAD
  '5', // KEYPAD
  '6', // KEYPAD
  '+', // KEYPAD
  '1', // KEYPAD
  '2', // KEYPAD
  '3', // KEYPAD
  '0', // KEYPAD
  '.', // KEYPAD
  '\0', // 0x54
  '\0', // 0x55
  '\\',
  '\0', // TODO: F11
  '\0' // TODO: F12
};

static inline void charinv(unsigned char sc) {
  /*if (sc < 0x40) {
    keys -> states_low ^= (1 << sc);
  } else {
    keys -> states_high ^= (1 << (sc % 0x40));
  }*/

  bitinv(&(keys -> states_low), sc);
}

static inline void ps2_wait() { // wait for ps2 controller to be ready
  while (pbyte_in(0x64) & 0x02);
}

void indic_light_upd() { // update indicator lights
  ps2_wait();
  pbyte_out(K_PORT, 0xed);
  ps2_wait();
  pbyte_out(K_PORT, (keys -> modifs) & 0b00000111);
  ps2_wait();
}

char alt_code_buf[4];
int xgg = 0;

unsigned char quitting_prog = 0;

static inline void putch(char vf) {
  if (actv->ix >= (actv -> len)) return;
  backmemcpy(actv->buf + actv->len - 2, actv->buf + actv->len - 1, actv->len - (actv->ix + 1));
  actv -> buf[actv -> ix] = vf;
  actv -> ix++;
  comupd();
}

enum what_upd {
  CURSOR,
  COMMAND
};

void read_kbd() {
  unsigned char scan = pbyte_in(K_PORT);
  charinv(scan % 0x80);
  if (scan < 0x80) {
    if (keys -> modifs & 0b10000000) { // extended keys
      switch (scan) {
      default:
        break; // TODO: Cursor keys
      }
    }
    switch (scan) {
    case 0x46:
      keys -> modifs ^= (1 << 0); // scrollock
      indic_light_upd();
      return;
    case 0x45:
      keys -> modifs ^= (1 << 1); // numlock
      indic_light_upd();
      return;
    case 0x3a:
      keys -> modifs ^= (1 << 2); // capslock
      indic_light_upd();
      return;
    case 0x2a:
    case 0x36:
      keys -> modifs |= (1 << 3); // shift
      break;
    case 0x1d:
      keys -> modifs |= (1 << 4); // lctrl
      break;
    case 0x38:
      keys -> modifs |= (1 << 6); // lalt
      xgg = 0;
      memzero(alt_code_buf, 4);
      break;

    case 0x44: // shift-f10
      if (keys -> modifs & (1 << 3)) {
        clear_scr();
        set_cur(0);
        comupd();
      }
      break;

    case 0x47:
    case 0x48:
    case 0x49:
    case 0x4b:
    case 0x4c:
    case 0x4d:
    case 0x4f:
    case 0x50:
    case 0x51:
    case 0x52:
      if (keys -> modifs & (1 << 6)) {
        if (xgg < 3) {
          alt_code_buf[xgg++] = scan_map_en_UK[scan];
        } else if (xgg > 0) goto altput; // goto? what's the worst that could happen!
                                         // what do you mean i've angered the programming gods?

        break;
      }

    case 0x37:
    case 0x4a:
    case 0x4e:
    case 0x53:
      if ((keys -> modifs & 0b01010000) == 0b01010000 && scan == 0x53) { // ctrl alt del
        cpu_reset();
        break;
      }
      if (!(keys -> modifs & (1 << 1)) || keys -> modifs & (1 << 7)) {
        if (scan == 0x48 && IS_COM) {
          // up_key
          sh_hist_restore();
        } else if (scan == 0x4b) {
          // left_key
          actv -> ix--;
          curupd();
        } else if (scan == 0x4d) {
          // right_key
          actv -> ix++;
          curupd();
        } else if (scan == 0x53) {
          // del_key
          if (actv->ix >= (actv -> len)) break;
          memcpy(actv -> buf + actv -> ix + 1, actv -> buf + actv -> ix, actv -> len - (actv -> ix + 1));
          clear_ln(ln_nr());
          comupd();
        } else if (scan == 0x47) {
          // home_key
          actv -> ix = 0;
          comupd();
        } else if (scan == 0x4f) {
          actv -> ix = strlen(actv -> buf);
          comupd();
        } 
        break; // TODO: down cursor key
      }
    default:
      if (scan == 0x2e && keys -> modifs & (1 << 4) && IS_COM) { // ctrl-c
        sh_ctrl_c();
        break;
      }

      if (scan == 0x1f && keys -> modifs & (1 << 4) && IS_USR) { // ctrl-s
        usr_ctrl_s();
        break;
      }
      char ascii;
      char is_letter = scan_map_en_UK[scan] >= 'a' && scan_map_en_UK[scan] <= 'z';
      if (!!(keys -> modifs & 0b00001000) != ((keys -> modifs & 0b00000100) && is_letter)) {
        ascii = scan_map_en_UK_shift[scan];
      } else {
        ascii = scan_map_en_UK[scan];
      }

      putch(ascii);
      break;
    }
    keys -> modifs &= ~(1 << 7); // TODO: fix
  } else {
    switch (scan) {
    case 0xaa:
    case 0xb6:
      keys -> modifs &= ~(1 << 3); // shift i think
      break;
    case 0x9d:
      keys -> modifs &= ~(1 << 4); // ctrl i think
      break;
    case 0xb8:
      keys -> modifs &= ~(1 << 6); // alt i think
      
      altput:
        if (xgg) {
          unsigned int fl = to_uint(alt_code_buf);
          if (!fl || fl >= 256) break;
          putch(fl);
          memzero(alt_code_buf, 4);
        }


      break;
    case 0xe0:
      keys -> modifs |= (1 << 7); // extend
      break;
    }
    // TODO: release values
  }

  //write_cell(scan_map_en_UK[scan], get_cur(), COLOUR(BLACK, WHITE));
//  set_cur(get_cur() + 1);
}

void cpu_reset() {
  pbyte_out(K_PORT_COM, 0xfe);
}

#endif
