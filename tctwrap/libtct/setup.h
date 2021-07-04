#ifndef _SETUP_H
#define _SETUP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "des_data.h"

#define TCTNAME "libtct"
#define DES_FILE_VERSION "2.0"
#define _MAX_PATH 260

// Check x64 or x32
// C
#include <stdint.h>

#if INTPTR_MAX == INT64_MAX
// 64-bit
  #define _x64_
#elif INTPTR_MAX == INT32_MAX
// 32-bit
#else
#error Unknown pointer size or missing size macros!
#endif

extern INT_OS autotest;
extern INT_OS ring_active;
extern INT_OS debug_mode;
extern INT_OS timing_mode;
extern INT_OS cmdline;
extern INT_OS minflag;
extern char path[256];
extern char prefix[256];
extern char ctct_ini_file[256];
extern char info_file[256];
extern char *argv0;

typedef unsigned char boolean;

#ifdef __cplusplus
}
#endif

#endif
