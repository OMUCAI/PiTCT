#ifndef _MINM_H
#define _MINM_H

#include "des_data.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void minimize2(INT_S*, state_node**);

extern void loc_refinement_disable(INT_S*, state_node**, INT_S, state_node*);
extern void loc_refinement(INT_S*, state_node**, INT_S, part_node*);

#ifdef __cplusplus
}
#endif

#endif
