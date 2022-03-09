#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "des_proc.h"
#include "higen.h"
#include "des_supp.h"
#include "setup.h"
// #include "curses.h"
#include "mymalloc.h"

#ifdef __cplusplus
extern "C" {
#endif

/* This procedure will convert a non-deterministic DES into
   a semi-nondeterministic DES file the hard way. */
void fix_semi_deter(state_node **t2, INT_S *s2,
                    INT_B  higen_flag,
                    INT_T **list, INT_T *s_list,
                    INT_B  *ok)
{
   INT_S i, jj;
   INT_T j, k, new_tran_num, ee;
   INT_B  ok2, found;
   INT_T prev_tran;

   //if (prev_tran) {} /* Remove warning */

   INT_T *unused, s_unused;

   unused = NULL; s_unused = 0;
   new_tran_num = 0;

   /* First make a list of transitons */
   for (i=0; i < *s2; i++) {
      for (j=0; j < (*t2)[i].numelts; j++) {
        addordlist((*t2)[i].next[j].data1, &unused, s_unused, &ok2);
        if (ok2) s_unused++;
      }
   }

   for (i=0; i < *s2; i++) {
      if ( (*t2)[i].numelts > 0)
          prev_tran = (*t2)[i].next[0].data1;

      // if (debug_mode) {
      //    move(23,0); clrtoeol();
      //    printw("FIX_SEMI: %d %d", i, *s2);
      //    refresh();
      // }

      new_tran_num = 0;
      for (j=1; j < (*t2)[i].numelts; j++) {
          ee = (*t2)[i].next[j].data1;
          jj = (*t2)[i].next[j].data2;

          /* Check for non-determinisism of event labels */
          if ((higen_flag) && (ee == 0)) {
             prev_tran = ee;
          } else if (ee == prev_tran) {
             /* Create new state */
             (*t2) = (state_node*) REALLOC(*t2, sizeof(state_node)*(*s2+1));
             memset(&(*t2)[*s2], 0, sizeof(state_node));

             (*t2)[*s2].marked  = (*t2)[i].marked;
             (*t2)[*s2].numelts = 0;
             (*t2)[*s2].next = NULL;
             (*t2)[*s2].vocal = 0;

             /* Copy most transitions from "i" to new state */
             for (k=0; k < (*t2)[i].numelts; k++) {
                if ((*t2)[i].next[k].data1 != prev_tran) {
                    addordlist1((*t2)[i].next[k].data1,
                                (*t2)[i].next[k].data2,
                                &(*t2)[*s2].next, (*t2)[*s2].numelts, &ok2);
                    if (ok2) (*t2)[*s2].numelts++;
                }
             }

             /* Find an unused transition */
             found = false;
             do {
                if (inlist(new_tran_num, unused, s_unused)) {
                  new_tran_num++;
                } else {
                  found = true;
                }
             } while (!found);

             /* Add new unique transition to new state */
             addordlist1(new_tran_num, *s2, &(*t2)[i].next,
                         (*t2)[i].numelts, &ok2);
             if (ok2) (*t2)[i].numelts++;

             /* Add new transition to nullist */
             addordlist(new_tran_num, list, *s_list, &ok2);
             if (ok2) (*s_list)++;
             new_tran_num++;

             if (new_tran_num > 1022) {
                printf("Excessed maximum - fix semi-deterministism has fail\n");
                exit(1);
             }

             /* New transition from the newstate to the entrance_state */
             addordlist1(prev_tran, jj, &(*t2)[*s2].next,
                          (*t2)[*s2].numelts, &ok2);
             if (ok2) (*t2)[*s2].numelts++;

             /* Delete old transition */
             delete_ordlist1(ee, jj, &(*t2)[i].next,
                             (*t2)[i].numelts, &ok2);
             if (ok2) (*t2)[i].numelts--;
             (*s2)++;
             j--;
          } else {
             prev_tran = ee;
          }
      }
   }
   *ok = true;

   free(unused);
}

void eventmap_des(state_node *t1,  INT_S s1,
                  state_node **t2, INT_S *s2,
                  state_pair *sp,  INT_S s_sp,
                  INT_T **list,    INT_T *s_list,
                  INT_B  *ok)
{
   /* Map the old_label to the new_label
      Check if the resulting DES is deterministic
      If the new DES is non-deterministic, make it deterministic.
        ie. Add new states and events to make it non-deterministics */

   INT_B  ok2;
   INT_S i,j;
   INT_T k;
   INT_T e;
   INT_S s;

   *ok = true;
   for (i=0; i < s1; i++) {
       j = 0;
       k = 0;
       while ( (j < t1[i].numelts) && (k < s_sp) ) {
          if (t1[i].next[j].data1 == sp[k].data1) {
             t1[i].next[j].data1 = (INT_T)sp[k].data2;
             j++;
          } else if (t1[i].next[j].data1 < sp[k].data1) {
             j++;
          } else {
             k++;
          }
       }
   }

   /* The new transitions maybe unsorted, so copy and re-sort */
   *s2 = s1;
   *t2 = newdes(s1);
   if (*t2 == NULL) {
      *ok = false;
      mem_result = 1;
      return;
   }

   for (i=0; i < s1; i++) {
      (*t2)[i].marked = t1[i].marked;
      (*t2)[i].vocal  = t1[i].vocal;

      for (j=0; j < t1[i].numelts; j++) {
        e = t1[i].next[j].data1;
        s = t1[i].next[j].data2;
        addordlist1(e, s, &(*t2)[i].next, (*t2)[i].numelts, &ok2);
        if (ok2) (*t2)[i].numelts++;
      }
   }

   if (!checkdet(*s2,*t2)) {
      fix_semi_deter(t2,s2,false,list,s_list,ok);
   }
}

INT_B  mark_higen(state_node *t, INT_S s, INT_S i)
{
   /* Determine if the state should be marked */
   INT_S ii, jj;
   INT_T ee;
   INT_B  markable, ok;
   t_stack ts;

   if (t[i].marked) {
      return true;
   }

   if ( (t[i].vocal == 0) && (i != 0) ) {
      return false;
   }

   /* Initialize the "reach" variable to false */
   for (ii=1; ii < s; ii++)
      t[ii].reached = false;
   if (s > 0)
      t[0].reached = true;

   pstack_Init(&ts);

   markable = false;
   ii = i;
   ee = 0;

   do {
      if (ee < t[ii].numelts) {
         jj = t[ii].next[ee].data2;
         if (t[jj].reached) {
            if ( (t[jj].vocal == 0) && (t[jj].marked) ) {
               markable = true;
            } else if (t[jj].vocal == 0) {
               pstack_Push(&ts, ee, ii);
               t[jj].reached = true;
               ii = jj;
               ee = 0;
            } else {
               ee++;
            }
         } else {
            ee++;
         }
      } else {
         if (! pstack_IsEmpty(&ts)) {
           pstack_Pop(&ts, &ee, &ii, &ok);
           ee++;
         }
      }
   } while (((ee < t[ii].numelts) || !pstack_IsEmpty(&ts)) && (markable != true));

   pstack_Done(&ts);

   return markable;
}

INT_B  remap_events(state_node *t1, INT_S s1)
{
    INT_S i, k;
    INT_T j, ee;
    INT_T *vocal_list, s_vocal_list;
    INT_T *event_list, s_event_list;
    state_pair *pair; INT_S s_pair;
    INT_B  ok, found;
    INT_T new_event;

    vocal_list = NULL; s_vocal_list = 0;
    event_list = NULL; s_event_list = 0;
    pair = NULL; s_pair = 0;

    /* Relabel events in t1 so that vocal output do not
       map to existing events */

    /* Get list of vocal output */
    for (i=0; i < s1; i++) {
       addordlist( (t1)[i].vocal, &vocal_list, s_vocal_list, &ok);
       if (ok) s_vocal_list++;
    }

    /* Get list of events */
    for (i=0; i < s1; i++) {
       for (j=0; j < (t1)[i].numelts; j++) {
          ee = (t1)[i].next[j].data1;
          addordlist(ee, &event_list, s_event_list, &ok);
          if (ok) s_event_list++;
       }
    }

    s_pair = s_event_list;
    pair = (state_pair*) CALLOC(s_event_list, sizeof(state_pair));
    if ((pair == NULL) && (s_event_list !=0)) {
       free(vocal_list);
       free(event_list);
       mem_result = 0;
       return false;
    }

    new_event = 0;
    for (i=0; i < s_event_list; i++) {
       ee = event_list[i];

       found = false;
       do {
          new_event++;
          if (!inlist(new_event, vocal_list, s_vocal_list)) {
             found = true;
          }
       } while (!found && (new_event < 999));

       if (new_event > 999) {
          printf("Excessed maximum - re-map events has fail\n");
          free(vocal_list);
          free(event_list);
          free(pair);
          exit(1);
          return false;
       }

       pair[i].data1 = ee;
       pair[i].data2 = new_event;
    }

    /* Re-do event lists with new mapping */
    for (i=0; i < s1; i++) {
       for (j=0; j < (t1)[i].numelts; j++) {
          ee = (t1)[i].next[j].data1;
          for (k=0; k < s_pair; k++) {
             if (pair[k].data1 == ee)
                break;
          }
          (t1)[i].next[j].data1 = (INT_T)pair[k].data2;
       }
    }

    free(vocal_list);
    free(event_list);
    free(pair);

    return true;
}

void higen_des(state_node** t1, INT_S* s1, INT_T **nullist, INT_T *s_nullist)
{
    /* higen_des generates the high-level structure from the
       low-level structure.

    higen_des generates the high-level transition structure from the
    low-level transition structure by replacing transition [I,E,J] by
    [I,X,J] where X is the output at state J. */

    state_node *t2; INT_S s2;
    INT_S i, jj;
    INT_T j, ee;
    INT_B  ok;

    if (*s1 <= 0) return;

    s2 = *s1;
    t2 = newdes(s2);
    if (t2 == NULL) {
        mem_result = 1;
        return;
    }

    remap_events(*t1, *s1);

    for (i=0; i < *s1; i++) {
       for (j=0; j < (*t1)[i].numelts; j++) {
          jj = (*t1)[i].next[j].data2;
          ee = (*t1)[jj].vocal;
          addordlist1(ee, jj, &t2[i].next, t2[i].numelts, &ok);
          if (ok) t2[i].numelts++;
       }
    }

    /* Mark vocal states if it can reach a silent marked state */
    if (*s1 > 0) {
       t2[0].marked = (*t1)[0].marked;
    }

    for (i=1; i < *s1; i++) {
       if (mark_higen(*t1, *s1, i))
          t2[i].marked = true;
    }

    *nullist = NULL; *s_nullist = 0;
    addordlist(0, nullist, *s_nullist, &ok);
    if (ok) (*s_nullist)++;

    i = count_tran(t2, s2);

    if (!checkdet(s2,t2)) {
       fix_semi_deter(&t2, &s2, true, nullist, s_nullist, &ok);
    }

    i = count_tran(t2, s2);

    freedes(*s1, t1);
    *s1 = s2;
    *t1 = t2;
}

#ifdef __cplusplus
}
#endif

