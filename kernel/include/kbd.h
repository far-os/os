#pragma once

#define K_PORT 0x60
#define K_PORT_COM 0x64

struct keystates { /* a 104-bit struct containing data */
  /*
    bit 0 set: scroll lock
    bit 1 set: num lock
    bit 2 set: caps lock
    bit 3 set: shift held
    bit 4 set: ctrl held
    bit 5 set: meta held
    bit 6 set: alt held
    bit 7 set: waiting for another scancode in a multi-scancode key
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

enum what_upd {
  CURSOR,
  COMMAND
};

static inline void charinv(unsigned char sc);

static inline void ps2_wait();

void indic_light_upd(); 

static inline void putch(char vf);

void read_kbd(); 
void cpu_reset();
