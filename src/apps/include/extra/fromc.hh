#pragma once

// this file is just a wrapper for all the c stuff.
/*
  TODO: separate even further by using int 0x33
*/

#define k_app KApp
// all the function definitions, we just wrap them in `extern "C"' to stop the dreaded name mangling
extern "C" {
#include "../../../kernel/include/ata.h"
#include "../../../kernel/include/cmos.h"
#include "../../../kernel/include/config.h"
#include "../../../kernel/include/err.h"
#include "../../../kernel/include/fs.h"
#include "../../../kernel/include/hwinf.h"
#include "../../../kernel/include/kappldr.h"
#include "../../../kernel/include/kbd.h"
#include "../../../kernel/include/memring.h"
#include "../../../kernel/include/misc.h"
#include "../../../kernel/include/printf.h"
#include "../../../kernel/include/text.h"
#include "../../../kernel/include/util.h"
}

// execute prog.bin. omnce again, TODO, there ought to be no difference between anything of these apps and prog.bin. what we basically need is universal binary format
extern "C" int prog(int arg);

// stops `new' and `delete' operators from shitting themselves
inline void* operator new(unsigned int, void* p) { return p; }
inline void* operator new[](unsigned int, void* p) { return p; }
inline void* operator new(unsigned int size) { return malloc(size); }

// hmmm
inline void operator delete(void* p) { free(p); }
inline void operator delete(void* p, unsigned int) { free(p); }
