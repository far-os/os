#pragma once

#define NOT_WAITING 0x00
#define HAS_INTEGER 0x10
#define AFTER_PERCENT -1

void printf(const char *fmt, ...);
void sprintf(char *dest, const char *fmt, ...);

/* not exporting vfctprintf */
