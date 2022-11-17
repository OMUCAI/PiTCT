#ifndef _LOCALIZE_H
#define _LOCALIZE_H

#include "des_data.h"

#ifdef __cplusplus
extern "C" {
#endif

extern INT_OS localize_proc(INT_S, INT_S, char *, char *, char (*)[MAX_FILENAME], char (*)[MAX_FILENAME],INT_OS, INT_OS);

extern INT_OS exlocalize_proc1(char* , char *, char *, INT_T, INT_T *, INT_B);

extern INT_OS exlocalize_proc2(char* , char *, char *, INT_T, INT_T *, INT_T, INT_T *);

extern INT_OS exlocalize_proc_filltab(char* , char *, char *, INT_T, INT_T *);

extern INT_OS exlocalize_proc_new(char* , char *, char *, INT_T, INT_T *, INT_B);

extern INT_OS exlocalize_proc_nlogn(char* , char *, char *, INT_T, INT_T *);

extern void check_control_equ(INT_OS, char *, char *, char (*)[MAX_FILENAME], char (*)[MAX_FILENAME],INT_B *);

#ifdef __cplusplus
}
#endif
                   
#endif 
