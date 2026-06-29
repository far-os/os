#include "include/misc.h"
#include "include/config.h"

struct far_ver * curr_ver = &((struct far_ver) {
  .major =    0,
  .minor =    0,
  .patch =    6,
  .build = 0x04, // minor changes + hotfixes
});

struct farb_header __seg_fs *prog_head = 0;

struct xconfig_qi *xconf = (struct xconfig_qi *) 0xc800;

#ifdef KERN_SIZE
// where the kernel starts
void *kernel = (void *) (0x80000 - (unsigned int) (KERN_SIZE << 9));
#else
#error "KERN_SIZE not found, Please compile with Makefile"
#endif
