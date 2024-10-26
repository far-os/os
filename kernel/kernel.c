#include "include/text.h"
#include "include/ata.h"
#include "include/config.h"
#include "include/util.h"
#include "include/pic.h"
#include "include/ih.h"
// hold on
// #include "kapps/kshell.h"
#include "include/fs.h"
#include "include/hwinf.h"
#include "include/timer.h"
#include "include/memring.h"
#include "include/err.h"

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

  char * vbf = malloc(32);
  strcpy("Welcome to ", vbf);
  to_ver_string(curr_ver, vbf + strlen(vbf));
  vbf[strlen(vbf)] = '!';
  write_str(vbf, COLOUR(MAGENTA, B_GREEN)); // welcome message
  line_feed();
  free(vbf);
  
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

  // init fs and inode table
  fs_init();

  read_inode(
    name2inode("config.qi"),
    disk_config
  );
 
  // init fs (again) - for new files found in config.qi
  fs_init();

  // magic check
  if (disk_config -> qi_magic != CONFIG_MAGIC) {
    msg(INFO, NONE, hardware -> boot_disk_p.itrf_type);
    line_feed();
    msg(KERNERR, E_CONFIG, "Bad kernel config: invalid magic"); 
    line_feed();
  }

  shell();

//  eh_c(0xaa);
}
