#include "text.h"
#include "util.h"
#include "pic.h"
#include "ih.h"

struct idt_entry {
  unsigned short offset_low; // low 16 bits of offset
  unsigned short segment; // segment selector: the segment in the gdt
  unsigned char reserved; // idk
  unsigned char flags; // this is a huge red flag *badum tss*
  unsigned short offset_high; // high 16 bits of offset
} __attribute__((packed));

struct idtr_contents {
  unsigned short size;
  unsigned int offset;
} __attribute__((packed));

extern void *eh_list[];

void main() {
  clear_scr();

  set_cur(POS(0, 0)); // cursor at top left
  write_str("Welcome to the Kernel!\n", -1, COLOUR(CYAN, B_YELLOW)); // welcome message
  cp437(); // codepage 437: for testing purposes

  __attribute__((aligned(0x10)))
  static struct idt_entry idt[256]; // the entire idt
  static struct idtr_contents idtr; // idtr

  idtr.offset = (unsigned int) &idt;
  idtr.size   = (unsigned short) sizeof(struct idt_entry) * 256 - 1;

  for (unsigned char d = 0; d < 48; ++d) {
    struct idt_entry *desc = &idt[d];
    desc->offset_low = (unsigned int) eh_list[d] & 0xffff;
    desc->segment    = 0x08; // first in gdt
    desc->reserved = 0; // haha
    desc->flags = 0x8e; // a huge red flag
    desc->offset_high = (unsigned int) eh_list[d] >> 16;
  }

  asm volatile ("lidt %0" : : "m"(idtr)); // load idt
  asm volatile ("sti"); // set interrupt (opposite of cli)

  pic_init(); // pic

  write_str("!> ", -1, COLOUR(BLACK, WHITE));

//  eh_c(0xaa);
}
