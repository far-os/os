#include "include/misc.h"
#include "include/config.h"

struct far_ver * curr_ver = &((struct far_ver) {
  .major =    0,
  .minor =    0,
  .patch =    5,
  .build = 0x01, // minor changes + hotfixes
});

struct farb_header __seg_fs *prog_head = 0;

struct xconfig_qi *xconf = (struct config_qi *) 0xc800;

// where the kernel starts
void *kernel = 0x80000 - (KERN_LEN << 9);
