#include "des_proc.h"
#include "des_data.h"
#include "des_supp.h"

#include "minm.h"
#include "minm1.h"
#include "mymalloc.h"
#include "setup.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DES procedures */

void gentranlist(INT_S s1, state_node *t1, INT_T *s_t1, INT_T **list1) {
  /* Generate a list of all transition labels used in DES */
  INT_S i;
  INT_T j;
  INT_B ok;

  *s_t1 = 0;
  *list1 = NULL;
  if (s1 == 0L)
    return;

  for (i = 0L; i < s1; i++) {
    for (j = 0; j < t1[i].numelts; j++) {
      addordlist(t1[i].next[j].data1, list1, *s_t1, &ok);
      if (ok)
        (*s_t1)++;
    }
  }
}

void gensublist(INT_T s_t1, INT_T *tranlist1, INT_T s_t2, INT_T *tranlist2,
                INT_T *s_s1, INT_T **sublist1, INT_T *s_s2, INT_T **sublist2) {
  /* This generates sublist1 (list1-list2) and sublist2 (list2-list1) */
  INT_T i, j;
  INT_B ok;

  *sublist1 = *sublist2 = NULL;
  i = j = 0;
  while ((i < s_t1) && (j < s_t2)) {
    if (tranlist1[i] == tranlist2[j]) {
      i++;
      j++;
      continue;
    }
    if (tranlist1[i] > tranlist2[j]) {
      addordlist(tranlist2[j], sublist2, *s_s2, &ok);
      if (ok)
        (*s_s2)++;
      j++;
    } else {
      addordlist(tranlist1[i], sublist1, *s_s1, &ok);
      if (ok)
        (*s_s1)++;
      i++;
    }
  }

  if (i >= s_t1) {
    if (*sublist2 == NULL) {
      while (j < s_t2) {
        addordlist(tranlist2[j], sublist2, *s_s2, &ok);
        if (ok)
          (*s_s2)++;
        j++;
      }
      return;
    }

    while (j < s_t2) {
      addordlist(tranlist2[j], sublist2, *s_s2, &ok);
      if (ok)
        (*s_s2)++;
      j++;
    }
    return;
  }

  if (*sublist1 == NULL) {
    while (i < s_t1) {
      addordlist(tranlist1[i], sublist1, *s_s1, &ok);
      if (ok)
        (*s_s1)++;
      i++;
    }
    return;
  }
  while (i < s_t1) {
    addordlist(tranlist1[i], sublist1, *s_s1, &ok);
    if (ok)
      (*s_s1)++;
    i++;
  }
}

void gentran(INT_T s_list, INT_T *list, INT_S s, state_node *t) {
  /* Generate transitions by self-looping with labels in list */
  INT_S i;
  INT_T j;
  INT_B ok;

  for (i = 0L; i < s; i++) {
    for (j = 0; j < s_list; j++) {
      addordlist1(list[j], i, &t[i].next, t[i].numelts, &ok);
      if (ok)
        t[i].numelts++;
    }
  }
}

void selfloop_gentran(INT_T s_list, INT_T *list, INT_S s, state_node *t) {
  /* Generate transitions by self-looping with labels in list */
  INT_S i;
  INT_T j;
  INT_B ok;

  for (i = 0L; i < s; i++) {
    for (j = 0; j < s_list; j++) {
      if (!inordlist1(list[j], t[i].next, t[i].numelts)) {
        addordlist1(list[j], i, &t[i].next, t[i].numelts, &ok);
        if (ok)
          t[i].numelts++;
      }
    }
  }
}

void meet2(INT_S s1, state_node *t1, INT_S s2, state_node *t2, INT_S *s3,
           state_node **t3, INT_S **macro_ab, INT_S **macro_c) {
  INT_S t1i, t2j;
  INT_T colptr1, colptr2;
  INT_T tran1, tran2;
  INT_S srcstate, newstate, macrostate;
  INT_S a, b, i;

  if (s1 == 0L || s2 == 0L) {
    *s3 = 0;
    *t3 = NULL;
    return;
  }
  *s3 = 0;
  *t3 = NULL;

  *macro_ab = (INT_S *)CALLOC(s1 * s2, sizeof(INT_S));
  *macro_c = (INT_S *)CALLOC(s1 * s2, sizeof(INT_S));

  if ((*macro_ab == NULL) || (*macro_c == NULL) || s1 * s2 < 0) {
    mem_result = 1;
    return;
  }

  for (i = 0; i < s1 * s2; i++) {
    (*macro_ab)[i] = -1L;
    (*macro_c)[i] = -1L;
  }

  (*macro_ab)[0] = (*macro_c)[0] = 0L;
  srcstate = newstate = macrostate = 0L;
  t1i = t2j = 0L;
  do {
    colptr1 = 0;
    colptr2 = 0;
    while (colptr1 < t1[t1i].numelts && colptr2 < t2[t2j].numelts) {
      tran1 = t1[t1i].next[colptr1].data1;
      tran2 = t2[t2j].next[colptr2].data1;
      if (tran1 != tran2) {
        if (tran1 < tran2)
          colptr1++;
        else
          colptr2++;
        continue;
      }

      a = t1[t1i].next[colptr1].data2;
      b = t2[t2j].next[colptr2].data2;
      macrostate = (*macro_ab)[b * s1 + a];
      if (macrostate == -1L) {
        newstate++;
        (*macro_ab)[b * s1 + a] = newstate;
        (*macro_c)[newstate] = b * s1 + a;
        insertlist4(srcstate, tran1, newstate, s3, t3);
        if (mem_result == 1)
          return;
      } else {
        insertlist4(srcstate, tran1, macrostate, s3, t3);
        if (mem_result == 1)
          return;
      }
      colptr1++;
      colptr2++;
    }

    srcstate++;
    a = (*macro_c)[srcstate];
    if (a != -1L) {
      t1i = a % s1;
      t2j = a / s1;
    }

  } while (srcstate <= newstate);

  resize_des(t3, *s3, newstate + 1);
  *s3 = newstate + 1;

  (*t3)[0].reached = true;
  for (i = 0; i < *s3; i++) {
    a = (*macro_c)[i];
    (*t3)[i].marked = t1[a % s1].marked && t2[a / s1].marked;
  }

  /* Should be safe because the size is smaller than before */
  *macro_c = (INT_S *)REALLOC(*macro_c, sizeof(INT_S) * (*s3));
}
/* This differs from "meet2" in that the transitions
   generated are flipped */
void nc_meet2(INT_S s1, state_node *t1, INT_S s2, state_node *t2, INT_S *s3,
              state_node **t3, INT_S **macro_ab, INT_S **macro_c) {
  INT_S t1i, t2j;
  INT_T colptr1, colptr2;
  INT_T tran1, tran2;
  INT_S srcstate, newstate, macrostate;
  INT_S a, b, i;

  if (s1 == 0L || s2 == 0L) {
    *s3 = 0;
    *t3 = NULL;
    return;
  }
  *s3 = 0;
  *t3 = NULL;

  *macro_ab = (INT_S *)MALLOC(sizeof(INT_S) * s1 * s2);
  *macro_c = (INT_S *)MALLOC(sizeof(INT_S) * s1 * s2);

  if ((*macro_ab == NULL) || (*macro_c == NULL) || s1 * s2 < 0) {
    mem_result = 1;
    return;
  }

  for (i = 0; i < s1 * s2; i++) {
    (*macro_ab)[i] = -1L;
    (*macro_c)[i] = -1L;
  }

  (*macro_ab)[0] = (*macro_c)[0] = 0L;
  srcstate = newstate = macrostate = 0L;
  t1i = t2j = 0L;
  do {
    colptr1 = 0;
    colptr2 = 0;
    while (colptr1 < t1[t1i].numelts && colptr2 < t2[t2j].numelts) {
      tran1 = t1[t1i].next[colptr1].data1;
      tran2 = t2[t2j].next[colptr2].data1;
      if (tran1 != tran2) {
        if (tran1 < tran2)
          colptr1++;
        else
          colptr2++;
        continue;
      }

      a = t1[t1i].next[colptr1].data2;
      b = t2[t2j].next[colptr2].data2;
      macrostate = (*macro_ab)[b * s1 + a];

      if (macrostate == -1L) {
        newstate++;
        (*macro_ab)[b * s1 + a] = newstate;
        (*macro_c)[newstate] = b * s1 + a;

        /* This differs from meet2 in this procedure call */
        insertlist4(newstate, tran1, srcstate, s3, t3);
        if (mem_result == 1)
          return;
      } else {
        /* This differs from meet2 in this procedure call */
        insertlist4(macrostate, tran1, srcstate, s3, t3);
        if (mem_result == 1)
          return;
      }
      colptr1++;
      colptr2++;
    }

    srcstate++;
    a = (*macro_c)[srcstate];
    if (a != -1L) {
      t1i = a % s1;
      t2j = a / s1;
    }

  } while (srcstate <= newstate);

  resize_des(t3, *s3, newstate + 1);
  *s3 = newstate + 1;

  (*t3)[0].reached = true;
  for (i = 0; i < *s3; i++) {
    a = (*macro_c)[i];
    (*t3)[i].marked = t1[a % s1].marked && t2[a / s1].marked;
  }

  /* Should be safe because the size is smaller than before */
  *macro_c = (INT_S *)REALLOC(*macro_c, sizeof(INT_S) * (*s3));
}

void sync2(INT_S s1, state_node *t1, INT_S s2, state_node *t2, INT_S *s3,
           state_node **t3, INT_S **macro_ab, INT_S **macro_c) {
  INT_T *tranlist1, *tranlist2, *sublist1, *sublist2;
  INT_T s_t1, s_t2, s_s1, s_s2;

#if defined(_x64_)
  INT_S i;
  unsigned long long *macro64_c;
  macro64_c = NULL;
#endif

  s_t1 = s_t2 = s_s1 = s_s2 = 0;
  tranlist1 = tranlist2 = sublist1 = sublist2 = NULL;

  if (s1 == 0 || s2 == 0) {
    *s3 = 0;
    *t3 = NULL;
    return;
  }

  gentranlist(s1, t1, &s_t1, &tranlist1);
  gentranlist(s2, t2, &s_t2, &tranlist2);
  gensublist(s_t1, tranlist1, s_t2, tranlist2, &s_s1, &sublist1, &s_s2,
             &sublist2);

  gentran(s_s2, sublist2, s1, t1);
  gentran(s_s1, sublist1, s2, t2);
#if defined(_x64_)
  meet_x64(s1, t1, s2, t2, s3, t3, &macro64_c);
  *macro_c = (INT_S *)REALLOC(*macro_c, sizeof(INT_S) * (*s3));
  for (i = 0; i < *s3; i++)
    (*macro_c)[i] = macro64_c[i];

  free(macro64_c);
#else
  meet2(s1, t1, s2, t2, s3, t3, macro_ab, macro_c);
#endif
  // meet2(s1, t1, s2, t2, s3, t3, macro_ab, macro_c);

  free(tranlist1);
  free(tranlist2);
  free(sublist1);
  free(sublist2);
}
void sync3(INT_S s1, state_node *t1, INT_S s2, state_node *t2, INT_S *s3,
           state_node **t3, INT_OS mode, INT_T s_tranlist, INT_T *tranlist,
           INT_S **macro_ab, INT_S **macro_c) {
  INT_T *tranlist1, *tranlist2, *sublist1, *sublist2;
  INT_T s_t1, s_t2, s_s1, s_s2;
  INT_S i;
  INT_B ok;

  s_t1 = s_t2 = s_s1 = s_s2 = 0;
  tranlist1 = tranlist2 = sublist1 = sublist2 = NULL;

  if (s1 == 0 || s2 == 0) {
    *s3 = 0;
    *t3 = NULL;
    return;
  }

  if (mode == 0)
    gentranlist(s1, t1, &s_t1, &tranlist1);
  gentranlist(s2, t2, &s_t2, &tranlist2);
  // gensublist(s_t1, tranlist1,
  //           s_t2, tranlist2,
  //           &s_s1, &sublist1,
  //           &s_s2, &sublist2);
  for (i = 0; i < s_tranlist; i++) {
    if (mode == 0) {
      if (!inlist(tranlist[i], tranlist1, s_t1)) {
        addordlist(tranlist[i], &sublist1, s_s1, &ok);
        if (ok)
          s_s1++;
      }
    }
    if (!inlist(tranlist[i], tranlist2, s_t2)) {
      addordlist(tranlist[i], &sublist2, s_s2, &ok);
      if (ok)
        s_s2++;
    }
  }
  if (mode == 0)
    gentran(s_s1, sublist1, s1, t1);
  gentran(s_s2, sublist2, s2, t2);
  meet2(s1, t1, s2, t2, s3, t3, macro_ab, macro_c);

  free(tranlist1);
  free(tranlist2);
  free(sublist1);
  free(sublist2);
}

void sync4(INT_S s1, state_node *t1, INT_S s2, state_node *t2, INT_S *s3,
           state_node **t3, INT_S **macro_ab, INT_S **macro_c) {
  INT_T *tranlist1, *tranlist2, *sublist1, *sublist2;
  INT_T s_t1, s_t2, s_s1, s_s2;

  s_t1 = s_t2 = s_s1 = s_s2 = 0;
  tranlist1 = tranlist2 = sublist1 = sublist2 = NULL;

  if (s1 == 0 || s2 == 0) {
    *s3 = 0;
    *t3 = NULL;
    return;
  }

  gentranlist(s1, t1, &s_t1, &tranlist1);
  gentranlist(s2, t2, &s_t2, &tranlist2);
  gensublist(s_t1, tranlist1, s_t2, tranlist2, &s_s1, &sublist1, &s_s2,
             &sublist2);

  gentran(s_s2, sublist2, s1, t1);
  gentran(s_s1, sublist1, s2, t2);

  meet2(s1, t1, s2, t2, s3, t3, macro_ab, macro_c);

  free(tranlist1);
  free(tranlist2);
  free(sublist1);
  free(sublist2);
}

/* Breadth first search */
void b_reach(tran_node *init, INT_S s_init, state_node **t1, INT_S s1) {
  INT_T cur;
  INT_S s;
  INT_S es;
  t_queue tq;

  /* Assume the "reached" field in state_node structure already
     set correctly.  In most cases, it means all false except,
     the zero state but not always. */

  queue_init(&tq);

  enqueue(&tq, s_init);

  while (!queue_empty(&tq)) {
    s = dequeue(&tq);
    for (cur = 0; cur < (*t1)[s].numelts; cur++) {
      es = (*t1)[s].next[cur].data2;
      if (!(*t1)[es].reached) {
        enqueue(&tq, es);
        (*t1)[es].reached = true;
      }
    }
  }

  queue_done(&tq);
}

void reversetran(INT_S src, state_node *t1, state_node **t2) {
  INT_T cur;
  INT_B ok;
  INT_S target;

  cur = 0;
  while (cur < t1[src].numelts) {
    target = t1[src].next[cur].data2;
    addordlist1(t1[src].next[cur].data1, src, &(*t2)[target].next,
                (*t2)[target].numelts, &ok);
    if (ok)
      (*t2)[target].numelts++;
    cur++;
  }
}

void coreach1(tran_node *init, INT_S s_init, INT_S s1, state_node **t1,
              char *un) {
  INT_B ok, ok2, found;
  INT_T cur;
  INT_S s;
  INT_S es;
  INT_S i;
  char *visited;
  t_stack ts;

  visited = (char *)CALLOC(s1, sizeof(char));

  pstack_Init(&ts);

  cur = 0;
  s = s_init;

  /* To avoid selfloop, we mark the first state as being visited */
  visited[s] = 1;

  while (!((cur >= (*t1)[s].numelts) && pstack_IsEmpty(&ts))) {
    es = (*t1)[s].next[cur].data2;
    if ((*t1)[es].coreach || (*t1)[es].marked) {
      /* Unwind stack to marked all states as coreached */
      do {
        pstack_Pop(&ts, &cur, &s, &ok);
        if (ok)
          (*t1)[s].coreach = true;
      } while (!pstack_IsEmpty(&ts));
      (*t1)[s_init].coreach = true;
      break;
    } else {
      /* Have we visited this state already? */
      found = (visited[es] == 1) || (un[es] == 1);

      if (found == true) {
        cur++;
      } else {
        visited[es] = 1;
        ok2 = pstack_Push(&ts, cur, s);
        if (ok2 == false) {
          mem_result = 1;
          pstack_Done(&ts);
          return;
        }
        s = es;
        cur = 0L;
      }
    }

    if (cur >= (*t1)[s].numelts) {
      do {
        pstack_Pop(&ts, &cur, &s, &ok);
        if (cur < (*t1)[s].numelts && ok)
          cur++;
      } while (!((cur < (*t1)[s].numelts) || pstack_IsEmpty(&ts)));
    }
  }

  if ((*t1)[s_init].coreach == false) {
    /* Mark all others as uncoreachedable also */
    for (i = 0; i < s1; i++) {
      if (visited[i] == 1)
        un[i] = 1;
    }
  }

  pstack_Done(&ts);
  free(visited);
}

void coreach2(INT_S s1, state_node **t1) {
  INT_S state;
  char *un;

  if (!(*t1)[0].marked)
    (*t1)[0].coreach = false;

  /* Should always have "coreach" field in "t1" set to false
     to start */
  for (state = 1; state < s1; state++)
    (*t1)[state].coreach = false;

  un = (char *)CALLOC(s1, sizeof(char));

  for (state = 0; state < s1; state++) {
    if ((*t1)[state].marked || (*t1)[state].coreach) {
      (*t1)[state].coreach = true;
    } else if (un[state] == 1) {
      /* Already determined not a coreachable state */
    } else {
      /* Can we be coreached */
      coreach1((*t1)[state].next, state, s1, t1, un);
    }
  }

  free(un);
}

/* File base coreach */
void _coreach2(INT_S s1, state_node **t1) {
  INT_B ok;
  INT_S i, j, s;
  INT_T jj, e, num;
  FILE *in;
  tran_node *next;

  /* Save DES to file */
  printf("Save $$$\n");
  filedes("$$$", s1, 0L, *t1);

  printf("Saving done\n");
  /* Free transitions */
  for (i = 0L; i < s1; i++) {
    if ((*t1)[i].next != NULL) {
      free((*t1)[i].next);
      (*t1)[i].next = NULL;
    }
    (*t1)[i].numelts = 0;
    (*t1)[i].coreach = (*t1)[i].reached;
    (*t1)[i].reached = false;
  }
  (*t1)[0].reached = true;

  printf("Freeing t1 tempory done\n");

  /* Read it back */
  /* Skip over marking and other states */
  in = fopen("$$$.des", "rb");
  if (in == NULL) {
    printf("Error reading $$$.des\n");
    exit(1);
  }

  fread(&s, sizeof(INT_S), 1, in);
  fread(&s, sizeof(INT_S), 1, in);

  /* Read over the marked states */
  s = 0L;
  while (s != -1L) {
    fread(&s, sizeof(INT_S), 1, in);
  }

  /* Read the transitions */
  s = 0L;
  while (s != -1L) {
    fread(&s, sizeof(INT_S), 1, in);

    if (s == -1L)
      break;

    fread(&num, sizeof(INT_T), 1, in);
    next = (tran_node *)MALLOC(sizeof(tran_node) * num);
    if (num != 0) {
      if (next == NULL) {
        printf("Out of memory\n");
        exit(1);
      }
    }
    fread(next, sizeof(tran_node), num, in);

    for (jj = 0; jj < num; jj++) {
      e = next[jj].data1;
      j = next[jj].data2;
      addordlist1(e, s, &(*t1)[j].next, (*t1)[j].numelts, &ok);
      if (ok)
        (*t1)[j].numelts++;
    }

    free(next);
  }
  fclose(in);

  printf("Read back and reverse transitions done\n");

  for (i = 0; i < s1; i++) {
    (*t1)[i].reached = false;
  }

  /* Now do reachable */
  for (i = 0; i < s1; i++) {
    if ((*t1)[i].marked) {
      (*t1)[i].reached = true;
      b_reach((*t1)[i].next, i, t1, s1);
    }
  }

  printf("Reach done\n");

  /* Free again */
  /* Free transitions */
  for (i = 0L; i < s1; i++) {
    if ((*t1)[i].next != NULL) {
      free((*t1)[i].next);
      (*t1)[i].next = NULL;
    }
    (*t1)[i].numelts = 0;
  }

  printf("Freeing memory done\n");

  /* Read it back */
  /* Skip over marking and other states */
  in = fopen("$$$.des", "rb");
  if (in == NULL) {
    printf("Error reading $$$.des\n");
    exit(1);
  }

  fread(&s, sizeof(INT_S), 1, in);
  fread(&s, sizeof(INT_S), 1, in);

  /* Read over the marked states */
  s = 0L;
  while (s != -1L) {
    fread(&s, sizeof(INT_S), 1, in);
  }

  s = 0L;
  while (s != -1L) {
    fread(&s, sizeof(INT_S), 1, in);

    if (s == -1L)
      break;

    fread(&num, sizeof(INT_T), 1, in);
    (*t1)[s].next = (tran_node *)MALLOC(sizeof(tran_node) * num);
    if ((*t1)[s].next == NULL) {
      printf("Out of memory\n");
      fclose(in);
      exit(1);
    }

    (*t1)[s].numelts = num;
    fread((*t1)[s].next, sizeof(tran_node), num, in);
  }

  fclose(in);
  remove("$$$.des");

  printf("Read back done\n");
}

void recode(INT_S s1, state_node **t1, recode_node *recode_states) {
  INT_S state, es, new_state;
  INT_T numelts, j, diff;

  for (state = 0; state < s1; state++) {
    if (recode_states[state].reached) {
      /* Purge transitions that point to dead states */
      j = 0;
      numelts = (*t1)[state].numelts;
      while (j < (*t1)[state].numelts) {
        es = (*t1)[state].next[j].data2;
        if (!recode_states[es].reached) {
          diff = (*t1)[state].numelts - (j + 1);
          if (diff > 0)
            memmove(&(*t1)[state].next[j], &(*t1)[state].next[j + 1],
                    sizeof(tran_node) * diff);
          (*t1)[state].numelts--;
        } else {
          (*t1)[state].next[j].data2 = recode_states[es].recode;
          j++;
        }
      }

      if (numelts != (*t1)[state].numelts) {
        (*t1)[state].next = (tran_node *)REALLOC(
            (*t1)[state].next, sizeof(tran_node) * (*t1)[state].numelts);
      }

      /* Move state over if necessary */
      new_state = recode_states[state].recode;
      if (new_state != state) {
        memmove(&(*t1)[new_state], &(*t1)[state], sizeof(state_node));
        /* Remove all traces of the moved state */
        (*t1)[state].next = NULL;
      }
    } else {
      /* Purge the transitions from this state */
      (*t1)[state].numelts = 0;
      if ((*t1)[state].next != NULL) {
        free((*t1)[state].next);
        (*t1)[state].next = NULL;
      }
    }
  }
}

/* Faster Trim and less memory */
void trim1(INT_S *s1, state_node **t1) {
  INT_S s2, state;
  recode_node *recode_states;
  INT_S num_reachable;

  if (*s1 <= 0)
    return;

  /* Pre-condition.  Assume states are unreachable first */
  for (state = 0; state < *s1; state++)
    (*t1)[state].reached = false;

  (*t1)[0].reached = true;
  b_reach((*t1)[0].next, 0L, t1, *s1);
  coreach2(*s1, t1);

  s2 = 0;
  for (state = *s1 - 1L; state >= 0; state--) {
    if ((*t1)[state].reached && (*t1)[state].coreach)
      s2++;
    else
      (*t1)[state].reached = false;
  }
  if (s2 == *s1) {
    return;
  }

  /* Remove all "unreachable states" */
  /* Allocate tempory data structure for new recorded names */
  recode_states = (recode_node *)CALLOC(*s1, sizeof(recode_node));

  /* Re-name all reached states to the new state names */
  num_reachable = 0;
  for (state = 0; state < *s1; state++) {
    if ((*t1)[state].reached) {
      recode_states[state].recode = num_reachable;
      recode_states[state].reached = true;
      num_reachable++;
    }
  }

  /* Purge dead transitions followed by purging states */
  recode(*s1, t1, recode_states);

  *t1 = (state_node *)REALLOC(*t1, sizeof(state_node) * num_reachable);
  *s1 = num_reachable;

  free(recode_states);
}

void recode2(INT_S s1, state_node **t1, recode_node *recode_states,
             INT_S *macro_c) {
  INT_S state, es, new_state;
  INT_T numelts, j, diff;

  for (state = 0; state < s1; state++) {
    if (recode_states[state].reached) {
      /* Purge transitions that point to dead states */
      j = 0;
      numelts = (*t1)[state].numelts;
      while (j < (*t1)[state].numelts) {
        es = (*t1)[state].next[j].data2;
        if (!recode_states[es].reached) {
          diff = (*t1)[state].numelts - (j + 1);
          if (diff > 0)
            memmove(&(*t1)[state].next[j], &(*t1)[state].next[j + 1],
                    sizeof(tran_node) * diff);
          (*t1)[state].numelts--;
        } else {
          (*t1)[state].next[j].data2 = recode_states[es].recode;
          j++;
        }
      }

      if (numelts != (*t1)[state].numelts) {
        (*t1)[state].next = (tran_node *)REALLOC(
            (*t1)[state].next, sizeof(tran_node) * (*t1)[state].numelts);
      }

      /* Move state over if necessary */
      new_state = recode_states[state].recode;
      if (new_state != state) {
        memmove(&(*t1)[new_state], &(*t1)[state], sizeof(state_node));
        /* Remove all traces of the moved state */
        (*t1)[state].next = NULL;
        macro_c[new_state] = macro_c[state];
      }
    } else {
      /* Purge the transitions from this state */
      (*t1)[state].numelts = 0;
      if ((*t1)[state].next != NULL) {
        free((*t1)[state].next);
        (*t1)[state].next = NULL;
      }
    }
  }
}

/* Pre-condition.
   DES reach is set to false if you want to purge the bad state
 */
void purgebadstates(INT_S s1, state_node **t1) {
  INT_S state, es;
  INT_T numelts, j, diff;

  for (state = 0; state < s1; state++) {
    if ((*t1)[state].reached) {
      /* Purge transitions that point to dead states */
      j = 0;
      numelts = (*t1)[state].numelts;
      while (j < (*t1)[state].numelts) {
        es = (*t1)[state].next[j].data2;
        if (!(*t1)[es].reached) {
          diff = (*t1)[state].numelts - (j + 1);
          if (diff > 0)
            memmove(&(*t1)[state].next[j], &(*t1)[state].next[j + 1],
                    sizeof(tran_node) * diff);
          (*t1)[state].numelts--;
        } else {
          j++;
        }
      }

      if (numelts != (*t1)[state].numelts) {
        (*t1)[state].next = (tran_node *)REALLOC(
            (*t1)[state].next, sizeof(tran_node) * (*t1)[state].numelts);
      }
    } else {
      /* Purge the transitions from this state */
      (*t1)[state].numelts = 0;
      if ((*t1)[state].next != NULL) {
        free((*t1)[state].next);
        (*t1)[state].next = NULL;
      }
    }
  }
}

void _trim2(INT_S *s1, state_node **t1, INT_S *macro_c) {
  INT_S s2, state;
  recode_node *recode_states;
  INT_S num_reachable;

  if (*s1 <= 0)
    return;

  for (state = 0; state < *s1; state++)
    (*t1)[state].reached = false;

  (*t1)[0].reached = true;
  b_reach((*t1)[0].next, 0L, t1, *s1);
  _coreach2(*s1, t1);

  s2 = 0;
  for (state = *s1 - 1L; state >= 0; state--) {
    if ((*t1)[state].reached && (*t1)[state].coreach)
      s2++;
    else
      (*t1)[state].reached = false;
  }
  if (s2 == *s1) {
    return;
  }

  /* Remove all "unreachable states" */
  /* Allocate tempory data structure for new recorded names */
  recode_states = (recode_node *)CALLOC(*s1, sizeof(recode_node));

  /* Re-name all reached states to the new state names */
  num_reachable = 0;
  for (state = 0; state < *s1; state++) {
    if ((*t1)[state].reached) {
      recode_states[state].recode = num_reachable;
      recode_states[state].reached = true;
      num_reachable++;
    }
  }

  /* Purge dead transitions followed by purging states */
  recode2(*s1, t1, recode_states, macro_c);

  *t1 = (state_node *)REALLOC(*t1, sizeof(state_node) * num_reachable);
  *s1 = num_reachable;

  free(recode_states);
}

void trim2(INT_S *s1, state_node **t1, INT_S *macro_c) {
  INT_S s2, state;
  recode_node *recode_states;
  INT_S num_reachable;

  if (*s1 <= 0)
    return;

  /* Set all reached to false */
  for (state = 0; state < *s1; state++)
    (*t1)[state].reached = false;

  (*t1)[0].reached = true;
  b_reach((*t1)[0].next, 0L, t1, *s1);
  coreach2(*s1, t1);

  s2 = 0;
  for (state = *s1 - 1L; state >= 0; state--) {
    if ((*t1)[state].reached && (*t1)[state].coreach)
      s2++;
    else
      (*t1)[state].reached = false;
  }
  if (s2 == *s1) {
    return;
  }

  /* Remove all "unreachable states" */
  /* Allocate tempory data structure for new recorded names */
  recode_states = (recode_node *)CALLOC(*s1, sizeof(recode_node));

  /* Re-name all reached states to the new state names */
  num_reachable = 0;
  for (state = 0; state < *s1; state++) {
    if ((*t1)[state].reached) {
      recode_states[state].recode = num_reachable;
      recode_states[state].reached = true;
      num_reachable++;
    }
  }

  /* Purge dead transitions followed by purging states */
  recode2(*s1, t1, recode_states, macro_c);

  *t1 = (state_node *)REALLOC(*t1, sizeof(state_node) * num_reachable);
  *s1 = num_reachable;

  free(recode_states);
}

void uncontrolist(INT_S state, state_node *t, INT_T *numlist) {
  INT_T i;

  *numlist = 0;
  /* Count the number of uncontrollable events */
  for (i = 0; i < t[state].numelts; i++) {
    if (t[state].next[i].data1 % 2 == 0)
      (*numlist)++;
  }
}

void uncontrolist2(INT_S state, state_node *t, INT_T *numlist) {
  INT_T i;
  INT_S es;

  *numlist = 0;
  /* Count the number of uncontrollable events */
  for (i = 0; i < t[state].numelts; i++) {
    if (t[state].next[i].data1 % 2 == 0) {
      es = t[state].next[i].data2;
      if (t[es].reached)
        (*numlist)++;
    }
  }
}

void shave1(INT_S s1, state_node *t1, INT_S *s3, state_node **t3,
            INT_S *macro_c) {
  INT_S state, state1, s2;
  INT_T numlist1, numlist2;

  s2 = *s3;
  do {
    s2 = *s3;
    for (state = 0; state < *s3; state++) {
      (*t3)[state].reached = true;
      state1 = macro_c[state] % s1;
      uncontrolist(state1, t1, &numlist1);
      if (numlist1 != 0) {
        uncontrolist2(state, *t3, &numlist2);
        if (numlist1 != numlist2) {
          (*t3)[state].reached = false;
          /* Need to mark this state as to be deleted */
        }
      }
    }

    /* Purge all references to the unreached states? */
    purgebadstates(*s3, t3);

    if (*s3 > 0) {
      if (!(*t3)[0].reached) {
        freedes(*s3, t3);
        *s3 = 0;
        (*t3) = NULL;
        return;
      }
    }

    for (state = 1; state < *s3; state++) {
      (*t3)[state].reached = false;
    }
    trim2(s3, t3, macro_c);
  } while (s2 != *s3);
}

void genlist(INT_T slist1, tran_node *list1, INT_T slist2, tran_node *list2,
             INT_S s, state_node *t) {
  INT_T j1, j2;
  INT_B ok;

  j1 = j2 = 0;
  while ((j1 < slist1) && (j2 < slist2)) {
    if (list1[j1].data1 == list2[j2].data1) {
      j1++;
      j2++;
    } else if (list1[j1].data1 > list2[j2].data1) {
      j2++;
    } else {
      addordlist1(list1[j1].data1, 0, &t[s].next, t[s].numelts, &ok);
      if (ok)
        t[s].numelts++;
      j1++;
    }
  }

  while (j1 < slist1) {
    addordlist1(list1[j1].data1, 0, &t[s].next, t[s].numelts, &ok);
    if (ok)
      t[s].numelts++;
    j1++;
  }
}

void genstatetran(INT_S state, state_node *t, INT_T *s_state_tranlist,
                  INT_T **state_tranlist) {
  INT_T i;
  INT_B ok;

  *state_tranlist = NULL;
  *s_state_tranlist = 0;

  for (i = 0; i < t[state].numelts; i++) {
    addordlist(t[state].next[i].data1, state_tranlist, *s_state_tranlist, &ok);
    if (ok)
      (*s_state_tranlist)++;
  }
}

void gendifflist(INT_T s_list1, INT_T *list1, INT_T s_list2, INT_T *list2,
                 INT_T *s_sublist, INT_T **sublist) {
  INT_T i, j;
  INT_B ok;

  /* Generate sublist (list1-list2) */
  *sublist = NULL;
  *s_sublist = 0;
  i = j = 0;

  while ((i < s_list1) && (j < s_list2)) {
    if (list1[i] == list2[j]) {
      i++;
      j++;
    } else if (list1[i] > list2[j]) {
      j++;
    } else {
      addordlist(list1[i], sublist, *s_sublist, &ok);
      if (ok)
        (*s_sublist)++;
      i++;
    }
  }

  /* append the rest of "list1" to "sublist" */
  while (i < s_list1) {
    addordlist(list1[i], sublist, *s_sublist, &ok);
    if (ok)
      (*s_sublist)++;
    i++;
  }
}

void complement1(INT_S *s, state_node **t, INT_T slist, INT_T *list) {
  INT_S state;
  INT_B newsize, ok;
  INT_T *tranlist;
  INT_T s_tranlist;

  INT_T *state_tranlist;
  INT_T s_state_tranlist;

  INT_T *new_tranlist;
  INT_T s_new_tranlist;

  INT_T i;

  s_tranlist = 0;
  tranlist = NULL;
  s_new_tranlist = 0;
  new_tranlist = NULL;
  s_state_tranlist = 0;
  state_tranlist = NULL;

  gentranlist(*s, *t, &s_tranlist, &tranlist);

  for (i = 0; i < slist; i++) {
    addordlist(list[i], &tranlist, s_tranlist, &ok);
    if (ok)
      s_tranlist++;
  }

  newsize = false;
  if (*s == 0) {
    newsize = true;
    *t = newdes(1);
  } else {
    for (state = 0; state < *s; state++) {
      (*t)[state].marked = !(*t)[state].marked;

      if (state_tranlist != NULL) {
        free(state_tranlist);
        state_tranlist = NULL;
      }
      s_state_tranlist = 0;

      genstatetran(state, *t, &s_state_tranlist, &state_tranlist);

      if (s_state_tranlist < s_tranlist) {
        if (newsize == false) {
          *t = (state_node *)REALLOC(*t, sizeof(state_node) * (*s + 1));
          newsize = true;
        }

        if (new_tranlist != NULL) {
          free(new_tranlist);
          new_tranlist = NULL;
        }
        s_new_tranlist = 0;

        gendifflist(s_tranlist, tranlist, s_state_tranlist, state_tranlist,
                    &s_new_tranlist, &new_tranlist);

        for (i = 0; i < s_new_tranlist; i++) {
          addordlist1(new_tranlist[i], *s, &(*t)[state].next,
                      (*t)[state].numelts, &ok);
          if (ok)
            (*t)[state].numelts++;
        }
      }
    }
  }

  if (newsize) {
    (*t)[*s].numelts = 0;
    (*t)[*s].marked = true;
    (*t)[*s].vocal = 0;
    (*t)[*s].reached = false;
    (*t)[*s].next = NULL;
    for (i = 0; i < s_tranlist; i++) {
      addordlist1(tranlist[i], *s, &(*t)[*s].next, (*t)[*s].numelts, &ok);
      if (ok)
        (*t)[*s].numelts++;
    }
    (*s)++;
  }

  if (tranlist != NULL)
    free(tranlist);
  if (state_tranlist != NULL)
    free(state_tranlist);
  if (new_tranlist != NULL)
    free(new_tranlist);
}

/* Regular reach */
void reach(INT_S *s1, state_node **t1) {
  INT_S s2, state;
  recode_node *recode_states;
  INT_S num_reachable;

  if (*s1 <= 0)
    return;

  /* Zero all the reach variables */
  for (state = 0; state < *s1; state++)
    (*t1)[state].reached = false;

  (*t1)[0].reached = true;
  b_reach((*t1)[0].next, 0L, t1, *s1);

  s2 = 0;
  for (state = *s1 - 1L; state >= 0; state--) {
    if ((*t1)[state].reached)
      s2++;
  }
  if (s2 == *s1) {
    return;
  }

  /* Remove all "unreachable states" */
  /* Allocate tempory data structure for new recorded names */
  recode_states = (recode_node *)CALLOC(*s1, sizeof(recode_node));

  /* Re-name all reached states to the new state names */
  num_reachable = 0;
  for (state = 0; state < *s1; state++) {
    if ((*t1)[state].reached) {
      recode_states[state].recode = num_reachable;
      recode_states[state].reached = true;
      num_reachable++;
    }
  }

  /* Purge dead transitions followed by purging states */
  recode(*s1, t1, recode_states);

  *t1 = (state_node *)REALLOC(*t1, sizeof(state_node) * num_reachable);
  *s1 = num_reachable;

  free(recode_states);
}

void compare_tran(INT_S i, INT_S j, state_node *t1, state_node *t2,
                  INT_B *equal, INT_S *mapState, state_pair **iEj,
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

        /*       Third attempt at fixing code -- 2007 */
        if ((mapState[entr1] != -1) && (mapState[entr2] == -1))
          /*             *equal = (entr1 == entr2) && (mapState[entr1] ==
           * mapState[entr2]); */
          *equal = false;
        else
          *equal = mapState[entr1] == entr2;

        /*       Second attempt at fixing code -- 2006 */
        /*       if ((mapState[entr1] != -1) && (mapState[entr2] != -1))
                    *equal = entr1 == entr2;
                 else
                    *equal = mapState[entr1] == entr2;
        */

        /*       First attempt at fixing code -- 1995 */
        /*       *equal = (entr1 == entr2) && (mapState[entr1] ==
         * mapState[entr2]);  */

        /*       Original code -- 1991 */
        /*       *equal = mapState[entr1] == entr2;  */

        if (!(*equal)) {
          *equal = instatepair(entr1, entr2, iEj, *s_iEj);
        }

        if (!(*equal)) {
          if ((entr1 != i) || ((entr1 == j) && (entr2 > j))) {
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
          }
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
      else {
        *equal = true;
        mapState[i] = j;
        t1[i].reached = true;
        t2[j].reached = true;
      }
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

void compare_states(INT_S i, INT_S j, INT_B *equal, INT_S *mapState,
                    state_pair **iEj, INT_S *s_iEj, state_node *t1,
                    state_node *t2) {
  INT_B bothmarked, marktest;

  bothmarked = (t1[i].marked == t2[j].marked);
  marktest = bothmarked && (t1[i].numelts == 0) && (t2[j].numelts == 0) &&
             (t1[i].vocal == t2[j].vocal);
  if (marktest)
    *equal = true;
  if (!marktest) {
    if (bothmarked && (t1[i].numelts == t2[j].numelts) &&
        t1[i].vocal == t2[j].vocal)
      compare_tran(i, j, t1, t2, equal, mapState, iEj, s_iEj);
    else
      *equal = false;
  }
}

void minimize(INT_S *s1, state_node **t1) {
  if (minflag == 0) {
    /* Run the old minimization algorithm */
    minimize1(s1, t1);
  } else {
    /* Run the new minimization algorithm */
    minimize2(s1, t1);
  }
}

void recodelist(INT_S state, state_node *t1, INT_T *nullist, INT_T s_nullist,
                INT_B *nullstate_bool) {
  /* Convert the transition labels to be projected to 1023.
     Cannot use -1 because it is an unsigned integer.
     Need better method to mark transitions to be converted. */

  INT_T cur, elem;

  *nullstate_bool = false;
  for (cur = 0; cur < t1[state].numelts; cur++) {
    elem = t1[state].next[cur].data1;
    if (inlist(elem, nullist, s_nullist)) {
      t1[state].next[cur].data1 = 1023; /* hard code for now */
      *nullstate_bool = true;
    }
  }
}

void reorderlist(tran_node *next, INT_T numelts, tran_node **orderlist,
                 INT_T *s_orderlist) {
  INT_B ok;
  INT_T i;

  *s_orderlist = 0;
  for (i = 0; i < numelts; i++) {
    addordlist1(next[i].data1, next[i].data2, orderlist, *s_orderlist, &ok);
    if (ok)
      (*s_orderlist)++;
  }
}

void addpart(INT_S e, INT_S **L, INT_S size, INT_B *ok) {
  INT_S pos;
  INT_S lower, upper;
  INT_B found;

  *ok = false;

  /* Do a binary search. */
  found = false;
  pos = 0;
  if (size > 1) {
    lower = 1;
    upper = size;
    while ((found == false) && (lower <= upper)) {
      pos = (lower + upper) / 2;
      if (e == (*L)[pos - 1]) {
        found = true;
      } else if (e > (*L)[pos - 1]) {
        lower = pos + 1;
      } else {
        upper = pos - 1;
      }
    }

    if (found == false) {
      if (e < (*L)[pos - 1])
        pos--;
    }
  } else if (size == 1) {
    if (e == (*L)[0]) {
      found = true;
    } else if (e > (*L)[0]) {
      pos = 1;
    }
  }

  if (found == true) {
    return;
  }

  /* Make space for new element */
  *L = (INT_S *)REALLOC(*L, sizeof(INT_S) * (size + 1));
  if (*L == NULL) {
    mem_result = 1;
    *ok = false;
    return;
  }

  /* Move over any elements down the list */
  if ((size - pos) > 0)
    memmove(&(*L)[pos + 1], &(*L)[pos], sizeof(INT_S) * (size - pos));

  /* Insert the element into the list */
  (*L)[pos] = e;

  *ok = true;
}

void build_nullsets(INT_S state, state_node *t1, part_node *nullsets) {
  INT_B ok;
  INT_S entr2;
  INT_S s;
  INT_T cur;
  t_stack ts;

  pstack_Init(&ts);

  t1[state].reached = true;
  s = state;
  cur = 0;

  do {
  LABEL1:
    while ((cur < t1[s].numelts) && (t1[s].next[cur].data1 != 1023)) {
      cur++;
    }

    while ((cur < t1[s].numelts) && (t1[s].next[cur].data1 == 1023)) {
      entr2 = t1[s].next[cur].data2;
      addpart(entr2, &nullsets[state].next, nullsets[state].numelts, &ok);
      if (ok)
        nullsets[state].numelts++;
      if (!t1[entr2].reached) {
        t1[entr2].reached = true;
        pstack_Push(&ts, cur, s);
        cur = 0;
        s = entr2;
        goto LABEL1;
      }
    LABEL2:
      cur++;
    }

    pstack_Pop(&ts, &cur, &s, &ok);
    if (cur < t1[s].numelts && ok)
      goto LABEL2;
  } while (!pstack_IsEmpty(&ts));

  pstack_Done(&ts);
}

void mark_newstate(INT_S *next, INT_S numelts, state_node *t1, INT_B *marked) {
  INT_T cur;
  INT_S cur_state;

  *marked = false;
  cur = 0;
  while (cur < numelts && !(*marked)) {
    cur_state = next[cur];
    if (t1[cur_state].marked)
      *marked = true;
    else
      cur++;
  }
}

void generate(INT_S *next, INT_S numelts, state_node **t1, state_map **templist,
              INT_S *s_templist) {
  INT_T i, j;
  INT_T event;
  INT_S entr;
  INT_B ok;

  for (i = 0; i < numelts; i++) {
    for (j = 0; j < (*t1)[next[i]].numelts; j++) {
      event = (*t1)[next[i]].next[j].data1;
      entr = (*t1)[next[i]].next[j].data2;
      if (event != 1023) {
        addstatemap(event, entr, templist, *s_templist, &ok);
        if (ok)
          (*s_templist)++;
      }
    }
  }
}

void unionsets(INT_S *list1, INT_S size1, INT_S **list2, INT_S *size2) {
  /* Form the union: list2 <- list1 + list2 */
  INT_S cur;
  INT_B ok;

  for (cur = 0; cur < size1; cur++) {
    addstatelist(list1[cur], list2, *size2, &ok);
    if (ok)
      (*size2)++;
  }
}

INT_B equal_list(INT_S *list1, INT_S *list2, INT_S numelts) {
  INT_S i;

  for (i = 0; i < numelts; i++) {
    if (list1[i] != list2[i])
      return false;
  }
  return true;
}

void memberset(INT_S *tempset, INT_S numelts, state_map *macrosets, INT_S size,
               INT_S *macrostate) {
  INT_B found;
  INT_S firstelt, i;

  /* Check that the list of states exists in the set of existing states */
  if (numelts <= 0) {
    *macrostate = -2;
    return;
  }

  i = 0;
  found = false;
  firstelt = tempset[0];
  while ((i < size) && !found) {
    /* Try to find the first element */
    if (firstelt > macrosets[i].next[0]) {
    } else if (firstelt == macrosets[i].next[0]) {
      if (numelts == macrosets[i].numelts) {
        if (equal_list(tempset, macrosets[i].next, numelts)) {
          *macrostate = macrosets[i].state;
          found = true;
        }
      }
    } /* else {
       found = true;
       *macrostate = -1;
    } */
    i++;
  }

  if (found == false) {
    *macrostate = -1;
  }
}

void addordlist3(INT_S newstate, INT_S setsize, INT_B marked, INT_S *tempset,
                 state_map **macrosets, INT_S *s_macrosets) {
  INT_B ok;
  INT_S i;

  for (i = 0; i < setsize; i++) {
    addstatemap(newstate, tempset[i], macrosets, *s_macrosets, &ok);
    if (ok)
      (*s_macrosets)++;
  }

  for (i = 0; i < *s_macrosets; i++) {
    if ((*macrosets)[i].state == newstate) {
      (*macrosets)[i].marked = marked;
      break;
    }
  }
}

INT_B is_deterministic(state_node *t, INT_S s) {
  INT_S i;
  INT_T j, ee;

  for (i = 0; i < s; i++) {
    if (t[i].numelts > 0) {
      ee = t[i].next[0].data1;

      if (ee == EEE)
        return false;
    }

    for (j = 1; j < t[i].numelts; j++) {
      if (t[i].next[j].data1 == ee)
        return false;
      else if (t[i].next[j].data1 == EEE)
        return false;
      else
        ee = t[i].next[j].data1;
    }
  }

  return true;
}

void determinize(state_node **t1, INT_S *s1) {
  INT_S state, i;
  // INT_B  nullstate_bool;
  // INT_S *nullstates_list, s_nullstates_list;
  // tran_node *orderlist; INT_T s_orderlist;
  part_node *nullsets;
  INT_B ok, marked;
  state_map *macrosets;
  INT_S s_macrosets;
  INT_S s2;
  state_node *t2;
  state_map *templist;
  INT_S s_templist;
  INT_S *tempset;
  INT_S setsize;
  INT_S macrostate;
  INT_S j, k;
  INT_S srcstate, newstate, curr_row;
  INT_B found;
  INT_S macroptr;

  // nullstates_list = NULL;   s_nullstates_list = 0;
  // orderlist = NULL; s_orderlist = 0;
  nullsets = NULL;
  macrosets = NULL;
  s_macrosets = 0;
  t2 = NULL;
  s2 = 0;
  templist = NULL;
  s_templist = 0;
  tempset = NULL;
  setsize = 0;

  if (is_deterministic(*t1, *s1))
    return;

  nullsets = (part_node *)CALLOC(*s1, sizeof(part_node));
  if (nullsets == NULL) {
    mem_result = 1;
    goto FREE_ALL;
  }

  for (state = 0; state < *s1; state++) {
    nullsets[state].numelts = 1;
    nullsets[state].next = (INT_S *)MALLOC(sizeof(INT_S));
    if (nullsets[state].next == NULL) {
      mem_result = 1;
      goto FREE_ALL;
    }
    nullsets[state].next[0] = state;
  }

  s_macrosets = 1;
  macrosets = (state_map *)MALLOC(sizeof(state_map));
  if (macrosets == NULL) {
    mem_result = 1;
    goto FREE_ALL;
  }

  macrosets[0].state = 0;
  macrosets[0].numelts = 0;
  mark_newstate(nullsets[0].next, nullsets[0].numelts, *t1, &marked);
  macrosets[0].marked = marked;
  macrosets[0].next = NULL;

  /* Copy nullsets[0].next -> macrosets[0].next */
  for (i = 0; i < nullsets[0].numelts; i++) {
    addstatelist(nullsets[0].next[i], &macrosets[0].next, macrosets[0].numelts,
                 &ok);
    if (ok)
      macrosets[0].numelts++;
  }

  templist = NULL;
  s_templist = 0;

  generate(nullsets[0].next, nullsets[0].numelts, t1, &templist, &s_templist);
  if (templist == NULL) {
    /* Free all pointer -> do a "goto" free part */
    s2 = 1;
    t2 = newdes(s2);
    t2[0].marked = macrosets[0].marked;
    freedes(*s1, t1);
    *s1 = s2;
    *t1 = t2;
    s2 = 0;
    t2 = NULL; /* Goto free part of the procedure */
    goto FREE_ALL;
  }

  srcstate = 0;
  newstate = 0;
  s2 = 0;
  t2 = NULL;

  do {
    for (i = 0; i < s_templist; i++) {
      curr_row = templist[i].state;
      for (j = 0; j < templist[i].numelts; j++) {
        k = templist[i].next[j];
        unionsets(nullsets[k].next, nullsets[k].numelts, &tempset, &setsize);
      }
      memberset(tempset, setsize, macrosets, s_macrosets, &macrostate);
      if (macrostate == -1) {
        newstate++;
        mark_newstate(tempset, setsize, *t1, &marked);
        addordlist3(newstate, setsize, marked, tempset, &macrosets,
                    &s_macrosets);
        insertlist4(srcstate, (INT_T)curr_row, newstate, &s2, &t2);
      } else {
        if (macrostate != -2) {
          insertlist4(srcstate, (INT_T)curr_row, macrostate, &s2, &t2);
        }
      }

      if (tempset != NULL) {
        free(tempset);
        tempset = NULL;
      }
      setsize = 0;
    }
    srcstate++;

    /* Some stuff here */
    found = false;
    for (i = 0; i < s_macrosets; i++) {
      if (macrosets[i].state == srcstate) {
        found = true;
        macroptr = i;
        break;
      }
    }

    if (found) {
      if (s_templist > 0) {
        free(templist);
        templist = NULL;
        s_templist = 0;
      }
      generate(macrosets[macroptr].next, macrosets[macroptr].numelts, t1,
               &templist, &s_templist);
    }
  } while (srcstate <= newstate);

  resize_des(&t2, s2, newstate + 1);
  s2 = newstate + 1;

  for (i = 0; i < s_macrosets; i++) {
    t2[macrosets[i].state].marked = macrosets[i].marked;
  }

  freedes(*s1, t1);
  *s1 = s2;
  *t1 = t2;

  /* Free all tempory variables here */
FREE_ALL:
  // free(nullstates_list);
  // free(orderlist);
  free(nullsets);
  free(macrosets);
  free(templist);
  free(tempset);
}

void project1(INT_S *s1, state_node **t1, INT_T s_nullist, INT_T *nullist) {
  INT_S state, i;
  INT_B nullstate_bool;
  INT_S *nullstates_list, s_nullstates_list;
  tran_node *orderlist;
  INT_T s_orderlist;
  part_node *nullsets;
  INT_B ok, marked;
  state_map *macrosets;
  INT_S s_macrosets;
  INT_S s2;
  state_node *t2;
  state_map *templist;
  INT_S s_templist;
  INT_S *tempset;
  INT_S setsize;
  INT_S macrostate;
  INT_S j, k;
  INT_S srcstate, newstate, curr_row;
  INT_B found;
  INT_S macroptr;

  nullstates_list = NULL;
  s_nullstates_list = 0;
  orderlist = NULL;
  s_orderlist = 0;
  nullsets = NULL;
  macrosets = NULL;
  s_macrosets = 0;
  t2 = NULL;
  s2 = 0;
  templist = NULL;
  s_templist = 0;
  tempset = NULL;
  setsize = 0;

  for (state = 0; state < *s1; state++) {
    nullstate_bool = false;
    recodelist(state, *t1, nullist, s_nullist, &nullstate_bool);
    if (nullstate_bool) {
      addstatelist(state, &nullstates_list, s_nullstates_list, &ok);
      if (ok)
        s_nullstates_list++;
      reorderlist((*t1)[state].next, (*t1)[state].numelts, &orderlist,
                  &s_orderlist);
      free((*t1)[state].next);
      (*t1)[state].next = orderlist;
      (*t1)[state].numelts = s_orderlist;
      s_orderlist = 0;
      orderlist = NULL;
    }
    (*t1)[state].reached = false;
  }

  if (s_nullstates_list == 0)
    return;

  nullsets = (part_node *)CALLOC(*s1, sizeof(part_node));
  if (nullsets == NULL) {
    mem_result = 1;
    goto FREE_ALL;
  }

  for (state = 0; state < *s1; state++) {
    nullsets[state].numelts = 1;
    nullsets[state].next = (INT_S *)MALLOC(sizeof(INT_S));
    if (nullsets[state].next == NULL) {
      mem_result = 1;
      goto FREE_ALL;
    }
    nullsets[state].next[0] = state;
  }

  for (i = 0; i < s_nullstates_list; i++) {
    state = nullstates_list[i];
    build_nullsets(state, *t1, nullsets);
    for (state = 0; state < *s1; state++)
      (*t1)[state].reached = false;
  }

  s_macrosets = 1;
  macrosets = (state_map *)MALLOC(sizeof(state_map));
  if (macrosets == NULL) {
    mem_result = 1;
    goto FREE_ALL;
  }

  macrosets[0].state = 0;
  macrosets[0].numelts = 0;
  mark_newstate(nullsets[0].next, nullsets[0].numelts, *t1, &marked);
  macrosets[0].marked = marked;
  macrosets[0].next = NULL;

  /* Copy nullsets[0].next -> macrosets[0].next */
  for (i = 0; i < nullsets[0].numelts; i++) {
    addstatelist(nullsets[0].next[i], &macrosets[0].next, macrosets[0].numelts,
                 &ok);
    if (ok)
      macrosets[0].numelts++;
  }

  templist = NULL;
  s_templist = 0;

  generate(nullsets[0].next, nullsets[0].numelts, t1, &templist, &s_templist);
  if (templist == NULL) {
    /* Free all pointer -> do a "goto" free part */
    s2 = 1;
    t2 = newdes(s2);
    t2[0].marked = macrosets[0].marked;
    freedes(*s1, t1);
    *s1 = s2;
    *t1 = t2;
    s2 = 0;
    t2 = NULL; /* Goto free part of the procedure */
    goto FREE_ALL;
  }

  srcstate = 0;
  newstate = 0;
  s2 = 0;
  t2 = NULL;

  do {
    for (i = 0; i < s_templist; i++) {
      curr_row = templist[i].state;
      for (j = 0; j < templist[i].numelts; j++) {
        k = templist[i].next[j];
        unionsets(nullsets[k].next, nullsets[k].numelts, &tempset, &setsize);
      }
      memberset(tempset, setsize, macrosets, s_macrosets, &macrostate);
      if (macrostate == -1) {
        newstate++;
        mark_newstate(tempset, setsize, *t1, &marked);
        addordlist3(newstate, setsize, marked, tempset, &macrosets,
                    &s_macrosets);
        insertlist4(srcstate, (INT_T)curr_row, newstate, &s2, &t2);
      } else {
        if (macrostate != -2) {
          insertlist4(srcstate, (INT_T)curr_row, macrostate, &s2, &t2);
        }
      }

      if (tempset != NULL) {
        free(tempset);
        tempset = NULL;
      }
      setsize = 0;
    }
    srcstate++;

    /* Some stuff here */
    found = false;
    for (i = 0; i < s_macrosets; i++) {
      if (macrosets[i].state == srcstate) {
        found = true;
        macroptr = i;
        break;
      }
    }

    if (found) {
      if (s_templist > 0) {
        free(templist);
        templist = NULL;
        s_templist = 0;
      }
      generate(macrosets[macroptr].next, macrosets[macroptr].numelts, t1,
               &templist, &s_templist);
    }
  } while (srcstate <= newstate);

  resize_des(&t2, s2, newstate + 1);
  s2 = newstate + 1;

  for (i = 0; i < s_macrosets; i++) {
    t2[macrosets[i].state].marked = macrosets[i].marked;
  }

  freedes(*s1, t1);
  *s1 = s2;
  *t1 = t2;

  /* Free all tempory variables here */
FREE_ALL:
  free(nullstates_list);
  free(orderlist);
  free(nullsets);
  free(macrosets);
  free(templist);
  free(tempset);
}

void project0(INT_S *s1, state_node **t1, INT_T s_nullist, INT_T *nullist) {
  INT_T *list, s_list, i;
  INT_B ok;

  list = NULL;
  s_list = 0;
  if (s_nullist == 0) {
    reach(s1, t1);
    minimize(s1, t1);
  } else {

    /* Ad-hoc method to improve runtime of project */
    if (*s1 < 100) {
      /* Do what I did before */
      project1(s1, t1, s_nullist, nullist);
      if (*s1 > 1) {
        reach(s1, t1);
        minimize(s1, t1);
      }
    } else if (*s1 < 1000) {
      for (i = 0; i < s_nullist; i += 2) {
        addordlist(nullist[i], &list, s_list, &ok);
        if (ok)
          s_list++;

        if (i + 1 < s_nullist) {
          addordlist(nullist[i + 1], &list, s_list, &ok);
          if (ok)
            s_list++;
        }

        project1(s1, t1, s_list, list);

        if (*s1 > 1) {
          reach(s1, t1);
          minimize(s1, t1);
        }
        free(list);
        s_list = 0;
        list = NULL;
      }
    } else {
      for (i = 0; i < s_nullist; i++) {
        addordlist(nullist[i], &list, s_list, &ok);
        if (ok)
          s_list++;

        project1(s1, t1, s_list, list);
        if (*s1 > 1) {
          reach(s1, t1);
          minimize(s1, t1);
        }
        free(list);
        s_list = 0;
        list = NULL;
      }
    }
  }

  free(list);
}
void plain_project_proc(INT_S *s1, state_node **t1, INT_T s_nullist,
                        INT_T *nullist) {
  INT_T *list, s_list, i;
  INT_B ok;

  list = NULL;
  s_list = 0;
  if (s_nullist == 0) {
    // reach(s1, t1);
    // minimize(s1, t1);
  } else {

    /* Ad-hoc method to improve runtime of project */
    if (*s1 < 100) {
      /* Do what I did before */
      project1(s1, t1, s_nullist, nullist);
      if (*s1 > 1) {
        // reach(s1, t1);
        // minimize(s1, t1);
      }
    } else if (*s1 < 1000) {
      for (i = 0; i < s_nullist; i += 2) {
        addordlist(nullist[i], &list, s_list, &ok);
        if (ok)
          s_list++;

        if (i + 1 < s_nullist) {
          addordlist(nullist[i + 1], &list, s_list, &ok);
          if (ok)
            s_list++;
        }

        project1(s1, t1, s_list, list);

        if (*s1 > 1) {
          // reach(s1, t1);
          // minimize(s1, t1);
        }
        free(list);
        s_list = 0;
        list = NULL;
      }
    } else {
      for (i = 0; i < s_nullist; i++) {
        addordlist(nullist[i], &list, s_list, &ok);
        if (ok)
          s_list++;

        project1(s1, t1, s_list, list);
        if (*s1 > 1) {
          // reach(s1, t1);
          // minimize(s1, t1);
        }
        free(list);
        s_list = 0;
        list = NULL;
      }
    }
  }

  free(list);
}
void project_selfloop(INT_S *s1, state_node **t1, INT_T s_nullist,
                      INT_T *nullist) {
  INT_S state, i;
  INT_B nullstate_bool;
  INT_S *nullstates_list, s_nullstates_list;
  tran_node *orderlist;
  INT_T s_orderlist;
  part_node *nullsets;
  INT_B ok, marked;
  state_map *macrosets;
  INT_S s_macrosets;
  INT_S s2, s3;
  state_node *t2, *t3;
  state_map *templist;
  INT_S s_templist;
  INT_S *tempset;
  INT_S setsize;
  INT_S macrostate;
  INT_S j, k;
  INT_S srcstate, newstate, curr_row;
  INT_B found;
  INT_S macroptr;
  INT_T ee;

  nullstates_list = NULL;
  s_nullstates_list = 0;
  orderlist = NULL;
  s_orderlist = 0;
  nullsets = NULL;
  macrosets = NULL;
  s_macrosets = 0;
  t2 = t3 = NULL;
  s2 = s3 = 0;
  templist = NULL;
  s_templist = 0;
  tempset = NULL;
  setsize = 0;

  export_copy_des(&s3, &t3, *s1, *t1);

  for (state = 0; state < *s1; state++) {
    nullstate_bool = false;
    recodelist(state, *t1, nullist, s_nullist, &nullstate_bool);
    if (nullstate_bool) {
      addstatelist(state, &nullstates_list, s_nullstates_list, &ok);
      if (ok)
        s_nullstates_list++;
      reorderlist((*t1)[state].next, (*t1)[state].numelts, &orderlist,
                  &s_orderlist);
      free((*t1)[state].next);
      (*t1)[state].next = orderlist;
      (*t1)[state].numelts = s_orderlist;
      s_orderlist = 0;
      orderlist = NULL;
    }
    (*t1)[state].reached = false;
  }

  if (s_nullstates_list == 0)
    return;

  nullsets = (part_node *)CALLOC(*s1, sizeof(part_node));
  if (nullsets == NULL) {
    mem_result = 1;
    goto FREE_ALL;
  }

  for (state = 0; state < *s1; state++) {
    nullsets[state].numelts = 1;
    nullsets[state].next = (INT_S *)MALLOC(sizeof(INT_S));
    if (nullsets[state].next == NULL) {
      mem_result = 1;
      goto FREE_ALL;
    }
    nullsets[state].next[0] = state;
  }

  for (i = 0; i < s_nullstates_list; i++) {
    state = nullstates_list[i];
    build_nullsets(state, *t1, nullsets);
    for (state = 0; state < *s1; state++)
      (*t1)[state].reached = false;
  }

  s_macrosets = 1;
  macrosets = (state_map *)MALLOC(sizeof(state_map));
  if (macrosets == NULL) {
    mem_result = 1;
    goto FREE_ALL;
  }

  macrosets[0].state = 0;
  macrosets[0].numelts = 0;
  mark_newstate(nullsets[0].next, nullsets[0].numelts, *t1, &marked);
  macrosets[0].marked = marked;
  macrosets[0].next = NULL;

  /* Copy nullsets[0].next -> macrosets[0].next */
  for (i = 0; i < nullsets[0].numelts; i++) {
    addstatelist(nullsets[0].next[i], &macrosets[0].next, macrosets[0].numelts,
                 &ok);
    if (ok)
      macrosets[0].numelts++;
  }

  templist = NULL;
  s_templist = 0;

  generate(nullsets[0].next, nullsets[0].numelts, t1, &templist, &s_templist);
  if (templist == NULL) {
    /* Free all pointer -> do a "goto" free part */
    s2 = 1;
    t2 = newdes(s2);
    t2[0].marked = macrosets[0].marked;
    freedes(*s1, t1);
    *s1 = s2;
    *t1 = t2;
    s2 = 0;
    t2 = NULL; /* Goto free part of the procedure */
    goto FREE_ALL;
  }

  srcstate = 0;
  newstate = 0;
  s2 = 0;
  t2 = NULL;

  do {
    for (i = 0; i < s_templist; i++) {
      curr_row = templist[i].state;
      for (j = 0; j < templist[i].numelts; j++) {
        k = templist[i].next[j];
        unionsets(nullsets[k].next, nullsets[k].numelts, &tempset, &setsize);
      }
      memberset(tempset, setsize, macrosets, s_macrosets, &macrostate);
      if (macrostate == -1) {
        newstate++;
        mark_newstate(tempset, setsize, *t1, &marked);
        addordlist3(newstate, setsize, marked, tempset, &macrosets,
                    &s_macrosets);
        insertlist4(srcstate, (INT_T)curr_row, newstate, &s2, &t2);
      } else {
        if (macrostate != -2) {
          insertlist4(srcstate, (INT_T)curr_row, macrostate, &s2, &t2);
        }
      }

      if (tempset != NULL) {
        free(tempset);
        tempset = NULL;
      }
      setsize = 0;
    }
    srcstate++;

    /* Some stuff here */
    found = false;
    for (i = 0; i < s_macrosets; i++) {
      if (macrosets[i].state == srcstate) {
        found = true;
        macroptr = i;
        break;
      }
    }

    if (found) {
      if (s_templist > 0) {
        free(templist);
        templist = NULL;
        s_templist = 0;
      }
      generate(macrosets[macroptr].next, macrosets[macroptr].numelts, t1,
               &templist, &s_templist);
    }
  } while (srcstate <= newstate);

  resize_des(&t2, s2, newstate + 1);
  s2 = newstate + 1;

  for (i = 0; i < s_macrosets; i++) {
    t2[macrosets[i].state].marked = macrosets[i].marked;
  }

  for (i = 0; i < s_macrosets; i++) {
    newstate = macrosets[i].state;
    for (j = 0; j < macrosets[i].numelts; j++) {
      state = macrosets[i].next[j];
      for (k = 0; k < t3[state].numelts; k++) {
        ee = t3[state].next[k].data1;
        if (inlist(ee, nullist, s_nullist) &&
            (!inordlist1(ee, t2[newstate].next, t2[newstate].numelts))) {
          addordlist1(ee, newstate, &(t2[newstate].next), t2[newstate].numelts,
                      &ok);
          if (ok)
            t2[newstate].numelts++;
        }
      }
    }
  }

  freedes(s3, &t3);
  freedes(*s1, t1);
  *s1 = s2;
  *t1 = t2;

  /* Free all tempory variables here */
FREE_ALL:
  free(nullstates_list);
  free(orderlist);
  free(nullsets);
  free(macrosets);
  free(templist);
  free(tempset);
}
void project_proc_selfloop(INT_S *s1, state_node **t1, INT_T s_nullist,
                           INT_T *nullist) {
  INT_T *list, s_list, i;
  INT_B ok;

  list = NULL;
  s_list = 0;
  if (s_nullist == 0) {
    reach(s1, t1);
    minimize(s1, t1);
  } else {

    /* Ad-hoc method to improve runtime of project */
    if (*s1 < 100) {
      /* Do what I did before */
      project_selfloop(s1, t1, s_nullist, nullist);
      if (*s1 > 1) {
        reach(s1, t1);
        minimize(s1, t1);
      }
    } else if (*s1 < 1000) {
      for (i = 0; i < s_nullist; i += 2) {
        addordlist(nullist[i], &list, s_list, &ok);
        if (ok)
          s_list++;

        if (i + 1 < s_nullist) {
          addordlist(nullist[i + 1], &list, s_list, &ok);
          if (ok)
            s_list++;
        }

        project_selfloop(s1, t1, s_list, list);

        if (*s1 > 1) {
          reach(s1, t1);
          minimize(s1, t1);
        }
        free(list);
        s_list = 0;
        list = NULL;
      }
    } else {
      for (i = 0; i < s_nullist; i++) {
        addordlist(nullist[i], &list, s_list, &ok);
        if (ok)
          s_list++;

        project_selfloop(s1, t1, s_list, list);
        if (*s1 > 1) {
          reach(s1, t1);
          minimize(s1, t1);
        }
        free(list);
        s_list = 0;
        list = NULL;
      }
    }
  }

  free(list);
}
void condat1(state_node *t1, INT_S s1, INT_S s2, INT_S s3, state_node *t3,
             INT_S *s4, state_node **t4, INT_S *macro_c) {
  INT_S state, state1, state2;

  *s4 = s2;
  *t4 = newdes(*s4);

  for (state = 0; state < s3; state++) {
    state1 = macro_c[state] % s1;
    state2 = macro_c[state] / s1;

    genlist(t1[state1].numelts, t1[state1].next, t3[state].numelts,
            t3[state].next, state2, *t4);
  }
}

void b_recode(INT_S s1, state_node **t1, INT_S *s2, INT_S **recode_array) {
  INT_T cur, diff, numelts;
  INT_S s;
  INT_S es;
  t_queue tq;
  /*   long num_hits = 0; */
  state_node *t2;

  if (s1 <= 0)
    return;

  *s2 = 0;

  *recode_array = (INT_S *)MALLOC(s1 * sizeof(INT_S));
  if (*recode_array == NULL) {
    mem_result = 1;
    return;
  }

  for (s = 0; s < s1; s++)
    (*t1)[s].reached = false;
  (*t1)[0].reached = true;

  queue_init(&tq);

  s = 0;
  enqueue(&tq, s);
  (*recode_array)[*s2] = s;
  (*s2)++;

  while (!queue_empty(&tq)) {
    s = dequeue(&tq);
    for (cur = 0; cur < (*t1)[s].numelts; cur++) {
      es = (*t1)[s].next[cur].data2;
      if (!(*t1)[es].reached) {
        enqueue(&tq, es);
        (*recode_array)[es] = *s2;
        (*s2)++;
        (*t1)[es].reached = true;
      }
    }
  }

  queue_done(&tq);

  /* Recode the transitions - based on the recode array */
  for (s = 0; s < s1; s++) {
    if ((*t1)[s].reached) {
      /* Purge transitions that point to dead states */
      cur = 0;
      numelts = (*t1)[s].numelts;
      while (cur < (*t1)[s].numelts) {
        es = (*t1)[s].next[cur].data2;
        if (!(*t1)[es].reached) {
          diff = (*t1)[s].numelts - (cur + 1);
          if (diff > 0)
            memmove(&(*t1)[s].next[cur], &(*t1)[s].next[cur + 1],
                    sizeof(tran_node) * diff);
          (*t1)[s].numelts--;
        } else {
          (*t1)[s].next[cur].data2 = (*recode_array)[es];
          cur++;
        }

        if (numelts != (*t1)[s].numelts) {
          (*t1)[s].next = (tran_node *)REALLOC(
              (*t1)[s].next, sizeof(tran_node) * (*t1)[s].numelts);
        }
      }
    } else {
      if ((*t1)[s].next != NULL)
        free((*t1)[s].next);
      (*t1)[s].next = NULL;
      (*t1)[s].numelts = 0;
    }
  }

  t2 = newdes(*s2);
  if ((*s2 != 0) && (t2 == NULL)) {
    mem_result = 1;
    return;
  }

  for (s = 0; s < *s2; s++) {
    es = (*recode_array)[s];
    t2[es].marked = (*t1)[s].marked;
    t2[es].vocal = (*t1)[s].vocal;
    t2[es].next = (*t1)[s].next;
    t2[es].numelts = (*t1)[s].numelts;
  }

  free(*t1);
  *t1 = t2;
}

void reversetran1(INT_S state, state_node *t3, state_node *t) {
  INT_T j, event;
  INT_S target;
  INT_B ok;

  for (j = 0; j < t3[state].numelts; j++) {
    event = t3[state].next[j].data1;
    target = t3[state].next[j].data2;
    addordlist1(event, state, &t[target].next, t[target].numelts, &ok);
    if (ok)
      t[target].numelts++;
  }
}

void ya_coreach2(INT_S state, state_node *t1, state_node *t2) {
  INT_B ok;
  INT_T cur, event;
  INT_S s;
  INT_S es;
  t_stack ts;

  pstack_Init(&ts);

  cur = 0;
  s = state;

  while (!((cur >= t2[s].numelts) && pstack_IsEmpty(&ts))) {
    es = t2[s].next[cur].data2;
    if (!t2[es].reached) {
      event = t2[s].next[cur].data1;
      delete_ordlist1(event, state, &t1[es].next, t1[es].numelts, &ok);
      if (ok)
        t1[es].numelts--;

      if ((event % 2) == 0) {
        t2[es].reached = true;
        t1[es].reached = true;
        ok = pstack_Push(&ts, cur, s);
        s = es;
        cur = 0;
        goto LABEL1;
      }
    }
    cur++;
  LABEL1:
    if (cur >= t2[s].numelts) {
      do {
        pstack_Pop(&ts, &cur, &s, &ok);
        if (cur < t2[s].numelts && ok)
          cur++;
      } while (!((cur < t2[s].numelts) || pstack_IsEmpty(&ts)));
    }
  }

  pstack_Done(&ts);
}

void mutex1(INT_S *s3, state_node **t3, INT_S s1, INT_S s2, INT_S *macro_ab,
            state_pair *badlist, INT_S s_badlist) {
  INT_S i, macrostate;
  INT_S *mutexlist, s_mutexlist;
  INT_S badstate;
  INT_B ok;
  INT_S substate1, substate2;
  state_node *ttemp;
  INT_S state, s4;
  recode_node *recode_states;
  INT_S num_reachable, stemp;

  stemp = 0;
  ttemp = NULL;

  mutexlist = NULL;
  s_mutexlist = 0;

  if (*s3 == 0) {
    return;
  }

  for (i = 0; i < s_badlist; i++) {
    substate1 = badlist[i].data1;
    substate2 = badlist[i].data2;
    if ((0 <= substate1 && substate1 < s1) &&
        (0 <= substate2 && substate2 < s2)) {
      macrostate = macro_ab[substate2 * s1 + substate1];
      if (macrostate != -1L) {
        addstatelist(macrostate, &mutexlist, s_mutexlist, &ok);
        if (ok)
          s_mutexlist++;
      }
    }
  }

  if (mutexlist == NULL) {
    return;
  }

  (*t3)[0].reached = false;
  for (i = 0; i < s_mutexlist; i++) {
    badstate = mutexlist[i];
    (*t3)[badstate].reached = true;
    if ((*t3)[badstate].next != NULL) {
      free((*t3)[badstate].next);
      (*t3)[badstate].next = NULL;
    }
    (*t3)[badstate].numelts = 0;
  }

  if ((*t3)[0].reached) {
    free(mutexlist);
    freedes(*s3, t3);
    *s3 = 0;
    *t3 = NULL;
    return;
  }

  /* do some kind of reachability on t3 so that
     certain states are not marked as reach */
  /* Do some recoding on the states left */
  /* Operations very similar to TRIM1 but instead of taking
     the reachable, take the NOT reachable */

  stemp = *s3;
  ttemp = newdes(stemp);
  if (ttemp == NULL) {
    free(mutexlist);
    mem_result = 1;
    return;
  }

  for (i = 0; i < *s3; i++)
    reversetran1(i, *t3, ttemp);

  for (i = 0; i < s_mutexlist; i++) {
    for (state = 0; state < *s3; state++)
      ttemp[state].reached = false;
    badstate = mutexlist[i];
    ttemp[*s3 - 1].reached = true;
    ya_coreach2(badstate, *t3, ttemp);
  }

  freedes(stemp, &ttemp);
  free(mutexlist);

  if ((*t3)[0].reached) {
    freedes(*s3, t3);
    *s3 = 0;
    *t3 = NULL;
    return;
  }

  s4 = 0;
  for (state = 0; state < *s3; state++) {
    /* Switch all the reach variable to its complement */
    (*t3)[state].reached = !((*t3)[state].reached);
    if ((*t3)[state].reached)
      s4++;
  }

  if (s4 == *s3)
    return;

  /* Recode all "unreachable states" */
  recode_states = (recode_node *)CALLOC(*s3, sizeof(recode_node));

  /* Re-name all reached states to the new state names */
  num_reachable = 0;
  for (state = 0; state < *s3; state++) {
    if ((*t3)[state].reached) {
      recode_states[state].recode = num_reachable;
      recode_states[state].reached = true;
      num_reachable++;
    }
  }

  /* Purge dead transitions followed by purging states */
  recode(*s3, t3, recode_states);

  *t3 = (state_node *)REALLOC(*t3, sizeof(state_node) * num_reachable);
  *s3 = num_reachable;

  free(recode_states);
}

INT_B checkdet(INT_S s1, state_node *t1) {
  INT_S i, s;
  INT_T j, cur;

  for (i = 0; i < s1; i++) {
    cur = (INT_T)(-1);
    for (j = 0; j < t1[i].numelts; j++) {
      if (cur == t1[i].next[j].data1) {
        return false;
      } else {
        cur = t1[i].next[j].data1;
      }

      /* Make sure data2 is valid */
      s = t1[i].next[j].data2;
      if ((s < 0) || (s >= s1)) {
        return false;
      }
    }
  }
  return true;
}

void expand_des_vocal(state_node **t1, INT_S *s1, quad__t **list,
                      INT_S *slist) {
  /* Increase the number of states by the number of unique
     [entry,vocal_output] pairs */

  state_pair *pl;
  INT_S s_pl;
  INT_S cur;
  INT_B ok;
  INT_S old_max, new_max;

  pl = NULL;
  s_pl = 0;

  cur = 0;
  for (cur = 0; cur < *slist; cur++) {
    addstatepair((*list)[cur].c, (*list)[cur].d, &pl, s_pl, &ok);
    if (ok) {
      s_pl++;
    }
  }

  free(pl);

  /* s_pl = # of new states to add */
  old_max = *s1;
  new_max = *s1 + s_pl;

  *t1 = (state_node *)REALLOC(*t1, sizeof(state_node) * new_max);
  if ((new_max != 0) && (*t1 == NULL)) {
    mem_result = 1;
    return;
  }

  for (cur = old_max; cur < new_max; cur++) {
    (*t1)[cur].numelts = 0;
    (*t1)[cur].marked = true;
    (*t1)[cur].vocal = 0;
    (*t1)[cur].reached = false;
    (*t1)[cur].next = NULL;
  }

  *s1 = new_max;
}

INT_B inlist_quad(quad__t *list, INT_S slist, INT_S i, INT_T e, INT_S j,
                  INT_V *v) {
  INT_S cur;

  for (cur = 0; cur < slist; cur++) {
    if ((list[cur].a == i) && (list[cur].b == e) && (list[cur].c == j)) {
      *v = list[cur].d;
      return true;
    }
  }

  return false;
}

void vocalize_des(state_node **t1, INT_S *s1, quad__t **list, INT_S *slist) {
  INT_S size, cur, j;
  quad__t *state_list;
  INT_S s_state_list;
  quad__t *one_state_list;
  INT_S s_one_state_list;
  INT_S *vocal_list;
  INT_S s_vocal_list;
  INT_S i;
  INT_B ok;
  INT_S start_state, new_state;
  INT_T k, ee;
  INT_S ii, jj, entry;
  INT_V vocal_output;
  INT_S *recode_array;
  INT_S s2;

  state_list = NULL;
  s_state_list = 0;
  one_state_list = NULL;
  s_one_state_list = 0;
  vocal_list = NULL;
  s_vocal_list = 0;
  recode_array = NULL;
  s2 = 0;

  size = *s1;
  start_state = *s1;
  expand_des_vocal(t1, s1, list, slist);

  for (i = 0; i < size; i++) {
    free(state_list);
    state_list = NULL;
    s_state_list = 0;
    free(vocal_list);
    vocal_list = NULL;
    s_vocal_list = 0;

    /* Get a list of significant events entering state "i" */
    for (cur = 0; cur < *slist; cur++) {
      if ((*list)[cur].c == i) {
        add_quad((*list)[cur].a, (*list)[cur].b, (*list)[cur].c, (*list)[cur].d,
                 &state_list, s_state_list, &ok);
        if (ok)
          s_state_list++;
      }
    }

    if (s_state_list > 0) {
      /* Count # of unique vocal output.
         Stored in "s_vocal_list */
      for (cur = 0; cur < s_state_list; cur++) {
        addstatelist(state_list[cur].d, &vocal_list, s_vocal_list, &ok);
        if (ok)
          s_vocal_list++;
      }

      for (j = 0; j < s_vocal_list; j++) {
        new_state = start_state + j;
        (*t1)[new_state].marked = (*t1)[i].marked;
        (*t1)[new_state].vocal = (INT_V)vocal_list[j];

        /* Do we need to update the user_list and state_list? */
        for (k = 0; k < (*t1)[i].numelts; k++) {
          /* Self-loop? */
          ee = (*t1)[i].next[k].data1;
          jj = (*t1)[i].next[k].data2;

          if (jj == i) {
            entry = i;
          } else {
            entry = jj;
          }

          if (inlist_quad(*list, *slist, i, ee, jj, &vocal_output)) {
            add_quad(new_state, ee, jj, vocal_output, list, *slist, &ok);
            if (ok)
              (*slist)++;

            if (jj == i) {
              add_quad(new_state, ee, jj, vocal_output, &state_list,
                       s_state_list, &ok);
              if (ok)
                (s_state_list)++;
            }
          }

          addordlist1(ee, entry, &(*t1)[new_state].next,
                      (*t1)[new_state].numelts, &ok);
          if (ok)
            (*t1)[new_state].numelts++;
        }
      }

      for (j = 0; j < s_vocal_list; j++) {
        /* Construct a list so that they have the same vocal output */
        free(one_state_list);
        one_state_list = NULL;
        s_one_state_list = 0;
        for (cur = 0; cur < s_state_list; cur++) {
          if (state_list[cur].d == vocal_list[j]) {
            add_quad(state_list[cur].a, state_list[cur].b, state_list[cur].c,
                     state_list[cur].d, &one_state_list, s_one_state_list, &ok);
            if (ok)
              s_one_state_list++;
          }
        }

        for (cur = 0; cur < s_one_state_list; cur++) {
          ii = one_state_list[cur].a;
          delete_ordlist1(one_state_list[cur].b, one_state_list[cur].c,
                          &(*t1)[ii].next, (*t1)[ii].numelts, &ok);
          if (ok)
            (*t1)[ii].numelts--;

          addordlist1(one_state_list[cur].b, start_state + j, &(*t1)[ii].next,
                      (*t1)[ii].numelts, &ok);
          if (ok)
            (*t1)[ii].numelts++;
        }
      }
    }

    start_state += s_vocal_list;
  }

  free(state_list);
  free(one_state_list);
  free(vocal_list);

  reach(s1, t1);
  b_recode(*s1, t1, &s2, &recode_array);
  free(recode_array);
}

INT_S num_marked(state_node *t, INT_S s) {
  INT_S i, count;

  count = 0;
  for (i = 0; i < s; i++) {
    if (t[i].marked)
      count++;
  }
  return count;
}

void iso1(INT_S s1, INT_S s2, state_node *t1, state_node *t2, INT_B *inflag,
          INT_S *mapState) {
  state_pair *iEj;
  INT_S s_iEj;
  /* Pre-conditions:
     Same number of states.
     Same number of transitions.
     Same number of marker states.
     Same number of vocal states
   */
  INT_B flag, equal;
  INT_S count1, count2;
  INT_S t1i, t2j;

  iEj = NULL;
  s_iEj = 0;

  flag = (s1 == s2);
  if (flag) {
    count1 = num_marked(t1, s1);
    count2 = num_marked(t2, s2);
    flag = (count1 == count2);
  }
  if (flag) {
    count1 = count_tran(t1, s1);
    count2 = count_tran(t2, s2);
    flag = (count1 == count2);
  }
  if (flag) {
    if ((s1 == 0) && (s2 == 0)) {
      flag = true;
    } else {
      equal = false;
      if (iEj != NULL) {
        free(iEj);
        iEj = NULL;
        s_iEj = 0;
      }
      compare_states(0L, 0L, &equal, mapState, &iEj, &s_iEj, t1, t2);
      if (equal) {
        mapState[0] = 0;
        t1i = 1;
        while ((t1i < s1) && equal) {
          if (!t1[t1i].reached) {
            equal = false;
            t2j = 1;
            while ((t2j < s2) && (!equal)) {
              if (!t2[t2j].reached) {
                if (iEj != NULL) {
                  free(iEj);
                  iEj = NULL;
                  s_iEj = 0;
                }
                compare_states(t1i, t2j, &equal, mapState, &iEj, &s_iEj, t1,
                               t2);

                if (equal) {
                  mapState[t1i] = t2j;
                  t1[t1i].reached = true;
                  t2[t2j].reached = true;
                  t1i++;
                  t2j = 1;
                } else {
                  t2j++;
                }
              } else {
                t2j++;
              }
            }
          } else {
            t1i++;
          }
        }
      }
      flag = equal;
    }
  }

  *inflag = flag;

  if (iEj != NULL) {
    free(iEj);
    iEj = NULL;
    s_iEj = 0;
  }
}

INT_S count_tran(state_node *t1, INT_S s1) {
  INT_S count, i;
  count = 0;

  for (i = 0; i < s1; i++) {
    count += t1[i].numelts;
  }
  return count;
}

INT_B compute_controllable(state_node *t1, INT_S s1) {
  INT_S ii;
  INT_T jj;

  for (ii = 0; ii < s1; ii++) {
    for (jj = 0; jj < t1[ii].numelts; jj++) {
      if ((t1[ii].next[jj].data1 % 2) == 0) /* Even */
        return false;
    }
  }

  return true;
}

void allevent_des(state_node **t1, INT_S *s1, state_node **t2, INT_S *s2) {
  INT_S i;
  INT_T j;
  INT_B ok;

  if (*s1 < 0)
    return;

  *s2 = 1;
  *t2 = newdes(*s2);
  (*t2[0]).marked = true;

  for (i = 0; i < *s1; i++) {
    for (j = 0; j < (*t1)[i].numelts; j++) {
      addordlist1((*t1)[i].next[j].data1, 0, &(*t2)[0].next, (*t2)[0].numelts,
                  &ok);
      if (ok)
        (*t2[0]).numelts++;
    }
  }
}

void supclo_des(state_node **t1, INT_S *s1, state_node **t2, INT_S *s2) {
  INT_B found, ok;
  INT_S *statelist;
  INT_S s_statelist;
  INT_S i;
  INT_T j;

  statelist = NULL;
  s_statelist = 0;

  *s2 = 0;

  if (*s1 == 0)
    return; /* If DES1 = EMPTY the result is EMPTY */

  if ((*t1)[0].marked == false)
    return; /* if initial state is not marked then result is EMPTY */

  found = false;
  for (i = 1; i < *s1; i++) {
    if ((*t1)[i].marked == true) {
      found = true;
      break;
    }
  }

  if (!found) {
    /* If only the initial state is marked then result is the initial state */
    *s2 = 1;
    *t2 = newdes(*s2);
    (*t2)[0].marked = true;

    /* Also, all transitions selflooped [0,e,0] */
    for (j = 0; j < (*t1)[0].numelts; j++) {
      if ((*t1)[0].next[j].data2 == 0) {
        addordlist1((*t1)[0].next[j].data1, 0, &(*t2)[0].next, (*t2)[0].numelts,
                    &ok);
        if (ok)
          (*t2)[0].numelts++;
      }
    }

    return;
  }

  found = true;
  for (i = 0; i < *s1; i++) {
    if ((*t1)[i].marked == false) {
      found = false;
      break;
    }
  }

  if (found) {
    /* If all states are marked then result is same as input DES */
    *s2 = *s1;
    *t2 = *t1;
    *s1 = 0;
    *t1 = NULL;
    return;
  }

  /* Get a list of unmarked states */
  for (i = 0; i <= *s1; i++) {
    if ((*t1)[i].marked == false) {
      addstatelist(i, &statelist, s_statelist, &ok);
      if (ok)
        s_statelist++;
    }
  }

  /* Delete the transitions that point to unmarked states */
  for (i = 0; i < *s1; i++) {
    for (j = 0; j < (*t1)[i].numelts;) {
      ok = instatelist((*t1)[i].next[j].data2, statelist, s_statelist);
      if (ok) {
        delete_ordlist1((*t1)[i].next[j].data1, (*t1)[i].next[j].data2,
                        &(*t1)[i].next, (*t1)[i].numelts, &ok);
        if (ok)
          (*t1)[i].numelts--;
      } else {
        j++;
      }
    }
  }

  /* Trim the result */
  trim1(s1, t1);

  *s2 = *s1;
  *t2 = *t1;
  *s1 = 0;
  *t1 = NULL;
}

INT_OS force_proc(char *name1, char *name2, INT_T s_force_event_list,
                  INT_T *force_event_list, INT_T s_preempt_event_list,
                  INT_T *preempt_event_list, INT_T timeout_event) {
  INT_S s1, s2;
  state_node *t1, *t2;
  INT_S init, i, j;
  INT_OS result = 0;
  INT_T *l1, *l2, *l3;
  INT_T s_l1, s_l2, s_l3;
  INT_T event;
  INT_S newstate, state;
  INT_B ok;

  s1 = s2 = 0;
  t1 = t2 = NULL;

  l1 = l2 = l3 = NULL;
  s_l1 = s_l2 = s_l3 = 0;
  result = 0;

  init = 0L;
  if (getdes(name1, &s1, &init, &t1) == false)
    return -1;

  export_copy_des(&s2, &t2, s1, t1);
  if (mem_result == 1) {
    result = 3;
    goto FORCE_LABEL;
  }
  for (i = 0; i < s1; i++) {
    for (j = 0; j < t1[i].numelts; j++) {
      event = t1[i].next[j].data1;
      if (inlist(event, force_event_list, s_force_event_list)) {
        addordlist((INT_T)j, &l1, s_l1, &ok);
        if (ok)
          s_l1++;
      } else if (inlist(event, preempt_event_list, s_preempt_event_list)) {
        addordlist((INT_T)j, &l2, s_l2, &ok);
        if (ok)
          s_l2++;
      } else {
        addordlist((INT_T)j, &l3, s_l3, &ok);
        if (ok)
          s_l3++;
      }
    }
    if (mem_result == 1) {
      result = 3;
      goto FORCE_LABEL;
    }
    if (s_l1 != 0 && s_l2 != 0) {
      newstate = s2;
      insertlist4(i, timeout_event, newstate, &s2, &t2);
      for (j = 0; j < s_l2; j++) {
        event = t1[i].next[l2[j]].data1;
        state = t1[i].next[l2[j]].data2;
        insertlist4(newstate, event, state, &s2, &t2);
        delete_ordlist1(event, state, &t2[i].next, t2[i].numelts, &ok);
        if (ok)
          t2[i].numelts--;
      }

      for (j = 0; j < s_l3; j++) {
        event = t1[i].next[l3[j]].data1;
        state = t1[i].next[l3[j]].data2;
        if (state != i)
          insertlist4(newstate, event, state, &s2, &t2);
        else {
          insertlist4(newstate, event, newstate, &s2, &t2);
        }
      }
    }
    if (mem_result == 1) {
      result = 3;
      goto FORCE_LABEL;
    }
    free(l1);
    free(l2);
    free(l3);
    s_l1 = s_l2 = s_l3 = 0;
    l1 = l2 = l3 = NULL;
  }
  reach(&s2, &t2);

  if (mem_result != 1) {
    init = 0L;
    filedes(name2, s2, init, t2);
  }

FORCE_LABEL:
  freedes(s1, &t1);
  freedes(s2, &t2);
  free(l1);
  free(l2);
  free(l3);

  return result;
}

#ifdef __cplusplus
}
#endif
