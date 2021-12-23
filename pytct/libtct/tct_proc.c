#include "tct_proc.h"
#include "des_data.h"
#include "des_proc.h"
#include <stdio.h>

void print_des_stat_header(FILE *out, char *name, INT_S s, INT_S init) {
  fprintf(out, "%s    # states: %ld", name, s);
  if (s > 0) {
    fprintf(out, "    state set: 0 ... %ld", s - 1);
    fprintf(out, "    initial state: %ld\n", init);
  } else {
    fprintf(out, "    state set: empty ");
    fprintf(out, "    initial state: none\n");
  }
}

INT_B print_marker_states(FILE *out, state_node *t1, INT_S s1) {
  INT_S i, total_marker;

  total_marker = 0;

  fprintf(out, "marker states:       ");
  fprintf(out, "\n");
  fprintf(out, "\n");
  for (i = 0; i < s1; i++) {
    if (t1[i].marked) {
      fprintf(out, "%7ld ", i);
      total_marker++;
      if ((total_marker % 8) == 0) {
        fprintf(out, "\n");
      }
    }
  }
  fprintf(out, "\n");
  return false;
}

INT_B print_vocal_output(FILE *out, state_node *t1, INT_S s1) {
  INT_S i, total_vocal;

  total_vocal = 0;

  fprintf(out, "\n");
  fprintf(out, "vocal states:       \n");
  for (i = 0; i < s1; i++) {
    if (t1[i].vocal > 0) {
      /* Tempory format fix - for 7 digit numbers */
      if (s1 <= 100000)
        fprintf(out, "[%5ld,%4d]   ", i, t1[i].vocal);
      else if (s1 <= 1000000)
        fprintf(out, "[%6ld,%4d]  ", i, t1[i].vocal);
      else
        fprintf(out, "[%7ld,%4d] ", i, t1[i].vocal);

      total_vocal++;
      if ((total_vocal % 5) == 0) {
        fprintf(out, "\n");
      }
    }
  }
  fprintf(out, "\n");
  return false;
}

INT_B print_transitions(FILE *out, state_node *t1, INT_S s1) {
  INT_S i, total_tran;
  INT_T j;

  total_tran = 0;

  fprintf(out, "\n");
  fprintf(out, "# transitions: %ld\n", count_tran(t1, s1));
  fprintf(out, "\n");
  fprintf(out, "transitions: \n");
  fprintf(out, "\n");
  for (i = 0; i < s1; i++) {
    for (j = 0; j < t1[i].numelts; j++) {

      if (t1[i].next[j].data1 == EEE)
        fprintf(out, "[%5ld,  e,%5ld]  ", i, t1[i].next[j].data2);
      else
        fprintf(out, "[%5ld,%3d,%5ld]  ", i, t1[i].next[j].data1,
                t1[i].next[j].data2);

      total_tran++;
      if ((total_tran % 4) == 0) {
        fprintf(out, "\n");
      }
    }
  }
  return false;
}

INT_B nonconflict(INT_S s, state_node *t) {
  INT_S state;

  if (s == 0)
    return true;

  /* Make all the "reached" variable to false.
     This is needed because b_reach assumes this field is set to false
     before calling it. */
  for (state = 0; state < s; state++)
    t[state].reached = false;

  for (state = 0; state < s; state++) {
    if (t[state].marked) {
      t[state].reached = true;
      b_reach(t[state].next, state, &t, s);
    }
  }

  for (state = 0; state < s; state++) {
    if (!t[state].reached) {
      return false;
    }
  }
  return true;
}

void print_dat_header_stat(FILE *out, char *name1, INT_B controllable) {
  fprintf(out, "%s\n", name1);
  fprintf(out, "\n\n");
  fprintf(out, "Control data are displayed as a list of supervisor states\n");
  fprintf(out, "where disabling occurs, together with the events that must\n");
  fprintf(out, "be disabled there.\n\n");
  fprintf(out, "%s is ", name1);
  if (controllable)
    fprintf(out, "CONTROLLABLE");
  else
    fprintf(out, "NOT CONTROLLABLE");
  fprintf(out, "\n\n");
  fprintf(out, "control data:\n");
}

INT_B print_dat(FILE *out, state_node *t1, INT_S s1) {
  INT_S k, i, prevNumTran;
  INT_T j;
  INT_B leftSide;

  leftSide = false;
  prevNumTran = 0;

  fprintf(out, "\n");
  for (i = 0; i < s1; i++) {
    if (t1[i].numelts > 0) {
      if ((prevNumTran > 6) || (t1[i].numelts > 6) || (leftSide == false)) {
        fprintf(out, "\n");
        fprintf(out, "%4s", " ");
        leftSide = true;
      } else {
        for (k = prevNumTran; k <= 6; k++)
          fprintf(out, "%5s", " ");
        leftSide = false;
      }
      fprintf(out, "%4ld:", i);
      prevNumTran = t1[i].numelts;
    }

    for (j = 0; j < t1[i].numelts; j++) {
      if (t1[i].next[j].data1 == EEE)
        fprintf(out, "e    ");
      else
        fprintf(out, "%4d ", t1[i].next[j].data1);

      if ((j != 0) && ((j % 12) == 0) && (j < prevNumTran - 1)) {
        fprintf(out, "\n");
        fprintf(out, "%9s", " ");
      }
    }
  }
  fprintf(out, "\n");
  return false;
}

void gen_complement_list(state_node *t1,
                         INT_S s1,
                         INT_T *imagelist, INT_T s_imagelist,
                         INT_T **list, INT_T *slist)
{
   INT_S i;
   INT_T j;
   INT_B  ok;

   for (i=0; i < s1; i++)
   {
      for (j=0; j < t1[i].numelts; j++)
      {
         if (!inlist(t1[i].next[j].data1, imagelist, s_imagelist))
         {
            addordlist(t1[i].next[j].data1, list, *slist, &ok);
            if (ok) (*slist)++;
         }
      }
   }
}