A list of all (predetermined) locations of objects in memory.

```c
struct keystates *keys = &((struct keystates) { .states_high = 0x0, .states_low = 0x0, .modifs = 0b00000000 });

struct fatfs_superblock *bpb = (struct csdfs_superblock *) 0x7c00; // until 0x7e00
struct timestamp *curr_time = (struct timestamp *) 0xc7f0;
struct xconf_qi *xconf = (struct config_qi *) 0xc800;
struct hwinf *hardware = (struct hwinf *) 0xcc00;
unsigned short ata_identity[256] = 0xd000; // until 0xd200

unsigned char *memring = (unsigned char *) 0x12_000; // until 0x13_000

struct inode file_table[] = (struct inode *) 0x20_000; // until 0x20_800

code *kernel = 0x80000 - (KERN_LEN << 9);

char *vram = (char *) 0xb8000;

void *mem = (void *) 0x120_000; // until 0x130_000
```
