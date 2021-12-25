#ifndef _CNORM_H
#define _CNORM_H

#include "des_data.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void suprema_normal(state_node*, INT_S,
                           state_node*, INT_S,
                           state_node**,INT_S*,
                           INT_T*, INT_T);
extern void suprema_normal1(state_node*, INT_S,
							state_node*, INT_S,
							state_node**,INT_S*,
							INT_T*, INT_T);
                           
extern void suprema_normal_clo(state_node*, INT_S,
                               state_node*, INT_S,
                               state_node**,INT_S*,
                               INT_T*, INT_T);                           

extern void suprema_normal_scop(state_node*, INT_S,
                                state_node*, INT_S,
                                state_node**,INT_S*,
                                INT_T*, INT_T);  

extern void suprema_normal_clo1(state_node*, INT_S,
                               state_node*, INT_S,
                               state_node**,INT_S*,
                               INT_T*, INT_T);                           

extern void suprema_normal_scop1(state_node*, INT_S,
                                state_node*, INT_S,
                                state_node**,INT_S*,
                                INT_T*, INT_T);  
                                
extern void get_event_list(INT_S, state_node *, 
                           INT_T **, INT_T *);
                           
extern void get_event_diff(INT_T **, INT_T *,
                           INT_T *, INT_T,
                           INT_T *, INT_T);

#ifdef __cplusplus
}
#endif

#endif
