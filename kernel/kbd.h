#include "port.h"
#include "text.h"
#include "util.h"
#include "shell.h" //cyclic include

#ifndef KBD_H
#define KBD_H

#define K_PORT 0x60
#define K_PORT_COM 0x64

struct keystates { /* a 104-bit struct containing data */
  /*
    bit 0 set: caps lock
    bit 1 set: num lock
    bit 2 set: scroll lock
    bit 3 set: shift held
    bit 4 set: ctrl held
    bit 5 set: meta held
    bit 6 set: alt held
    bit 7 set: reserved
  */
  unsigned char modifs;

  /*
    the following two comprise a 96-bit value, where each bit is whether that key is held
  */
  unsigned long long int states_low;
  unsigned int states_high;
} __attribute__((packed));

struct keystates *keys = &((struct keystates) { .states_high = 0x0, .states_low = 0x0, .modifs = 0b00000000 });

char scan_map_en_UK[96] = { // scancode map for UK keyboard.
  '\0',
  '\x1b',
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
  '\b',
  '\t',
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
  '\n',
  '\0', // TODO: LCTRL
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
  '\0', // TODO: LSHIFT
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
  '\0', // TODO: RSHIFT
  '*',
  '\0', // TODO: LALT
  ' ',
  '\0', // TODO: CAPS
  '\0', // TODO: F1
  '\0', // TODO: F2
  '\0', // TODO: F3
  '\0', // TODO: F4
  '\0', // TODO: F5
  '\0', // TODO: F6
  '\0', // TODO: F7
  '\0', // TODO: F8
  '\0', // TODO: F9
  '\0', // TODO: F10
  '\0', // TODO: NUMLOCK
  '\0', // TODO: SCROLLLOCK
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
  0x9c,
  '$',
  '%',
  '^',
  '&',
  '*',
  '(',
  ')',
  '_',
  '+',
  '\b',
  '\t',
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
  '\n',
  '\0', // TODO: LCTRL
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
  0xaa,
  '\0', // TODO: LSHIFT
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
  '\0', // TODO: RSHIFT
  '*',
  '\0', // TODO: LALT
  ' ',
  '\0', // TODO: CAPS
  '\0', // TODO: F1
  '\0', // TODO: F2
  '\0', // TODO: F3
  '\0', // TODO: F4
  '\0', // TODO: F5
  '\0', // TODO: F6
  '\0', // TODO: F7
  '\0', // TODO: F8
  '\0', // TODO: F9
  '\0', // TODO: F10
  '\0', // TODO: NUMLOCK
  '\0', // TODO: SCROLLLOCK
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

void charinv(unsigned char sc) {
  if (sc < 0x40) {
    keys -> states_low ^= (1 << sc);
  } else {
    keys -> states_high ^= (1 << (sc % 0x40));
  }
}

void read_kbd() {
  unsigned char scan = pbyte_in(K_PORT);
  charinv(scan % 0x80);
  if (scan < 0x80) {
    switch (scan) {
    case 0x3a:
      keys -> modifs ^= (1 << 0); // capslock
      return;
    case 0x45:
      keys -> modifs ^= (1 << 1); // numlock
      return;
    case 0x46:
      keys -> modifs ^= (1 << 2); // scrollock
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
      break;
    case 0x53:
      if ((keys -> modifs & 0b01010000) == 0b01010000) {
        cpu_reset();
        break;
      }
    default:
      char ascii;
      if (((keys -> modifs & 0b00001000) != 0) == ((keys -> modifs & 0b00000001) != 0)) {
        ascii = scan_map_en_UK[scan];
      } else {
        ascii = scan_map_en_UK_shift[scan];
      }

      combuf[strlen(combuf)] = ascii;
      comupd();
      break;
    }
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
