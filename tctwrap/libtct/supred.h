#ifndef _SUPRED_H
#define _SUPRED_H

#include <stdio.h>
#include "des_data.h"

#ifdef __cplusplus
extern "C" {
#endif

extern INT_OS Get_DES(INT_S *, INT_S *, INT_OS, char*);
extern INT_B Forbidden_Event(char*);
extern void Final_Result();
extern INT_OS Txt_DES(char *, INT_S);
extern void Controller_Tree();
extern INT_B Combined_Tree();
extern void Tree_Structure_Conversion(char*);
extern INT_OS Selfloop_Node(INT_S, INT_S, INT_S, INT_OS);
extern void Reduction();
//extern void Refinement();

extern INT_OS supreduce(char *,
                     char *,
                     char *,
                     char *,
                     INT_S*,
                     float *,
					 INT_B);
extern INT_OS supreduce1(char *,
                     char *,
                     char *,
                     char *,
                     INT_S*,
                     float *,
                     INT_S *);

extern INT_OS supreduce2(char *,
                     char *,
                     char *,
                     char *,
                     INT_S*,
                     float *,
                     INT_S *);
extern INT_OS supreduce3(char *,
                     char *,
                     char *,
                     char *,
                     INT_S*,
                     float *,
                     INT_S *);
extern INT_OS supreduce4(char *,
                     char *,
                     char *,
                     char *,
                     INT_S*,
                     float *,
                     INT_S *);
extern INT_OS supreduce5(char *,
	char *,
	char *,
	char *,
	INT_S*,
	float *);
extern INT_OS supreduce6(char *,
	char *,
	char *,
	char *,
	INT_S*,
	float *);
                     
extern void clean_selfloop(INT_S, state_node *, INT_S, state_node *);


#ifdef __cplusplus
}
#endif

#endif /* _SUPRED_H */

