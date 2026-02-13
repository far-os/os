#include "include/text.h"
#include "include/ata.h"
#include "include/config.h"
#include "include/util.h"
#include "include/pic.h"
#include "include/ih.h"
#include "include/fs.h"
#include "include/hwinf.h"
#include "include/timer.h"
#include "include/memring.h"
#include "include/err.h"
#include "include/kappldr.h"
#include "include/printf.h"

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

  // clear cursor location cache
  memzero(page_curloc_cache, sizeof page_curloc_cache);
  set_cur(POS(0, 0)); // cursor at top left
   
  quitting_prog = 0;
  
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

  pic_init(); // pic
  
  mem_init(); // init memory

  time(curr_time);
  irq_m_free(0x0); // timer
  init_timer(get_divisor(100));

  irq_m_free(0x1); // keyboard

  asm volatile ("sti"); // set interrupt (opposite of cli)
  // do it after pic init and stuff, to avoid somewhat rare qemu race condition

  query_cpuid();

   ata_identify(
    ata_identity,
    hardware -> boot_disk_p.dev_path[0] & 0x01
  );

  // init fs
  init_locs();
  read_fat();
  read_root();

  read_file(
    "xconfig.qi",
    xconf
  );

  // run shell, TODO: further separate, and make it its own file (will need a decent syscall library tho)
  app_handle shell = instantiate(mk_shell(32), -1, 1);
  (*app_db[shell]->virts->invoke)(app_db[shell]);

  // stop. just stop.
  for (;;) {
    asm volatile ("");
    asm volatile ("hlt");
  }
}
