#ifndef _MYMALLOC_H
#define _MYMALLOC_H

/*
 * CTCT version of memory management routines.
 * Ideally, the algorithm should be designed to gracefully fail
 * when it can not get it memory do it's work.
 * However, the current written algorithm does not do that.
 * Design approach.
 *    The CTCT is now broken down into a interface program
 * and a command line program which runs the CTCT procedures (except
 * create, edit, directory, etc... which are not procedures).
 * Second, when running in command line mode, calling these memory
 * management routines will exit the program and write the corresponding
 * file back.
 */

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

void *MALLOC(size_t size);
void *CALLOC(size_t nitems, size_t size);
void *REALLOC(void *block, size_t size);

#ifdef __cplusplus
}
#endif

#endif
