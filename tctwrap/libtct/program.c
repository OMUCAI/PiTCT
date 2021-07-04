#include "program.h"
#include "des_data.h"
#include "des_proc.h"
#include "tct_io.h"
#include "tct_proc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef char filename1[MAX_FILENAME];

static filename1 name1, name2, name3, name4, names1[MAX_DESS];
static char long_name1[MAX_FILENAME];
static char long_name2[MAX_FILENAME];
static char long_name3[MAX_FILENAME];
static char long_name4[MAX_FILENAME];

int create_program(const char *filename) {
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return -1;
  }

  // INT_T slist, *list;
  state_node *t;
  INT_S s, init;
  INT_OS ee, flag;
  INT_S i, j, k;
  INT_B ok;

  t = NULL;
  s = 0;

  /* Use "fgets" as names could have spaces in it */
  if (fgets(name1, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return -1;
  }
  name1[strlen(name1) - 1] = '\0';

  fscanf(f1, "%d\n", &s);

  t = newdes(s);

  while (fscanf(f1, "%d", &ee) != EOF) {
    if (ee == -1) {
      break;
    }
    t[ee].marked = true;
  }

  flag = 0;
  while (fscanf(f1, "%d", &ee) != EOF) {
    if (flag == 0) {
      flag = 1;
      i = ee;
      continue;
    } else if (flag == 1) {
      flag = 2;
      j = ee;
      continue;
    } else if (flag == 2) {
      flag = 0;
      k = ee;
      addordlist1((INT_T)j, k, &t[i].next, t[i].numelts, &ok);
      if (ok)
        t[i].numelts++;
      continue;
    }
  }

  fclose(f1);

  init = 0L;
  filedes(name1, s, init, t);

  if (mem_result == 1) {
    return -1;
  }
  freedes(s, &t);

  return 0;
}

int selfloop_program(const char *filename) {
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return -1;
  }

  state_node *t1;
  INT_S s1, init;
  INT_T *list, slist;
  INT_T e;
  INT_OS ee;
  INT_B ok;

  t1 = NULL;
  s1 = 0;
  list = NULL;
  slist = 0;

  /* Use "fgets" as names could have spaces in it */
  if (fgets(name1, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return -1;
  }
  name1[strlen(name1) - 1] = '\0';

  if (fgets(name2, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return -1;
  }
  name2[strlen(name2) - 1] = '\0';

  while (fscanf(f1, "%d", &ee) != EOF) {
    e = (INT_T)ee;
    addordlist(e, &list, slist, &ok);
    if (ok)
      slist++;
  }

  fclose(f1);

  init = 0L;
  getdes(name1, &s1, &init, &t1);
  selfloop_gentran(slist, list, s1, t1);

  if (mem_result != 1) {
    filedes(name2, s1, init, t1);
  } else {
    return -1;
  }

  return 0;
}

int trim_program(const char *filename) {
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return -1;
  }

  state_node *t1;
  INT_S s1, init; //,i;

  t1 = NULL;
  s1 = 0;
  /* Use "fgets" as names could have spaces in it */
  if (fgets(name1, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return -1;
  }
  name1[strlen(name1) - 1] = '\0';

  if (fgets(name2, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return -1;
  }
  name2[strlen(name2) - 1] = '\0';

  fclose(f1);

  init = 0L;
  getdes(name1, &s1, &init, &t1);

  trim1(&s1, &t1);

  if (mem_result != 1) {
    filedes(name2, s1, init, t1);
  } else {
    return -1;
  }

  return 0;
}

int printdes_program(const char *filename) {
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return -1;
  }

  INT_S result;
  INT_S init;
  FILE *out;
  INT_S s1;
  state_node *t1;

  s1 = 0;
  t1 = NULL;

  result = 0;
  /* Use "fgets" as names could have spaces in it */
  if (fgets(name1, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return -1;
  }
  name1[strlen(name1) - 1] = '\0';

  if (fgets(name2, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return -1;
  }
  name2[strlen(name2) - 1] = '\0';

  fclose(f1);

  init = 0L;
  getdes(name1, &s1, &init, &t1);

  make_filename_ext(long_name2, name2, EXT_TXT);
  out = fopen(long_name2, "w");

  print_des_stat_header(out, name1, s1, init);
  fprintf(out, "\n");

  if (num_mark_states(t1, s1) > 0) {
    print_marker_states(out, t1, s1);
  } else {
    fprintf(out, "marker states: none\n");
    fprintf(out, "\n");
  }

  if (num_vocal_output(t1, s1) > 0) {
    print_vocal_output(out, t1, s1);
  } else {
    fprintf(out, "\n");
    fprintf(out, "vocal states: none\n");
    fprintf(out, "\n");
  }

  if (count_tran(t1, s1) > 0) {
    print_transitions(out, t1, s1);
    fprintf(out, "\n");
  } else {
    fprintf(out, "transition table : empty\n");
  }

  fclose(f1);
  freedes(s1, &t1);

  if (result != 0) {
    return -1;
  }

  return 0;
}

int sync_program(const char *filename) {
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return -1;
  }

  state_node *t1, *t2, *t3;
  INT_S s1, s2, s3, init;
  INT_S *macro_ab, *macro_c;
  INT_OS num, i, j, k;
  INT_T s_tranlist, *tranlist;
  INT_B ok;

  macro_ab = NULL;
  macro_c = NULL;
  t1 = t2 = t3 = NULL;
  s1 = s2 = s3 = 0;
  s_tranlist = 0;
  tranlist = NULL;

  /* Use "fgets" as names could have spaces in it */
  if (fgets(name3, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return -1;
  }
  name3[strlen(name3) - 1] = '\0';

  num = 0;
  // get number of files to be composed
  fscanf(f1, "%d\n", &num);

  for (i = 0; i < num; i++) {
    /* Use "fgets" as names could have spaces in it */
    if (fgets(names1[i], MAX_FILENAME, f1) == NULL) {
      fclose(f1);
      return -1;
    }
    names1[i][strlen(names1[i]) - 1] = '\0';
    init = 0L;
    getdes(names1[i], &s1, &init, &t1);
    for (j = 0; j < s1; j++) {
      for (k = 0; k < t1[j].numelts; k++) {
        addordlist(t1[j].next[k].data1, &tranlist, s_tranlist, &ok);
        if (ok)
          s_tranlist++;
      }
    }
    freedes(s1, &t1);
    s1 = 0;
    t1 = NULL;
  }

  fclose(f1);

  init = 0L;
  getdes(names1[0], &s1, &init, &t1);

  for (i = 1; i < num; i++) {
    getdes(names1[i], &s2, &init, &t2);
    if (i == 1) {
      sync3(s1, t1, s2, t2, &s3, &t3, 0, s_tranlist, tranlist, &macro_ab,
            &macro_c);
    } else {
      sync3(s1, t1, s2, t2, &s3, &t3, 1, s_tranlist, tranlist, &macro_ab,
            &macro_c);
    }
    free(macro_ab);
    free(macro_c);
    macro_ab = macro_c = NULL;
    freedes(s1, &t1);
    freedes(s2, &t2);
    s1 = s2 = 0;
    t1 = t2 = NULL;
    export_copy_des(&s1, &t1, s3, t3);
    freedes(s3, &t3);
    s3 = 0;
    t3 = NULL;
  }

  if (mem_result != 1) {
    init = 0L;
    filedes(name3, s1, init, t1);
  } else {
    return -1;
  }

  return 0;
}

int meet_program(const char *filename) {
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return -1;
  }

  state_node *t1, *t2, *t3;
  INT_S s1, s2, s3, init;
  INT_S *macro_ab, *macro_c;
  INT_OS num, i;

  macro_ab = NULL;
  macro_c = NULL;
  t1 = t2 = t3 = NULL;
  s1 = s2 = s3 = 0;

  /* Use "fgets" as names could have spaces in it */
  if (fgets(name3, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return -1;
  }
  name3[strlen(name3) - 1] = '\0';

  num = 0;
  // get number of files to be composed
  fscanf(f1, "%d\n", &num);

  for (i = 0; i < num; i++) {
    /* Use "fgets" as names could have spaces in it */
    if (fgets(names1[i], MAX_FILENAME, f1) == NULL) {
      fclose(f1);
      return -1;
    }
    names1[i][strlen(names1[i]) - 1] = '\0';
  }

  fclose(f1);

  init = 0L;
  getdes(names1[0], &s1, &init, &t1);

  for (i = 1; i < num; i++) {
    getdes(names1[i], &s2, &init, &t2);

    meet2(s1, t1, s2, t2, &s3, &t3, &macro_ab, &macro_c);
    free(macro_ab);
    free(macro_c);
    macro_ab = macro_c = NULL;

    freedes(s1, &t1);
    freedes(s2, &t2);
    s1 = s2 = 0;
    t1 = t2 = NULL;
    export_copy_des(&s1, &t1, s3, t3);
    freedes(s3, &t3);
    s3 = 0;
    t3 = NULL;
  }

  free(macro_ab);

  if (mem_result != 1) {
    init = 0L;
    filedes(name3, s1, init, t1);
    free(macro_c);
  } else {
    free(macro_c);
    return -1;
  }

  return 0;
}

int supcon_program(const char *filename) {
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return -1;
  }

  state_node *t1, *t2, *t3;
  INT_S s1, s2, s3, init;
  INT_S *macro_ab, *macro_c;

  macro_ab = NULL;
  macro_c = NULL;
  t1 = t2 = t3 = NULL;
  s1 = s2 = s3 = 0;

  /* Use "fgets" as names could have spaces in it */
  if (fgets(name1, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return -1;
  }
  name1[strlen(name1) - 1] = '\0';

  if (fgets(name2, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return -1;
  }
  name2[strlen(name2) - 1] = '\0';

  if (fgets(name3, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return -1;
  }
  name3[strlen(name3) - 1] = '\0';

  fclose(f1);

  init = 0L;
  getdes(name1, &s1, &init, &t1);
  getdes(name2, &s2, &init, &t2);

  meet2(s1, t1, s2, t2, &s3, &t3, &macro_ab, &macro_c);
  freedes(s2, &t2);
  t2 = NULL;
  trim2(&s3, &t3, macro_c);
  shave1(s1, t1, &s3, &t3, macro_c);

  if (mem_result != 1) {
    filedes(name3, s3, init, t3);
  } else {
    return -1;
  }
  return 0;
}

int allevents_program(const char *filename) {
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return -1;
  }

  state_node *t1, *t2;
  INT_S s1, s2, init;
  INT_OS entry_type, ee;
  INT_T e, s_list, *list, i;
  INT_B ok;

  t1 = t2 = NULL;
  s1 = s2 = 0;
  s_list = 0;
  list = NULL;

  /* Use "fgets" as names could have spaces in it */
  if (fgets(name1, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return -1;
  }
  name1[strlen(name1) - 1] = '\0';

  if (fgets(name2, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return -1;
  }
  name2[strlen(name2) - 1] = '\0';

  fscanf(f1, "%d", &entry_type);

  if (entry_type == 3) {
    while (fscanf(f1, "%d", &ee) != EOF) {
      e = (INT_T)ee;
      addordlist(e, &list, s_list, &ok);
      if (ok)
        s_list++;
    }
  }

  fclose(f1);

  if (entry_type == 1) {
    init = 0L;
    getdes(name1, &s1, &init, &t1);
    allevent_des(&t1, &s1, &t2, &s2);
  } else if (entry_type == 2) {
    init = -1L;
    getdes(name1, &s1, &init, &t1);
    allevent_des(&t1, &s1, &t2, &s2);
  } else {
    s2 = 1;
    t2 = newdes(s2);
    (t2[0]).marked = true;
    for (i = 0; i < s_list; i++) {
      addordlist1(list[i], 0, &t2[0].next, t2[0].numelts, &ok);
      if (ok)
        t2[0].numelts++;
    }
  }

  free(list);
  if (mem_result != 1) {
    init = 0L;
    filedes(name2, s2, init, t2);
  } else {
    return -1;
  }

  return 0;
}

int mutex_program(const char *filename) {
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return -2;
  }

  state_node *t1, *t2, *t3;
  INT_S s1, s2, s3, init;
  INT_S *macro_ab, *macro_c;
  state_pair *sp;
  INT_S s_sp;
  INT_S i, j;
  INT_B ok;

  macro_ab = NULL;
  macro_c = NULL;
  t1 = t2 = t3 = NULL;
  s1 = s2 = s3 = 0;
  sp = NULL;
  s_sp = 0;

  /* Use "fgets" as names could have spaces in it */
  if (fgets(name1, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return -3;
  }
  name1[strlen(name1) - 1] = '\0';

  if (fgets(name2, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return -4;
  }
  name2[strlen(name2) - 1] = '\0';

  if (fgets(name3, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return -5;
  }
  name3[strlen(name3) - 1] = '\0';

  while (fscanf(f1, "%ld %ld", &i, &j) != EOF) {
    addstatepair(i, j, &sp, s_sp, &ok);
    if (ok)
      s_sp++;
  }

  fclose(f1);

  init = 0L;
  getdes(name1, &s1, &init, &t1);
  getdes(name2, &s2, &init, &t2);

  sync4(s1, t1, s2, t2, &s3, &t3, &macro_ab, &macro_c);
  free(macro_c);
  mutex1(&s3, &t3, s1, s2, macro_ab, sp, s_sp);
  reach(&s3, &t3);

  if (mem_result != 1) {
    filedes(name3, s3, init, t3);
  } else {
    return -6;
  }
}

int complement_program(const char *filename) {
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return -2;
  }

  state_node *t1;
  INT_S s1, init;
  INT_T *list, slist;
  INT_T e;
  INT_OS ee;
  INT_B ok;

  t1 = NULL;
  s1 = 0;
  list = NULL;
  slist = 0;

  /* Use "fgets" as names could have spaces in it */
  if (fgets(name1, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return -1;
  }
  name1[strlen(name1) - 1] = '\0';

  if (fgets(name2, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return -1;
  }
  name2[strlen(name2) - 1] = '\0';

  while (fscanf(f1, "%d", &ee) != EOF) {
    e = (INT_T)ee;
    addordlist(e, &list, slist, &ok);
    if (ok)
      slist++;
  }

  fclose(f1);
  // remove(prm_file);

  init = 0L;
  getdes(name1, &s1, &init, &t1);

  complement1(&s1, &t1, slist, list);
  reach(&s1, &t1);

  if (mem_result != 1) {
    filedes(name2, s1, init, t1);
  } else {
    // ctct_result(CR_OUT_OF_MEMORY);
    // exit(0);
    return -2;
  }
}

int nonconflict_program(const char *filename) {
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return -1;
  }
  state_node *t1, *t2, *t3;
  INT_S s1, s2, s3, init;
  INT_B is_nonconflict;
  INT_S *macro_ab, *macro_c;

  t1 = t2 = t3 = NULL;
  s1 = s2 = s3 = 0;
  is_nonconflict = false;
  macro_c = macro_ab = NULL;

  /* Use "fgets" as names could have spaces in it */
  if (fgets(name1, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    // remove(prm_file);
    // ctct_result(CR_PRM_ERR);
    // exit(0);
    return -1;
  }
  name1[strlen(name1) - 1] = '\0';

  if (fgets(name2, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    // remove(prm_file);
    // ctct_result(CR_PRM_ERR);
    // exit(0);
    return -1;
  }
  name2[strlen(name2) - 1] = '\0';

  fclose(f1);
  // remove(prm_file);

  init = 0L;
  getdes(name1, &s1, &init, &t1);

  init = 0L;
  getdes(name2, &s2, &init, &t2);

  is_nonconflict = false;

  nc_meet2(s1, t1, s2, t2, &s3, &t3, &macro_ab, &macro_c);
  is_nonconflict = nonconflict(s3, t3);

  if (mem_result == 1) {
    // ctct_result(CR_OUT_OF_MEMORY);
    return -2;
  } else {
    // ctct_result_flag(CR_OK, is_nonconflict);
    // return is_nonconflict;
  }

  freedes(s1, &t1);
  freedes(s2, &t2);
  freedes(s3, &t3);
  free(macro_ab);
  free(macro_c);
  // exit(0);
  return is_nonconflict;
}

int condat_program(const char *filename) {
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return -1;
  }
  state_node *t1, *t2, *t3, *t4;
  INT_S s1, s2, s3, s4, init;
  INT_S *macro_ab, *macro_c;

  macro_ab = NULL;
  macro_c = NULL;
  t1 = t2 = t3 = t4 = NULL;
  s1 = s2 = s3 = s4 = 0;

  /* Use "fgets" as names could have spaces in it */
  if (fgets(name1, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return -1;
    // remove(prm_file);
    // ctct_result(CR_PRM_ERR);
    // exit(0);
  }
  name1[strlen(name1) - 1] = '\0';

  if (fgets(name2, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return -1;
    // remove(prm_file);
    // ctct_result(CR_PRM_ERR);
    // exit(0);
  }
  name2[strlen(name2) - 1] = '\0';

  if (fgets(name3, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return -1;
    // remove(prm_file);
    // ctct_result(CR_PRM_ERR);
    // exit(0);
  }
  name3[strlen(name3) - 1] = '\0';

  fclose(f1);
  // remove(prm_file);

  init = 0L;
  getdes(name1, &s1, &init, &t1);
  getdes(name2, &s2, &init, &t2);

  meet2(s1, t1, s2, t2, &s3, &t3, &macro_ab, &macro_c);
  freedes(s2, &t2);
  t2 = NULL;
  free(macro_ab);

  condat1(t1, s1, s2, s3, t3, &s4, &t4, macro_c);

  if (mem_result != 1) {
    filedes(name3, s4, -1L, t4);
    return 0;
  } else {
    return -2;
    // ctct_result(CR_OUT_OF_MEMORY);
    // exit(0);
  }
}