/* Old minimization using n^3 algorithm.
   However, this function uses less memory than the
   (n log n) algorithm.
 */

#include "minm1.h"
#include "des_supp.h"
#include "mymalloc.h"
#include "setup.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

void compare_tran2(INT_S i, INT_S j, state_node *t1, state_node *t2,
                   boolean *equal, INT_S *mapState, state_pair **iEj,
                   INT_S *s_iEj) {
  INT_T a, b;
  INT_B ok;
  INT_S entr1, entr2;
  t_stack ts;

  pstack_Init(&ts);

  a = 0;
  b = 0;

  for (;;) {
  LABEL1:
    if (a < t1[i].numelts) {
      if (t1[i].next[a].data1 != t2[j].next[b].data1)
        *equal = false;
      else {
        addstatepair(i, j, iEj, *s_iEj, &ok);
        if (ok)
          (*s_iEj)++;
        entr1 = t1[i].next[a].data2;
        entr2 = t2[j].next[b].data2;
        *equal = (entr1 == entr2) && (mapState[entr1] == mapState[entr2]);
        if (!(*equal)) {
          *equal = instatepair(entr1, entr2, iEj, *s_iEj);
        }
        if (!(*equal)) {
          /*         if ((entr1 !=i) || ((entr1==j) && (entr2 > j)) ) { */
          addstatepair(i, j, iEj, *s_iEj, &ok);
          if (ok)
            (*s_iEj)++;
          *equal = (t1[entr1].marked == t2[entr2].marked) &&
                   (t1[entr1].vocal == t2[entr2].vocal) &&
                   (t1[entr1].numelts == t2[entr2].numelts);
          if (*equal) {
            ok = pstack_Push(&ts, a, i);
            ok = pstack_Push(&ts, b, j);
            i = entr1;
            j = entr2;
            a = 0;
            b = 0;
            goto LABEL1;
          }
          /*         } */
        }
      LABEL2:
        if (*equal) {
          a++;
          b++;
          goto LABEL1;
        }
      }
    } else {
      if (b < t2[j].numelts)
        *equal = false;
      else
        *equal = true;
    }

    if (pstack_IsEmpty(&ts)) {
      break;
    } else {
      pstack_Pop(&ts, &b, &j, &ok);
      pstack_Pop(&ts, &a, &i, &ok);
      goto LABEL2;
    }

  } /*forever*/
  pstack_Done(&ts);
}

void compare_states2(INT_S i, INT_S j, boolean *equal, INT_S *mapState,
                     state_pair **iEj, INT_S *s_iEj, state_node *t1,
                     state_node *t2) {
  boolean bothmarked, marktest;

  bothmarked = (t1[i].marked == t2[j].marked);
  marktest = bothmarked && (t1[i].numelts == 0) && (t2[j].numelts == 0) &&
             (t1[i].vocal == t2[j].vocal);
  if (marktest)
    *equal = true;
  if (!marktest) {
    if (bothmarked && (t1[i].numelts == t2[j].numelts) &&
        t1[i].vocal == t2[j].vocal)
      compare_tran2(i, j, t1, t2, equal, mapState, iEj, s_iEj);
    else
      *equal = false;
  }
}

void addrow(INT_S i, part_node *pn) {
  pn[i].next = (INT_S *)MALLOC(sizeof(INT_S));
  if (pn[i].next == NULL) {
    mem_result = 1;
    return;
  }
  pn[i].numelts = 1;
  pn[i].next[0] = i;
}

void addcolumn(INT_S i, INT_S j, part_node *pn) {
  INT_S pos;

  if (pn[i].numelts == 0)
    return; /* State does not exist */

  /* Append to the end */
  pos = pn[i].numelts;
  pn[i].numelts++;
  pn[i].next = (INT_S *)REALLOC(pn[i].next, sizeof(INT_S) * pn[i].numelts);
  pn[i].next[pos] = j;
}

void fix_partition(INT_S s1, INT_S *mapState, part_node *pn) {
  INT_S kk, mm, nn;
  INT_S row_index, col_index;

  for (kk = 0; kk < 2; kk++) {
    for (mm = 0; mm < s1; mm++) {
      if (pn[mm].numelts > 0) {
        row_index = mm;
        for (nn = 0; nn < pn[mm].numelts; nn++) {
          col_index = pn[mm].next[nn];
          if (mapState[col_index] > row_index)
            mapState[col_index] = row_index;
          if (mapState[row_index] > mapState[col_index])
            mapState[row_index] = mapState[col_index];
        }
      }
    }
  }
}

void build_partition(INT_S s1, state_node *t1, INT_S *mapState, INT_S *s2) {
  INT_S i, j, state;
  part_node *pn;
  INT_S *ord_states;
  INT_S classnum;
  boolean equal_class;
  state_pair *iEj;
  INT_S s_iEj;

  *s2 = s1;
  pn = NULL;
  iEj = NULL;

  pn = (part_node *)CALLOC(s1, sizeof(part_node));
  if (pn == NULL) {
    mem_result = 1;
    return;
  }

  ord_states = (INT_S *)CALLOC(s1, sizeof(INT_S));
  if (ord_states == NULL) {
    free(pn);
    mem_result = 1;
    return;
  }

  s_iEj = 0;

  for (i = 0; i < s1; i++) {
    mapState[i] = i;
    ord_states[i] = i;
  }

  classnum = -1;
  for (state = 0; state < s1; state++) {
    i = ord_states[state];
    if (i != -1) {
      addrow(i, pn);
      ord_states[i] = -1;
      classnum++;
      mapState[i] = classnum;
      for (j = i + 1; j < s1; j++) {
        if (t1[i].numelts != t1[j].numelts)
          continue;
        if (iEj != NULL) {
          free(iEj);
          iEj = NULL;
          s_iEj = 0;
        }
        equal_class = false;
        compare_states2(i, j, &equal_class, mapState, &iEj, &s_iEj, t1, t1);
        if (equal_class) {
          mapState[j] = mapState[i];
          addcolumn(i, j, pn);
          ord_states[j] = -1;
          (*s2)--;
        }
      }
    }
  }
  fix_partition(s1, mapState, pn);

  free(iEj);
  free(ord_states);
  free_part(s1, &pn);
}

void minimize1(INT_S *s1, state_node **t1) {
  INT_S s2;
  state_node *t2;
  INT_S *mapState;
  //   INT_S i;
  //   FILE *f1;

  if (*s1 == 0)
    return;

  mapState = (INT_S *)CALLOC(*s1, sizeof(INT_S));
  if (mapState == NULL) {
    mem_result = 1;
    return;
  }

  build_partition(*s1, *t1, mapState, &s2);

  if (*s1 == s2) {
    free(mapState);
    return; /* No change */
  }

  t2 = newdes(s2);
  if ((s2 != 0) && (t2 == NULL)) {
    mem_result = 1;
    free(mapState);
    return;
  }

  recode_min(*s1, *t1, s2, t2, mapState);

  free(mapState);
  freedes(*s1, t1);
  *s1 = s2;
  *t1 = t2;
}

#ifdef __cplusplus
}
#endif
