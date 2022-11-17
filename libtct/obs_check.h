#ifndef _OBS_CHECK_H
#define _OBS_CHECK_H

#include "des_data.h"

#define EXT_DES       ".DES"
#define  CR_OK            0
#define  CR_OUT_OF_MEMORY 3

#ifdef __cplusplus
extern "C" {
#endif

extern void obs_project(INT_S *, state_node **, INT_T , INT_T *, INT_S *, state_map **);

extern INT_OS obs_proc(char*, char*, INT_S *, state_node **, INT_T, INT_T*, INT_T, INT_T*, INT_OS, INT_B *);

extern INT_OS supobs_proc1(char *, char* , char* , INT_T, INT_T *, INT_T, INT_T*);

extern INT_OS supobs_proc3(char *, char* , char* , INT_T, INT_T *, INT_T, INT_T*);

extern INT_OS rel_observ_proc(char *, char* , char* , INT_T, INT_T *, INT_T, INT_T *, INT_T, INT_T*, INT_B*);

extern INT_OS supobs_proc5(char *, char* , char* , char *, INT_T, INT_T *, INT_T, INT_T*);

extern INT_OS transition_suprobs_proc(char *, char* , char* , char *, INT_T, INT_T *, INT_T, INT_T *, INT_T, INT_T*, INT_OS);

extern INT_OS language_suprobs_proc(char *, char* , char* , char *, INT_T, INT_T *, INT_T, INT_T *, INT_T, INT_T*, INT_OS);

extern INT_OS supconrobs_proc(char *, char* , char* , INT_T, INT_T *, INT_T, INT_T*);


#ifdef __cplusplus
}
#endif
                   
#endif 
