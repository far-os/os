#pragma once

static inline unsigned char pbyte_in(unsigned short port) {
  unsigned char result;
  asm volatile ("in %%dx, %%al"
    : "=a" (result)
    : "d" (port));
  return result;
}

static inline void pbyte_out(unsigned short port, unsigned char data) {
  asm volatile ("out %%al, %%dx"
    : : "a" (data),
        "d" (port));
}

static inline unsigned short pword_in(unsigned short port) {
  unsigned char result;
  asm volatile ("in %%dx, %%ax"
    : "=a" (result)
    : "d" (port));
  return result;
}

static inline void pword_out(unsigned short port, unsigned short data) {
  asm volatile ("out %%ax, %%dx"
    : : "a" (data),
        "d" (port));
}

static inline void rep_insw(unsigned short port, unsigned int length, void *dest) {
  asm volatile ("cld\n"
      "rep insw\n" :
   : "c" (length),
     "d" (port),
     "D" (dest)
   : "memory", "cc");
}

static inline void rep_outsw(unsigned short port, unsigned int length, void *src) {
  asm volatile ("cld\n"
      "rep outsw\n" :
   : "c" (length),
     "d" (port),
     "S" (src)
   : "memory", "cc");
}

// HERE: DO NOT OPTIMISE
// any c compiler fluff is used as padding to stop it from being too fast.
// writing this in pure assembler was... error-prone to say the least
static inline void fake_outsw(unsigned short port, unsigned int length, unsigned short *src) {
  for (unsigned int ix = 0; ix < length; ++ix) {
    pword_out(port, src[ix]);
  }
}
