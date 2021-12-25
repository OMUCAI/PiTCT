#ifndef _TCT_IO_H
#define _TCT_IO_H

#include "des_data.h"
#include <stdio.h>

#define MAXSCREENY 22

#define MAX_DES_NAME_LEN 20

#define CBack 8
#define CEsc 27
#define CEnter 13
#define CPgUp 117
#define CPgDn 100

#ifdef __cplusplus
extern "C" {
#endif

extern FILE *auto_in;

extern void make_filename_ext(char *, char *, char *);
extern INT_OS exist(char*);

#ifdef __cplusplus
}
#endif

#endif
