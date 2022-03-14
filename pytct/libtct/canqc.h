#ifndef _CANQC1_H
#define _CANQC1_H

#include "des_data.h"

#ifdef __cplusplus
extern "C" {
#endif

extern INT_OS CanQC_proc1(char *, char*, INT_T, INT_T *, INT_T, INT_T *, INT_S*, INT_S **, INT_OS);
extern INT_OS CanQC_proc2(char *, char*, INT_T, INT_T *, INT_T, INT_T *, INT_S*, INT_S **, INT_OS);
extern INT_OS ext_obs_proc(char *, char *,char *, char *,INT_T*, INT_T **,INT_T* , INT_T **, INT_T *, INT_T ** , INT_S *, INT_S **, INT_OS, INT_B, INT_OS);
extern INT_OS supqc_cc_proc(char *, char *, char*, INT_T, INT_T *, INT_T, INT_T *,INT_S *, INT_S **, INT_OS);
extern INT_OS supqc_lcc_proc(char *, char *, INT_T, INT_T *, INT_T, INT_T *,INT_S *, INT_S **, INT_OS);

#ifdef __cplusplus
}
#endif
                   
#endif 
