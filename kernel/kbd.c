#include "include/kbd.h"
#include "include/port.h"
#include "include/text.h"
#include "include/util.h"
#include "include/ih.h"
#include "include/kappldr.h"

struct keystates *keys = &((struct keystates) { .states_high = 0x0, .states_low = 0x0, .modifs = 0b00000000 });

char scan_map_en_UK[96] = { // scancode map for UK keyboard.
  0, '\x1b' /* esc */, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\0' /* backspace */,
  '\t' /* tab */, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n' /* enter */,
  '\0' /* lctrl */, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
  '\0' /* lshift */, '#', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', '\0' /* rshift */,
  '*' /* keypad times */,
  '\0' /* lalt */, ' ', 0, '\0' /* f1 */, '\0' /* f2 */, '\0' /* f3 */, '\0' /* f4 */, '\0' /* f5 */, '\0' /* f6 */, '\0' /* f7 */, '\0' /* f8 */, '\0' /* f9 */, '\0' /* f10 */,
  '\0' /* numlck */, '\0' /* scrlck */,
  // keypad
  '7', '8', '9', '-', 
  '4', '5', '6', '+', 
  '1', '2', '3', 
  '0', '.', 
  0 /* 0x54 */, 0 /* 0x55 */,
  '\\',
  '\0' /* f11 */, '\0' /* f12 */
};

char scan_map_en_UK_shift[96] = { // scancode map for UK keyboard.
  0, '\x1b' /* esc */, '!', '"', 0x9c /* £ in cp437 */, '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b' /* backspace */,
  '\t' /* tab */, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n' /* enter */,
  '\0' /* lctrl */, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '@', 0xaa /* ¬ in cp437 */,
  '\0' /* lshift */, '~', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', '\0' /* rshift */,
  '*' /* keypad times */,
  '\0' /* lalt */, ' ', 0, '\0' /* f1 */, '\0' /* f2 */, '\0' /* f3 */, '\0' /* f4 */, '\0' /* f5 */, '\0' /* f6 */, '\0' /* f7 */, '\0' /* f8 */, '\0' /* f9 */, '\0' /* f10 */,
  '\0' /* numlck */, '\0' /* scrlck */,
  // keypad
  '7', '8', '9', '-', 
  '4', '5', '6', '+', 
  '1', '2', '3', 
  '0', '.', 
  0 /* 0x54 */, 0 /* 0x55 */,
  '\\',
  '\0' /* f11 */, '\0' /* f12 */
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

#define KEY 0
#define CTRL 1
// put chatacter
void putch(char vf, unsigned char where) {
  // where = 1 => ctrl; = 0 => key
  char *curr_buf = where ? app_db[curr_kapp]->ctrl_q : app_db[curr_kapp]->key_q;

  int len = strlen(curr_buf);
  if (len + 1 >= QUEUE_LEN) {
    return; // key dropped
  }
  curr_buf[len] = vf;
  (**app_db[curr_kapp]->invoke)(app_db[curr_kapp]);
}

void read_kbd() {
  unsigned char scan = pbyte_in(K_PORT);
  charinv(scan % 0x80);
  if (scan < 0x80) {
    if (keys -> modifs & 0b10000000) { // extended keys
      switch (scan) {
        default: break; // Cursor keys
      }
    }
    switch (scan) {
    case 0x0e:
      putch(BACKSPACE, CTRL);
      break;
    case 0x1c: // enter key
      if (app_db[curr_kapp]->config_flags & NEEDS_ENTER_CTRLCODE) putch(ENTER, CTRL);
      else putch('\n', KEY);

      break;
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
        putch(SHIFT_F10, CTRL); // TODO: jank, needs improving
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
        if (scan == 0x48) {
          // up
          putch(UP, CTRL);
        } else if (scan == 0x4b) {
          // left
          putch(LEFT, CTRL);
        } else if (scan == 0x50) {
          // down
          putch(RIGHT, CTRL);
        } else if (scan == 0x4d) {
          // right
          putch(RIGHT, CTRL);
        } else if (scan == 0x53) {
          // del
          putch(DEL, CTRL);
        } else if (scan == 0x47) {
          // home
          putch(HOME, CTRL);
        } else if (scan == 0x4f) {
          // end
          putch(END, CTRL);
        } 
        break;
      }
    default:
      char ascii;
      char is_letter = scan_map_en_UK[scan] >= 'a' && scan_map_en_UK[scan] <= 'z';
      if (keys -> modifs & (1 << 4) && is_letter) { // ctrl key sequence (^A, ^B, etc)
        putch(scan_map_en_UK[scan] - 0x60 + CTRL_BASE, CTRL);
        break;
      }
      if (!!(keys -> modifs & 0b00001000) != ((keys -> modifs & 0b00000100) && is_letter)) {
        ascii = scan_map_en_UK_shift[scan];
      } else {
        ascii = scan_map_en_UK[scan];
      }

      putch(ascii, KEY);
      break;
    }
    keys -> modifs &= ~(1 << 7); // fix | thanks past me, what needs fixing?
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
          putch(fl, KEY);
          memzero(alt_code_buf, 4);
        }


      break;
    case 0xe0:
      keys -> modifs |= (1 << 7); // extend
      break;
    }
    // release values | laziness
  }

  //write_cell(scan_map_en_UK[scan], get_cur(), COLOUR(BLACK, WHITE));
//  set_cur(get_cur() + 1);
}

void cpu_reset() {
  pbyte_out(K_PORT_COM, 0xfe);
}
