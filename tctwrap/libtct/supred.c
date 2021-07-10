#include "supred.h"
#include "des_data.h"
#include "des_supp.h"
#include "mymalloc.h"
#include "setup.h"
#include "supred2.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// #include "curses.h"

// static char signature[8] = {"Z8^0L;1"};
// static char signature_x32[8] = {"Z8^0L;1"};
// static char signature_x64[8] = {"Z8^0L;2"};
// static INT_OS  signature_length = 7;
// static INT_OS endian = 0xFF00AA55;

// INT_T *forcible_event;

static INT_S *controller, *controller_tree;
INT_S *simpler_controller; // Use it from supred2.cpp
static INT_S *plant, *c_marked_states, *p_marked_states;
static char **merging_table;

static INT_S *record;

// INT_OS data_length = 32;

INT_S *tmpu_stack, *cross_stack, index_cross_stack, index_tmpu_stack,
    *update_flag;
INT_S s_tmpu_stack;
INT_S stack_flag = 0;
static cc_check_table *track_table;
static INT_S s_track_table;
static INT_B iterate_flag, *mergelist;
static part_node *state_par;
static part_node *cross_state_par;

// static INT_S s_waitlist;
// static state_pair *waitlist;
// static INT_S num_drawn_cross, num_use_cross;

static INT_S tran_number, num_states;

struct node *root_node, *tmp_root_node;
struct track_node {
  struct equivalent_state_set *equal_set;
  struct equivalent_state_set **next_node;
} * track_root_node;
struct virtual_stack {
  INT_S state_1;
  INT_S state_2;
  INT_S node_1, node_2, c_flag, flag, r_flag_1, r_flag_2;
  INT_S tmpu_point_1, tmpu_point_2;
  INT_S record_point_1, record_point_2;
  struct equivalent_state_set *temp6, *temp7;
  struct transitions *temp3, *temp4;
  INT_S tmp_state_3, tmp_state_4;
  INT_S tmp_state_1, tmp_state_2;
  struct virtual_stack *last_state_pair;
  struct virtual_stack *next_state_pair;
} * stack;

// Corresponding relationship:
// Reduction, Selfloop_Node,Refinement are called in Supreduce and Supreduce2:
// Strong Control Consistency Reduction1, Selfloop_Node1,Refinement1 are called
// in Supreduce1: Weak Control Consistency Reduction2,
// Selfloop_Node2,Refinement2 are called in Supreduce3: New Control
// Consistency(Dx = Dx')

INT_OS Txt_DES(FILE *, INT_S);
void Controller_Tree();
INT_B Combined_Tree();
INT_OS Set_State_No_of_Plant(INT_S);
INT_B Forbidden_Event(char *);
void Tree_Structure_Conversion(char *);
void Reduction();
void Reduction1();
void Reduction2();
void Reduction3();
INT_OS Selfloop_Node(INT_S, INT_S, INT_S, INT_OS);
INT_OS Selfloop_Node1(INT_S, INT_S, INT_S, INT_OS);
INT_OS Selfloop_Node2(INT_S, INT_S, INT_S, INT_OS);
INT_OS Selfloop_Node3(INT_S, INT_S, INT_S, INT_OS);
INT_OS Selfloop_Node5(INT_S, INT_S, INT_S, INT_OS);
void Final_Result();
INT_S Refinement();
INT_S Refinement1();
INT_S Refinement2();
INT_S Refinement3();
INT_OS Get_DES(INT_S *, INT_S *, INT_OS, char *);

void father_node_in_controller_tree(INT_S);

/*I use this module to read the data directly from DES files.*/
INT_OS Get_DES(INT_S *tran_number, INT_S *num_states, INT_OS index,
               char *name) {
  // INT_S base_point;
  // INT_S mark, s, yy;

  // long address;
  // INT_S tmp;
  // INT_S counter, mark_states_number, index_1;
  // tran_node tran;
  // INT_B bFormat;

  // temporary variable
  INT_S i, j;

  INT_S base_point;
  INT_S mark_states_number;

  state_node *t1;
  INT_S size, init;
  getdes(name, &size, &init, &t1);

  // read x64 DES
  num_states = size;

  /* Read the marked states */
  mark_states_number = num_mark_states(t1, size);
  // mark = 0L;

  if (index == 0) {
    c_marked_states = (INT_S *)MALLOC((mark_states_number + 1) * sizeof(INT_S));
    if (c_marked_states == NULL) {
      mem_result = 1;
      return 5;
    }
    mark_state_list(t1, size, c_marked_states);
    c_marked_states[mark_states_number] = -1;
  }
  if (index == 1) {
    p_marked_states = (INT_S *)MALLOC((mark_states_number + 1) * sizeof(INT_S));
    if (p_marked_states == NULL) {
      free(c_marked_states);
      c_marked_states = NULL;
      mem_result = 1;
      return 6;
    }
    mark_state_list(t1, size, c_marked_states);
    c_marked_states[mark_states_number] = -1;
  }

  /* Read the transitions */
  // address = ftell(in);

  *tran_number = 0;

  for (i = 0; i < size; i++) {
    *tran_number += t1[i].numelts;
  }

  if (index == 0) {
    controller =
        (INT_S *)MALLOC((3 * (*tran_number) + 1) * sizeof(INT_S)); /*.....*/
    if (controller == NULL) {
      free(c_marked_states);
      c_marked_states = NULL;
      free(p_marked_states);
      p_marked_states = NULL;
      mem_result = 1;
      return 7;
    }
  }
  if (index == 1) {
    plant = (INT_S *)MALLOC((3 * (*tran_number) + 1) * sizeof(INT_S)); /*.....*/
    if (plant == NULL) {
      free(c_marked_states);
      c_marked_states = NULL;
      free(p_marked_states);
      p_marked_states = NULL;
      free(controller);
      controller = NULL;
      mem_result = 1;
      return 8;
    }
  }

  /* Modification start here ......................... */
  // fseek(in,address,SEEK_SET);
  base_point = 0;
  for (i = 0; i < size; i++) {
    if (index == 0) {
      if (t1[i].numelts == 0) {
        controller[3 * base_point] = i;
        controller[3 * base_point + 1] = 1000 + i;
        controller[3 * base_point + 2] = i;
        base_point += 1;
      }
      for (INT_S j = 0; j < t1[i].numelts; i++) {
        controller[3 * base_point] = i;
        controller[3 * base_point + 2] =
            t1[i].next[j].data2; //(int) (yy & 0x003FFFFF);
        controller[3 * base_point + 1] =
            (INT_S)t1[i].next[j].data1; //(int) ((unsigned long) tmp >> 22);
        base_point += 1;
      }
    } else if (index == 1) {
      if (t1[i].numelts == 0) {
        plant[3 * base_point] = i;
        plant[3 * base_point + 1] = 5000 + i;
        plant[3 * base_point + 2] = i;
        base_point += 1;
      }
      for (INT_S j = 0; j < t1[i].numelts; i++) {
        plant[3 * base_point] = i;
        plant[3 * base_point + 2] =
            t1[i].next[j].data2; //(int) (yy & 0x003FFFFF);
        plant[3 * base_point + 1] =
            (INT_S)t1[i].next[j].data1; //(int) ((unsigned long) tmp >> 22);
        base_point += 1;
      }
    }
  }

  if (index == 0) {
    if ((*num_states > 0) &&
        (*num_states - 1 > controller[3 * (base_point - 1)])) {
      for (i = controller[3 * (base_point - 1)] + 1; i < *num_states; i++) {
        controller[3 * base_point] = i;
        controller[3 * base_point + 1] = 1000 + i;
        controller[3 * base_point + 2] = i;
        base_point += 1;
      }
    }
    controller[3 * base_point] = -1;
  }
  if (index == 1) {
    if ((*num_states > 0) && (*num_states - 1 > plant[3 * (base_point - 1)])) {
      for (i = plant[3 * (base_point - 1)] + 1; i < *num_states; i++) {
        plant[3 * base_point] = i;
        plant[3 * base_point + 1] = 5000 + i;
        plant[3 * base_point + 2] = i;
        base_point += 1;
      }
    }
    plant[3 * base_point] = -1;
  }
  /* Read the vocal states */

  /* We don't read the last block type because we don't know how to
  process anything else after this point */
  return 0;
}

// Operations on equivalent classes
void memory_detect(struct node *loc_root_node) {
  INT_S i;
  struct equivalent_state_set *temp5;

  for (i = 0; i < num_states; i++) {
    temp5 = loc_root_node[i].equal_set;
    while (temp5 != NULL) {
      if (temp5->state_number >= num_states) {
        zprintn(temp5->state_number);
      }
      temp5 = temp5->next_node;
    }
  }
}

void recover_node(struct node *root_node_1, struct node *root_node_2,
                  INT_S state) {
  struct equivalent_state_set **temp1, *temp11, *temp12;
  INT_S equiv_state_1, equiv_state_2, i, tmp_state;
  INT_S s_list, *list;
  INT_B ok;

  s_list = 0;
  list = NULL;

  // obtain the states in equiv set of state in root_node_1
  equiv_state_2 = root_node_2[state].equal_set->state_number;
  temp12 = root_node_2[equiv_state_2].equal_set;
  while (temp12 != NULL) {
    tmp_state = temp12->state_number;
    temp12 = temp12->next_node;
    equiv_state_1 = root_node_1[tmp_state].equal_set->state_number;
    temp11 = root_node_1[equiv_state_1].equal_set;
    while (temp11 != NULL) {
      addstatelist(temp11->state_number, &list, s_list, &ok);
      if (ok)
        s_list++;
      temp11 = temp11->next_node;
    }
  }

  for (i = 0; i < s_list; i++) {
    temp1 = &root_node_1[list[i]].equal_set;
    temp12 = root_node_2[list[i]].equal_set;
    while (temp12 != NULL) {
      if ((*temp1) == NULL) {
        *temp1 = (struct equivalent_state_set *)CALLOC(
            1, sizeof(struct equivalent_state_set));
        if (*temp1 == NULL) {
          mem_result = 1;
          return;
        }
        (*temp1)->next_node = NULL;
      } // else{
      //	temp1 = ()
      //}
      (*temp1)->state_number = temp12->state_number;
      temp1 = &((*temp1)->next_node);
      temp12 = temp12->next_node;
    }
    if ((*temp1) != NULL) {
      temp11 = (*temp1)->next_node;
      free(*temp1);
      *temp1 = temp11;
    }

    /*while (temp11 != NULL){
            temp12 = temp11->next_node;
            free(temp11);
            temp11 = temp12;
    }
    root_node_1[list[i]].equal_set->next_node = NULL;

    memory_detect(tmp_root_node);

    //copy the equivalent set of each state from root_node_2 to root_node_1
    temp1 = &(root_node_1[list[i]].equal_set);
    equiv_state_2 = root_node_2[list[i]].equal_set->state_number;

    if(list[i] == equiv_state_2){
            (*temp1)->state_number = equiv_state_2;
            temp1 = &((*temp1)->next_node);
            temp12 = root_node_2[equiv_state_2].equal_set->next_node;
            while(temp12 != NULL)
            for(j = 1; j < state_par[equiv_state_2].numelts; j ++){
                    tmp_state = state_par[equiv_state_2].next[j];
                    if(tmp_state == equiv_state_2)
                            continue;
                    *temp1 = (struct equivalent_state_set *)
    CALLOC(1,sizeof(struct equivalent_state_set)); if(*temp1 == NULL){
                            mem_result = 1;
                            return;
                    }
                    (*temp1)->state_number = tmp_state;
                    root_node_1[tmp_state].equal_set->state_number =
    equiv_state_2; temp1 = &((*temp1)->next_node);
            }
            (*temp1) = NULL;
    }

    memory_detect(tmp_root_node); */

    /*while(temp11!=NULL){
            *temp1 = (struct equivalent_state_set *) CALLOC(1,sizeof(struct
    equivalent_state_set)); if(*temp1 == NULL){ mem_result = 1; return;
            }
            (*temp1)->state_number = temp11->state_number;
            temp11 = temp11->next_node;
            if(temp11 != NULL){
                    temp1 = &((*temp1)->next_node);
            }else{
                    (*temp1)->next_node = NULL;
            }
    }*/
  }
  free(list);
  memory_detect(tmp_root_node);
  memory_detect(root_node);
}
void divide_nodes(struct node *root_node_1, struct node *root_node_2,
                  INT_S state_1, INT_S state_2) {
  INT_S equiv_state_1, equiv_state_2, state; //, tmp_state;
  struct equivalent_state_set **temp1, *temp12, *temp11, *temp22;
  INT_S *list, s_list, i; //, j;
  INT_B ok;

  list = NULL;
  s_list = 0;

  /*	if(state_1 == 1 && state_2 == 23)
                  s_list = 0;

          temp22 =
     root_node_2[root_node_2[state_1].equal_set->state_number].equal_set;
          while(temp22 != NULL){
                  state = temp22->state_number;
                  temp22 = temp22->next_node;
          }
          temp22 =
     root_node_1[root_node_1[state_1].equal_set->state_number].equal_set;
          while(temp22 != NULL){
                  state = temp22->state_number;
                  temp22 = temp22->next_node;
          }
          temp22 =
     root_node_2[root_node_2[state_2].equal_set->state_number].equal_set;
          while(temp22 != NULL){
                  state = temp22->state_number;
                  temp22 = temp22->next_node;
          }
          temp22 =
     root_node_1[root_node_1[state_2].equal_set->state_number].equal_set;
          while(temp22 != NULL){
                  state = temp22->state_number;
                  temp22 = temp22->next_node;
          }*/

  equiv_state_1 = root_node_1[state_1].equal_set->state_number;

  equiv_state_2 = root_node_2[state_2].equal_set->state_number;

  // tmp_state = root_node_2[state_1].equal_set->state_number;

  if (equiv_state_2 == equiv_state_1) {
    equiv_state_2 = root_node_2[state_1].equal_set->state_number;
  }

  // divide the one with larger index from the smaller one
  /*if(state_1 > state_2){
          equiv_state_1 = root_node_1[state_1].equal_set->state_number;
          equiv_state_2 = root_node_2[state_2].equal_set->state_number;
  }else{
          equiv_state_1 = root_node_1[state_2].equal_set->state_number;
          equiv_state_2 = root_node_2[state_1].equal_set->state_number;
  }*/

  temp22 = root_node_2[equiv_state_2].equal_set;
  while (temp22 != NULL) {
    state = temp22->state_number;
    temp22 = temp22->next_node;
    temp11 = root_node_1[equiv_state_1].equal_set;
    if (temp11->state_number == state) {
      addstatelist(state, &list, s_list, &ok);
      if (ok)
        s_list++;
      root_node_1[equiv_state_1].equal_set = temp11->next_node;
      free(temp11);
      temp11 = NULL;
    } else {
      while (temp11 != NULL) {
        // remove the equiv set of equiv_state_2 in root_node_2 from equiv set
        // of equiv_state_1 in root_node_1
        temp12 = temp11->next_node;
        if (temp12 != NULL) {
          if (temp12->state_number == state) {
            addstatelist(state, &list, s_list, &ok);
            if (ok)
              s_list++;
            temp11->next_node = temp12->next_node;
            free(temp12);
            temp12 = NULL;
            // break;
          }
        }
        temp11 = temp11->next_node;
      }
    }
  }

  // Recover equiv_state_2 in root_node_1 from equiv_state_2 in root_node_2
  for (i = 0; i < s_list; i++) {
    temp1 = &root_node_1[list[i]].equal_set;
    temp12 = root_node_2[list[i]].equal_set;
    while (temp12 != NULL) {
      if ((*temp1) == NULL) {
        *temp1 = (struct equivalent_state_set *)CALLOC(
            1, sizeof(struct equivalent_state_set));
        if (*temp1 == NULL) {
          mem_result = 1;
          return;
        }
        (*temp1)->next_node = NULL;
      } // else{
      //	temp1 = ()
      //}
      (*temp1)->state_number = temp12->state_number;
      temp1 = &((*temp1)->next_node);
      temp12 = temp12->next_node;
    }
    while ((*temp1) != NULL) {
      temp11 = (*temp1)->next_node;
      free(*temp1);
      *temp1 = temp11;
    }
  }

  free(list);
  memory_detect(tmp_root_node);
  memory_detect(root_node);
}

void merge_state(struct node *loc_root_node, INT_S state_1, INT_S state_2) {
  INT_S temp3, temp4, temp_state, tmp_state_1, tmp_state_2;
  struct equivalent_state_set **temp5, *temp6, **temp7, *temp8; //,**temp9;
  INT_S k, tmp_state;

  temp7 = NULL;

  temp3 = loc_root_node[state_1].equal_set->state_number;
  temp4 = loc_root_node[state_2].equal_set->state_number;

  if (temp3 == temp4)
    return;

  temp6 = loc_root_node[temp3].equal_set;
  while (temp6 != NULL) {
    tmp_state_1 = temp6->state_number;
    temp6 = temp6->next_node;

    temp8 = loc_root_node[temp4].equal_set;
    while (temp8 != NULL) {
      tmp_state_2 = temp8->state_number;
      temp8 = temp8->next_node;
      if (tmp_state_1 < tmp_state_2) {
        track_table[tmp_state_2 - 1].next[tmp_state_1].flag = 4;
      } else if (tmp_state_1 > tmp_state_2)
        track_table[tmp_state_1 - 1].next[tmp_state_2].flag = 4;
    }
  }
  if (track_table[1626].next[225].flag != 0)
    temp3 = temp3;
  temp6 = loc_root_node[temp3].equal_set;
  while (temp6 != NULL) {
    tmp_state_1 = temp6->state_number;
    temp6 = temp6->next_node;
    for (k = 0; k < cross_state_par[tmp_state_1].numelts; k++) {
      tmp_state = cross_state_par[tmp_state_1].next[k];
      temp8 = loc_root_node[temp4].equal_set;
      while (temp8 != NULL) {
        tmp_state_2 = temp8->state_number;
        temp8 = temp8->next_node;
        if (tmp_state < tmp_state_2) {
          track_table[tmp_state_2 - 1].next[tmp_state].flag = 3;
        } else if (tmp_state > tmp_state_2)
          track_table[tmp_state - 1].next[tmp_state_2].flag = 3;
      }
    }
  }

  temp6 = loc_root_node[temp4].equal_set;
  while (temp6 != NULL) {
    tmp_state_1 = temp6->state_number;
    temp6 = temp6->next_node;
    for (k = 0; k < cross_state_par[tmp_state_1].numelts; k++) {
      tmp_state = cross_state_par[tmp_state_1].next[k];
      temp8 = loc_root_node[temp3].equal_set;
      while (temp8 != NULL) {
        tmp_state_2 = temp8->state_number;
        temp8 = temp8->next_node;
        if (tmp_state < tmp_state_2) {
          track_table[tmp_state_2 - 1].next[tmp_state].flag = 3;
        } else if (tmp_state > tmp_state_2)
          track_table[tmp_state - 1].next[tmp_state_2].flag = 3;
      }
    }
  }
  if (track_table[1626].next[225].flag != 0)
    temp3 = temp3;
  if (temp3 == temp4)
    return;
  else if (temp3 < temp4) {
    temp5 = &(loc_root_node[temp3].equal_set); /*acceptor*/
    temp6 = loc_root_node[temp4].equal_set;    /*merger*/

  } else if (temp3 > temp4) {
    temp5 = &(loc_root_node[temp4].equal_set); /*acceptor*/
    temp6 = loc_root_node[temp3].equal_set;    /*merger*/
  }

  temp_state = (*temp5)->state_number;

  while (*temp5 != NULL) {
    update_flag[(*temp5)->state_number] = 1;
    temp5 = &((*temp5)->next_node);
  }

  while (temp6 != NULL) {
    *temp5 = (struct equivalent_state_set *)CALLOC(
        1, sizeof(struct equivalent_state_set));
    if (*temp5 == NULL) {
      mem_result = 1;
      return;
    }
    (*temp5)->state_number = temp6->state_number;
    update_flag[(*temp5)->state_number] = 1;
    loc_root_node[temp6->state_number].equal_set->state_number = temp_state;
    temp6 = temp6->next_node;
    if (temp6 != NULL) {
      temp5 = &((*temp5)->next_node);
    } else {
      (*temp5)->next_node = NULL;
    }
  }
  if (temp3 < temp4) {
    temp6 = loc_root_node[temp4].equal_set->next_node; /*merger*/
    while (temp6 != NULL) {
      temp8 = temp6->next_node;
      free(temp6);
      temp6 = temp8;
    }
    loc_root_node[temp4].equal_set->next_node = NULL;

  } else if (temp3 > temp4) {
    temp6 = loc_root_node[temp3].equal_set->next_node; /*merger*/
    while (temp6 != NULL) {
      temp8 = temp6->next_node;
      free(temp6);
      temp6 = temp8;
    }
    loc_root_node[temp3].equal_set->next_node = NULL;
  }

  memory_detect(tmp_root_node);
  memory_detect(root_node);
}
void merge_tmp_state(struct node *root_node_1, INT_S state_1, INT_S state_2) {
  INT_S temp3, temp4, temp_state, tmp_state_1, tmp_state_2;
  struct equivalent_state_set **temp5, *temp6, **temp7, *temp8; //,**temp9;
  //		INT_S  tmp_state;
  //		INT_B merge_flag;

  // recover_node(root_node_1, root_node_2, state_2);

  if (state_1 == 416 && state_2 == 555)
    state_1 = state_1;

  temp7 = NULL;

  temp3 = root_node_1[state_1].equal_set->state_number;
  temp4 = root_node_1[state_2].equal_set->state_number;

  // merge_flag = true;

  temp6 = root_node_1[temp3].equal_set;
  while (temp6 != NULL) {
    tmp_state_1 = temp6->state_number;
    temp6 = temp6->next_node;

    temp8 = root_node_1[temp4].equal_set;
    while (temp8 != NULL) {
      tmp_state_2 = temp8->state_number;
      temp8 = temp8->next_node;
      if (tmp_state_1 < tmp_state_2) {
        if (track_table[tmp_state_2 - 1].next[tmp_state_1].flag == 3)
          return;
      } else if (tmp_state_1 > tmp_state_2) {
        if (track_table[tmp_state_1 - 1].next[tmp_state_2].flag == 3)
          return;
      }
    }
  }

  if (temp3 == temp4)
    return;
  else if (temp3 < temp4) {
    temp5 = &(root_node_1[temp3].equal_set); /*acceptor*/
    temp6 = root_node_1[temp4].equal_set;    /*merger*/

  } else if (temp3 > temp4) {
    temp5 = &(root_node_1[temp4].equal_set); /*acceptor*/
    temp6 = root_node_1[temp3].equal_set;    /*merger*/
  }

  temp_state = (*temp5)->state_number;

  while (*temp5 != NULL) {
    temp5 = &((*temp5)->next_node);
  }

  while (temp6 != NULL) {
    *temp5 = (struct equivalent_state_set *)CALLOC(
        1, sizeof(struct equivalent_state_set));
    if (*temp5 == NULL) {
      mem_result = 1;
      return;
    }
    (*temp5)->state_number = temp6->state_number;
    root_node_1[temp6->state_number].equal_set->state_number = temp_state;
    temp6 = temp6->next_node;
    if (temp6 != NULL) {
      temp5 = &((*temp5)->next_node);
    } else {
      (*temp5)->next_node = NULL;
    }
  }
  if (temp3 < temp4) {
    temp6 = root_node_1[temp4].equal_set->next_node; /*merger*/
    while (temp6 != NULL) {
      temp8 = temp6->next_node;
      free(temp6);
      temp6 = temp8;
    }
    root_node_1[temp4].equal_set->next_node = NULL;

  } else if (temp3 > temp4) {
    temp6 = root_node_1[temp3].equal_set->next_node; /*merger*/
    while (temp6 != NULL) {
      temp8 = temp6->next_node;
      free(temp6);
      temp6 = temp8;
    }
    root_node_1[temp3].equal_set->next_node = NULL;
  }
  if (tmp_root_node[1610].equal_set->state_number == 3)
    memory_detect(tmp_root_node);
  memory_detect(root_node);
}
void merge_state_trans(struct node *root_node_1, INT_S state_1, INT_S state_2) {
  INT_S temp3, temp4, temp_state, tmp_state_1, tmp_state_2;
  struct equivalent_state_set **temp5, *temp6, **temp7, *temp8; //,**temp9;
  struct transitions **temp1, *temp2;
  //		INT_S  tmp_state;
  //		INT_B merge_flag;

  temp7 = NULL;

  temp3 = root_node_1[state_1].equal_set->state_number;
  temp4 = root_node_1[state_2].equal_set->state_number;

  // merge_flag = true;

  temp6 = root_node_1[temp3].equal_set;
  while (temp6 != NULL) {
    tmp_state_1 = temp6->state_number;
    temp6 = temp6->next_node;

    temp8 = root_node_1[temp4].equal_set;
    while (temp8 != NULL) {
      tmp_state_2 = temp8->state_number;
      temp8 = temp8->next_node;
      if (tmp_state_1 < tmp_state_2) {
        if (track_table[tmp_state_2 - 1].next[tmp_state_1].flag == 3)
          return;
      } else if (tmp_state_1 > tmp_state_2) {
        if (track_table[tmp_state_1 - 1].next[tmp_state_2].flag == 3)
          return;
      }
    }
  }

  if (temp3 == temp4)
    return;
  else if (temp3 < temp4) {
    temp5 = &(root_node_1[temp3].equal_set); /*acceptor*/
    temp1 = &(root_node_1[temp3].tran_set);
    temp6 = root_node_1[temp4].equal_set; /*merger*/
    temp2 = root_node_1[temp4].tran_set;

  } else if (temp3 > temp4) {
    temp5 = &(root_node_1[temp4].equal_set); /*acceptor*/
    temp1 = &(root_node_1[temp4].tran_set);
    temp6 = root_node_1[temp3].equal_set; /*merger*/
    temp2 = root_node_1[temp3].tran_set;
  }

  // Step 1: merge states

  temp_state = (*temp5)->state_number;

  while (*temp5 != NULL) {
    temp5 = &((*temp5)->next_node);
  }

  while (temp6 != NULL) {
    *temp5 = (struct equivalent_state_set *)CALLOC(
        1, sizeof(struct equivalent_state_set));
    if (*temp5 == NULL) {
      mem_result = 1;
      return;
    }
    (*temp5)->state_number = temp6->state_number;
    root_node_1[temp6->state_number].equal_set->state_number = temp_state;
    temp6 = temp6->next_node;
    if (temp6 != NULL) {
      temp5 = &((*temp5)->next_node);
    } else {
      (*temp5)->next_node = NULL;
    }
  }

  // Step 2 merge transitions and forbidden events
  // if(root_node_1[temp3].marked_in_plant == true ||
  // root_node_1[temp4].marked_in_plant == true)
  //	root_node_1[temp_state].marked_in_plant = true;
  // if(root_node_1[temp3].marked_in_controller == true ||
  // root_node_1[temp4].marked_in_controller == true)
  //	root_node_1[temp_state].marked_in_plant = true;
  while (*temp1 != NULL) {
    temp1 = &((*temp1)->next_transition);
  }
  while (temp2 != NULL) {
    *temp1 = (struct transitions *)CALLOC(1, sizeof(struct transitions));
    if (*temp1 == NULL) {
      mem_result = 1;
      return;
    }
    (*temp1)->event = temp2->event;
    (*temp1)->target_state_number = temp2->target_state_number;
    //(*temp5)->state_number = temp6->state_number;
    // root_node_1[temp6->state_number].equal_set->state_number=temp_state;
    temp2 = temp2->next_transition;
    if (temp2 != NULL) {
      temp1 = &((*temp1)->next_transition);
    } else {
      (*temp1)->next_transition = NULL;
    }
  }

  memory_detect(tmp_root_node);
  memory_detect(root_node);
}
void merge_state_par(INT_S state_1, INT_S state_2) {
  INT_S i, j, k, numelts1, numelts2, tmp_state, state_eq, tmp_state_1,
      tmp_state_2;
  //		INT_B ok;

  if (root_node[state_1].equal_set->state_number ==
      root_node[state_2].equal_set->state_number)
    return;
  else if (root_node[state_1].equal_set->state_number <
           root_node[state_2].equal_set->state_number) {
    state_eq = root_node[state_1].equal_set->state_number;
  } else
    state_eq = root_node[state_2].equal_set->state_number;

  numelts1 = state_par[state_1].numelts;
  numelts2 = state_par[state_2].numelts;

  for (i = 0; i < numelts1; i++) {
    tmp_state_1 = state_par[state_1].next[i];
    for (k = 0; k < cross_state_par[tmp_state_1].numelts; k++) {
      tmp_state = cross_state_par[tmp_state_1].next[k];
      for (j = 0; j < numelts2; j++) {
        tmp_state_2 = state_par[state_2].next[i];
        if (tmp_state < tmp_state_2) {
          track_table[tmp_state_2 - 1].next[tmp_state].flag = 3;
        } else if (tmp_state > tmp_state_2)
          track_table[tmp_state - 1].next[tmp_state_2].flag = 3;
      }
    }
  }
  for (i = 0; i < numelts2; i++) {
    tmp_state_2 = state_par[state_2].next[i];
    for (k = 0; k < cross_state_par[tmp_state_2].numelts; k++) {
      tmp_state = cross_state_par[tmp_state_2].next[k];
      for (j = 0; j < numelts1; j++) {
        tmp_state_1 = state_par[state_1].next[i];
        if (tmp_state < tmp_state_1) {
          track_table[tmp_state_1 - 1].next[tmp_state].flag = 3;
        } else if (tmp_state > tmp_state_1)
          track_table[tmp_state - 1].next[tmp_state_1].flag = 3;
      }
    }
  }

  for (i = 0; i < numelts1; i++) {
    tmp_state = state_par[state_1].next[i];
    memcpy(state_par[tmp_state].next + numelts1, state_par[state_2].next,
           numelts2 * sizeof(INT_S));
    state_par[tmp_state].numelts += numelts2;
    root_node[tmp_state].equal_set->state_number = state_eq;
  }
  for (i = 0; i < numelts2; i++) {
    tmp_state = state_par[state_2].next[i];
    memcpy(state_par[tmp_state].next + numelts2, state_par[state_1].next,
           numelts1 * sizeof(INT_S));
    state_par[tmp_state].numelts += numelts1;
    root_node[tmp_state].equal_set->state_number = state_eq;
  }
  memory_detect(tmp_root_node);
  memory_detect(root_node);
}

INT_B add_equiv_state(INT_S state, struct equivalent_state_set *base_node,
                      INT_B *state_number,
                      struct equivalent_state_set **next_node) {
  if (state_number[state] == true)
    return false;
  *next_node = (struct equivalent_state_set *)MALLOC(
      sizeof(struct equivalent_state_set));
  if (*next_node == NULL) {
    mem_result = 1;
    return false;
  }
  (*next_node)->state_number = state;
  state_number[state] = true;
  (*next_node)->next_node = NULL;

  return true;
}

INT_B add_temp_state(INT_S state, struct equivalent_state_set *base_node,
                     struct equivalent_state_set **next_node, INT_S *s_tmp_list,
                     INT_S **tmp_list) {
  INT_B ok;
  addstatelist(state, tmp_list, *s_tmp_list, &ok);
  if (ok)
    (*s_tmp_list)++;

  if (!ok)
    return false;

  *next_node = (struct equivalent_state_set *)MALLOC(
      sizeof(struct equivalent_state_set));
  if (*next_node == NULL) {
    mem_result = 1;
    return false;
  }
  (*next_node)->state_number = state;
  (*next_node)->next_node = NULL;

  return true;
}

// Modefied by ZRY to implement Localization algorithm
/*This module is part of the main module Reduction.*/
INT_OS Selfloop_Node1(INT_S base_state, INT_S tmp_state, INT_S nodel,
                      INT_OS indexx) {
  INT_S node_1, node_2, c_flag, flag, r_flag_1, r_flag_2;
  INT_S tmpu_point, tmpu_point_1, tmpu_point_2, k;
  struct forbidden_event_set *temp1;
  struct transitions *temp2, *temp22;
  struct equivalent_state_set *temp6, *temp7;
  INT_S tmp_state_1, tmp_state_2;

  node_1 = base_state;
  node_2 = tmp_state;
  temp1 = 0;
  temp2 = 0;

  // zprintn(node_1); zprints("-"); zprintn(node_2); zprints("\n");

  temp6 = root_node[root_node[node_1].equal_set->state_number].equal_set;
  r_flag_1 = 0;
  tmpu_point_1 = 0;

  while (r_flag_1 == 0) {
    if (indexx == 0) {
      if (temp6 != NULL) {
        tmp_state_1 = temp6->state_number;
        temp6 = temp6->next_node;
      } else {
        tmp_state_1 = -1;
        while (*(tmpu_stack + 2 * tmpu_point_1) != -1) {
          if (*(tmpu_stack + 2 * tmpu_point_1) == node_1) {
            tmp_state_1 = *(tmpu_stack + 2 * tmpu_point_1 + 1);
            tmpu_point_1 += 1;
            break;
          }
          if (*(tmpu_stack + 2 * tmpu_point_1 + 1) == node_1) {
            tmp_state_1 = *(tmpu_stack + 2 * tmpu_point_1);
            tmpu_point_1 += 1;
            break;
          }
          tmpu_point_1 += 1;
        }
        if (tmp_state_1 == -1)
          r_flag_1 = 1;
        ;
      }
    } else {
      r_flag_1 = 1;
      tmp_state_1 = node_1;
    }

    if (tmp_state_1 != -1) {
      temp7 = root_node[root_node[node_2].equal_set->state_number].equal_set;
      r_flag_2 = 0;
      tmpu_point_2 = 0;
      while (r_flag_2 == 0) {
        if (indexx == 0) {
          if (temp7 != NULL) {
            tmp_state_2 = temp7->state_number;
            temp7 = temp7->next_node;
          } else {
            tmp_state_2 = -1;
            while (*(tmpu_stack + 2 * tmpu_point_2) != -1) {
              if (*(tmpu_stack + 2 * tmpu_point_2) == node_2) {
                tmp_state_2 = *(tmpu_stack + 2 * tmpu_point_2 + 1);
                tmpu_point_2 += 1;
                break;
              }
              if (*(tmpu_stack + 2 * tmpu_point_2 + 1) == node_2) {
                tmp_state_2 = *(tmpu_stack + 2 * tmpu_point_2);
                tmpu_point_2 += 1;
                break;
              }
              tmpu_point_2 += 1;
            }
            if (tmp_state_2 == -1)
              r_flag_2 = 1;
            ;
          }
        } else {
          r_flag_2 = 1;
          tmp_state_2 = node_2;
        }
        if (tmp_state_2 != -1) {
          c_flag = 0;
          if ((tmp_state_1 != node_1 || tmp_state_2 != node_2) &&
              (tmp_state_1 != node_2 || tmp_state_2 != node_1)) {
            if (indexx == 1) {
              if (merging_table[tmp_state_1][tmp_state_2] == 0)
                continue;
              if (merging_table[tmp_state_1][tmp_state_2] == 1 &&
                  tmp_state_1 != tmp_state_2)
                return 10;
            }
            if (root_node[tmp_state_1].equal_set->state_number ==
                root_node[tmp_state_2].equal_set->state_number)
              continue;

            tmpu_point = 0;
            while (*(tmpu_stack + 2 * tmpu_point) != -1) {
              if (*(tmpu_stack + 2 * tmpu_point) == tmp_state_1 &&
                  *(tmpu_stack + 2 * tmpu_point + 1) == tmp_state_2) {
                c_flag = 8;
                break;
              }
              if (*(tmpu_stack + 2 * tmpu_point + 1) == tmp_state_1 &&
                  *(tmpu_stack + 2 * tmpu_point) == tmp_state_2) {
                c_flag = 8;
                break;
              }
              tmpu_point += 1;
            }
            if (c_flag == 8)
              continue;

            // if(root_node[tmp_state_1].marked_in_plant==root_node[tmp_state_2].marked_in_plant
            // &&
            // root_node[tmp_state_1].marked_in_controller!=root_node[tmp_state_2].marked_in_controller)
            //        return 10;

            for (k = 1; k < 3; k++) {
              if (k == 1) {
                temp1 = root_node[tmp_state_1].forb_set;
                temp2 = root_node[tmp_state_2].tran_set;
              }
              if (k == 2) {
                temp1 = root_node[tmp_state_2].forb_set;
                temp2 = root_node[tmp_state_1].tran_set;
              }
              temp22 = temp2;
              while (temp1 != NULL) {
                temp2 = temp22;
                while (temp2 != NULL) {
                  if (temp1->event == temp2->event)
                    return 10;
                  temp2 = temp2->next_transition;
                }
                temp1 = temp1->next_event;
              }
            }
            if (c_flag != 8) {
              *(tmpu_stack + 2 * tmpu_point) = tmp_state_1;
              *(tmpu_stack + 2 * tmpu_point + 1) = tmp_state_2;
              *(tmpu_stack + 2 * tmpu_point + 2) = -1;
              // zprints("Add to wait list:  "); zprintn(tmp_state_1);
              // zprints("-"); zprintn(tmp_state_2); zprints("\n");
            }
          }
          if (c_flag == 0) {
            struct transitions *temp3, *temp4;
            INT_S tmp_state_3, tmp_state_4;
            temp3 = root_node[tmp_state_1].tran_set;
            while (temp3 != NULL) {
              temp4 = root_node[tmp_state_2].tran_set;
              while (temp4 != NULL) {
                if (temp3->event == temp4->event) {
                  flag = 0;
                  tmp_state_3 = temp3->target_state_number;
                  tmp_state_4 = temp4->target_state_number;
                  if (indexx == 1) {
                    if (merging_table[tmp_state_3][tmp_state_4] == 0) {
                      temp4 = temp4->next_transition;
                      continue;
                    }
                    if (merging_table[tmp_state_3][tmp_state_4] == 1 &&
                        tmp_state_3 != tmp_state_4)
                      return 10;
                  }
                  if ((root_node[tmp_state_3].equal_set)->state_number !=
                      (root_node[tmp_state_4].equal_set)->state_number) {

                    /*****************************************************************/

                    if ((root_node[tmp_state_3].equal_set)->state_number <
                            nodel ||
                        (root_node[tmp_state_4].equal_set)->state_number <
                            nodel)
                      return 10;
                    /*****************************************************************/

                    tmpu_point = 0;
                    while (*(tmpu_stack + 2 * tmpu_point) != -1) {
                      if (*(tmpu_stack + 2 * tmpu_point) == tmp_state_3 &&
                          *(tmpu_stack + 2 * tmpu_point + 1) == tmp_state_4) {
                        flag = 8;
                        break;
                      }
                      if (*(tmpu_stack + 2 * tmpu_point + 1) == tmp_state_3 &&
                          *(tmpu_stack + 2 * tmpu_point) == tmp_state_4) {
                        flag = 8;
                        break;
                      }
                      tmpu_point += 1;
                    }
                    if (flag == 8) {
                      temp4 = temp4->next_transition;
                      continue;
                    }

                    // if(root_node[tmp_state_3].marked_in_plant==root_node[tmp_state_4].marked_in_plant
                    // &&
                    // root_node[tmp_state_3].marked_in_controller!=root_node[tmp_state_4].marked_in_controller)
                    //     return 10;

                    for (k = 1; k < 3; k++) {
                      if (k == 1) {
                        temp1 = root_node[tmp_state_3].forb_set;
                        temp2 = root_node[tmp_state_4].tran_set;
                      }
                      if (k == 2) {
                        temp1 = root_node[tmp_state_4].forb_set;
                        temp2 = root_node[tmp_state_3].tran_set;
                      }
                      temp22 = temp2;
                      while (temp1 != NULL) {
                        temp2 = temp22;
                        while (temp2 != NULL) {
                          if (temp1->event == temp2->event)
                            return 10;
                          temp2 = temp2->next_transition;
                        }
                        temp1 = temp1->next_event;
                      }
                    }
                    if (flag == 0) {
                      *(tmpu_stack + 2 * tmpu_point) = tmp_state_3;
                      *(tmpu_stack + 2 * tmpu_point + 1) = tmp_state_4;
                      *(tmpu_stack + 2 * tmpu_point + 2) = -1;
                      // zprints("Add to wait list:  "); zprintn(tmp_state_3);
                      // zprints("-"); zprintn(tmp_state_4); zprints("\n");
                      flag = Selfloop_Node1(tmp_state_3, tmp_state_4, nodel,
                                            indexx);
                      if (flag == 10) {
                        if (indexx == 1) {
                          merging_table[tmp_state_3][tmp_state_4] = 1;
                          merging_table[tmp_state_4][tmp_state_3] = 1;
                        }
                        return 10;
                      }
                    }
                  }
                }
                temp4 = temp4->next_transition;
              }
              temp3 = temp3->next_transition;
            }
          }
        }
      }
    }
  }
  return 9;
}

INT_OS Selfloop_Node2(INT_S base_state, INT_S tmp_state, INT_S nodel,
                      INT_OS indexx) {
  INT_S node_1, node_2, c_flag, flag, r_flag_1, r_flag_2;
  INT_S tmpu_point, tmpu_point_1, tmpu_point_2, k;
  struct forbidden_event_set *temp1, *temp11, *temp12;
  struct transitions *temp2; //, *temp22;
  struct equivalent_state_set *temp6, *temp7;
  INT_S tmp_state_1, tmp_state_2;
  INT_B cc_flag;

  node_1 = base_state;
  node_2 = tmp_state;
  temp1 = 0;
  temp2 = 0;

  temp6 = root_node[root_node[node_1].equal_set->state_number].equal_set;
  r_flag_1 = 0;
  tmpu_point_1 = 0;

  while (r_flag_1 == 0) {
    if (indexx == 0) {
      if (temp6 != NULL) {
        tmp_state_1 = temp6->state_number;
        temp6 = temp6->next_node;
      } else {
        tmp_state_1 = -1;
        while (*(tmpu_stack + 2 * tmpu_point_1) != -1) {
          if (*(tmpu_stack + 2 * tmpu_point_1) == node_1) {
            tmp_state_1 = *(tmpu_stack + 2 * tmpu_point_1 + 1);
            tmpu_point_1 += 1;
            break;
          }
          if (*(tmpu_stack + 2 * tmpu_point_1 + 1) == node_1) {
            tmp_state_1 = *(tmpu_stack + 2 * tmpu_point_1);
            tmpu_point_1 += 1;
            break;
          }
          tmpu_point_1 += 1;
        }
        if (tmp_state_1 == -1)
          r_flag_1 = 1;
        ;
      }
    } else {
      r_flag_1 = 1;
      tmp_state_1 = node_1;
    }

    if (tmp_state_1 != -1) {
      temp7 = root_node[root_node[node_2].equal_set->state_number].equal_set;
      r_flag_2 = 0;
      tmpu_point_2 = 0;
      while (r_flag_2 == 0) {
        if (indexx == 0) {
          if (temp7 != NULL) {
            tmp_state_2 = temp7->state_number;
            temp7 = temp7->next_node;
          } else {
            tmp_state_2 = -1;
            while (*(tmpu_stack + 2 * tmpu_point_2) != -1) {
              if (*(tmpu_stack + 2 * tmpu_point_2) == node_2) {
                tmp_state_2 = *(tmpu_stack + 2 * tmpu_point_2 + 1);
                tmpu_point_2 += 1;
                break;
              }
              if (*(tmpu_stack + 2 * tmpu_point_2 + 1) == node_2) {
                tmp_state_2 = *(tmpu_stack + 2 * tmpu_point_2);
                tmpu_point_2 += 1;
                break;
              }
              tmpu_point_2 += 1;
            }
            if (tmp_state_2 == -1)
              r_flag_2 = 1;
            ;
          }
        } else {
          r_flag_2 = 1;
          tmp_state_2 = node_2;
        }
        if (tmp_state_2 != -1) {
          c_flag = 0;
          if ((tmp_state_1 != node_1 || tmp_state_2 != node_2) &&
              (tmp_state_1 != node_2 || tmp_state_2 != node_1)) {
            if (indexx == 1) {
              if (merging_table[tmp_state_1][tmp_state_2] == 0)
                continue;
              if (merging_table[tmp_state_1][tmp_state_2] == 1 &&
                  tmp_state_1 != tmp_state_2)
                return 10;
            }
            if (root_node[tmp_state_1].equal_set->state_number ==
                root_node[tmp_state_2].equal_set->state_number)
              continue;

            tmpu_point = 0;
            while (*(tmpu_stack + 2 * tmpu_point) != -1) {
              if (*(tmpu_stack + 2 * tmpu_point) == tmp_state_1 &&
                  *(tmpu_stack + 2 * tmpu_point + 1) == tmp_state_2) {
                c_flag = 8;
                break;
              }
              if (*(tmpu_stack + 2 * tmpu_point + 1) == tmp_state_1 &&
                  *(tmpu_stack + 2 * tmpu_point) == tmp_state_2) {
                c_flag = 8;
                break;
              }
              tmpu_point += 1;
            }
            if (c_flag == 8)
              continue;

            temp1 = root_node[tmp_state_1].forb_set;
            temp11 = root_node[tmp_state_2].forb_set;

            if ((temp1 == NULL && temp11 != NULL) ||
                (temp1 != NULL && temp11 == NULL)) {
              return 10;
            } else {
              for (k = 1; k < 3; k++) {
                if (k == 2) {
                  temp1 = root_node[tmp_state_2].forb_set;
                  temp11 = root_node[tmp_state_1].forb_set;
                }
                while (temp1 != NULL) {
                  temp12 = temp11;
                  cc_flag = false;
                  while (temp12 != NULL) {
                    if (temp1->event == temp12->event) {
                      cc_flag = true;
                      break;
                    }
                    temp12 = temp12->next_event;
                  }
                  if (!cc_flag)
                    return 10;
                  temp1 = temp1->next_event;
                }
              }
            }

            // if(root_node[tmp_state_1].marked_in_plant==root_node[tmp_state_2].marked_in_plant
            // &&
            // root_node[tmp_state_1].marked_in_controller!=root_node[tmp_state_2].marked_in_controller)
            //        return 10;

            /* for(k=1;k<3;k++)
            {
            if(k==1)
            {
            temp1 = root_node[tmp_state_1].forb_set;
            temp2 = root_node[tmp_state_2].tran_set;
            }
            if(k==2)
            {
            temp1 = root_node[tmp_state_2].forb_set;
            temp2 = root_node[tmp_state_1].tran_set;
            }
            temp22 = temp2;
            while(temp1 != NULL)
            {
            temp2 = temp22;
            while(temp2 != NULL)
            {
            if(temp1->event == temp2->event)
            return 10;
            temp2 = temp2->next_transition;
            }
            temp1 = temp1->next_event;
            }
            }*/
            if (c_flag != 8) {
              *(tmpu_stack + 2 * tmpu_point) = tmp_state_1;
              *(tmpu_stack + 2 * tmpu_point + 1) = tmp_state_2;
              *(tmpu_stack + 2 * tmpu_point + 2) = -1;
            }
          }
          if (c_flag == 0) {
            struct transitions *temp3, *temp4;
            INT_S tmp_state_3, tmp_state_4;
            temp3 = root_node[tmp_state_1].tran_set;
            while (temp3 != NULL) {
              temp4 = root_node[tmp_state_2].tran_set;
              while (temp4 != NULL) {
                if (temp3->event == temp4->event) {
                  flag = 0;
                  tmp_state_3 = temp3->target_state_number;
                  tmp_state_4 = temp4->target_state_number;
                  if (indexx == 1) {
                    if (merging_table[tmp_state_3][tmp_state_4] == 0) {
                      temp4 = temp4->next_transition;
                      continue;
                    }
                    if (merging_table[tmp_state_3][tmp_state_4] == 1 &&
                        tmp_state_3 != tmp_state_4)
                      return 10;
                  }
                  if ((root_node[tmp_state_3].equal_set)->state_number !=
                      (root_node[tmp_state_4].equal_set)->state_number) {

                    /*****************************************************************/

                    if ((root_node[tmp_state_3].equal_set)->state_number <
                            nodel ||
                        (root_node[tmp_state_4].equal_set)->state_number <
                            nodel)
                      return 10;
                    /*****************************************************************/

                    tmpu_point = 0;
                    while (*(tmpu_stack + 2 * tmpu_point) != -1) {
                      if (*(tmpu_stack + 2 * tmpu_point) == tmp_state_3 &&
                          *(tmpu_stack + 2 * tmpu_point + 1) == tmp_state_4) {
                        flag = 8;
                        break;
                      }
                      if (*(tmpu_stack + 2 * tmpu_point + 1) == tmp_state_3 &&
                          *(tmpu_stack + 2 * tmpu_point) == tmp_state_4) {
                        flag = 8;
                        break;
                      }
                      tmpu_point += 1;
                    }
                    if (flag == 8) {
                      temp4 = temp4->next_transition;
                      continue;
                    }

                    // if(root_node[tmp_state_3].marked_in_plant==root_node[tmp_state_4].marked_in_plant
                    // &&
                    // root_node[tmp_state_3].marked_in_controller!=root_node[tmp_state_4].marked_in_controller)
                    //     return 10;
                    temp1 = root_node[tmp_state_3].forb_set;
                    temp11 = root_node[tmp_state_4].forb_set;

                    if ((temp1 == NULL && temp11 != NULL) ||
                        (temp1 != NULL && temp11 == NULL)) {
                      return 10;
                    } else {
                      for (k = 1; k < 3; k++) {
                        if (k == 2) {
                          temp1 = root_node[tmp_state_4].forb_set;
                          temp11 = root_node[tmp_state_3].forb_set;
                        }
                        while (temp1 != NULL) {
                          temp12 = temp11;
                          cc_flag = false;
                          while (temp12 != NULL) {
                            if (temp1->event == temp12->event) {
                              cc_flag = true;
                              break;
                            }
                            temp12 = temp12->next_event;
                          }
                          if (!cc_flag)
                            return 10;
                          temp1 = temp1->next_event;
                        }
                      }
                    }
                    if (flag == 0) {
                      *(tmpu_stack + 2 * tmpu_point) = tmp_state_3;
                      *(tmpu_stack + 2 * tmpu_point + 1) = tmp_state_4;
                      *(tmpu_stack + 2 * tmpu_point + 2) = -1;
                      flag = Selfloop_Node2(tmp_state_3, tmp_state_4, nodel,
                                            indexx);
                      if (flag == 10) {
                        if (indexx == 1) {
                          merging_table[tmp_state_3][tmp_state_4] = 1;
                          merging_table[tmp_state_4][tmp_state_3] = 1;
                        }
                        return 10;
                      }
                    }
                  }
                }
                temp4 = temp4->next_transition;
              }
              temp3 = temp3->next_transition;
            }
          }
        }
      }
    }
  }
  return 9;
}

INT_OS Selfloop_Node3(INT_S base_state, INT_S tmp_state, INT_S nodel,
                      INT_OS indexx) {
  INT_S node_1, node_2, c_flag, flag, r_flag_1, r_flag_2;
  INT_S tmpu_point, tmpu_point_1, tmpu_point_2; //,k;
  struct forbidden_event_set *temp1;            //, *temp11;
  struct transitions *temp2;                    // *temp22;
  struct equivalent_state_set *temp6, *temp7;
  INT_S tmp_state_1, tmp_state_2;

  node_1 = base_state;
  node_2 = tmp_state;
  temp1 = 0;
  temp2 = 0;

  temp6 = root_node[root_node[node_1].equal_set->state_number].equal_set;
  r_flag_1 = 0;
  tmpu_point_1 = 0;

  while (r_flag_1 == 0) {
    if (indexx == 0) {
      if (temp6 != NULL) {
        tmp_state_1 = temp6->state_number;
        temp6 = temp6->next_node;
      } else {
        tmp_state_1 = -1;
        while (*(tmpu_stack + 2 * tmpu_point_1) != -1) {
          if (*(tmpu_stack + 2 * tmpu_point_1) == node_1) {
            tmp_state_1 = *(tmpu_stack + 2 * tmpu_point_1 + 1);
            tmpu_point_1 += 1;
            break;
          }
          if (*(tmpu_stack + 2 * tmpu_point_1 + 1) == node_1) {
            tmp_state_1 = *(tmpu_stack + 2 * tmpu_point_1);
            tmpu_point_1 += 1;
            break;
          }
          tmpu_point_1 += 1;
        }
        if (tmp_state_1 == -1)
          r_flag_1 = 1;
        ;
      }
    } else {
      r_flag_1 = 1;
      tmp_state_1 = node_1;
    }

    if (tmp_state_1 != -1) {
      temp7 = root_node[root_node[node_2].equal_set->state_number].equal_set;
      r_flag_2 = 0;
      tmpu_point_2 = 0;
      while (r_flag_2 == 0) {
        if (indexx == 0) {
          if (temp7 != NULL) {
            tmp_state_2 = temp7->state_number;
            temp7 = temp7->next_node;
          } else {
            tmp_state_2 = -1;
            while (*(tmpu_stack + 2 * tmpu_point_2) != -1) {
              if (*(tmpu_stack + 2 * tmpu_point_2) == node_2) {
                tmp_state_2 = *(tmpu_stack + 2 * tmpu_point_2 + 1);
                tmpu_point_2 += 1;
                break;
              }
              if (*(tmpu_stack + 2 * tmpu_point_2 + 1) == node_2) {
                tmp_state_2 = *(tmpu_stack + 2 * tmpu_point_2);
                tmpu_point_2 += 1;
                break;
              }
              tmpu_point_2 += 1;
            }
            if (tmp_state_2 == -1)
              r_flag_2 = 1;
            ;
          }
        } else {
          r_flag_2 = 1;
          tmp_state_2 = node_2;
        }
        if (tmp_state_2 != -1) {
          c_flag = 0;
          if ((tmp_state_1 != node_1 || tmp_state_2 != node_2) &&
              (tmp_state_1 != node_2 || tmp_state_2 != node_1)) {
            if (indexx == 1) {
              if (merging_table[tmp_state_1][tmp_state_2] == 0)
                continue;
              if (merging_table[tmp_state_1][tmp_state_2] == 1 &&
                  tmp_state_1 != tmp_state_2)
                return 10;
            }
            if (root_node[tmp_state_1].equal_set->state_number ==
                root_node[tmp_state_2].equal_set->state_number)
              continue;

            tmpu_point = 0;
            while (*(tmpu_stack + 2 * tmpu_point) != -1) {
              if (*(tmpu_stack + 2 * tmpu_point) == tmp_state_1 &&
                  *(tmpu_stack + 2 * tmpu_point + 1) == tmp_state_2) {
                c_flag = 8;
                break;
              }
              if (*(tmpu_stack + 2 * tmpu_point + 1) == tmp_state_1 &&
                  *(tmpu_stack + 2 * tmpu_point) == tmp_state_2) {
                c_flag = 8;
                break;
              }
              tmpu_point += 1;
            }
            if (c_flag == 8)
              continue;

            /*temp1 = root_node[tmp_state_1].forb_set;
            temp11 = root_node[tmp_state_2].forb_set;

            if((temp1 == NULL && temp11 != NULL) || (temp1 != NULL && temp11 ==
            NULL)) return 10;*/

            if (root_node[tmp_state_1].marked_in_controller !=
                root_node[tmp_state_2].marked_in_controller)
              return 10;

            /* for(k=1;k<3;k++)
            {
            if(k==1)
            {
            temp1 = root_node[tmp_state_1].forb_set;
            temp2 = root_node[tmp_state_2].tran_set;
            }
            if(k==2)
            {
            temp1 = root_node[tmp_state_2].forb_set;
            temp2 = root_node[tmp_state_1].tran_set;
            }
            temp22 = temp2;
            while(temp1 != NULL)
            {
            temp2 = temp22;
            while(temp2 != NULL)
            {
            if(temp1->event == temp2->event)
            return 10;
            temp2 = temp2->next_transition;
            }
            temp1 = temp1->next_event;
            }
            }*/
            if (c_flag != 8) {
              *(tmpu_stack + 2 * tmpu_point) = tmp_state_1;
              *(tmpu_stack + 2 * tmpu_point + 1) = tmp_state_2;
              *(tmpu_stack + 2 * tmpu_point + 2) = -1;
            }
          }
          if (c_flag == 0) {
            struct transitions *temp3, *temp4;
            INT_S tmp_state_3, tmp_state_4;
            temp3 = root_node[tmp_state_1].tran_set;
            while (temp3 != NULL) {
              temp4 = root_node[tmp_state_2].tran_set;
              while (temp4 != NULL) {
                if (temp3->event == temp4->event) {
                  flag = 0;
                  tmp_state_3 = temp3->target_state_number;
                  tmp_state_4 = temp4->target_state_number;
                  if (indexx == 1) {
                    if (merging_table[tmp_state_3][tmp_state_4] == 0) {
                      temp4 = temp4->next_transition;
                      continue;
                    }
                    if (merging_table[tmp_state_3][tmp_state_4] == 1 &&
                        tmp_state_3 != tmp_state_4)
                      return 10;
                  }
                  if ((root_node[tmp_state_3].equal_set)->state_number !=
                      (root_node[tmp_state_4].equal_set)->state_number) {

                    /*****************************************************************/

                    if ((root_node[tmp_state_3].equal_set)->state_number <
                            nodel ||
                        (root_node[tmp_state_4].equal_set)->state_number <
                            nodel)
                      return 10;
                    /*****************************************************************/

                    tmpu_point = 0;
                    while (*(tmpu_stack + 2 * tmpu_point) != -1) {
                      if (*(tmpu_stack + 2 * tmpu_point) == tmp_state_3 &&
                          *(tmpu_stack + 2 * tmpu_point + 1) == tmp_state_4) {
                        flag = 8;
                        break;
                      }
                      if (*(tmpu_stack + 2 * tmpu_point + 1) == tmp_state_3 &&
                          *(tmpu_stack + 2 * tmpu_point) == tmp_state_4) {
                        flag = 8;
                        break;
                      }
                      tmpu_point += 1;
                    }
                    if (flag == 8) {
                      temp4 = temp4->next_transition;
                      continue;
                    }

                    if (root_node[tmp_state_3].marked_in_controller !=
                        root_node[tmp_state_4].marked_in_controller)
                      return 10;
                    /*temp1 = root_node[tmp_state_3].forb_set;
                    temp11 = root_node[tmp_state_4].forb_set;

                    if((temp1 == NULL && temp11 != NULL) || (temp1 != NULL &&
                    temp11 == NULL)) return 10;*/
                    /*
                    for(k=1;k<3;k++)
                    {
                    if(k==1)
                    {
                    temp1 = root_node[tmp_state_3].forb_set;
                    temp2 = root_node[tmp_state_4].tran_set;
                    }
                    if(k==2)
                    {
                    temp1 = root_node[tmp_state_4].forb_set;
                    temp2 = root_node[tmp_state_3].tran_set;
                    }
                    temp22 = temp2;
                    while(temp1 != NULL)
                    {
                    temp2 = temp22;
                    while(temp2 != NULL)
                    {
                    if(temp1->event == temp2->event)
                    return 10;
                    temp2 = temp2->next_transition;
                    }
                    temp1 = temp1->next_event;
                    }
                    }*/
                    if (flag == 0) {
                      *(tmpu_stack + 2 * tmpu_point) = tmp_state_3;
                      *(tmpu_stack + 2 * tmpu_point + 1) = tmp_state_4;
                      *(tmpu_stack + 2 * tmpu_point + 2) = -1;
                      flag = Selfloop_Node3(tmp_state_3, tmp_state_4, nodel,
                                            indexx);
                      if (flag == 10) {
                        if (indexx == 1) {
                          merging_table[tmp_state_3][tmp_state_4] = 1;
                          merging_table[tmp_state_4][tmp_state_3] = 1;
                        }
                        return 10;
                      }
                    }
                  }
                }
                temp4 = temp4->next_transition;
              }
              temp3 = temp3->next_transition;
            }
          }
        }
      }
    }
  }
  return 9;
}

void drawn_check_mark(INT_S state1, INT_S state2) {
  INT_S tmpu_point, k;
  INT_S tmp_state1, tmp_state2, temp;

  if (state1 < state2) {
    temp = state2 - 1 - state1;
    if (track_table[state1].next[temp].flag == 1)
      track_table[state1].next[temp].flag = 2;
    else if (track_table[state1].next[temp].flag == 2) {
      return;
    } else {
      ;
    }

    for (k = 0; k < track_table[state1].next[temp].numelts; k++) {
      tmpu_point = track_table[state1].next[temp].dynindex[k];
      tmp_state1 = *(tmpu_stack + 2 * tmpu_point);
      tmp_state2 = *(tmpu_stack + 2 * tmpu_point + 1);
      drawn_check_mark(tmp_state1, tmp_state2);
    }
  } else {
    temp = state1 - 1 - state2;
    if (track_table[state2].next[temp].flag == 1)
      track_table[state2].next[temp].flag = 2;
    else if (track_table[state2].next[temp].flag == 2) {
      return;
    } else {
      ;
    }

    for (k = 0; k < track_table[state2].next[temp].numelts; k++) {
      tmpu_point = track_table[state2].next[temp].dynindex[k];
      tmp_state1 = *(tmpu_stack + 2 * tmpu_point);
      tmp_state2 = *(tmpu_stack + 2 * tmpu_point + 1);
      drawn_check_mark(tmp_state1, tmp_state2);
    }
  }
}

INT_OS Selfloop_Node5(INT_S base_state, INT_S tmp_state, INT_S nodel,
                      INT_OS indexx) {
  INT_S node_1, node_2, c_flag, flag, r_flag_1, r_flag_2;
  INT_S tmpu_point, tmpu_point_1, tmpu_point_2, k;
  struct forbidden_event_set *temp1;
  struct transitions *temp2, *temp22;
  struct equivalent_state_set *temp6, *temp7;
  INT_S tmp_state_1, tmp_state_2;
  // INT_S cur_1, cur_2, tmp_cur_1, tmp_cur_2, tmp_cur_3, tmp_cur_4;
  struct check_mark *cm_tmp1, *cm_tmp2, *cm_tmp3;

  node_1 = base_state;
  node_2 = tmp_state;
  temp1 = 0;
  temp2 = 0;

  if (node_1 < node_2) {
    // cur_1 = node_1;
    // cur_2 = node_2 - 1 - cur_1;
    cm_tmp1 = &(track_table[node_1].next[node_2 - 1 - node_1]);
  } else {
    // cur_1 = node_2;
    // cur_2 = node_1 - 1 - cur_1;
    cm_tmp1 = &(track_table[node_2].next[node_1 - 1 - node_2]);
  }

  // zprintn(node_1); zprints("-"); zprintn(node_2); zprints("\n");

  temp6 = root_node[root_node[node_1].equal_set->state_number].equal_set;
  r_flag_1 = 0;
  tmpu_point_1 = 0;

  while (r_flag_1 == 0) {
    if (indexx == 0) {
      if (temp6 != NULL) {
        tmp_state_1 = temp6->state_number;
        temp6 = temp6->next_node;
      } else {
        tmp_state_1 = -1;
        while (*(tmpu_stack + 2 * tmpu_point_1) != -1) {
          if (*(tmpu_stack + 2 * tmpu_point_1) == node_1) {
            tmp_state_1 = *(tmpu_stack + 2 * tmpu_point_1 + 1);
            tmpu_point_1 += 1;
            break;
          }
          if (*(tmpu_stack + 2 * tmpu_point_1 + 1) == node_1) {
            tmp_state_1 = *(tmpu_stack + 2 * tmpu_point_1);
            tmpu_point_1 += 1;
            break;
          }
          tmpu_point_1 += 1;
        }
        if (tmp_state_1 == -1)
          r_flag_1 = 1;
        ;
      }
    } else {
      r_flag_1 = 1;
      tmp_state_1 = node_1;
    }

    if (tmp_state_1 != -1) {
      temp7 = root_node[root_node[node_2].equal_set->state_number].equal_set;
      r_flag_2 = 0;
      tmpu_point_2 = 0;
      while (r_flag_2 == 0) {
        if (indexx == 0) {
          if (temp7 != NULL) {
            tmp_state_2 = temp7->state_number;
            temp7 = temp7->next_node;
          } else {
            tmp_state_2 = -1;
            while (*(tmpu_stack + 2 * tmpu_point_2) != -1) {
              if (*(tmpu_stack + 2 * tmpu_point_2) == node_2) {
                tmp_state_2 = *(tmpu_stack + 2 * tmpu_point_2 + 1);
                tmpu_point_2 += 1;
                break;
              }
              if (*(tmpu_stack + 2 * tmpu_point_2 + 1) == node_2) {
                tmp_state_2 = *(tmpu_stack + 2 * tmpu_point_2);
                tmpu_point_2 += 1;
                break;
              }
              tmpu_point_2 += 1;
            }
            if (tmp_state_2 == -1)
              r_flag_2 = 1;
            ;
          }
        } else {
          r_flag_2 = 1;
          tmp_state_2 = node_2;
        }
        if (tmp_state_2 != -1) {
          if (tmp_state_1 == tmp_state_2) {
            continue;
          } else if (tmp_state_1 < tmp_state_2) {
            cm_tmp2 =
                &(track_table[tmp_state_1].next[tmp_state_2 - 1 - tmp_state_1]);
          } else {
            cm_tmp2 =
                &(track_table[tmp_state_2].next[tmp_state_1 - 1 - tmp_state_2]);
          }
          if (cm_tmp2->flag == 3) {
            return 10;
          }

          c_flag = 0;
          if ((tmp_state_1 != node_1 || tmp_state_2 != node_2) &&
              (tmp_state_1 != node_2 || tmp_state_2 != node_1)) {
            // if(indexx == 1)
            //{
            //	if(merging_table[tmp_state_1][tmp_state_2] == 0) continue;
            //	if(merging_table[tmp_state_1][tmp_state_2] == 1 && tmp_state_1
            //!= tmp_state_2) return 10;
            //}
            if (root_node[tmp_state_1].equal_set->state_number ==
                root_node[tmp_state_2].equal_set->state_number)
              continue;

            tmpu_point = 0;
            while (*(tmpu_stack + 2 * tmpu_point) != -1) {
              if (*(tmpu_stack + 2 * tmpu_point) == tmp_state_1 &&
                  *(tmpu_stack + 2 * tmpu_point + 1) == tmp_state_2) {
                c_flag = 8;
                break;
              }
              if (*(tmpu_stack + 2 * tmpu_point + 1) == tmp_state_1 &&
                  *(tmpu_stack + 2 * tmpu_point) == tmp_state_2) {
                c_flag = 8;
                break;
              }
              tmpu_point += 1;
            }
            if (c_flag == 8) {
              cm_tmp1->numelts += 1;
              cm_tmp1->dynindex = (INT_S *)REALLOC(
                  cm_tmp1->dynindex, (cm_tmp1->numelts) * sizeof(INT_S));
              cm_tmp1->dynindex[cm_tmp1->numelts - 1] = tmpu_point;
              if (cm_tmp2->flag == 1) {
                if (cm_tmp1->minindex > tmpu_point) {
                  cm_tmp1->minindex = tmpu_point;
                }
              }
              continue;
            }

            // if(root_node[tmp_state_1].marked_in_plant==root_node[tmp_state_2].marked_in_plant
            // &&
            // root_node[tmp_state_1].marked_in_controller!=root_node[tmp_state_2].marked_in_controller)
            //        return 10;

            for (k = 1; k < 3; k++) {
              if (k == 1) {
                temp1 = root_node[tmp_state_1].forb_set;
                temp2 = root_node[tmp_state_2].tran_set;
              }
              if (k == 2) {
                temp1 = root_node[tmp_state_2].forb_set;
                temp2 = root_node[tmp_state_1].tran_set;
              }
              temp22 = temp2;
              while (temp1 != NULL) {
                temp2 = temp22;
                while (temp2 != NULL) {
                  if (temp1->event == temp2->event) {
                    cm_tmp2->flag = 3;
                    return 10;
                  }
                  temp2 = temp2->next_transition;
                }
                temp1 = temp1->next_event;
              }
            }
            if (c_flag != 8) {
              *(tmpu_stack + 2 * tmpu_point) = tmp_state_1;
              *(tmpu_stack + 2 * tmpu_point + 1) = tmp_state_2;
              *(tmpu_stack + 2 * tmpu_point + 2) = -1;

              // update indices of cur_1 and cur_2

              cm_tmp2->flag = 1;
              cm_tmp2->index = tmpu_point;
              cm_tmp2->minindex = tmpu_point;

              // update indices of cur_1 and cur_2
              cm_tmp1->numelts += 1;
              cm_tmp1->dynindex = (INT_S *)REALLOC(
                  cm_tmp1->dynindex, (cm_tmp1->numelts) * sizeof(INT_S));
              cm_tmp1->dynindex[cm_tmp1->numelts - 1] = tmpu_point;
              if (cm_tmp1->minindex > tmpu_point) {
                cm_tmp1->minindex = tmpu_point;
              }

              // zprints("Add to wait list:  "); zprintn(tmp_state_1);
              // zprints("-"); zprintn(tmp_state_2); zprints("\n");
            }
          }
          if (c_flag == 0) {
            struct transitions *temp3, *temp4;
            INT_S tmp_state_3, tmp_state_4;
            temp3 = root_node[tmp_state_1].tran_set;
            while (temp3 != NULL) {
              temp4 = root_node[tmp_state_2].tran_set;
              while (temp4 != NULL) {
                if (temp3->event == temp4->event) {
                  flag = 0;
                  tmp_state_3 = temp3->target_state_number;
                  tmp_state_4 = temp4->target_state_number;
                  // if(indexx == 1)
                  //{
                  //	if(merging_table[tmp_state_3][tmp_state_4] == 0)
                  //	{
                  //		temp4 = temp4->next_transition;
                  //		continue;
                  //	}
                  // if(merging_table[tmp_state_3][tmp_state_4] == 1 &&
                  // tmp_state_3 != tmp_state_4) return 10;
                  //}

                  if ((root_node[tmp_state_3].equal_set)->state_number !=
                      (root_node[tmp_state_4].equal_set)->state_number) {

                    if (tmp_state_3 < tmp_state_4) {
                      // tmp_cur_3 = tmp_state_3;
                      // tmp_cur_4 = tmp_state_4 - 1 - tmp_cur_3;
                      cm_tmp3 = &(track_table[tmp_state_3]
                                      .next[tmp_state_4 - 1 - tmp_state_3]);
                    } else {
                      // tmp_cur_3 = tmp_state_4;
                      // tmp_cur_4 = tmp_state_3 - 1 - tmp_cur_3;
                      cm_tmp3 = &(track_table[tmp_state_4]
                                      .next[tmp_state_3 - 1 - tmp_state_4]);
                    }

                    if (cm_tmp3->flag == 3) {
                      return 10;
                    }

                    /*****************************************************************/

                    if ((root_node[tmp_state_3].equal_set)->state_number <
                            nodel ||
                        (root_node[tmp_state_4].equal_set)->state_number <
                            nodel)
                      return 10;
                    /*****************************************************************/

                    tmpu_point = 0;
                    while (*(tmpu_stack + 2 * tmpu_point) != -1) {
                      if (*(tmpu_stack + 2 * tmpu_point) == tmp_state_3 &&
                          *(tmpu_stack + 2 * tmpu_point + 1) == tmp_state_4) {
                        flag = 8;
                        break;
                      }
                      if (*(tmpu_stack + 2 * tmpu_point + 1) == tmp_state_3 &&
                          *(tmpu_stack + 2 * tmpu_point) == tmp_state_4) {
                        flag = 8;
                        break;
                      }
                      tmpu_point += 1;
                    }
                    if (flag == 8) {
                      cm_tmp2->numelts += 1;
                      cm_tmp2->dynindex =
                          (INT_S *)REALLOC(cm_tmp2->dynindex,
                                           (cm_tmp2->numelts) * sizeof(INT_S));
                      cm_tmp2->dynindex[cm_tmp2->numelts - 1] = tmpu_point;

                      if (cm_tmp3->flag == 1) {
                        if (cm_tmp2->minindex > tmpu_point) {
                          cm_tmp2->minindex = tmpu_point;
                        }
                        if (cm_tmp1->minindex > cm_tmp2->minindex)
                          cm_tmp1->minindex = cm_tmp2->minindex;
                      }

                      temp4 = temp4->next_transition;
                      continue;
                    }

                    // if(root_node[tmp_state_3].marked_in_plant==root_node[tmp_state_4].marked_in_plant
                    // &&
                    // root_node[tmp_state_3].marked_in_controller!=root_node[tmp_state_4].marked_in_controller)
                    //     return 10;

                    for (k = 1; k < 3; k++) {
                      if (k == 1) {
                        temp1 = root_node[tmp_state_3].forb_set;
                        temp2 = root_node[tmp_state_4].tran_set;
                      }
                      if (k == 2) {
                        temp1 = root_node[tmp_state_4].forb_set;
                        temp2 = root_node[tmp_state_3].tran_set;
                      }
                      temp22 = temp2;
                      while (temp1 != NULL) {
                        temp2 = temp22;
                        while (temp2 != NULL) {
                          if (temp1->event == temp2->event) {
                            cm_tmp3->flag = 3;
                            return 10;
                          }
                          temp2 = temp2->next_transition;
                        }
                        temp1 = temp1->next_event;
                      }
                    }
                    if (flag == 0) {
                      *(tmpu_stack + 2 * tmpu_point) = tmp_state_3;
                      *(tmpu_stack + 2 * tmpu_point + 1) = tmp_state_4;
                      *(tmpu_stack + 2 * tmpu_point + 2) = -1;

                      cm_tmp3->flag = 1;
                      cm_tmp3->index = tmpu_point;
                      cm_tmp3->minindex = tmpu_point;

                      cm_tmp2->numelts += 1;
                      cm_tmp2->dynindex =
                          (INT_S *)REALLOC(cm_tmp2->dynindex,
                                           (cm_tmp2->numelts) * sizeof(INT_S));
                      cm_tmp2->dynindex[cm_tmp2->numelts - 1] = tmpu_point;

                      // zprints("Add to wait list:  "); zprintn(tmp_state_3);
                      // zprints("-"); zprintn(tmp_state_4); zprints("\n");
                      flag = Selfloop_Node5(tmp_state_3, tmp_state_4, nodel,
                                            indexx);

                      if (flag == 10) {
                        // if(indexx == 1)
                        //{
                        //	merging_table[tmp_state_3][tmp_state_4] = 1;
                        //	merging_table[tmp_state_4][tmp_state_3] = 1;
                        //}
                        return 10;
                      }

                      if (cm_tmp3->flag == 1) {
                        if (cm_tmp2->minindex > cm_tmp3->minindex) {
                          cm_tmp2->minindex = cm_tmp3->minindex;
                        }
                        if (cm_tmp1->minindex > cm_tmp2->minindex)
                          cm_tmp1->minindex = cm_tmp2->minindex;
                      }
                    }
                  }
                }
                temp4 = temp4->next_transition;
              }
              temp3 = temp3->next_transition;
            }
          }
        }
      }
    }
  }
  if (cm_tmp1->minindex == cm_tmp1->index) {
    drawn_check_mark(node_1, node_2);
  }
  return 9;
}

void drawn_check_mark1(INT_S pos) {
  INT_S temp3, temp4, temp_state;
  struct equivalent_state_set **temp5, *temp6;
  INT_S tmpu_point;
  INT_S state_1, state_2;
  check_mark *cm_tmp2;

  tmpu_point = pos;
  while (*(tmpu_stack + 2 * tmpu_point) != -1) {
    state_1 = *(tmpu_stack + 2 * tmpu_point);
    state_2 = *(tmpu_stack + 2 * tmpu_point + 1);
    if (state_1 < state_2) {
      cm_tmp2 = &(track_table[state_1].next[state_2 - 1 - state_1]);
    } else {
      cm_tmp2 = &(track_table[state_2].next[state_1 - 1 - state_2]);
    }
    if (cm_tmp2->flag == 3) {
      tmpu_point += 1;
      continue;
    }

    cm_tmp2->flag = 2;

    temp3 = root_node[state_1].equal_set->state_number;
    temp4 = root_node[state_2].equal_set->state_number;
    if (temp3 < temp4) {
      temp5 = &(root_node[temp3].equal_set); /*acceptor*/
      temp6 = root_node[temp4].equal_set;    /*merger*/
    } else if (temp3 > temp4) {
      temp5 = &(root_node[temp4].equal_set); /*acceptor*/
      temp6 = root_node[temp3].equal_set;    /*merger*/
    }
    temp_state = (*temp5)->state_number;
    if (temp3 == temp4) {
      tmpu_point += 1;
      continue;
    }
    while (*temp5 != NULL)
      temp5 = &((*temp5)->next_node);
    while (temp6 != NULL) {
      *temp5 = (struct equivalent_state_set *)MALLOC(
          sizeof(struct equivalent_state_set));
      (*temp5)->state_number = temp6->state_number;
      root_node[temp6->state_number].equal_set->state_number = temp_state;
      temp6 = temp6->next_node;
      if (temp6 != NULL)
        temp5 = &((*temp5)->next_node);
      else
        (*temp5)->next_node = NULL;
    }
    tmpu_point += 1;
  }
  *(tmpu_stack + 2 * pos) = -1;
}

INT_OS Selfloop_Node51(INT_S base_state, INT_S tmp_state, INT_S nodel,
                       INT_OS indexx) {
  INT_S node_1, node_2, c_flag, flag, r_flag_1, r_flag_2;
  INT_S tmpu_point, tmpu_point_1, tmpu_point_2, k;
  struct forbidden_event_set *temp1;
  struct transitions *temp2, *temp22;
  struct equivalent_state_set *temp6, *temp7;
  INT_S tmp_state_1, tmp_state_2;
  // INT_S cur_1, cur_2, tmp_cur_1, tmp_cur_2, tmp_cur_3, tmp_cur_4;
  struct check_mark *cm_tmp1, *cm_tmp2, *cm_tmp3;

  node_1 = base_state;
  node_2 = tmp_state;
  temp1 = 0;
  temp2 = 0;

  if (node_1 < node_2) {
    // cur_1 = node_1;
    // cur_2 = node_2 - 1 - cur_1;
    cm_tmp1 = &(track_table[node_1].next[node_2 - 1 - node_1]);
  } else {
    // cur_1 = node_2;
    // cur_2 = node_1 - 1 - cur_1;
    cm_tmp1 = &(track_table[node_2].next[node_1 - 1 - node_2]);
  }

  // zprintn(node_1); zprints("-"); zprintn(node_2); zprints("\n");

  temp6 = root_node[root_node[node_1].equal_set->state_number].equal_set;
  r_flag_1 = 0;
  tmpu_point_1 = 0;

  while (r_flag_1 == 0) {
    if (indexx == 0) {
      if (temp6 != NULL) {
        tmp_state_1 = temp6->state_number;
        temp6 = temp6->next_node;
      } else {
        tmp_state_1 = -1;
        while (*(tmpu_stack + 2 * tmpu_point_1) != -1) {
          if (*(tmpu_stack + 2 * tmpu_point_1) == node_1) {
            tmp_state_1 = *(tmpu_stack + 2 * tmpu_point_1 + 1);
            tmpu_point_1 += 1;
            break;
          }
          if (*(tmpu_stack + 2 * tmpu_point_1 + 1) == node_1) {
            tmp_state_1 = *(tmpu_stack + 2 * tmpu_point_1);
            tmpu_point_1 += 1;
            break;
          }
          tmpu_point_1 += 1;
        }
        if (tmp_state_1 == -1)
          r_flag_1 = 1;
        ;
      }
    } else {
      r_flag_1 = 1;
      tmp_state_1 = node_1;
    }

    if (tmp_state_1 != -1) {
      temp7 = root_node[root_node[node_2].equal_set->state_number].equal_set;
      r_flag_2 = 0;
      tmpu_point_2 = 0;
      while (r_flag_2 == 0) {
        if (indexx == 0) {
          if (temp7 != NULL) {
            tmp_state_2 = temp7->state_number;
            temp7 = temp7->next_node;
          } else {
            tmp_state_2 = -1;
            while (*(tmpu_stack + 2 * tmpu_point_2) != -1) {
              if (*(tmpu_stack + 2 * tmpu_point_2) == node_2) {
                tmp_state_2 = *(tmpu_stack + 2 * tmpu_point_2 + 1);
                tmpu_point_2 += 1;
                break;
              }
              if (*(tmpu_stack + 2 * tmpu_point_2 + 1) == node_2) {
                tmp_state_2 = *(tmpu_stack + 2 * tmpu_point_2);
                tmpu_point_2 += 1;
                break;
              }
              tmpu_point_2 += 1;
            }
            if (tmp_state_2 == -1)
              r_flag_2 = 1;
            ;
          }
        } else {
          r_flag_2 = 1;
          tmp_state_2 = node_2;
        }
        if (tmp_state_2 != -1) {
          if (tmp_state_1 == tmp_state_2) {
            continue;
          } else if (tmp_state_1 < tmp_state_2) {
            cm_tmp2 =
                &(track_table[tmp_state_1].next[tmp_state_2 - 1 - tmp_state_1]);
          } else {
            cm_tmp2 =
                &(track_table[tmp_state_2].next[tmp_state_1 - 1 - tmp_state_2]);
          }
          if (cm_tmp2->flag == 3) {
            return 10;
          }

          c_flag = 0;
          if ((tmp_state_1 != node_1 || tmp_state_2 != node_2) &&
              (tmp_state_1 != node_2 || tmp_state_2 != node_1)) {
            // if(indexx == 1)
            //{
            //	if(merging_table[tmp_state_1][tmp_state_2] == 0) continue;
            //	if(merging_table[tmp_state_1][tmp_state_2] == 1 && tmp_state_1
            //!= tmp_state_2) return 10;
            //}
            if (root_node[tmp_state_1].equal_set->state_number ==
                root_node[tmp_state_2].equal_set->state_number)
              continue;

            tmpu_point = 0;
            while (*(tmpu_stack + 2 * tmpu_point) != -1) {
              if (*(tmpu_stack + 2 * tmpu_point) == tmp_state_1 &&
                  *(tmpu_stack + 2 * tmpu_point + 1) == tmp_state_2) {
                c_flag = 8;
                break;
              }
              if (*(tmpu_stack + 2 * tmpu_point + 1) == tmp_state_1 &&
                  *(tmpu_stack + 2 * tmpu_point) == tmp_state_2) {
                c_flag = 8;
                break;
              }
              tmpu_point += 1;
            }
            if (c_flag == 8) {
              // cm_tmp1->numelts += 1;
              // cm_tmp1->dynindex = (INT_S*)REALLOC(cm_tmp1->dynindex,
              // (cm_tmp1->numelts)*sizeof(INT_S));
              // cm_tmp1->dynindex[cm_tmp1->numelts-1] = tmpu_point;
              if (cm_tmp2->flag == 1) {
                if (cm_tmp1->minindex > tmpu_point) {
                  cm_tmp1->minindex = tmpu_point;
                }
              }
              continue;
            }

            // if(root_node[tmp_state_1].marked_in_plant==root_node[tmp_state_2].marked_in_plant
            // &&
            // root_node[tmp_state_1].marked_in_controller!=root_node[tmp_state_2].marked_in_controller)
            //        return 10;

            for (k = 1; k < 3; k++) {
              if (k == 1) {
                temp1 = root_node[tmp_state_1].forb_set;
                temp2 = root_node[tmp_state_2].tran_set;
              }
              if (k == 2) {
                temp1 = root_node[tmp_state_2].forb_set;
                temp2 = root_node[tmp_state_1].tran_set;
              }
              temp22 = temp2;
              while (temp1 != NULL) {
                temp2 = temp22;
                while (temp2 != NULL) {
                  if (temp1->event == temp2->event) {
                    cm_tmp2->flag = 3;
                    return 10;
                  }
                  temp2 = temp2->next_transition;
                }
                temp1 = temp1->next_event;
              }
            }
            if (c_flag != 8) {
              *(tmpu_stack + 2 * tmpu_point) = tmp_state_1;
              *(tmpu_stack + 2 * tmpu_point + 1) = tmp_state_2;
              *(tmpu_stack + 2 * tmpu_point + 2) = -1;

              // update indices of cur_1 and cur_2

              cm_tmp2->flag = 1;
              cm_tmp2->index = tmpu_point;
              cm_tmp2->minindex = tmpu_point;

              // update indices of cur_1 and cur_2
              // cm_tmp1->numelts += 1;
              // cm_tmp1->dynindex = (INT_S*)REALLOC(cm_tmp1->dynindex,
              // (cm_tmp1->numelts)*sizeof(INT_S));
              // cm_tmp1->dynindex[cm_tmp1->numelts-1] = tmpu_point;
              if (cm_tmp1->minindex > tmpu_point) {
                cm_tmp1->minindex = tmpu_point;
              }

              // zprints("Add to wait list:  "); zprintn(tmp_state_1);
              // zprints("-"); zprintn(tmp_state_2); zprints("\n");
            }
          }
          if (c_flag == 0) {
            struct transitions *temp3, *temp4;
            INT_S tmp_state_3, tmp_state_4;
            temp3 = root_node[tmp_state_1].tran_set;
            while (temp3 != NULL) {
              temp4 = root_node[tmp_state_2].tran_set;
              while (temp4 != NULL) {
                if (temp3->event == temp4->event) {
                  flag = 0;
                  tmp_state_3 = temp3->target_state_number;
                  tmp_state_4 = temp4->target_state_number;
                  // if(indexx == 1)
                  //{
                  //	if(merging_table[tmp_state_3][tmp_state_4] == 0)
                  //	{
                  //		temp4 = temp4->next_transition;
                  //		continue;
                  //	}
                  // if(merging_table[tmp_state_3][tmp_state_4] == 1 &&
                  // tmp_state_3 != tmp_state_4) return 10;
                  //}

                  if ((root_node[tmp_state_3].equal_set)->state_number !=
                      (root_node[tmp_state_4].equal_set)->state_number) {

                    if (tmp_state_3 < tmp_state_4) {
                      // tmp_cur_3 = tmp_state_3;
                      // tmp_cur_4 = tmp_state_4 - 1 - tmp_cur_3;
                      cm_tmp3 = &(track_table[tmp_state_3]
                                      .next[tmp_state_4 - 1 - tmp_state_3]);
                    } else {
                      // tmp_cur_3 = tmp_state_4;
                      // tmp_cur_4 = tmp_state_3 - 1 - tmp_cur_3;
                      cm_tmp3 = &(track_table[tmp_state_4]
                                      .next[tmp_state_3 - 1 - tmp_state_4]);
                    }

                    if (cm_tmp3->flag == 3) {
                      return 10;
                    }

                    /*****************************************************************/

                    if ((root_node[tmp_state_3].equal_set)->state_number <
                            nodel ||
                        (root_node[tmp_state_4].equal_set)->state_number <
                            nodel)
                      return 10;
                    /*****************************************************************/

                    tmpu_point = 0;
                    while (*(tmpu_stack + 2 * tmpu_point) != -1) {
                      if (*(tmpu_stack + 2 * tmpu_point) == tmp_state_3 &&
                          *(tmpu_stack + 2 * tmpu_point + 1) == tmp_state_4) {
                        flag = 8;
                        break;
                      }
                      if (*(tmpu_stack + 2 * tmpu_point + 1) == tmp_state_3 &&
                          *(tmpu_stack + 2 * tmpu_point) == tmp_state_4) {
                        flag = 8;
                        break;
                      }
                      tmpu_point += 1;
                    }
                    if (flag == 8) {
                      // cm_tmp2->numelts += 1;
                      // cm_tmp2->dynindex = (INT_S*)REALLOC(cm_tmp2->dynindex,
                      // (cm_tmp2->numelts)*sizeof(INT_S));
                      // cm_tmp2->dynindex[cm_tmp2->numelts-1] = tmpu_point;

                      if (cm_tmp3->flag == 1) {
                        if (cm_tmp2->minindex > tmpu_point) {
                          cm_tmp2->minindex = tmpu_point;
                        }
                        if (cm_tmp1->minindex > cm_tmp2->minindex)
                          cm_tmp1->minindex = cm_tmp2->minindex;
                      }

                      temp4 = temp4->next_transition;
                      continue;
                    }

                    // if(root_node[tmp_state_3].marked_in_plant==root_node[tmp_state_4].marked_in_plant
                    // &&
                    // root_node[tmp_state_3].marked_in_controller!=root_node[tmp_state_4].marked_in_controller)
                    //     return 10;

                    for (k = 1; k < 3; k++) {
                      if (k == 1) {
                        temp1 = root_node[tmp_state_3].forb_set;
                        temp2 = root_node[tmp_state_4].tran_set;
                      }
                      if (k == 2) {
                        temp1 = root_node[tmp_state_4].forb_set;
                        temp2 = root_node[tmp_state_3].tran_set;
                      }
                      temp22 = temp2;
                      while (temp1 != NULL) {
                        temp2 = temp22;
                        while (temp2 != NULL) {
                          if (temp1->event == temp2->event) {
                            cm_tmp3->flag = 3;
                            return 10;
                          }
                          temp2 = temp2->next_transition;
                        }
                        temp1 = temp1->next_event;
                      }
                    }
                    if (flag == 0) {
                      *(tmpu_stack + 2 * tmpu_point) = tmp_state_3;
                      *(tmpu_stack + 2 * tmpu_point + 1) = tmp_state_4;
                      *(tmpu_stack + 2 * tmpu_point + 2) = -1;

                      cm_tmp3->flag = 1;
                      cm_tmp3->index = tmpu_point;
                      cm_tmp3->minindex = tmpu_point;

                      // cm_tmp2->numelts += 1;
                      // cm_tmp2->dynindex = (INT_S*)REALLOC(cm_tmp2->dynindex,
                      // (cm_tmp2->numelts)*sizeof(INT_S));
                      // cm_tmp2->dynindex[cm_tmp2->numelts-1] = tmpu_point;

                      // zprints("Add to wait list:  "); zprintn(tmp_state_3);
                      // zprints("-"); zprintn(tmp_state_4); zprints("\n");
                      flag = Selfloop_Node51(tmp_state_3, tmp_state_4, nodel,
                                             indexx);

                      if (flag == 10) {
                        // if(indexx == 1)
                        //{
                        //	merging_table[tmp_state_3][tmp_state_4] = 1;
                        //	merging_table[tmp_state_4][tmp_state_3] = 1;
                        //}
                        return 10;
                      }

                      if (cm_tmp3->flag == 1) {
                        if (cm_tmp2->minindex > cm_tmp3->minindex) {
                          cm_tmp2->minindex = cm_tmp3->minindex;
                        }
                        if (cm_tmp1->minindex > cm_tmp2->minindex)
                          cm_tmp1->minindex = cm_tmp2->minindex;
                      }
                    }
                  }
                }
                temp4 = temp4->next_transition;
              }
              temp3 = temp3->next_transition;
            }
          }
        }
      }
    }
  }
  if (cm_tmp1->minindex == cm_tmp1->index) {
    drawn_check_mark1(cm_tmp1->index);
  }
  return 9;
}

// Original version
/*This module is part of the main module Reduction.*/
INT_OS Selfloop_Node(INT_S base_state, INT_S tmp_state, INT_S nodel,
                     INT_OS indexx) {
  INT_S node_1, node_2, c_flag, flag, r_flag_1, r_flag_2;
  INT_S tmpu_point, tmpu_point_1, tmpu_point_2, k;
  struct forbidden_event_set *temp1;
  struct transitions *temp2, *temp22;
  struct equivalent_state_set *temp6, *temp7;
  INT_S tmp_state_1, tmp_state_2;
  INT_S *stack_test;

  stack_test = NULL;

  node_1 = base_state;
  node_2 = tmp_state;
  temp1 = 0;
  temp2 = 0;

  temp6 = root_node[root_node[node_1].equal_set->state_number].equal_set;
  r_flag_1 = 0;
  tmpu_point_1 = 0;

  while (r_flag_1 == 0) {
    if (indexx == 0) {
      if (temp6 != NULL) {
        tmp_state_1 = temp6->state_number;
        temp6 = temp6->next_node;
      } else {
        tmp_state_1 = -1;
        while (*(tmpu_stack + 2 * tmpu_point_1) != -1) {
          if (*(tmpu_stack + 2 * tmpu_point_1) == node_1) {
            tmp_state_1 = *(tmpu_stack + 2 * tmpu_point_1 + 1);
            tmpu_point_1 += 1;
            break;
          }
          if (*(tmpu_stack + 2 * tmpu_point_1 + 1) == node_1) {
            tmp_state_1 = *(tmpu_stack + 2 * tmpu_point_1);
            tmpu_point_1 += 1;
            break;
          }
          tmpu_point_1 += 1;
        }
        if (tmp_state_1 == -1)
          r_flag_1 = 1;
        ;
      }
    } else {
      r_flag_1 = 1;
      tmp_state_1 = node_1;
    }

    if (tmp_state_1 != -1) {
      temp7 = root_node[root_node[node_2].equal_set->state_number].equal_set;
      r_flag_2 = 0;
      tmpu_point_2 = 0;
      while (r_flag_2 == 0) {
        if (indexx == 0) {
          if (temp7 != NULL) {
            tmp_state_2 = temp7->state_number;
            temp7 = temp7->next_node;
          } else {
            tmp_state_2 = -1;
            while (*(tmpu_stack + 2 * tmpu_point_2) != -1) {
              if (*(tmpu_stack + 2 * tmpu_point_2) == node_2) {
                tmp_state_2 = *(tmpu_stack + 2 * tmpu_point_2 + 1);
                tmpu_point_2 += 1;
                break;
              }
              if (*(tmpu_stack + 2 * tmpu_point_2 + 1) == node_2) {
                tmp_state_2 = *(tmpu_stack + 2 * tmpu_point_2);
                tmpu_point_2 += 1;
                break;
              }
              tmpu_point_2 += 1;
            }
            if (tmp_state_2 == -1)
              r_flag_2 = 1;
            ;
          }
        } else {
          r_flag_2 = 1;
          tmp_state_2 = node_2;
        }

        if (tmp_state_2 != -1) {
          // zprints("sub check merge: "); zprintn(tmp_state_1); zprints("-");
          // zprintn(tmp_state_2); zprints("\n");
          c_flag = 0;
          if ((tmp_state_1 != node_1 || tmp_state_2 != node_2) &&
              (tmp_state_1 != node_2 || tmp_state_2 != node_1)) {
            if (indexx == 1) {
              if (merging_table[tmp_state_1][tmp_state_2] == 0)
                continue;
              if (merging_table[tmp_state_1][tmp_state_2] == 1 &&
                  tmp_state_1 != tmp_state_2)
                return 10;
            }
            if (root_node[tmp_state_1].equal_set->state_number ==
                root_node[tmp_state_2].equal_set->state_number)
              continue;

            tmpu_point = 0;
            while (*(tmpu_stack + 2 * tmpu_point) != -1) {
              if (*(tmpu_stack + 2 * tmpu_point) == tmp_state_1 &&
                  *(tmpu_stack + 2 * tmpu_point + 1) == tmp_state_2) {
                c_flag = 8;
                break;
              }
              if (*(tmpu_stack + 2 * tmpu_point + 1) == tmp_state_1 &&
                  *(tmpu_stack + 2 * tmpu_point) == tmp_state_2) {
                c_flag = 8;
                break;
              }
              tmpu_point += 1;
            }
            if (c_flag == 8)
              continue;

            if (root_node[tmp_state_1].marked_in_plant ==
                    root_node[tmp_state_2].marked_in_plant &&
                root_node[tmp_state_1].marked_in_controller !=
                    root_node[tmp_state_2].marked_in_controller)
              return 10;

            for (k = 1; k < 3; k++) {
              if (k == 1) {
                temp1 = root_node[tmp_state_1].forb_set;
                temp2 = root_node[tmp_state_2].tran_set;
              }
              if (k == 2) {
                temp1 = root_node[tmp_state_2].forb_set;
                temp2 = root_node[tmp_state_1].tran_set;
              }
              temp22 = temp2;
              while (temp1 != NULL) {
                temp2 = temp22;
                while (temp2 != NULL) {
                  if (temp1->event == temp2->event)
                    return 10;
                  temp2 = temp2->next_transition;
                }
                temp1 = temp1->next_event;
              }
            }
            if (c_flag != 8) {
              *(tmpu_stack + 2 * tmpu_point) = tmp_state_1;
              *(tmpu_stack + 2 * tmpu_point + 1) = tmp_state_2;
              *(tmpu_stack + 2 * tmpu_point + 2) = -1;
            }
          }
          if (c_flag == 0) {
            struct transitions *temp3, *temp4;
            INT_S tmp_state_3, tmp_state_4;
            temp3 = root_node[tmp_state_1].tran_set;
            while (temp3 != NULL) {
              temp4 = root_node[tmp_state_2].tran_set;
              while (temp4 != NULL) {
                if (temp3->event == temp4->event) {
                  flag = 0;
                  tmp_state_3 = temp3->target_state_number;
                  tmp_state_4 = temp4->target_state_number;
                  if (indexx == 1) {
                    if (merging_table[tmp_state_3][tmp_state_4] == 0) {
                      temp4 = temp4->next_transition;
                      continue;
                    }
                    if (merging_table[tmp_state_3][tmp_state_4] == 1 &&
                        tmp_state_3 != tmp_state_4)
                      return 10;
                  }
                  if ((root_node[tmp_state_3].equal_set)->state_number !=
                      (root_node[tmp_state_4].equal_set)->state_number) {

                    /*****************************************************************/

                    if ((root_node[tmp_state_3].equal_set)->state_number <
                            nodel ||
                        (root_node[tmp_state_4].equal_set)->state_number <
                            nodel)
                      return 10;
                    /*****************************************************************/

                    tmpu_point = 0;
                    while (*(tmpu_stack + 2 * tmpu_point) != -1) {
                      if (*(tmpu_stack + 2 * tmpu_point) == tmp_state_3 &&
                          *(tmpu_stack + 2 * tmpu_point + 1) == tmp_state_4) {
                        flag = 8;
                        break;
                      }
                      if (*(tmpu_stack + 2 * tmpu_point + 1) == tmp_state_3 &&
                          *(tmpu_stack + 2 * tmpu_point) == tmp_state_4) {
                        flag = 8;
                        break;
                      }
                      tmpu_point += 1;
                    }
                    if (flag == 8) {
                      temp4 = temp4->next_transition;
                      continue;
                    }

                    if (root_node[tmp_state_3].marked_in_plant ==
                            root_node[tmp_state_4].marked_in_plant &&
                        root_node[tmp_state_3].marked_in_controller !=
                            root_node[tmp_state_4].marked_in_controller)
                      return 10;

                    for (k = 1; k < 3; k++) {
                      if (k == 1) {
                        temp1 = root_node[tmp_state_3].forb_set;
                        temp2 = root_node[tmp_state_4].tran_set;
                      }
                      if (k == 2) {
                        temp1 = root_node[tmp_state_4].forb_set;
                        temp2 = root_node[tmp_state_3].tran_set;
                      }
                      temp22 = temp2;
                      while (temp1 != NULL) {
                        temp2 = temp22;
                        while (temp2 != NULL) {
                          if (temp1->event == temp2->event)
                            return 10;
                          temp2 = temp2->next_transition;
                        }
                        temp1 = temp1->next_event;
                      }
                    }
                    if (flag == 0) {
                      *(tmpu_stack + 2 * tmpu_point) = tmp_state_3;
                      *(tmpu_stack + 2 * tmpu_point + 1) = tmp_state_4;
                      *(tmpu_stack + 2 * tmpu_point + 2) = -1;
                      stack_test = (INT_S *)CALLOC(100, sizeof(INT_S));
                      if (stack_test == NULL) {
                        stack_flag = 100;
                        return 100;
                      } else {
                        free(stack_test);
                        stack_test = NULL;
                      }
                      flag = Selfloop_Node(tmp_state_3, tmp_state_4, nodel,
                                           indexx);
                      if (stack_flag == 100)
                        return 100;
                      if (flag == 10) {
                        if (indexx == 1) {
                          merging_table[tmp_state_3][tmp_state_4] = 1;
                          merging_table[tmp_state_4][tmp_state_3] = 1;
                        }
                        return 10;
                      }
                    }
                  }
                }
                temp4 = temp4->next_transition;
              }
              temp3 = temp3->next_transition;
            }
          }
        }
      }
    }
  }
  return 9;
}

/*This module is used to change data from tree form into transition table
 * list.*/
void Final_Result() {
  INT_S s_base_point = 0;
  INT_S exit_node, transition, target_node, trace_node, tmp_node, tmp_value;
  INT_S i;

  ///////////////////////////////////////////////////
  // Print the final congruence
  /*INT_S s_par; part_node *par;
  INT_S s_list, *list,j;
  INT_B ok;

  s_par = 0; par = NULL;
  s_list = 0; list = NULL;

  s_list = num_states;
  list = (INT_S*)CALLOC(s_list, sizeof(INT_S));

  for(i = 0; i < num_states; i ++){
          list[i] = -1;
          exit_node = (root_node[i].equal_set)->state_number;
          if(exit_node == i){
                  s_par ++;
                  par = (part_node *)REALLOC(par, s_par * sizeof(part_node));
                  j = s_par - 1;
                  par[j].numelts = 1;
                  par[j].next = (INT_S*)CALLOC(1, sizeof(INT_S));
                  par[j].next[0] = i;
                  list[i] = j;
          }else{
                  j = list[exit_node];
                  addstatelist(i,&par[j].next, par[j].numelts,&ok);
                  if(ok) par[j].numelts ++;
          }
  }
  zprint_par(s_par, par);

  free_part(s_par, &par);
  free(list);*/
  /////////////////////////////////////////////////////////

  for (i = 0; i < num_states; i++) {
    struct transitions *temp1;
    temp1 = root_node[i].tran_set;
    while (temp1 != NULL) {
      exit_node = (root_node[i].equal_set)->state_number;
      transition = (INT_S)temp1->event;
      target_node =
          (root_node[temp1->target_state_number].equal_set)->state_number;
      *(simpler_controller + 3 * s_base_point) = exit_node;
      *(simpler_controller + 3 * s_base_point + 1) = transition;
      *(simpler_controller + 3 * s_base_point + 2) = target_node;
      s_base_point += 1;
      temp1 = temp1->next_transition;
    }
  }
  *(simpler_controller + 3 * s_base_point) = -1;
  *c_marked_states = -1;
  for (i = 0; i < num_states; i++) {
    if (root_node[i].marked_in_controller == true) {
      trace_node = 0;
      while (*(c_marked_states + trace_node) != -1) {
        if (*(c_marked_states + trace_node) ==
            (root_node[i].equal_set)->state_number)
          break;
        trace_node += 1;
      }
      if (*(c_marked_states + trace_node) == -1) {
        *(c_marked_states + trace_node) =
            (root_node[i].equal_set)->state_number;
        *(c_marked_states + trace_node + 1) = -1;
      }
    }
  }
  trace_node = 0;
  while (*(c_marked_states + trace_node) != -1) {
    tmp_node = trace_node + 1;
    tmp_value = trace_node;
    while (*(c_marked_states + tmp_node) != -1) {
      if (*(c_marked_states + tmp_node) < *(c_marked_states + tmp_value))
        tmp_value = tmp_node;
      tmp_node += 1;
    }
    tmp_node = *(c_marked_states + trace_node);
    *(c_marked_states + trace_node) = *(c_marked_states + tmp_value);
    *(c_marked_states + tmp_value) = tmp_node;
    trace_node += 1;
  }
}

// Modefied by ZRY to implement Localization algorithm
/*This module is used to reduce the transition structure of optimal
 * controller.*/
void Reduction1() {
  struct forbidden_event_set *temp1;
  struct transitions *temp2, *temp22;
  INT_S temp3, temp4, temp_state;
  struct equivalent_state_set **temp5, *temp6;
  INT_S tmpu_point;
  INT_OS flag;
  INT_S i, j;
  INT_S state_1, state_2;
  *tmpu_stack = -1;

  //  if (temp5) {} /* Remove compiler warning */
  // if (temp6) {} /* Remove compiler warning */

  for (i = 0; i < num_states; i++) {
    if ((root_node[i].equal_set)->state_number == i) {
      for (j = i + 1; j < num_states; j++) {
        if ((root_node[j].equal_set)->state_number == j) {
          flag = 0;
          // if(flag!=10)
          //{
          //   if(root_node[i].marked_in_plant==root_node[j].marked_in_plant &&
          //   root_node[i].marked_in_controller!=root_node[j].marked_in_controller)
          //       flag = 10;
          //}
          temp1 = root_node[i].forb_set;
          temp2 = root_node[j].tran_set;
          if (flag != 10) {
            temp22 = temp2;
            while (temp1 != NULL) {
              temp2 = temp22;
              while (temp2 != NULL) {
                if (temp1->event == temp2->event) {
                  flag = 10;
                  break;
                }
                temp2 = temp2->next_transition;
              }
              if (flag == 10)
                break;
              temp1 = temp1->next_event;
            }
            temp1 = root_node[j].forb_set;
            temp2 = root_node[i].tran_set;
            temp22 = temp2;
            while (temp1 != NULL) {
              temp2 = temp22;
              while (temp2 != NULL) {
                if (temp1->event == temp2->event) {
                  flag = 10;
                  break;
                }
                temp2 = temp2->next_transition;
              }
              if (flag == 10)
                break;
              temp1 = temp1->next_event;
            }
          }
          if (flag != 10) {
            *tmpu_stack = i;
            *(tmpu_stack + 1) = j;
            *(tmpu_stack + 2) = -1;
            // zprints("Add to wait list:  "); zprintn(i); zprints("-");
            // zprintn(j); zprints("\n");
            flag = Selfloop_Node1(i, j, i, 0);
          }
          if (flag == 10) {
            *tmpu_stack = -1;
          }
          if (flag != 10) {
            tmpu_point = 0;
            while (*(tmpu_stack + 2 * tmpu_point) != -1) {
              state_1 = *(tmpu_stack + 2 * tmpu_point);
              state_2 = *(tmpu_stack + 2 * tmpu_point + 1);
              temp3 = root_node[state_1].equal_set->state_number;
              temp4 = root_node[state_2].equal_set->state_number;
              if (temp3 < temp4) {
                temp5 = &(root_node[temp3].equal_set); /*acceptor*/
                temp6 = root_node[temp4].equal_set;    /*merger*/
              } else if (temp3 > temp4) {
                temp5 = &(root_node[temp4].equal_set); /*acceptor*/
                temp6 = root_node[temp3].equal_set;    /*merger*/
              }
              temp_state = (*temp5)->state_number;
              if (temp3 == temp4) {
                tmpu_point += 1;
                continue;
              }
              while (*temp5 != NULL)
                temp5 = &((*temp5)->next_node);
              while (temp6 != NULL) {
                *temp5 = (struct equivalent_state_set *)MALLOC(
                    sizeof(struct equivalent_state_set));
                (*temp5)->state_number = temp6->state_number;
                root_node[temp6->state_number].equal_set->state_number =
                    temp_state;
                temp6 = temp6->next_node;
                if (temp6 != NULL)
                  temp5 = &((*temp5)->next_node);
                else
                  (*temp5)->next_node = NULL;
              }
              tmpu_point += 1;
            }
            *tmpu_stack = -1;
          }
        }
      }
    }
  }
  simpler_controller = (INT_S *)MALLOC((3 * tran_number + 1) * sizeof(INT_S));
  Final_Result();
}

void Reduction2() {
  struct forbidden_event_set *temp1, *temp11, *temp12;
  //  struct transitions *temp2;//, *temp22;
  INT_S temp3, temp4, temp_state;
  struct equivalent_state_set **temp5, *temp6;
  INT_S tmpu_point;
  INT_S flag;
  INT_S i, j, k;
  INT_S state_1, state_2;
  INT_B cc_flag;
  *tmpu_stack = -1;

  //  if (temp5) {} /* Remove compiler warning */
  //  if (temp6) {} /* Remove compiler warning */

  for (i = 0; i < num_states; i++) {
    if ((root_node[i].equal_set)->state_number == i) {
      for (j = i + 1; j < num_states; j++) {
        if ((root_node[j].equal_set)->state_number == j) {
          flag = 0;
          // if(flag!=10)
          //{
          //   if(root_node[i].marked_in_plant==root_node[j].marked_in_plant &&
          //   root_node[i].marked_in_controller!=root_node[j].marked_in_controller)
          //       flag = 10;
          //}
          temp1 = root_node[i].forb_set;
          temp11 = root_node[j].forb_set;
          if (flag != 10) {
            if ((temp1 == NULL && temp11 != NULL) ||
                (temp1 != NULL && temp11 == NULL)) {
              flag = 10;
            } else {
              for (k = 1; k < 3; k++) {
                if (k == 2) {
                  temp1 = root_node[j].forb_set;
                  temp11 = root_node[i].forb_set;
                }
                while (temp1 != NULL) {
                  temp12 = temp11;
                  cc_flag = false;
                  while (temp12 != NULL) {
                    if (temp1->event == temp12->event) {
                      cc_flag = true;
                      break;
                    }
                    temp12 = temp12->next_event;
                  }
                  if (!cc_flag) {
                    flag = 10;
                    break;
                  }
                  temp1 = temp1->next_event;
                }
              }
            }
            /*temp22 = temp2;
            while(temp1 != NULL)
            {
            temp2 = temp22;
            while(temp2 != NULL)
            {
            if(temp1->event == temp2->event)
            {
            flag = 10;
            break;
            }
            temp2 = temp2->next_transition;
            }
            if(flag == 10) break;
            temp1 = temp1->next_event;
            }
            temp1 = root_node[j].forb_set;
            temp2 = root_node[i].tran_set;
            temp22 = temp2;
            while(temp1 != NULL)
            {
            temp2 = temp22;
            while(temp2 != NULL)
            {
            if(temp1->event == temp2->event)
            {
            flag = 10;
            break;
            }
            temp2 = temp2->next_transition;
            }
            if(flag == 10) break;
            temp1 = temp1->next_event;
            }*/
          }
          if (flag != 10) {
            *tmpu_stack = i;
            *(tmpu_stack + 1) = j;
            *(tmpu_stack + 2) = -1;
            flag = Selfloop_Node2(i, j, i, 0);
          }
          if (flag == 10) {
            *tmpu_stack = -1;
          }
          if (flag != 10) {
            tmpu_point = 0;
            while (*(tmpu_stack + 2 * tmpu_point) != -1) {
              state_1 = *(tmpu_stack + 2 * tmpu_point);
              state_2 = *(tmpu_stack + 2 * tmpu_point + 1);
              temp3 = root_node[state_1].equal_set->state_number;
              temp4 = root_node[state_2].equal_set->state_number;
              if (temp3 < temp4) {
                temp5 = &(root_node[temp3].equal_set); /*acceptor*/
                temp6 = root_node[temp4].equal_set;    /*merger*/
              } else if (temp3 > temp4) {
                temp5 = &(root_node[temp4].equal_set); /*acceptor*/
                temp6 = root_node[temp3].equal_set;    /*merger*/
              }
              temp_state = (*temp5)->state_number;
              if (temp3 == temp4) {
                tmpu_point += 1;
                continue;
              }
              while (*temp5 != NULL)
                temp5 = &((*temp5)->next_node);
              while (temp6 != NULL) {
                *temp5 = (struct equivalent_state_set *)MALLOC(
                    sizeof(struct equivalent_state_set));
                (*temp5)->state_number = temp6->state_number;
                root_node[temp6->state_number].equal_set->state_number =
                    temp_state;
                temp6 = temp6->next_node;
                if (temp6 != NULL)
                  temp5 = &((*temp5)->next_node);
                else
                  (*temp5)->next_node = NULL;
              }
              tmpu_point += 1;
            }
            *tmpu_stack = -1;
          }
        }
      }
    }
  }
  simpler_controller = (INT_S *)MALLOC((3 * tran_number + 1) * sizeof(INT_S));
  Final_Result();
}

void Reduction3() {
  //  struct forbidden_event_set *temp11;//*temp1,
  //  struct transitions *temp2;//, *temp22;
  INT_S temp3, temp4, temp_state;
  struct equivalent_state_set **temp5, *temp6;
  INT_S tmpu_point;
  INT_OS flag;
  INT_S i, j;
  INT_S state_1, state_2;
  *tmpu_stack = -1;

  //  if (temp5) {} /* Remove compiler warning */
  // if (temp6) {} /* Remove compiler warning */

  for (i = 0; i < num_states; i++) {
    if ((root_node[i].equal_set)->state_number == i) {
      for (j = i + 1; j < num_states; j++) {
        if ((root_node[j].equal_set)->state_number == j) {
          flag = 0;
          if (flag != 10) {
            if (root_node[i].marked_in_controller !=
                root_node[j].marked_in_controller)
              flag = 10;
          }
          /*temp1 = root_node[i].forb_set;
          temp11 = root_node[j].forb_set;
          if(flag != 10)
          {
          if((temp1 == NULL && temp11 != NULL) || (temp1 != NULL && temp11 ==
          NULL)) flag = 10;*/
          /*temp22 = temp2;
          while(temp1 != NULL)
          {
          temp2 = temp22;
          while(temp2 != NULL)
          {
          if(temp1->event == temp2->event)
          {
          flag = 10;
          break;
          }
          temp2 = temp2->next_transition;
          }
          if(flag == 10) break;
          temp1 = temp1->next_event;
          }
          temp1 = root_node[j].forb_set;
          temp2 = root_node[i].tran_set;
          temp22 = temp2;
          while(temp1 != NULL)
          {
          temp2 = temp22;
          while(temp2 != NULL)
          {
          if(temp1->event == temp2->event)
          {
          flag = 10;
          break;
          }
          temp2 = temp2->next_transition;
          }
          if(flag == 10) break;
          temp1 = temp1->next_event;
          }
          }*/
          if (flag != 10) {
            *tmpu_stack = i;
            *(tmpu_stack + 1) = j;
            *(tmpu_stack + 2) = -1;
            flag = Selfloop_Node3(i, j, i, 0);
          }
          if (flag == 10) {
            *tmpu_stack = -1;
          }
          if (flag != 10) {
            tmpu_point = 0;
            while (*(tmpu_stack + 2 * tmpu_point) != -1) {
              state_1 = *(tmpu_stack + 2 * tmpu_point);
              state_2 = *(tmpu_stack + 2 * tmpu_point + 1);
              temp3 = root_node[state_1].equal_set->state_number;
              temp4 = root_node[state_2].equal_set->state_number;
              if (temp3 < temp4) {
                temp5 = &(root_node[temp3].equal_set); /*acceptor*/
                temp6 = root_node[temp4].equal_set;    /*merger*/
              } else if (temp3 > temp4) {
                temp5 = &(root_node[temp4].equal_set); /*acceptor*/
                temp6 = root_node[temp3].equal_set;    /*merger*/
              }
              temp_state = (*temp5)->state_number;
              if (temp3 == temp4) {
                tmpu_point += 1;
                continue;
              }
              while (*temp5 != NULL)
                temp5 = &((*temp5)->next_node);
              while (temp6 != NULL) {
                *temp5 = (struct equivalent_state_set *)MALLOC(
                    sizeof(struct equivalent_state_set));
                (*temp5)->state_number = temp6->state_number;
                root_node[temp6->state_number].equal_set->state_number =
                    temp_state;
                temp6 = temp6->next_node;
                if (temp6 != NULL)
                  temp5 = &((*temp5)->next_node);
                else
                  (*temp5)->next_node = NULL;
              }
              tmpu_point += 1;
            }
            *tmpu_stack = -1;
          }
        }
      }
    }
  }
  simpler_controller = (INT_S *)MALLOC((3 * tran_number + 1) * sizeof(INT_S));
  Final_Result();
}

void Reduction5() {
  struct forbidden_event_set *temp1;
  struct transitions *temp2, *temp22;
  INT_S temp3, temp4, temp_state;
  struct equivalent_state_set **temp5, *temp6;
  INT_S tmpu_point;
  INT_OS flag;
  INT_S i, j;
  INT_S state_1, state_2;
  check_mark *cm_tmp1, *cm_tmp2;

  *tmpu_stack = -1;
  s_tmpu_stack = 0;

  //  if (temp5) {} /* Remove compiler warning */
  // if (temp6) {} /* Remove compiler warning */

  for (i = 0; i < num_states; i++) {
    if ((root_node[i].equal_set)->state_number == i) {
      for (j = i + 1; j < num_states; j++) {
        if ((root_node[j].equal_set)->state_number == j) {
          flag = 0;
          cm_tmp1 = &(track_table[i].next[j - 1 - i]);
          if (cm_tmp1->flag == 3)
            flag = 10;
          temp1 = root_node[i].forb_set;
          temp2 = root_node[j].tran_set;
          if (flag != 10) {
            temp22 = temp2;
            while (temp1 != NULL) {
              temp2 = temp22;
              while (temp2 != NULL) {
                if (temp1->event == temp2->event) {
                  flag = 10;
                  break;
                }
                temp2 = temp2->next_transition;
              }
              if (flag == 10)
                break;
              temp1 = temp1->next_event;
            }
            temp1 = root_node[j].forb_set;
            temp2 = root_node[i].tran_set;
            temp22 = temp2;
            while (temp1 != NULL) {
              temp2 = temp22;
              while (temp2 != NULL) {
                if (temp1->event == temp2->event) {
                  flag = 10;
                  break;
                }
                temp2 = temp2->next_transition;
              }
              if (flag == 10)
                break;
              temp1 = temp1->next_event;
            }
          }
          if (flag != 10) {
            *tmpu_stack = i;
            *(tmpu_stack + 1) = j;
            *(tmpu_stack + 2) = -1;
            cm_tmp1->flag =
                1; // state pair (i, j) has been added to waiting list
            cm_tmp1->index = 0;
            cm_tmp1->minindex = 0;
            // zprints("Add to wait list:  "); zprintn(i); zprints("-");
            // zprintn(j); zprints("\n");
            flag = Selfloop_Node51(i, j, i, 0);
          } else {
            cm_tmp1->flag = 3;
          }

          // if(flag != 10)
          //{
          tmpu_point = 0;
          while (*(tmpu_stack + 2 * tmpu_point) != -1) {
            state_1 = *(tmpu_stack + 2 * tmpu_point);
            state_2 = *(tmpu_stack + 2 * tmpu_point + 1);
            if (state_1 < state_2) {
              cm_tmp2 = &(track_table[state_1].next[state_2 - 1 - state_1]);
            } else {
              cm_tmp2 = &(track_table[state_2].next[state_1 - 1 - state_2]);
            }
            if (cm_tmp2->flag != 2) {
              cm_tmp2->flag = 3;
              // if(track_table[state_1].next[state_2-1-state_1].dynindex !=
              // NULL){
              //	free(track_table[state_1].next[state_2-1-state_1].dynindex);
              //	track_table[state_1].next[state_2-1-state_1].dynindex =
              // NULL;
              //}
              tmpu_point += 1;
              continue;
            }

            temp3 = root_node[state_1].equal_set->state_number;
            temp4 = root_node[state_2].equal_set->state_number;
            if (temp3 < temp4) {
              temp5 = &(root_node[temp3].equal_set); /*acceptor*/
              temp6 = root_node[temp4].equal_set;    /*merger*/
            } else if (temp3 > temp4) {
              temp5 = &(root_node[temp4].equal_set); /*acceptor*/
              temp6 = root_node[temp3].equal_set;    /*merger*/
            }
            temp_state = (*temp5)->state_number;
            if (temp3 == temp4) {
              tmpu_point += 1;
              continue;
            }
            while (*temp5 != NULL)
              temp5 = &((*temp5)->next_node);
            while (temp6 != NULL) {
              *temp5 = (struct equivalent_state_set *)MALLOC(
                  sizeof(struct equivalent_state_set));
              (*temp5)->state_number = temp6->state_number;
              root_node[temp6->state_number].equal_set->state_number =
                  temp_state;
              temp6 = temp6->next_node;
              if (temp6 != NULL)
                temp5 = &((*temp5)->next_node);
              else
                (*temp5)->next_node = NULL;
            }
            tmpu_point += 1;
          }
          *tmpu_stack = -1;
          //}
        }
      }
    }
  }
  simpler_controller = (INT_S *)MALLOC((3 * tran_number + 1) * sizeof(INT_S));
  Final_Result();
}

// Original Version
/*This module is used to reduce the transition structure of optimal
 * controller.*/
void Reduction() {
  struct forbidden_event_set *temp1;
  struct transitions *temp2, *temp22;
  INT_S temp3, temp4, temp_state;
  struct equivalent_state_set **temp5, *temp6;
  INT_S tmpu_point;
  INT_OS flag;
  INT_S i, j;
  INT_S state_1, state_2;

  *tmpu_stack = -1;

  //  if (temp5) {} /* Remove compiler warning */
  // if (temp6) {} /* Remove compiler warning */

  for (i = 0; i < num_states; i++) {
    if ((root_node[i].equal_set)->state_number == i) {
      for (j = i + 1; j < num_states; j++) {
        if ((root_node[j].equal_set)->state_number == j) {
          flag = 0;
          if (flag != 10) {
            if (root_node[i].marked_in_plant == root_node[j].marked_in_plant &&
                root_node[i].marked_in_controller !=
                    root_node[j].marked_in_controller)
              flag = 10;
          }
          temp1 = root_node[i].forb_set;
          temp2 = root_node[j].tran_set;
          if (flag != 10) {
            temp22 = temp2;
            while (temp1 != NULL) {
              temp2 = temp22;
              while (temp2 != NULL) {
                if (temp1->event == temp2->event) {
                  flag = 10;
                  break;
                }
                temp2 = temp2->next_transition;
              }
              if (flag == 10)
                break;
              temp1 = temp1->next_event;
            }
            temp1 = root_node[j].forb_set;
            temp2 = root_node[i].tran_set;
            temp22 = temp2;
            while (temp1 != NULL) {
              temp2 = temp22;
              while (temp2 != NULL) {
                if (temp1->event == temp2->event) {
                  flag = 10;
                  break;
                }
                temp2 = temp2->next_transition;
              }
              if (flag == 10)
                break;
              temp1 = temp1->next_event;
            }
          }
          if (flag != 10) {
            *tmpu_stack = i;
            *(tmpu_stack + 1) = j;
            *(tmpu_stack + 2) = -1;
            flag = Selfloop_Node(i, j, i, 0);
          }
          if (flag == 10) {
            *tmpu_stack = -1;
          } else if (flag ==
                     100) { // used to indicate whether the stack is overflow
            stack_flag = 100;
            return;
          }
          if (flag != 10) {
            tmpu_point = 0;
            while (*(tmpu_stack + 2 * tmpu_point) != -1) {
              state_1 = *(tmpu_stack + 2 * tmpu_point);
              state_2 = *(tmpu_stack + 2 * tmpu_point + 1);
              temp3 = root_node[state_1].equal_set->state_number;
              temp4 = root_node[state_2].equal_set->state_number;
              if (temp3 < temp4) {
                temp5 = &(root_node[temp3].equal_set); /*acceptor*/
                temp6 = root_node[temp4].equal_set;    /*merger*/
              } else if (temp3 > temp4) {
                temp5 = &(root_node[temp4].equal_set); /*acceptor*/
                temp6 = root_node[temp3].equal_set;    /*merger*/
              }
              temp_state = (*temp5)->state_number;
              if (temp3 == temp4) {
                tmpu_point += 1;
                continue;
              }
              while (*temp5 != NULL)
                temp5 = &((*temp5)->next_node);
              while (temp6 != NULL) {
                *temp5 = (struct equivalent_state_set *)MALLOC(
                    sizeof(struct equivalent_state_set));
                (*temp5)->state_number = temp6->state_number;
                root_node[temp6->state_number].equal_set->state_number =
                    temp_state;
                temp6 = temp6->next_node;
                if (temp6 != NULL)
                  temp5 = &((*temp5)->next_node);
                else
                  (*temp5)->next_node = NULL;
              }
              tmpu_point += 1;
            }
            *tmpu_stack = -1;
          }
        }
      }
    }
  }
  simpler_controller = (INT_S *)MALLOC((3 * tran_number + 1) * sizeof(INT_S));
  Final_Result();
}

/*This module is used to add any possible forbidden event at each node of
 * controller_tree.*/
INT_B Forbidden_Event(char *name) {
  // FILE *in;
  // INT_S mark, s, z, init, yy;
  // INT_T num;
  // char ch;
  // long address;
  // INT_S exit_state;
  // tran_node tran;

  // yy = 0;

  // temp variable
  INT_S i, j;

  state_node *t1;
  INT_S size, init;
  getdes(name, &size, &init, &t1);
  num_states = size;

  /* Read the transitions */
  struct forbidden_event_set **temp1;
  for (i = 0; i < size; i++) {
    if (t1[i].numelts > 0) {
      temp1 = &(root_node[i].forb_set);
      for (j = 0; j < t1[i].numelts; j++) {
        *temp1 = (struct forbidden_event_set *)MALLOC(
            sizeof(struct forbidden_event_set));
        if (*temp1 == NULL) {
          mem_result = 1;
        }
        (*temp1)->event = t1[i].next[j].data1; //((unsigned long) yy>>22);
        if (j < t1[i].numelts - 1) {
          temp1 = &((*temp1)->next_event);
        } else {
          (*temp1)->next_event = NULL;
        }
      }
    }
  }
  return 0;
}


/*I use this module to reindex the controller_tree by the index set of plant.*/
/*So that each node of the controller_tree will have two index, one from the */
/*index set of optimal controller, and the other from the index set of plant.*/
INT_OS Set_State_No_of_Plant(INT_S c_st_no_of_con) {
  INT_S base_point = 0, father_base_point = 0, father_relative_point = 0,
        trace_in_plant = 0;
  INT_S *stack_test;

  stack_test = NULL;

  while (*(controller_tree + base_point) != c_st_no_of_con) {
    base_point += 15 + 2 * (*(controller_tree + base_point + 14));
    if (*(controller_tree + base_point) == -1) {
      return 1;
    }
  }
  while (*(controller_tree + father_base_point) !=
         *(controller_tree + base_point + 13)) {
    father_base_point += 15 + 2 * (*(controller_tree + father_base_point + 14));
    if (*(controller_tree + father_base_point) == -1) {
      return 1;
    }
  }
  if (*(controller_tree + father_base_point + 2) == -1) {
    stack_test = (INT_S *)CALLOC(100, sizeof(INT_S));
    if (stack_test == NULL) {
      stack_flag = 100;
      return 100;
    } else {
      free(stack_test);
      stack_test = NULL;
    }
    Set_State_No_of_Plant(*(controller_tree + father_base_point));
    if (stack_flag == 100)
      return 100;
  }
  while (*(controller_tree + father_base_point + 16 +
           2 * father_relative_point) != *(controller_tree + base_point)) {
    father_relative_point += 1;
    if (father_relative_point > *(controller_tree + father_base_point + 14)) {
      return 1;
    }
  }
  while ((*(plant + 3 * trace_in_plant) !=
          *(controller_tree + father_base_point + 2)) ||
         (*(plant + 3 * trace_in_plant + 1) !=
          *(controller_tree + father_base_point + 15 +
            2 * father_relative_point))) {
    trace_in_plant += 1;
    if (*(plant + 3 * trace_in_plant) == -1) {
      return 1;
    }
  }
  *(controller_tree + base_point + 2) = *(plant + 3 * trace_in_plant + 2);
  return 0;
}

/*I use this module to add any possible forbidden event at each node of
 * controller_tree.*/
/*This kind of information is very important.*/
INT_B Combined_Tree() {
  INT_S flag, c_st_no_of_con, base_point = 0;
  *(controller_tree + 2) = 0;
  while (*(controller_tree + base_point) != -1) {
    c_st_no_of_con = *(controller_tree + base_point);
    if (c_st_no_of_con != 0) {
      flag = Set_State_No_of_Plant(c_st_no_of_con);
      if (flag == 1)
        return 1;
    }
    base_point += 15 + 2 * (*(controller_tree + base_point + 14));
  }
  return 0;
}

/*I use this module to create a controller_tree just using the transition
 * table*/
/*obtained by CTCT software. In this tree, each branch has a father node, a son
 */
/*node and a transition index corresponding to the source state, transition
 * and*/
/*the target state in the transition tables.*/
void Controller_Tree() {
  INT_S index, tran_number = 0, trace_father_node = 0, base_point = 0,
               relative_point = 0;
  INT_S stno_in_controller, stno_in_plant = -1, forbidden_event = -1;
  INT_S father_node = -1, no_of_son_nodes = 0, forbidden_index;
  index = *controller;
  while (index != -1) {
    stno_in_controller = *(controller + 3 * tran_number);
    /*the following is to find the father_node.*/
    if (stno_in_controller != 0) {
      while (*(controller + 3 * trace_father_node + 2) != stno_in_controller) {
        trace_father_node += 1;
      }
      trace_father_node = 0;
    }
    /*the following is to find the number of the son_nodes and the
     * corresponding*/
    /*branches which are connected between the current node and its son_nodes.*/
    while (*(controller + 3 * tran_number) == stno_in_controller) {
      *(controller_tree + base_point + relative_point + 15) =
          *(controller + 3 * tran_number + 1);
      *(controller_tree + base_point + relative_point + 16) =
          *(controller + 3 * tran_number + 2);
      relative_point += 2;
      no_of_son_nodes += 1;
      tran_number += 1;
    }
    *(controller_tree + base_point) = stno_in_controller;
    *(controller_tree + base_point + 2) = stno_in_plant;
    for (forbidden_index = 0; forbidden_index < 10; forbidden_index++)
      *(controller_tree + base_point + 3 + forbidden_index) = forbidden_event;
    *(controller_tree + base_point + 13) = father_node;
    *(controller_tree + base_point + 14) = no_of_son_nodes;
    base_point += 15 + 2 * no_of_son_nodes;
    relative_point = 0;
    no_of_son_nodes = 0;
    index = *(controller + 3 * tran_number);
  }
  *(controller_tree + base_point) = -1;

  father_node_in_controller_tree(0);
}

void father_node_in_controller_tree(INT_S base_point) {
  INT_S relative_point, son_base_point, son_node, no_of_son_nodes;
  // INT_S *stack_test;

  // stack_test = NULL;
  /* modification starts here */
  /* assign father-node-number to each node in controller_tree */
  relative_point = 0;
  for (relative_point = 0;
       relative_point < *(controller_tree + base_point + 14);
       relative_point++) {
    son_node = *(controller_tree + base_point + 2 * relative_point + 16);
    son_base_point = 0;
    while (*(controller_tree + son_base_point) != son_node &&
           *(controller_tree + son_base_point) != -1) {
      no_of_son_nodes = *(controller_tree + son_base_point + 14);
      son_base_point += 15 + 2 * no_of_son_nodes;
    }
    if (*(controller_tree + son_base_point) == -1) {
      /*cout << "something wrong 20, program halt"; */
      /*return 1; */
    }
    if (*(controller_tree + son_base_point + 13) == -1 &&
        *(controller_tree + son_base_point) != 0) {
      *(controller_tree + son_base_point + 13) =
          *(controller_tree + base_point);
      // stack_test = (INT_S *)CALLOC(100, sizeof(INT_S));
      // if(stack_test == NULL){
      //	stack_flag = 100;
      //	return;
      //}else{
      //	free(stack_test);
      //	stack_test = NULL;
      //}
      father_node_in_controller_tree(son_base_point);
      // if(stack_flag == 100)
      //	return;
    }
  }
  /* end of modification */
}

void Tree_Structure_Conversion(char *name) {
  INT_S base_point = 0;
  INT_S p_mark, c_mark;
  INT_S plant_number;
  INT_S i;
  INT_S relative_point;
  struct transitions **temp_1;
  for (i = 0; i < num_states; i++) {
    p_mark = 0;
    c_mark = 0;
    root_node[i].equal_set = (struct equivalent_state_set *)MALLOC(
        sizeof(struct equivalent_state_set));
    if (root_node[i].equal_set == NULL) {
      mem_result = 1;
    }
    (root_node[i].equal_set)->state_number = *(controller_tree + base_point);
    (root_node[i].equal_set)->next_node = NULL;
    plant_number = *(controller_tree + base_point + 2);
    root_node[i].marked_in_plant = false;
    root_node[i].marked_in_controller = false;
    while (*(p_marked_states + p_mark) != -1) {
      if (*(p_marked_states + p_mark) == plant_number) {
        root_node[i].marked_in_plant = true;
        break;
      }
      p_mark += 1;
    }
    while (*(c_marked_states + c_mark) != -1) {
      if (*(c_marked_states + c_mark) == *(controller_tree + base_point)) {
        root_node[i].marked_in_controller = true;
        break;
      }
      c_mark += 1;
    }
    root_node[i].forb_set = NULL;
    root_node[i].tran_set = NULL;
    temp_1 = &(root_node[i].tran_set);
    for (relative_point = 0;
         relative_point < *(controller_tree + base_point + 14);
         relative_point++) {
      *temp_1 = (struct transitions *)MALLOC(sizeof(struct transitions));
      if (*temp_1 == NULL) {
        mem_result = 1;
      }
      (**temp_1).event =
          (INT_T)(*(controller_tree + base_point + 15 + 2 * relative_point));
      (**temp_1).target_state_number =
          *(controller_tree + base_point + 15 + 2 * relative_point + 1);
      if (relative_point < *(controller_tree + base_point + 14) - 1)
        temp_1 = &((**temp_1).next_transition);
      else
        (**temp_1).next_transition = NULL;
    }
    base_point += 15 + 2 * (*(controller_tree + base_point + 14));
  }
  Forbidden_Event(name);
}

/*I use this module to convert the .txt file into .des file so that CTCT
 * software*/
/*can use this simplified controller directly.*/
INT_OS Txt_DES(FILE *out, INT_S number_of_states) {
  INT_S init = 0;

  INT_S mark_state = 0;
  INT_S marked_idx;

  INT_S i, j;
  INT_S base_point = 0;
  INT_S trace_point, counter;
  INT_B ok;
  INT_T event;
  INT_S next;

  state_node *t1;
  t1 = newdes(number_of_states);
  if (t1 == NULL) {
    mem_result = 1;
    return 0;
  }

  while (*(c_marked_states + mark_state) != -1) {
    marked_idx = *(c_marked_states + mark_state);
    t1[marked_idx].marked = true;
    mark_state += 1;
  }

  for (i = 0; i < number_of_states; i++) {
    trace_point = base_point;
    counter = 0;
    while (*(simpler_controller + 3 * base_point) == i) {
      if (*(simpler_controller + 3 * base_point + 1) >= 1000) {
        counter += 1;
      }
      base_point += 1;
    }
    tran_number = (INT_T)(base_point - trace_point - counter);
    for (j = 0; j < tran_number; j++) {
      if (simpler_controller[3 * trace_point + 1] < 1000) {
        event = (INT_T)simpler_controller[3 * trace_point + 1];
        next = simpler_controller[3 * trace_point + 2];

        addordlist1(event, next, &t1[i].next, t1[i].numelts, &ok);
        if (ok) {
          t1[i].numelts++;
        }
      }
      trace_point += 1;
    }
  }

  filedes(out, number_of_states, init, t1);

  if (mem_result == 1) {
    return -1;
  }
  freedes(number_of_states, &t1);
  return 1;
}


// Modefied by ZRY to implement Localization algorithm
INT_S Refinement1() {
  INT_S i, j, k, flag, flag_1, ref_char_control_pattern;
  INT_S l;
  INT_S **filtering, counter, max_num, max_state;
  struct forbidden_event_set *temp1;
  struct transitions *temp2, *temp22;
  struct virtual_stack *current_stack, *next_stack;

  //   if (temp1) {} /* Remove warning */
  //  if (temp2) {} /* Remove warning */

  stack = NULL;
  *tmpu_stack = -1;
  ref_char_control_pattern = 0;
  for (i = 0; i < num_states; i++) {
    root_node[i].equal_set->state_number = i;
    root_node[i].equal_set->next_node = NULL;
  }
  merging_table = (char **)MALLOC(num_states * sizeof(char *));
  if (merging_table == NULL) {
    mem_result = 1;
  }

  for (j = 0; j < num_states; j++) {
    merging_table[j] = (char *)MALLOC(num_states * sizeof(char));
    if (merging_table[j] == NULL) {
      mem_result = 1;
    }
    for (k = 0; k < num_states; k++)
      merging_table[j][k] = -1; /*initialize merging_table*/
  }

  for (j = 0; j < num_states - 1; j++) {
    for (k = j + 1; k < num_states; k++) {
      flag = 0;
      if (merging_table[j][k] != -1)
        continue;
      // if((root_node[j].marked_in_plant==root_node[k].marked_in_plant) &&
      // (root_node[j].marked_in_controller!=root_node[k].marked_in_controller))
      //   flag = 10;

      for (l = 1; l < 3; l++) {
        if (l == 1) {
          temp1 = root_node[j].forb_set;
          temp2 = root_node[k].tran_set;
        }
        if (l == 2) {
          temp1 = root_node[k].forb_set;
          temp2 = root_node[j].tran_set;
        }
        temp22 = temp2;
        while (temp1 != NULL) {
          temp2 = temp22;
          while (temp2 != NULL) {
            if (temp1->event == temp2->event) {
              flag = 10;
              break;
            }
            temp2 = temp2->next_transition;
          }
          if (flag == 10)
            break;
          temp1 = temp1->next_event;
        }
      }

      if (flag == 0) {
        tmpu_stack[0] = j;
        tmpu_stack[1] = k;
        tmpu_stack[2] = -1;

        /* modification is here ........*/
        stack = (struct virtual_stack *)MALLOC(sizeof(struct virtual_stack));
        stack->state_1 = j;
        stack->state_2 = k;
        stack->last_state_pair = NULL;
        stack->next_state_pair = NULL;
        /* end of modification .........*/

        flag = Selfloop_Node1(j, k, -1, 1);
        if (flag != 10) {
          l = 0;
          while (tmpu_stack[2 * l] != -1) {
            merging_table[tmpu_stack[2 * l]][tmpu_stack[2 * l + 1]] = 0;
            merging_table[tmpu_stack[2 * l + 1]][tmpu_stack[2 * l]] = 0;
            l += 1;
          }
          tmpu_stack[0] = -1;

          /*clear memory */
          current_stack = stack;
          while (current_stack != NULL) {
            next_stack = current_stack->next_state_pair;
            free(current_stack);
            current_stack = next_stack;
          }

          stack = NULL;
          /*end of clearing memory */

        } else {
          /*clear memory */
          current_stack = stack;
          while (current_stack != NULL) {
            next_stack = current_stack->next_state_pair;
            merging_table[current_stack->state_1][current_stack->state_2] = 1;
            merging_table[current_stack->state_2][current_stack->state_1] = 1;
            free(current_stack);
            current_stack = next_stack;
          }

          stack = NULL;
          /*end of clearing memory */
        }
      }
    }
  }
  /*Begin the
   * refinement........................................................*/
  filtering = (INT_S **)MALLOC(num_states * sizeof(INT_S *));
  for (j = 0; j < num_states; j++)
    filtering[j] = (INT_S *)MALLOC(2 * sizeof(INT_S));
  for (j = 0; j < num_states; j++) {
    counter = 0;
    for (k = 0; k < num_states; k++)
      if (merging_table[j][k] == 0)
        counter += 1;
    filtering[j][0] = j;
    filtering[j][1] = counter;
  }
  flag_1 = 0;
  while (flag_1 == 0) {
    for (j = 0; j < num_states; j++) {
      if (filtering[j][0] != -1)
        break;
    }
    max_num = filtering[j][1];
    max_state = j;
    for (k = j + 1; k < num_states; k++) {
      if (filtering[k][0] != -1 && filtering[k][1] > max_num) {
        max_num = filtering[k][1];
        max_state = k;
      }
    }
    if (max_num == 0)
      break;
    for (k = 0; k < num_states; k++) {
      if (merging_table[max_state][k] == 0) {
        if (filtering[k][0] != -1)
          filtering[k][1] -= 1;
      }
    }
    filtering[max_state][0] = -1;
  }

  counter = 0;
  for (k = 0; k < num_states; k++)
    if (filtering[k][0] != -1)
      counter += 1;
  ref_char_control_pattern += counter;

  for (j = 0; j < num_states; j++)
    free(filtering[j]);
  free(filtering);
  filtering = NULL;
  free(tmpu_stack);
  tmpu_stack = NULL;

  /* free the memory of merging_table */
  for (k = 0; k < num_states; k++)
    free(merging_table[k]);
  free(merging_table);

  return ref_char_control_pattern;
}

INT_S Refinement2() {
  INT_S i, j, k, flag, flag_1, ref_char_control_pattern;
  INT_S l;
  INT_S **filtering, counter, max_num, max_state;
  struct forbidden_event_set *temp1, *temp11, *temp12;
  //   struct transitions *temp2;//, *temp22;
  struct virtual_stack *current_stack, *next_stack;
  INT_B cc_flag;

  //   if (temp1) {} /* Remove warning */
  //  if (temp2) {} /* Remove warning */

  stack = NULL;
  *tmpu_stack = -1;
  ref_char_control_pattern = 0;
  for (i = 0; i < num_states; i++) {
    root_node[i].equal_set->state_number = i;
    root_node[i].equal_set->next_node = NULL;
  }
  merging_table = (char **)MALLOC(num_states * sizeof(char *));
  if (merging_table == NULL) {
    mem_result = 1;
  }

  for (j = 0; j < num_states; j++) {
    merging_table[j] = (char *)MALLOC(num_states * sizeof(char));
    if (merging_table[j] == NULL) {
      mem_result = 1;
    }
    for (k = 0; k < num_states; k++)
      merging_table[j][k] = -1; /*initialize merging_table*/
  }

  for (j = 0; j < num_states - 1; j++) {
    for (k = j + 1; k < num_states; k++) {
      flag = 0;
      if (merging_table[j][k] != -1)
        continue;
      // if((root_node[j].marked_in_plant==root_node[k].marked_in_plant) &&
      // (root_node[j].marked_in_controller!=root_node[k].marked_in_controller))
      //   flag = 10;

      temp1 = root_node[j].forb_set;
      temp11 = root_node[k].forb_set;

      if ((temp1 == NULL && temp11 != NULL) ||
          (temp1 != NULL && temp11 == NULL)) {
        flag = 10;
      } else {
        for (l = 1; l < 3; l++) {
          if (l == 2) {
            temp1 = root_node[k].forb_set;
            temp11 = root_node[j].forb_set;
          }
          while (temp1 != NULL) {
            temp12 = temp11;
            cc_flag = false;
            while (temp12 != NULL) {
              if (temp1->event == temp12->event) {
                cc_flag = true;
                break;
              }
              temp12 = temp12->next_event;
            }
            if (!cc_flag) {
              flag = 10;
              break;
            }
            temp1 = temp1->next_event;
          }
        }
      }
      /*
      for(l=1;l<3;l++)
      {
      if(l==1)
      {
      temp1 = root_node[j].forb_set;
      temp2 = root_node[k].tran_set;
      }
      if(l==2)
      {
      temp1 = root_node[k].forb_set;
      temp2 = root_node[j].tran_set;
      }
      temp22 = temp2;
      while(temp1 != NULL)
      {
      temp2 = temp22;
      while(temp2 != NULL)
      {
      if(temp1->event == temp2->event)
      {
      flag = 10;
      break;
      }
      temp2 = temp2->next_transition;
      }
      if(flag == 10) break;
      temp1 = temp1->next_event;
      }
      }
      */

      if (flag == 0) {
        tmpu_stack[0] = j;
        tmpu_stack[1] = k;
        tmpu_stack[2] = -1;

        /* modification is here ........*/
        stack = (struct virtual_stack *)MALLOC(sizeof(struct virtual_stack));
        stack->state_1 = j;
        stack->state_2 = k;
        stack->last_state_pair = NULL;
        stack->next_state_pair = NULL;
        /* end of modification .........*/

        flag = Selfloop_Node2(j, k, -1, 1);
        if (flag != 10) {
          l = 0;
          while (tmpu_stack[2 * l] != -1) {
            merging_table[tmpu_stack[2 * l]][tmpu_stack[2 * l + 1]] = 0;
            merging_table[tmpu_stack[2 * l + 1]][tmpu_stack[2 * l]] = 0;
            l += 1;
          }
          tmpu_stack[0] = -1;

          /*clear memory */
          current_stack = stack;
          while (current_stack != NULL) {
            next_stack = current_stack->next_state_pair;
            free(current_stack);
            current_stack = next_stack;
          }

          stack = NULL;
          /*end of clearing memory */

        } else {
          /*clear memory */
          current_stack = stack;
          while (current_stack != NULL) {
            next_stack = current_stack->next_state_pair;
            merging_table[current_stack->state_1][current_stack->state_2] = 1;
            merging_table[current_stack->state_2][current_stack->state_1] = 1;
            free(current_stack);
            current_stack = next_stack;
          }

          stack = NULL;
          /*end of clearing memory */
        }
      }
    }
  }
  /*Begin the
   * refinement........................................................*/
  filtering = (INT_S **)MALLOC(num_states * sizeof(INT_S *));
  for (j = 0; j < num_states; j++)
    filtering[j] = (INT_S *)MALLOC(2 * sizeof(INT_S));
  for (j = 0; j < num_states; j++) {
    counter = 0;
    for (k = 0; k < num_states; k++)
      if (merging_table[j][k] == 0)
        counter += 1;
    filtering[j][0] = j;
    filtering[j][1] = counter;
  }
  flag_1 = 0;
  while (flag_1 == 0) {
    for (j = 0; j < num_states; j++) {
      if (filtering[j][0] != -1)
        break;
    }
    max_num = filtering[j][1];
    max_state = j;
    for (k = j + 1; k < num_states; k++) {
      if (filtering[k][0] != -1 && filtering[k][1] > max_num) {
        max_num = filtering[k][1];
        max_state = k;
      }
    }
    if (max_num == 0)
      break;
    for (k = 0; k < num_states; k++) {
      if (merging_table[max_state][k] == 0) {
        if (filtering[k][0] != -1)
          filtering[k][1] -= 1;
      }
    }
    filtering[max_state][0] = -1;
  }

  counter = 0;
  for (k = 0; k < num_states; k++)
    if (filtering[k][0] != -1)
      counter += 1;
  ref_char_control_pattern += counter;

  for (j = 0; j < num_states; j++)
    free(filtering[j]);
  free(filtering);
  filtering = NULL;
  free(tmpu_stack);
  tmpu_stack = NULL;

  /* free the memory of merging_table */
  for (k = 0; k < num_states; k++)
    free(merging_table[k]);
  free(merging_table);

  return ref_char_control_pattern;
}

INT_S Refinement3() {
  INT_S i, j, k, flag, flag_1, ref_char_control_pattern;
  INT_S l;
  INT_S **filtering, counter, max_num, max_state;
  //   struct forbidden_event_set *temp1;//, *temp11;
  //   struct transitions *temp2;//, *temp22;
  struct virtual_stack *current_stack, *next_stack;

  //   if (temp1) {} /* Remove warning */
  //   if (temp2) {} /* Remove warning */

  stack = NULL;
  *tmpu_stack = -1;
  ref_char_control_pattern = 0;
  for (i = 0; i < num_states; i++) {
    root_node[i].equal_set->state_number = i;
    root_node[i].equal_set->next_node = NULL;
  }
  merging_table = (char **)MALLOC(num_states * sizeof(char *));
  if (merging_table == NULL) {
    mem_result = 1;
  }

  for (j = 0; j < num_states; j++) {
    merging_table[j] = (char *)MALLOC(num_states * sizeof(char));
    if (merging_table[j] == NULL) {
      mem_result = 1;
    }
    for (k = 0; k < num_states; k++)
      merging_table[j][k] = -1; /*initialize merging_table*/
  }

  for (j = 0; j < num_states - 1; j++) {
    for (k = j + 1; k < num_states; k++) {
      flag = 0;
      if (merging_table[j][k] != -1)
        continue;
      if ((root_node[j].marked_in_controller !=
           root_node[k].marked_in_controller))
        flag = 10;

      // temp1 = root_node[j].forb_set;
      // temp11 = root_node[k].forb_set;

      // if((temp1 == NULL && temp11 != NULL) || (temp1 != NULL && temp11 ==
      // NULL))
      //   flag = 10;
      /*
      for(l=1;l<3;l++)
      {
      if(l==1)
      {
      temp1 = root_node[j].forb_set;
      temp2 = root_node[k].tran_set;
      }
      if(l==2)
      {
      temp1 = root_node[k].forb_set;
      temp2 = root_node[j].tran_set;
      }
      temp22 = temp2;
      while(temp1 != NULL)
      {
      temp2 = temp22;
      while(temp2 != NULL)
      {
      if(temp1->event == temp2->event)
      {
      flag = 10;
      break;
      }
      temp2 = temp2->next_transition;
      }
      if(flag == 10) break;
      temp1 = temp1->next_event;
      }
      }
      */

      if (flag == 0) {
        tmpu_stack[0] = j;
        tmpu_stack[1] = k;
        tmpu_stack[2] = -1;

        /* modification is here ........*/
        stack = (struct virtual_stack *)MALLOC(sizeof(struct virtual_stack));
        stack->state_1 = j;
        stack->state_2 = k;
        stack->last_state_pair = NULL;
        stack->next_state_pair = NULL;
        /* end of modification .........*/

        flag = Selfloop_Node3(j, k, -1, 1);
        if (flag != 10) {
          l = 0;
          while (tmpu_stack[2 * l] != -1) {
            merging_table[tmpu_stack[2 * l]][tmpu_stack[2 * l + 1]] = 0;
            merging_table[tmpu_stack[2 * l + 1]][tmpu_stack[2 * l]] = 0;
            l += 1;
          }
          tmpu_stack[0] = -1;

          /*clear memory */
          current_stack = stack;
          while (current_stack != NULL) {
            next_stack = current_stack->next_state_pair;
            free(current_stack);
            current_stack = next_stack;
          }

          stack = NULL;
          /*end of clearing memory */

        } else {
          /*clear memory */
          current_stack = stack;
          while (current_stack != NULL) {
            next_stack = current_stack->next_state_pair;
            merging_table[current_stack->state_1][current_stack->state_2] = 1;
            merging_table[current_stack->state_2][current_stack->state_1] = 1;
            free(current_stack);
            current_stack = next_stack;
          }

          stack = NULL;
          /*end of clearing memory */
        }
      }
    }
  }
  /*Begin the
   * refinement........................................................*/
  filtering = (INT_S **)MALLOC(num_states * sizeof(INT_S *));
  for (j = 0; j < num_states; j++)
    filtering[j] = (INT_S *)MALLOC(2 * sizeof(INT_S));
  for (j = 0; j < num_states; j++) {
    counter = 0;
    for (k = 0; k < num_states; k++)
      if (merging_table[j][k] == 0)
        counter += 1;
    filtering[j][0] = j;
    filtering[j][1] = counter;
  }
  flag_1 = 0;
  while (flag_1 == 0) {
    for (j = 0; j < num_states; j++) {
      if (filtering[j][0] != -1)
        break;
    }
    max_num = filtering[j][1];
    max_state = j;
    for (k = j + 1; k < num_states; k++) {
      if (filtering[k][0] != -1 && filtering[k][1] > max_num) {
        max_num = filtering[k][1];
        max_state = k;
      }
    }
    if (max_num == 0)
      break;
    for (k = 0; k < num_states; k++) {
      if (merging_table[max_state][k] == 0) {
        if (filtering[k][0] != -1)
          filtering[k][1] -= 1;
      }
    }
    filtering[max_state][0] = -1;
  }

  counter = 0;
  for (k = 0; k < num_states; k++)
    if (filtering[k][0] != -1)
      counter += 1;
  ref_char_control_pattern += counter;

  for (j = 0; j < num_states; j++)
    free(filtering[j]);
  free(filtering);
  filtering = NULL;
  free(tmpu_stack);
  tmpu_stack = NULL;

  /* free the memory of merging_table */
  for (k = 0; k < num_states; k++)
    free(merging_table[k]);
  free(merging_table);

  return ref_char_control_pattern;
}

// Original version
INT_S Refinement() {
  INT_S i, j, k, flag, flag_1, ref_char_control_pattern;
  INT_S l;
  INT_S **filtering, counter, max_num, max_state;
  struct forbidden_event_set *temp1;
  struct transitions *temp2, *temp22;
  struct virtual_stack *current_stack, *next_stack;

  //   if (temp1) {} /* Remove warning */
  //  if (temp2) {} /* Remove warning */

  stack = NULL;
  *tmpu_stack = -1;
  ref_char_control_pattern = 0;
  for (i = 0; i < num_states; i++) {
    root_node[i].equal_set->state_number = i;
    root_node[i].equal_set->next_node = NULL;
  }
  merging_table = (char **)MALLOC(num_states * sizeof(char *));
  if (merging_table == NULL) {
    mem_result = 1;
  }

  for (j = 0; j < num_states; j++) {
    merging_table[j] = (char *)MALLOC(num_states * sizeof(char));
    if (merging_table[j] == NULL) {
      mem_result = 1;
    }
    for (k = 0; k < num_states; k++)
      merging_table[j][k] = -1; /*initialize merging_table*/
  }

  for (j = 0; j < num_states - 1; j++) {
    for (k = j + 1; k < num_states; k++) {
      flag = 0;
      if (merging_table[j][k] != -1)
        continue;
      if ((root_node[j].marked_in_plant == root_node[k].marked_in_plant) &&
          (root_node[j].marked_in_controller !=
           root_node[k].marked_in_controller))
        flag = 10;

      for (l = 1; l < 3; l++) {
        if (l == 1) {
          temp1 = root_node[j].forb_set;
          temp2 = root_node[k].tran_set;
        }
        if (l == 2) {
          temp1 = root_node[k].forb_set;
          temp2 = root_node[j].tran_set;
        }
        temp22 = temp2;
        while (temp1 != NULL) {
          temp2 = temp22;
          while (temp2 != NULL) {
            if (temp1->event == temp2->event) {
              flag = 10;
              break;
            }
            temp2 = temp2->next_transition;
          }
          if (flag == 10)
            break;
          temp1 = temp1->next_event;
        }
      }

      if (flag == 0) {
        tmpu_stack[0] = j;
        tmpu_stack[1] = k;
        tmpu_stack[2] = -1;

        /* modification is here ........*/
        stack = (struct virtual_stack *)MALLOC(sizeof(struct virtual_stack));
        stack->state_1 = j;
        stack->state_2 = k;
        stack->last_state_pair = NULL;
        stack->next_state_pair = NULL;
        /* end of modification .........*/

        flag = Selfloop_Node(j, k, -1, 1);
        if (stack_flag == 100) {
          return 0;
        }
        if (flag != 10) {
          l = 0;
          while (tmpu_stack[2 * l] != -1) {
            merging_table[tmpu_stack[2 * l]][tmpu_stack[2 * l + 1]] = 0;
            merging_table[tmpu_stack[2 * l + 1]][tmpu_stack[2 * l]] = 0;
            l += 1;
          }
          tmpu_stack[0] = -1;

          /*clear memory */
          current_stack = stack;
          while (current_stack != NULL) {
            next_stack = current_stack->next_state_pair;
            free(current_stack);
            current_stack = next_stack;
          }

          stack = NULL;
          /*end of clearing memory */

        } else {
          /*clear memory */
          current_stack = stack;
          while (current_stack != NULL) {
            next_stack = current_stack->next_state_pair;
            merging_table[current_stack->state_1][current_stack->state_2] = 1;
            merging_table[current_stack->state_2][current_stack->state_1] = 1;
            free(current_stack);
            current_stack = next_stack;
          }

          stack = NULL;
          /*end of clearing memory */
        }
      }
    }
  }
  /*Begin the
   * refinement........................................................*/
  filtering = (INT_S **)MALLOC(num_states * sizeof(INT_S *));
  for (j = 0; j < num_states; j++)
    filtering[j] = (INT_S *)MALLOC(2 * sizeof(INT_S));
  for (j = 0; j < num_states; j++) {
    counter = 0;
    for (k = 0; k < num_states; k++)
      if (merging_table[j][k] == 0)
        counter += 1;
    filtering[j][0] = j;
    filtering[j][1] = counter;
  }
  flag_1 = 0;
  while (flag_1 == 0) {
    for (j = 0; j < num_states; j++) {
      if (filtering[j][0] != -1)
        break;
    }
    max_num = filtering[j][1];
    max_state = j;
    for (k = j + 1; k < num_states; k++) {
      if (filtering[k][0] != -1 && filtering[k][1] > max_num) {
        max_num = filtering[k][1];
        max_state = k;
      }
    }
    if (max_num == 0)
      break;
    for (k = 0; k < num_states; k++) {
      if (merging_table[max_state][k] == 0) {
        if (filtering[k][0] != -1)
          filtering[k][1] -= 1;
      }
    }
    filtering[max_state][0] = -1;
  }

  counter = 0;
  for (k = 0; k < num_states; k++)
    if (filtering[k][0] != -1)
      counter += 1;
  ref_char_control_pattern += counter;

  for (j = 0; j < num_states; j++)
    free(filtering[j]);
  free(filtering);
  filtering = NULL;
  free(tmpu_stack);
  tmpu_stack = NULL;

  /* free the memory of merging_table */
  for (k = 0; k < num_states; k++)
    free(merging_table[k]);
  free(merging_table);

  return ref_char_control_pattern;
}
// Modefied by ZRY to implement Localization algorithm without consideration of
// marking information
INT_OS supreduce1(char *name1, char *name2, char *name3, char *name4, INT_S *lb,
                  float *cr, INT_S *fnum) {
  INT_S min_exit, tmp_data, tmp_trace_node, trace_node, index, min_tran_point;
  INT_S base_point, number_of_transitions, tmp_data_1;
  float compress_ratio;
  INT_S trace_mark;
  INT_OS return_code = 0, flag;
  INT_S i;
  FILE *out;

  /* Initial all arrays to NULL */
  c_marked_states = NULL;
  p_marked_states = NULL;
  controller = NULL;
  plant = NULL;
  root_node = NULL;
  controller_tree = NULL;
  simpler_controller = NULL;
  tmpu_stack = NULL;
  record = NULL;

  flag = Get_DES(&tran_number, &num_states, 1, name1);
  if (flag != 0)
    return flag;
  if (num_states == 0)
    return -1;

  flag = Get_DES(&tran_number, &num_states, 0, name2);
  if (flag != 0)
    return flag + 10;
  if (num_states == 0)
    return -1;
  if (tran_number == 1) {
    return_code = -2;
    goto FREEMEM;
  }

  root_node = (struct node *)CALLOC(num_states, sizeof(struct node));
  controller_tree =
      (INT_S *)CALLOC((2 * tran_number + 15 * num_states + 1), sizeof(INT_S));
  tmpu_stack =
      (INT_S *)MALLOC((2 * num_states * num_states + 1) * sizeof(INT_S));
  record = (INT_S *)MALLOC((num_states + 1) * sizeof(INT_S));

  if ((root_node == NULL) || (controller_tree == NULL) ||
      (tmpu_stack == NULL) || (record == NULL)) {
    mem_result = 1;
    return_code = 30;
    goto FREEMEM;
  }

  /* generate controller_tree */
  Controller_Tree();
  free(controller);
  controller = NULL;

  /* generate combined_tree */
  flag = Combined_Tree();
  free(plant);
  plant = NULL;
  if (flag == 1) {
    return_code = 40;
    goto FREEMEM;
  }

  /* generate root_node[] */
  Tree_Structure_Conversion(name3);
  free(controller_tree);
  controller_tree = NULL;

  Reduction1();

  if (mem_result == 1)
    goto FREEMEM;

  /* refine simpler_controller to generate the final text version transition
   * structure of the reduced supervisor */
  base_point = 0;
  while (*(simpler_controller + 3 * base_point) != -1) {
    min_exit = base_point;
    trace_node = base_point + 1;
    while (*(simpler_controller + 3 * trace_node) != -1) {
      if (*(simpler_controller + 3 * trace_node) <
          *(simpler_controller + 3 * min_exit))
        min_exit = trace_node;
      trace_node += 1;
    }
    tmp_data = *(simpler_controller + 3 * base_point);
    *(simpler_controller + 3 * base_point) =
        *(simpler_controller + 3 * min_exit);
    *(simpler_controller + 3 * min_exit) = tmp_data;
    tmp_data = *(simpler_controller + 3 * base_point + 1);
    *(simpler_controller + 3 * base_point + 1) =
        *(simpler_controller + 3 * min_exit + 1);
    *(simpler_controller + 3 * min_exit + 1) = tmp_data;
    tmp_data = *(simpler_controller + 3 * base_point + 2);
    *(simpler_controller + 3 * base_point + 2) =
        *(simpler_controller + 3 * min_exit + 2);
    *(simpler_controller + 3 * min_exit + 2) = tmp_data;
    base_point += 1;
  }
  base_point = 0;
  while (*(simpler_controller + 3 * base_point) != -1) {
    trace_node = base_point + 1;
    while (*(simpler_controller + 3 * trace_node) != -1) {
      if (*(simpler_controller + 3 * trace_node) >
          *(simpler_controller + 3 * base_point))
        break;
      else {
        if ((*(simpler_controller + 3 * trace_node + 1) ==
             *(simpler_controller + 3 * base_point + 1)) &&
            (*(simpler_controller + 3 * trace_node + 2) ==
             *(simpler_controller + 3 * base_point + 2))) {
          tmp_trace_node = trace_node + 1;
          while (*(simpler_controller + 3 * tmp_trace_node) != -1) {
            *(simpler_controller + 3 * tmp_trace_node - 3) =
                *(simpler_controller + 3 * tmp_trace_node);
            *(simpler_controller + 3 * tmp_trace_node - 2) =
                *(simpler_controller + 3 * tmp_trace_node + 1);
            *(simpler_controller + 3 * tmp_trace_node - 1) =
                *(simpler_controller + 3 * tmp_trace_node + 2);
            tmp_trace_node += 1;
          }
          *(simpler_controller + 3 * tmp_trace_node - 3) = -1;
          trace_node -= 1;
        }
      }
      trace_node += 1;
    }
    base_point += 1;
  }
  base_point = 0;
  index = 1;
  tmp_data = 0;
  while (*(simpler_controller + 3 * base_point) != -1) {
    while (*(simpler_controller + 3 * base_point) > tmp_data) {
      if (*(simpler_controller + 3 * base_point) == -1)
        break;
      tmp_data = *(simpler_controller + 3 * base_point);
      trace_mark = 0;
      while (*(c_marked_states + trace_mark) != -1) {
        if (*(c_marked_states + trace_mark) == tmp_data)
          *(c_marked_states + trace_mark) = index;
        trace_mark += 1;
      }
      while ((*(simpler_controller + 3 * base_point) == tmp_data) &&
             (*(simpler_controller + 3 * base_point) != -1)) {
        *(simpler_controller + 3 * base_point) = index;
        base_point += 1;
      }
      tmp_trace_node = 0;
      while (*(simpler_controller + 3 * tmp_trace_node) != -1) {
        if (*(simpler_controller + 3 * tmp_trace_node + 2) == tmp_data)
          *(simpler_controller + 3 * tmp_trace_node + 2) = index;
        tmp_trace_node += 1;
      }
      index += 1;
    }
    if (*(simpler_controller + 3 * base_point) != -1)
      base_point += 1;
  }
  base_point = 0;
  while (*(simpler_controller + 3 * base_point) != -1) {
    tmp_data = *(simpler_controller + 3 * base_point);
    trace_node = base_point;
    while (*(simpler_controller + 3 * base_point) == tmp_data)
      base_point += 1;
    while (trace_node < base_point) {
      min_tran_point = trace_node;
      for (tmp_trace_node = trace_node; tmp_trace_node < base_point;
           tmp_trace_node++) {
        if (*(simpler_controller + 3 * tmp_trace_node + 1) <
            *(simpler_controller + 3 * min_tran_point + 1)) {
          min_tran_point = tmp_trace_node;
        }
      }
      tmp_data_1 = *(simpler_controller + 3 * trace_node + 1);
      *(simpler_controller + 3 * trace_node + 1) =
          *(simpler_controller + 3 * min_tran_point + 1);
      *(simpler_controller + 3 * min_tran_point + 1) = tmp_data_1;
      tmp_data_1 = *(simpler_controller + 3 * trace_node + 2);
      *(simpler_controller + 3 * trace_node + 2) =
          *(simpler_controller + 3 * min_tran_point + 2);
      *(simpler_controller + 3 * min_tran_point + 2) = tmp_data_1;
      trace_node += 1;
    }
  }

  base_point = 0;
  number_of_transitions = 0;
  while (*(simpler_controller + 3 * base_point) != -1) {
    number_of_transitions += 1;
    if (*(simpler_controller + 3 * base_point + 1) >= 1000)
      number_of_transitions -= 1;
    base_point += 1;
  }

  /* output simsup.des */
  tmp_data = *(simpler_controller + 3 * base_point - 3) + 1;
  compress_ratio = ((float)num_states) / ((float)tmp_data);
  *cr = compress_ratio;

  out = fopen(name4, "wb");
  if (out == NULL)
    return 1;
  flag = Txt_DES(out, tmp_data);
  *fnum = tmp_data;
  fclose(out);

  free(simpler_controller);
  simpler_controller = NULL;
  free(c_marked_states);
  c_marked_states = NULL;
  free(p_marked_states);
  p_marked_states = NULL;

  if (flag != 1) {
    return_code = 50;
    goto FREEMEM;
  }

  /* lower bound estimate */
  *lb = 0;

FREEMEM:
  if (root_node != NULL) {
    for (i = 0; i < num_states; i++) {
      struct equivalent_state_set *temp1, *temp11;
      struct forbidden_event_set *temp2, *temp22;
      struct transitions *temp3, *temp33;

      temp1 = root_node[i].equal_set;
      while (temp1 != NULL) {
        temp11 = temp1->next_node;
        free(temp1);
        temp1 = temp11;
      }

      temp2 = root_node[i].forb_set;
      while (temp2 != NULL) {
        temp22 = temp2->next_event;
        free(temp2);
        temp2 = temp22;
      }

      temp3 = root_node[i].tran_set;
      while (temp3 != NULL) {
        temp33 = temp3->next_transition;
        free(temp3);
        temp3 = temp33;
      }
    }
  }
  free(root_node);
  free(controller_tree);
  free(tmpu_stack);
  free(controller);
  free(plant);
  free(record);

  return return_code;
}

// Modefied by ZRY to implement Localization algorithm
INT_OS supreduce2(char *name1, char *name2, char *name3, char *name4, INT_S *lb,
                  float *cr, INT_S *fnum) {
  INT_S min_exit, tmp_data, tmp_trace_node, trace_node, index, min_tran_point;
  INT_S base_point, number_of_transitions, tmp_data_1;
  float compress_ratio;
  INT_S trace_mark;
  INT_OS return_code = 0, flag;
  INT_S i;
  FILE *out;

  /* Initial all arrays to NULL */
  c_marked_states = NULL;
  p_marked_states = NULL;
  controller = NULL;
  plant = NULL;
  root_node = NULL;
  controller_tree = NULL;
  simpler_controller = NULL;
  tmpu_stack = NULL;
  record = NULL;

  flag = Get_DES(&tran_number, &num_states, 1, name1);
  if (flag != 0)
    return flag;
  if (num_states == 0)
    return -1;

  flag = Get_DES(&tran_number, &num_states, 0, name2);
  if (flag != 0)
    return flag + 10;
  if (num_states == 0)
    return -1;
  if (tran_number == 1) {
    return_code = -2;
    goto FREEMEM;
  }

  root_node = (struct node *)CALLOC(num_states, sizeof(struct node));
  controller_tree =
      (INT_S *)CALLOC((2 * tran_number + 15 * num_states + 1), sizeof(INT_S));
  tmpu_stack =
      (INT_S *)MALLOC((2 * num_states * num_states + 1) * sizeof(INT_S));
  record = (INT_S *)MALLOC((num_states + 1) * sizeof(INT_S));

  if ((root_node == NULL) || (controller_tree == NULL) ||
      (tmpu_stack == NULL) || (record == NULL)) {
    mem_result = 1;
    return_code = 30;
    goto FREEMEM;
  }

  /* generate controller_tree */
  Controller_Tree();
  free(controller);
  controller = NULL;

  /* generate combined_tree */
  flag = Combined_Tree();
  free(plant);
  plant = NULL;
  if (flag == 1) {
    return_code = 40;
    goto FREEMEM;
  }

  /* generate root_node[] */
  Tree_Structure_Conversion(name3);
  free(controller_tree);
  controller_tree = NULL;

  Reduction_Faster(root_node, num_states, tran_number);
  // simpler_controller=(INT_S *) MALLOC((3*tran_number+1)*sizeof(INT_S));
  // Final_Result();

  if (mem_result == 1)
    goto FREEMEM;

  /* refine simpler_controller to generate the final text version transition
   * structure of the reduced supervisor */
  base_point = 0;
  while (*(simpler_controller + 3 * base_point) != -1) {
    min_exit = base_point;
    trace_node = base_point + 1;
    while (*(simpler_controller + 3 * trace_node) != -1) {
      if (*(simpler_controller + 3 * trace_node) <
          *(simpler_controller + 3 * min_exit))
        min_exit = trace_node;
      trace_node += 1;
    }
    tmp_data = *(simpler_controller + 3 * base_point);
    *(simpler_controller + 3 * base_point) =
        *(simpler_controller + 3 * min_exit);
    *(simpler_controller + 3 * min_exit) = tmp_data;
    tmp_data = *(simpler_controller + 3 * base_point + 1);
    *(simpler_controller + 3 * base_point + 1) =
        *(simpler_controller + 3 * min_exit + 1);
    *(simpler_controller + 3 * min_exit + 1) = tmp_data;
    tmp_data = *(simpler_controller + 3 * base_point + 2);
    *(simpler_controller + 3 * base_point + 2) =
        *(simpler_controller + 3 * min_exit + 2);
    *(simpler_controller + 3 * min_exit + 2) = tmp_data;
    base_point += 1;
  }
  base_point = 0;
  while (*(simpler_controller + 3 * base_point) != -1) {
    trace_node = base_point + 1;
    while (*(simpler_controller + 3 * trace_node) != -1) {
      if (*(simpler_controller + 3 * trace_node) >
          *(simpler_controller + 3 * base_point))
        break;
      else {
        if ((*(simpler_controller + 3 * trace_node + 1) ==
             *(simpler_controller + 3 * base_point + 1)) &&
            (*(simpler_controller + 3 * trace_node + 2) ==
             *(simpler_controller + 3 * base_point + 2))) {
          tmp_trace_node = trace_node + 1;
          while (*(simpler_controller + 3 * tmp_trace_node) != -1) {
            *(simpler_controller + 3 * tmp_trace_node - 3) =
                *(simpler_controller + 3 * tmp_trace_node);
            *(simpler_controller + 3 * tmp_trace_node - 2) =
                *(simpler_controller + 3 * tmp_trace_node + 1);
            *(simpler_controller + 3 * tmp_trace_node - 1) =
                *(simpler_controller + 3 * tmp_trace_node + 2);
            tmp_trace_node += 1;
          }
          *(simpler_controller + 3 * tmp_trace_node - 3) = -1;
          trace_node -= 1;
        }
      }
      trace_node += 1;
    }
    base_point += 1;
  }
  base_point = 0;
  index = 1;
  tmp_data = 0;
  while (*(simpler_controller + 3 * base_point) != -1) {
    while (*(simpler_controller + 3 * base_point) > tmp_data) {
      if (*(simpler_controller + 3 * base_point) == -1)
        break;
      tmp_data = *(simpler_controller + 3 * base_point);
      trace_mark = 0;
      while (*(c_marked_states + trace_mark) != -1) {
        if (*(c_marked_states + trace_mark) == tmp_data)
          *(c_marked_states + trace_mark) = index;
        trace_mark += 1;
      }
      while ((*(simpler_controller + 3 * base_point) == tmp_data) &&
             (*(simpler_controller + 3 * base_point) != -1)) {
        *(simpler_controller + 3 * base_point) = index;
        base_point += 1;
      }
      tmp_trace_node = 0;
      while (*(simpler_controller + 3 * tmp_trace_node) != -1) {
        if (*(simpler_controller + 3 * tmp_trace_node + 2) == tmp_data)
          *(simpler_controller + 3 * tmp_trace_node + 2) = index;
        tmp_trace_node += 1;
      }
      index += 1;
    }
    if (*(simpler_controller + 3 * base_point) != -1)
      base_point += 1;
  }
  base_point = 0;
  while (*(simpler_controller + 3 * base_point) != -1) {
    tmp_data = *(simpler_controller + 3 * base_point);
    trace_node = base_point;
    while (*(simpler_controller + 3 * base_point) == tmp_data)
      base_point += 1;
    while (trace_node < base_point) {
      min_tran_point = trace_node;
      for (tmp_trace_node = trace_node; tmp_trace_node < base_point;
           tmp_trace_node++) {
        if (*(simpler_controller + 3 * tmp_trace_node + 1) <
            *(simpler_controller + 3 * min_tran_point + 1)) {
          min_tran_point = tmp_trace_node;
        }
      }
      tmp_data_1 = *(simpler_controller + 3 * trace_node + 1);
      *(simpler_controller + 3 * trace_node + 1) =
          *(simpler_controller + 3 * min_tran_point + 1);
      *(simpler_controller + 3 * min_tran_point + 1) = tmp_data_1;
      tmp_data_1 = *(simpler_controller + 3 * trace_node + 2);
      *(simpler_controller + 3 * trace_node + 2) =
          *(simpler_controller + 3 * min_tran_point + 2);
      *(simpler_controller + 3 * min_tran_point + 2) = tmp_data_1;
      trace_node += 1;
    }
  }

  base_point = 0;
  number_of_transitions = 0;
  while (*(simpler_controller + 3 * base_point) != -1) {
    number_of_transitions += 1;
    if (*(simpler_controller + 3 * base_point + 1) >= 1000)
      number_of_transitions -= 1;
    base_point += 1;
  }

  /* output simsup.des */
  tmp_data = *(simpler_controller + 3 * base_point - 3) + 1;
  compress_ratio = ((float)num_states) / ((float)tmp_data);
  *cr = compress_ratio;

  out = fopen(name4, "wb");
  if (out == NULL)
    return 1;
  flag = Txt_DES(out, tmp_data);
  *fnum = tmp_data;
  fclose(out);

  free(simpler_controller);
  simpler_controller = NULL;
  free(c_marked_states);
  c_marked_states = NULL;
  free(p_marked_states);
  p_marked_states = NULL;

  if (flag != 1) {
    return_code = 50;
    goto FREEMEM;
  }

  /* lower bound estimate */
  *lb = 0;

FREEMEM:
  if (root_node != NULL) {
    for (i = 0; i < num_states; i++) {
      struct equivalent_state_set *temp1, *temp11;
      struct forbidden_event_set *temp2, *temp22;
      struct transitions *temp3, *temp33;

      temp1 = root_node[i].equal_set;
      while (temp1 != NULL) {
        temp11 = temp1->next_node;
        free(temp1);
        temp1 = temp11;
      }

      temp2 = root_node[i].forb_set;
      while (temp2 != NULL) {
        temp22 = temp2->next_event;
        free(temp2);
        temp2 = temp22;
      }

      temp3 = root_node[i].tran_set;
      while (temp3 != NULL) {
        temp33 = temp3->next_transition;
        free(temp3);
        temp3 = temp33;
      }
    }
  }
  free(root_node);
  free(controller_tree);
  free(tmpu_stack);
  free(controller);
  free(plant);
  free(record);

  return return_code;
}
// Modefied by ZRY to implement extLocalization algorithm
INT_OS supreduce3(char *name1, char *name2, char *name3, char *name4, INT_S *lb,
                  float *cr, INT_S *fnum) {
  INT_S min_exit, tmp_data, tmp_trace_node, trace_node, index, min_tran_point;
  INT_S base_point, number_of_transitions, tmp_data_1;
  float compress_ratio;
  INT_S trace_mark;
  INT_OS return_code = 0, flag;
  INT_S i;
  FILE *out;

  /* Initial all arrays to NULL */
  c_marked_states = NULL;
  p_marked_states = NULL;
  controller = NULL;
  plant = NULL;
  root_node = NULL;
  controller_tree = NULL;
  simpler_controller = NULL;
  tmpu_stack = NULL;
  record = NULL;

  flag = Get_DES(&tran_number, &num_states, 1, name1);
  if (flag != 0)
    return flag;
  if (num_states == 0)
    return -1;

  flag = Get_DES(&tran_number, &num_states, 0, name2);
  if (flag != 0)
    return flag + 10;
  if (num_states == 0)
    return -1;
  if (tran_number == 1) {
    return_code = -2;
    goto FREEMEM;
  }

  root_node = (struct node *)CALLOC(num_states, sizeof(struct node));
  controller_tree =
      (INT_S *)CALLOC((2 * tran_number + 15 * num_states + 1), sizeof(INT_S));
  tmpu_stack =
      (INT_S *)MALLOC((2 * num_states * num_states + 1) * sizeof(INT_S));
  record = (INT_S *)MALLOC((num_states + 1) * sizeof(INT_S));

  if ((root_node == NULL) || (controller_tree == NULL) ||
      (tmpu_stack == NULL) || (record == NULL)) {
    mem_result = 1;
    return_code = 30;
    goto FREEMEM;
  }

  /* generate controller_tree */
  Controller_Tree();
  free(controller);
  controller = NULL;

  /* generate combined_tree */
  flag = Combined_Tree();
  free(plant);
  plant = NULL;
  if (flag == 1) {
    return_code = 40;
    goto FREEMEM;
  }

  /* generate root_node[] */
  Tree_Structure_Conversion(name3);
  free(controller_tree);
  controller_tree = NULL;

  Reduction2();

  if (mem_result == 1)
    goto FREEMEM;

  /* refine simpler_controller to generate the final text version transition
   * structure of the reduced supervisor */
  base_point = 0;
  while (*(simpler_controller + 3 * base_point) != -1) {
    min_exit = base_point;
    trace_node = base_point + 1;
    while (*(simpler_controller + 3 * trace_node) != -1) {
      if (*(simpler_controller + 3 * trace_node) <
          *(simpler_controller + 3 * min_exit))
        min_exit = trace_node;
      trace_node += 1;
    }
    tmp_data = *(simpler_controller + 3 * base_point);
    *(simpler_controller + 3 * base_point) =
        *(simpler_controller + 3 * min_exit);
    *(simpler_controller + 3 * min_exit) = tmp_data;
    tmp_data = *(simpler_controller + 3 * base_point + 1);
    *(simpler_controller + 3 * base_point + 1) =
        *(simpler_controller + 3 * min_exit + 1);
    *(simpler_controller + 3 * min_exit + 1) = tmp_data;
    tmp_data = *(simpler_controller + 3 * base_point + 2);
    *(simpler_controller + 3 * base_point + 2) =
        *(simpler_controller + 3 * min_exit + 2);
    *(simpler_controller + 3 * min_exit + 2) = tmp_data;
    base_point += 1;
  }
  base_point = 0;
  while (*(simpler_controller + 3 * base_point) != -1) {
    trace_node = base_point + 1;
    while (*(simpler_controller + 3 * trace_node) != -1) {
      if (*(simpler_controller + 3 * trace_node) >
          *(simpler_controller + 3 * base_point))
        break;
      else {
        if ((*(simpler_controller + 3 * trace_node + 1) ==
             *(simpler_controller + 3 * base_point + 1)) &&
            (*(simpler_controller + 3 * trace_node + 2) ==
             *(simpler_controller + 3 * base_point + 2))) {
          tmp_trace_node = trace_node + 1;
          while (*(simpler_controller + 3 * tmp_trace_node) != -1) {
            *(simpler_controller + 3 * tmp_trace_node - 3) =
                *(simpler_controller + 3 * tmp_trace_node);
            *(simpler_controller + 3 * tmp_trace_node - 2) =
                *(simpler_controller + 3 * tmp_trace_node + 1);
            *(simpler_controller + 3 * tmp_trace_node - 1) =
                *(simpler_controller + 3 * tmp_trace_node + 2);
            tmp_trace_node += 1;
          }
          *(simpler_controller + 3 * tmp_trace_node - 3) = -1;
          trace_node -= 1;
        }
      }
      trace_node += 1;
    }
    base_point += 1;
  }
  base_point = 0;
  index = 1;
  tmp_data = 0;
  while (*(simpler_controller + 3 * base_point) != -1) {
    while (*(simpler_controller + 3 * base_point) > tmp_data) {
      if (*(simpler_controller + 3 * base_point) == -1)
        break;
      tmp_data = *(simpler_controller + 3 * base_point);
      trace_mark = 0;
      while (*(c_marked_states + trace_mark) != -1) {
        if (*(c_marked_states + trace_mark) == tmp_data)
          *(c_marked_states + trace_mark) = index;
        trace_mark += 1;
      }
      while ((*(simpler_controller + 3 * base_point) == tmp_data) &&
             (*(simpler_controller + 3 * base_point) != -1)) {
        *(simpler_controller + 3 * base_point) = index;
        base_point += 1;
      }
      tmp_trace_node = 0;
      while (*(simpler_controller + 3 * tmp_trace_node) != -1) {
        if (*(simpler_controller + 3 * tmp_trace_node + 2) == tmp_data)
          *(simpler_controller + 3 * tmp_trace_node + 2) = index;
        tmp_trace_node += 1;
      }
      index += 1;
    }
    if (*(simpler_controller + 3 * base_point) != -1)
      base_point += 1;
  }
  base_point = 0;
  while (*(simpler_controller + 3 * base_point) != -1) {
    tmp_data = *(simpler_controller + 3 * base_point);
    trace_node = base_point;
    while (*(simpler_controller + 3 * base_point) == tmp_data)
      base_point += 1;
    while (trace_node < base_point) {
      min_tran_point = trace_node;
      for (tmp_trace_node = trace_node; tmp_trace_node < base_point;
           tmp_trace_node++) {
        if (*(simpler_controller + 3 * tmp_trace_node + 1) <
            *(simpler_controller + 3 * min_tran_point + 1)) {
          min_tran_point = tmp_trace_node;
        }
      }
      tmp_data_1 = *(simpler_controller + 3 * trace_node + 1);
      *(simpler_controller + 3 * trace_node + 1) =
          *(simpler_controller + 3 * min_tran_point + 1);
      *(simpler_controller + 3 * min_tran_point + 1) = tmp_data_1;
      tmp_data_1 = *(simpler_controller + 3 * trace_node + 2);
      *(simpler_controller + 3 * trace_node + 2) =
          *(simpler_controller + 3 * min_tran_point + 2);
      *(simpler_controller + 3 * min_tran_point + 2) = tmp_data_1;
      trace_node += 1;
    }
  }

  base_point = 0;
  number_of_transitions = 0;
  while (*(simpler_controller + 3 * base_point) != -1) {
    number_of_transitions += 1;
    if (*(simpler_controller + 3 * base_point + 1) >= 1000)
      number_of_transitions -= 1;
    base_point += 1;
  }

  /* output simsup.des */
  tmp_data = *(simpler_controller + 3 * base_point - 3) + 1;
  compress_ratio = ((float)num_states) / ((float)tmp_data);
  *cr = compress_ratio;

  out = fopen(name4, "wb");
  if (out == NULL)
    return 1;
  flag = Txt_DES(out, tmp_data);
  *fnum = tmp_data;
  fclose(out);

  free(simpler_controller);
  simpler_controller = NULL;
  free(c_marked_states);
  c_marked_states = NULL;
  free(p_marked_states);
  p_marked_states = NULL;

  if (flag != 1) {
    return_code = 50;
    goto FREEMEM;
  }

  /* lower bound estimate */
  *lb = Refinement2();

FREEMEM:
  if (root_node != NULL) {
    for (i = 0; i < num_states; i++) {
      struct equivalent_state_set *temp1, *temp11;
      struct forbidden_event_set *temp2, *temp22;
      struct transitions *temp3, *temp33;

      temp1 = root_node[i].equal_set;
      while (temp1 != NULL) {
        temp11 = temp1->next_node;
        free(temp1);
        temp1 = temp11;
      }

      temp2 = root_node[i].forb_set;
      while (temp2 != NULL) {
        temp22 = temp2->next_event;
        free(temp2);
        temp2 = temp22;
      }

      temp3 = root_node[i].tran_set;
      while (temp3 != NULL) {
        temp33 = temp3->next_transition;
        free(temp3);
        temp3 = temp33;
      }
    }
  }
  free(root_node);
  free(controller_tree);
  free(tmpu_stack);
  free(controller);
  free(plant);
  free(record);

  return return_code;
}
INT_OS supreduce4(char *name1, char *name2, char *name3, char *name4, INT_S *lb,
                  float *cr, INT_S *fnum) {
  INT_S min_exit, tmp_data, tmp_trace_node, trace_node, index, min_tran_point;
  INT_S base_point, number_of_transitions, tmp_data_1;
  float compress_ratio;
  INT_S trace_mark;
  INT_OS return_code = 0, flag;
  INT_S i;
  FILE *out;

  /* Initial all arrays to NULL */
  c_marked_states = NULL;
  p_marked_states = NULL;
  controller = NULL;
  plant = NULL;
  root_node = NULL;
  controller_tree = NULL;
  simpler_controller = NULL;
  tmpu_stack = NULL;
  record = NULL;

  flag = Get_DES(&tran_number, &num_states, 1, name1);
  if (flag != 0)
    return flag;
  if (num_states == 0)
    return -1;

  flag = Get_DES(&tran_number, &num_states, 0, name2);
  if (flag != 0)
    return flag + 10;
  if (num_states == 0)
    return -1;
  if (tran_number == 1) {
    return_code = -2;
    goto FREEMEM;
  }

  root_node = (struct node *)CALLOC(num_states, sizeof(struct node));
  controller_tree =
      (INT_S *)CALLOC((2 * tran_number + 15 * num_states + 1), sizeof(INT_S));
  tmpu_stack =
      (INT_S *)MALLOC((2 * num_states * num_states + 1) * sizeof(INT_S));
  record = (INT_S *)MALLOC((num_states + 1) * sizeof(INT_S));

  if ((root_node == NULL) || (controller_tree == NULL) ||
      (tmpu_stack == NULL) || (record == NULL)) {
    mem_result = 1;
    return_code = 30;
    goto FREEMEM;
  }

  /* generate controller_tree */
  Controller_Tree();
  free(controller);
  controller = NULL;

  /* generate combined_tree */
  flag = Combined_Tree();
  free(plant);
  plant = NULL;
  if (flag == 1) {
    return_code = 40;
    goto FREEMEM;
  }

  /* generate root_node[] */
  Tree_Structure_Conversion(name3);
  free(controller_tree);
  controller_tree = NULL;

  Reduction3();

  if (mem_result == 1)
    goto FREEMEM;

  /* refine simpler_controller to generate the final text version transition
   * structure of the reduced supervisor */
  base_point = 0;
  while (*(simpler_controller + 3 * base_point) != -1) {
    min_exit = base_point;
    trace_node = base_point + 1;
    while (*(simpler_controller + 3 * trace_node) != -1) {
      if (*(simpler_controller + 3 * trace_node) <
          *(simpler_controller + 3 * min_exit))
        min_exit = trace_node;
      trace_node += 1;
    }
    tmp_data = *(simpler_controller + 3 * base_point);
    *(simpler_controller + 3 * base_point) =
        *(simpler_controller + 3 * min_exit);
    *(simpler_controller + 3 * min_exit) = tmp_data;
    tmp_data = *(simpler_controller + 3 * base_point + 1);
    *(simpler_controller + 3 * base_point + 1) =
        *(simpler_controller + 3 * min_exit + 1);
    *(simpler_controller + 3 * min_exit + 1) = tmp_data;
    tmp_data = *(simpler_controller + 3 * base_point + 2);
    *(simpler_controller + 3 * base_point + 2) =
        *(simpler_controller + 3 * min_exit + 2);
    *(simpler_controller + 3 * min_exit + 2) = tmp_data;
    base_point += 1;
  }
  base_point = 0;
  while (*(simpler_controller + 3 * base_point) != -1) {
    trace_node = base_point + 1;
    while (*(simpler_controller + 3 * trace_node) != -1) {
      if (*(simpler_controller + 3 * trace_node) >
          *(simpler_controller + 3 * base_point))
        break;
      else {
        if ((*(simpler_controller + 3 * trace_node + 1) ==
             *(simpler_controller + 3 * base_point + 1)) &&
            (*(simpler_controller + 3 * trace_node + 2) ==
             *(simpler_controller + 3 * base_point + 2))) {
          tmp_trace_node = trace_node + 1;
          while (*(simpler_controller + 3 * tmp_trace_node) != -1) {
            *(simpler_controller + 3 * tmp_trace_node - 3) =
                *(simpler_controller + 3 * tmp_trace_node);
            *(simpler_controller + 3 * tmp_trace_node - 2) =
                *(simpler_controller + 3 * tmp_trace_node + 1);
            *(simpler_controller + 3 * tmp_trace_node - 1) =
                *(simpler_controller + 3 * tmp_trace_node + 2);
            tmp_trace_node += 1;
          }
          *(simpler_controller + 3 * tmp_trace_node - 3) = -1;
          trace_node -= 1;
        }
      }
      trace_node += 1;
    }
    base_point += 1;
  }
  base_point = 0;
  index = 1;
  tmp_data = 0;
  while (*(simpler_controller + 3 * base_point) != -1) {
    while (*(simpler_controller + 3 * base_point) > tmp_data) {
      if (*(simpler_controller + 3 * base_point) == -1)
        break;
      tmp_data = *(simpler_controller + 3 * base_point);
      trace_mark = 0;
      while (*(c_marked_states + trace_mark) != -1) {
        if (*(c_marked_states + trace_mark) == tmp_data)
          *(c_marked_states + trace_mark) = index;
        trace_mark += 1;
      }
      while ((*(simpler_controller + 3 * base_point) == tmp_data) &&
             (*(simpler_controller + 3 * base_point) != -1)) {
        *(simpler_controller + 3 * base_point) = index;
        base_point += 1;
      }
      tmp_trace_node = 0;
      while (*(simpler_controller + 3 * tmp_trace_node) != -1) {
        if (*(simpler_controller + 3 * tmp_trace_node + 2) == tmp_data)
          *(simpler_controller + 3 * tmp_trace_node + 2) = index;
        tmp_trace_node += 1;
      }
      index += 1;
    }
    if (*(simpler_controller + 3 * base_point) != -1)
      base_point += 1;
  }
  base_point = 0;
  while (*(simpler_controller + 3 * base_point) != -1) {
    tmp_data = *(simpler_controller + 3 * base_point);
    trace_node = base_point;
    while (*(simpler_controller + 3 * base_point) == tmp_data)
      base_point += 1;
    while (trace_node < base_point) {
      min_tran_point = trace_node;
      for (tmp_trace_node = trace_node; tmp_trace_node < base_point;
           tmp_trace_node++) {
        if (*(simpler_controller + 3 * tmp_trace_node + 1) <
            *(simpler_controller + 3 * min_tran_point + 1)) {
          min_tran_point = tmp_trace_node;
        }
      }
      tmp_data_1 = *(simpler_controller + 3 * trace_node + 1);
      *(simpler_controller + 3 * trace_node + 1) =
          *(simpler_controller + 3 * min_tran_point + 1);
      *(simpler_controller + 3 * min_tran_point + 1) = tmp_data_1;
      tmp_data_1 = *(simpler_controller + 3 * trace_node + 2);
      *(simpler_controller + 3 * trace_node + 2) =
          *(simpler_controller + 3 * min_tran_point + 2);
      *(simpler_controller + 3 * min_tran_point + 2) = tmp_data_1;
      trace_node += 1;
    }
  }

  base_point = 0;
  number_of_transitions = 0;
  while (*(simpler_controller + 3 * base_point) != -1) {
    number_of_transitions += 1;
    if (*(simpler_controller + 3 * base_point + 1) >= 1000)
      number_of_transitions -= 1;
    base_point += 1;
  }

  /* output simsup.des */
  tmp_data = *(simpler_controller + 3 * base_point - 3) + 1;
  compress_ratio = ((float)num_states) / ((float)tmp_data);
  *cr = compress_ratio;

  out = fopen(name4, "wb");
  if (out == NULL)
    return 1;
  flag = Txt_DES(out, tmp_data);
  *fnum = tmp_data;
  fclose(out);

  free(simpler_controller);
  simpler_controller = NULL;
  free(c_marked_states);
  c_marked_states = NULL;
  free(p_marked_states);
  p_marked_states = NULL;

  if (flag != 1) {
    return_code = 50;
    goto FREEMEM;
  }

  /* lower bound estimate */
  *lb = Refinement3();

FREEMEM:
  if (root_node != NULL) {
    for (i = 0; i < num_states; i++) {
      struct equivalent_state_set *temp1, *temp11;
      struct forbidden_event_set *temp2, *temp22;
      struct transitions *temp3, *temp33;

      temp1 = root_node[i].equal_set;
      while (temp1 != NULL) {
        temp11 = temp1->next_node;
        free(temp1);
        temp1 = temp11;
      }

      temp2 = root_node[i].forb_set;
      while (temp2 != NULL) {
        temp22 = temp2->next_event;
        free(temp2);
        temp2 = temp22;
      }

      temp3 = root_node[i].tran_set;
      while (temp3 != NULL) {
        temp33 = temp3->next_transition;
        free(temp3);
        temp3 = temp33;
      }
    }
  }
  free(root_node);
  free(controller_tree);
  free(tmpu_stack);
  free(controller);
  free(plant);
  free(record);

  return return_code;
}
// Modified by ZRY to test a more efficient algorithm 2018.01.15 with quadratic
// complexity
INT_OS supreduce5(char *name1, char *name2, char *name3, char *name4, INT_S *lb,
                  float *cr) {
  INT_S min_exit, tmp_data, tmp_trace_node, trace_node, index, min_tran_point;
  INT_S base_point, number_of_transitions, tmp_data_1;
  float compress_ratio;
  INT_S trace_mark;
  INT_OS return_code = 0, flag;
  INT_S i, j;
  FILE *out;

  /* Initial all arrays to NULL */
  c_marked_states = NULL;
  p_marked_states = NULL;
  controller = NULL;
  plant = NULL;
  root_node = NULL;
  tmp_root_node = NULL;
  track_root_node = NULL;
  controller_tree = NULL;
  simpler_controller = NULL;
  tmpu_stack = NULL;
  cross_stack = NULL;
  record = NULL;
  track_table = NULL;
  mergelist = NULL;

  flag = Get_DES(&tran_number, &num_states, 1, name1);
  if (flag != 0)
    return flag;
  if (num_states == 0)
    return -1;

  flag = Get_DES(&tran_number, &num_states, 0, name2);
  if (flag != 0)
    return flag + 10;
  if (num_states == 0)
    return -1;
  if (tran_number == 1) {
    return_code = -2;
    goto FREEMEM;
  }

  root_node = (struct node *)CALLOC(num_states, sizeof(struct node));
  controller_tree =
      (INT_S *)CALLOC((2 * tran_number + 15 * num_states + 1), sizeof(INT_S));
  tmpu_stack =
      (INT_S *)MALLOC((2 * num_states * num_states + 1) * sizeof(INT_S));
  record = (INT_S *)MALLOC((num_states + 1) * sizeof(INT_S));

  if ((root_node == NULL) || (controller_tree == NULL) ||
      (tmpu_stack == NULL) || (record == NULL)) {
    mem_result = 1;
    return_code = 30;
    goto FREEMEM;
  }

  /* generate controller_tree */
  Controller_Tree();
  free(controller);
  controller = NULL;
  if (stack_flag == 100) {
    return_code = 100;
    goto FREEMEM;
  }

  /* generate combined_tree */
  flag = Combined_Tree();
  free(plant);
  plant = NULL;
  if (stack_flag == 100) {
    return_code = 100;
    goto FREEMEM;
  }
  if (flag == 1) {
    return_code = 40;
    goto FREEMEM;
  }

  /* generate root_node[] */
  Tree_Structure_Conversion(name3);
  free(controller_tree);
  controller_tree = NULL;

  // generate a track table and initialize it
  s_track_table = num_states - 1;
  track_table = (cc_check_table *)CALLOC(s_track_table, sizeof(cc_check_table));
  if (track_table == NULL) {
    mem_result = 1;
    return_code = 30;
    goto FREEMEM;
  }
  for (i = 0; i < s_track_table; i++) {
    track_table[i].numelts = (num_states - 1) - i;
    track_table[i].next =
        (check_mark *)CALLOC(track_table[i].numelts, sizeof(check_mark));
    if (track_table[i].next == NULL) {
      mem_result = 1;
      return_code = 30;
      goto FREEMEM;
    }
    for (j = 0; j < track_table[i].numelts; j++) {
      track_table[i].next[j].flag = 0;
      track_table[i].next[j].index = -1;
      track_table[i].next[j].minindex = -1;
      track_table[i].next[j].numelts = 0;
      track_table[i].next[j].dynindex = NULL;
    }
  }

  tmp_root_node = (struct node *)CALLOC(num_states, sizeof(struct node));
  track_root_node =
      (struct track_node *)CALLOC(num_states, sizeof(struct track_node));
  cross_stack =
      (INT_S *)MALLOC((2 * num_states * num_states + 1) * sizeof(INT_S));
  // mergelist = (INT_B*)CALLOC(num_states, sizeof(INT_B));

  if (tmp_root_node == NULL || track_root_node == NULL || cross_stack == NULL) {
    mem_result = 1;
    return_code = 30;
    goto FREEMEM;
  }

  Reduction5();
  if (mem_result == 1) {
    return_code = 30;
    goto FREEMEM;
  }

  if (root_node != NULL) {
    for (i = 0; i < num_states; i++) {
      struct equivalent_state_set *temp1, *temp11;
      struct forbidden_event_set *temp2, *temp22;
      struct transitions *temp3, *temp33;

      temp1 = root_node[i].equal_set;
      while (temp1 != NULL) {
        temp11 = temp1->next_node;
        free(temp1);
        temp1 = temp11;
      }

      temp2 = root_node[i].forb_set;
      while (temp2 != NULL) {
        temp22 = temp2->next_event;
        free(temp2);
        temp2 = temp22;
      }

      temp3 = root_node[i].tran_set;
      while (temp3 != NULL) {
        temp33 = temp3->next_transition;
        free(temp3);
        temp3 = temp33;
      }
    }
    free(root_node);
    root_node = NULL;
  }
  if (tmp_root_node != NULL) {
    for (i = 0; i < num_states; i++) {
      struct equivalent_state_set *temp4, *temp44;

      temp4 = tmp_root_node[i].equal_set;
      while (temp4 != NULL) {
        temp44 = temp4->next_node;
        free(temp4);
        temp4 = temp44;
      }
    }
    free(tmp_root_node);
    tmp_root_node = NULL;
  }

  free_cc_check_table(s_track_table, &track_table);
  track_table = NULL;
  free(cross_stack);
  cross_stack = NULL;

  /* refine simpler_controller to generate the final text version transition
   * structure of the reduced supervisor */
  base_point = 0;
  while (*(simpler_controller + 3 * base_point) != -1) {
    min_exit = base_point;
    trace_node = base_point + 1;
    while (*(simpler_controller + 3 * trace_node) != -1) {
      if (*(simpler_controller + 3 * trace_node) <
          *(simpler_controller + 3 * min_exit))
        min_exit = trace_node;
      trace_node += 1;
    }
    tmp_data = *(simpler_controller + 3 * base_point);
    *(simpler_controller + 3 * base_point) =
        *(simpler_controller + 3 * min_exit);
    *(simpler_controller + 3 * min_exit) = tmp_data;
    tmp_data = *(simpler_controller + 3 * base_point + 1);
    *(simpler_controller + 3 * base_point + 1) =
        *(simpler_controller + 3 * min_exit + 1);
    *(simpler_controller + 3 * min_exit + 1) = tmp_data;
    tmp_data = *(simpler_controller + 3 * base_point + 2);
    *(simpler_controller + 3 * base_point + 2) =
        *(simpler_controller + 3 * min_exit + 2);
    *(simpler_controller + 3 * min_exit + 2) = tmp_data;
    base_point += 1;
  }
  base_point = 0;
  while (*(simpler_controller + 3 * base_point) != -1) {
    trace_node = base_point + 1;
    while (*(simpler_controller + 3 * trace_node) != -1) {
      if (*(simpler_controller + 3 * trace_node) >
          *(simpler_controller + 3 * base_point))
        break;
      else {
        if ((*(simpler_controller + 3 * trace_node + 1) ==
             *(simpler_controller + 3 * base_point + 1)) &&
            (*(simpler_controller + 3 * trace_node + 2) ==
             *(simpler_controller + 3 * base_point + 2))) {
          tmp_trace_node = trace_node + 1;
          while (*(simpler_controller + 3 * tmp_trace_node) != -1) {
            *(simpler_controller + 3 * tmp_trace_node - 3) =
                *(simpler_controller + 3 * tmp_trace_node);
            *(simpler_controller + 3 * tmp_trace_node - 2) =
                *(simpler_controller + 3 * tmp_trace_node + 1);
            *(simpler_controller + 3 * tmp_trace_node - 1) =
                *(simpler_controller + 3 * tmp_trace_node + 2);
            tmp_trace_node += 1;
          }
          *(simpler_controller + 3 * tmp_trace_node - 3) = -1;
          trace_node -= 1;
        }
      }
      trace_node += 1;
    }
    base_point += 1;
  }
  base_point = 0;
  index = 1;
  tmp_data = 0;
  while (*(simpler_controller + 3 * base_point) != -1) {
    while (*(simpler_controller + 3 * base_point) > tmp_data) {
      if (*(simpler_controller + 3 * base_point) == -1)
        break;
      tmp_data = *(simpler_controller + 3 * base_point);
      trace_mark = 0;
      while (*(c_marked_states + trace_mark) != -1) {
        if (*(c_marked_states + trace_mark) == tmp_data)
          *(c_marked_states + trace_mark) = index;
        trace_mark += 1;
      }
      while ((*(simpler_controller + 3 * base_point) == tmp_data) &&
             (*(simpler_controller + 3 * base_point) != -1)) {
        *(simpler_controller + 3 * base_point) = index;
        base_point += 1;
      }
      tmp_trace_node = 0;
      while (*(simpler_controller + 3 * tmp_trace_node) != -1) {
        if (*(simpler_controller + 3 * tmp_trace_node + 2) == tmp_data)
          *(simpler_controller + 3 * tmp_trace_node + 2) = index;
        tmp_trace_node += 1;
      }
      index += 1;
    }
    if (*(simpler_controller + 3 * base_point) != -1)
      base_point += 1;
  }
  base_point = 0;
  while (*(simpler_controller + 3 * base_point) != -1) {
    tmp_data = *(simpler_controller + 3 * base_point);
    trace_node = base_point;
    while (*(simpler_controller + 3 * base_point) == tmp_data)
      base_point += 1;
    while (trace_node < base_point) {
      min_tran_point = trace_node;
      for (tmp_trace_node = trace_node; tmp_trace_node < base_point;
           tmp_trace_node++) {
        if (*(simpler_controller + 3 * tmp_trace_node + 1) <
            *(simpler_controller + 3 * min_tran_point + 1)) {
          min_tran_point = tmp_trace_node;
        }
      }
      tmp_data_1 = *(simpler_controller + 3 * trace_node + 1);
      *(simpler_controller + 3 * trace_node + 1) =
          *(simpler_controller + 3 * min_tran_point + 1);
      *(simpler_controller + 3 * min_tran_point + 1) = tmp_data_1;
      tmp_data_1 = *(simpler_controller + 3 * trace_node + 2);
      *(simpler_controller + 3 * trace_node + 2) =
          *(simpler_controller + 3 * min_tran_point + 2);
      *(simpler_controller + 3 * min_tran_point + 2) = tmp_data_1;
      trace_node += 1;
    }
  }

  base_point = 0;
  number_of_transitions = 0;
  while (*(simpler_controller + 3 * base_point) != -1) {
    number_of_transitions += 1;
    if (*(simpler_controller + 3 * base_point + 1) >= 1000)
      number_of_transitions -= 1;
    base_point += 1;
  }

  /* output simsup.des */
  tmp_data = *(simpler_controller + 3 * base_point - 3) + 1;
  compress_ratio = ((float)num_states) / ((float)tmp_data);
  *cr = compress_ratio;

  out = fopen(name4, "wb");
  if (out == NULL)
    return 1;
  flag = Txt_DES(out, tmp_data);
  fclose(out);

  free(simpler_controller);
  simpler_controller = NULL;
  free(c_marked_states);
  c_marked_states = NULL;
  free(p_marked_states);
  p_marked_states = NULL;

  if (flag != 1) {
    return_code = 50;
    goto FREEMEM;
  }

  *lb = 0;
  /* lower bound estimate */
  //*lb = Refinement();
  // if(stack_flag == 100)
  //	return_code = 100;

FREEMEM:
  if (root_node != NULL) {
    for (i = 0; i < num_states; i++) {
      struct equivalent_state_set *temp1, *temp11;
      struct forbidden_event_set *temp2, *temp22;
      struct transitions *temp3, *temp33;

      temp1 = root_node[i].equal_set;
      while (temp1 != NULL) {
        temp11 = temp1->next_node;
        free(temp1);
        temp1 = temp11;
      }

      temp2 = root_node[i].forb_set;
      while (temp2 != NULL) {
        temp22 = temp2->next_event;
        free(temp2);
        temp2 = temp22;
      }

      temp3 = root_node[i].tran_set;
      while (temp3 != NULL) {
        temp33 = temp3->next_transition;
        free(temp3);
        temp3 = temp33;
      }
    }
    free(root_node);
  }
  if (tmp_root_node != NULL) {
    for (i = 0; i < num_states; i++) {
      struct equivalent_state_set *temp4, *temp44;

      temp4 = tmp_root_node[i].equal_set;
      while (temp4 != NULL) {
        temp44 = temp4->next_node;
        free(temp4);
        temp4 = temp44;
      }
    }
    free(tmp_root_node);
  }
  /*if (track_root_node != NULL)
  {
          for (i=0; i < num_states; i++) {
                  struct equivalent_state_set *temp4, *temp44;

                  temp4 = track_root_node[i].equal_set;
                  while (temp4 != NULL)
                  {
                          temp44 = temp4->next_node;
                          free(temp4);
                          temp4 = temp44;
                  }
          }
          free(track_root_node);
  }*/
  free_cc_check_table(s_track_table, &track_table);
  free(cross_stack);
  free(mergelist);
  free(controller_tree);
  free(tmpu_stack);
  free(controller);
  free(plant);
  free(record);

  return return_code;
}
// Original version without lower bound estimation
INT_OS supreduce6(char *name1, char *name2, char *name3, char *name4, INT_S *lb,
                  float *cr) {
  INT_S min_exit, tmp_data, tmp_trace_node, trace_node, index, min_tran_point;
  INT_S base_point, number_of_transitions, tmp_data_1;
  float compress_ratio;
  INT_S trace_mark;
  INT_OS return_code = 0, flag;
  INT_S i;
  FILE *out;

  /* Initial all arrays to NULL */
  c_marked_states = NULL;
  p_marked_states = NULL;
  controller = NULL;
  plant = NULL;
  root_node = NULL;
  controller_tree = NULL;
  simpler_controller = NULL;
  tmpu_stack = NULL;
  record = NULL;

  flag = Get_DES(&tran_number, &num_states, 1, name1);
  if (flag != 0)
    return flag;
  if (num_states == 0)
    return -1;

  flag = Get_DES(&tran_number, &num_states, 0, name2);
  if (flag != 0)
    return flag + 10;
  if (num_states == 0)
    return -1;
  if (tran_number == 1) {
    return_code = -2;
    goto FREEMEM;
  }

  root_node = (struct node *)CALLOC(num_states, sizeof(struct node));
  controller_tree =
      (INT_S *)CALLOC((2 * tran_number + 15 * num_states + 1), sizeof(INT_S));
  tmpu_stack =
      (INT_S *)MALLOC((2 * num_states * num_states + 1) * sizeof(INT_S));
  record = (INT_S *)MALLOC((num_states + 1) * sizeof(INT_S));

  if ((root_node == NULL) || (controller_tree == NULL) ||
      (tmpu_stack == NULL) || (record == NULL)) {
    mem_result = 1;
    return_code = 30;
    goto FREEMEM;
  }

  /* generate controller_tree */
  Controller_Tree();
  free(controller);
  controller = NULL;
  if (stack_flag == 100) {
    return_code = 100;
    goto FREEMEM;
  }

  /* generate combined_tree */
  flag = Combined_Tree();
  free(plant);
  plant = NULL;
  if (stack_flag == 100) {
    return_code = 100;
    goto FREEMEM;
  }
  if (flag == 1) {
    return_code = 40;
    goto FREEMEM;
  }

  /* generate root_node[] */
  Tree_Structure_Conversion(name3);
  free(controller_tree);
  controller_tree = NULL;

  Reduction_Faster(root_node, num_states, tran_number);
  if (stack_flag == 100) {
    return_code = 100;
    goto FREEMEM;
  }

  if (mem_result == 1)
    goto FREEMEM;

  /* refine simpler_controller to generate the final text version transition
   * structure of the reduced supervisor */
  base_point = 0;
  while (*(simpler_controller + 3 * base_point) != -1) {
    min_exit = base_point;
    trace_node = base_point + 1;
    while (*(simpler_controller + 3 * trace_node) != -1) {
      if (*(simpler_controller + 3 * trace_node) <
          *(simpler_controller + 3 * min_exit))
        min_exit = trace_node;
      trace_node += 1;
    }
    tmp_data = *(simpler_controller + 3 * base_point);
    *(simpler_controller + 3 * base_point) =
        *(simpler_controller + 3 * min_exit);
    *(simpler_controller + 3 * min_exit) = tmp_data;
    tmp_data = *(simpler_controller + 3 * base_point + 1);
    *(simpler_controller + 3 * base_point + 1) =
        *(simpler_controller + 3 * min_exit + 1);
    *(simpler_controller + 3 * min_exit + 1) = tmp_data;
    tmp_data = *(simpler_controller + 3 * base_point + 2);
    *(simpler_controller + 3 * base_point + 2) =
        *(simpler_controller + 3 * min_exit + 2);
    *(simpler_controller + 3 * min_exit + 2) = tmp_data;
    base_point += 1;
  }
  base_point = 0;
  while (*(simpler_controller + 3 * base_point) != -1) {
    trace_node = base_point + 1;
    while (*(simpler_controller + 3 * trace_node) != -1) {
      if (*(simpler_controller + 3 * trace_node) >
          *(simpler_controller + 3 * base_point))
        break;
      else {
        if ((*(simpler_controller + 3 * trace_node + 1) ==
             *(simpler_controller + 3 * base_point + 1)) &&
            (*(simpler_controller + 3 * trace_node + 2) ==
             *(simpler_controller + 3 * base_point + 2))) {
          tmp_trace_node = trace_node + 1;
          while (*(simpler_controller + 3 * tmp_trace_node) != -1) {
            *(simpler_controller + 3 * tmp_trace_node - 3) =
                *(simpler_controller + 3 * tmp_trace_node);
            *(simpler_controller + 3 * tmp_trace_node - 2) =
                *(simpler_controller + 3 * tmp_trace_node + 1);
            *(simpler_controller + 3 * tmp_trace_node - 1) =
                *(simpler_controller + 3 * tmp_trace_node + 2);
            tmp_trace_node += 1;
          }
          *(simpler_controller + 3 * tmp_trace_node - 3) = -1;
          trace_node -= 1;
        }
      }
      trace_node += 1;
    }
    base_point += 1;
  }
  base_point = 0;
  index = 1;
  tmp_data = 0;
  while (*(simpler_controller + 3 * base_point) != -1) {
    while (*(simpler_controller + 3 * base_point) > tmp_data) {
      if (*(simpler_controller + 3 * base_point) == -1)
        break;
      tmp_data = *(simpler_controller + 3 * base_point);
      trace_mark = 0;
      while (*(c_marked_states + trace_mark) != -1) {
        if (*(c_marked_states + trace_mark) == tmp_data)
          *(c_marked_states + trace_mark) = index;
        trace_mark += 1;
      }
      while ((*(simpler_controller + 3 * base_point) == tmp_data) &&
             (*(simpler_controller + 3 * base_point) != -1)) {
        *(simpler_controller + 3 * base_point) = index;
        base_point += 1;
      }
      tmp_trace_node = 0;
      while (*(simpler_controller + 3 * tmp_trace_node) != -1) {
        if (*(simpler_controller + 3 * tmp_trace_node + 2) == tmp_data)
          *(simpler_controller + 3 * tmp_trace_node + 2) = index;
        tmp_trace_node += 1;
      }
      index += 1;
    }
    if (*(simpler_controller + 3 * base_point) != -1)
      base_point += 1;
  }
  base_point = 0;
  while (*(simpler_controller + 3 * base_point) != -1) {
    tmp_data = *(simpler_controller + 3 * base_point);
    trace_node = base_point;
    while (*(simpler_controller + 3 * base_point) == tmp_data)
      base_point += 1;
    while (trace_node < base_point) {
      min_tran_point = trace_node;
      for (tmp_trace_node = trace_node; tmp_trace_node < base_point;
           tmp_trace_node++) {
        if (*(simpler_controller + 3 * tmp_trace_node + 1) <
            *(simpler_controller + 3 * min_tran_point + 1)) {
          min_tran_point = tmp_trace_node;
        }
      }
      tmp_data_1 = *(simpler_controller + 3 * trace_node + 1);
      *(simpler_controller + 3 * trace_node + 1) =
          *(simpler_controller + 3 * min_tran_point + 1);
      *(simpler_controller + 3 * min_tran_point + 1) = tmp_data_1;
      tmp_data_1 = *(simpler_controller + 3 * trace_node + 2);
      *(simpler_controller + 3 * trace_node + 2) =
          *(simpler_controller + 3 * min_tran_point + 2);
      *(simpler_controller + 3 * min_tran_point + 2) = tmp_data_1;
      trace_node += 1;
    }
  }

  base_point = 0;
  number_of_transitions = 0;
  while (*(simpler_controller + 3 * base_point) != -1) {
    number_of_transitions += 1;
    if (*(simpler_controller + 3 * base_point + 1) >= 1000)
      number_of_transitions -= 1;
    base_point += 1;
  }

  /* output simsup.des */
  tmp_data = *(simpler_controller + 3 * base_point - 3) + 1;
  compress_ratio = ((float)num_states) / ((float)tmp_data);
  *cr = compress_ratio;

  out = fopen(name4, "wb");
  if (out == NULL)
    return 1;
  flag = Txt_DES(out, tmp_data);
  fclose(out);

  free(simpler_controller);
  simpler_controller = NULL;
  free(c_marked_states);
  c_marked_states = NULL;
  free(p_marked_states);
  p_marked_states = NULL;

  if (flag != 1) {
    return_code = 50;
    goto FREEMEM;
  }

  /* lower bound estimate */
  *lb = 0; // Refinement();
  // if(stack_flag == 100)
  //	return_code = 100;

FREEMEM:
  if (root_node != NULL) {
    for (i = 0; i < num_states; i++) {
      struct equivalent_state_set *temp1, *temp11;
      struct forbidden_event_set *temp2, *temp22;
      struct transitions *temp3, *temp33;

      temp1 = root_node[i].equal_set;
      while (temp1 != NULL) {
        temp11 = temp1->next_node;
        free(temp1);
        temp1 = temp11;
      }

      temp2 = root_node[i].forb_set;
      while (temp2 != NULL) {
        temp22 = temp2->next_event;
        free(temp2);
        temp2 = temp22;
      }

      temp3 = root_node[i].tran_set;
      while (temp3 != NULL) {
        temp33 = temp3->next_transition;
        free(temp3);
        temp3 = temp33;
      }
    }
  }
  free(root_node);
  free(controller_tree);
  free(tmpu_stack);
  free(controller);
  free(plant);
  free(record);

  return return_code;
}
// Original version
INT_OS supreduce(char *name1, char *name2, char *name3, char *name4, INT_S *lb,
                 float *cr, INT_B slb_flag) {
  INT_S min_exit, tmp_data, tmp_trace_node, trace_node, index, min_tran_point;
  INT_S base_point, number_of_transitions, tmp_data_1;
  float compress_ratio;
  INT_S trace_mark;
  INT_OS return_code = 0, flag;
  INT_S i;
  FILE *out;

  /* Initial all arrays to NULL */
  c_marked_states = NULL;
  p_marked_states = NULL;
  controller = NULL;
  plant = NULL;
  root_node = NULL;
  controller_tree = NULL;
  simpler_controller = NULL;
  tmpu_stack = NULL;
  record = NULL;

  flag = Get_DES(&tran_number, &num_states, 1, name1);
  if (flag != 0)
    return flag;
  if (num_states == 0)
    return -1;

  flag = Get_DES(&tran_number, &num_states, 0, name2);
  if (flag != 0)
    return flag + 10;
  if (num_states == 0)
    return -1;
  if (tran_number == 1) {
    return_code = -2;
    goto FREEMEM;
  }

  root_node = (struct node *)CALLOC(num_states, sizeof(struct node));
  controller_tree =
      (INT_S *)CALLOC((2 * tran_number + 15 * num_states + 1), sizeof(INT_S));
  tmpu_stack =
      (INT_S *)MALLOC((2 * num_states * num_states + 1) * sizeof(INT_S));
  record = (INT_S *)MALLOC((num_states + 1) * sizeof(INT_S));

  if ((root_node == NULL) || (controller_tree == NULL) ||
      (tmpu_stack == NULL) || (record == NULL)) {
    mem_result = 1;
    return_code = 30;
    goto FREEMEM;
  }

  /* generate controller_tree */
  Controller_Tree();
  free(controller);
  controller = NULL;
  /*if(stack_flag == 100){
          return_code = 100;
          goto FREEMEM;
  }*/

  /* generate combined_tree */
  flag = Combined_Tree();
  free(plant);
  plant = NULL;
  if (stack_flag == 100) {
    return_code = 100;
    goto FREEMEM;
  }
  if (flag == 1) {
    return_code = 40;
    goto FREEMEM;
  }

  /* generate root_node[] */
  Tree_Structure_Conversion(name3);
  free(controller_tree);
  controller_tree = NULL;

  Reduction_Faster(root_node, num_states, tran_number);
  if (stack_flag == 100) {
    return_code = 100;
    goto FREEMEM;
  }

  if (mem_result == 1)
    goto FREEMEM;

  /* refine simpler_controller to generate the final text version transition
   * structure of the reduced supervisor */
  base_point = 0;
  while (*(simpler_controller + 3 * base_point) != -1) {
    min_exit = base_point;
    trace_node = base_point + 1;
    while (*(simpler_controller + 3 * trace_node) != -1) {
      if (*(simpler_controller + 3 * trace_node) <
          *(simpler_controller + 3 * min_exit))
        min_exit = trace_node;
      trace_node += 1;
    }
    tmp_data = *(simpler_controller + 3 * base_point);
    *(simpler_controller + 3 * base_point) =
        *(simpler_controller + 3 * min_exit);
    *(simpler_controller + 3 * min_exit) = tmp_data;
    tmp_data = *(simpler_controller + 3 * base_point + 1);
    *(simpler_controller + 3 * base_point + 1) =
        *(simpler_controller + 3 * min_exit + 1);
    *(simpler_controller + 3 * min_exit + 1) = tmp_data;
    tmp_data = *(simpler_controller + 3 * base_point + 2);
    *(simpler_controller + 3 * base_point + 2) =
        *(simpler_controller + 3 * min_exit + 2);
    *(simpler_controller + 3 * min_exit + 2) = tmp_data;
    base_point += 1;
  }
  base_point = 0;
  while (*(simpler_controller + 3 * base_point) != -1) {
    trace_node = base_point + 1;
    while (*(simpler_controller + 3 * trace_node) != -1) {
      if (*(simpler_controller + 3 * trace_node) >
          *(simpler_controller + 3 * base_point))
        break;
      else {
        if ((*(simpler_controller + 3 * trace_node + 1) ==
             *(simpler_controller + 3 * base_point + 1)) &&
            (*(simpler_controller + 3 * trace_node + 2) ==
             *(simpler_controller + 3 * base_point + 2))) {
          tmp_trace_node = trace_node + 1;
          while (*(simpler_controller + 3 * tmp_trace_node) != -1) {
            *(simpler_controller + 3 * tmp_trace_node - 3) =
                *(simpler_controller + 3 * tmp_trace_node);
            *(simpler_controller + 3 * tmp_trace_node - 2) =
                *(simpler_controller + 3 * tmp_trace_node + 1);
            *(simpler_controller + 3 * tmp_trace_node - 1) =
                *(simpler_controller + 3 * tmp_trace_node + 2);
            tmp_trace_node += 1;
          }
          *(simpler_controller + 3 * tmp_trace_node - 3) = -1;
          trace_node -= 1;
        }
      }
      trace_node += 1;
    }
    base_point += 1;
  }
  base_point = 0;
  index = 1;
  tmp_data = 0;
  while (*(simpler_controller + 3 * base_point) != -1) {
    while (*(simpler_controller + 3 * base_point) > tmp_data) {
      if (*(simpler_controller + 3 * base_point) == -1)
        break;
      tmp_data = *(simpler_controller + 3 * base_point);
      trace_mark = 0;
      while (*(c_marked_states + trace_mark) != -1) {
        if (*(c_marked_states + trace_mark) == tmp_data)
          *(c_marked_states + trace_mark) = index;
        trace_mark += 1;
      }
      while ((*(simpler_controller + 3 * base_point) == tmp_data) &&
             (*(simpler_controller + 3 * base_point) != -1)) {
        *(simpler_controller + 3 * base_point) = index;
        base_point += 1;
      }
      tmp_trace_node = 0;
      while (*(simpler_controller + 3 * tmp_trace_node) != -1) {
        if (*(simpler_controller + 3 * tmp_trace_node + 2) == tmp_data)
          *(simpler_controller + 3 * tmp_trace_node + 2) = index;
        tmp_trace_node += 1;
      }
      index += 1;
    }
    if (*(simpler_controller + 3 * base_point) != -1)
      base_point += 1;
  }
  base_point = 0;
  while (*(simpler_controller + 3 * base_point) != -1) {
    tmp_data = *(simpler_controller + 3 * base_point);
    trace_node = base_point;
    while (*(simpler_controller + 3 * base_point) == tmp_data)
      base_point += 1;
    while (trace_node < base_point) {
      min_tran_point = trace_node;
      for (tmp_trace_node = trace_node; tmp_trace_node < base_point;
           tmp_trace_node++) {
        if (*(simpler_controller + 3 * tmp_trace_node + 1) <
            *(simpler_controller + 3 * min_tran_point + 1)) {
          min_tran_point = tmp_trace_node;
        }
      }
      tmp_data_1 = *(simpler_controller + 3 * trace_node + 1);
      *(simpler_controller + 3 * trace_node + 1) =
          *(simpler_controller + 3 * min_tran_point + 1);
      *(simpler_controller + 3 * min_tran_point + 1) = tmp_data_1;
      tmp_data_1 = *(simpler_controller + 3 * trace_node + 2);
      *(simpler_controller + 3 * trace_node + 2) =
          *(simpler_controller + 3 * min_tran_point + 2);
      *(simpler_controller + 3 * min_tran_point + 2) = tmp_data_1;
      trace_node += 1;
    }
  }

  base_point = 0;
  number_of_transitions = 0;
  while (*(simpler_controller + 3 * base_point) != -1) {
    number_of_transitions += 1;
    if (*(simpler_controller + 3 * base_point + 1) >= 1000)
      number_of_transitions -= 1;
    base_point += 1;
  }

  /* output simsup.des */
  tmp_data = *(simpler_controller + 3 * base_point - 3) + 1;
  compress_ratio = ((float)num_states) / ((float)tmp_data);
  *cr = compress_ratio;

  out = fopen(name4, "wb");
  if (out == NULL)
    return 1;
  flag = Txt_DES(out, tmp_data);
  fclose(out);

  free(simpler_controller);
  simpler_controller = NULL;
  free(c_marked_states);
  c_marked_states = NULL;
  free(p_marked_states);
  p_marked_states = NULL;

  if (flag != 1) {
    return_code = 50;
    goto FREEMEM;
  }

  /* lower bound estimate */
  if (slb_flag == true)
    *lb = Refinement();
  else
    *lb = 0;
  if (stack_flag == 100)
    return_code = 100;

FREEMEM:
  if (root_node != NULL) {
    for (i = 0; i < num_states; i++) {
      struct equivalent_state_set *temp1, *temp11;
      struct forbidden_event_set *temp2, *temp22;
      struct transitions *temp3, *temp33;

      temp1 = root_node[i].equal_set;
      while (temp1 != NULL) {
        temp11 = temp1->next_node;
        free(temp1);
        temp1 = temp11;
      }

      temp2 = root_node[i].forb_set;
      while (temp2 != NULL) {
        temp22 = temp2->next_event;
        free(temp2);
        temp2 = temp22;
      }

      temp3 = root_node[i].tran_set;
      while (temp3 != NULL) {
        temp33 = temp3->next_transition;
        free(temp3);
        temp3 = temp33;
      }
    }
  }
  free(root_node);
  free(controller_tree);
  free(tmpu_stack);
  free(controller);
  free(plant);
  free(record);

  return return_code;
}

void clean_selfloop_orig(INT_S s, state_node *t, INT_S s1, state_node *t1) {
  INT_T s_list, s_list1, *list, *list1;
  INT_S i, j;
  INT_B ok;
  INT_S ss;
  INT_T ee;
  state_node *tp;
  INT_S stp;

  s_list = s_list1 = 0;
  list = list1 = NULL;
  stp = 0;
  tp = NULL;

  export_copy_des(&stp, &tp, s, t);

  for (i = 0; i < s; i++) {
    for (j = 0; j < t[i].numelts; j++) {
      if (t[i].next[j].data2 != i && t[i].next[j].data1 % 2 == 0) {
        addordlist(t[i].next[j].data1, &list, s_list, &ok);
        if (ok)
          s_list++;
      }
    }
  }
  for (i = 0; i < stp; i++) {
    for (j = 0; j < tp[i].numelts; j++) {
      ee = tp[i].next[j].data1;
      ss = tp[i].next[j].data2;
      if (ss == i && ee % 2 == 0) {
        if (!inlist(ee, list, s_list)) {
          delete_ordlist1(ee, ss, &t[i].next, t[i].numelts, &ok);
          if (ok)
            t[i].numelts--;
        }
      }
    }
  }
  freedes(stp, &tp);
  stp = 0;
  tp = NULL;
  free(list);
  list = NULL;
  s_list = 0;

  export_copy_des(&stp, &tp, s, t);

  for (j = 0; j < t[0].numelts; j++) {
    if (t[0].next[j].data2 == 0 && t[0].next[j].data1 % 2 == 1) {
      addordlist(t[0].next[j].data1, &list, s_list, &ok);
      if (ok)
        s_list++;
    }
  }

  for (i = 1; i < s; i++) {
    for (j = 0; j < t[i].numelts; j++) {
      ee = t[i].next[j].data1;
      ss = t[i].next[j].data2;
      if (ss == i && ee % 2 == 1) {
        if (inlist(ee, list, s_list)) {
          addordlist(ee, &list1, s_list1, &ok);
          if (ok)
            s_list1++;
        }
      }
    }
    s_list = s_list1;
    list = (INT_T *)REALLOC(list, s_list * sizeof(INT_T));
    for (j = 0; j < s_list1; j++)
      list[j] = list1[j];

    free(list1);
    list1 = NULL;
    s_list1 = 0;
  }
  for (i = 0; i < stp; i++) {
    for (j = 0; j < tp[i].numelts; j++) {
      ee = tp[i].next[j].data1;
      if (inlist(ee, list, s_list)) {
        delete_ordlist1(ee, i, &t[i].next, t[i].numelts, &ok);
        if (ok)
          t[i].numelts--;
      }
    }
  }

  freedes(stp, &tp);
  free(list);
}
void clean_selfloop(INT_S s, state_node *t, INT_S s1, state_node *t1) {
  INT_T s_list, s_list1, *list, *list1;
  INT_S i, j;
  INT_B ok;
  INT_S ss;
  INT_T ee;
  state_node *tp;
  INT_S stp;

  s_list = s_list1 = 0;
  list = list1 = NULL;
  stp = 0;
  tp = NULL;

  // Compute the set of controllable events to be disabled
  for (i = 0; i < s1; i++) {
    for (j = 0; j < t1[i].numelts; j++) {
      addordlist(t1[i].next[j].data1, &list1, s_list1, &ok);
      if (ok)
        s_list1++;
    }
  }
  // Compute the set of events that cause state changes
  for (i = 0; i < s; i++) {
    for (j = 0; j < t[i].numelts; j++) {
      if (t[i].next[j].data2 != i) {
        addordlist(t[i].next[j].data1, &list, s_list, &ok);
        if (ok)
          s_list++;
      }
    }
  }
  // Selfloops of these two kinds of events will not be removed.
  // To simplify the computation, put them into one set
  for (i = 0; i < s_list1; i++) {
    addordlist(list1[i], &list, s_list, &ok);
    if (ok)
      s_list++;
  }

  export_copy_des(&stp, &tp, s, t);

  for (i = 0; i < stp; i++) {
    for (j = 0; j < tp[i].numelts; j++) {
      ee = tp[i].next[j].data1;
      ss = tp[i].next[j].data2;
      if (ss == i && (!inlist(ee, list, s_list))) {
        delete_ordlist1(ee, ss, &t[i].next, t[i].numelts, &ok);
        if (ok)
          t[i].numelts--;
      }
    }
  }
  freedes(stp, &tp);
  stp = 0;
  tp = NULL;
  free(list);
  list = NULL;
  s_list = 0;

  freedes(stp, &tp);
  // free(list);
}

#ifdef __cplusplus
}
#endif
