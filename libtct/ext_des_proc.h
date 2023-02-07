#ifndef _EXT_DES_PROC_H
#define _EXT_DES_PROC_H

#include "des_data.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void ext_copy_des( INT_S *,state_node **, INT_S ,state_node *);
extern void ext_copy_part( INT_S *,part_node **, INT_S, part_node *);
extern void sr_search(INT_S, state_node **, state_node **, INT_S, INT_S);

extern INT_OS path2block_proc(char*, char*);
extern INT_OS ext_occ_proc(INT_S, state_node *, INT_T, INT_T *, INT_T *, INT_T **);
extern INT_OS rendez_proc(char *, int, char (*)[MAX_FILENAME], INT_T *, INT_T, part_node *, INT_S); 
extern INT_OS splitevent_proc(char *, char *, INT_T , INT_T *, INT_S , part_node *);
extern INT_OS augment_proc(char *, char *, INT_T, INT_T*);
extern INT_OS syncondat_proc1(char *, char*, char*, char *);
extern INT_OS syncondat_proc(char *, char*, char*, char *,  char*);
extern INT_OS attach_proc(char *, char *, char * , INT_S );
extern void project_f_proc(INT_S, state_node*, INT_S *, state_node **, INT_T ,INT_T *);
extern INT_OS path2deadlock_proc(char*, char*);
extern void melt_e_proc(INT_S, state_node*, INT_S *, state_node **, INT_T ,INT_T *);
extern void local_coreach2(INT_S, state_node **);
extern INT_OS statemap_proc(char *, char* , INT_S , state_pair *);
extern INT_OS suprr_proc(char *, char* , char *, INT_S , state_pair *);
extern INT_OS inv_relabel_proc(char *, char* , INT_S , state_map *);

#ifdef __cplusplus
}
#endif
                   
#endif 
