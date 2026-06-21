#pragma once

#define NOT_WAITING 0x00
#define HAS_INTEGER 0x10
#define AFTER_PERCENT -1

// not really using standard library - stdarg just provides platform-dependent defines
#include <stdarg.h>
#include "text.h"

typedef void (*putch_callback)(char ch, unsigned char style);
unsigned int callback_str(putch_callback put, char *str, unsigned char style, unsigned int lim);

void printf(const char *fmt, ...);
void nprintf(unsigned int n, const char *fmt, ...);
void sprintf(char *dest, const char *fmt, ...);
void snprintf(char *dest, unsigned int n, const char *fmt, ...);
void vpfctprintf(putch_callback put, const char *fmt, unsigned char start_style, va_list args);

/* not exporting vfctprintf */
