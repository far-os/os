#include "misc.h"

#pragma once

extern inline void idle();

int strlen(char *str);
void memcpy(void *src, void *dest, unsigned int amount);
void backmemcpy(void *srcend, void *destend, unsigned int amount);
void strcpy(char *src, char *dest);
unsigned char memcmp(void *src, void *dest, unsigned int amount);

extern inline unsigned char bittest(void *src, unsigned int bit);
extern inline unsigned char bitinv(void *src, unsigned int bit);
extern inline unsigned char bitclear(void *src, unsigned int bit);
extern inline unsigned char bitset(void *src, unsigned int bit);

unsigned char strcmp(char *src, char *dest);
extern inline void memset(void *dest, unsigned int amount, unsigned char val);
extern inline void memzero(void *dest, unsigned int amount);
void memrev(char *src, int len, char *dest);

char nybble_to_hex(int num);
void to_hex(void *data, unsigned char i_len, char *out);
void to_filled_dec(int input, char *out, unsigned char size, char fill);
void to_dec(int input, char *out);
unsigned int to_uint(char *input);

extern inline char *strcat(char *out, char *in);

#define NOT_WAITING 0x00
#define HAS_INTEGER 0x10
#define AFTER_PERCENT -1
void sprintf(char *dest, const char *fmt, ...);

void to_ver_string(struct far_ver * ver, char * vbuf);

#define trace_ch_until(a,b) trace_ch_until_with(a,b,0)
unsigned short trace_ch_until_with(char *str, int until, int start);
