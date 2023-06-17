#include "text.h"
#include "ata.h"
#include "config.h"
#include "defs.h"
#include "util.h"
#include "pic.h"
#include "ih.h"
#include "shell.h"
#include "fs.h"
#include "hwinf.h"
#include "timer.h"
#include "memring.h"
#include "err.h"

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
//  clear_scr();

  set_cur(POS(0, 0)); // cursor at top left
  write_str("Welcome to the Kernel!\n", COLOUR(CYAN, B_YELLOW)); // welcome message
//  cp437(); // codepage 437: for testing purposes

  __attribute__((aligned(0x10)))
  static struct idt_entry idt[256]; // the entire idt
  static struct idtr_contents idtr; // idtr

  idtr.offset = (unsigned int) &idt;
  idtr.size   = (unsigned short) sizeof(struct idt_entry) * 256 - 1;

  for (unsigned char d = 0; d < 64; ++d) {
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
  
  mem_init(); // init memory

  irq_m_free(0x0); // timer
  init_timer(get_divisor(1000));

  irq_m_free(0x1); // keyboard

  query_cpuid();

  read_pio28(disk_config, KERN_LEN, 1, hardware -> boot_disk_p.dev_path[0] & 0x01); // reads disk for config, has to get master or slave
 
  // magic check
  if (disk_config -> qi_magic != CONFIG_MAGIC) {
    msg(INFO, 0, hardware -> boot_disk_p.itrf_type);
    line_feed();
    msg(KERNERR, 18, "Bad kernel config: invalid magic"); 
    line_feed();
  }

  shell();

//  eh_c(0xaa);
}
