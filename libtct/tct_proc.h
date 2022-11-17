#pragma once

#include "des_data.h"
#include <stdio.h>

extern void print_des_stat_header(FILE *, char *, INT_S, INT_S);
extern INT_B print_marker_states(FILE *, state_node *, INT_S);
extern INT_B print_vocal_output(FILE *, state_node *, INT_S);
extern INT_B print_transitions(FILE *, state_node *, INT_S);
extern void print_dat_header_stat(FILE *, char *, INT_B);
extern INT_B print_dat(FILE *, state_node *, INT_S);

//used in executing script file
extern INT_B nonconflict(INT_S, state_node *);
// used in supconrobs
extern void gen_complement_list(state_node *, INT_S, INT_T *, INT_T , INT_T **, INT_T *);
