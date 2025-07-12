#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "program.h"
#include "des_data.h"
#include "des_proc.h"
#include "supred.h"
#include "tct_io.h"
#include "tct_proc.h"
#include "mymalloc.h"
#include "obs_check.h"
#include "localize.h"
#include "higen.h"
#include "cnorm.h"
#include "canqc.h"
#include "ext_proc.h"

static filename1 name1, name2, name3, name4, names1[MAX_DESS];
static char long_name1[MAX_FILENAME];
static char long_name2[MAX_FILENAME];
static char long_name3[MAX_FILENAME];
static char long_name4[MAX_FILENAME];

int create_program(const char *filename) {
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return ERR_FILE_OPEN;
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
    return ERR_PRM_FILE;
  }
  name1[strlen(name1) - 1] = '\0';

  fscanf(f1, "%ld\n", &s);

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
    return ERR_MEM;
  }
  freedes(s, &t);

  return RESULT_OK;
}

int selfloop_program(const char *filename) {
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return ERR_FILE_OPEN;
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
    return ERR_PRM_FILE;
  }
  name1[strlen(name1) - 1] = '\0';

  if (fgets(name2, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return ERR_PRM_FILE;
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
    return ERR_MEM;
  }
  freedes(s1, &t1);
  free(list);
  return RESULT_OK;
}

int trim_program(const char *filename) {
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return ERR_FILE_OPEN;
  }

  state_node *t1;
  INT_S s1, init; //,i;

  t1 = NULL;
  s1 = 0;
  /* Use "fgets" as names could have spaces in it */
  if (fgets(name1, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return ERR_PRM_FILE;
  }
  name1[strlen(name1) - 1] = '\0';

  if (fgets(name2, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return ERR_PRM_FILE;
  }
  name2[strlen(name2) - 1] = '\0';

  fclose(f1);

  init = 0L;
  getdes(name1, &s1, &init, &t1);

  trim1(&s1, &t1);

  if (mem_result != 1) {
    filedes(name2, s1, init, t1);
  } else {
    return ERR_MEM;
  }
  freedes(s1, &t1);

  return RESULT_OK;
}

int printdes_program(const char *filename) {
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return ERR_FILE_OPEN;
  }

  INT_S init;
  FILE *out;
  INT_S s1;
  state_node *t1;

  s1 = 0;
  t1 = NULL;

  /* Use "fgets" as names could have spaces in it */
  if (fgets(name1, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return ERR_PRM_FILE;
  }
  name1[strlen(name1) - 1] = '\0';

  if (fgets(name2, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return ERR_PRM_FILE;
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

  fclose(out);
  freedes(s1, &t1);

  return RESULT_OK;
}

int sync_program(const char *filename) {
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return ERR_FILE_OPEN;
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
    return ERR_PRM_FILE;
  }
  name3[strlen(name3) - 1] = '\0';

  num = 0;
  // get number of files to be composed
  fscanf(f1, "%d\n", &num);

  for (i = 0; i < num; i++) {
    /* Use "fgets" as names could have spaces in it */
    if (fgets(names1[i], MAX_FILENAME, f1) == NULL) {
      fclose(f1);
      return ERR_PRM_FILE;
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
    return ERR_MEM;
  }

  return RESULT_OK;
}

int meet_program(const char *filename) {
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return ERR_FILE_OPEN;
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
    return ERR_PRM_FILE;
  }
  name3[strlen(name3) - 1] = '\0';

  num = 0;
  // get number of files to be composed
  fscanf(f1, "%d\n", &num);

  for (i = 0; i < num; i++) {
    /* Use "fgets" as names could have spaces in it */
    if (fgets(names1[i], MAX_FILENAME, f1) == NULL) {
      fclose(f1);
      return ERR_PRM_FILE;
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
    freedes(s1, &t1);
    free(macro_c);
  } else {
    free(macro_c);
    return ERR_MEM;
  }

  return RESULT_OK;
}

int supcon_program(const char *filename) {
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return ERR_FILE_OPEN;
  }

  state_node *t1, *t2, *t3;
  INT_S s1, s2, s3, init;
  INT_S *macro_ab, *macro_c;
  filename1 local_name1, local_name2, local_name3;

  macro_ab = NULL;
  macro_c = NULL;
  t1 = t2 = t3 = NULL;
  s1 = s2 = s3 = 0;

  /* Use "fgets" as names could have spaces in it */
  if (fgets(local_name1, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return ERR_PRM_FILE;
  }
  local_name1[strlen(local_name1) - 1] = '\0';

  if (fgets(local_name2, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return ERR_PRM_FILE;
  }
  local_name2[strlen(local_name2) - 1] = '\0';

  if (fgets(local_name3, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return ERR_PRM_FILE;
  }
  local_name3[strlen(local_name3) - 1] = '\0';

  fclose(f1);

  init = 0L;
  getdes(local_name1, &s1, &init, &t1);
  getdes(local_name2, &s2, &init, &t2);

  meet2(s1, t1, s2, t2, &s3, &t3, &macro_ab, &macro_c);
  freedes(s2, &t2);
  t2 = NULL;
  trim2(&s3, &t3, macro_c);
  shave1(s1, t1, &s3, &t3, macro_c);

  if (mem_result != 1) {
    filedes(local_name3, s3, init, t3);
  } else {
    return ERR_MEM;
  }
  freedes(s1, &t1);
  freedes(s3, &t3);
  free(macro_ab);
  free(macro_c);

  return RESULT_OK;
}

int allevents_program(const char *filename) {
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return ERR_FILE_OPEN;
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
    return ERR_PRM_FILE;
  }
  name1[strlen(name1) - 1] = '\0';

  if (fgets(name2, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return ERR_PRM_FILE;
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
    return ERR_MEM;
  }
  freedes(s1, &t1);
  freedes(s2, &t2);

  return RESULT_OK;
}

int mutex_program(const char *filename) {
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return ERR_FILE_OPEN;
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
    return ERR_PRM_FILE;
  }
  name1[strlen(name1) - 1] = '\0';

  if (fgets(name2, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return ERR_PRM_FILE;
  }
  name2[strlen(name2) - 1] = '\0';

  if (fgets(name3, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return ERR_PRM_FILE;
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
    return ERR_MEM;
  }
  freedes(s1, &t1);
  freedes(s2, &t2);
  freedes(s3, &t3);
  free(macro_ab);
  free(sp);
  return RESULT_OK;
}

int complement_program(const char *filename) {
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return ERR_FILE_OPEN;
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
    return ERR_PRM_FILE;
  }
  name1[strlen(name1) - 1] = '\0';

  if (fgets(name2, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return ERR_PRM_FILE;
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
    return ERR_MEM;
  }
  freedes(s1, &t1);
  free(list);
  return RESULT_OK;
}

int nonconflict_program(const char *filename) {
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return ERR_FILE_OPEN;
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
    return ERR_PRM_FILE;
  }
  name1[strlen(name1) - 1] = '\0';

  if (fgets(name2, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    // remove(prm_file);
    // ctct_result(CR_PRM_ERR);
    // exit(0);
    return ERR_PRM_FILE;
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
    return ERR_MEM;
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
    return ERR_FILE_OPEN;
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
    return ERR_PRM_FILE;
    // remove(prm_file);
    // ctct_result(CR_PRM_ERR);
    // exit(0);
  }
  name1[strlen(name1) - 1] = '\0';

  if (fgets(name2, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return ERR_PRM_FILE;
    // remove(prm_file);
    // ctct_result(CR_PRM_ERR);
    // exit(0);
  }
  name2[strlen(name2) - 1] = '\0';

  if (fgets(name3, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return ERR_PRM_FILE;
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
  } else {
    return ERR_MEM;
    // ctct_result(CR_OUT_OF_MEMORY);
    // exit(0);
  }
  freedes(s1, &t1);
  freedes(s2, &t2);
  freedes(s3, &t3);
  freedes(s4, &t4);
  free(macro_c);
  return RESULT_OK;
}

int supreduce_program(const char *filename) {
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return ERR_FILE_OPEN;
  }
  // state_node *t1, *t2, *t3, *t4;
  // INT_S s1, s2, s3, s4, init;
  state_node *t4;
  INT_S s4, init;
  INT_S lb;
  float cr;
  INT_OS supreduce_flag;
  INT_OS mode;
  INT_OS slb_flag;

  t4 = NULL;
  s4 = 0;
  lb = 0;
  cr = 0.0;
  mode = 0;

  /* Use "fgets" as names could have spaces in it */
  if (fgets(name1, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return ERR_PRM_FILE;
    // remove(prm_file);
    // ctct_result(CR_PRM_ERR);
    // exit(0);
  }
  name1[strlen(name1) - 1] = '\0';

  if (fgets(name2, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    // remove(prm_file);
    // ctct_result(CR_PRM_ERR);
    // exit(0);
    return ERR_PRM_FILE;
  }
  name2[strlen(name2) - 1] = '\0';

  if (fgets(name3, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    // remove(prm_file);
    // ctct_result(CR_PRM_ERR);
    // exit(0);
    return ERR_PRM_FILE;
  }
  name3[strlen(name3) - 1] = '\0';

  if (fgets(name4, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    // remove(prm_file);
    // ctct_result(CR_PRM_ERR);
    // exit(0);
    return ERR_PRM_FILE;
  }
  name4[strlen(name4) - 1] = '\0';

  fscanf(f1, "%d\n", &mode);
  fscanf(f1, "%d\n", &slb_flag);

  fclose(f1);
  // remove(prm_file);

  strcpy(long_name1, "");
  strcpy(long_name2, "");
  strcpy(long_name3, "");
  strcpy(long_name4, "");
  make_filename_ext(long_name1, name1, EXT_DES);
  make_filename_ext(long_name2, name2, EXT_DES);
  make_filename_ext(long_name3, name3, EXT_DAT);
  make_filename_ext(long_name4, name4, EXT_DES);

  // TODO: Implement ex_supreduce
  // if (mode == 0)
  supreduce_flag = supreduce(long_name1, long_name2, long_name3, long_name4,
                             &lb, &cr, slb_flag);
  // else {
  //   supreduce_flag = ex_supreduce(long_name1, long_name2, long_name3,
  //                                 long_name4, &lb, &cr, slb_flag);
  // }

  if (supreduce_flag == 100) {
    return 100;
    // ctct_result(100);
    // exit(0);
  }

  if (mem_result != 1) {
    switch (supreduce_flag) {
    case 0:
      break;
    case -1:
      filedes(name4, s4, 0L, t4);
      break;
    case -2:
      init = 0;
      getdes(name2, &s4, &init, &t4);
      filedes(name4, s4, init, t4);
      freedes(s4, &t4);
      cr = 1;
      lb = s4;
      break;
    }

    if (supreduce_flag > 0) {
      return ERR_SUPREDUCE;
      // ctct_result(CR_SUPREDUCE_ERR);
      // exit(0);
    } else {
      /* On success, we need to pass the "lb" and "cr" */

      // ctct_result_supreduce(CR_OK, (INT_OS)lb, cr);
      // exit(0);
      
      return RESULT_OK;
    }
  } else {
    // ctct_result(CR_OUT_OF_MEMORY);
    return ERR_MEM;
  }
}

int isomorph_program(const char *filename) {
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return ERR_FILE_OPEN;
  }

  state_node *t1, *t2;
  INT_S s1, s2, init;
  INT_B  flag; //is_iso, identity
  INT_S *mapState;
  INT_S result;

  t1 = t2 = NULL;
  s1 = s2 = 0;
  // is_iso = false;
  mapState = NULL;
  // identity = false;
  result = 0;

  /* Use "fgets" as names could have spaces in it */
  if (fgets(name1, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return ERR_PRM_FILE;
  }
  name1[strlen(name1) - 1] = '\0';

  if (fgets(name2, MAX_FILENAME, f1) == NULL) {
    fclose(f1);
    return ERR_PRM_FILE;
  }
  name2[strlen(name2) - 1] = '\0';

  fclose(f1);
  // remove(prm_file);

  init = 0L;
  getdes(name1, &s1, &init, &t1);

  init = 0L;
  getdes(name2, &s2, &init, &t2);

  if ((strcmp(name1, name2) == 0) || ((s1 == 0) && (s2 == 0))) {
    // is_iso = true;
    // identity = true;
    result = true;
    // ctct_result_isomorph(CR_OK, is_iso, identity, 0, NULL);
    // goto FREE_MEM;
  } else if (s1 != s2) {
    result = false;
    // ctct_result_isomorph(CR_OK, is_iso, identity, 0, NULL);
    // goto FREE_MEM;
  } else {
    /* Need some memory here - Allocate map state */
    mapState = (INT_S *)CALLOC(s1, sizeof(INT_S));

    if ((s1 != 0) && (mapState == NULL)) {
      mem_result = 1;
      // ctct_result(CR_OUT_OF_MEMORY);
      // goto FREE_MEM;
    }
    memset(mapState, -1, sizeof(INT_S) * (s1));
    mapState[0] = 0;

    flag = true;
    iso1(s1, s2, t1, t2, &flag, mapState);
    if (flag) {
      // is_iso = true;
      result = true;
    }
  }

  if (mem_result == 1) {
    result = ERR_MEM;
    // ctct_result(CR_OUT_OF_MEMORY);
  } else {
    // ctct_result_isomorph(CR_OK, is_iso, identity, s1, mapState);
  }
  freedes(s1, &t1);
  freedes(s2, &t2);
  free(mapState);

  return result;
}

int printdat_program(const char *filename)
{
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return ERR_FILE_OPEN;
  }

	INT_S init;
	FILE *out;
	INT_S s1; state_node *t1;

	s1 = 0; t1 = NULL;

	/* Use "fgets" as names could have spaces in it */
	if (fgets(name1, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name1[strlen(name1)-1] = '\0';

	if (fgets(name2, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name2[strlen(name2)-1] = '\0';

	fclose(f1);

	init = -1L;
	getdes(name1, &s1, &init, &t1);

	make_filename_ext(long_name2, name2, EXT_TXT);
	out = fopen(long_name2, "w");

	print_dat_header_stat(out, name1, compute_controllable(t1,s1));

	if (count_tran(t1, s1) > 0) {
		print_dat(out, t1, s1);
		fprintf(out, "\n");
	} else {
		fprintf(out, "empty.\n");
	}

  fclose(out);
	freedes(s1, &t1);

  if(mem_result == 1)	{			
    return ERR_MEM;
  }
  return RESULT_OK;
}


int getdes_parameter_program(const char *filename)
{
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return ERR_FILE_OPEN;
  }
  FILE *out;
	state_node *t1;
	INT_S s1, init, tran_size;
	INT_B is_determin, is_controllable;
	int format;

	t1 = NULL; 
	s1 = 0;
	tran_size = -1;

	/* Use "fgets" as names could have spaces in it */
	if (fgets(name1, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name1[strlen(name1)-1] = '\0';

  if (fgets(name2, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name2[strlen(name2)-1] = '\0';
	make_filename_ext(long_name2, name2, EXT_RST);

	fscanf(f1, "%d\n", &format);

	fclose(f1);

	init = format;    
	getdes(name1, &s1, &init, &t1);
	if(mem_result == 1){
		return ERR_MEM;
	}

	tran_size = count_tran(t1,s1);

	is_determin = is_deterministic(t1,s1);

	if(format == -1)
		is_controllable = compute_controllable(t1, s1);
	else
		is_controllable = 2; // No check code

  out = fopen(long_name2, "w");
	if (out == NULL) 
		return ERR_FILE_OPEN;       /* Can do not much here so just return */
	fprintf(out, "%d\n", RESULT_OK);
	fprintf(out, "%d\n", s1);
	fprintf(out, "%d\n", tran_size);
	fprintf(out, "%d\n", is_determin);
	fprintf(out, "%d\n", is_controllable);
	fprintf(out, "\n");
	fclose(out);
  freedes(s1, &t1);
	// ctct_result_des_parameter(RESULT_OK, s1, tran_size, is_determin, is_controllable);
	return RESULT_OK;
}

int supconrobs_program(const char *filename)
{
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return ERR_FILE_OPEN;
  }
	INT_T slist, *list;
	INT_T s_imagelist, *imagelist;
	INT_B ok;
	INT_T e;
	INT_OS ee;
	INT_S init, s1;
	state_node *t1;

	INT_S result;

	slist = s_imagelist = 0;
	list = imagelist = NULL;
	t1 = NULL; s1 = 0;

	result = 0;
	/* Use "fgets" as names could have spaces in it */
	if (fgets(name1, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name1[strlen(name1)-1] = '\0';
	if (fgets(name2, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name2[strlen(name2)-1] = '\0';

	if (fgets(name3, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name3[strlen(name3)-1] = '\0';

	while( fscanf(f1, "%d" , &ee) != EOF)
	{
		e = (INT_T)ee;		
		addordlist(e, &list, slist,&ok);
		if(ok) slist ++;
	}
	fclose(f1);
	

	init = 0L;
	getdes(name1, &s1, &init, &t1);
	gen_complement_list(t1, s1,	list, slist, &imagelist, &s_imagelist);
	freedes(s1, &t1); s1 = 0; t1 = NULL;

	result = supconrobs_proc(name3, name1, name2, slist, list, s_imagelist, imagelist);

  free(list);
  free(imagelist);
	if(result == 0){
		return RESULT_OK;
	}else{
		if(mem_result == 1)	{			
			return ERR_MEM;
		}
    return RESULT_OK;
	}
}


int project_program(const char *filename)
{
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return ERR_FILE_OPEN;
  }
  state_node *t1;
  INT_S s1, init;
  INT_T *list, slist;
  INT_T e;
  INT_OS ee;
  INT_B  ok;

  t1 = NULL; s1 = 0;
  list = NULL; slist = 0;

    /* Use "fgets" as names could have spaces in it */
	if (fgets(name1, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name1[strlen(name1)-1] = '\0';

	if (fgets(name2, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
  name2[strlen(name2)-1] = '\0';
  
  while( fscanf(f1, "%d" , &ee) != EOF)
  {
    e = (INT_T) ee;
    addordlist(e, &list, slist, &ok);
    if (ok) slist++;
  }
	fclose(f1);
	
  init = 0L;   
  getdes(name1, &s1, &init, &t1);    

  project0(&s1,&t1,slist,list);

  if (mem_result != 1) {
    init = 0L;
    filedes(name2, s1, init, t1);
    freedes(s1, &t1);
    free(list);
    return RESULT_OK;
  } else {
    return ERR_MEM;
  }
}

int localize_program(const char *filename)
{
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return ERR_FILE_OPEN;
  }
	INT_OS result;
	INT_S sfile, sloc, i;
	char names1[MAX_DESS][MAX_FILENAME];
	char names2[MAX_DESS][MAX_FILENAME];  

	result = 0;
	/* Use "fgets" as names could have spaces in it */
	if (fgets(name1, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name1[strlen(name1)-1] = '\0';

	if (fgets(name2, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name2[strlen(name2)-1] = '\0';

	fscanf(f1,"%lld\n",&sfile);
	for(i = 0; i < sfile; i++) {
		if (fgets(names1[i], MAX_FILENAME, f1) == NULL)
		{
			fclose(f1);
			return ERR_PRM_FILE;
		}
		names1[i][strlen(names1[i])-1] = '\0';
	}
	fscanf(f1,"%lld\n",&sloc);
	for(i = 0; i < sloc; i++){
		if (fgets(names2[i], MAX_FILENAME, f1) == NULL)
		{
			return ERR_PRM_FILE;
		}
		names2[i][strlen(names2[i])-1] = '\0';
	}
	fclose(f1);

	result = localize_proc(sfile,sloc,name1,name2,names1,names2,0,0); 

	if (result != 0)
	{       
		if (mem_result == 1){
			return ERR_MEM;
		} 
		return ERR_UNKNOWN;
	}
  return RESULT_OK;
}

int minstate_program(const char *filename)
{
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return ERR_FILE_OPEN;
  }
  state_node *t1;
  INT_S s1, init;

  t1 = NULL; s1 = 0;
    
    /* Use "fgets" as names could have spaces in it */
	if (fgets(name1, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name1[strlen(name1)-1] = '\0';

	if (fgets(name2, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
  name2[strlen(name2)-1] = '\0';

	fclose(f1);
    
  init = 0L;
  getdes(name1, &s1, &init, &t1);
  
  reach(&s1, &t1);
  minimize(&s1, &t1);
    
  if (mem_result != 1) {
    filedes(name2, s1, init, t1);
    freedes(s1, &t1);
  } else {
    return ERR_MEM;
  }

  return RESULT_OK;
}

int force_program(const char *filename)
{
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return ERR_FILE_OPEN;
  }
	INT_T s_force_event_list, *force_event_list;
	INT_T s_preempt_event_list, *preempt_event_list;
	INT_T timeout_event;
	INT_B flag, ok;
	INT_T e;
	INT_OS ee;

	s_force_event_list = s_preempt_event_list = 0;
	force_event_list = preempt_event_list = NULL;

	/* Use "fgets" as names could have spaces in it */
	if (fgets(name1, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name1[strlen(name1)-1] = '\0';

	if (fgets(name2, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name2[strlen(name2)-1] = '\0';

	fscanf(f1, "%d\n", &ee);
	timeout_event = (INT_T)ee;
	flag = false;
	while( fscanf(f1, "%d" , &ee) != EOF)
	{
		if(ee == -1){
			flag = true;
			continue;
		}
		e = (INT_T)ee;
		if(!flag){			
			addordlist(e, &force_event_list, s_force_event_list,&ok);
			if(ok) s_force_event_list ++;
		}else{
			addordlist(e, &preempt_event_list, s_preempt_event_list, &ok);
			if(ok) s_preempt_event_list ++;
		}

	}

	fclose(f1);

	force_proc(name1, name2, s_force_event_list, force_event_list,
		s_preempt_event_list, preempt_event_list, timeout_event);

	if(mem_result == 1)
	{
		return ERR_MEM;
	}

  free(force_event_list);
  free(preempt_event_list);
  return RESULT_OK;
}

int convert_program(const char *filename)
{
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return ERR_FILE_OPEN;
  }
  state_node *t1, *t2;
  INT_S s1, s2, init;
  state_pair *sp;
  INT_S s_sp;
  INT_S i,j;
  INT_B  ok;
  INT_T *list;
  INT_T s_list;

  t1 = t2 = NULL;
  s1 = s2 = 0;
  sp = NULL;
  s_sp = 0;
  list = NULL;
  s_list = 0;
    
  /* Use "fgets" as names could have spaces in it */
	if (fgets(name1, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name1[strlen(name1)-1] = '\0';

	if (fgets(name2, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
  name2[strlen(name2)-1] = '\0';
  
  while( fscanf(f1, "%lld %lld" , &i, &j) != EOF)
  {
    addstatepair(i, j, &sp, s_sp, &ok);
    if (ok) s_sp++;
  }

	fclose(f1);
 
  init = 0L;   
  getdes(name1, &s1, &init, &t1);
  
  eventmap_des(t1,s1,&t2,&s2,sp,s_sp,&list,&s_list,&ok);

  if (s_list != 0) 
  project0(&s2,&t2,s_list,list);

  if (mem_result != 1) {
    filedes(name2, s2, init, t2);  
  } else {
    return ERR_MEM;
  }
  freedes(s1, &t1);
  freedes(s2, &t2);
  free(list);
  free(sp);
  return RESULT_OK;
}

int supnorm_program(const char *filename)
{
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return ERR_FILE_OPEN;
  }
  state_node *t1, *t2, *t3;
  INT_S s1, s2, s3, init;
  INT_T *list;
  INT_T slist;
  INT_T e;
  INT_OS ee;
  INT_B  ok;

  t1 = t2 = t3 = NULL;
  s1 = s2 = s3 = 0;
  list = NULL;  slist = 0;
    
	/* Use "fgets" as names could have spaces in it */
	if (fgets(name1, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name1[strlen(name1)-1] = '\0';

	if (fgets(name2, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name2[strlen(name2)-1] = '\0';

	if (fgets(name3, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
  name3[strlen(name3)-1] = '\0';
  
  while( fscanf(f1, "%d" , &ee) != EOF)
  {
    e = (INT_T) ee;
    addordlist(e, &list, slist, &ok);
    if (ok) slist++;
  }
	fclose(f1);
 
  init = 0L;   
  getdes(name1, &s1, &init, &t1);
  getdes(name2, &s2, &init, &t2);
  
  suprema_normal(t1, s1, t2, s2, &t3, &s3, list, slist);

  if (mem_result != 1) {
    filedes(name3, s3, init, t3);  
  } else {
    return ERR_MEM;
  }
  freedes(s1, &t1);
  freedes(s2, &t2);
  freedes(s3, &t3);
  free(list);
  return RESULT_OK;
}

int supscop_program(const char *filename)
{
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return ERR_FILE_OPEN;
  }
  state_node *t1, *t2, *t3;
  INT_S s1, s2, s3, init;
  INT_T *list;
  INT_T slist;
  INT_T e;
  INT_OS ee;
  INT_B  ok;

  t1 = t2 = t3 = NULL;
  s1 = s2 = s3 = 0;
  list = NULL;  slist = 0;
    
  /* Use "fgets" as names could have spaces in it */
	if (fgets(name1, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name1[strlen(name1)-1] = '\0';

	if (fgets(name2, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name2[strlen(name2)-1] = '\0';

	if (fgets(name3, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
  name3[strlen(name3)-1] = '\0';
    
  while( fscanf(f1, "%d" , &ee) != EOF)
  {
    e = (INT_T) ee;
    addordlist(e, &list, slist, &ok);
    if (ok) slist++;
  }  

	fclose(f1);
 
  init = 0L;   
  getdes(name1, &s1, &init, &t1);
  getdes(name2, &s2, &init, &t2);
  
  suprema_normal_scop(t1, s1, t2, s2, &t3, &s3, list, slist);

  if (mem_result != 1) {
    filedes(name3, s3, init, t3);  
  } else {
    return ERR_MEM;
  }
  freedes(s1, &t1);
  freedes(s2, &t2);
  freedes(s3, &t3);
  free(list);

  return RESULT_OK;
}

int canQC_program(const char *filename)
{
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return ERR_FILE_OPEN;
  }
	state_node *t1;
	INT_S s1, init;
	INT_T *list, slist;
	INT_T *imagelist, s_imagelist;
	INT_S *statemap, s_statemap;
	INT_T e;
	INT_OS ee;
	INT_B  ok;
	INT_OS result;
	INT_OS mode; 
  FILE *out;

	t1 = NULL; s1 = 0;
	list = NULL; slist = 0;
	imagelist = NULL; s_imagelist = 0;
	statemap = NULL; s_statemap = 0;

	fscanf(f1, "%d\n", &mode);

	/* Use "fgets" as names could have spaces in it */
	if (fgets(name1, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name1[strlen(name1)-1] = '\0';

	if (fgets(name2, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name2[strlen(name2)-1] = '\0';

  /* warn: (pitct) Change API */
  if (fgets(name3, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name3[strlen(name3)-1] = '\0';
	make_filename_ext(long_name3, name3, EXT_RST);

	while( fscanf(f1, "%d" , &ee) != EOF)
	{
		e = (INT_T) ee;
		addordlist(e, &list, slist, &ok);
		if (ok) slist++;
	} 
	fclose(f1);

	init = 0L;
	getdes(name1, &s1, &init, &t1);
	gen_complement_list(t1, s1,	list, slist, &imagelist, &s_imagelist);
	freedes(s1, &t1); s1 = 0; t1 = NULL;

	result = CanQC_proc1(name2, name1, slist, list, s_imagelist, imagelist,
		&s_statemap, &statemap, mode);

	if (result == 0)
	{
		/* On success, we need to pass the state partition */
    out = fopen(long_name3, "w");
    if (out == NULL) 
        return ERR_FILE_OPEN;       /* Can do not much here so just return */
    fprintf(out, "%d\n", result);
    for (int i=0; i < s_statemap; i++)
      fprintf(out, "%lld\n ", statemap[i]);
	  fclose(out);
    free(list);
    free(imagelist);
    free(statemap);
		return RESULT_OK;
	} else if (mem_result == 1) {
		return ERR_MEM;
	}
	return ERR_UNKNOWN;
}


int obs_program(const char *filename)
{
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return ERR_FILE_OPEN;
  }
	state_node *t1;
	INT_S s1, init;
	INT_T *list, slist;
	INT_T *imagelist, s_imagelist;
	INT_S *statemap, s_statemap;
	INT_T e;
	INT_OS ee;
	INT_B  ok;
	INT_B  flag;
	INT_OS result;
	INT_OS mode; 
	INT_B  is_observable;
  FILE *out;

	t1 = NULL; s1 = 0;
	list = NULL; slist = 0;
	imagelist = NULL; s_imagelist = 0;
	statemap = NULL; s_statemap = 0;
	is_observable = true;

	fscanf(f1, "%d\n", &mode);
	/* Use "fgets" as names could have spaces in it */
	if (fgets(name1, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name1[strlen(name1)-1] = '\0';

	if (fgets(name2, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name2[strlen(name2)-1] = '\0';

  /* warn: (pitct) Change API */
  if (fgets(name3, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name3[strlen(name3)-1] = '\0';
	make_filename_ext(long_name3, name3, EXT_RST);

	flag = false;
	while( fscanf(f1, "%d" , &ee) != EOF)
	{
		e = (INT_T) ee;
		addordlist(e, &list, slist, &ok);
		if (ok) slist++;
	} 
	fclose(f1);

	init = 0L;
	getdes(name1, &s1, &init, &t1);
	gen_complement_list(t1, s1,	list, slist, &imagelist, &s_imagelist);
	freedes(s1, &t1); s1 = 0; t1 = NULL;
	

	result = obs_proc(name1,name2,&s1,&t1,slist,list,s_imagelist,imagelist,mode,&is_observable);

	if (result == 0) {
    out = fopen(long_name3, "w");
    if (out == NULL) 
      return -1;       /* Can do not much here so just return */
    fprintf(out, "%d\n", CR_OK);
    fprintf(out, "%d\n", is_observable);
    // init = 0;
    // filedes(OBS_TEMP_NAME,s1,init,t1);
    // fprintf(out,OBS_TEMP_NAME); 
    // fprintf(out, "\n");
    fclose(out);
    free(list);
    free(imagelist);
    free(statemap);

    return RESULT_OK;
	} else if (mem_result == 1) {
		return ERR_MEM;
	}
  return ERR_UNKNOWN;
}

int natobs_program(const char *filename)
{
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return ERR_FILE_OPEN;
  }
	state_node *t1;
	INT_S s1, init;
	INT_T *list, slist;
	INT_T *imagelist, s_imagelist, *ext_imagelist, s_ext_imagelist;
	INT_S *statemap, s_statemap;
	INT_T e;
	INT_OS ee,i;
	INT_B ok;
	INT_OS result;

	t1 = NULL; s1 = 0;
	list = NULL; slist = 0;
	imagelist = ext_imagelist = NULL; s_imagelist = s_ext_imagelist = 0;
	statemap = NULL; s_statemap = 0;

	/* Use "fgets" as names could have spaces in it */
	if (fgets(name1, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name1[strlen(name1)-1] = '\0';

	if (fgets(name2, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name2[strlen(name2)-1] = '\0';

	if (fgets(name3, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name3[strlen(name3)-1] = '\0';

	while( fscanf(f1, "%d" , &ee) != EOF)
	{
		e = (INT_T) ee;
		addordlist(e, &imagelist, s_imagelist, &ok);
		if (ok) s_imagelist++;
	} 

	fclose(f1);

	init = 0L;
	getdes(name1, &s1, &init, &t1);

	gen_complement_list(t1, s1,	imagelist, s_imagelist, &list, &slist);

	freedes(s1, &t1); s1 = 0; t1 = NULL;

	result = ext_obs_proc(name2, name4, name1, name3, &slist, &list, &s_imagelist, &imagelist,
		&s_ext_imagelist, &ext_imagelist, &s_statemap, &statemap, 1, 0, 3);

	free(list); free(imagelist);  free(statemap);

	if(result == CR_OK){
		s1 = 1;
		t1 = newdes(s1);
		(t1[0]).marked = true;
		for(i = 0; i < s_ext_imagelist; i ++){
			addordlist1(ext_imagelist[i],0, &(t1)[0].next, (t1)[0].numelts, &ok);
			if(ok) (t1)[0].numelts ++;
		}
		if (mem_result != 1)
		{
			init = 0L;
			filedes(name3, s1, init, t1); 
		}
    freedes(s1, &t1);
		free(ext_imagelist);
		return RESULT_OK;
	} else {
		free(ext_imagelist);
		if(mem_result == 1)
			return ERR_MEM;
		return ERR_UNKNOWN;
	}
}

int supobs_program(const char *filename)
{
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return ERR_FILE_OPEN;
  }
	INT_T slist, *list;
	INT_T s_imagelist, *imagelist;
	INT_OS mode;
	INT_B ok;
	INT_T e;
	INT_OS ee;
	INT_S init, s1;
	state_node *t1;
	INT_S result;

	slist = s_imagelist = 0;
	list = imagelist = NULL;
	t1 = NULL; s1 = 0;

	result = 0;
	/* Use "fgets" as names could have spaces in it */
	if (fgets(name1, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name1[strlen(name1)-1] = '\0';
	if (fgets(name2, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name2[strlen(name2)-1] = '\0';

	if (fgets(name3, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name3[strlen(name3)-1] = '\0';

	fscanf(f1, "%d\n", &mode);

	while( fscanf(f1, "%d" , &ee) != EOF)
	{
		e = (INT_T)ee;		
		addordlist(e, &list, slist,&ok);
		if(ok) slist ++;
	}

	fclose(f1);

	init = 0L;
	getdes(name1, &s1, &init, &t1);
	gen_complement_list(t1, s1,	list, slist, &imagelist, &s_imagelist);
	freedes(s1, &t1); s1 = 0; t1 = NULL;

	if(mode == 1)
		result = supobs_proc1(name3, name1, name2, slist, list, s_imagelist, imagelist);
    free(list);
    free(imagelist);
	if(result != 0) {
		if(mem_result == 1)	{			
			return ERR_MEM;
		}
		return ERR_UNKNOWN;
	}
  return RESULT_OK;
}


int bfs_recode_program(const char *filename)
{
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return ERR_FILE_OPEN;
  }
	state_node *t1;
	INT_S s1, s2, init;
	INT_S *recode_array;

	recode_array = NULL;
	t1 = NULL; 
	s1 = s2 = 0;

	/* Use "fgets" as names could have spaces in it */
	if (fgets(name1, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name1[strlen(name1)-1] = '\0';

	if (fgets(name2, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name2[strlen(name2)-1] = '\0';

	fclose(f1);

	init = 0L;
	getdes(name1, &s1, &init, &t1);

	reach(&s1,&t1);
	b_recode(s1,&t1,&s2,&recode_array);

	if (mem_result != 1) {
		filedes(name2, s2, init, t1);
    freedes(s2, &t1);
    return RESULT_OK;
	} else {
		return ERR_MEM;
	}
}

int ext_suprobs_program(const char* filename) {
	// original function
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return ERR_FILE_OPEN;
  }

  INT_T  *list, slist, *imagelist, s_imagelist, *contr_list, s_contr_list;
	INT_S  init;
	INT_OS result; 
	INT_OS mark_flag, alg_flag;
  INT_T  e;
  INT_OS ee;
  INT_B  flag, ok;
	state_node *t1;
	INT_S  s1;

	list = imagelist = contr_list = NULL;
	slist = s_imagelist = s_contr_list = 0;
	s1 = 0;
	t1 = NULL;
	result = -1;

  if (fgets(name1, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name1[strlen(name1)-1] = '\0';

	if (fgets(name2, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name2[strlen(name2)-1] = '\0';
  
  if (fgets(name3, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name3[strlen(name3)-1] = '\0';

  if (fgets(name4, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name4[strlen(name4)-1] = '\0';

  // fscanf(f1, "%d\n", &mark_flag);
  mark_flag = 2;  // force not making information
  fscanf(f1, "%d\n", &alg_flag);

  flag = false;
  while( fscanf(f1, "%d" , &ee) != EOF)
	{
		if(ee == -1) {
			flag = true;
			continue;
		}
		e = (INT_T)ee;
		if(!flag) {
      // list of controllable event
			addordlist(e, &contr_list, s_contr_list, &ok);
			if(ok) s_contr_list++;
		} else {
      // get_imagelist(&s_imagelist, &imagelist, &slist, &list, &null_flag, 7);
      // support only null list
			addordlist(e, &list, slist, &ok);
			if(ok) slist++;
		}
	}

  init = 0L;
	if (getdes(name1, &s1, &init, &t1) == false) {
    return ERR_PRM_FILE;
	}

  gen_complement_list(t1, s1, list, slist, &imagelist, &s_imagelist);
	
	freedes(s1, &t1);
	s1 = 0; t1 = NULL;

	/*the main program of checking observability*/  
	if(alg_flag == 1) {
		result = transition_suprobs_proc(name4, name1, name2, name3, s_contr_list, contr_list, slist, list, s_imagelist, imagelist, mark_flag);
	} else if(alg_flag == 2) {
		result = language_suprobs_proc(name4, name1, name2, name3, s_contr_list, contr_list, slist, list, s_imagelist, imagelist, mark_flag);
	} else	{
		// do nothing
	}

  free(list);
	free(imagelist);
	free(contr_list);

  if(result == CR_OK) {
    return RESULT_OK;
	} else if(mem_result != 1) {
    return ERR_MEM;
  } else {
    return ERR_UNKNOWN;
  }
}

// .DES -> .EDES 
// it is used to check reach, coreach
int export_ext_des_program(const char *filename)
{
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return ERR_FILE_OPEN;
  }
	state_node *t1;
	INT_S s1, init, state;

	t1 = NULL; 
	s1 = state = 0;

	/* Use "fgets" as names could have spaces in it */
	if (fgets(name1, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name1[strlen(name1)-1] = '\0';

	fclose(f1);

	init = 0L;
	getdes(name1, &s1, &init, &t1);

  // Initialize calculate reach
  /* Zero all the reach variables */
  for (state = 0; state < s1; state++)
    (t1)[state].reached = false;

  (t1)[0].reached = true;
  b_reach((t1)[0].next, 0L, &t1, s1);

  // calculate coreach
  coreach2(s1, &t1);

	if (mem_result != 1) {
		file_extdes(name1, s1, init, t1);
    freedes(s1, &t1);
    return RESULT_OK;
	} else {
		return ERR_MEM;
	}
}

int eh_sync_program(const char *filename)
{
  FILE *f1 = fopen(filename, "r");
  if (f1 == NULL) {
    return ERR_FILE_OPEN;
  }

  INT_S num_of_sync;
  state_node *t3;
  INT_S s3;
  INT_T s_blockevents, *blockevents;
  INT_S s_pn;
  part_node *pn;
  FILE *out;

  num_of_sync = 0;
  t3 = NULL;
  s3 = 0;
  s_blockevents = 0;
  blockevents = NULL;
  s_pn = 0;
  pn = NULL;

	/* Use "fgets" as names could have spaces in it */
	if (fgets(name1, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name1[strlen(name1)-1] = '\0';

  if (fgets(name2, MAX_FILENAME, f1) == NULL)
	{
		fclose(f1);
		return ERR_PRM_FILE;
	}
	name2[strlen(name2)-1] = '\0';
  make_filename_ext(long_name2, name2, EXT_TXT);

  fscanf(f1, "%ld\n", &num_of_sync);

  for (INT_S i = 0; i < num_of_sync; i++) {
    if (fgets(names1[i], MAX_FILENAME, f1) == NULL)
    {
      fclose(f1);
      return ERR_PRM_FILE;
    }
    names1[i][strlen(names1[i])-1] = '\0';
  }
	fclose(f1);

	ehsync1(num_of_sync, name1, names1, &t3, &s3, &blockevents, &s_blockevents, &s_pn, &pn);
  freedes(s3, &t3); s3 = 0; t3 = NULL;

	if (mem_result != 1) {
    out = fopen(long_name2, "w");
    for(INT_S i = 0; i < s_pn; i++){
      fprintf(out, "%ld: ", i);
      for(INT_S j = 0; j < pn[i].numelts; j++){
        fprintf(out, "%ld", pn[i].next[j]);
        if (j < pn[i].numelts - 1) fprintf(out, ",");
      }
      fprintf(out, "\n");
    }
    fclose(out);
    free_part(s_pn, &pn);
    return RESULT_OK;
	} else {
		return ERR_MEM;
	}
}
