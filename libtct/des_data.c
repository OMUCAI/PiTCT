/* DES data structure */

#include "des_data.h"
#include "mpack/mpack.h"
#include "mymalloc.h"
#include "setup.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

INT_OS mem_result = 0;

/* Func: newdes(size)
   Description:
      Allocate a new DES data structure with a given number of states.
      No transitions, silent states.

   Parameters:
      size    - number of states.  Must be greater than zero.
   Returns:
      Pointer to start of "state_node" array.
*/
state_node *newdes(INT_S size) {
  return (state_node *)CALLOC(size, sizeof(state_node));
}

state_node nil_state_node = {false, false, false, 0, 0, NULL};

void resize_des(state_node **t, INT_S oldsize, INT_S newsize) {
  INT_S i;

  *t = (state_node *)REALLOC(*t, sizeof(state_node) * (newsize));
  if (*t == NULL) {
    mem_result = 1;
    return;
  }

  if (oldsize < newsize) {
    for (i = oldsize; i < newsize; i++) {
      /*         (*t)[i].marked       = false;
               (*t)[i].reached      = false;
               (*t)[i].coreach      = false;
               (*t)[i].vocal        = 0;
               (*t)[i].numelts      = 0;
               (*t)[i].next         = NULL;
      */
      (*t)[i] = nil_state_node;
    }
  }
}

/* Func: freedes(size, data)
   Description:
      Deallocate all memory held by the DES data structure.

   Parameters:
      size   - number of states.  Must be a number zero or greater.
      data   - pointer to DES data structure.
   Returns:
      nothing.
      Sets the pointer of "data" to NIL.
*/
void freedes(INT_S size, state_node **data) {
  INT_S i;

  if (*data == NULL)
    return;
  if (size <= 0)
    return;

  for (i = 0L; i < size; i++)
    if ((*data)[i].next != NULL) {
      free((*data)[i].next);
      (*data)[i].next = NULL;
    }

  if (*data != NULL)
    free(*data);
  *data = NULL;
}

void free_map(INT_S size, state_map **data) {
  INT_S i;

  if (*data == NULL)
    return;

  for (i = 0L; i < size; i++)
    if ((*data)[i].next != NULL) {
      free((*data)[i].next);
      (*data)[i].next = NULL;
    }

  if (*data != NULL)
    free(*data);
  *data = NULL;
}

void free_part(INT_S s1, part_node **pn) {
  INT_S i;

  for (i = 0; i < s1; i++) {
    if ((*pn)[i].next != NULL)
      free((*pn)[i].next);
  }
  free(*pn);
}

void free_t_part(INT_S s1, t_part_node **pn) {
  INT_S i;

  for (i = 0; i < s1; i++) {
    if ((*pn)[i].next != NULL)
      free((*pn)[i].next);
  }
  free(*pn);
}
void free_bt_part(INT_S s1, bt_part_node **pn) {
  INT_S i;

  for (i = 0; i < s1; i++) {
    if ((*pn)[i].next != NULL)
      free((*pn)[i].next);
  }
  free(*pn);
}
void free_cc_check_table(INT_S size, cc_check_table **data) {
  INT_S i, j;

  if (*data == NULL)
    return;
  if (size <= 0)
    return;

  for (i = 0L; i < size; i++) {
    if ((*data)[i].next != NULL) {
      for (j = 0; j < (*data)[i].numelts; j++) {
        if ((*data)[i].next[j].dynindex != NULL) {
          free((*data)[i].next[j].dynindex);
          (*data)[i].next[j].dynindex = NULL;
        }
      }
      free((*data)[i].next);
      (*data)[i].next = NULL;
    }
  }

  if (*data != NULL)
    free(*data);
  *data = NULL;
}

// static char signature[8] = {"Z8^0L;1"};
// static char signature_x32[8] = {"Z8^0L;1"};
// static char signature_x64[8] = {"Z8^0L;2"};
// static INT_OS signature_length = 7;
// static INT_OS endian = 0xFF00AA55;

/* "filedes" function */
INT_OS filedes(char *name, INT_S size, INT_S init, state_node *data) {
  char fullname[_MAX_PATH];
  INT_S i;

  //   if (z) {}  /* Remove warning in some version of TCT */
  //   if (yy) {} /* Remove warning in some version of TCT */

  if (init == -1L) /* CONDAT file */
    sprintf(fullname, "%s%s.DAT", prefix, name);
  else
    sprintf(fullname, "%s%s.DES", prefix, name);

  /* Use MessagePack to store DES data */
  /* Setup a writer */
  mpack_writer_t writer;
  FILE *out = fopen(fullname, "wb");
  if (out == NULL) {
    printf("Could not create file: %s\n", fullname);
    return 1;
  }
  mpack_writer_init_stdfile(&writer, out, true);

  /* Start writing the schema */
  /*
    {
      "name": TCTNAME, // identity field
      "version": DES_FILE_VERSION, // identity field
      "size": num_of_states,
      "init": init_state,
      "states": {
        state: {
          "marked": true_or_false,
          "next": [[label, entrance]],
          "vocal": vocal_state
        }
      }
    }
  */
  mpack_start_map(&writer, 5);
  /* Write some information to indicate the file is a DES file */
  mpack_write_cstr(&writer, "name");
  mpack_write_cstr(&writer, TCTNAME);
  mpack_write_cstr(&writer, "version");
  mpack_write_cstr(&writer, DES_FILE_VERSION);

  /* Write the DES size */
  mpack_write_cstr(&writer, "size");
  mpack_write_int(&writer, size);

  /* Write the init state */
  mpack_write_cstr(&writer, "init");
  mpack_write_int(&writer, init);

  /* Write states, transitions, and vocals */
  mpack_write_cstr(&writer, "states");
  mpack_start_map(&writer, size);
  for (i = 0L; i < size; ++i) {
    mpack_write_int(&writer, i);
    mpack_start_map(&writer, 3);
    mpack_write_cstr(&writer, "marked");
    mpack_write_bool(&writer, data[i].marked == true);
    mpack_write_cstr(&writer, "next");
    if ((data[i].next != NULL) && (data[i].numelts > 0)) {
      mpack_start_array(&writer, data[i].numelts);
      for (size_t n = 0; n < data[i].numelts; ++n) {
        mpack_start_array(&writer, 2);
        mpack_write_int(&writer, data[i].next[n].data1);
        mpack_write_int(&writer, data[i].next[n].data2);
        mpack_finish_array(&writer);
      }
      mpack_finish_array(&writer);
    } else {
      mpack_write_nil(&writer);
    }
    mpack_write_cstr(&writer, "vocal");
    mpack_write_i16(&writer, data[i].vocal);
    mpack_finish_map(&writer);
  }
  mpack_finish_map(&writer);

  if (mpack_writer_destroy(&writer) != mpack_ok) {
    return 1;
  }

  return 0; /* Success in writing file to disk */
}

// export extend des file
INT_OS file_extdes(char *name, INT_S size, INT_S init, state_node *data) {
  char fullname[_MAX_PATH];
  INT_S i;

  //   if (z) {}  /* Remove warning in some version of TCT */
  //   if (yy) {} /* Remove warning in some version of TCT */

  if (init == -1L) /* CONDAT file */
    sprintf(fullname, "%s%s.EDAT", prefix, name);
  else
    sprintf(fullname, "%s%s.EDES", prefix, name);

  /* Use MessagePack to store DES data */
  /* Setup a writer */
  mpack_writer_t writer;
  FILE *out = fopen(fullname, "wb");
  if (out == NULL) {
    printf("Could not create file: %s\n", fullname);
    return 1;
  }
  mpack_writer_init_stdfile(&writer, out, true);

  /* Start writing the schema */
  /*
    {
      "name": TCTNAME, // identity field
      "version": DES_FILE_VERSION, // identity field
      "size": num_of_states,
      "init": init_state,
      "states": {
        state: {
          "reached": true_or_false
          "coreach": true_or_false
          "marked": true_or_false,
          "next": [[label, entrance]],
          "vocal": vocal_state
        }
      }
    }
  */
  mpack_start_map(&writer, 5);
  /* Write some information to indicate the file is a DES file */
  mpack_write_cstr(&writer, "name");
  mpack_write_cstr(&writer, TCTNAME);
  mpack_write_cstr(&writer, "version");
  mpack_write_cstr(&writer, EDES_FILE_VERSION);

  /* Write the DES size */
  mpack_write_cstr(&writer, "size");
  mpack_write_int(&writer, size);

  /* Write the init state */
  mpack_write_cstr(&writer, "init");
  mpack_write_int(&writer, init);

  /* Write states, transitions, and vocals */
  mpack_write_cstr(&writer, "states");
  mpack_start_map(&writer, size);
  for (i = 0L; i < size; ++i) {
    mpack_write_int(&writer, i);
    mpack_start_map(&writer, 5);
    mpack_write_cstr(&writer, "reached");
    mpack_write_bool(&writer, data[i].reached == true);
    mpack_write_cstr(&writer, "coreach");
    mpack_write_bool(&writer, data[i].coreach == true);
    mpack_write_cstr(&writer, "marked");
    mpack_write_bool(&writer, data[i].marked == true);
    mpack_write_cstr(&writer, "next");
    if ((data[i].next != NULL) && (data[i].numelts > 0)) {
      mpack_start_array(&writer, data[i].numelts);
      for (size_t n = 0; n < data[i].numelts; ++n) {
        mpack_start_array(&writer, 2);
        mpack_write_int(&writer, data[i].next[n].data1);
        mpack_write_int(&writer, data[i].next[n].data2);
        mpack_finish_array(&writer);
      }
      mpack_finish_array(&writer);
    } else {
      mpack_write_nil(&writer);
    }
    mpack_write_cstr(&writer, "vocal");
    mpack_write_i16(&writer, data[i].vocal);
    mpack_finish_map(&writer);
  }
  mpack_finish_map(&writer);

  if (mpack_writer_destroy(&writer) != mpack_ok) {
    return 1;
  }

  return 0; /* Success in writing file to disk */
}

/* GetDes function */
INT_B getdes(char *name, INT_S *size, INT_S *init, state_node **data) {
  FILE *in = NULL;
  char fullname[_MAX_PATH];

  //   if (z) {}  /* Remove warning in some versions of TCT */
  //   if (yy) {} /* Remove warning in some versions of TCT */
  strcpy(fullname, "");

  if (*init == -1L) {
    sprintf(fullname, "%s%s.DAT", prefix, name);
  } else {
    sprintf(fullname, "%s%s.DES", prefix, name);
  }

  in = fopen(fullname, "rb");
  if (in == NULL) {
    return false;
  }
  /* Start reading the schema */
  mpack_tree_t tree;
  mpack_tree_init_stdfile(&tree, in, 0, true);
  mpack_tree_parse(&tree);
  mpack_node_t root = mpack_tree_root(&tree);

  /* Check the identity fields */
  const char *name_field = mpack_node_str(mpack_node_map_cstr(root, "name"));
  const char *version = mpack_node_str(mpack_node_map_cstr(root, "version"));
  if (strncmp(name_field, TCTNAME, strlen(TCTNAME)) != 0 ||
      strncmp(version, DES_FILE_VERSION, strlen(DES_FILE_VERSION)) != 0) {
    mpack_tree_destroy(&tree);
    return false;
  }

  /* Read the size */
  *size = (INT_S)mpack_node_int(mpack_node_map_cstr(root, "size"));

  /* Create a new plant */
  *data = newdes(*size);
  if ((*size != 0) && (*data == NULL)) {
    mem_result = 1;
    mpack_tree_destroy(&tree);
    return false;
  }

  /* Assume that the initial state is always reachable */
  if (*size > 0L)
    (*data)[0L].reached = true;

  /* Read the initial state */
  *init = mpack_node_int(mpack_node_map_cstr(root, "init"));

  /* Read the states */
  mpack_node_t states_map = mpack_node_map_cstr(root, "states");
  size_t count = mpack_node_map_count(states_map);
  if (count != (size_t)*size) {
    mpack_tree_destroy(&tree);
    return false;
  }
  for (INT_S i = 0; i < *size; ++i) {
    mpack_node_t state = mpack_node_map_int(states_map, i);
    if (mpack_node_error(state) != mpack_ok) {
      mpack_tree_destroy(&tree);
      return false;
    }
    bool marked = mpack_node_bool(mpack_node_map_cstr(state, "marked"));
    if (marked) {
      (*data)[i].marked = true;
    }
    mpack_node_t next_states = mpack_node_map_cstr(state, "next");
    if (!mpack_node_is_nil(next_states)) {
      INT_T num_trans = (INT_T)mpack_node_array_length(next_states);
      (*data)[i].numelts = num_trans;
      (*data)[i].next = (tran_node *)MALLOC(sizeof(tran_node) * num_trans);
      if ((*data)[i].next == NULL) {
        mem_result = 1;
        mpack_tree_destroy(&tree);
        return false;
      }
      for (INT_T trans = 0; trans < num_trans; ++trans) {
        mpack_node_t transition = mpack_node_array_at(next_states, trans);
        INT_T label = (INT_T)mpack_node_int(mpack_node_array_at(transition, 0));
        INT_S entrance =
            (INT_S)mpack_node_int(mpack_node_array_at(transition, 1));
        (*data)[i].next[trans].data1 = label;
        (*data)[i].next[trans].data2 = entrance;
      }
    } else {
      (*data)[i].numelts = 0;
      (*data)[i].next = NULL;
    }
    (*data)[i].vocal =
        (INT_V)mpack_node_int(mpack_node_map_cstr(state, "vocal"));
  }

  if (mpack_tree_destroy(&tree) != mpack_ok) {
    return false;
  };

  return true;
}

void export_copy_des(INT_S *s_dest, state_node **t_dest, INT_S s_src,
                     state_node *t_src) {
  INT_S i, jj;
  INT_T j, ee;
  INT_B ok;

  *s_dest = s_src;
  *t_dest = newdes(s_src);

  if ((s_src != 0) && (*t_dest == NULL)) {
    mem_result = 1;
    return;
  }

  for (i = 0; i < s_src; i++) {
    (*t_dest)[i].marked = t_src[i].marked;
    (*t_dest)[i].reached = t_src[i].reached;
    (*t_dest)[i].coreach = t_src[i].coreach;
    (*t_dest)[i].vocal = t_src[i].vocal;
    for (j = 0; j < t_src[i].numelts; j++) {
      ee = t_src[i].next[j].data1;
      jj = t_src[i].next[j].data2;
      addordlist1(ee, jj, &(*t_dest)[i].next, (*t_dest)[i].numelts, &ok);
      if (ok)
        (*t_dest)[i].numelts++;
    }
  }
}
void reverse_des(INT_S *s_dest, state_node **t_dest, INT_S s_src,
                 state_node *t_src) {
  INT_S i, jj;
  INT_T j, ee;
  INT_B ok;

  *s_dest = s_src;
  *t_dest = newdes(s_src);

  if ((s_src != 0) && (*t_dest == NULL)) {
    mem_result = 1;
    return;
  }

  for (i = 0; i < s_src; i++) {
    for (j = 0; j < t_src[i].numelts; j++) {
      ee = t_src[i].next[j].data1;
      jj = t_src[i].next[j].data2;
      addordlist1(ee, i, &(*t_dest)[jj].next, (*t_dest)[jj].numelts, &ok);
      if (ok)
        (*t_dest)[jj].numelts++;
    }
  }
}

/* Add transition,extrance state pair */
void addordlist1(INT_T e, INT_S j, tran_node **L, INT_T size, INT_B *ok) {
  INT_T pos;
  INT_T lower, upper;
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
      if (e == (*L)[pos - 1].data1 && j == (*L)[pos - 1].data2) {
        found = true;
      } else if (e > (*L)[pos - 1].data1 ||
                (e == (*L)[pos - 1].data1 && j > (*L)[pos - 1].data2)) {
        lower = pos + 1;
      } else {
        upper = pos - 1;
      }
    }

    if (found == false) {
      if ((e < (*L)[pos - 1].data1) ||
          ((e == (*L)[pos - 1].data1) && j < (*L)[pos - 1].data2))
        pos--;
    }
  } else if (size == 1) {
    if (e == (*L)[0].data1 && j == (*L)[0].data2) {
      found = true;
    } else if ((e > (*L)[0].data1) ||
               ((e == (*L)[0].data1) && j > (*L)[0].data2)) {
      pos = 1;
    }
  }

  if (found == true) {
    return;
  }

  /* Make space for new element */
  *L = (tran_node *)REALLOC(*L, sizeof(tran_node) * (size + 1));
  if (*L == NULL) {
    mem_result = 1;
    return;
  }

  /* Move over any elements down the list */
  if ((size - pos) > 0)
    memmove(&(*L)[pos + 1], &(*L)[pos], sizeof(tran_node) * (size - pos));

  /* Insert the element into the list */
  (*L)[pos].data1 = e;
  (*L)[pos].data2 = j;

  *ok = true;
}

/* event in the transition, extrance state pair */
INT_B inordlist1(INT_T e,      /* event to find */
                 tran_node *L, /* list to search */
                 INT_T size) {
  INT_T pos;
  INT_T lower, upper;
  INT_B found;

  /* Do a binary search. */
  found = false;
  pos = 0;
  if (size > 1) {
    lower = 1;
    upper = size;
    while ((found == false) && (lower <= upper)) {
      pos = (lower + upper) / 2;
      if (e == L[pos - 1].data1) {
        found = true;
      } else if (e > L[pos - 1].data1) {
        lower = pos + 1;
      } else {
        upper = pos - 1;
      }
    }
  } else if (size == 1) {
    if (e == L[0].data1) {
      found = true;
    }
  }

  return found;
}

/* Add transition,extrance state pair */
INT_B inordlist2(INT_T e, INT_S j, tran_node *L, INT_T size) {
  INT_T pos;
  INT_T lower, upper;
  INT_B found;

  /* Do a binary search. */
  found = false;
  pos = 0;
  if (size > 1) {
    lower = 1;
    upper = size;
    while ((found == false) && (lower <= upper)) {
      pos = (lower + upper) / 2;
      if (e == L[pos - 1].data1 && j == L[pos - 1].data2) {
        found = true;
      } else if (e > L[pos - 1].data1 ||
                (e == L[pos - 1].data1 && j > L[pos - 1].data2)) {
        lower = pos + 1;
      } else {
        upper = pos - 1;
      }
    }

    if (found == false) {
      if ((e < L[pos - 1].data1) ||
          ((e == L[pos - 1].data1) && j < L[pos - 1].data2))
        pos--;
    }
  } else if (size == 1) {
    if (e == L[0].data1 && j == L[0].data2) {
      found = true;
    } else if ((e > L[0].data1) || ((e == L[0].data1) && j > L[0].data2)) {
      pos = 1;
    }
  }

  return found;
}
/* Add transition */
void addordlist(INT_T e, INT_T **L, INT_T size, INT_B *ok) {
  INT_T pos;
  INT_T lower, upper;
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
  *L = (INT_T *)REALLOC(*L, sizeof(INT_T) * (size + 1));
  if (*L == NULL) {
    mem_result = 1;
    return;
  }

  /* Move over any elements down the list */
  if ((size - pos) > 0)
    memmove(&(*L)[pos + 1], &(*L)[pos], sizeof(INT_T) * (size - pos));

  /* Insert the element into the list */
  (*L)[pos] = e;

  *ok = true;
}

/* Add state */
void addstatelist(INT_S e, INT_S **L, INT_S size, INT_B *ok) {
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
    return;
  }

  /* Move over any elements down the list */
  if ((size - pos) > 0)
    memmove(&(*L)[pos + 1], &(*L)[pos], sizeof(INT_S) * (size - pos));

  /* Insert the element into the list */
  (*L)[pos] = e;

  *ok = true;
}

/* Find a state in a list */
INT_B instatelist(INT_S e,  /* element to find */
                  INT_S *L, /* list to search */
                  INT_S size) {
  INT_S pos;
  INT_S lower, upper;
  INT_B found;

  /* Do a binary search. */
  found = false;
  pos = 0;
  if (size > 1) {
    lower = 1;
    upper = size;
    while ((found == false) && (lower <= upper)) {
      pos = (lower + upper) / 2;
      if (e == L[pos - 1]) {
        found = true;
      } else if (e > L[pos - 1]) {
        lower = pos + 1;
      } else {
        upper = pos - 1;
      }
    }

  } else if (size == 1) {
    if (e == L[0]) {
      found = true;
    }
  }

  return found;
}

/* Add state map */
void addstatemap(INT_S e1, INT_S e2, state_map **L, INT_S size, INT_B *ok) {
  INT_S pos;
  INT_S lower, upper;
  INT_B found, ok2;

  *ok = false;

  /* Do a binary search. */
  found = false;
  pos = 0;
  if (size > 1) {
    lower = 1;
    upper = size;
    while ((found == false) && (lower <= upper)) {
      pos = (lower + upper) / 2;
      if (e1 == (*L)[pos - 1].state) {
        found = true;
      } else if (e1 > (*L)[pos - 1].state) {
        lower = pos + 1;
      } else {
        upper = pos - 1;
      }
    }

    if (found == false) {
      if (e1 < (*L)[pos - 1].state)
        pos--;
    }
  } else if (size == 1) {
    if (e1 == (*L)[0].state) {
      pos = 1;
      found = true;
    } else if (e1 > (*L)[0].state) {
      pos = 1;
    }
  }

  if (found == true) {
    addstatelist(e2, &(*L)[pos - 1].next, (*L)[pos - 1].numelts, &ok2);
    if (ok2)
      (*L)[pos - 1].numelts++;
    return;
  }

  /* Make space for new element */
  *L = (state_map *)REALLOC(*L, sizeof(state_map) * (size + 1));
  if (*L == NULL) {
    mem_result = 1;
    return;
  }

  /* Move over any elements down the list */
  if ((size - pos) > 0)
    memmove(&(*L)[pos + 1], &(*L)[pos], sizeof(state_map) * (size - pos));

  /* Insert the element into the list */
  (*L)[pos].state = e1;
  (*L)[pos].numelts = 0;
  (*L)[pos].next = NULL;

  addstatelist(e2, &(*L)[pos].next, (*L)[pos].numelts, &ok2);
  if (ok2)
    (*L)[pos].numelts++;

  *ok = true;
}

/* Add State Pair */
void addstatepair(INT_S e, INT_S j, state_pair **L, INT_S size, INT_B *ok) {
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
      if (e == (*L)[pos - 1].data1 && j == (*L)[pos - 1].data2) {
        found = true;
      } else if (e > (*L)[pos - 1].data1 ||
                (e == (*L)[pos - 1].data1 && j > (*L)[pos - 1].data2)) {
        lower = pos + 1;
      } else {
        upper = pos - 1;
      }
    }

    if (found == false) {
      if ((e < (*L)[pos - 1].data1) ||
          ((e == (*L)[pos - 1].data1) && j < (*L)[pos - 1].data2))
        pos--;
    }
  } else if (size == 1) {
    if (e == (*L)[0].data1 && j == (*L)[0].data2) {
      found = true;
    } else if ((e > (*L)[0].data1) ||
               ((e == (*L)[0].data1) && j > (*L)[0].data2)) {
      pos = 1;
    }
  }

  if (found == true) {
    return;
  }

  /* Make space for new element */
  *L = (state_pair *)REALLOC(*L, sizeof(state_pair) * (size + 1));
  if (*L == NULL) {
    mem_result = 1;
    return;
  }

  /* Move over any elements down the list */
  if ((size - pos) > 0)
    memmove(&(*L)[pos + 1], &(*L)[pos], sizeof(state_pair) * (size - pos));

  /* Insert the element into the list */
  (*L)[pos].data1 = e;
  (*L)[pos].data2 = j;

  *ok = true;
}

/* Find state pair */
INT_B instatepair(INT_S e, INT_S j, state_pair **L, INT_S size) {
  INT_S pos;
  INT_S lower, upper;
  INT_B found;

  /* Do a binary search. */
  found = false;
  pos = 0;
  if (size > 1) {
    lower = 1;
    upper = size;
    while ((found == false) && (lower <= upper)) {
      pos = (lower + upper) / 2;
      if (e == (*L)[pos - 1].data1 && j == (*L)[pos - 1].data2) {
        found = true;
      } else if (e > (*L)[pos - 1].data1 ||
                (e == (*L)[pos - 1].data1 && j > (*L)[pos - 1].data2)) {
        lower = pos + 1;
      } else {
        upper = pos - 1;
      }
    }

  } else if (size == 1) {
    if (e == (*L)[0].data1 && j == (*L)[0].data2) {
      found = true;
    }
  }

  return found;
}

void insertlist4(INT_S src, INT_T tran, INT_S dest, INT_S *s, state_node **t) {
  INT_B ok;
  INT_S i;
  state_node *temp;

  if (src < *s) {
    addordlist1(tran, dest, &(*t)[src].next, (*t)[src].numelts, &ok);
    if (ok)
      (*t)[src].numelts++;
  } else {
    /* Need to increase the size of the array */
    temp = (state_node *)REALLOC(*t, sizeof(state_node) * (src + 1));

    if (temp == NULL) {
      mem_result = 1;
      return;
    }
    *t = temp;

    for (i = *s; i <= src; i++) {
      (*t)[i].marked = false;
      (*t)[i].reached = false;
      (*t)[i].coreach = false;
      (*t)[i].vocal = 0;
      (*t)[i].numelts = 0;
      (*t)[i].next = NULL;
    }

    *s = src + 1;
    addordlist1(tran, dest, &(*t)[src].next, (*t)[src].numelts, &ok);
    if (ok)
      (*t)[src].numelts++;
  }
}

/* For triple, list of [i,e,j] */
void addtriple(INT_S i, INT_T e, INT_S j, triple **L, INT_S size, INT_B *ok) {
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
      if (i == (*L)[pos - 1].i && e == (*L)[pos - 1].e &&
          j == (*L)[pos - 1].j) {
        found = true;
      } else if (i > (*L)[pos - 1].i ||
                 (i == (*L)[pos - 1].i && e > (*L)[pos - 1].e) ||
                 (i == (*L)[pos - 1].i && e == (*L)[pos - 1].e &&
                  j > (*L)[pos - 1].j)) {
        lower = pos + 1;
      } else {
        upper = pos - 1;
      }
    }

    if (found == false) {
      if ((i < (*L)[pos - 1].i) ||
          ((i == (*L)[pos - 1].i) && (e < (*L)[pos - 1].e)) ||
          ((i == (*L)[pos - 1].i) && (e == (*L)[pos - 1].e) &&
           (j < (*L)[pos - 1].j)))
        pos--;
    }
  } else if (size == 1) {
    if (i == (*L)[0].i && e == (*L)[0].e && j == (*L)[0].j) {
      found = true;
    } else if ((i > (*L)[0].i) || ((i == (*L)[0].i) && (e > (*L)[0].e)) ||
               ((i == (*L)[0].i) && (e == (*L)[0].e) && (j > (*L)[0].j))) {
      pos = 1;
    }
  }

  if (found == true) {
    return;
  }

  /* Make space for new element */
  *L = (triple *)REALLOC(*L, sizeof(triple) * (size + 1));
  if (*L == NULL) {
    mem_result = 1;
    return;
  }

  /* Move over any elements down the list */
  if ((size - pos) > 0)
    memmove(&(*L)[pos + 1], &(*L)[pos], sizeof(triple) * (size - pos));

  /* Insert the element into the list */
  (*L)[pos].i = i;
  (*L)[pos].e = e;
  (*L)[pos].j = j;

  *ok = true;
}

/* Delete transition,extrance state pair */
void delete_ordlist1(INT_T e, INT_S j, tran_node **L, INT_T size, INT_B *ok) {
  INT_T pos;
  INT_T lower, upper;
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
      if (e == (*L)[pos - 1].data1 && j == (*L)[pos - 1].data2) {
        found = true;
      } else if (e > (*L)[pos - 1].data1 ||
                (e == (*L)[pos - 1].data1 && j > (*L)[pos - 1].data2)) {
        lower = pos + 1;
      } else {
        upper = pos - 1;
      }
    }

    if (found == false) {
      if ((e < (*L)[pos - 1].data1) ||
          ((e == (*L)[pos - 1].data1) && j < (*L)[pos - 1].data2))
        pos--;
    }
  } else if (size == 1) {
    if (e == (*L)[0].data1 && j == (*L)[0].data2) {
      pos = 1;
      found = true;
    } else if ((e > (*L)[0].data1) ||
               ((e == (*L)[0].data1) && j > (*L)[0].data2)) {
      pos = 1;
    }
  }

  if (found == false) {
    return;
  }

  /* Move over any elements up the list */
  if ((size - pos) > 0 && pos > 0)
    memmove(&(*L)[pos - 1], &(*L)[pos], sizeof(tran_node) * (size - pos));

  /* Remove space for element */
  *L = (tran_node *)REALLOC(*L, sizeof(tran_node) * (size - 1));
  if (size > 1) {
    if (*L == NULL) {
      mem_result = 1;
      return;
    }
  } else {
    *L = NULL;
  }

  *ok = true;
}

/* Find an event in a list */
INT_B inlist(INT_T e,  /* element to find */
             INT_T *L, /* list to search */
             INT_T size) {
  INT_T pos;
  INT_T lower, upper;
  INT_B found;

  /* Do a binary search. */
  found = false;
  pos = 0;
  if (size > 1) {
    lower = 1;
    upper = size;
    while ((found == false) && (lower <= upper)) {
      pos = (lower + upper) / 2;
      if (e == L[pos - 1]) {
        found = true;
      } else if (e > L[pos - 1]) {
        lower = pos + 1;
      } else {
        upper = pos - 1;
      }
    }
  } else if (size == 1) {
    if (e == L[0]) {
      found = true;
    }
  }

  return found;
}

void add_quad(INT_S i, INT_T e, INT_S j, INT_V v, quad__t **L, INT_S slist,
              INT_B *ok) {
  INT_S pos;
  INT_S lower, upper;
  INT_B found;

  *ok = false;

  /* Do a binary search. */
  pos = 0;
  found = false;
  if (slist > 1) {
    lower = 1;
    upper = slist;
    while ((found == false) && (lower <= upper)) {
      pos = (lower + upper) / 2;
      if ((i == (*L)[pos - 1].a) && (e == (*L)[pos - 1].b) &&
          (j == (*L)[pos - 1].c) && (v == (*L)[pos - 1].d)) {
        found = true;
      } else if ((i > (*L)[pos - 1].a) ||
                 ((i == (*L)[pos - 1].a) && (e > (*L)[pos - 1].b)) ||
                 ((i == (*L)[pos - 1].a) && (e == (*L)[pos - 1].b) &&
                  (j > (*L)[pos - 1].c)) ||
                 ((i == (*L)[pos - 1].a) && (e == (*L)[pos - 1].b) &&
                  (j == (*L)[pos - 1].c) && (v > (*L)[pos - 1].d))) {
        lower = pos + 1;
      } else {
        upper = pos - 1;
      }
    }

    if (found == false) {
      if ((i < (*L)[pos - 1].a) ||
          ((i == (*L)[pos - 1].a) && (e < (*L)[pos - 1].b)) ||
          ((i == (*L)[pos - 1].a) && (e == (*L)[pos - 1].b) &&
           (j < (*L)[pos - 1].c)) ||
          ((i == (*L)[pos - 1].a) && (e == (*L)[pos - 1].b) &&
           (j == (*L)[pos - 1].c) && (v < (*L)[pos - 1].d)))
        pos--;
    }
  } else if (slist == 1) {
    if (i == (*L)[0].a && e == (*L)[0].b && j == (*L)[0].c && v == (*L)[0].d) {
      found = true;
    } else if ((i > (*L)[0].a) || ((i == (*L)[0].a) && (e > (*L)[0].b)) ||
               ((i == (*L)[0].a) && (e == (*L)[0].b) && (j > (*L)[0].c)) ||
               ((i == (*L)[0].a) && (e == (*L)[0].b) && (j == (*L)[0].c) &&
                (v > (*L)[0].d))) {
      pos = 1;
    }
  }

  if (found == true) {
    return;
  }

  /* Make space for new element */
  *L = (quad__t *)REALLOC(*L, sizeof(quad__t) * (slist + 1));
  if (*L == NULL) {
    mem_result = 1;
    return;
  }

  /* Move over any elements down the list */
  if ((slist - pos) > 0)
    memmove(&(*L)[pos + 1], &(*L)[pos], sizeof(quad__t) * (slist - pos));

  /* Insert the element into the list */
  (*L)[pos].a = i;
  (*L)[pos].b = e;
  (*L)[pos].c = j;
  (*L)[pos].d = v;

  *ok = true;
}

/* Determine if the DES files contains correct values */
INT_B check_des_sanity(state_node *t, INT_S s) {
  INT_S i, jj;
  INT_T j, ee;

  if ((t == NULL) || (s <= 0))
    return true;

  for (i = 0; i < s; i++) {
    for (j = 0; j < t[i].numelts; j++) {
      ee = t[i].next[j].data1;
      jj = t[i].next[j].data2;
      if (ee > 999)
        return false;
      if (!((0 <= jj) && (jj < s)))
        return false;
    }
  }
  return true;
}

/* Return the number of mark states */
INT_S num_mark_states(state_node *t1, INT_S s1) {
  INT_S i;
  INT_S num_mark;

  num_mark = 0L;

  for (i = 0; i < s1; i++) {
    if (t1[i].marked)
      num_mark++;
  }
  return num_mark;
}

/* get the marked states list(array) */
void mark_state_list(state_node *t1, INT_S size, INT_S *result) {
  // You must allocate memory for the number of marker states..
  // you can get from num_mark_states function.
  // Not Add last -1.
  INT_S i;
  INT_S result_idx = 0;
  for(i = 0; i < size; i++) {
    if(t1[i].marked) {
      result[result_idx++] = i;
    }
  }
}

/* Return the number of vocal states */
INT_S num_vocal_output(state_node *t1, INT_S s1) {
  INT_S i;
  INT_S num_vocal;

  num_vocal = 0;
  for (i = 0; i < s1; i++) {
    if (t1[i].vocal > 0)
      num_vocal++;
  }
  return num_vocal;
}

void recode_min(INT_S s1, state_node *t1, INT_S s2, state_node *t2,
                INT_S *mapState) {
  INT_S i, jj;
  INT_T ee, j;
  INT_S classlist;
  INT_B ok;

  for (i = 0; i < s1; i++) {
    classlist = mapState[i];
    t2[classlist].marked = t1[i].marked;
    t2[classlist].vocal = t1[i].vocal;
    for (j = 0; j < t1[i].numelts; j++) {
      ee = t1[i].next[j].data1;
      jj = mapState[t1[i].next[j].data2];
      addordlist1(ee, jj, &t2[classlist].next, t2[classlist].numelts, &ok);
      if (ok) {
        t2[classlist].numelts++;
      }
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////
// Used to print the contents of the strings, tables ...
static char tmp_result[_MAX_PATH];

void zprints(char *s) {
  FILE *out;
  //     INT_S i;

  strcpy(tmp_result, "");
  strcat(tmp_result, prefix);
  strcat(tmp_result, "Ouput.txt");
  out = fopen(tmp_result, "a");

  fprintf(out, "%s", s);

  fclose(out);
}
void zprintn(INT_S s) {
  FILE *out;

  strcpy(tmp_result, "");
  strcat(tmp_result, prefix);
  strcat(tmp_result, "Ouput.txt");

  out = fopen(tmp_result, "a");
  fprintf(out, "%ld", s);
  fclose(out);
}
void zprintsn(char *str, INT_S s) {
  FILE *out;

  strcpy(tmp_result, "");
  strcat(tmp_result, prefix);
  strcat(tmp_result, "Ouput.txt");
  out = fopen(tmp_result, "a");

  fprintf(out, "%s%ld\n", str, s);
  fclose(out);
}
void zprint_list(INT_S s, INT_S *list) {
  FILE *out;
  INT_S i;

  strcpy(tmp_result, "");
  strcat(tmp_result, prefix);
  strcat(tmp_result, "Ouput.txt");
  out = fopen(tmp_result, "a");

  for (i = 0; i < s; i++) {
    fprintf(out, "%ld ", list[i]);
  }
  fprintf(out, "\n");
  fclose(out);
}
void zprint_list1(INT_T s, INT_T *list) {
  INT_T i;
  FILE *out;

  strcpy(tmp_result, "");
  strcat(tmp_result, prefix);
  strcat(tmp_result, "Ouput.txt");
  out = fopen(tmp_result, "a");
  for (i = 0; i < s; i++) {
    fprintf(out, "%d ", list[i]);
  }
  fprintf(out, "\n");
  fclose(out);
}
void zprint_par(INT_S s, part_node *par) {
  FILE *out;
  INT_S i;
  INT_S j;

  strcpy(tmp_result, "");
  strcat(tmp_result, prefix);
  strcat(tmp_result, "Ouput.txt");
  out = fopen(tmp_result, "a");

  for (i = 0; i < s; i++) {
    for (j = 0; j < par[i].numelts; j++) {
      fprintf(out, "%ld ", par[i].next[j]);
    }
    fprintf(out, "\n");
  }
  fclose(out);
}
void zprint_tpar(INT_S s, t_part_node *tpar) {
  FILE *out;
  INT_S i;
  INT_T j;

  strcpy(tmp_result, "");
  strcat(tmp_result, prefix);
  strcat(tmp_result, "Ouput.txt");
  out = fopen(tmp_result, "a");

  for (i = 0; i < s; i++) {
    for (j = 0; j < tpar[i].numelts; j++) {
      fprintf(out, "%d ", tpar[i].next[j]);
    }
    fprintf(out, "\n");
  }
  fclose(out);
}
void zprint_btpar(INT_S s, bt_part_node *btpar) {
  FILE *out;
  INT_S i;
  INT_T j;

  strcpy(tmp_result, "");
  strcat(tmp_result, prefix);
  strcat(tmp_result, "Ouput.txt");
  out = fopen(tmp_result, "a");

  for (i = 0; i < s; i++) {
    for (j = 0; j < btpar[i].numelts; j++) {
      fprintf(out, "%d ", btpar[i].next[j]);
    }
    fprintf(out, "\n");
  }
  fclose(out);
}
void zprint_map(INT_S s, state_map *map) {
  FILE *out;
  INT_S i, j;

  strcpy(tmp_result, "");
  strcat(tmp_result, prefix);
  strcat(tmp_result, "Ouput.txt");
  out = fopen(tmp_result, "a");

  for (i = 0; i < s; i++) {
    for (j = 0; j < map[i].numelts; j++)
      fprintf(out, "%ld ", map[i].next[j]);
    fprintf(out, "\n");
  }
  fclose(out);
}
void zprint_pair(INT_S s, state_pair *pair) {
  INT_S i;
  FILE *out;

  strcpy(tmp_result, "");
  strcat(tmp_result, prefix);
  strcat(tmp_result, "Ouput.txt");
  out = fopen(tmp_result, "a");

  for (i = 0; i < s; i++) {
    fprintf(out, "%ld  %ld \n", pair[i].data1, pair[i].data2);
  }
  fprintf(out, "\n");
  fclose(out);
}
void zprint_triple(INT_S s, triple *trip) {
  INT_S i;
  FILE *out;

  strcpy(tmp_result, "");
  strcat(tmp_result, prefix);
  strcat(tmp_result, "Ouput.txt");
  out = fopen(tmp_result, "a");

  for (i = 0; i < s; i++)
    fprintf(out, "%ld  %d   %ld\n", trip[i].i, trip[i].e, trip[i].j);

  fprintf(out, "\n");
  fclose(out);
}
void zprint_check_table(INT_S size, cc_check_table *data) {
  FILE *out;
  INT_S i;
  INT_T j;
  char tmp_result[_MAX_PATH];

  strcpy(tmp_result, "");
  strcat(tmp_result, prefix);
  strcat(tmp_result, "Ouput.txt");
  out = fopen(tmp_result, "a");

  for (i = 0; i < size; i++) {
    for (j = (INT_T)data[i].numelts - 1; j >= 0 && j < data[i].numelts; j--) {
      fprintf(out, "%10d ", data[i].next[j].flag);
    }
    fprintf(out, "\n");
  }
  fclose(out);
}

#ifdef __cplusplus
}
#endif
