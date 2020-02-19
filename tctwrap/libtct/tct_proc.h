#pragma once

#include "des_data.h"
#include <stdio.h>

extern void print_des_stat_header(FILE *, char *, INT_S, INT_S);
extern INT_B print_marker_states(FILE *, state_node *, INT_S);
extern INT_B print_vocal_output(FILE *, state_node *, INT_S);
extern INT_B print_transitions(FILE *, state_node *, INT_S);
