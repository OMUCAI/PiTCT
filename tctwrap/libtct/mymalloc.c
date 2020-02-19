#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"
#include "setup.h"
#include "des_data.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CR_OUT_OF_MEMORY 3

extern INT_OS mem_result;

void *MALLOC(size_t size)
{
    void *ptr;

    ptr = malloc(size);
    if (cmdline) {
        if ((ptr == NULL) && (size > 0)) {
            /* Going to exit the program */
            exit(0);
        } else {
            return ptr;
        }
    } else {
       if ((ptr == NULL) && (size > 0))
          mem_result = 1;
       return ptr;
    }
}

void *CALLOC(size_t nitems, size_t size)
{
    void *ptr;

    ptr = calloc(nitems, size);
    if (cmdline) {
       if ((ptr == NULL) && (size > 0)) {
           /* Going to exit the program */
           exit(0);
       } else {
           return ptr;
       }
    } else {
       if ((ptr == NULL) && (size > 0))
          mem_result = 1;
       return ptr;
    }
}

void *REALLOC(void *block, size_t size)
{
    void *ptr;

    ptr = realloc(block, size);
    if (cmdline) {
       if ((ptr == NULL) && (size > 0)) {
           /* Going to exit the program */
           exit(0);
       } else {
           return ptr;
       }
    } else {
       if ((ptr == NULL) && (size > 0))
           mem_result = 1;
       return ptr;
    }
}

#ifdef __cplusplus
}
#endif
