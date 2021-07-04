#ifndef SUPRED2_H
#define SUPRED2_H

#include "des_data.h"

#ifdef __cplusplus
// Reduction_Faster currently doesn't use these, but they may be useful elsewhere.
inline int uf_find(int *array, int idx);
inline int uf_union(int *array, int i, int j);
inline int uf_union(int *array, int i, int j, unsigned char* rank);
#endif

#ifdef __cplusplus
extern "C" {
#endif
	struct equivalent_state_set{
			INT_S state_number;
			struct equivalent_state_set *next_node;
	};

	struct forbidden_event_set{
			INT_T event;
			struct forbidden_event_set *next_event;
	};

	struct transitions{
			INT_T event;
			INT_S target_state_number;
			struct transitions *next_transition;
	};

	struct node{
			struct equivalent_state_set *equal_set;
			INT_B marked_in_plant;
			INT_B marked_in_controller;
			struct forbidden_event_set *forb_set;
			struct transitions *tran_set;
	};


	void Reduction_Faster(struct node *root_node, INT_S num_states, INT_S tran_number);


#ifdef __cplusplus
}
#endif

#endif
