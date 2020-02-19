#ifndef _MINM1_H
#define _MINM1_H

#include "des_data.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void minimize1(INT_S*, state_node**);
extern void build_partition(INT_S s1, state_node * t1, INT_S * mapState, INT_S * s2);

#ifdef __cplusplus
}
#endif

#endif /* _MINM1_H */
