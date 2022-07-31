#ifndef PORT_H
#define PORT_H

unsigned char pbyte_in(unsigned short port) {
  unsigned char result;
  asm("in %%dx, %%al"
    : "=a" (result)
    : "d" (port));
  return result;
}

void pbyte_out(unsigned short port, unsigned char data) {
  asm("out %%al, %%dx"
    : : "a" (data),
        "d" (port));
}

/*
unsigned short pword_in(unsigned short port) {
  unsigned char result;
  asm("in %%dx, %%ax"
    : "=a" (result)
    : "d" (port));
  return result;
}

void pword_out(unsigned short port, unsigned short data) {
  asm("out %%ax, %%dx"
    : : "a" (result),
        "d" (port));
}
*/

#endif
