#ifndef _HIGEN_H
#define _HIGEN_H

#include "des_data.h"

#ifdef __cplusplus
extern "C" {
#endif

/* For Eventmap */
extern void eventmap_des(state_node*, INT_S,
                         state_node**, INT_S*,
                         state_pair*, INT_S,
                         INT_T**, INT_T*, INT_B *);

extern void higen_des(state_node**, INT_S*, INT_T**, INT_T*);

#ifdef __cplusplus
}
#endif

#endif
