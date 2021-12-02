#ifndef _EX_SUPRED_H
#define _EX_SUPRED_H

#include "des_data.h"

#ifdef __cplusplus
extern "C" {
#endif

extern INT_OS ex_supreduce(char *,
                     char *,
                     char *,
                     char *,
                     INT_S*,
                     float *,
					 INT_B);
extern INT_OS ex_supreduce1(char *,
					char *,
					char *,
					char *,
					INT_S*,
					float *,
					INT_S *);

extern INT_OS ex_supreduce2(char *,
					char *,
					char *,
					char *,
					INT_S*,
					float *,
					INT_S *);

#ifdef __cplusplus
}
#endif

#endif /* _SUPRED_H */

