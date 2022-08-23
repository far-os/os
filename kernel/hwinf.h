#ifndef HWINF_H
#define HWINF_H

struct hwinf {
  unsigned char bios_disk;
} __attribute__((packed));

struct hwinf *hardware = (struct hwinf *) 0xcc00;

#endif
