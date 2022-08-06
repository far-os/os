#ifndef PORT_H
#define PORT_H

unsigned char pbyte_in(unsigned short port) {
  unsigned char result;
  asm volatile ("in %%dx, %%al"
    : "=a" (result)
    : "d" (port));
  return result;
}

void pbyte_out(unsigned short port, unsigned char data) {
  asm volatile ("out %%al, %%dx"
    : : "a" (data),
        "d" (port));
}

void idle() {
  pbyte_out(0x80, 0x0); // just passing the time (1-4 microseconds)
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
