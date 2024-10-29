#include "include/misc.h"
#include "include/config.h"

struct far_ver * curr_ver = &((struct far_ver) {
  .major = 0,
  .minor = 0,
  .patch = 3,
  .build = 101, // minor changes + hotfixes
});

struct farb_header __seg_fs *prog_head = 0;

struct config_qi *disk_config = (struct config_qi *) 0xc800;
