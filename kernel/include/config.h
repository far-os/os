#pragma once

#include "fs.h"

#define CONFIG_MAGIC 0xc091fa2b
// read config file

/*
struct config_qi {
  unsigned int qi_magic; // should be 0xc091fa2b
  struct sector_box exec;
  struct sector_box wdata;
} __attribute__((packed));
*/

// this is in misc.c, because like i dont feel like creating a one-line c file
// future me, this was foreshadowing
extern struct config_qi *disk_config;
