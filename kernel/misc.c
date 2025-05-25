#include "include/misc.h"
#include "include/config.h"

struct far_ver * curr_ver = &((struct far_ver) {
  .major = 0x00,
  .minor = 0x00,
  .patch = 0x04,
  .build = 0x80, // minor changes + hotfixes
});

struct farb_header __seg_fs *prog_head = 0;

struct config_qi *disk_config = (struct config_qi *) 0xc800;

// where the kernel starts
void *kernel = 0x80000 - (KERN_LEN << 9);
