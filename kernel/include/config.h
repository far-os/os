#pragma once

#define CONFIG_MAGIC 0xc091fa2b
// read config file

struct config_qi {
  unsigned int qi_magic; // should be 0xc091fa2b
  struct sector_box exec;
  struct sector_box wdata;
} __attribute__((packed));

struct config_qi *disk_config = (struct config_qi *) 0xc800;
