// #include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "des_data.h"
#include "supred2.h"
#include "supred.h"


// External stuff.
extern "C" {
	extern INT_S *simpler_controller;
}

// Types used internally.
struct r2_node {
	transitions *trans;
	forbidden_event_set *forb;
	int equal_set;		// Used for union find
	unsigned char marking;		// 00 -> No elements; 10 -> exist and not marked, 11 -> exists and marked. Conflict if [1:0] + [3:2] == 5
};

struct MergeEntry {
	int i;
	int j;
};


// Union-find on r2_node is only used within this file due to the struct r2_node.
// This algorithm cannot use union-by-rank, as it relies on the lower-index node to always be the set representative.
static inline int uf_find(struct r2_node *array, int idx);
static inline int uf_union(struct r2_node *array, int i, int j);

static int Check_Merge(r2_node *nodes, int num_states, MergeEntry *merge_list, int nodei, int nodej);
static bool check_R(struct r2_node *node1, struct r2_node *node2);

//#define DBGLOG "supred2.log"		// Define the log file to enable debugging

#ifdef DBGLOG
	#include <Windows.h>	// For QueryPerformanceCounter
//	#define DBG_DUMPGRAPH		// Also print out the initial and final graphs.
	FILE *lf = NULL;		// Debug log file
	LARGE_INTEGER start_time, counter_freq;
	#define dbg_printf(FMT,...) do { if (lf) { LARGE_INTEGER dpf_stopt; QueryPerformanceCounter(&dpf_stopt); fprintf(lf,  "%llu: " FMT, ((dpf_stopt.QuadPart-start_time.QuadPart)*1000000LL)/counter_freq.QuadPart, __VA_ARGS__); }}while(0)
	#define dbg_printf2(FMT,...) do { if (lf) { fprintf(lf, FMT, __VA_ARGS__); fflush(lf); }}while(0)
#else
	#define dbg_printf(...)
	#define dbg_printf2(...)
#endif



inline int uf_find(int *array, int idx) {
	int i;
	for (i = idx; array[i] != -1; i = array[i]) ;	// Find representative
	for ( ; idx != i; idx = array[idx])				// Path compression
		array[idx] = i;
	return i;
}

// Returns the new representatitve of the union.
inline int uf_union(int *array, int i, int j) {		
	i = uf_find(array, i);
	j = uf_find(array, j);

	// Our particular unioning pattern seems to always produce the shallowest trees
	// just by tacking tree j onto node i
	array[j] = i;
	return i;
}

// Union by rank, if a rank array is used. A rank array holds the rank (depth)
// of the tree rooted at index i (or j). It should be initialized to zero.
inline int uf_union(int *array, int i, int j, unsigned char* rank) {		
	i = uf_find(array, i);
	j = uf_find(array, j);

	if (rank[i] <= rank[j]) {
		array[j] = i;
		if (rank[i] == rank[j])	// Won't overflow until you have 2^255 nodes in the set
			rank[j]++;
		return i;
	} else {
		array[i] = j;
		return j;
	} 
}

inline int uf_find(struct r2_node *array, int idx) {
	int i;
	for (i = idx; array[i].equal_set != -1; i = array[i].equal_set) ;	// Find representative
	for ( ; idx != i; idx = array[idx].equal_set)				// Path compression
		array[idx].equal_set = i;
	return i;
}

// This one must guarantee the new representative will be min(i,j). Needed for correctness in Reduction()'s set merging.
inline int uf_union(struct r2_node *array, int i, int j) {
	i = uf_find(array, i);
	j = uf_find(array, j);

	if (i<j) {
		array[j].equal_set = i;
		return i;
	} else {
		array[i].equal_set = j;
		return j;
	}
}







// Accessors for precomputed R array.
#define RIDX(I,J) ((I)*num_states+(J))
#define RGET(R,I,J) ((R)[ RIDX(I,J) ])

// This module is used to reduce the transition structure of optimal controller.
// It should theoretically do the same thing as Reduction() and Ex1_Reduction(), but faster.
// Parameters:
//    root_node -- The graph
//    num_states -- number of nodes in the graph
//    tran_number -- Total number of edges in the graph
// This function depends on global variables:
//	  simpler_controller -- Final_Output() expects the output to be here, apparently.
// -- Henry Wong <henry@stuffedcow.net>, 2016

void Reduction_Faster(struct node* root_node, INT_S num_states, INT_S tran_number)
{

	// Allocate memory and a duplicate copy to allow rolling back because Check_Merge modifies the structures speculatively.
	r2_node *nodes = (r2_node*)malloc(num_states*sizeof(r2_node));
	r2_node *nodes_checkpoint = (r2_node*)malloc(num_states*sizeof(r2_node));

	// Allocate a slab of memory for transitions, then free it later, so we don't have to worry about cleanup while processing.
	transitions *trans_mem = (transitions*)malloc(tran_number * sizeof(transitions));
	transitions *trans_mem_checkpoint = (transitions*)malloc(tran_number * sizeof(transitions));

	// Yep, we need to count the total number of forbidden events to allocate memory for them.
	int forb_number = 0;
	for (int i=0;i<num_states;i++) {
		for (forbidden_event_set *f = root_node[i].forb_set; f; f = f->next_event) {
			forb_number++;
		}
	}

	forbidden_event_set *forb_mem = (forbidden_event_set*)malloc(forb_number * sizeof(forbidden_event_set));
	forbidden_event_set *forb_mem_checkpoint = (forbidden_event_set*)malloc(forb_number * sizeof(forbidden_event_set));

#ifdef DBGLOG
	LARGE_INTEGER start_time2;
	if (!lf) {
		lf = fopen (DBGLOG, "wt");
		QueryPerformanceCounter(&start_time);
		QueryPerformanceFrequency(&counter_freq);
	}
	QueryPerformanceCounter(&start_time2);		

	dbg_printf("Starting Reduction_Faster(). num_states is %d, tran_number is %d, forb_number is %d\n", num_states, tran_number, forb_number);
	
#ifdef DBG_DUMPGRAPH
	dbg_printf ("Initial state of graph:\n");
	for (int i=0; i<num_states; i++) {		
		dbg_printf2("  Node %d marked(%d, %d) eq_set:", i, root_node[i].marked_in_controller, root_node[i].marked_in_plant, i);
		for (equivalent_state_set *temp6 = root_node[i].equal_set; temp6; temp6 = temp6->next_node) {
			dbg_printf2(" %d", temp6->state_number);
		}

		dbg_printf2("\n      forbidden events:");
		for (forbidden_event_set *temp1 = root_node[i].forb_set; temp1 ; temp1 = temp1->next_event) {
			dbg_printf2(" %d", temp1->event);
		}
		
		dbg_printf2("\n      transitions:");
		for (transitions *temp2 = root_node[i].tran_set; temp2; temp2 = temp2->next_transition) {
			dbg_printf2(" (%d,%d)", temp2->event, temp2->target_state_number);
		}
		dbg_printf2("\n");
	}
#endif
#endif

	// Initialize nodes and transitions:
	//   nodes[i].equal_set points to -1 (No nodes merged)
	//   transitions is a linked list sorted by event
	//	 forb is a linked list sorted by event
	for (int i=0, n=0, nf=0 ;i<num_states;i++) {
		nodes[i].equal_set = -1;
		nodes[i].trans = NULL;		
		nodes[i].forb = NULL;

		transitions *t_end = NULL;
		for (transitions * t = root_node[i].tran_set; t ; t = t->next_transition)
		{
			transitions *p = &trans_mem[n++];
			p->event = t->event;
			p->target_state_number = t->target_state_number;

			if (nodes[i].trans) {	// Search for correct location to append
				// I'm lazy. Here's an insertion sort. Sort by event number. But I think input data usually
				// comes pre-sorted (?), so insert at end of list if event number always increases.
				// Thus, this sort performs poorly if the input data starts with a single big number, then the rest sorted in ascending order.
				transitions *pt;
				if (t_end->event > t->event) {
					pt = t_end;
					t_end = p;
				}
				else {
					for (pt = nodes[i].trans; pt->next_transition && pt->next_transition->event < t->event; pt = pt->next_transition);
				}
				p->next_transition = pt->next_transition;
				pt->next_transition = p;				
			}
			else	// This is the first element of the linked list.
			{
				p->next_transition = NULL;
				nodes[i].trans = p;
				t_end = p;
			}
		}

		forbidden_event_set *f_end = NULL;
		for (forbidden_event_set *f = root_node[i].forb_set; f; f = f->next_event) {
			forbidden_event_set *p = &forb_mem[nf++];
			p->event = f->event;

			if (nodes[i].forb) {
				forbidden_event_set *pt;
				if (f_end->event > f->event) {
					pt = f_end;
					f_end = p;
				} else {
					for (pt=nodes[i].forb; pt->next_event && pt->next_event->event < f->event; pt = pt->next_event);
				}
				p->next_event = pt->next_event;
				pt->next_event = p;
			}
			else {
				p->next_event = NULL;
				nodes[i].forb = p;
				f_end = p;
			}
		}

		nodes[i].marking = (0x2 | (unsigned char)(!!root_node[i].marked_in_controller)) << (2 * !!root_node[i].marked_in_plant);

	}

	// Because we merge sets as we see them, there can never be more than num_states-1 merges.
	MergeEntry *merge_list = (MergeEntry*)malloc(num_states * sizeof(MergeEntry));

	// Initial checkpoint: I'm just going to memcpy these because I don't see an easy way to chase down
	// and revert only those entries that changed.
	memcpy( trans_mem_checkpoint, trans_mem, tran_number * sizeof(transitions) );	
	memcpy( nodes_checkpoint, nodes, num_states * sizeof(r2_node) );
	memcpy( forb_mem_checkpoint, forb_mem, forb_number * sizeof(forbidden_event_set) );

	// For every node pair, try to merge them.
	// Note: Each merge reduces the cluster count by 1, so the sum of all merges performed + number of states
	// in the reduced output must equal num_states, the number of states of the original input.
	// Complexity: n = num_states, s = event alphabet size (typically << n, but never greater than n^2)
	// Outer loop is n^2
	//   Each iteration performs three memcpys (Two O(s) and one O(n))
	//   Each call to Check_Merge is O( n * s )
	// Total: O(n^3 * s)
	for(int i=0; i<num_states; i++) {
		if(nodes[i].equal_set != -1) continue;		// If a node has been moved, it is not the representative of the set.
		for(int j=i+1; j<num_states; j++) {
			if(nodes[j].equal_set != -1) continue;

			// For every pair of set representatives, merge:
			dbg_printf("    Calling Check_Merge(%d, %d)\n", i, j);
			int num_merged = Check_Merge(nodes, (int)num_states, merge_list, i, j);
			dbg_printf("        Returned %d\n", num_merged);

			if (num_merged) {
				// for (int i=0;i<num_merged;i++) {
				// 	dbg_printf("        Merging clusters %d and %d\n", merge_list[i].i, merge_list[i].j);
				// }

				// Make a new checkpoint. Check_Merge already did all the work of merging, so just keep the results.
				memcpy( trans_mem_checkpoint, trans_mem, tran_number * sizeof(transitions) );	// Update checkpoint				
				memcpy( nodes_checkpoint, nodes, num_states * sizeof(r2_node) );
				memcpy( forb_mem_checkpoint, forb_mem, forb_number * sizeof(forbidden_event_set) );
			}
			else {
				// Oops, the merge failed. Undo the mess that Check_Merge made.
				memcpy( trans_mem, trans_mem_checkpoint, tran_number * sizeof(transitions) );	// Restore checkpoint				
				memcpy( nodes, nodes_checkpoint, num_states * sizeof(r2_node) );
				memcpy( forb_mem, forb_mem_checkpoint, forb_number * sizeof(forbidden_event_set) );
			}
		}
	}


	// Copy out final graph to root_node and free our internal data structures.
	for (int i=0;i<num_states; i++) {
		root_node[i].equal_set->state_number = uf_find(nodes, i);
	}
	free (nodes);
	free (nodes_checkpoint);
	free (trans_mem);
	free (trans_mem_checkpoint);
	free (forb_mem);
	free (forb_mem_checkpoint);

	// We're done the reduction now. Do the usual transformation of data structures (Final_Result()).

#ifdef DBG_DUMPGRAPH
	dbg_printf ("Final state of graph:\n");
	for (int i=0; i<num_states; i++) {		
		dbg_printf2("  Node %d marked(%d, %d) eq_set:", i, root_node[i].marked_in_controller, root_node[i].marked_in_plant, i);
		equivalent_state_set *temp6 = root_node[i].equal_set;
		while (temp6) {
			dbg_printf2(" %d", temp6->state_number);
			temp6 = temp6->next_node;
		}
		dbg_printf2("\n      forbidden events:");
		forbidden_event_set *temp1 = root_node[i].forb_set;
		while (temp1) {
			dbg_printf2(" %d", temp1->event);
			temp1 = temp1->next_event;
		}
		dbg_printf2("\n      transitions:");
		transitions *temp2 = root_node[i].tran_set;
		while (temp2) {
			dbg_printf2(" (%d,%d)", temp2->event, temp2->target_state_number);
			temp2 = temp2->next_transition;
		}
		dbg_printf2("\n");
  }
#endif

  simpler_controller = (INT_S *) calloc((3*tran_number+1), sizeof(INT_S));
  if(simpler_controller == NULL){
     mem_result = 1;
	 goto CLEANUP;
  }
  Final_Result();

CLEANUP:
#ifdef DBGLOG
	LARGE_INTEGER stop_time;
	QueryPerformanceCounter(&stop_time);
	dbg_printf ("Done Reduction_Faster() in %llu us.\n\n", 
	  ((stop_time.QuadPart-start_time2.QuadPart)*1000000LL)/counter_freq.QuadPart);
#endif
  return;
}


int Check_Merge(r2_node *nodes, int num_states, MergeEntry *merge_list, int nodei, int nodej)
{
	
	// Initialization: Merge the first two nodes, but not the transitions list yet
	int n_merge = 1;
	int c = uf_union(nodes, nodei, nodej);
	if (c == nodei) {
		merge_list[0].i = nodei;
		merge_list[0].j = nodej;
	} else {
		merge_list[0].i = nodej;
		merge_list[0].j = nodei;
	}

	int retval = 0;		// Default return value of 0.
	
	for (int curnode = 0; curnode < n_merge; curnode++) 
	{
		const int node1 = merge_list[curnode].i;
		const int node2 = merge_list[curnode].j;

		// node1 and node2 were set representatives of the now-merged cluster node1. At this point in the code,
		// the nodes belonging to node1 and node2 have already been merged, but not the transitions set, the forbidden set, 
		// nor marking info. Thus, do not attempt to run uf_find(nodes, node1/node2), because the result will be incorrect.
		// Check whether the node pair can be merged: This uses the forb and tran sets before merging to do the check.
		if (!check_R(&nodes[node1], &nodes[node2])) {
			goto CLEANUP;
		}

		// Now that R has been checked, we can merge the forb and tran sets.
		// The transition and forb lists are sorted by event, and each event can appear at most once. Duplicate events are dropped.
		// Merging two sorted linked lists has runtime linear in the sum of the two list lengths.

		// Merge the forb lists and marking info
		{
			forbidden_event_set *f1 = nodes[node1].forb;
			forbidden_event_set *f2 = nodes[node2].forb;
			forbidden_event_set *ftail = NULL;
			while (f1 || f2)
			{
				forbidden_event_set *f;
				if (!f2 || (f1 && f1->event < f2->event)) {
					f = f1;
					f1 = f1->next_event;
				}
				else if (!f1 || (f2 && f1->event > f2->event)) {
					f = f2;
					f2 = f2->next_event;
				}
				else	// Events are the same, so this must be a duplicate. Keep one/drop one.
				{
					f = f1;
					f1 = f1->next_event;
					f2 = f2->next_event;
				}

				if (ftail == NULL) {
					nodes[node1].forb = f;		// node1 is always the new merged set representative
				} else {
					ftail->next_event = f;
				}
				ftail = f;
			}
			nodes[node1].marking |= nodes[node2].marking;		// See Check_R for an explanation.
		}	// Done merging forb list and marking info


		// Traverse transition lists, looking for shared events, and build a new merged list at the same time
		// to save one traversal of the transitions list. For each pair of shared events,
		// merge their targets if they belong to different clusters.
		transitions *t1 = nodes[node1].trans;
		transitions *t2 = nodes[node2].trans;
		transitions *ptail = NULL;
		while (t1 || t2)
		{
			transitions *t;		// Append this transition
			if (!t2 || (t1 && t1->event < t2->event)) {
				t = t1;
				t1 = t1->next_transition;
			}
			else if (!t1 || (t2 && t1->event > t2->event)) {
				t = t2;
				t2 = t2->next_transition;
			}
			else	
			{
				// Found a shared event: Merge their targets and keep one of the events in the new list.
				int cluster1 = uf_find(nodes, (int)(t1->target_state_number));
				int cluster2 = uf_find(nodes, (int)(t2->target_state_number));
				//dbg_printf("            %d,%d; %d: Trying to merge %d(%d) and %d(%d)\n", node1, node2, t1->event, t1->target_state_number, cluster1, t2->target_state_number, cluster2);

				t = t1;		// Keep t1, drop t2
				t1 = t1->next_transition;
				t2 = t2->next_transition;

				if (cluster1 != cluster2) {
					// If the targets belong to different clusters, merge them.
					// Merge the nodes now, so future node pairs know whether they're redundantly trying to merge already-merged clusters.
					// But don't check R here, because we need the transitions lists to be correct before we can check R.
					// Thus, R checking is deferred until the node is dequeued off the merge_list.
					
					// Performance optimization: Fail if we try to merge clusters we ought to have already
					// merged in a previous iteration, which implies they were impossible to merge.
					if (cluster1 < nodei || cluster2 < nodei) {
						goto CLEANUP;		// retval is zero
					}

					// Union the two sets of nodes, and enqueue this pair for further processing.
					// Caution: merge_list.i always contains the new set representative. i and j are not interchangeable.
					// This requirement can be relaxed if merging forbidden and transition lists generates lists for both nodes 
					// (pointing to the same list) so it's still correct regardless of which node became the representative.
					int c = uf_union(nodes, cluster1, cluster2);
					if (c == cluster1) {
						merge_list[n_merge].i = cluster1;
						merge_list[n_merge].j = cluster2;
					} else {
						merge_list[n_merge].i = cluster2;
						merge_list[n_merge].j = cluster1;
					}
					n_merge++;
				}
			}

			t->target_state_number = uf_find(nodes, (int)(t->target_state_number));	// Optional

			if (ptail == NULL) {
				nodes[node1].trans = t;		// node1 is always the new merged set representative
			} else {
				ptail->next_transition = t;
			}
			ptail = t;
		}	// Done merging transitions list and merging nodes (just the nodes) belonging to clusters that these transitions required to be merged.
	}	

	retval = n_merge;		// Successfully merged this many nodes.
CLEANUP:
	dbg_printf("        Clusters speculatively merged: %d\n", n_merge);
	return retval;
}

bool check_R(struct r2_node *node1, struct r2_node *node2)
{
	// Warning: node1 and node2 must be set representatives. I'm not running uf_find on them here.
	// Runtime is linear in length of tran + forb lists.
	/*
		Check that node1->marked_in_plant == node2->marked_in_plant implies node1->marked_in_controller == node2->marked_in_controller.
		This comparison is non-obvious:
			node1->marking is a 4-bit number, with two groups of two bits indicating whether any nodes exist 
			in the cluster with marked_in_plant==true bit[3] and false bit[1], and if so, whether those nodes are 
			marked_in_controller (bit[2] and bit[0]). 
			There are three encodings for each of the two cases:
			00 - This cluster does not contain any nodes with marked_in_plant == (false [1:0] or true [3:2])
			01 - Does not occur
			10 - Node(s) exist, and they are not marked_in_controller
			11 - Node(s) exist, and they are marked_in_controller.

			The case that conflicts is if nodes exist in both sets, but they have a different marked_in_controller value:
			10 + 11, or 11 + 10. These are precisely the cases that sum to 5.

			Merging two non-conflicting sets is a bitwise OR of the two marking numbers:
			  00 00 => 00
			  00 10 => 10
			  00 11 => 11
			  10 00 => 10
			  10 10 => 10
			  11 00 => 11
			  11 11 => 11
			The other 2 cases are conflicting.
	*/
	if ((node1->marking&0x3) + (node2->marking&0x3) == 0x5) return false;
	if ((node1->marking&0xc) + (node2->marking&0xc) == 0x14) return false;

	// Traverse the forb list of one node and tran list of the other. Any shared events indicates a conflict.
	for (int i=0;i<2;i++) {
		forbidden_event_set *f;
		transitions *t;
		if (i == 0) {
			f = node1->forb;
			t = node2->trans;
		} else {
			f = node2->forb;
			t = node1->trans;
		}

		while (f && t) {
			if (f->event == t->event) return false;
			if (f->event > t->event) t = t->next_transition;
			else f = f->next_event;
		}
	}
	return true;
}