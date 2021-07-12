/* Minimization using an (n log n) algorithm.
   This is a fairly memory intensive function */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "minm.h"
#include "setup.h"
#include "mymalloc.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Find the position of the event in a list */
INT_T findpos(INT_T e,           /* element to find */
              INT_T *L,          /* list to search */
              INT_T size)
{
   /* Binary search */
   INT_T pos;
   INT_T lower, upper;
   boolean found;

   /* Do a binary search. */
   found = false;
   pos = 0;
   if (size > 1) {
     lower = 1;
     upper = size;
     while ( (found == false) && (lower <= upper) ) {
       pos = (lower + upper) / 2;
       if (e == L[pos-1]) {
          pos--;
          found = true;
       } else if (e > L[pos-1]) {
          lower = pos+1;
       } else {
          upper = pos-1;
       }
     }
   } else if (size == 1) {
     if (e == L[0]) {
       found = true;
     }
   }

   assert(found);     /* Found should always be true */

   return pos;
}

boolean inlist2(INT_T e,
                tran_node **L,
                INT_T size)
{
   INT_T pos;
   INT_T lower, upper;
   boolean found;

   /* Do a binary search. */
   found = false;
   pos = 0;
   if (size > 1) {
     lower = 1;
     upper = size;
     while ( (found == false) && (lower <= upper) ) {
       pos = (lower + upper) / 2;
       if (e == (*L)[pos-1].data1) {
          found = true;
       } else if (e > (*L)[pos-1].data1) {
          lower = pos+1;
       } else {
          upper = pos-1;
       }
     }

     if (found == false) {
        if (e < (*L)[pos-1].data1)
           pos--;
     }
   } else if (size == 1) {
     if (e == (*L)[0].data1) {
       found = true;
     } else if (e > (*L)[0].data1) {
       pos = 1;
     }
   }

   return found;
}

typedef struct t_list {
   INT_S numelts;
   INT_S *next;
} t_list;

void remove_first(t_list *L)
{
/*   INT_S i; */

   if (L->numelts > 0) {
     L->numelts--;
     if (L->numelts > 0)
       memmove(&(L->next[0]), &(L->next[1]), sizeof(INT_S)*(L->numelts));

/*     for (i=0; i < L->numelts; i++) {
        L->next[i] = L->next[i+1];
     } */
   }
   L->next = (INT_S*) REALLOC(L->next, sizeof(INT_S)*(L->numelts));
}

static INT_S n;                                  /* Number of states */
static INT_S *statenext, *statelast;             /* 2 x N            */
static INT_S *tr_next, *tr_last;                 /* 2 x N x events   */
static t_list *split;                            /* 2 x N            */
static INT_S *block, *numinblock, *numinsplit;   /* N                */
static INT_S *blocksplit;                        /* N                */
static INT_S *numtran, *num_invtran;                           /* N x events       */
static INT_S *onblocklist_tr;                    /* N x events       */
static INT_S *n__tr, *n__invtr;                             /* N x events       */
static t_list *ptr_blocklist_tr;                 /* events           */

static t_list ptr_splitblocks;                   /* a list           */

static INT_T *event,  s_event;    /* List of all unique events */

typedef INT_S TWO_DIM[2];

typedef struct part_state {
   boolean m: 1;        /* 1-bit   */
   INT_V v: 15;         /* 15-bit  */    /* Covers 0..999 */
   INT_T e;             /* 16-bit  */
   INT_S state;
} part_state;

void add_part_state(boolean m, INT_V v, INT_T e,
                    part_state **L, INT_S size, INT_S *state, INT_B  *ok)
{
   /* If ok = true then
         we add it to the list because it is not there.
      else
         in "state" return the position
      endif
    */

   INT_S pos;
   INT_S lower, upper;
   boolean found;

   *ok = false;

   /* Do a binary search. */
   found = false;
   pos = 0;
   if (size > 1) {
     lower = 1;
     upper = size;
     while ( (found == false) && (lower <= upper) ) {
       pos = (lower + upper) / 2;
       if (m == (*L)[pos-1].m && v == (*L)[pos-1].v && e == (*L)[pos-1].e) {
          found = true;
       } else if (m > (*L)[pos-1].m || (m == (*L)[pos-1].m && v > (*L)[pos-1].v ) ||
                  (m == (*L)[pos-1].m && v == (*L)[pos-1].v && e > (*L)[pos-1].e ) ) {
          lower = pos+1;
       } else {
          upper = pos-1;
       }
     }

     if (found == false) {
        if (m < (*L)[pos-1].m || (m == (*L)[pos-1].m && v < (*L)[pos-1].v) ||
            (m == (*L)[pos-1].m && v == (*L)[pos-1].v && e < (*L)[pos-1].e ) )
           pos--;
     }
   } else if (size == 1) {
     if (m == (*L)[0].m && v == (*L)[0].v && e == (*L)[0].e) {
       pos = 1;
       found = true;
     } else if (m > (*L)[0].m || (m == (*L)[0].m && v > (*L)[0].v) ||
                (m == (*L)[0].m && v == (*L)[0].v && e > (*L)[0].e ) ) {
       pos = 1;
     }
   }

   if (found == true) {
     *state = (*L)[pos-1].state;
     return;
   }

   /* Make space for new element */
   *L = (part_state*) REALLOC(*L, sizeof(part_state)*(size+1));
   if (*L == NULL) {
      mem_result = 1;
      return;
   }

   /* Move over any elements down the list */
   if ((size-pos) > 0)
      memmove(&(*L)[pos+1], &(*L)[pos], sizeof(part_state)*(size-pos));

   /* Insert the element into the list */
   (*L)[pos].m = m;
   (*L)[pos].v = v;
   (*L)[pos].e = e;
   (*L)[pos].state = *state;

   *ok = true;
}

void insertstate(INT_S x, INT_S y)
{
   statenext[n+y] = statenext[x];
   statelast[n+y] = x;
   if (statenext[x] != -1)
       statelast[statenext[x]] = n+y;
   statenext[x] = n+y;
}

void insert_tr(INT_T j, INT_S x, INT_S y)
{
   tr_next[(2*n*j)+n+y] = tr_next[(2*n*j)+x];
   tr_last[(2*n*j)+n+y] = x;
   if (tr_next[(2*n*j)+x] != -1)
       tr_last[ (2*n*j) + (tr_next[ (2*n*j)]+ x ) ] = n+y;
   tr_next[(2*n*j)+x] = n+y;
}

void minimize2(INT_S *s1, state_node **t1)
{
   INT_S i, s;                /* Counter */
   INT_T j, t;                /* Counter */
   INT_B  ok;                /* Boolean flag */
   INT_S ii, jj;              /* Tempory variables */
   INT_T ee, pos;
   state_node *inv_t1;        /* Inverse t1 */
   INT_S freeblock;
/*   INT_S xx, yy; */
   INT_S state, laststate, k, bsplit, cell;
   INT_S *mapState, *mapState2;
   INT_S s2;
   state_node *t2;
   part_state *part_array;  INT_S s_part_array;
   INT_S block_id;
   INT_B  any_diff, equal; /* contain_flag, other_flag */
/*   INT_S which_block; */


   inv_t1 = NULL;

   statenext = statelast = NULL;
   tr_next = tr_last = NULL;
   split = NULL;
   block = numinblock = numinsplit = NULL;
   numtran = NULL;
   onblocklist_tr = NULL;
   n__tr = NULL;
   ptr_blocklist_tr = NULL;
   ptr_splitblocks.next = NULL; ptr_splitblocks.numelts = 0;
   mapState = NULL; mapState2 = NULL;
   t2 = NULL;
   part_array = NULL; s_part_array = 0;

   event = NULL; s_event = 0;

   /* Need at least 2 states to minimize */
   if (*s1 <= 1) return;

   n = *s1;

   /* Step 0a.
    *   Construct a list of all unique transition labels.
    */

   for (s=0; s < n; s++) {
      for (j=0; j < (*t1)[s].numelts; j++) {
         addordlist( (*t1)[s].next[j].data1, &event, s_event, &ok);
         if (ok) s_event++;
      }
   }

   /* Step 0b.
    *   Allocate memory for all non-dynamic data structures
    */

   inv_t1 = newdes(n);
   if (inv_t1 == NULL) {
      mem_result = 1;
      goto FREEMEM;
   }

   /* Double link-list for B(i) */
   statenext = (INT_S*) MALLOC(sizeof(INT_S) * 2 * n);
   statelast = (INT_S*) MALLOC(sizeof(INT_S) * 2 * n);
   if ( (statenext == NULL) || (statelast == NULL) ) {
      mem_result = 1;
      goto FREEMEM;
   }

   /* Double link list for B(B(i),a) */
/*   tr_next = (INT_S*) malloc(sizeof(INT_S) * 2 * n * s_event);
   tr_last = (INT_S*) malloc(sizeof(INT_S) * 2 * n * s_event);
   if ( (tr_next == NULL) || (tr_last == NULL) ) {
      mem_result = 1;
      goto FREEMEM;
   }
*/
   /* Fill state_next, state_last, tr_next, tr_last with -1 */
   memset(statenext, 0xff, sizeof(INT_S) * 2 * n);
   memset(statelast, 0xff, sizeof(INT_S) * 2 * n);
/*   memset(tr_next,   0xff, sizeof(INT_S) * 2 * n * s_event);
   memset(tr_last,   0xff, sizeof(INT_S) * 2 * n * s_event);
*/
   split = (t_list*) MALLOC(sizeof(t_list) * 2 * n);
   if (split == NULL) {
      mem_result = 1;
      goto FREEMEM;
   }
   memset(split, 0x00, sizeof(t_list) * 2 * n);

   block      = (INT_S*) MALLOC(sizeof(INT_S) * n);
   numinblock = (INT_S*) MALLOC(sizeof(INT_S) * n);
   numinsplit = (INT_S*) MALLOC(sizeof(INT_S) * n);
   if ( (block == NULL) || (numinblock == NULL) || (numinsplit == NULL) ) {
      mem_result = 1;
      goto FREEMEM;
   }

   memset(block,      0x00, sizeof(INT_S) * n);
   memset(numinblock, 0x00, sizeof(INT_S) * n);
   memset(numinsplit, 0x00, sizeof(INT_S) * n);

   blocksplit = (INT_S*) MALLOC(sizeof(INT_S) * n);
   if (blocksplit == NULL) {
      mem_result = 1;
      goto FREEMEM;
   }
   memset(blocksplit, 0x00, sizeof(INT_S) * n);

   numtran = (INT_S*) MALLOC(sizeof(INT_S) * n * s_event);
   if (numtran == NULL) {
      mem_result = 1;
      goto FREEMEM;
   }
   memset(numtran, 0x00, sizeof(INT_S) * n * s_event);

   onblocklist_tr = (INT_S*) MALLOC(sizeof(INT_S) * n * s_event);
   if (onblocklist_tr == NULL) {
      mem_result = 1;
      goto FREEMEM;
   }
   memset(onblocklist_tr, 0x00, sizeof(INT_S) * n * s_event);

   n__tr = (INT_S*) MALLOC(sizeof(INT_S) * n * s_event);
   if (n__tr == NULL) {
      mem_result = 1;
      goto FREEMEM;
   }
   memset(n__tr, 0x00, sizeof(INT_S) * n * s_event);

   ptr_blocklist_tr = (t_list*) CALLOC(s_event, sizeof(t_list) * s_event);
   if (ptr_blocklist_tr == NULL) {
      mem_result = 1;
      goto FREEMEM;
   }

   mapState = (INT_S*) CALLOC(*s1, sizeof(INT_S));
   if (mapState == NULL) {
      mem_result = 1;
      goto FREEMEM;
   }

   /* Step 1.
    *   For each state 's' in 'S' and each transition 'a' in 'I'
    *   construct the inverse transition table.
    *   Also, fill in 'n__tr'.
    *   Also, map the "events" so they are sequence from 0 to s_event-1.
    */

   for (s=0; s < n; s++) {
      for (j=0; j < (*t1)[s].numelts; j++) {
         /* [ii,ee,jj] -> [jj,ee,ii] */
         ii = s;
         ee = (*t1)[s].next[j].data1;
         jj = (*t1)[s].next[j].data2;

         /* Translate 'ee' into a position to 'n__tr' */
         pos = findpos(ee, event, s_event);

         /* Do the mapping to the new transition names */
         ee = pos;
         (*t1)[s].next[j].data1 = pos;

         addordlist1(ee, ii, &inv_t1[jj].next, inv_t1[jj].numelts, &ok);
         if (ok) inv_t1[jj].numelts++;

         n__tr[(n*pos)+s]++;
      }
   }

   /* Step 2.
    *   Set up the initial blocks.
    *   Construct B(0)=F, B(1)=S-F.   -- We number from 0, not 1 in 'C'
    *   Construct for each 'a' in I and  0 <= i < s_event
    *
    *     ^                                   -1
    *     B(B(i),a) = { s| s in B(i) and gamma  (s,a) ^= empty }
    *          ^
    * B(i) and B(B(i),a) are represented by double link-list.
    *
    * Changed so that the initial partition is based on
    * (mark, vocal, degree).
    */

   for (i=0; i < *s1; i++) {
      block_id = s_part_array;
      add_part_state( (*t1)[i].marked,
                      (*t1)[i].vocal,
                      (*t1)[i].numelts,
                      &part_array, s_part_array, &block_id, &ok);
      if (ok) s_part_array++;

      /* Add to B(block_id) list */
      insertstate(block_id,i);
      for (j=0; j < s_event; j++) {
         if ( inlist2(j, &(*t1)[i].next, (*t1)[i].numelts) ) {
/*            insert_tr(j,block_id,i);*/
            numtran[(n*j)+block_id] += n__tr[(n*j)+i];
         }
      }
      block[i] = block_id;
      numinblock[block_id]++;
   }

   if (s_part_array == 1) {
      /* No partition found */

      /* Reset the variables as if we did not do any thing above */
      memset(statenext, 0xff, sizeof(INT_S) * 2 * n);
      memset(statelast, 0xff, sizeof(INT_S) * 2 * n);
/*      memset(tr_next,   0xff, sizeof(INT_S) * 2 * n * s_event);
      memset(tr_last,   0xff, sizeof(INT_S) * 2 * n * s_event);
*/
      memset(numtran, 0x00, sizeof(INT_S) * n * s_event);

      memset(block,      0x00, sizeof(INT_S) * n);
      memset(numinblock, 0x00, sizeof(INT_S) * n);

      /* Use second partition method. */

      block_id = 0;
      /* Add to B(block_id) list */
      insertstate(block_id,0);
      for (j=0; j < s_event; j++) {
         if ( inlist2(j, &(*t1)[0].next, (*t1)[0].numelts) ) {
/*            insert_tr(j,block_id,0);*/
            numtran[(n*j)+block_id] += n__tr[(n*j)+0];
         }
      }
      block[0] = block_id;
      numinblock[block_id]++;

      any_diff = false;
      for (i=1; i < *s1; i++) {
         equal = true;
         for (j=0; j < (*t1)[0].numelts; j++) {
            if ( (*t1)[0].next[j].data1 != (*t1)[i].next[j].data1 ) {
                equal = false;
                break;
            }
         }

         if (equal == false) {
            any_diff = true;
            block_id = 1;
         } else {
            block_id = 0;
         }

         /* Add to B(block_id) list */
         insertstate(block_id,i);
         for (j=0; j < s_event; j++) {
           if ( inlist2(j, &(*t1)[i].next, (*t1)[i].numelts) ) {
/*              insert_tr(j,block_id,i);*/
              numtran[(n*j)+block_id] += n__tr[(n*j)+i];
           }
         }
         block[i] = block_id;
         numinblock[block_id]++;
      }

      if (any_diff == false) {
         /* Minimize to one state and recode */
         s2 = 1;
         t2 = newdes(s2);
         if ( t2 == NULL) {
            mem_result = 1;
            goto FREEMEM;
         }

         /* Map back the event labels */
         for (s=0; s < n; s++) {
            for (j=0; j < (*t1)[s].numelts; j++) {
               ee = (*t1)[s].next[j].data1;
               (*t1)[s].next[j].data1 = event[ee];
            }
         }

         goto RECODE;
      } else {
         s_part_array = 2;
      }
   }

   free(part_array); part_array = NULL;

   /* Step 3
    *   Set k = 2
    */
   freeblock = s_part_array;

   /* Step 4
    *   For each 'a' in 'I' construct
    *
    *  L(a) = {0} if  |B(B(0),a)| <= |B(B(1),a)|
    *         {1} otherwise
    *
    *  Change: Assign the lowest |B(B(i),a|.  If equal then
    *          use the lowest block number.
    *
    *  To improve speed, we should not put in the "biggest" block
    *  only the smaller ones.
    */

   for (j=0; j < s_event; j++) {
/*      INT_S mm;

      xx = numtran[n*j+0];
      mm = 0;

      for (i=1; i < s_part_array; i++) {
         yy = numtran[n*j+i];
         if (yy > xx) {
            block_id = mm;
            xx = yy;
            mm = i;
         } else {
            block_id = i;
         }

         // block_id is the smallest |B(B(i),a|
         addstatelist(block_id, &(ptr_blocklist_tr[j].next),
                                  ptr_blocklist_tr[j].numelts, &ok);
         if (ok) ptr_blocklist_tr[j].numelts++;
         onblocklist_tr[n*j+block_id] = block_id+1;
      }
*/

      for (i=0; i < s_part_array; i++) {
         block_id = i;
         addstatelist(block_id, &(ptr_blocklist_tr[j].next),
                                  ptr_blocklist_tr[j].numelts, &ok);
         if (ok) ptr_blocklist_tr[j].numelts++;
         onblocklist_tr[n*j+block_id] = block_id+1;
      }
   }

REPEAT:
   /* Step 5
    *   Select 'a' in 'I' and 'i' in L(a).  The algorithm
    *   terminates when L(a) = empty for each 'a' in 'I'.
    */

    for (t=0; t < s_event; t++) {
       if (ptr_blocklist_tr[t].numelts != 0) {
           jj = ptr_blocklist_tr[t].next[0];

           /* Step 6
            * Delete 'i' from L(a)
            */
           remove_first(&ptr_blocklist_tr[t]);
           onblocklist_tr[(n*t)+jj] = 0;
           goto DIV;
       }
    }
    goto FINISH;
DIV:
    /* Step 7 */
/*    state = tr_next[(2*n*t)+jj]; */

    state = statenext[jj];

    while (state != -1) {
       /* t = transition */
       /* state */

       s = state-n;

       for (ee=0; ee < inv_t1[s].numelts; ee++) {
          if (inv_t1[s].next[ee].data1 == t) {
             laststate = inv_t1[s].next[ee].data2;
             k = block[laststate];
             addstatelist(laststate, &(split[k].next), split[k].numelts, &ok);
             if (ok) split[k].numelts++;

             if (blocksplit[k] == 0) {
                addstatelist(k, &(ptr_splitblocks.next),
                                 ptr_splitblocks.numelts, &ok);
                if (ok) ptr_splitblocks.numelts++;
             }
             numinsplit[k]++;
             blocksplit[k] = 1;
          }
       }

/*       state = tr_next[(2*n*t)+state]; */
       state = statenext[state];
    }

RETURN:
    /* Refine block */
    if (ptr_splitblocks.numelts == 0) goto REPEAT;
    bsplit = ptr_splitblocks.next[0];
    remove_first(&ptr_splitblocks);
    blocksplit[bsplit] = 0;

    if (numinblock[bsplit] == numinsplit[bsplit]) {
       numinsplit[bsplit] = 0;
       if (split[bsplit].next != NULL)
          free(split[bsplit].next);
       split[bsplit].next = NULL;
       split[bsplit].numelts = 0;
       goto RETURN;
    }

    numinblock[bsplit] -= numinsplit[bsplit];
    numinblock[freeblock] = numinsplit[bsplit];
    numinsplit[bsplit] = 0;
    while(split[bsplit].numelts != 0) {
       state = split[bsplit].next[0];
       remove_first(&split[bsplit]);

       statenext[statelast[n+state]] = statenext[n+state];
       if (statenext[n+state] != -1)
           statelast[statenext[n+state]] = statelast[n+state];
       insertstate(freeblock,state);
       block[state] = freeblock;

       for (ee=0; ee < s_event; ee++) {
          if (inlist2(ee, &inv_t1[state].next, inv_t1[state].numelts)) {
/*
            tr_next[ (2*n*ee) + tr_last[(2*n*ee)+n+state]]
               = tr_next[ (2*n*ee) + n + state];
            if (tr_next[ (2*n*ee) + n + state] != -1) {
               tr_last[ (2*n*ee) + tr_next[(2*n*ee)+n+state]]
                = tr_last[ (2*n*ee) + n + state];
            }
            insert_tr(ee,freeblock,state);
*/
            numtran[n*ee+bsplit] -= n__tr[n*ee+state];
            numtran[n*ee+freeblock] += n__tr[n*ee+state];
          }
       }
    }

    /* Add new block to blocklist */
    for (j=0; j < s_event; j++) {
       if ( (onblocklist_tr[n*j+bsplit] != 1) && (numtran[n*j+bsplit] > 0)
            && (numtran[n*j+bsplit] <= numtran[n*j+freeblock]) )
       {
          addstatelist(bsplit, &(ptr_blocklist_tr[j].next),
                                ptr_blocklist_tr[j].numelts, &ok);
          if (ok) ptr_blocklist_tr[j].numelts++;
          onblocklist_tr[n*j+bsplit] = 1;
       } else {
          addstatelist(freeblock, &(ptr_blocklist_tr[j].next),
                                   ptr_blocklist_tr[j].numelts, &ok);
          if (ok) ptr_blocklist_tr[j].numelts++;
          onblocklist_tr[n*j+freeblock] = 1;
       }
    }

    /* Step 7d
     *  Set k = k + 1
     */
    freeblock++;

    /* Step 8
     * Return to step 5
     */

    if (ptr_splitblocks.numelts == 0) goto REPEAT;
    goto RETURN;
FINISH:
    /* Map back the transition to the one saved */
    /* [ii,ee,jj] -> [jj,ee,ii] */
    for (s=0; s < n; s++) {
      for (j=0; j < (*t1)[s].numelts; j++) {
         ee = (*t1)[s].next[j].data1;
         (*t1)[s].next[j].data1 = event[ee];
      }
    }

    if (freeblock == n) {
       /* No change */
       goto FREEMEM;
    }

    for (i=0; i < n; i++) {
       cell = statenext[i];
       while (cell != -1) {
          s = cell-n;
          mapState[s] = i;
          cell = statenext[cell];
       }
    }

    freedes(n, &inv_t1);
    inv_t1 = NULL;

    s2 = freeblock;
    t2 = newdes(s2);
    if ( t2 == NULL) {
       mem_result = 1;
       goto FREEMEM;
    }

    /* This minimization algorithm may cause INIT state zero to be
       mapped to another state number.  This cause isomorph to fail.
       For now map state zero be at zero always. */
    if (mapState[0] != 0) {
       cell = mapState[0];
       mapState[0] = 0;
       for (i=1; i < n; i++) {
         if (mapState[i] == 0) {
           mapState[i] = cell;
         } else if (mapState[i] == cell) {
           mapState[i] = 0;
         }
       }
    }

    /* Recode state */
    mapState2 = (INT_S*) MALLOC(n*sizeof(INT_S));
    if (mapState == NULL) {
       mem_result = 1;
       goto FREEMEM;
    }
    memset(mapState2, 0xff, sizeof(INT_S) * n);

    s = 0;
    for (i=0; i < n; i++) {
       if (mapState2[mapState[i]] == -1L) {
          mapState2[mapState[i]] = s;
          mapState[i] = s;
          s++;
       } else {
          mapState[i] = mapState2[mapState[i]];
       }
    }

    /* Do some checking */
    for (i=0; i < n; i++) {
       s = mapState[i];
       if (s >= 0 && s < s2) {
       } else {
          assert(s > 0);
       }
    }

RECODE:
    recode_min(*s1,*t1,s2,t2,mapState);
    freedes(*s1, t1);
    *s1 = s2;
    *t1 = t2;

FREEMEM:
    if (mapState != NULL) free(mapState);
    if (mapState2 != NULL) free(mapState2);
    if (event != NULL) free(event);
    freedes(n, &inv_t1);
    if (statenext != NULL) free(statenext);
    if (statelast != NULL) free(statelast);
    if (tr_next != NULL) free(tr_next);
    if (tr_last != NULL) free(tr_last);
    if (split != NULL) {
       for (i=0; i < 2*n; i++) {
          if (split[i].numelts != 0)
             free(split[i].next);
       }
    }
    if (split != NULL) free(split);
    if (block != NULL) free(block);
    if (numinblock != NULL) free(numinblock);
    if (numinsplit != NULL) free(numinsplit);
    if (blocksplit != NULL) free(blocksplit);
    if (numtran != NULL) free(numtran);
    if (onblocklist_tr != NULL) free(onblocklist_tr);
    if (n__tr != NULL) free(n__tr);
    if (ptr_blocklist_tr != NULL) {
        for (j=0; j < s_event; j++) {
            if (ptr_blocklist_tr[j].numelts != 0)
               free(ptr_blocklist_tr[j].next);
        }
        free(ptr_blocklist_tr);
    }

    if (ptr_splitblocks.next != NULL) free(ptr_splitblocks.next);
    if (part_array != NULL) free(part_array);
}

void loc_refinement_disable(INT_S*s1, state_node**t1, INT_S sn, state_node*tn)
{
   INT_S i, s;                /* Counter */
   INT_T j, t;                /* Counter */
   INT_B  ok;                /* Boolean flag */
   INT_S ii, jj;              /* Tempory variables */
   INT_T ee, pos;
   state_node *inv_t1;        /* Inverse t1 */
   INT_S freeblock;
/*   INT_S xx, yy; */
   INT_S state, laststate, k, bsplit, cell;
   INT_S *mapState, *mapState2;
   INT_S s2;
   state_node *t2;
   // part_state *part_array; INT_S s_part_array;
   INT_S block_id;

   // part_node *par; INT_S s_par;

   // s_par = 0; par = NULL;
 //  INT_B  any_diff, equal; /* contain_flag, other_flag */
/*   INT_S which_block; */

   if(*s1 != sn){
	   return;
   }

   inv_t1 = NULL;

   statenext = statelast = NULL;
   tr_next = tr_last = NULL;
   split = NULL;
   block = numinblock = numinsplit = NULL;
   numtran = num_invtran = NULL;
   onblocklist_tr = NULL;
   n__tr = n__invtr = NULL;
   ptr_blocklist_tr = NULL;
   ptr_splitblocks.next = NULL; ptr_splitblocks.numelts = 0;
   mapState = NULL; mapState2 = NULL;
   t2 = NULL;
   // part_array = NULL; s_part_array = 0;

   event = NULL; s_event = 0;

   /* Need at least 2 states to minimize */
   if (*s1 <= 1) return;

   n = *s1;

   /* Step 0a.
    *   Construct a list of all unique transition labels.
    */

   for (s=0; s < n; s++) {
      for (j=0; j < (*t1)[s].numelts; j++) {
         addordlist( (*t1)[s].next[j].data1, &event, s_event, &ok);
         if (ok) s_event++;
      }
   }

   /* Step 0b.
    *   Allocate memory for all non-dynamic data structures
    */

   inv_t1 = newdes(n);
   if (inv_t1 == NULL) {
      mem_result = 1;
      goto FREEMEM;
   }

   /* Double link-list for B(i) */
   statenext = (INT_S*) MALLOC(sizeof(INT_S) * 2 * n);
   statelast = (INT_S*) MALLOC(sizeof(INT_S) * 2 * n);
   if ( (statenext == NULL) || (statelast == NULL) ) {
      mem_result = 1;
      goto FREEMEM;
   }

   /* Double link list for B(B(i),a) */
/*   tr_next = (INT_S*) malloc(sizeof(INT_S) * 2 * n * s_event);
   tr_last = (INT_S*) malloc(sizeof(INT_S) * 2 * n * s_event);
   if ( (tr_next == NULL) || (tr_last == NULL) ) {
      mem_result = 1;
      goto FREEMEM;
   }
*/
   /* Fill state_next, state_last, tr_next, tr_last with -1 */
   memset(statenext, 0xff, sizeof(INT_S) * 2 * n);
   memset(statelast, 0xff, sizeof(INT_S) * 2 * n);
/*   memset(tr_next,   0xff, sizeof(INT_S) * 2 * n * s_event);
   memset(tr_last,   0xff, sizeof(INT_S) * 2 * n * s_event);
*/
   split = (t_list*) MALLOC(sizeof(t_list) * 2 * n);
   if (split == NULL) {
      mem_result = 1;
      goto FREEMEM;
   }
   memset(split, 0x00, sizeof(t_list) * 2 * n);

   block      = (INT_S*) MALLOC(sizeof(INT_S) * n);
   numinblock = (INT_S*) MALLOC(sizeof(INT_S) * n);
   numinsplit = (INT_S*) MALLOC(sizeof(INT_S) * n);
   if ( (block == NULL) || (numinblock == NULL) || (numinsplit == NULL) ) {
      mem_result = 1;
      goto FREEMEM;
   }

   memset(block,      0x00, sizeof(INT_S) * n);
   memset(numinblock, 0x00, sizeof(INT_S) * n);
   memset(numinsplit, 0x00, sizeof(INT_S) * n);

   blocksplit = (INT_S*) MALLOC(sizeof(INT_S) * n);
   if (blocksplit == NULL) {
      mem_result = 1;
      goto FREEMEM;
   }
   memset(blocksplit, 0x00, sizeof(INT_S) * n);

   numtran = (INT_S*) MALLOC(sizeof(INT_S) * n * s_event);
   if (numtran == NULL) {
      mem_result = 1;
      goto FREEMEM;
   }
   memset(numtran, 0x00, sizeof(INT_S) * n * s_event);

   num_invtran = (INT_S*) MALLOC(sizeof(INT_S) * n * s_event);
   if (num_invtran == NULL) {
	   mem_result = 1;
	   goto FREEMEM;
   }
   memset(num_invtran, 0x00, sizeof(INT_S) * n * s_event);

   onblocklist_tr = (INT_S*) MALLOC(sizeof(INT_S) * n * s_event);
   if (onblocklist_tr == NULL) {
      mem_result = 1;
      goto FREEMEM;
   }
   memset(onblocklist_tr, 0x00, sizeof(INT_S) * n * s_event);

   n__tr = (INT_S*) MALLOC(sizeof(INT_S) * n * s_event);
   if (n__tr == NULL) {
      mem_result = 1;
      goto FREEMEM;
   }
   memset(n__tr, 0x00, sizeof(INT_S) * n * s_event);

   n__invtr = (INT_S*) MALLOC(sizeof(INT_S) * n * s_event);
   if (n__invtr == NULL) {
	   mem_result = 1;
	   goto FREEMEM;
   }
   memset(n__invtr, 0x00, sizeof(INT_S) * n * s_event);

   ptr_blocklist_tr = (t_list*) CALLOC(s_event, sizeof(t_list) * s_event);
   if (ptr_blocklist_tr == NULL) {
      mem_result = 1;
      goto FREEMEM;
   }

   mapState = (INT_S*) CALLOC(*s1, sizeof(INT_S));
   if (mapState == NULL) {
      mem_result = 1;
      goto FREEMEM;
   }

   /* Step 1.
    *   For each state 's' in 'S' and each transition 'a' in 'I'
    *   construct the inverse transition table.
    *   Also, fill in 'n__tr'.
    *   Also, map the "events" so they are sequence from 0 to s_event-1.
    */

   for (s=0; s < n; s++) {
      for (j=0; j < (*t1)[s].numelts; j++) {
         /* [ii,ee,jj] -> [jj,ee,ii] */
         ii = s;
         ee = (*t1)[s].next[j].data1;
         jj = (*t1)[s].next[j].data2;

         /* Translate 'ee' into a position to 'n__tr' */
         pos = findpos(ee, event, s_event);

         /* Do the mapping to the new transition names */
         ee = pos;
         (*t1)[s].next[j].data1 = pos;

         addordlist1(ee, ii, &inv_t1[jj].next, inv_t1[jj].numelts, &ok);
         if (ok) inv_t1[jj].numelts++;

         n__tr[(n*pos)+s]++;
		 n__invtr[(n*pos)+jj]++;
      }
   }

   //Initialize the partition
   for (i=0; i < *s1; i++) {
	   if(tn[i].numelts == 0)
			block_id = 0;
	   else
		   block_id = 1;

	   /* Add to B(block_id) list */
	   insertstate(block_id,i);
	   for (j=0; j < s_event; j++) {
		   if ( inlist2(j, &(*t1)[i].next, (*t1)[i].numelts) ) {
			   numtran[(n*j)+block_id] += n__tr[(n*j)+i];
		   }
		   if ( inlist2(j, &(inv_t1)[i].next, (inv_t1)[i].numelts) ) {
			   num_invtran[(n*j)+block_id] += n__invtr[(n*j)+i];
		   }
	   }
	   block[i] = block_id;
	   numinblock[block_id]++;
   }

   if((numinblock[0] == *s1) || (numinblock[0] == 0)){
	   goto FINISH;
   }

   freeblock = 2;
   //Initialize the splitter
   if(numinblock[0] <= numinblock[1])
	   block_id = 0;
   else
	   block_id = 1;
   for (j=0; j < s_event; j++) {
	   if(num_invtran[n*j+block_id] >0){
			addstatelist(block_id, &(ptr_blocklist_tr[j].next),
                                  ptr_blocklist_tr[j].numelts, &ok);
			if (ok) ptr_blocklist_tr[j].numelts++;
			onblocklist_tr[n*j+block_id] = block_id + 1;
	   }
   }

REPEAT:
   /* Step 5
    *   Select 'a' in 'I' and 'i' in L(a).  The algorithm
    *   terminates when L(a) = empty for each 'a' in 'I'.
    */

    for (t=0; t < s_event; t++) {
       if (ptr_blocklist_tr[t].numelts != 0) {
           jj = ptr_blocklist_tr[t].next[0];

           /* Step 6
            * Delete 'i' from L(a)
            */
           remove_first(&ptr_blocklist_tr[t]);
           onblocklist_tr[(n*t)+jj] = 0;
           goto DIV;
       }
    }
    goto FINISH;
DIV:
    /* Step 7 */
/*    state = tr_next[(2*n*t)+jj]; */

    state = statenext[jj];

    while (state != -1) {
       /* t = transition */
       /* state */

       s = state-n;

       for (ee=0; ee < inv_t1[s].numelts; ee++) {
          if (inv_t1[s].next[ee].data1 == t) {
             laststate = inv_t1[s].next[ee].data2;
             k = block[laststate];
			 //if(numtran[n*t+k] < 2)  // First condition specific to loc algorithm
			//	 continue;
             addstatelist(laststate, &(split[k].next), split[k].numelts, &ok);
             if (ok) split[k].numelts++;

             if (blocksplit[k] == 0) {
                addstatelist(k, &(ptr_splitblocks.next),
                                 ptr_splitblocks.numelts, &ok);
                if (ok) ptr_splitblocks.numelts++;
             }
             numinsplit[k]++;
             blocksplit[k] = 1;
          }
       }
       state = statenext[state];
    }
RETURN:
    // Refine block
    if (ptr_splitblocks.numelts == 0) goto REPEAT;
    bsplit = ptr_splitblocks.next[0];
    remove_first(&ptr_splitblocks);
    blocksplit[bsplit] = 0;

    if (numtran[n*t+bsplit] == numinsplit[bsplit]) { //Second condition specific to loc algorithm
       numinsplit[bsplit] = 0;
       if (split[bsplit].next != NULL)
          free(split[bsplit].next);
       split[bsplit].next = NULL;
       split[bsplit].numelts = 0;
       goto RETURN;
    }
	//zprintn(numtran[n*t+bsplit]); zprints("-"); zprintn(numinsplit[bsplit]); zprints("\n");

    numinblock[bsplit] -= numinsplit[bsplit];
    numinblock[freeblock] = numinsplit[bsplit];
    numinsplit[bsplit] = 0;
    while(split[bsplit].numelts != 0) {
       state = split[bsplit].next[0];
       remove_first(&split[bsplit]);

       statenext[statelast[n+state]] = statenext[n+state];
       if (statenext[n+state] != -1)
           statelast[statenext[n+state]] = statelast[n+state];
       insertstate(freeblock,state);
       block[state] = freeblock;

       for (ee=0; ee < s_event; ee++) {
          if (inlist2(ee, &(*t1)[state].next, (*t1)[state].numelts)) {
            numtran[n*ee+bsplit] -= n__tr[n*ee+state];
            numtran[n*ee+freeblock] += n__tr[n*ee+state];
          }
		  if (inlist2(ee, &(inv_t1)[state].next, (inv_t1)[state].numelts)) {
			  num_invtran[n*ee+bsplit] -= n__invtr[n*ee+state];
			  num_invtran[n*ee+freeblock] += n__invtr[n*ee+state];
		  }
       }
    }
	//zprint_list(n*s_event, numtran);

    // Add new block to blocklist
    for (j=0; j < s_event; j++) {
       if ( (onblocklist_tr[n*j+bsplit] != 1)
            && (numinblock[bsplit] <= numinblock[freeblock]))
       {
          addstatelist(bsplit, &(ptr_blocklist_tr[j].next),
                                ptr_blocklist_tr[j].numelts, &ok);
          if (ok) ptr_blocklist_tr[j].numelts++;
          onblocklist_tr[n*j+bsplit] = 1;
       } else{
          addstatelist(freeblock, &(ptr_blocklist_tr[j].next),
                                   ptr_blocklist_tr[j].numelts, &ok);
          if (ok) ptr_blocklist_tr[j].numelts++;
          onblocklist_tr[n*j+freeblock] = 1;
       }
    }

    freeblock++;

/*	for (i=0; i < n; i++) {
		cell = statenext[i];
		while (cell != -1) {
			s = cell-n;
			mapState[s] = i;
			cell = statenext[cell];
		}
	}

	s_par = freeblock;
	par = (part_node*)CALLOC(s_par, sizeof(part_node));
	for (i=0; i < n; i++) {
		s = mapState[i];
		addstatelist(i, &(par[s].next), par[s].numelts,&ok);
		if(ok) par[s].numelts ++;
	}
	zprint_par(s_par,par);
	free_part(s_par, &par); s_par = 0; par = NULL;*/


    if (ptr_splitblocks.numelts == 0) goto REPEAT;
    goto RETURN;

FINISH:
    for (s=0; s < n; s++) {
      for (j=0; j < (*t1)[s].numelts; j++) {
         ee = (*t1)[s].next[j].data1;
         (*t1)[s].next[j].data1 = event[ee];
      }
    }

    if (freeblock == n) {
       /* No change */
       goto FREEMEM;
    }

    for (i=0; i < n; i++) {
       cell = statenext[i];
       while (cell != -1) {
          s = cell-n;
          mapState[s] = i;
          cell = statenext[cell];
       }
    }
/*
	s_par = freeblock;
	par = (part_node*)CALLOC(s_par, sizeof(part_node));
	for (i=0; i < n; i++) {
		s = mapState[i];
		addstatelist(i, &(par[s].next), par[s].numelts,&ok);
		if(ok) par[s].numelts ++;
	}
	zprint_par(s_par,par);
	free_part(s_par, &par); s_par = 0; par = NULL;*/

    freedes(n, &inv_t1);
    inv_t1 = NULL;

    s2 = freeblock;
    t2 = newdes(s2);
    if ( t2 == NULL) {
       mem_result = 1;
       goto FREEMEM;
    }

    /* This minimization algorithm may cause INIT state zero to be
       mapped to another state number.  This cause isomorph to fail.
       For now map state zero be at zero always. */
    if (mapState[0] != 0) {
       cell = mapState[0];
       mapState[0] = 0;
       for (i=1; i < n; i++) {
         if (mapState[i] == 0) {
           mapState[i] = cell;
         } else if (mapState[i] == cell) {
           mapState[i] = 0;
         }
       }
    }

    /* Recode state */
    mapState2 = (INT_S*) MALLOC(n*sizeof(INT_S));
    if (mapState == NULL) {
       mem_result = 1;
       goto FREEMEM;
    }
    memset(mapState2, 0xff, sizeof(INT_S) * n);

    s = 0;
    for (i=0; i < n; i++) {
       if (mapState2[mapState[i]] == -1L) {
          mapState2[mapState[i]] = s;
          mapState[i] = s;
          s++;
       } else {
          mapState[i] = mapState2[mapState[i]];
       }
    }

    /* Do some checking */
    for (i=0; i < n; i++) {
       s = mapState[i];
       if (s >= 0 && s < s2) {
       } else {
          assert(s > 0);
       }
    }

//RECODE:
    recode_min(*s1,*t1,s2,t2,mapState);
    freedes(*s1, t1);
    *s1 = s2;
    *t1 = t2;

FREEMEM:
   if (mapState != NULL) free(mapState);
   if (mapState2 != NULL) free(mapState2);
   if (event != NULL) free(event);
   freedes(n, &inv_t1);
   if (statenext != NULL) free(statenext);
   if (statelast != NULL) free(statelast);
   if (tr_next != NULL) free(tr_next);
   if (tr_last != NULL) free(tr_last);
   if (split != NULL) {
	   for (i=0; i < 2*n; i++) {
		   if (split[i].numelts != 0)
			   free(split[i].next);
	   }
   }
   if (split != NULL) free(split);
   if (block != NULL) free(block);
   if (numinblock != NULL) free(numinblock);
   if (numinsplit != NULL) free(numinsplit);
   if (blocksplit != NULL) free(blocksplit);
   if (numtran != NULL) free(numtran);
   if (num_invtran != NULL) free(num_invtran);
   if (onblocklist_tr != NULL) free(onblocklist_tr);
   if (n__tr != NULL) free(n__tr);
   if (n__invtr != NULL) free(n__invtr);
   if (ptr_blocklist_tr != NULL) {
	   for (j=0; j < s_event; j++) {
		   if (ptr_blocklist_tr[j].numelts != 0)
			   free(ptr_blocklist_tr[j].next);
	   }
	   free(ptr_blocklist_tr);
   }

   if (ptr_splitblocks.next != NULL) free(ptr_splitblocks.next);
   // if (part_array != NULL) free(part_array);
}

void loc_refinement(INT_S*s1, state_node**t1, INT_S s_par, part_node *par)
{
   INT_S i, s;                /* Counter */
   INT_T j, t;                /* Counter */
   INT_B  ok;                /* Boolean flag */
   INT_S ii, jj;              /* Tempory variables */
   INT_T ee, pos;
   state_node *inv_t1;        /* Inverse t1 */
   INT_S freeblock;
/*   INT_S xx, yy; */
   INT_S state, laststate, k, bsplit, cell;
   INT_S *mapState, *mapState2;
   INT_S s2;
   state_node *t2;
   // part_state *part_array;  INT_S s_part_array;
   INT_S block_id;
 //  INT_B  any_diff, equal; /* contain_flag, other_flag */
/*   INT_S which_block; */

   inv_t1 = NULL;

   statenext = statelast = NULL;
   tr_next = tr_last = NULL;
   split = NULL;
   block = numinblock = numinsplit = NULL;
   numtran = num_invtran = NULL;
   onblocklist_tr = NULL;
   n__tr = n__invtr = NULL;
   ptr_blocklist_tr = NULL;
   ptr_splitblocks.next = NULL; ptr_splitblocks.numelts = 0;
   mapState = NULL; mapState2 = NULL;
   t2 = NULL;
   // part_array = NULL; s_part_array = 0;

   event = NULL; s_event = 0;

   /* Need at least 2 states to minimize */
   if (*s1 <= 1) return;

   n = *s1;

   /* Step 0a.
    *   Construct a list of all unique transition labels.
    */

   for (s=0; s < n; s++) {
      for (j=0; j < (*t1)[s].numelts; j++) {
         addordlist( (*t1)[s].next[j].data1, &event, s_event, &ok);
         if (ok) s_event++;
      }
   }

   /* Step 0b.
    *   Allocate memory for all non-dynamic data structures
    */

   inv_t1 = newdes(n);
   if (inv_t1 == NULL) {
      mem_result = 1;
      goto FREEMEM;
   }

   /* Double link-list for B(i) */
   statenext = (INT_S*) MALLOC(sizeof(INT_S) * 2 * n);
   statelast = (INT_S*) MALLOC(sizeof(INT_S) * 2 * n);
   if ( (statenext == NULL) || (statelast == NULL) ) {
      mem_result = 1;
      goto FREEMEM;
   }

   /* Double link list for B(B(i),a) */
/*   tr_next = (INT_S*) malloc(sizeof(INT_S) * 2 * n * s_event);
   tr_last = (INT_S*) malloc(sizeof(INT_S) * 2 * n * s_event);
   if ( (tr_next == NULL) || (tr_last == NULL) ) {
      mem_result = 1;
      goto FREEMEM;
   }
*/
   /* Fill state_next, state_last, tr_next, tr_last with -1 */
   memset(statenext, 0xff, sizeof(INT_S) * 2 * n);
   memset(statelast, 0xff, sizeof(INT_S) * 2 * n);
/*   memset(tr_next,   0xff, sizeof(INT_S) * 2 * n * s_event);
   memset(tr_last,   0xff, sizeof(INT_S) * 2 * n * s_event);
*/
   split = (t_list*) MALLOC(sizeof(t_list) * 2 * n);
   if (split == NULL) {
      mem_result = 1;
      goto FREEMEM;
   }
   memset(split, 0x00, sizeof(t_list) * 2 * n);

   block      = (INT_S*) MALLOC(sizeof(INT_S) * n);
   numinblock = (INT_S*) MALLOC(sizeof(INT_S) * n);
   numinsplit = (INT_S*) MALLOC(sizeof(INT_S) * n);
   if ( (block == NULL) || (numinblock == NULL) || (numinsplit == NULL) ) {
      mem_result = 1;
      goto FREEMEM;
   }

   memset(block,      0x00, sizeof(INT_S) * n);
   memset(numinblock, 0x00, sizeof(INT_S) * n);
   memset(numinsplit, 0x00, sizeof(INT_S) * n);

   blocksplit = (INT_S*) MALLOC(sizeof(INT_S) * n);
   if (blocksplit == NULL) {
      mem_result = 1;
      goto FREEMEM;
   }
   memset(blocksplit, 0x00, sizeof(INT_S) * n);

   numtran = (INT_S*) MALLOC(sizeof(INT_S) * n * s_event);
   if (numtran == NULL) {
      mem_result = 1;
      goto FREEMEM;
   }
   memset(numtran, 0x00, sizeof(INT_S) * n * s_event);

   num_invtran = (INT_S*) MALLOC(sizeof(INT_S) * n * s_event);
   if (num_invtran == NULL) {
	   mem_result = 1;
	   goto FREEMEM;
   }
   memset(num_invtran, 0x00, sizeof(INT_S) * n * s_event);

   onblocklist_tr = (INT_S*) MALLOC(sizeof(INT_S) * n * s_event);
   if (onblocklist_tr == NULL) {
      mem_result = 1;
      goto FREEMEM;
   }
   memset(onblocklist_tr, 0x00, sizeof(INT_S) * n * s_event);

   n__tr = (INT_S*) MALLOC(sizeof(INT_S) * n * s_event);
   if (n__tr == NULL) {
      mem_result = 1;
      goto FREEMEM;
   }
   memset(n__tr, 0x00, sizeof(INT_S) * n * s_event);

   n__invtr = (INT_S*) MALLOC(sizeof(INT_S) * n * s_event);
   if (n__invtr == NULL) {
	   mem_result = 1;
	   goto FREEMEM;
   }
   memset(n__invtr, 0x00, sizeof(INT_S) * n * s_event);

   ptr_blocklist_tr = (t_list*) CALLOC(s_event, sizeof(t_list) * s_event);
   if (ptr_blocklist_tr == NULL) {
      mem_result = 1;
      goto FREEMEM;
   }

   mapState = (INT_S*) CALLOC(*s1, sizeof(INT_S));
   if (mapState == NULL) {
      mem_result = 1;
      goto FREEMEM;
   }

   /* Step 1.
    *   For each state 's' in 'S' and each transition 'a' in 'I'
    *   construct the inverse transition table.
    *   Also, fill in 'n__tr'.
    *   Also, map the "events" so they are sequence from 0 to s_event-1.
    */

   for (s=0; s < n; s++) {
      for (j=0; j < (*t1)[s].numelts; j++) {
         /* [ii,ee,jj] -> [jj,ee,ii] */
         ii = s;
         ee = (*t1)[s].next[j].data1;
         jj = (*t1)[s].next[j].data2;

         /* Translate 'ee' into a position to 'n__tr' */
         pos = findpos(ee, event, s_event);

         /* Do the mapping to the new transition names */
         ee = pos;
         (*t1)[s].next[j].data1 = pos;

         addordlist1(ee, ii, &inv_t1[jj].next, inv_t1[jj].numelts, &ok);
         if (ok) inv_t1[jj].numelts++;

         n__tr[(n*pos)+s]++;
		 n__invtr[(n*pos)+jj]++;
      }
   }
/*   for(i = 0; i < sn; i ++){
	   if(tn[i].numelts > 0){
		ee = tn[i].next[0].data1;
		pos = findpos(ee,event,s_event);
		break;
	   }
   }*/
   //Initialize the partition
   for(i = 0; i < s_par; i ++){
	   block_id = i;
	   for(s = 0; s < par[i].numelts; s ++){
		   ii = par[i].next[s];
		   insertstate(block_id,ii);
		   for (j=0; j < s_event; j++) {
			   if ( inlist2(j, &(*t1)[ii].next, (*t1)[ii].numelts) ) {
				   numtran[(n*j)+block_id] += n__tr[(n*j)+ii];
			   }
			   if ( inlist2(j, &(inv_t1)[ii].next, (inv_t1)[ii].numelts) ) {
				   num_invtran[(n*j)+block_id] += n__invtr[(n*j)+ii];
			   }
		   }

		   block[ii] = block_id;
		   numinblock[block_id]++;
	   }
   }

 /*  for (i=0; i < *s1; i++) {


	   // Add to B(block_id) list
	   insertstate(block_id,i);
	   for (j=0; j < s_event; j++) {
		   if ( inlist2(j, &(*t1)[i].next, (*t1)[i].numelts) ) {
			   numtran[(n*j)+block_id] += n__tr[(n*j)+i];
		   }
		   if ( inlist2(j, &(inv_t1)[i].next, (inv_t1)[i].numelts) ) {
			   num_invtran[(n*j)+block_id] += n__invtr[(n*j)+i];
		   }
	   }
	   block[i] = block_id;
	   numinblock[block_id]++;
   }*/

 /*  if((numinblock[0] == *s1) || (numinblock[0] == 0)){
	   goto FINISH;
   }*/

   freeblock = 2;
   //Initialize the splitter
 /*  if(numinblock[0] <= numinblock[1])
	   block_id = 0;
   else
	   block_id = 1;
   for (j=0; j < s_event; j++) {
	   if(num_invtran[n*j+block_id] >0){
			addstatelist(block_id, &(ptr_blocklist_tr[j].next),
                                  ptr_blocklist_tr[j].numelts, &ok);
			if (ok) ptr_blocklist_tr[j].numelts++;
			onblocklist_tr[n*j+block_id] = 1;
	   }
   }*/

   for (j=0; j < s_event; j++) {
      for (i=0; i < s_par; i++) {
         block_id = i;
         addstatelist(block_id, &(ptr_blocklist_tr[j].next),
                                  ptr_blocklist_tr[j].numelts, &ok);
         if (ok) ptr_blocklist_tr[j].numelts++;
         onblocklist_tr[n*j+block_id] = 1;
      }
   }

REPEAT:
   /* Step 5
    *   Select 'a' in 'I' and 'i' in L(a).  The algorithm
    *   terminates when L(a) = empty for each 'a' in 'I'.
    */

    for (t=0; t < s_event; t++) {
       if (ptr_blocklist_tr[t].numelts != 0) {
           jj = ptr_blocklist_tr[t].next[0];

           /* Step 6
            * Delete 'i' from L(a)
            */
           remove_first(&ptr_blocklist_tr[t]);
           onblocklist_tr[(n*t)+jj] = 0;
           goto DIV;
       }
    }
    goto FINISH;
DIV:
    /* Step 7 */
/*    state = tr_next[(2*n*t)+jj]; */

    state = statenext[jj];

    while (state != -1) {
       /* t = transition */
       /* state */

       s = state-n;

       for (ee=0; ee < inv_t1[s].numelts; ee++) {
          if (inv_t1[s].next[ee].data1 == t) {
             laststate = inv_t1[s].next[ee].data2;
             k = block[laststate];
			 if(numtran[n*t+k] < 2)  // First condition specific to loc algorithm
				 continue;
             addstatelist(laststate, &(split[k].next), split[k].numelts, &ok);
             if (ok) split[k].numelts++;

             if (blocksplit[k] == 0) {
                addstatelist(k, &(ptr_splitblocks.next),
                                 ptr_splitblocks.numelts, &ok);
                if (ok) ptr_splitblocks.numelts++;
             }
             numinsplit[k]++;
             blocksplit[k] = 1;
          }
       }
       state = statenext[state];
    }
RETURN:
    // Refine block
    if (ptr_splitblocks.numelts == 0) goto REPEAT;
    bsplit = ptr_splitblocks.next[0];
    remove_first(&ptr_splitblocks);
    blocksplit[bsplit] = 0;



    if (numtran[n*t+bsplit] == numinsplit[bsplit]) { //Second condition specific to loc algorithm
       numinsplit[bsplit] = 0;
       if (split[bsplit].next != NULL)
          free(split[bsplit].next);
       split[bsplit].next = NULL;
       split[bsplit].numelts = 0;
       goto RETURN;
    }


    numinblock[bsplit] -= numinsplit[bsplit];
    numinblock[freeblock] = numinsplit[bsplit];
    numinsplit[bsplit] = 0;
    while(split[bsplit].numelts != 0) {
       state = split[bsplit].next[0];
       remove_first(&split[bsplit]);

       statenext[statelast[n+state]] = statenext[n+state];
       if (statenext[n+state] != -1)
           statelast[statenext[n+state]] = statelast[n+state];
       insertstate(freeblock,state);
       block[state] = freeblock;

       for (ee=0; ee < s_event; ee++) {
          if (inlist2(ee, &(*t1)[state].next, (*t1)[state].numelts)) {
            numtran[n*ee+bsplit] -= n__tr[n*ee+state];
            numtran[n*ee+freeblock] += n__tr[n*ee+state];
          }
		  if (inlist2(ee, &(inv_t1)[state].next, (inv_t1)[state].numelts)) {
			  num_invtran[n*ee+bsplit] -= n__invtr[n*ee+state];
			  num_invtran[n*ee+freeblock] += n__invtr[n*ee+state];
		  }
       }
    }
	//zprint_list(n*s_event, numtran);

    // Add new block to blocklist
    for (j=0; j < s_event; j++) {
       if ( (onblocklist_tr[n*j+bsplit] != 1) && (num_invtran[n*j+bsplit] > 0)
            && (numinblock[bsplit] <= numinblock[freeblock]))
       {
          addstatelist(bsplit, &(ptr_blocklist_tr[j].next),
                                ptr_blocklist_tr[j].numelts, &ok);
          if (ok) ptr_blocklist_tr[j].numelts++;
          onblocklist_tr[n*j+bsplit] = 1;
       } else if ((num_invtran[n*j+ freeblock] > 0)){
          addstatelist(freeblock, &(ptr_blocklist_tr[j].next),
                                   ptr_blocklist_tr[j].numelts, &ok);
          if (ok) ptr_blocklist_tr[j].numelts++;
          onblocklist_tr[n*j+freeblock] = 1;
       }
    }

    freeblock++;

    if (ptr_splitblocks.numelts == 0) goto REPEAT;
    goto RETURN;

FINISH:
    for (s=0; s < n; s++) {
      for (j=0; j < (*t1)[s].numelts; j++) {
         ee = (*t1)[s].next[j].data1;
         (*t1)[s].next[j].data1 = event[ee];
      }
    }

    if (freeblock == n) {
       /* No change */
       goto FREEMEM;
    }

    for (i=0; i < n; i++) {
       cell = statenext[i];
       while (cell != -1) {
          s = cell-n;
          mapState[s] = i;
          cell = statenext[cell];
       }
    }

    freedes(n, &inv_t1);
    inv_t1 = NULL;

    s2 = freeblock;
    t2 = newdes(s2);
    if ( t2 == NULL) {
       mem_result = 1;
       goto FREEMEM;
    }

    /* This minimization algorithm may cause INIT state zero to be
       mapped to another state number.  This cause isomorph to fail.
       For now map state zero be at zero always. */
    if (mapState[0] != 0) {
       cell = mapState[0];
       mapState[0] = 0;
       for (i=1; i < n; i++) {
         if (mapState[i] == 0) {
           mapState[i] = cell;
         } else if (mapState[i] == cell) {
           mapState[i] = 0;
         }
       }
    }

    /* Recode state */
    mapState2 = (INT_S*) MALLOC(n*sizeof(INT_S));
    if (mapState == NULL) {
       mem_result = 1;
       goto FREEMEM;
    }
    memset(mapState2, 0xff, sizeof(INT_S) * n);

    s = 0;
    for (i=0; i < n; i++) {
       if (mapState2[mapState[i]] == -1L) {
          mapState2[mapState[i]] = s;
          mapState[i] = s;
          s++;
       } else {
          mapState[i] = mapState2[mapState[i]];
       }
    }

    /* Do some checking */
    for (i=0; i < n; i++) {
       s = mapState[i];
       if (s >= 0 && s < s2) {
       } else {
          assert(s > 0);
       }
    }

//RECODE:
    recode_min(*s1,*t1,s2,t2,mapState);
    freedes(*s1, t1);
    *s1 = s2;
    *t1 = t2;

FREEMEM:
   if (mapState != NULL) free(mapState);
   if (mapState2 != NULL) free(mapState2);
   if (event != NULL) free(event);
   freedes(n, &inv_t1);
   if (statenext != NULL) free(statenext);
   if (statelast != NULL) free(statelast);
   if (tr_next != NULL) free(tr_next);
   if (tr_last != NULL) free(tr_last);
   if (split != NULL) {
	   for (i=0; i < 2*n; i++) {
		   if (split[i].numelts != 0)
			   free(split[i].next);
	   }
   }
   if (split != NULL) free(split);
   if (block != NULL) free(block);
   if (numinblock != NULL) free(numinblock);
   if (numinsplit != NULL) free(numinsplit);
   if (blocksplit != NULL) free(blocksplit);
   if (numtran != NULL) free(numtran);
   if (num_invtran != NULL) free(num_invtran);
   if (onblocklist_tr != NULL) free(onblocklist_tr);
   if (n__tr != NULL) free(n__tr);
   if (n__invtr != NULL) free(n__invtr);
   if (ptr_blocklist_tr != NULL) {
	   for (j=0; j < s_event; j++) {
		   if (ptr_blocklist_tr[j].numelts != 0)
			   free(ptr_blocklist_tr[j].next);
	   }
	   free(ptr_blocklist_tr);
   }

   if (ptr_splitblocks.next != NULL) free(ptr_splitblocks.next);
   // if (part_array != NULL) free(part_array);
}

#ifdef __cplusplus
}
#endif
