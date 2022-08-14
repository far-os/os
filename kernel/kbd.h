#include "port.h"
#include "text.h"
#include "util.h"
#include "shell.h" //cyclic include

#ifndef KBD_H
#define KBD_H

#define K_PORT 0x60

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

void read_kbd() {
  unsigned char scan = pbyte_in(K_PORT) % 0x80;
  combuf[strlen(combuf)] = scan_map_en_UK[scan];
  comupd();
  //write_cell(scan_map_en_UK[scan], get_cur(), COLOUR(BLACK, WHITE));
//  set_cur(get_cur() + 1);
}

#endif
