
extern "C" {
#include "des_data.h"
#include "mymalloc.h"
void meet_x64(INT_S s1,
           state_node *t1,
           INT_S s2,
           state_node *t2,
           INT_S *s3,
           state_node **t3,
           unsigned long long **macro64_c);

}

#include <unordered_map>

/*
	s1, t1 is the size and first automaton.
	s2, t2 is the second. This routine computes MEET(t1, t2).
	s3, t3 is the resulting automaton. This routine allocates memory for this, so it must be freed by the caller.
	macro64_c is an array of 64-bit long long that maps each state in t3 to the pair of states in (t2, t1) that
	    produced this output state. In each entry, the upper 32 bits is an index into t2, and the lower 32 bits
		is an index into t1. macro64_c[0] is 0x00000000_00000000, mapping to (t2, t1) = (0,0): The first output state
		corresponds to the initial states of t1 and t2. I renamed it macro64_c to emphasize that this uses a different
		encoding than macro_c used elsewhere.
	macro_ab is no longer produced as the original meet2() function did, as it required n^2 memory. It's been replaced
	    with a hash_map, as it's anticipated that only a small fraction of the possible (s1*s2) states would be visited.

	I have not benchmarked hash_map vs map for speed and memory usage, nor thought through how to deal with
	input automata that really do generate very large n^2 outputs. For worst-case input, using hash_map/map would 
	consume more memory than the original macro_ab n^2 array. This routine is betting that nothing close to this
	worst case occurs in practice.
*/
void meet_x64(INT_S s1,
           state_node *t1,
           INT_S s2,
           state_node *t2,
           INT_S *s3,
           state_node **t3,
           unsigned long long **macro64_c)
{
   INT_S t1i, t2j;
   INT_T colptr1, colptr2;
   INT_T tran1, tran2;
   INT_S srcstate, newstate;
   INT_S a,b,i;

   std::unordered_map< unsigned long long , int > macro_ab;	// Maps a pair of input states (t2, t1) to the output state in t3.

   if (s1 == 0L || s2 == 0L) {
      *s3 = 0;
      *t3 = NULL;
      return;
   }
   *s3  = 0;
   *t3  = NULL;
   
   // Common case: Output size doesn't exceed input size. Dynamically resize macro64_c later if necessary, to reduce peak memory usage.
   INT_S macro64_c_size = CMAX(s1,s2);
   *macro64_c  = (unsigned long long*) CALLOC(macro64_c_size, sizeof(unsigned long long));
   if (*macro64_c == NULL) {
      mem_result = 1;
      return;
   }

   // Visit initial state: (t2=0, t1=0) -> (t3 = 0)
   macro_ab[0] = 0;
   (*macro64_c)[0] = 0;
   srcstate = newstate = 0L;
   t1i = t2j = 0L;
   do {
      t1i = (INT_OS)(*macro64_c)[srcstate];
	  t2j = (*macro64_c)[srcstate] >> 32;
      colptr1 = 0;
      colptr2 = 0;
	  // Starting from this pair of states, find events shared by states t1[t1i] and t2[t2i].
      while (colptr1 < t1[t1i].numelts && colptr2 < t2[t2j].numelts) {
         tran1 = t1[t1i].next[colptr1].data1;	// Event
         tran2 = t2[t2j].next[colptr2].data1;
         if (tran1 != tran2) {
            if (tran1 < tran2)
               colptr1++;
            else
               colptr2++;
            continue;
         }

         a = t1[t1i].next[colptr1].data2;	// target state
         b = t2[t2j].next[colptr2].data2;

		 // See if (b, a) has been visited already. 
		 // If not, add a new output state. Otherwise, add the edge to the existing output state. 
		 std::unordered_map< unsigned long long, int>::iterator ms = macro_ab.find((((unsigned long long)b)<<32) + a);
         if (ms == macro_ab.end()) {
            newstate++;
			if (newstate >= macro64_c_size) {
				// Too many output states: Expand macro64_c output array. Add 25% and round up to 4KB.
				macro64_c_size = (macro64_c_size + (macro64_c_size>>2) + 0x1ff) & ~0x1ff;
				*macro64_c = (unsigned long long*) REALLOC(*macro64_c, macro64_c_size * sizeof(unsigned long long));
				if (!(*macro64_c)) {
					mem_result = 1;
					return;
				}
			}
			// Record that we've visited this state pair, and update macro_ab and macro64_c mappings.
			macro_ab[ (((unsigned long long)b)<<32) + a ] = (int)newstate;
            (*macro64_c)[newstate] = (((unsigned long long)b)<<32) + a;
            insertlist4(srcstate, tran1, newstate, s3, t3);
            if (mem_result == 1) return;
         } else {
            insertlist4(srcstate, tran1, ms->second, s3, t3);
            if (mem_result == 1) return;
         }
         colptr1++;
         colptr2++;
      }

      srcstate++;
   } while (srcstate <= newstate);

   resize_des(t3,*s3,newstate+1);
   *s3 = newstate+1;

   (*t3)[0].reached = true;
   for (i=0; i < *s3; i++) {
     unsigned long long ij = (*macro64_c)[i];
     (*t3)[i].marked = t1[(unsigned int)ij].marked && t2[ij>>32].marked;
   }

   /* Should be safe because the size is smaller than before */
   *macro64_c = (unsigned long long*) REALLOC(*macro64_c, sizeof(unsigned long long)*(*s3));
}