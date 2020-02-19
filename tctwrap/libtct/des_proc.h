#ifndef _DES_PROC_H
#define _DES_PROC_H

#include "des_data.h"

#ifdef __cplusplus
extern "C" {
#endif

/* For selfloop */
extern void gentran(INT_T, INT_T *, INT_S, state_node *);
extern void selfloop_gentran(INT_T, INT_T *, INT_S, state_node *);

/* For sync */
extern void sync2(INT_S, state_node *, INT_S, state_node *, INT_S *,
                  state_node **, INT_S **, INT_S **);
extern void sync3(INT_S, state_node *, INT_S, state_node *, INT_S *,
                  state_node **, INT_OS, INT_T, INT_T *, INT_S **, INT_S **);
extern void sync4(INT_S, state_node *, INT_S, state_node *, INT_S *,
                  state_node **, INT_S **, INT_S **);
/* For meet */
extern void meet2(INT_S, state_node *, INT_S, state_node *, INT_S *,
                  state_node **, INT_S **, INT_S **);
extern void meet_x64(INT_S, state_node *, INT_S, state_node *, INT_S *,
                     state_node **, unsigned long long **);

/* For nonconflict meet version */
extern void nc_meet2(INT_S, state_node *, INT_S, state_node *, INT_S *,
                     state_node **, INT_S **, INT_S **);

/* For Trim */
extern void trim1(INT_S *, state_node **);

/* For Supcon */
extern void trim2(INT_S *, state_node **, INT_S *);
extern void shave1(INT_S, state_node *, INT_S *, state_node **, INT_S *);

/* For Mutex */
extern void mutex1(INT_S *, state_node **, INT_S, INT_S, INT_S *, state_pair *,
                   INT_S);
extern void reach(INT_S *, state_node **);

/* For Condat */
extern void condat1(state_node *, INT_S, INT_S, INT_S, state_node *, INT_S *,
                    state_node **, INT_S *);

/* For Minimize */
extern void minimize(INT_S *, state_node **);

/* For complement */
extern void complement1(INT_S *, state_node **, INT_T, INT_T *);

// For determinize a generator by subset constuction algorithm
extern INT_B is_deterministic(state_node *, INT_S);
extern void determinize(state_node **, INT_S *);

/* For project0 */
extern void project0(INT_S *, state_node **, INT_T, INT_T *);
extern void project1(INT_S *, state_node **, INT_T, INT_T *);

/* For isomorph */
extern void compare_states(INT_S, INT_S, INT_B *, INT_S *, state_pair **,
                           INT_S *, state_node *, state_node *);

/* For BFS-recode */
extern void b_reach(tran_node *, INT_S, state_node **, INT_S);

extern void coreach2(INT_S, state_node **);

/* For check determinism */
extern INT_B checkdet(INT_S, state_node *);

extern void allevent_des(state_node **, INT_S *, state_node **, INT_S *);

void supclo_des(state_node **, INT_S *, state_node **, INT_S *);

/* For editing */
extern void purgebadstates(INT_S, state_node **);

extern void b_recode(INT_S, state_node **, INT_S *, INT_S **);

extern void recode(INT_S, state_node **, recode_node *);

extern void vocalize_des(state_node **, INT_S *, quad__t **, INT_S *);

extern void iso1(INT_S, INT_S, state_node *, state_node *, INT_B *, INT_S *);

extern INT_S count_tran(state_node *, INT_S);
extern INT_B compute_controllable(state_node *, INT_S);
extern void gentranlist(INT_S, state_node *, INT_T *, INT_T **);
// For force
extern INT_OS force_proc(char *, char *, INT_T, INT_T *, INT_T, INT_T *, INT_T);
// For project without minstate operation
extern void plain_project_proc(INT_S *, state_node **, INT_T, INT_T *);
extern void project_proc_selfloop(INT_S *, state_node **, INT_T, INT_T *);

#ifdef __cplusplus
}
#endif

#endif
