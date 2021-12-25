#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <ctype.h>
// #include <io.h>

#include "obs_check.h"
#include "setup.h"
#include "des_data.h"
#include "des_proc.h"
#include "des_supp.h"
// #include "curses.h"
#include "mymalloc.h"
#include "minm1.h"
// #include "ext_des_proc.h"
#include "cnorm.h"
#include "higen.h"
// #include "cl_tct.h"
#include "program.h"
#include "tct_io.h"


typedef struct item_node{     
     INT_S data1;
     INT_S data2;
     INT_T numelts1;
     INT_T numelts2;
     INT_B  *next1;
     INT_B  *next2;
}item_node;
typedef struct obs_table{
	 INT_S state;
	 INT_S Gstate;
     INT_S numelts;
     item_node *next;
}obs_table;

void print_table(INT_S s_table, obs_table *table, INT_T slist, INT_T *list, INT_S iteration)
{
     FILE *f1;
     item_node *node;
     INT_S i, j, k;
	 char tmp_result[_MAX_PATH];

	 strcpy(tmp_result, "");
	 //strcat(tmp_result, prefix);
	 //strcat(tmp_result, "Table.txt");
	 sprintf(tmp_result,"%sTable%d.txt", prefix, iteration);

     f1 = fopen(tmp_result, "w");

	 fprintf(f1, "  Event   ");

	 for(i = 0; i < slist; i ++){
		 fprintf(f1,"  %3d   ", list[i]);
	 }
	 fprintf(f1, "\n");
     for(i = 0; i < s_table; i ++){
        node = table[i].next;
        for(j = 0; j < table[i].numelts; j ++){
            fprintf(f1,"%3d %3d:  ", node[j].data1, node[j].data2);
            for( k = 0; k < node[j].numelts1; k ++){
                fprintf(f1, "%3d %3d,", node[j].next1[k], node[j].next2[k]);
            }
            fprintf(f1,"\n");
        }
        fprintf(f1, "\n\n");
     }
     fclose(f1);
     
}
void obs_free_par(INT_S *s_par, part_node** par)
{
   INT_S i;
     
   for (i=0; i < *s_par; i++)
      free((*par)[i].next);
   free(*par); 
   *par = NULL;
   *s_par = 0;   
}     
void free_table(INT_S s_table, obs_table **table)
{
     INT_S i,j;
     for(i = 0; i < s_table; i ++){
        for(j = 0; j < (*table)[i].numelts; j ++){
           free((*table)[i].next[j].next1);
           free((*table)[i].next[j].next2);
        }
        free((*table)[i].next);
     }     
     free(*table); *table = NULL;
}

void obs_meet(INT_S s1, state_node *t1, INT_S s2, state_node*t2, INT_S * s3, state_node **t3, INT_S *s_macro_c, INT_S **macro_c)
{
     INT_S t1i, t2j;
     INT_T colptr1, colptr2;
     INT_T tran1, tran2;
     INT_S srcstate, newstate, macrostate;
     INT_S a,b,i;
     INT_S *macro_ab;

#if defined(_x64_)
	 unsigned long long *macro64_c;
	 macro64_c = NULL;

	 meet_x64(s1,t1,s2,t2,s3,t3,&macro64_c); 
	 *s_macro_c = *s3;
	 *macro_c = (INT_S*)MALLOC(*s_macro_c * sizeof(INT_S));
	 for(i = 0; i < *s_macro_c; i ++){
		 a = (INT_OS)macro64_c[i];
		 b = (INT_OS)(macro64_c[i]>>32);
		 (*macro_c)[i] = b * s1 + a;
	 }
	 free(macro64_c); macro64_c = NULL;
	 return;
#endif
     
     if (s1 == 0L || s2 == 0L) {
        *s3 = 0;
        *t3 = NULL;
        return;
     }
     *s3 = 0; *t3 = NULL;
     macro_ab = (INT_S*)MALLOC(sizeof(INT_S) * s1 * s2);
     *macro_c  = (INT_S*)MALLOC(sizeof(INT_S) * s1 * s2);
     if((macro_ab == NULL) || *macro_c == NULL){
        mem_result = 1;
        return;
     }
     
     for(i = 0; i < s1 * s2; i ++){
        macro_ab[i] = -1L;
        (*macro_c)[i]  = -1L;
     }
     macro_ab[0] = (*macro_c)[0] = 0L;
     srcstate = newstate = macrostate = 0L;
     t1i = t2j = 0L;
     do {
        colptr1 = 0;
        colptr2 = 0;
        while(colptr1 < t1[t1i].numelts && colptr2 < t2[t2j].numelts){
           tran1 = t1[t1i].next[colptr1].data1;
           tran2 = t2[t2j].next[colptr2].data1;
           if(tran1 != tran2){
              if(tran1 < tran2)
                 colptr1 ++;
              else
                 colptr2 ++;
              continue;
           }
           a = t1[t1i].next[colptr1].data2;
           b = t2[t2j].next[colptr2].data2;
           macrostate = (macro_ab)[b * s1 + a];
           if(macrostate == -1L){
              newstate ++;
              macro_ab[b * s1 + a] = newstate;
              (*macro_c)[newstate] = b * s1 + a;
              insertlist4(srcstate, tran1, newstate, s3, t3);
              if(mem_result == 1) goto MLABEL;
           } else{
              insertlist4(srcstate, tran1, macrostate, s3, t3);
              if(mem_result == 1) goto MLABEL;
           }
           colptr1 ++;
           colptr2 ++;
        }    
        srcstate ++;
        a = (*macro_c)[srcstate];
        if(a != -1L){
           t1i = a % s1;
           t2j = a / s1;
        }
     }while (srcstate <= newstate);
     resize_des(t3, *s3, newstate + 1);
     *s3 = newstate + 1;
     (*t3)[0].reached = true;
     for(i = 0; i < *s3; i ++){
        a = (*macro_c)[i];
        (*t3)[i].marked = t1[a%s1].marked & t2[a/s1].marked;
     }     
     /*resize the macro_c because the size is smaller than befroe*/
     *s_macro_c = *s3;
     *macro_c = (INT_S*)REALLOC(*macro_c, sizeof(INT_S) * (*s3));

MLABEL:
     free(macro_ab);

}
void obs_recodelist(INT_S state,
                state_node *t1,
                INT_T *nullist,
                INT_T s_nullist,
                INT_B  *nullstate_bool)
{
   /* Convert the transition labels to be projected to 1023.
      Cannot use -1 because it is an unsigned integer.
      Need better method to mark transitions to be converted. */

   INT_T cur, elem;

   *nullstate_bool = false;
   for (cur=0; cur < t1[state].numelts; cur++) {
      elem = t1[state].next[cur].data1;
      if (inlist(elem, nullist, s_nullist)) {
          t1[state].next[cur].data1 = 1023;       /* hard code for now */
          *nullstate_bool = true;
      }
   }
}
void obs_reorderlist(tran_node* next,
                 INT_T numelts,
                 tran_node **orderlist,
                 INT_T *s_orderlist)
{
   INT_B  ok;
   INT_T i;

   *s_orderlist = 0;
   for (i=0; i < numelts; i++) {
      addordlist1(next[i].data1, next[i].data2, orderlist, *s_orderlist, &ok);
      if (ok) (*s_orderlist)++;
   }
}
void obs_addpart(INT_S e,
             INT_S **L,
             INT_S size,
             INT_B  *ok)
{
   INT_S pos;
   INT_S lower, upper;
   INT_B  found;

   *ok = false;

   /* Do a binary search. */
   found = false;
   pos = 0;
   if (size > 1) {
     lower = 1;
     upper = size;
     while ( (found == false) && (lower <= upper) ) {
       pos = (lower + upper) / 2;
       if (e == (*L)[pos-1]) {
          found = true;
       } else if (e > (*L)[pos-1]) {
          lower = pos+1;
       } else {
          upper = pos-1;
       }
     }

     if (found == false) {
        if (e < (*L)[pos-1])
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
   *L = (INT_S*) REALLOC(*L, sizeof(INT_S)*(size+1));
   if (*L == NULL) {
      mem_result = 1;
      *ok = false;
      return;
   }

   /* Move over any elements down the list */
   if ((size-pos) > 0)
      memmove(&(*L)[pos+1], &(*L)[pos], sizeof(INT_S)*(size-pos));

   /* Insert the element into the list */
   (*L)[pos]= e;

   *ok = true;
}

void obs_build_nullsets(INT_S state,
                    state_node *t1,
                    part_node *nullsets)
{
   INT_B  ok;
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
      while ( (cur < t1[s].numelts) && (t1[s].next[cur].data1 != 1023) ) {
         cur++;
      }

      while ( (cur < t1[s].numelts) && (t1[s].next[cur].data1 == 1023) ) {
         entr2 = t1[s].next[cur].data2;
         obs_addpart(entr2, &nullsets[state].next, nullsets[state].numelts, &ok);
         if (ok) nullsets[state].numelts++;
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
      if (cur < t1[s].numelts && ok) goto LABEL2;
   } while (!pstack_IsEmpty(&ts));

   pstack_Done(&ts);
}
void obs_mark_newstate(INT_S* next,
                   INT_S numelts,
                   state_node *t1,
                   INT_B  *marked)
{
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

void obs_generate(INT_S *next,
              INT_S numelts,
              state_node **t1,
              state_map **templist,
              INT_S *s_templist)
{
   INT_T i,j;
   INT_T event;
   INT_S entr;
   INT_B  ok;

   for (i=0; i < numelts; i++) {
      for (j=0; j < (*t1)[next[i]].numelts; j++) {
         event = (*t1)[next[i]].next[j].data1;
         entr  = (*t1)[next[i]].next[j].data2;
         if (event != 1023) {
            addstatemap(event,entr,templist,*s_templist,&ok);
            if (ok) (*s_templist)++;
         }
      }
   }
}

void obs_unionsets(INT_S *list1,
               INT_S size1,
               INT_S **list2,
               INT_S *size2)
{
   /* Form the union: list2 <- list1 + list2 */
   INT_S cur;
   INT_B  ok;

   for (cur=0; cur < size1; cur++) {
      addstatelist(list1[cur],list2,*size2,&ok);
      if (ok) (*size2)++;
   }
}

INT_B  obs_equal_list(INT_S *list1,
                   INT_S *list2,
                   INT_S numelts)
{
   INT_S i;

   for (i=0; i < numelts; i++) {
      if (list1[i] != list2[i])
         return false;
   }
   return true;
}

void obs_memberset(INT_S *tempset,
               INT_S numelts,
               state_map *macrosets,
               INT_S size,
               INT_S *macrostate)
{
    INT_B  found;
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
              if (obs_equal_list(tempset,macrosets[i].next,numelts)) {
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

void obs_addordlist3(INT_S newstate,
                 INT_S setsize,
                 INT_B  marked,
                 INT_S *tempset,
                 state_map **macrosets,
                 INT_S *s_macrosets)
{
   INT_B  ok;
   INT_S i;

   for (i=0; i < setsize; i++) {
     addstatemap(newstate, tempset[i], macrosets, *s_macrosets, &ok);
     if (ok) (*s_macrosets)++;
   }

   for (i=0; i < *s_macrosets; i++) {
     if ( (*macrosets)[i].state == newstate ) {
       (*macrosets)[i].marked = marked;
       break;
     }
   }
}

void obs_project(INT_S *s, state_node **t, INT_T s_nullist, INT_T *nullist, INT_S * s_macrosets, state_map ** macrosets)
{
     INT_S state, i, j, k;
     INT_B  nullstate_bool;
     INT_S *nullstates_list, s_nullstates_list;
     tran_node *orderlist; INT_T s_orderlist;
     part_node * nullsets;
     INT_B  ok, found, marked;
     INT_S s2; state_node *t2;
     state_map *templist; INT_S s_templist;
     INT_S *tempset, setsize;
     INT_S macrostate;
     INT_S srcstate, newstate, curr_row, macroptr;
     
     nullstates_list = tempset = NULL;
     s_nullstates_list =  setsize = 0;
     orderlist = NULL; s_orderlist = 0;
     nullsets = NULL;
     templist = NULL; s_templist = 0;
     
     for(state = 0; state < *s; state ++){
        nullstate_bool = false;
        obs_recodelist(state, *t, nullist, s_nullist, &nullstate_bool);
        if(nullstate_bool){
           addstatelist(state, &nullstates_list, s_nullstates_list, &ok);
           if(ok) s_nullstates_list ++;
           obs_reorderlist((*t)[state].next, (*t)[state].numelts, &orderlist, &s_orderlist);
           free((*t)[state].next);
           (*t)[state].next = orderlist;
           (*t)[state].numelts = s_orderlist;
           s_orderlist = 0;
           orderlist = NULL;
        }
        (*t)[state].reached = false;
     }
     if(s_nullstates_list == 0)  
        goto PROJ_LABEL;
     nullsets = (part_node*)CALLOC(*s, sizeof(part_node));
     if(nullsets == NULL){
        mem_result = 1;
        goto PROJ_LABEL;
     }
     
     for(state = 0; state < *s; state ++){
        nullsets[state].numelts = 1;
        nullsets[state].next = (INT_S*)MALLOC(sizeof(INT_S));
        if(nullsets[state].next == NULL){
           mem_result = 1;
           goto PROJ_LABEL;
        }
        nullsets[state].next[0] = state;
     }
    // print_list(s_nullstates_list,nullstates_list);

     for(i = 0; i < s_nullstates_list; i ++){
        state = nullstates_list[i];
        obs_build_nullsets(state, *t, nullsets);
        for(state = 0; state < *s; state ++)
           (*t)[state].reached = false;
     }
     //print_par(*s,nullsets);
     *s_macrosets = 1;
     *macrosets = (state_map*)MALLOC(sizeof(state_map));
     if(*macrosets == NULL){
         mem_result = 1;
         goto PROJ_LABEL;
     }
     (*macrosets)[0].state = 0;
     (*macrosets)[0].numelts = 0;
     obs_mark_newstate(nullsets[0].next, nullsets[0].numelts,*t,&marked);
     (*macrosets)[0].marked = marked;
     (*macrosets)[0].next = NULL;
     
     /*Copy nullsets[0].next to macrosets[0].next*/
     for(i = 0; i < nullsets[0].numelts; i ++){
        addstatelist(nullsets[0].next[i], &(*macrosets)[0].next, (*macrosets)[0].numelts, &ok);
        if(ok)  (*macrosets)[0].numelts ++;
     }
     
     obs_generate(nullsets[0].next, nullsets[0].numelts, t, &templist, &s_templist);
     if(templist == NULL){
        /*Free all pointer*/
        s2 = 1;
        t2 = newdes(s2);
        t2[0].marked = (*macrosets)[0].marked;
        freedes(*s,t);
        *s = s2;
        *t = t2;
        s2 = 0; t2 = NULL;
        goto PROJ_LABEL;
     }
     
     srcstate = 0;
     newstate = 0;
     s2 = 0; t2 = NULL;
     
     do{
        for(i = 0; i < s_templist; i ++){
           curr_row = templist[i].state;
           for(j = 0; j < templist[i].numelts; j ++){
              k = templist[i].next[j];
              obs_unionsets(nullsets[k].next, nullsets[k].numelts, &tempset, &setsize);
           }
           obs_memberset(tempset, setsize, *macrosets, *s_macrosets, &macrostate);
           if(macrostate == -1){
              newstate ++;
              obs_mark_newstate(tempset, setsize, *t, &marked);
              obs_addordlist3(newstate, setsize, marked, tempset, macrosets, s_macrosets);
              insertlist4(srcstate, (INT_T)curr_row, newstate, &s2, &t2);
           }else{
              if(macrostate != -2){
                 insertlist4(srcstate, (INT_T)curr_row, macrostate, &s2, &t2);
              }
           }
           if(tempset != NULL){
              free(tempset); tempset = NULL;
           }
           setsize = 0;
        }
        srcstate ++;
        
        found = false;
        for(i = 0; i < *s_macrosets; i ++){
           if((*macrosets)[i].state == srcstate){
              found = true;
              macroptr = i;
              break;
           }
        }
        if(found){
           if(s_templist > 0){
              free(templist); templist = NULL;
              s_templist = 0;
           }
           obs_generate((*macrosets)[macroptr].next, (*macrosets)[macroptr].numelts, t, &templist, &s_templist);
        }
     } while(srcstate <= newstate); 
     
     resize_des(&t2, s2, newstate + 1);
     s2 = newstate + 1;
     
     for(i = 0; i < *s_macrosets; i ++){
        t2[(*macrosets)[i].state].marked = (*macrosets)[i].marked;
     }
     freedes(*s,t);
     *s = s2;
     *t = t2;
     
PROJ_LABEL:
     free(nullstates_list);
     free(orderlist);
     free(nullsets);
     free(templist);
     free(tempset);
}
INT_S obs_getindex(INT_T e, INT_T *L, INT_T size)
{
     INT_S k;
     INT_S pos;
     INT_S lower, upper;
     INT_B  found;
   k = 0;
   /* Do a binary search. */
   found = false;
   pos = 0;
   if (size > 1) {
     lower = 1;
     upper = size;
     while ( (found == false) && (lower <= upper) ) {
       pos = (lower + upper) / 2;
       if (e == L[pos-1]) {
          found = true;
          k = pos - 1;
       } else if (e > L[pos-1]) {
          lower = pos+1;
       } else {
          upper = pos-1;
       }
     }

   } else if (size == 1) {
     if (e == L[0]) {
       found = true;
       k = 0;
     }
   }
   if(found = false) k = -1;

   return k;
}

INT_OS obs_proc(char*name1, char* name2, INT_S *s, state_node **t, INT_T s_nulllist, INT_T *nulllist, 
                         INT_T s_imagelist, INT_T* imagelist, INT_OS mode, INT_B  *is_observable)
{
     state_node *t1, *t2, *t3, *t4, *t5;
     INT_S s1, s2, s3, s4, s5, init;
     INT_S s_macro_c, s_macro_b, * macro_c, * macro_b;     
     INT_S s_macrosets, s_macrosets1;
     state_map * macrosets, *macrosets1;
     part_node * par1, *par2;
     INT_S s_par1, s_par2;
     INT_S i, j, k;
     INT_T s_list, *list;
     INT_T event, found;
     INT_B  ok;
     INT_S s_table; obs_table *table;
     INT_S state1, state2;
//     INT_B  *item1, *item2;
     item_node *inode;
     INT_S * mapstate;
//     FILE * f1;
     INT_OS result;
     
     t1 = t2 = t3 = t4 = t5 = NULL;
     s1 = s2 = s3 = s4 = s5 = 0;
     s_macro_c = s_macro_b = 0; macro_c = macro_b = NULL;
     s_macrosets = s_macrosets1 = 0; macrosets = macrosets1 = NULL;
     par1 = par2 = NULL;
     s_par1 = s_par2 = 0;
     s_table = 0; table = NULL;
     result = 0;
	 mapstate = NULL;
	 list = NULL;
     
     /*step1 : get des1 and des2*/
     init = 0L;
     if(getdes(name1, &s1, &init, &t1) == false)
         return -1;
     init = 0L;    
     if(getdes(name2, &s2, &init, &t2) == false)
         return -1;
      
   
     /*Copy s2 to s4*/
	 export_copy_des(&s4, &t4, s2, t2);
         
     /*step2 : des3 = meet(des1, des2)*/     
     obs_meet(s1, t1, s2, t2, &s3, &t3,&s_macro_c, &macro_c);
     //filedes("TS",s3,0,t3);
     //print_list(s3,macro_c);
     if(s3 == 0 && t3 == NULL){
		goto OBS_LABEL;
	 }
     if(mode == 1){             
        /*step3 : project des3 and get the partition (Ps = ps')*/
        obs_project(&s3, &t3, s_nulllist, nulllist, &s_macrosets, &macrosets);    

     }
     else{
        /*Step3 : project des3 and get the partition (Ps = Ps') (mod P(DES2))*/        
        obs_project(&s4, &t4, s_nulllist, nulllist, &s_macrosets1, &macrosets1);  
     
        mapstate = (INT_S*) CALLOC(s4, sizeof(INT_S));
        if (mapstate == NULL) {
           mem_result = 1;
           goto OBS_LABEL;
        }
        build_partition(s4,t4,mapstate,&state2);
        
        k = 0;
        for(i = 0; i < s4; i ++){
           if(k <= mapstate[i]) 
              k = mapstate[i];           
        }
        s_par1 = k + 1;
        par1 = (part_node*)CALLOC(s_par1, sizeof(part_node));
        if(par1 == NULL){
           mem_result = 1;
           goto OBS_LABEL;
        } 
         
        for(i = 0; i < s_macrosets1; i ++){
           k = mapstate[macrosets1[i].state];
           for( j = 0; j < macrosets1[i].numelts; j ++){
               addstatelist(macrosets1[i].next[j],&par1[k].next,par1[k].numelts,&ok);
               if (ok)  par1[k].numelts ++;
           }
        }                          
        s_macrosets = 1;
        macrosets = (state_map*)CALLOC(s_macrosets, sizeof(state_map));
        if(macrosets == NULL){
           mem_result = 1;
           goto OBS_LABEL;
        }
        for(i = 0; i < s_par1; i ++){
           for(j = 0; j < par1[i].numelts; j ++){
              state1 = par1[i].next[j];
              for(k = 0; k < s_macro_c; k ++){
                  if(state1 == macro_c[k]/s1){
                      addstatemap(i,k,&macrosets,s_macrosets,&ok);
                      if(ok) s_macrosets ++;
                  }
              }      

           }
        }        
     }
     free(macrosets1); s_macrosets1 = 0; macrosets1 = NULL;   
     obs_free_par(&s_par1, &par1); s_par1 = 0; par1 = NULL;


     /*step 4 :Compute the state subsets Q1 and Q2*/
     s_list = 0; list = NULL;
     for(i = 0; i < s_nulllist; i ++){
        addordlist(nulllist[i], &list, s_list, &ok);
        if(ok) s_list ++;
     }
     for(i = 0; i < s_imagelist; i ++){
        addordlist(imagelist[i], &list, s_list, &ok);
        if(ok) s_list ++;
     }     
     /*add m = 1001 to the list*/
     addordlist(1001, &list, s_list, &ok);
     if(ok) s_list ++;

     s_par1 = s_list;
     par1 = (part_node*)CALLOC(s_par1 , sizeof(part_node));
     if(par1 == NULL){
        mem_result = 1;
        goto OBS_LABEL;
     }    
     for(i = 0; i < s1; i ++){
        for(j = 0; j < t1[i].numelts; j ++){
           event = t1[i].next[j].data1;
           k = obs_getindex(event, list, s_list);
           addstatelist(i, &(par1[k].next), par1[k].numelts, &ok);
           if(ok) par1[k].numelts ++;   
        }
        if(t1[i].marked){           
           event = 1001;
           k = obs_getindex(event, list, s_list);
           addstatelist(i, &(par1[k].next), par1[k].numelts, &ok);
           if(ok) par1[k].numelts ++; 
        }
     }

     s_par2 = s_list;
     par2 = (part_node*)CALLOC(s_par2, sizeof(part_node));
     if(par2 == NULL){
        mem_result = 1;
        goto OBS_LABEL;
     }
     for(i = 0; i < s2; i ++){
        for(j = 0; j < t2[i].numelts; j ++){
           event = t2[i].next[j].data1;
           k = obs_getindex(event, list, s_list);
           if(k == -1){
              result = -1;
              goto OBS_LABEL;
           }
           addstatelist(i, &(par2[k].next), par2[k].numelts, &ok);
           if(ok) par2[k].numelts ++; 
        }
        if(t2[i].marked){
           event = 1001;
           k = obs_getindex(event, list, s_list);
           if(k == -1){
              result = -1;
              goto OBS_LABEL;
           }
           addstatelist(i, &(par2[k].next), par2[k].numelts, &ok);
           if(ok) par2[k].numelts ++; 
        }
     }
     /*step 5 :Construct the checking table*/
    /* s_table = 0;
     table = (obs_table*)CALLOC(s_table + 1, sizeof(obs_table));
     if(table == NULL){
        mem_result = 1;
        goto OBS_LABEL;
     }*/
     for(i = 0; i < s_macrosets; i ++){        
        if(macrosets[i].numelts > 1){           
           table = (obs_table*)REALLOC(table, (s_table + 1) * sizeof(obs_table));           
           if(table == NULL){
              mem_result = 1;
              goto OBS_LABEL;
           }
           table[s_table].numelts = 0; 
           table[s_table].next = NULL;                    
           for(j = 0; j < macrosets[i].numelts; j ++){
              k = macrosets[i].next[j];              
              state1 = macro_c[k] % s1;
              state2 = macro_c[k] / s1;                           
              table[s_table].numelts ++;                            
              table[s_table].next = (item_node*)REALLOC(table[s_table].next, table[s_table].numelts * sizeof(item_node));  
              if(table[s_table].next == NULL){
                 mem_result = 1;
                 goto OBS_LABEL;
              } 
              table[s_table].next[j].data1 = state1;
              table[s_table].next[j].data2 = state2;           
           }
           s_table ++;           
        }
     }   
     for( i = 0; i < s_table; i ++){
        inode = table[i].next;            
        for(j = 0; j < table[i].numelts; j ++){
           state1 = inode[j].data1;
           state2 = inode[j].data2;           
           inode[j].numelts1 = (INT_T)s_par1;
           inode[j].numelts2 = (INT_T)s_par2;   
           inode[j].next1 = NULL;        
           inode[j].next1 = (INT_B *)REALLOC(inode[j].next1, inode[j].numelts1 * sizeof(INT_B ));
           inode[j].next2 = NULL;
           inode[j].next2 = (INT_B *)REALLOC(inode[j].next2, inode[j].numelts2 * sizeof(INT_B ));
           if(inode[j].next1 == NULL || inode[j].next2 == NULL){
              mem_result = 1;
              goto OBS_LABEL;
           }
           if(s_par1 != s_par2 || s_par1 != s_list){
              result = -1;
              goto OBS_LABEL;
           }
                          
           for(k = 0; k < s_list; k ++){              
              if(instatelist(state1, par1[k].next, par1[k].numelts)){
                 inode[j].next1[k] = true; 
              }
              else inode[j].next1[k] = false;
              
              if(instatelist(state2, par2[k].next, par2[k].numelts)){
                 inode[j].next2[k] = true; 
              }
              else inode[j].next2[k] = false;                                                
           }           
        }
     }
   
     /*step 6 :Veryfy the observability according to the checking table*/

     //print_table(s_table,table);
     *s = s2;
     *t = newdes(*s);
     if(*t == NULL){
        mem_result = 1;
        goto OBS_LABEL;
     }   

     for(i = 0; i < s_table; i ++){
        inode = table[i].next;        
        for(k = 0; k < s_list; k ++){
           //if(list[k]%2 ==0 ){
              found = false;       
              for(j = 0; j < table[i].numelts; j ++){                       
                if((inode[j].next1[k] == true) && (inode[j].next2[k] == true)){
                   found = true;
                   break;
                }
             }           
             if(found){
                for(j = 0; j < table[i].numelts; j ++){              
                   if(inode[j].next1[k] == true){
                      if(inode[j].next2[k] == false){                    
                         *is_observable = false;
                         addordlist1(list[k],0, &(*t)[inode[j].data2].next, (*t)[inode[j].data2].numelts, &ok);
                         if(ok) (*t)[inode[j].data2].numelts ++;
                      }                    
                   }
                }
             }
           //}
        }
     }
OBS_LABEL:
     free(macro_c);
     free(macrosets);
     free(macrosets1);
     free(mapstate);
     free(list);
     freedes(s1, &t1);
     freedes(s2, &t2);
     freedes(s3, &t3);
     freedes(s4, &t4);
     freedes(s5, &t5);     
     obs_free_par(&s_par1, &par1);
     obs_free_par(&s_par2, &par2);
     free_table(s_table, &table);
     return result; 
}

INT_B  member_par(INT_S s_list, INT_S *list, INT_S s_par, part_node *par)
{
     INT_S i;
     for(i = 0; i < s_par; i ++){
        if(s_list == par[i].numelts){
           if(obs_equal_list(list, par[i].next, par[i].numelts)){
              return true;
           }
        }
     }
     return false;
}
INT_B  equal_member(state_map map, part_node *par )
{
     INT_S i, state1, state2;
     for(i = 0; i < map.numelts - 1; i ++){
        state1 = map.next[i];
        state2 = map.next[i + 1];
        if(par[state1].numelts != par[state2].numelts)
           return false;
        if(!obs_equal_list(par[state1].next, par[state2].next, par[state2].numelts))
           return false;
     }
     return true;
}

/* Delete transition,extrance state pair */
void DeleteOrdlist(INT_T e, tran_node **L, INT_T *size)
{
	INT_T pos;
	INT_T lower, upper;
	INT_B  found;

	/* Do a binary search. */
	do{
		found = false;
		pos = 0;
		if (*size > 1) {
			lower = 1;
			upper = *size;
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
		} else if (*size == 1) {
			if (e == (*L)[0].data1) {
				found = true;
			}
		}

		if(found == false)
			break;

		/* Move over any elements up the list */
		if ((*size-pos) > 0 && pos > 0)
			memmove(&(*L)[pos-1], &(*L)[pos], sizeof(tran_node)*(*size-pos));

		/* Remove space for element */
		*L = (tran_node*) REALLOC(*L, sizeof(tran_node)*(*size-1));
		if (*size > 1) {
			if (*L == NULL) {
				mem_result = 1;
				return;
			}
		} else {
			*L = NULL;
		}

		(*size) --;

	}while(true);
}

/* event in the transition, extrance state pair */
INT_B  GetExitState(INT_T e,  tran_node *L, INT_T size, INT_S * ss)
{
	INT_T pos;
	INT_T lower, upper;
	INT_B  found;

	/* Do a binary search. */
	found = false;
	pos = 0;
	if (size > 1) {
		lower = 1;
		upper = size;
		while ( (found == false) && (lower <= upper) ) {
			pos = (lower + upper) / 2;
			if (e == L[pos-1].data1) {
				found = true;
				*ss = L[pos-1].data2;
			} else if (e > L[pos-1].data1) {
				lower = pos+1;
			} else {
				upper = pos-1;
			}
		}
	} else if (size == 1) {
		if (e == L[0].data1) {
			found = true;
			*ss = L[0].data2;
		}
	}

	return found;                   
}      
INT_B inmaplist(INT_S s, INT_S *list, INT_S slist, INT_S s1)
{
	INT_S i;

	for(i = 0; i < slist; i ++){
		if(list[i]/s1 == s){
			return true;
		}
	}
	return false;
}
INT_B intablelist(INT_S s, item_node * next, INT_S numelts)
{
	INT_S i;

	for(i = 0; i < numelts; i ++){
		if(s == next[i].data1)
			return true;
	}
	return false;
}
INT_B intablelist1(INT_S s1, INT_S s2, item_node * next, INT_S numelts)
{
	INT_S i;

	for(i = 0; i < numelts; i ++){
		if((s1 == next[i].data1) && (s2 == next[i].data2))
			return true;
	}
	return false;
}
void remove_tran(INT_S s1, state_node **t1)
{
	INT_S i, j;
	INT_T event;
	INT_S state;
	INT_B ok;

	for(i = 0; i < s1; i ++){
		if(!(*t1)[i].reached){
			if((*t1)[i].next != NULL)
				free((*t1)[i].next);
			(*t1)[i].numelts = 0;
			(*t1)[i].next = NULL;
		}
		for(j = 0; j < (*t1)[i].numelts; j ++){
			event = (*t1)[i].next[j].data1;
			state = (*t1)[i].next[j].data2;
			if((!(*t1)[state].coreach) ||(!(*t1)[state].reached)){
				delete_ordlist1(event, state, &(*t1)[i].next, (*t1)[i].numelts, &ok);
				if(ok) (*t1)[i].numelts --;
			}
		}
	}
}
void new_trim1(INT_S *s1, state_node **t1)
{
	INT_S s2, state;
	//recode_node *recode_states;
	//INT_S num_reachable;

	if (*s1 <= 0) return;

	/* Pre-condition.  Assume states are unreachable first */
	for (state=0; state < *s1; state++)
		(*t1)[state].reached = false;

	(*t1)[0].reached = true;
	b_reach((*t1)[0].next, 0L, t1, *s1);
	coreach2(*s1, t1);

	s2 = 0;
	for (state = *s1-1L; state >= 0; state--) {
		if ((*t1)[state].reached && (*t1)[state].coreach)
			s2++;
		else
			(*t1)[state].reached = false;
	}
	if (s2 == *s1) {
		return;
	}

	/* Purge dead transitions followed by purging states */
	remove_tran(*s1, t1);

}
void obs_recode(INT_S s1, state_node *t1, INT_S s_macro_b, INT_S * macro_b, INT_B *ok)
{
   INT_T cur;
   INT_S s;
   INT_S es;
   t_queue tq;
   long num_hits = 0;
   INT_S *list, i;

   list = NULL;   
   *ok = true;

   /* Assume the "reached" field in state_node structure already
      set correctly.  In most cases, it means all false except,
      the zero state but not always. */

   queue_init(&tq);

   enqueue(&tq, 0);

   list = (INT_S*)CALLOC(s_macro_b, sizeof(INT_S));

   list[0] = macro_b[0];
   i = 0;

   while (!queue_empty(&tq)) {
      s = dequeue(&tq);
      for (cur=0; cur < t1[s].numelts; cur++) {
         es = t1[s].next[cur].data2;
         if (!t1[es].reached) {
            enqueue(&tq,es);
            t1[es].reached = true;
			i ++;
			list[i] = macro_b[es];

         }
      }
   }
   queue_done(&tq);
   if(s_macro_b != i + 1){
	   *ok = false;
	   free(list);
	   return;
   }
   for(i = 0; i < s_macro_b; i ++){
	   macro_b[i] = list[i];
   }
   free(list);
}
void full_function(INT_S *s1, state_node **t1, INT_T s_list, INT_T *list)
{
	INT_S i,j, dump;
	INT_B ok;

	//add a dump state
	(*t1) = (state_node*)REALLOC(*t1, ((*s1)+1)*sizeof(state_node));
	dump = *s1;
	(*t1)[dump].next = NULL;
	(*t1)[dump].numelts = 0;
	(*t1)[dump].marked = 0;
	(*s1) ++;

	for(i = 0; i < *s1; i ++){
		for(j = 0; j < s_list; j ++){
			if(!inordlist1(list[j],(*t1)[i].next, (*t1)[i].numelts)){
				addordlist1(list[j],dump, &(*t1)[i].next, (*t1)[i].numelts, &ok);
				if(ok) (*t1)[i].numelts ++;
			}
		}
	}
}
void catenation_sigma(INT_S *s1, state_node **t1, INT_T sigma)
{
	INT_S i, newstate; //j, jj, ;
	INT_B ok;//, newstate_flag;
	INT_S newevent;
	state_pair *sp; INT_S s_sp;
	state_node *t2; INT_S s2;
	INT_T *list, s_list;

	sp = NULL; s_sp = 0;
	t2 = NULL; s2 = 0;
	list = NULL; s_list = 0;
	//newstate_flag = false;

	//for(i = 0; i < *s1; i ++)
	//	(*t1)[i].reached = false;

	newstate = *s1;
	resize_des(t1, *s1, newstate + 1);
	*s1 = newstate + 1;
	newevent = 1002;

	for(i = 0; i < *s1; i ++){
		if((*t1)[i].marked){
			addordlist1((INT_T)newevent, newstate, &(*t1)[i].next, (*t1)[i].numelts, &ok);
			if(ok) (*t1)[i].numelts ++;	
			(*t1)[i].marked = false;
		}
	}

	(*t1)[newstate].marked = true;

	addstatepair(1002, sigma, &sp, s_sp, &ok);
	if(ok) s_sp ++;

	eventmap_des(*t1,*s1,&t2,&s2,sp,s_sp,&list,&s_list,&ok);
	if (s_list != 0) plain_project_proc(&s2,&t2,s_list,list);
	freedes(*s1, t1); *s1 = 0; *t1 = NULL;
	if(mem_result != 1)
	    export_copy_des(s1, t1, s2, t2);

	freedes(s2, &t2);
	free(sp);
}
void suprema_closed(INT_S *s1, state_node **t1)
{
	INT_S i, j, ss, ee;
	INT_S s2; state_node *t2;
	INT_B ok;

	s2 = 0; t2 = NULL;

	export_copy_des(&s2, &t2, *s1, *t1);
	for(i = 0; i < s2; i ++){
		if(t2[i].marked) {
			for(j = 0; j < t2[i].numelts; j ++){
				ee = t2[i].next[j].data1;
				ss = t2[i].next[j].data2;
				if(!t2[ss].marked){
					delete_ordlist1((INT_T)ee,ss, &(*t1)[i].next, (*t1)[i].numelts, &ok);
					if(ok) (*t1)[i].numelts --;
				}
			}
		}
	}

	trim1(s1, t1);

	freedes(s2, &t2);
}
// In this function, we consider the observation consistency of all event
INT_OS supobs_proc1(char * name3, char* name1, char* name2, INT_T s_nulllist, INT_T *nulllist, INT_T s_imagelist, INT_T* imagelist)
{
	state_node *t1, *t2, *t3, *t4, *t6, *t7, *t8;
	INT_S s1, s2, s3, s4, s6, s7, s8, init;
	INT_S s_macro_c, s_macro_b, s_macro_ab, * macro_c, * macro_b, s_macro_d, *macro_d, *macro_ab;     
	INT_S s_macrosets, s_macrosets1;
	state_map * macrosets, *macrosets1;
	part_node * par1, *par2;
	INT_S s_par1, s_par2;
	INT_S i, j, k;//, l;
	INT_T s_list, s_tmplist, *list, *tmplist;
	INT_T event, found;
	INT_B  ok, bObs, bDel;
	INT_S s_table, iteration, Tpos; obs_table *table;
	INT_S state1, state2;//, state;//, ss;//, l1, l2;//, nstate1;//, nstate2;

	//     INT_B  *item1, *item2;
	item_node *inode;
	INT_S * mapstate;
	//     FILE * f1;
	INT_OS result;

	t1 = t2 = t3 = t4 = t6 = t7 = t8 = NULL;
	s1 = s2 = s3 = s4 = s6 = s7 = s8 = 0;
	s_macro_c = s_macro_b = s_macro_d = s_macro_ab = 0; macro_c = macro_b = macro_d = macro_ab = NULL;
	s_macrosets = s_macrosets1 = 0; macrosets = macrosets1 = NULL;
	par1 = par2 = NULL;
	s_par1 = s_par2 = 0;
	s_table = 0; table = NULL;
	mapstate = NULL;
	bObs = false;
	result = 0;

	iteration = 0;
	Tpos = -1;

	/*step1 : get des1 and des2*/
	init = 0L;
	if(getdes(name1, &s1, &init, &t1) == false)
		return -1;
	init = 0L;    
	if(getdes(name2, &s4, &init, &t4) == false)
		return -1;

	//Compute canonical form of G with respect to Nerode equivalence 
	//minimize(&s1, &t1);

	//Compute Normal form of NK   NK = K || PK
	export_copy_des(&s3, &t3, s4, t4);
	plain_project_proc(&s3, &t3, s_nulllist, nulllist);
	sync2(s3,t3,s4,t4, &s2, &t2, &macro_ab, &macro_c);
	free(macro_ab); free(macro_c);
	freedes(s3, &t3); freedes(s4, &t4);
	macro_ab = macro_c = NULL;
	s3 = s4 = 0; t3 = t4 = NULL;

	// Compute G * NK
	obs_meet(s1, t1, s2, t2, &s3, &t3,&s_macro_b, &macro_b);
	//free(macro_b); macro_b = NULL; s_macro_b = 0;
	export_copy_des(&s8, &t8, s3, t3);
	freedes(s3, &t3); s3 = 0;  t3 = NULL;

	s_list = s_tmplist = 0; list = tmplist = NULL;

	for(i = 0; i < s_nulllist; i ++){
		addordlist(nulllist[i], &list, s_list, &ok);
		if(ok) s_list ++;
	}
	for(i = 0; i < s_imagelist; i ++){
		addordlist(imagelist[i], &list, s_list, &ok);
		if(ok) s_list ++;
	}     
	/*add m = 1001 to the list*/
	addordlist(1001, &list, s_list, &ok);
	if(ok) s_list ++;

	s_par1 = s_list;
	par1 = (part_node*)CALLOC(s_par1 , sizeof(part_node));
	if(par1 == NULL){
		mem_result = 1;
		result = -1;
		goto SUPOBS_LABEL;
	}    
	for(i = 0; i < s1; i ++){
		for(j = 0; j < t1[i].numelts; j ++){
			event = t1[i].next[j].data1;
			k = obs_getindex(event, list, s_list);
			addstatelist(i, &(par1[k].next), par1[k].numelts, &ok);
			if(ok) par1[k].numelts ++;   
		}
		if(t1[i].marked){           
			event = 1001;
			k = obs_getindex(event, list, s_list);
			addstatelist(i, &(par1[k].next), par1[k].numelts, &ok);
			if(ok) par1[k].numelts ++; 
		}
	}

START:

	// add dump state to des2 and extend its transition to full function
	export_copy_des(&s4, &t4, s2, t2);
	full_function(&s4, &t4, s_list, list);
	/*step2 : des3 = meet(des1, des2)*/     
	obs_meet(s8, t8, s4, t4, &s3, &t3,&s_macro_c, &macro_c);	
	if(mem_result == 1){
		result = -1;
		goto SUPOBS_LABEL;
	}

	if(s3 == 0){
		init = 0L;
		filedes(name3, s3, init, t3);
		goto SUPOBS_LABEL;
	}else if(s3 == 1 && t3[0].marked == false){
		init = 0L;
		filedes(name3, 0, init, NULL);
		goto SUPOBS_LABEL;
	}
          
	/*step3 : project des3 and get the partition (Ps = ps')*/
	obs_project(&s3, &t3, s_nulllist, nulllist, &s_macrosets, &macrosets);    

	// compute the projection of plant and store the correspondence of two projected DESs 
/*	export_copy_des(&s6, &t6, s8, t8);
	obs_project(&s6, &t6, s_nulllist, nulllist, &s_macrosets1, &macrosets1); 
	//zprint_map(s_macrosets1, macrosets1);
	obs_meet(s6, t6, s3, t3, &s7, &t7, &s_macro_d, &macro_d);

	obs_recode(s7, t7, s_macro_d, macro_d, &ok);
	if(!ok){
		result = -2;
		goto SUPOBS_LABEL;
	}*/

	/*step 4 :Compute the state subsets Q1 and Q2*/

	s_par2 = s_list;
	par2 = (part_node*)CALLOC(s_par2, sizeof(part_node));
	if(par2 == NULL){
		mem_result = 1;
		result = -1;
		goto SUPOBS_LABEL;
	}
	for(i = 0; i < s2; i ++){
		for(j = 0; j < t2[i].numelts; j ++){
			event = t2[i].next[j].data1;
			k = obs_getindex(event, list, s_list);
			if(k == -1){
				result = -1;
				goto SUPOBS_LABEL;
			}
			addstatelist(i, &(par2[k].next), par2[k].numelts, &ok);
			if(ok) par2[k].numelts ++; 
		}
		if(t2[i].marked){
			event = 1001;
			k = obs_getindex(event, list, s_list);
			if(k == -1){
				result = -1;
				goto SUPOBS_LABEL;
			}
			addstatelist(i, &(par2[k].next), par2[k].numelts, &ok);
			if(ok) par2[k].numelts ++; 
		}
	}
	/*step 5 :Construct the checking table*/
	s_table = 0;
	table = (obs_table*)CALLOC(s_table + 1, sizeof(obs_table));
	if(table == NULL){
		mem_result = 1;
		result = -1;
		goto SUPOBS_LABEL;
	}
	//for(l = 0; l < s_macro_d; l ++){
	//	l1 = macro_d[l] % s6;
	//	l2 = macro_d[l] / s6;
		for(i = 0; i < s_macrosets; i ++){        
		  if(macrosets[i].numelts > 1){           
			table = (obs_table*)REALLOC(table, (s_table + 1) * sizeof(obs_table));           
			if(table == NULL){
				mem_result = 1;
				result = -1;
				goto SUPOBS_LABEL;
			}
			table[s_table].numelts = 0; 
			table[s_table].next = NULL;                    
			for(j = 0; j < macrosets[i].numelts; j ++){
				k = macrosets[i].next[j];              
				state1 = macro_c[k] % s8;
				state1 = macro_b[state1] % s1;
				state2 = macro_c[k] / s8;                           
				table[s_table].numelts ++;                            
				table[s_table].next = (item_node*)REALLOC(table[s_table].next, table[s_table].numelts * sizeof(item_node));  
				if(table[s_table].next == NULL){
					mem_result = 1;
					result = -1;
					goto SUPOBS_LABEL;
				} 
				table[s_table].next[j].data1 = state1;
				table[s_table].next[j].data2 = state2;
			}
			//table[s_table].state = i; 

			/*for(j = 0; j < macrosets1[i].numelts; j ++){
				state = macrosets1[i].next[j];
				state1 = macro_b[state]%s1;
				state2 = macro_b[state]/s1;
				if(!intablelist1(state1, state2, table[s_table].next, table[s_table].numelts)){
					ss = table[s_table].numelts;
					table[s_table].numelts ++;                            
					table[s_table].next = (item_node*)REALLOC(table[s_table].next, table[s_table].numelts * sizeof(item_node));  
					if(table[s_table].next == NULL){
						mem_result = 1;
						goto SUPOBS_LABEL;
					} 
					table[s_table].next[ss].data1 = state1;
					table[s_table].next[ss].data2 = -1; 
				}
			}*/
			//table[s_table].Gstate = l1;
			s_table ++;          
		  }
		}  
	//} 
	for( i = 0; i < s_table; i ++){
		inode = table[i].next;            
		for(j = 0; j < table[i].numelts; j ++){
			state1 = inode[j].data1;
			state2 = inode[j].data2;           
			inode[j].numelts1 = (INT_T)s_par1;
			inode[j].numelts2 = (INT_T)s_par2;   
			inode[j].next1 = NULL;        
			inode[j].next1 = (INT_B *)REALLOC(inode[j].next1, inode[j].numelts1 * sizeof(INT_B ));
			inode[j].next2 = NULL;
			inode[j].next2 = (INT_B *)REALLOC(inode[j].next2, inode[j].numelts2 * sizeof(INT_B ));
			if(inode[j].next1 == NULL || inode[j].next2 == NULL){
				mem_result = 1;
				result = -1;
				goto SUPOBS_LABEL;
			}
			if(s_par1 != s_par2 || s_par1 != s_list){
				result = -1;
				goto SUPOBS_LABEL;
			}

			for(k = 0; k < s_list; k ++){              
				if(instatelist(state1, par1[k].next, par1[k].numelts)){
					inode[j].next1[k] = true; 
				}
				else inode[j].next1[k] = false;

				if(state2 < s2){
					if(instatelist(state2, par2[k].next, par2[k].numelts)){
						inode[j].next2[k] = true; 
					}else inode[j].next2[k] = false; 
				}else{
					inode[j].next2[k] = false;
				}                                               
			}           
		}
	}

	/*step 6 :Veryfy the observability according to the checking table*/
//	iteration ++;
//	print_table(s_table,table, s_list, list, iteration);

	/*if(Tpos != -1){
		bObs = true;
		for(i = 0; i < s_table; i ++){
			inode = table[i].next;   
			if(Tpos == table[i].Gstate){
				for(k = 0; k < s_list; k ++){
					found = 0;      
					if(table[i].numelts > 1){
						for(j = 0; j < table[i].numelts; j ++){                  
							if((inode[j].next1[k] == true) && (inode[j].next2[k] == true)){
								found = 2;
							}
						}    
					}
					if(found != 0){
						for(j = 0; j < table[i].numelts; j ++){              
							if(inode[j].next1[k] == true){
								if(inode[j].next2[k] == false){                    
									bObs = false;
									event = list[k];
									state2 = inode[j].data2;
									state = table[i].state;
									goto Remove_Tran; // keep i, k
								}                    
							}
						}
					}
				}
			}
		}
		trim1(&s2, &t2);
		Tpos = -1;
	}*/

	bObs = true;
	for(i = 0; i < s_table; i ++){
		inode = table[i].next;        
		for(k = 0; k < s_list; k ++){
			found = 0;      
			if(table[i].numelts > 1){
				for(j = 0; j < table[i].numelts; j ++){                  
					if((inode[j].next1[k] == true) && (inode[j].next2[k] == true)){
						found = 1;
					}
				}    
			}
			if(found == 1){
				for(j = 0; j < table[i].numelts; j ++){              
					if(inode[j].next1[k] == true){
						if(inode[j].next2[k] == false){                    
							bObs = false;
							event = list[k];
							//state2 = inode[j].data2;
							//state = table[i].state;
							//Tpos = table[i].Gstate;
							goto Remove_Tran; // keep i, k
						}                    
					}
				}
			}
		}
	}
Remove_Tran:
	if(!bObs){
			//zprint_map(s_macrosets, macrosets);
			// delete enabled transitions
			bDel = false;
	/*		for(l = 0; l < macrosets[state].numelts; l ++){
				ss = macrosets[state].next[l];
				ss = macro_c[ss] / s8;
				if(ss < s2){
					if(event == 1001){
						if(t2[ss].marked){
							t2[ss].marked = false;
						}
					}else {
						DeleteOrdlist(event, &t2[ss].next, &t2[ss].numelts);
					}
					bDel = true;
				}
			}*/
			for(j = 0; j < table[i].numelts; j ++){
				state2 = inode[j].data2;
				if(state2 < s2){
					if(event == 1001){
						if(t2[state2].marked){
							t2[state2].marked = false;
						}
					}else {
						DeleteOrdlist(event, &t2[state2].next, &t2[state2].numelts);
					}
					bDel = true;
				}
			}
			new_trim1(&s2, &t2);
			if(!bDel)
				bObs = true;
		//}
	}
//REPEAT:
	if(!bObs){
		freedes(s3, &t3); s3 = 0; t3 = NULL;
		//freedes(s4, &t4); s4 = 0; t4 = NULL;
		//freedes(s5, &t5); s5 = 0; t5 = NULL;
		freedes(s6, &t6); s6 = 0; t6 = NULL;
		freedes(s7, &t7); s7 = 0; t7 = NULL;
		obs_free_par(&s_par2, &par2); s_par2 = 0; par2 = NULL;
		free_table(s_table, &table); s_table = 0; table = NULL;
		free(macro_c); macro_c = NULL; s_macro_c = 0;
		free(macrosets); macrosets = NULL; s_macrosets = 0;
		free(macro_d); macro_d = NULL; s_macro_d = 0;
		free(macrosets1); macrosets1 = NULL; s_macrosets1 = 0;
		free(tmplist); tmplist = NULL; s_tmplist = 0;
		goto START;
	}else{
		//reach(&s2, &t2);
		trim1(&s2, &t2);
		init = 0L;
		filedes(name3, s2, init, t2);
	}

SUPOBS_LABEL:
	if(mem_result == 1)
		result = -1;
	free(macro_b);
	free(macro_c);
	free(macro_d);
	free(macrosets);
	free(macrosets1);
	free(mapstate);
	free(list);
	free(tmplist);
	freedes(s1, &t1);
	freedes(s2, &t2);
	freedes(s3, &t3);
	freedes(s4, &t4);
	//freedes(s5, &t5);     
	freedes(s6, &t6);    
	freedes(s7, &t7);
	freedes(s8, &t8);
	obs_free_par(&s_par1, &par1);
	obs_free_par(&s_par2, &par2);
	free_table(s_table, &table);
	return result; 
}
// This function will be called by feasible, we don't compute the normal formal of given K.
INT_OS supobs_proc3(char * name3, char* name1, char* name2, INT_T s_nulllist, INT_T *nulllist, INT_T s_imagelist, INT_T* imagelist)
{
	state_node *t1, *t2, *t3, *t4, *t6, *t7, *t8;
	INT_S s1, s2, s3, s4, s6, s7, s8, init;
	INT_S s_macro_c, s_macro_b, s_macro_ab, * macro_c, * macro_b, s_macro_d, *macro_d, *macro_ab;     
	INT_S s_macrosets, s_macrosets1;
	state_map * macrosets, *macrosets1;
	part_node * par1, *par2;
	INT_S s_par1, s_par2;
	INT_S i, j, k;//, l;
	INT_T s_list, s_tmplist, *list, *tmplist;
	INT_T event, found;
	INT_B  ok, bObs, bDel;
	INT_S s_table, iteration, Tpos; obs_table *table;
	INT_S state1, state2;

	item_node *inode;
	INT_S * mapstate;
	INT_OS result;

	t1 = t2 = t3 = t4 = t6 = t7 = t8 = NULL;
	s1 = s2 = s3 = s4 = s6 = s7 = s8 = 0;
	s_macro_c = s_macro_b = s_macro_d = s_macro_ab = 0; 
	macro_c = macro_b = macro_d = macro_ab = NULL;

	s_macrosets = s_macrosets1 = 0; macrosets = macrosets1 = NULL;
	par1 = par2 = NULL;
	s_par1 = s_par2 = 0;
	s_table = 0; table = NULL;
	mapstate = NULL;
	bObs = false;
	result = 0;

	iteration = 0;
	Tpos = -1;

	/*step1 : get des1 and des2*/
	init = 0L;
	if(getdes(name1, &s1, &init, &t1) == false)
		return -1;
	init = 0L;    
	if(getdes(name2, &s2, &init, &t2) == false)
		return -1;
	
	// Compute G * K: K has already been the normal form
	obs_meet(s1, t1, s2, t2, &s3, &t3,&s_macro_b, &macro_b);

	export_copy_des(&s8, &t8, s3, t3);
	freedes(s3, &t3); s3 = 0;  t3 = NULL;

	s_list = s_tmplist = 0; list = tmplist = NULL;

	for(i = 0; i < s_nulllist; i ++){
		addordlist(nulllist[i], &list, s_list, &ok);
		if(ok) s_list ++;
	}
	for(i = 0; i < s_imagelist; i ++){
		addordlist(imagelist[i], &list, s_list, &ok);
		if(ok) s_list ++;
	}     
	/*add m = 1001 to the list*/
	addordlist(1001, &list, s_list, &ok);
	if(ok) s_list ++;

	s_par1 = s_list;
	par1 = (part_node*)CALLOC(s_par1 , sizeof(part_node));
	if(par1 == NULL){
		mem_result = 1;
		goto SUPOBS_LABEL;
	}    
	for(i = 0; i < s1; i ++){
		for(j = 0; j < t1[i].numelts; j ++){
			event = t1[i].next[j].data1;
			k = obs_getindex(event, list, s_list);
			addstatelist(i, &(par1[k].next), par1[k].numelts, &ok);
			if(ok) par1[k].numelts ++;   
		}
		if(t1[i].marked){           
			event = 1001;
			k = obs_getindex(event, list, s_list);
			addstatelist(i, &(par1[k].next), par1[k].numelts, &ok);
			if(ok) par1[k].numelts ++; 
		}
	}

START:
	// add dump state to des2 and extend its transition to full function
	export_copy_des(&s4, &t4, s2, t2);
	full_function(&s4, &t4, s_list, list);

	/*step2 : des3 = meet(des1, des2)*/     
	obs_meet(s8, t8, s4, t4, &s3, &t3, &s_macro_c, &macro_c);	
	if(mem_result == 1){
		result = -1;
		goto SUPOBS_LABEL;
	}

	if(s3 == 0){
		init = 0L;
		filedes(name3, s3, init, t3);
		goto SUPOBS_LABEL;
	}else if(s3 == 1 && t3[0].marked == false){
		init = 0L;
		filedes(name3, 0, init, NULL);
		goto SUPOBS_LABEL;
	}
          
	/*step3 : project des3 and get the partition (Ps = ps')*/
	obs_project(&s3, &t3, s_nulllist, nulllist, &s_macrosets, &macrosets);    

	/*step 4 :Compute the state subsets Q1 and Q2*/

	s_par2 = s_list;
	par2 = (part_node*)CALLOC(s_par2, sizeof(part_node));
	if(par2 == NULL){
		mem_result = 1;
		goto SUPOBS_LABEL;
	}
	for(i = 0; i < s2; i ++){
		for(j = 0; j < t2[i].numelts; j ++){
			event = t2[i].next[j].data1;
			k = obs_getindex(event, list, s_list);
			if(k == -1){
				result = -1;
				goto SUPOBS_LABEL;
			}
			addstatelist(i, &(par2[k].next), par2[k].numelts, &ok);
			if(ok) par2[k].numelts ++; 
		}
		if(t2[i].marked){
			event = 1001;
			k = obs_getindex(event, list, s_list);
			if(k == -1){
				result = -1;
				goto SUPOBS_LABEL;
			}
			addstatelist(i, &(par2[k].next), par2[k].numelts, &ok);
			if(ok) par2[k].numelts ++; 
		}
	}
	/*step 5 :Construct the checking table*/
	s_table = 0;
	table = (obs_table*)CALLOC(s_table + 1, sizeof(obs_table));
	if(table == NULL){
		mem_result = 1;
		goto SUPOBS_LABEL;
	}
	for(i = 0; i < s_macrosets; i ++){        
		if(macrosets[i].numelts > 1){           
			table = (obs_table*)REALLOC(table, (s_table + 1) * sizeof(obs_table));           
			if(table == NULL){
				mem_result = 1;
				goto SUPOBS_LABEL;
			}
			table[s_table].numelts = 0; 
			table[s_table].next = NULL;                    
			for(j = 0; j < macrosets[i].numelts; j ++){
				k = macrosets[i].next[j];              
				state1 = macro_c[k] % s8;
				state1 = macro_b[state1] % s1;
				state2 = macro_c[k] / s8;                           
				table[s_table].numelts ++;                            
				table[s_table].next = (item_node*)REALLOC(table[s_table].next, table[s_table].numelts * sizeof(item_node));  
				if(table[s_table].next == NULL){
					mem_result = 1;
					goto SUPOBS_LABEL;
				} 
				table[s_table].next[j].data1 = state1;
				table[s_table].next[j].data2 = state2;
			}
			s_table ++;          
		}
	}  
	for( i = 0; i < s_table; i ++){
		inode = table[i].next;            
		for(j = 0; j < table[i].numelts; j ++){
			state1 = inode[j].data1;
			state2 = inode[j].data2;           
			inode[j].numelts1 = (INT_T)s_par1;
			inode[j].numelts2 = (INT_T)s_par2;   
			inode[j].next1 = NULL;        
			inode[j].next1 = (INT_B *)REALLOC(inode[j].next1, inode[j].numelts1 * sizeof(INT_B ));
			inode[j].next2 = NULL;
			inode[j].next2 = (INT_B *)REALLOC(inode[j].next2, inode[j].numelts2 * sizeof(INT_B ));
			if(inode[j].next1 == NULL || inode[j].next2 == NULL){
				mem_result = 1;
				goto SUPOBS_LABEL;
			}
			if(s_par1 != s_par2 || s_par1 != s_list){
				result = -1;
				goto SUPOBS_LABEL;
			}

			for(k = 0; k < s_list; k ++){              
				if(instatelist(state1, par1[k].next, par1[k].numelts)){
					inode[j].next1[k] = true; 
				}
				else inode[j].next1[k] = false;

				if(state2 < s2){
					if(instatelist(state2, par2[k].next, par2[k].numelts)){
						inode[j].next2[k] = true; 
					}else inode[j].next2[k] = false; 
				}else{
					inode[j].next2[k] = false;
				}                                               
			}           
		}
	}

	/*step 6 :Veryfy the observability according to the checking table*/
//	iteration ++;
	//print_table(s_table,table, s_list, list, iteration);

	bObs = true;
	for(i = 0; i < s_table; i ++){
		inode = table[i].next;        
		for(k = 0; k < s_list; k ++){
			found = 0;      
			if(table[i].numelts > 1){
				for(j = 0; j < table[i].numelts; j ++){                  
					if((inode[j].next1[k] == true) && (inode[j].next2[k] == true)){
						found = 1;
					}
				}    
			}
			if(found == 1){
				for(j = 0; j < table[i].numelts; j ++){              
					if(inode[j].next1[k] == true){
						if(inode[j].next2[k] == false){                    
							bObs = false;
							event = list[k];
							goto Remove_Tran; // keep i, k
						}                    
					}
				}
			}
		}
	}
Remove_Tran:
	if(!bObs){
			// delete enabled transitions
			bDel = false;
			for(j = 0; j < table[i].numelts; j ++){
				state2 = inode[j].data2;
				if(state2 < s2){
					if(event == 1001){
						if(t2[state2].marked){
							t2[state2].marked = false;
						}
					}else {
						DeleteOrdlist(event, &t2[state2].next, &t2[state2].numelts);
					}
					bDel = true;
				}
			}
			new_trim1(&s2, &t2);
			if(!bDel)
				bObs = true;
	}
	if(!bObs){
		freedes(s3, &t3); s3 = 0; t3 = NULL;
		freedes(s6, &t6); s6 = 0; t6 = NULL;
		freedes(s7, &t7); s7 = 0; t7 = NULL;
		obs_free_par(&s_par2, &par2); s_par2 = 0; par2 = NULL;
		free_table(s_table, &table); s_table = 0; table = NULL;
		free(macro_c); macro_c = NULL; s_macro_c = 0;
		free(macrosets); macrosets = NULL; s_macrosets = 0;
		free(macro_d); macro_d = NULL; s_macro_d = 0;
		free(macrosets1); macrosets1 = NULL; s_macrosets1 = 0;
		free(tmplist); tmplist = NULL; s_tmplist = 0;
		goto START;
	}else{
		trim1(&s2, &t2);
		init = 0L;
		filedes(name3, s2, init, t2);
	}

SUPOBS_LABEL:
	free(macro_b);
	free(macro_c);
	free(macro_d);
	free(macrosets);
	free(macrosets1);
	free(mapstate);
	free(list);
	free(tmplist);
	freedes(s1, &t1);
	freedes(s2, &t2);
	freedes(s3, &t3);
	freedes(s4, &t4);
	//freedes(s5, &t5);     
	freedes(s6, &t6);    
	freedes(s7, &t7);
	freedes(s8, &t8);
	obs_free_par(&s_par1, &par1);
	obs_free_par(&s_par2, &par2);
	free_table(s_table, &table);
	return result; 
}
// Check observability of K(name3) relative to C(name2) with respect to G(name1)
INT_OS rel_observ_proc(char* name1, char* name2, char *name3, INT_T s_contr_list, INT_T* contr_list, INT_T s_nulllist, INT_T *nulllist, INT_T s_imagelist, INT_T* imagelist, INT_B *flag)
{
	state_node *t1, *t2, *t3, *t4, *t6, *t7, *t8;
	INT_S s1, s2, s3, s4, s6, s7, s8, init;
	INT_S s_macro_c, s_macro_b, s_macro_ab, * macro_c, * macro_b, s_macro_d, *macro_d, *macro_ab;     
	INT_S s_macrosets, s_macrosets1;
	state_map * macrosets, *macrosets1;
	part_node * par1, *par2;
	INT_S s_par1, s_par2;
	INT_S i, j, k;//, l;
	INT_T s_list, s_tmplist, *list, *tmplist;
	INT_T event, found;
	INT_B  ok, bObs;//, bDel;
	INT_S s_table, iteration, Tpos; obs_table *table;
	INT_S state1, state2;//, state;//, ss, l1, l2;//, nstate1;//, nstate2;

	//     INT_B  *item1, *item2;
	item_node *inode;
	INT_S * mapstate;
	//     FILE * f1;
	INT_OS result;

	t1 = t2 = t3 = t4 = t6 = t7 = t8 = NULL;
	s1 = s2 = s3 = s4 = s6 = s7 = s8 = 0;
	s_macro_c = s_macro_b = s_macro_d = s_macro_ab = 0; macro_c = macro_b = macro_d = macro_ab = NULL;
	s_macrosets = s_macrosets1 = 0; macrosets = macrosets1 = NULL;
	par1 = par2 = NULL;
	s_par1 = s_par2 = 0;
	s_table = 0; table = NULL;
	mapstate = NULL;
	bObs = false;
	result = 0;

	iteration = 0;
	Tpos = -1;

	/*step1 : get des1 and des2*/
	init = 0L;
	if(getdes(name1, &s1, &init, &t1) == false)
		return -1;
	init = 0L;    
	if(getdes(name2, &s2, &init, &t2) == false)
		return -1;

	init = 0L;    
	if(getdes(name3, &s3, &init, &t3) == false)
		return -1;
	
	//Compute Normal form of NK   NK = K || PK
	//export_copy_des(&s3, &t3, s4, t4);
	//plain_project_proc(&s3, &t3, s_nulllist, nulllist);
	//sync2(s3,t3,s4,t4, &s2, &t2, &macro_ab, &macro_c);
	//free(macro_ab); free(macro_c);
	//freedes(s3, &t3); freedes(s4, &t4);
	//macro_ab = macro_c = NULL;
	//s3 = s4 = 0; t3 = t4 = NULL;

	// Compute G * NK
	obs_meet(s1, t1, s3, t3, &s8, &t8,&s_macro_b, &macro_b);
	//free(macro_b); macro_b = NULL; s_macro_b = 0;
	//export_copy_des(&s8, &t8, s3, t3);
	freedes(s3, &t3); s3 = 0;  t3 = NULL;

	s_list = s_tmplist = 0; list = tmplist = NULL;

	for(i = 0; i < s_nulllist; i ++){
		addordlist(nulllist[i], &list, s_list, &ok);
		if(ok) s_list ++;
	}
	for(i = 0; i < s_imagelist; i ++){
		addordlist(imagelist[i], &list, s_list, &ok);
		if(ok) s_list ++;
	}     
	/*add m = 1001 to the list*/
	addordlist(1001, &list, s_list, &ok);
	if(ok) s_list ++;

	s_par1 = s_list;
	par1 = (part_node*)CALLOC(s_par1 , sizeof(part_node));
	if(par1 == NULL){
		mem_result = 1;
		goto SUPOBS_LABEL;
	}    
	for(i = 0; i < s1; i ++){
		for(j = 0; j < t1[i].numelts; j ++){
			event = t1[i].next[j].data1;
			k = obs_getindex(event, list, s_list);
			addstatelist(i, &(par1[k].next), par1[k].numelts, &ok);
			if(ok) par1[k].numelts ++;   
		}
		if(t1[i].marked){           
			event = 1001;
			k = obs_getindex(event, list, s_list);
			addstatelist(i, &(par1[k].next), par1[k].numelts, &ok);
			if(ok) par1[k].numelts ++; 
		}
	}

	// add dump state to des2 and extend its transition to full function
	export_copy_des(&s4, &t4, s2, t2);
	full_function(&s4, &t4, s_list, list);
	/*step2 : des3 = meet(des1, des2)*/     
	obs_meet(s8, t8, s4, t4, &s3, &t3,&s_macro_c, &macro_c);	
	if(mem_result == 1){
		result = -1;
		goto SUPOBS_LABEL;
	}

	if(s3 == 0){
		init = 0L;
		filedes(name3, s3, init, t3);
		goto SUPOBS_LABEL;
	}else if(s3 == 1 && t3[0].marked == false){
		init = 0L;
		filedes(name3, 0, init, NULL);
		goto SUPOBS_LABEL;
	}
          
	/*step3 : project des3 and get the partition (Ps = ps')*/
	obs_project(&s3, &t3, s_nulllist, nulllist, &s_macrosets, &macrosets);    

	//reverse_des(&s5, &t5, s3, t3);            // store the reverse diagram of projected des3

	// compute the projection of plant and store the correspondence of two projected DESs 
/*	export_copy_des(&s6, &t6, s8, t8);
	obs_project(&s6, &t6, s_nulllist, nulllist, &s_macrosets1, &macrosets1); 
	//zprint_map(s_macrosets1, macrosets1);
	obs_meet(s6, t6, s3, t3, &s7, &t7, &s_macro_d, &macro_d);

	obs_recode(s7, t7, s_macro_d, macro_d, &ok);
	if(!ok){
		result = -2;
		goto SUPOBS_LABEL;
	}*/

	/*step 4 :Compute the state subsets Q1 and Q2*/

	s_par2 = s_list;
	par2 = (part_node*)CALLOC(s_par2, sizeof(part_node));
	if(par2 == NULL){
		mem_result = 1;
		goto SUPOBS_LABEL;
	}
	for(i = 0; i < s2; i ++){
		for(j = 0; j < t2[i].numelts; j ++){
			event = t2[i].next[j].data1;
			k = obs_getindex(event, list, s_list);
			if(k == -1){
				result = -1;
				goto SUPOBS_LABEL;
			}
			addstatelist(i, &(par2[k].next), par2[k].numelts, &ok);
			if(ok) par2[k].numelts ++; 
		}
		if(t2[i].marked){
			event = 1001;
			k = obs_getindex(event, list, s_list);
			if(k == -1){
				result = -1;
				goto SUPOBS_LABEL;
			}
			addstatelist(i, &(par2[k].next), par2[k].numelts, &ok);
			if(ok) par2[k].numelts ++; 
		}
	}
	/*step 5 :Construct the checking table*/
	s_table = 0;
	table = (obs_table*)CALLOC(s_table + 1, sizeof(obs_table));
	if(table == NULL){
		mem_result = 1;
		goto SUPOBS_LABEL;
	}
	//for(l = 0; l < s_macro_d; l ++){
	//	l1 = macro_d[l] % s6;
	//	l2 = macro_d[l] / s6;
		for(i = 0; i < s_macrosets; i ++){        
		  if(macrosets[i].numelts > 1){           
			table = (obs_table*)REALLOC(table, (s_table + 1) * sizeof(obs_table));           
			if(table == NULL){
				mem_result = 1;
				goto SUPOBS_LABEL;
			}
			table[s_table].numelts = 0; 
			table[s_table].next = NULL;                    
			for(j = 0; j < macrosets[i].numelts; j ++){
				k = macrosets[i].next[j];              
				state1 = macro_c[k] % s8;
				state1 = macro_b[state1] % s1;
				state2 = macro_c[k] / s8;                           
				table[s_table].numelts ++;                            
				table[s_table].next = (item_node*)REALLOC(table[s_table].next, table[s_table].numelts * sizeof(item_node));  
				if(table[s_table].next == NULL){
					mem_result = 1;
					goto SUPOBS_LABEL;
				} 
				table[s_table].next[j].data1 = state1;
				table[s_table].next[j].data2 = state2;
			}
			//table[s_table].state = i; 

			/*for(j = 0; j < macrosets1[i].numelts; j ++){
				state = macrosets1[i].next[j];
				state1 = macro_b[state]%s1;
				state2 = macro_b[state]/s1;
				if(!intablelist1(state1, state2, table[s_table].next, table[s_table].numelts)){
					ss = table[s_table].numelts;
					table[s_table].numelts ++;                            
					table[s_table].next = (item_node*)REALLOC(table[s_table].next, table[s_table].numelts * sizeof(item_node));  
					if(table[s_table].next == NULL){
						mem_result = 1;
						goto SUPOBS_LABEL;
					} 
					table[s_table].next[ss].data1 = state1;
					table[s_table].next[ss].data2 = -1; 
				}
			}*/
			//table[s_table].Gstate = l1;
			s_table ++;          
		  }
		}  
	//} 
	for( i = 0; i < s_table; i ++){
		inode = table[i].next;            
		for(j = 0; j < table[i].numelts; j ++){
			state1 = inode[j].data1;
			state2 = inode[j].data2;           
			inode[j].numelts1 = (INT_T)s_par1;
			inode[j].numelts2 = (INT_T)s_par2;   
			inode[j].next1 = NULL;        
			inode[j].next1 = (INT_B *)REALLOC(inode[j].next1, inode[j].numelts1 * sizeof(INT_B ));
			inode[j].next2 = NULL;
			inode[j].next2 = (INT_B *)REALLOC(inode[j].next2, inode[j].numelts2 * sizeof(INT_B ));
			if(inode[j].next1 == NULL || inode[j].next2 == NULL){
				mem_result = 1;
				goto SUPOBS_LABEL;
			}
			if(s_par1 != s_par2 || s_par1 != s_list){
				result = -1;
				goto SUPOBS_LABEL;
			}

			for(k = 0; k < s_list; k ++){              
				if(instatelist(state1, par1[k].next, par1[k].numelts)){
					inode[j].next1[k] = true; 
				}
				else inode[j].next1[k] = false;

				if(state2 < s2){
					if(instatelist(state2, par2[k].next, par2[k].numelts)){
						inode[j].next2[k] = true; 
					}else inode[j].next2[k] = false; 
				}else{
					inode[j].next2[k] = false;
				}                                               
			}           
		}
	}

	/*step 6 :Veryfy the observability according to the checking table*/
	iteration ++;
	print_table(s_table,table, s_list, list, iteration);

	/*if(Tpos != -1){
		bObs = true;
		for(i = 0; i < s_table; i ++){
			inode = table[i].next;   
			if(Tpos == table[i].Gstate){
				for(k = 0; k < s_list; k ++){
					found = 0;      
					if(table[i].numelts > 1){
						for(j = 0; j < table[i].numelts; j ++){                  
							if((inode[j].next1[k] == true) && (inode[j].next2[k] == true)){
								found = 2;
							}
						}    
					}
					if(found != 0){
						for(j = 0; j < table[i].numelts; j ++){              
							if(inode[j].next1[k] == true){
								if(inode[j].next2[k] == false){                    
									bObs = false;
									event = list[k];
									state2 = inode[j].data2;
									state = table[i].state;
									goto Remove_Tran; // keep i, k
								}                    
							}
						}
					}
				}
			}
		}
		trim1(&s2, &t2);
		Tpos = -1;
	}*/

	bObs = true;
	for(i = 0; i < s_table; i ++){
		inode = table[i].next;        
		for(k = 0; k < s_list; k ++){
			if(!inlist(list[k], contr_list, s_contr_list))
				continue;
			found = 0;      
			if(table[i].numelts > 1){
				for(j = 0; j < table[i].numelts; j ++){                  
					if((inode[j].next1[k] == true) && (inode[j].next2[k] == true)){
						found = 1;
					}
				}    
			}
			if(found == 1){
				for(j = 0; j < table[i].numelts; j ++){              
					if(inode[j].next1[k] == true){
						if(inode[j].next2[k] == false){   
							*flag = false;
							//bObs = false;
							//event = list[k];
							//state2 = inode[j].data2;
							//state = table[i].state;
							//Tpos = table[i].Gstate;
							goto SUPOBS_LABEL; // keep i, k
						}                    
					}
				}
			}
		}
	}

SUPOBS_LABEL:
	free(macro_b);
	free(macro_c);
	free(macro_d);
	free(macrosets);
	free(macrosets1);
	free(mapstate);
	free(list);
	free(tmplist);
	freedes(s1, &t1);
	freedes(s2, &t2);
	freedes(s3, &t3);
	freedes(s4, &t4);
	//freedes(s5, &t5);     
	freedes(s6, &t6);    
	freedes(s7, &t7);
	freedes(s8, &t8);
	obs_free_par(&s_par1, &par1);
	obs_free_par(&s_par2, &par2);
	free_table(s_table, &table);
	return result; 
}

//This function will be called by suprcoobs. Here we specify an ambient language (by name3), 
//and we don't compute the normal formal of given K.
INT_OS supobs_proc5(char * name4, char* name1, char* name2, char *name3, INT_T s_nulllist, INT_T *nulllist, INT_T s_imagelist, INT_T* imagelist)
{
	state_node *t1, *t2, *t3, *t4, *t6, *t7, *t8;
	INT_S s1, s2, s3, s4, s6, s7, s8, init;
	INT_S s_macro_c, s_macro_b, s_macro_ab, * macro_c, * macro_b, s_macro_d, *macro_d, *macro_ab;     
	INT_S s_macrosets, s_macrosets1;
	state_map * macrosets, *macrosets1;
	part_node * par1, *par2;
	INT_S s_par1, s_par2;
	INT_S i, j, k;//, l;
	INT_T s_list, s_tmplist, *list, *tmplist;
	INT_T event, found;
	INT_B  ok, bObs, bDel;
	INT_S s_table, iteration, Tpos; obs_table *table;
	INT_S state1, state2;//, state;//, ss, l1, l2;//, nstate1;//, nstate2;

	//     INT_B  *item1, *item2;
	item_node *inode;
	INT_S * mapstate;
	//     FILE * f1;
	INT_OS result;

	t1 = t2 = t3 = t4 = t6 = t7 = t8 = NULL;
	s1 = s2 = s3 = s4 = s6 = s7 = s8 = 0;
	s_macro_c = s_macro_b = s_macro_d = s_macro_ab = 0; macro_c = macro_b = macro_d = macro_ab = NULL;
	s_macrosets = s_macrosets1 = 0; macrosets = macrosets1 = NULL;
	par1 = par2 = NULL;
	s_par1 = s_par2 = 0;
	s_table = 0; table = NULL;
	mapstate = NULL;
	bObs = false;
	result = 0;

	iteration = 0;
	Tpos = -1;

	/*step1 : get des1 and des2*/
	init = 0L;
	if(getdes(name1, &s1, &init, &t1) == false)
		return -1;
	init = 0L;    
	if(getdes(name2, &s2, &init, &t2) == false)
		return -1;

	init = 0L;    
	if(getdes(name3, &s3, &init, &t3) == false)
		return -1;
	
	//Compute Normal form of NK   NK = K || PK
	//export_copy_des(&s3, &t3, s4, t4);
	//plain_project_proc(&s3, &t3, s_nulllist, nulllist);
	//sync2(s3,t3,s4,t4, &s2, &t2, &macro_ab, &macro_c);
	//free(macro_ab); free(macro_c);
	//freedes(s3, &t3); freedes(s4, &t4);
	//macro_ab = macro_c = NULL;
	//s3 = s4 = 0; t3 = t4 = NULL;

	// Compute G * C
	obs_meet(s1, t1, s3, t3, &s8, &t8,&s_macro_b, &macro_b);
	freedes(s3, &t3); s3 = 0;  t3 = NULL;

	s_list = s_tmplist = 0; list = tmplist = NULL;

	for(i = 0; i < s_nulllist; i ++){
		addordlist(nulllist[i], &list, s_list, &ok);
		if(ok) s_list ++;
	}
	for(i = 0; i < s_imagelist; i ++){
		addordlist(imagelist[i], &list, s_list, &ok);
		if(ok) s_list ++;
	}     
	/*add m = 1001 to the list*/
	addordlist(1001, &list, s_list, &ok);
	if(ok) s_list ++;

	s_par1 = s_list;
	par1 = (part_node*)CALLOC(s_par1 , sizeof(part_node));
	if(par1 == NULL){
		mem_result = 1;
		goto SUPOBS_LABEL;
	}    
	for(i = 0; i < s1; i ++){
		for(j = 0; j < t1[i].numelts; j ++){
			event = t1[i].next[j].data1;
			k = obs_getindex(event, list, s_list);
			addstatelist(i, &(par1[k].next), par1[k].numelts, &ok);
			if(ok) par1[k].numelts ++;   
		}
		if(t1[i].marked){           
			event = 1001;
			k = obs_getindex(event, list, s_list);
			addstatelist(i, &(par1[k].next), par1[k].numelts, &ok);
			if(ok) par1[k].numelts ++; 
		}
	}

START:

	// add dump state to des2 and extend its transition to full function
	export_copy_des(&s4, &t4, s2, t2);
	full_function(&s4, &t4, s_list, list);
	/*step2 : des3 = meet(des1, des2)*/     
	obs_meet(s8, t8, s4, t4, &s3, &t3,&s_macro_c, &macro_c);	
	if(mem_result == 1){
		result = -1;
		goto SUPOBS_LABEL;
	}

	if(s3 == 0){
		init = 0L;
		filedes(name4, s3, init, t3);
		goto SUPOBS_LABEL;
	}else if(s3 == 1 && t3[0].marked == false){
		init = 0L;
		filedes(name4, 0, init, NULL);
		goto SUPOBS_LABEL;
	}
          
	/*step3 : project des3 and get the partition (Ps = ps')*/
	obs_project(&s3, &t3, s_nulllist, nulllist, &s_macrosets, &macrosets);    

	//reverse_des(&s5, &t5, s3, t3);            // store the reverse diagram of projected des3

	// compute the projection of plant and store the correspondence of two projected DESs 
/*	export_copy_des(&s6, &t6, s8, t8);
	obs_project(&s6, &t6, s_nulllist, nulllist, &s_macrosets1, &macrosets1); 
	//zprint_map(s_macrosets1, macrosets1);
	obs_meet(s6, t6, s3, t3, &s7, &t7, &s_macro_d, &macro_d);

	obs_recode(s7, t7, s_macro_d, macro_d, &ok);
	if(!ok){
		result = -2;
		goto SUPOBS_LABEL;
	}*/

	/*step 4 :Compute the state subsets Q1 and Q2*/

	s_par2 = s_list;
	par2 = (part_node*)CALLOC(s_par2, sizeof(part_node));
	if(par2 == NULL){
		mem_result = 1;
		goto SUPOBS_LABEL;
	}
	for(i = 0; i < s2; i ++){
		for(j = 0; j < t2[i].numelts; j ++){
			event = t2[i].next[j].data1;
			k = obs_getindex(event, list, s_list);
			if(k == -1){
				result = -1;
				goto SUPOBS_LABEL;
			}
			addstatelist(i, &(par2[k].next), par2[k].numelts, &ok);
			if(ok) par2[k].numelts ++; 
		}
		if(t2[i].marked){
			event = 1001;
			k = obs_getindex(event, list, s_list);
			if(k == -1){
				result = -1;
				goto SUPOBS_LABEL;
			}
			addstatelist(i, &(par2[k].next), par2[k].numelts, &ok);
			if(ok) par2[k].numelts ++; 
		}
	}
	/*step 5 :Construct the checking table*/
	s_table = 0;
	table = (obs_table*)CALLOC(s_table + 1, sizeof(obs_table));
	if(table == NULL){
		mem_result = 1;
		goto SUPOBS_LABEL;
	}
	//for(l = 0; l < s_macro_d; l ++){
	//	l1 = macro_d[l] % s6;
	//	l2 = macro_d[l] / s6;
		for(i = 0; i < s_macrosets; i ++){        
		  if(macrosets[i].numelts > 1){           
			table = (obs_table*)REALLOC(table, (s_table + 1) * sizeof(obs_table));           
			if(table == NULL){
				mem_result = 1;
				goto SUPOBS_LABEL;
			}
			table[s_table].numelts = 0; 
			table[s_table].next = NULL;                    
			for(j = 0; j < macrosets[i].numelts; j ++){
				k = macrosets[i].next[j];              
				state1 = macro_c[k] % s8;
				state1 = macro_b[state1] % s1;
				state2 = macro_c[k] / s8;                           
				table[s_table].numelts ++;                            
				table[s_table].next = (item_node*)REALLOC(table[s_table].next, table[s_table].numelts * sizeof(item_node));  
				if(table[s_table].next == NULL){
					mem_result = 1;
					goto SUPOBS_LABEL;
				} 
				table[s_table].next[j].data1 = state1;
				table[s_table].next[j].data2 = state2;
			}
			//table[s_table].state = i; 

			/*for(j = 0; j < macrosets1[i].numelts; j ++){
				state = macrosets1[i].next[j];
				state1 = macro_b[state]%s1;
				state2 = macro_b[state]/s1;
				if(!intablelist1(state1, state2, table[s_table].next, table[s_table].numelts)){
					ss = table[s_table].numelts;
					table[s_table].numelts ++;                            
					table[s_table].next = (item_node*)REALLOC(table[s_table].next, table[s_table].numelts * sizeof(item_node));  
					if(table[s_table].next == NULL){
						mem_result = 1;
						goto SUPOBS_LABEL;
					} 
					table[s_table].next[ss].data1 = state1;
					table[s_table].next[ss].data2 = -1; 
				}
			}*/
			//table[s_table].Gstate = l1;
			s_table ++;          
		  }
		}  
	//} 
	for( i = 0; i < s_table; i ++){
		inode = table[i].next;            
		for(j = 0; j < table[i].numelts; j ++){
			state1 = inode[j].data1;
			state2 = inode[j].data2;           
			inode[j].numelts1 = (INT_T)s_par1;
			inode[j].numelts2 = (INT_T)s_par2;   
			inode[j].next1 = NULL;        
			inode[j].next1 = (INT_B *)REALLOC(inode[j].next1, inode[j].numelts1 * sizeof(INT_B ));
			inode[j].next2 = NULL;
			inode[j].next2 = (INT_B *)REALLOC(inode[j].next2, inode[j].numelts2 * sizeof(INT_B ));
			if(inode[j].next1 == NULL || inode[j].next2 == NULL){
				mem_result = 1;
				goto SUPOBS_LABEL;
			}
			if(s_par1 != s_par2 || s_par1 != s_list){
				result = -1;
				goto SUPOBS_LABEL;
			}

			for(k = 0; k < s_list; k ++){              
				if(instatelist(state1, par1[k].next, par1[k].numelts)){
					inode[j].next1[k] = true; 
				}
				else inode[j].next1[k] = false;

				if(state2 < s2){
					if(instatelist(state2, par2[k].next, par2[k].numelts)){
						inode[j].next2[k] = true; 
					}else inode[j].next2[k] = false; 
				}else{
					inode[j].next2[k] = false;
				}                                               
			}           
		}
	}

	/*step 6 :Veryfy the observability according to the checking table*/
//	iteration ++;
	//print_table(s_table,table, s_list, list, iteration);

	bObs = true;
	for(i = 0; i < s_table; i ++){
		inode = table[i].next;        
		for(k = 0; k < s_list; k ++){
			found = 0;      
			if(table[i].numelts > 1){
				for(j = 0; j < table[i].numelts; j ++){                  
					if((inode[j].next1[k] == true) && (inode[j].next2[k] == true)){
						found = 1;
					}
				}    
			}
			if(found == 1){
				for(j = 0; j < table[i].numelts; j ++){              
					if(inode[j].next1[k] == true){
						if(inode[j].next2[k] == false){                    
							bObs = false;
							event = list[k];
							//state2 = inode[j].data2;
							//state = table[i].state;
							//Tpos = table[i].Gstate;
							goto Remove_Tran; // keep i, k
						}                    
					}
				}
			}
		}
	}
Remove_Tran:
	if(!bObs){
			//zprint_map(s_macrosets, macrosets);
			// delete enabled transitions
			bDel = false;
	/*		for(l = 0; l < macrosets[state].numelts; l ++){
				ss = macrosets[state].next[l];
				ss = macro_c[ss] / s8;
				if(ss < s2){
					if(event == 1001){
						if(t2[ss].marked){
							t2[ss].marked = false;
						}
					}else {
						DeleteOrdlist(event, &t2[ss].next, &t2[ss].numelts);
					}
					bDel = true;
				}
			}*/
			for(j = 0; j < table[i].numelts; j ++){
				state2 = inode[j].data2;
				if(state2 < s2){
					if(event == 1001){
						if(t2[state2].marked){
							t2[state2].marked = false;
						}
					}else {
						DeleteOrdlist(event, &t2[state2].next, &t2[state2].numelts);
					}
					bDel = true;
				}
			}
			new_trim1(&s2, &t2);
			if(!bDel)
				bObs = true;
		//}
	}
//REPEAT:
	if(!bObs){
		freedes(s3, &t3); s3 = 0; t3 = NULL;
		//freedes(s4, &t4); s4 = 0; t4 = NULL;
		//freedes(s5, &t5); s5 = 0; t5 = NULL;
		freedes(s6, &t6); s6 = 0; t6 = NULL;
		freedes(s7, &t7); s7 = 0; t7 = NULL;
		obs_free_par(&s_par2, &par2); s_par2 = 0; par2 = NULL;
		free_table(s_table, &table); s_table = 0; table = NULL;
		free(macro_c); macro_c = NULL; s_macro_c = 0;
		free(macrosets); macrosets = NULL; s_macrosets = 0;
		free(macro_d); macro_d = NULL; s_macro_d = 0;
		free(macrosets1); macrosets1 = NULL; s_macrosets1 = 0;
		free(tmplist); tmplist = NULL; s_tmplist = 0;
		goto START;
	}else{
		//reach(&s2, &t2);
		trim1(&s2, &t2);
		init = 0L;
		filedes(name4, s2, init, t2);
	}

SUPOBS_LABEL:
	free(macro_b);
	free(macro_c);
	free(macro_d);
	free(macrosets);
	free(macrosets1);
	free(mapstate);
	free(list);
	free(tmplist);
	freedes(s1, &t1);
	freedes(s2, &t2);
	freedes(s3, &t3);
	freedes(s4, &t4);
	//freedes(s5, &t5);     
	freedes(s6, &t6);    
	freedes(s7, &t7);
	freedes(s8, &t8);
	obs_free_par(&s_par1, &par1);
	obs_free_par(&s_par2, &par2);
	free_table(s_table, &table);
	return result; 
}
// This function implements the transition-based algorithm for computing supremal relatively observable sublanguage
//This function compute the the supremal C-obsevable (wrt. G) sublanguage of K, with specified events.
//G, K, C are represented by name1, name2, name3 respectively. 
INT_OS transition_suprobs_proc(char * name4, char* name1, char* name2, char *name3,INT_T s_contr_list, INT_T *contr_list, INT_T s_nulllist, INT_T *nulllist, INT_T s_imagelist, INT_T* imagelist, INT_OS mark_flag)
{
	state_node *t1, *t2, *t3, *t4, *t6, *t7, *t8;
	INT_S s1, s2, s3, s4, s6, s7, s8, init;
	INT_S s_macro_c, s_macro_b, s_macro_ab, * macro_c, * macro_b, s_macro_d, *macro_d, *macro_ab;     
	INT_S s_macrosets;//, s_macrosets1;
	state_map * macrosets;//, *macrosets1;
	part_node * par1, *par2;
	INT_S s_par1, s_par2;
	INT_S i, j, k;//, l;
	INT_T s_list, *list;
	INT_T event, found;
	INT_B  ok, bObs, bDel;
	INT_S s_table, iteration, Tpos; obs_table *table;
	INT_S state1, state2;//, state;//, ss, l1, l2;//, nstate1;//, nstate2;

	//     INT_B  *item1, *item2;
	item_node *inode;
	INT_S * mapstate;
	//     FILE * f1;
	INT_OS result;

	t1 = t2 = t3 = t4 = t6 = t7 = t8 = NULL;
	s1 = s2 = s3 = s4 = s6 = s7 = s8 = 0;
	s_macro_c = s_macro_b = s_macro_d = s_macro_ab = 0; macro_c = macro_b = macro_d = macro_ab = NULL;
	s_macrosets = /*s_macrosets1 =*/ 0; macrosets = /*macrosets1 =*/ NULL;
	par1 = par2 = NULL;
	s_par1 = s_par2 = 0;
	s_list = 0; list = NULL;
	s_table = 0; table = NULL;
	mapstate = NULL;
	bObs = false;
	result = 0;

	iteration = 0;
	Tpos = -1;

	/*step1 : get des1 and des2*/
	init = 0L;
	if(getdes(name1, &s1, &init, &t1) == false)
		return -1;

	 
	init = 0L;
	if(getdes(name2, &s3, &init, &t3) == false)
		return -1;
	//Compute Normal form NK of K   NK = K || PK
	export_copy_des(&s4, &t4, s3, t3);
	plain_project_proc(&s4, &t4, s_nulllist, nulllist);
	sync2(s3,t3,s4,t4, &s2, &t2, &macro_ab, &macro_c);
	free(macro_ab); free(macro_c);
	freedes(s3, &t3); freedes(s4, &t4);
	macro_ab = macro_c = NULL;
	s3 = s4 = 0; t3 = t4 = NULL;

	init = 0L;    
	if(getdes(name3, &s3, &init, &t3) == false)
		return -1;

	// Compute G * C
	obs_meet(s1, t1, s3, t3, &s8, &t8,&s_macro_b, &macro_b);
	freedes(s3, &t3); s3 = 0;  t3 = NULL;

	s_list = s_contr_list;
	list = (INT_T*)CALLOC(s_list, sizeof(INT_T));
	memcpy(list, contr_list, s_list*sizeof(INT_T));
	if(mark_flag){  //If marking behavior is considered, add m = 1001 to the list		
		addordlist(1001, &list, s_list, &ok);
		if(ok) s_list ++;
	}

	s_par1 = s_list;
	par1 = (part_node*)CALLOC(s_par1 , sizeof(part_node));
	if(par1 == NULL){
		mem_result = 1;
		goto SUPOBS_LABEL;
	}    
	for(i = 0; i < s1; i ++){
		for(j = 0; j < t1[i].numelts; j ++){
			event = t1[i].next[j].data1;
			k = obs_getindex(event, list, s_list);
			addstatelist(i, &(par1[k].next), par1[k].numelts, &ok);
			if(ok) par1[k].numelts ++;   
		}
		if(t1[i].marked){           
			event = 1001;
			k = obs_getindex(event, list, s_list);
			addstatelist(i, &(par1[k].next), par1[k].numelts, &ok);
			if(ok) par1[k].numelts ++; 
		}
	}

START:

	// add dump state to des2 and extend its transition to full function
	export_copy_des(&s4, &t4, s2, t2);
	full_function(&s4, &t4, s_list, list);
	/*step2 : des3 = meet(des1, des2)*/     
	obs_meet(s8, t8, s4, t4, &s3, &t3,&s_macro_c, &macro_c);	
	if(mem_result == 1){
		result = -1;
		goto SUPOBS_LABEL;
	}

	if(s3 == 0){
		init = 0L;
		filedes(name4, s3, init, t3);
		goto SUPOBS_LABEL;
	}else if(s3 == 1 && t3[0].marked == false){
		init = 0L;
		filedes(name4, 0, init, NULL);
		goto SUPOBS_LABEL;
	}
          
	/*step3 : project des3 and get the partition (Ps = ps')*/
	obs_project(&s3, &t3, s_nulllist, nulllist, &s_macrosets, &macrosets);    

	/*step 4 :Compute the state subsets Q1 and Q2*/

	s_par2 = s_list;
	par2 = (part_node*)CALLOC(s_par2, sizeof(part_node));
	if(par2 == NULL){
		mem_result = 1;
		goto SUPOBS_LABEL;
	}
	for(i = 0; i < s2; i ++){
		for(j = 0; j < t2[i].numelts; j ++){
			event = t2[i].next[j].data1;
			k = obs_getindex(event, list, s_list);
			if(k == -1){
				result = -1;
				goto SUPOBS_LABEL;
			}
			addstatelist(i, &(par2[k].next), par2[k].numelts, &ok);
			if(ok) par2[k].numelts ++; 
		}
		if(t2[i].marked){
			event = 1001;
			k = obs_getindex(event, list, s_list);
			if(k == -1){
				result = -1;
				goto SUPOBS_LABEL;
			}
			addstatelist(i, &(par2[k].next), par2[k].numelts, &ok);
			if(ok) par2[k].numelts ++; 
		}
	}
	/*step 5 :Construct the checking table*/
	s_table = 0;
	table = (obs_table*)CALLOC(s_table + 1, sizeof(obs_table));
	if(table == NULL){
		mem_result = 1;
		goto SUPOBS_LABEL;
	}
	for(i = 0; i < s_macrosets; i ++){        
		if(macrosets[i].numelts > 1){           
			table = (obs_table*)REALLOC(table, (s_table + 1) * sizeof(obs_table));           
			if(table == NULL){
				mem_result = 1;
				goto SUPOBS_LABEL;
			}
			table[s_table].numelts = 0; 
			table[s_table].next = NULL;                    
			for(j = 0; j < macrosets[i].numelts; j ++){
				k = macrosets[i].next[j];              
				state1 = macro_c[k] % s8;
				state1 = macro_b[state1] % s1;
				state2 = macro_c[k] / s8;                           
				table[s_table].numelts ++;                            
				table[s_table].next = (item_node*)REALLOC(table[s_table].next, table[s_table].numelts * sizeof(item_node));  
				if(table[s_table].next == NULL){
					mem_result = 1;
					goto SUPOBS_LABEL;
				} 
				table[s_table].next[j].data1 = state1;
				table[s_table].next[j].data2 = state2;
			}
			s_table ++;          
		}
	}  
	for( i = 0; i < s_table; i ++){
		inode = table[i].next;            
		for(j = 0; j < table[i].numelts; j ++){
			state1 = inode[j].data1;
			state2 = inode[j].data2;           
			inode[j].numelts1 = (INT_T)s_par1;
			inode[j].numelts2 = (INT_T)s_par2;   
			inode[j].next1 = NULL;        
			inode[j].next1 = (INT_B *)REALLOC(inode[j].next1, inode[j].numelts1 * sizeof(INT_B ));
			inode[j].next2 = NULL;
			inode[j].next2 = (INT_B *)REALLOC(inode[j].next2, inode[j].numelts2 * sizeof(INT_B ));
			if(inode[j].next1 == NULL || inode[j].next2 == NULL){
				mem_result = 1;
				goto SUPOBS_LABEL;
			}
			if(s_par1 != s_par2 || s_par1 != s_list){
				result = -1;
				goto SUPOBS_LABEL;
			}

			for(k = 0; k < s_list; k ++){              
				if(instatelist(state1, par1[k].next, par1[k].numelts)){
					inode[j].next1[k] = true; 
				}
				else inode[j].next1[k] = false;

				if(state2 < s2){
					if(instatelist(state2, par2[k].next, par2[k].numelts)){
						inode[j].next2[k] = true; 
					}else inode[j].next2[k] = false; 
				}else{
					inode[j].next2[k] = false;
				}                                               
			}           
		}
	}

	/*step 6 :Verify the observability according to the checking table*/
	iteration ++;
	print_table(s_table,table, s_list, list, iteration);

	bObs = true;
	for(i = 0; i < s_table; i ++){
		inode = table[i].next;        
		for(k = 0; k < s_list; k ++){
			found = 0;      
			if(table[i].numelts > 1){
				for(j = 0; j < table[i].numelts; j ++){                  
					if((inode[j].next1[k] == true) && (inode[j].next2[k] == true)){
						found = 1;
					}
				}    
			}
			if(found == 1){
				for(j = 0; j < table[i].numelts; j ++){              
					if(inode[j].next1[k] == true){
						if(inode[j].next2[k] == false){                    
							bObs = false;
							event = list[k];
							goto Remove_Tran; // i and k are not changed
						}                    
					}
				}
			}
		}
	}
Remove_Tran:
	if(!bObs){
		// delete transitions causing observation inconsistency
		bDel = false;
		for(j = 0; j < table[i].numelts; j ++){
			state2 = inode[j].data2;
			//if(inode[j].next2[k] == true){
				if(state2 < s2){
					if(event == 1001){
						if(t2[state2].marked){
							t2[state2].marked = false;
						}
					}else {
						DeleteOrdlist(event, &t2[state2].next, &t2[state2].numelts);
					}
					bDel = true;
				}
			//}
		}
		new_trim1(&s2, &t2);
		if(!bDel)
			bObs = true;
	}
	if(!bObs){
		freedes(s3, &t3); s3 = 0; t3 = NULL;
		freedes(s6, &t6); s6 = 0; t6 = NULL;
		freedes(s7, &t7); s7 = 0; t7 = NULL;
		obs_free_par(&s_par2, &par2); s_par2 = 0; par2 = NULL;
		free_table(s_table, &table); s_table = 0; table = NULL;
		free(macro_c); macro_c = NULL; s_macro_c = 0;
		free(macrosets); macrosets = NULL; s_macrosets = 0;
		free(macro_d); macro_d = NULL; s_macro_d = 0;
//		free(macrosets1); macrosets1 = NULL; s_macrosets1 = 0;
		goto START;
	}else{
		trim1(&s2, &t2);
		init = 0L;
		filedes(name4, s2, init, t2);
	}

SUPOBS_LABEL:
	free(macro_b);
	free(macro_c);
	free(macro_d);
	free(macrosets);
	//free(macrosets1);
	//free(mapstate);
	free(list);
	freedes(s1, &t1);
	freedes(s2, &t2);
	freedes(s3, &t3);
	freedes(s4, &t4);   
	freedes(s6, &t6);    
	freedes(s7, &t7);
	freedes(s8, &t8);
	obs_free_par(&s_par1, &par1);
	obs_free_par(&s_par2, &par2);
	free_table(s_table, &table);
	return result; 
}
// This function implements a language-based algorithm for computing supremal relatively observable sublanguage
//This function compute the the supremal C-obsevable (wrt. G) sublanguage of K, with specified events.
//G, K, C are represented by name1, name2, name3 respectively. 
INT_OS language_suprobs_proc(char * name4, char* name1, char* name2, char *name3, INT_T s_contr_list, INT_T *contr_list, INT_T s_nulllist, INT_T *nulllist, INT_T s_imagelist, INT_T* imagelist, INT_OS mark_flag)
{
	state_node *t0, *t1, *t2, *t3, *t4, *t5, *t6, *t7, *t8,*t9, *_t3, *_t1, *_t2, *_t0, *_t4;
	INT_S s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, _s3, _s1, _s2, _s0, _s4, init;
	INT_S s_macro_b, * macro_b, *macro_ab, *macro_c;     
	INT_S i;//, j;
	INT_T s_list,  *list;
	INT_B fix_flag, ok;
	INT_S * mapState;
	INT_OS result;
	char tmp_name[MAX_FILENAME];
	char long_tmp_name[_MAX_PATH];

#if defined(_x64_)
	unsigned long long *macro64_c;
	macro64_c = NULL;
#endif

	s_list = 0; list = NULL;
	t0 = t1 = t2 = t3 = t4 = t5 = t6 = t7 = t8 = t9 = _t0 = _t3 = _t1 = _t2 = _t4 = NULL;
	s0 = s1 = s2 = s3 = s4 = s5 = s6 = s7 = s8 = s9 = _s0 = _s3 = _s1 = _s2 = _s4 = 0;
	s_macro_b = 0; macro_b = macro_ab = macro_c = NULL;
	mapState = NULL;
	result = 0;


	// The algorithms are referred to the WODES paper on language-based algorithm
	//Step1 : get des1 and des2
	// Get plant
	init = 0L;
	if(getdes(name1, &s1, &init, &t1) == false)
		return -1;

	// get legal language K0
	init = 0L;    
	if(getdes(name2, &s2, &init, &t2) == false)
		return -1;

	// get ambient language C
	init = 0L;    
	if(getdes(name3, &s3, &init, &t3) == false)
		return -1;

	gentranlist(s1,t1,&s_list,&list);

	// (_s1, _t1) stores the null list
	_s0 = 1;
	_t0 = newdes(_s0);
	for(i = 0; i < s_nulllist;i ++){
		addordlist1(nulllist[i],0,&_t0[0].next, _t0[0].numelts, &ok);
		if(ok) _t0[0].numelts ++;
	}
	_t0[0].marked = 1;	

	// Sigma* = (s4, t4)
	allevent_des(&t1,&s1,&_t4, &_s4);

	// L(G)_bar = (_s1, _t1)
	export_copy_des(&_s1, &_t1, s1, t1); 
	for(i = 0; i < _s1; i ++)
		_t1[i].marked = true;

	// Store K into des2, store Kn into des0
	export_copy_des(&_s2, &_t2, s2, t2); 

	// Compute C_bar = (_s3, _t3)
	export_copy_des(&_s3, &_t3, s3, t3); 
	for(i = 0; i < _s3; i ++)
		_t3[i].marked = true;

	// Compute C_bar.sigma for each sigma and store them into a DES file
	for(i = 0; i < s_contr_list; i ++){
		sprintf(tmp_name, "AMBIENT$$$$%d",i);
		export_copy_des(&s4, &t4, _s3, _t3);
		//zprintn(s4);zprints(" - ");
		catenation_sigma(&s4, &t4, contr_list[i]);
		//zprintn(s4);zprints("\n");
		if(mem_result == 1){
			result = -1;
			goto FINISH;
		}
		init = 0L;
		filedes(tmp_name, s4, init, t4);
		freedes(s4, &t4); s4 = 0; t4 = NULL;
	}

	fix_flag = false;

START:
	//Step 1: Compute Aj'=L(G)\cap co(K_j-1_bar)
	export_copy_des(&s4, &t4, _s2, _t2);
	for(i = 0; i < s4; i ++)
		t4[i].marked = true;
	complement1(&s4, &t4, s_list, list);
#if defined(_x64_)
	meet_x64(_s1,_t1,s4,t4,&s5,&t5,&macro64_c); // Aj' = (s5, t5)
	free(macro64_c); macro64_c = NULL;
#else
	obs_meet(_s1, _t1, s4, t4, &s5, &t5, &s_macro_b, &macro_b); // Aj' = (s5, t5) 
	free(macro_b); s_macro_b = 0; macro_b = NULL;
#endif
	
	if(mem_result == 1){
		result = -1;
		goto FINISH;
	}
	freedes(s4, &t4); s4 = 0; t4 = NULL;

	trim1(&s5, &t5);

	//export_copy_des(&_s0, &_t0, s0, t0); // for small loop
	//Step2: for each sigma, compute B_j(sigma)"
	for(i = 0; i < s_contr_list; i ++){

		// Get C_bar.sigma
		sprintf(tmp_name, "AMBIENT$$$$%d",i);
		init = 0L;
		if(getdes(tmp_name, &s4, &init, &t4) == false){
			result = -1;
			goto FINISH;
		}
		
		// compute Bj(sigma)
#if defined(_x64_)
		meet_x64(s4,t4,s5,t5,&s6,&t6,&macro64_c); 
		free(macro64_c); macro64_c = NULL;
#else
		obs_meet(s4, t4, s5, t5, &s6, &t6, &s_macro_b, &macro_b); 
		free(macro_b); s_macro_b = 0; macro_b = NULL;
#endif
		
		if(mem_result == 1){
			result = -1;
			goto FINISH;
		}
		trim1(&s6, &t6);

		complement1(&s6, &t6, s_list, list); //(Bj(sigma) = (s6, t6))
		//filedes("TEST6", s6, 0, t6);

		// compute Cj'(sigma)
		//project0(&s6, &t6, s_nulllist, nulllist);
		//sync2(s6, t6, _s0, _t0, &s7, &t7, &macro_ab, &macro_c);
		suprema_normal1(t6, s6, _t4, _s4, &t7, &s7, nulllist, s_nulllist);
		if(mem_result == 1){
			result = -1;
			goto FINISH;
		}
		freedes(s6, &t6); s6 = 0; t6 = NULL;
		//zprintn(s7);zprints("\n");
		complement1(&s7, &t7, s_list, list); //(Bj(sigma) = (s6, t6))
		trim1(&s7, &t7);
		//filedes("TEST7", s7, 0, t7);
#if defined(_x64_)
		meet_x64(s4,t4,s7,t7,&s8,&t8,&macro64_c); 
		free(macro64_c); macro64_c = NULL;
#else
		obs_meet(s4, t4, s7, t7, &s8, &t8, &s_macro_b, &macro_b);
		free(macro_b); s_macro_b = 0; macro_b = NULL;
#endif		
		if(mem_result == 1){
			result = -1;
			goto FINISH;
		}

		freedes(s7, &t7); s7 = 0; t7 = NULL;
		complement1(&s8, &t8, s_list, list); //(Bj(sigma) = (s6, t6))
		trim1(&s8, &t8);
		//filedes("TEST8", s8, 0, t8);

		// compute D_j = (s0, t0)
		if(i == 0) {
			export_copy_des(&s0, &t0, s8, t8);
		}else{
			export_copy_des(&s9, &t9, s0, t0);
			freedes(s0, &t0); s0 = 0; t0 = NULL;
#if defined(_x64_)
			meet_x64(s8, t8, s9, t9, &s0, &t0, &macro64_c); 
			free(macro64_c); macro64_c = NULL;
#else
			obs_meet(s8, t8, s9, t9, &s0, &t0, &s_macro_b, &macro_b);
			free(macro_b); s_macro_b = 0; macro_b = NULL;
#endif
			if(mem_result == 1){
				result = -1;
				goto FINISH;
			}
		}
		minimize(&s0, &t0);
		freedes(s4, &t4);
		//freedes(s5, &t5); 
		freedes(s6, &t6);
		freedes(s7, &t7);
		freedes(s8, &t8);
		freedes(s9, &t9);

		t4 = t6 = t7 = t8 = t9 = NULL;
		s4 = s6 = s7 = s8 = s9 = 0;

	}
	freedes(s5, &t5); s5 = 0; t5 = NULL;
//	minimize(&s0, &t0);
//	filedes("TEST0", s0, 0, t0);

	//compute supF(D_j)
	suprema_closed(&s0, &t0);
//	filedes("TEST0", s0, 0, t0);
	// Ej = supF(D_j) \cap \bar{K_j-1} 
	// Fj = E_j \cap K_j-1 = supF(D_j) \cap K_j-1 = (s4,t4)
	export_copy_des(&s5, &t5, _s2, _t2);
#if defined(_x64_)
	meet_x64(s5, t5, s0, t0, &s4, &t4, &macro64_c); 
	free(macro64_c); macro64_c = NULL;
#else
	obs_meet(s5, t5, s0, t0, &s4, &t4, &s_macro_b, &macro_b);
	free(macro_b); s_macro_b = 0; macro_b = NULL;
#endif
	if(mem_result == 1){
		result = -1;
		goto FINISH;
	}
	freedes(s5, &t5); s5 = 0; t5 = NULL;
	trim1(&s4, &t4);
	filedes("TEST4", s4, 0, t4);

	if(mark_flag == 1){

		// Step4: K_j = supnorm(Dj, C_bar \cap Lm(G))
#if defined(_x64_)
		meet_x64(s1, t1, _s3, _t3, &s5, &t5, &macro64_c); 
		free(macro64_c); macro64_c = NULL;
#else
		obs_meet(s1, t1, _s3, _t3, &s5, &t5, &s_macro_b, &macro_b);
		free(macro_b); s_macro_b = 0; macro_b = NULL;
#endif
		if(mem_result == 1){
			result = -1;
			goto FINISH;
		}
		//filedes("TEST", s5, 0, t5);
		suprema_normal(t4, s4, t5, s5, &t6, &s6, nulllist, s_nulllist);	
		if(mem_result == 1){
			result = -1;
			goto FINISH;
		}
		trim1(&s6, &t6);
	}else{
		export_copy_des(&s6, &t6, s4, t4);
	}

	minimize(&s6, &t6);
	if(mem_result == 1){
		result = -1;
		goto FINISH;
	}

	//filedes("TEST", s6, 0, t6);

	if(_s2 == 0 && s6 == 0){
		goto FINISH;
	}

	/* Need some memory here - Allocate map state */
	if(_s2 > 0){
		mapState = (INT_S*) CALLOC(_s2, sizeof(INT_S));
		memset(mapState, -1, sizeof(INT_S)*(_s2));
		mapState[0] = 0;
	}
	iso1(_s2, s6, _t2, t6, &fix_flag, mapState);
	free(mapState); mapState = NULL;
	if(fix_flag){
		goto FINISH;
	}else{
		freedes(_s2, &_t2); _s2 = 0; _t2 = NULL;
		export_copy_des(&_s2, &_t2, s6, t6);
		freedes(s4, &t4);
		freedes(s5, &t5); 
		freedes(s6, &t6);
		freedes(s7, &t7);
		freedes(s8, &t8);
		freedes(s9, &t9);

		t4 = t5 = t6 = t7 = t8 = t9 = NULL;
		s4 = s5 = s6 = s7 = s8 = s9 = 0;
		goto START;
	}
FINISH:
	if(mem_result != 1){
		init = 0L;
		filedes(name4, s6, init, t6);
	}

	// remove the temp DES files storing C_bar.sigma
	for(i = 0; i < s_contr_list; i ++){
		sprintf(tmp_name, "AMBIENT$$$$%d",i);
		make_filename_ext(long_tmp_name,tmp_name,EXT_DES);
		if(exist(long_tmp_name)){
			remove(long_tmp_name);
		}
	}

#if defined(_x64_)
	free(macro64_c);
#endif

	free(macro_b);
	free(macro_ab);
	free(macro_c);
	free(mapState);
	free(list);
	freedes(s0, &t0);
	freedes(s1, &t1);
	freedes(s2, &t2);
	freedes(s3, &t3);
	freedes(s4, &t4);
	freedes(s5, &t5);     
	freedes(s6, &t6);    
	freedes(s7, &t7);
	freedes(s8, &t8);
	freedes(s9, &t9);
	freedes(_s0, &_t0);
	freedes(_s1, &_t1);
	freedes(_s2, &_t2);
	freedes(_s3, &_t3);
	freedes(_s4, &_t4);
	return result; 
}

INT_OS supcon_runProgram(char *name3, char *name1, char *name2)
{
    FILE *f1;
    const char ctct_prm[] = "ctct.prm";

    f1 = fopen(ctct_prm, "w");
    if (f1 == NULL)
       return -1;  /* Some type of system error */
    INT_OS debug_mode = 0;
    INT_OS minflag = 1;
    fprintf(f1, "%s\n", name1);
    fprintf(f1, "%s\n", name2);
    fprintf(f1, "%s\n", name3);
    fclose(f1);
    
	INT_OS result = supcon_program(ctct_prm);
    remove(ctct_prm);
    return result;
}

INT_OS supconrobs_proc(char * name3, char* name1, char* name2, INT_T slist, INT_T *list, INT_T s_imagelist, INT_T* imagelist)
{
	INT_S s1, s2, s3, s4, init;
	state_node *t1, *t2, *t3, *t4;	
	char name4[MAX_FILENAME], long_name4[MAX_FILENAME];
	INT_S *mapState, *macro_ab, *macro_c;
	INT_B fix_flag;

	INT_OS result;

	s1 = s2 = s3 = s4 = 0;
	t1 = t2 = t3 = t4 = NULL;
	mapState = macro_ab = macro_c = NULL;

	result = 0;
	
	strcpy(name4, "$$$$$$");
	result = supcon_runProgram(name4, name1, name2);
	if(result != RESULT_OK){
		if(result == ERR_MEM)
			mem_result = 1;
		goto FREE_MEM;
	}
	init = 0L;
	if(getdes(name4, &s4, &init, &t4) == false){
		result = -1;
		goto FREE_MEM;
	}  

	// Compute normal form of C = K0: NK = K0 || PK0

	export_copy_des(&s3, &t3, s4, t4);
	plain_project_proc(&s3, &t3, slist, list);
	sync2(s3,t3,s4,t4, &s2, &t2, &macro_ab, &macro_c);
	//filedes("TEST",s2,0,t2);
	free(macro_ab); 
	free(macro_c);
	freedes(s3, &t3); freedes(s4, &t4);
	macro_ab = macro_c = NULL;
	s3 = s4 = 0; t3 = t4 = NULL;

	init = 0L;
	filedes(name4, s2, init, t2);
	freedes(s2, &t2); s2 = 0; t2 = NULL;


	fix_flag = false;

	while(!fix_flag){
		// Obtain K_i to compare with K_i+1
		init = 0L;
		if(getdes(name4, &s2, &init, &t2) == false){
			result = -1;
			goto FREE_MEM;
		}  

		//compute supremal relative observable sublanguage
		if(supobs_proc3(name3, name1, name4, slist, list, s_imagelist, imagelist) != 0){
			result = -1;
			goto FREE_MEM;
		}
		

		// compute supremal controllabel sublanguage
		result = supcon_runProgram(name4, name1, name3);
		if(result != RESULT_OK){
			if(result == ERR_MEM)
				mem_result = 1;
			goto FREE_MEM;
		}


		init = 0L;
		if(getdes(name4, &s3, &init, &t3) == false){
			result = -1;
			goto FREE_MEM;
		}  

		minimize(&s2, &t2);
		minimize(&s3, &t3);

		if(s2 == 0 && s3 == 0){
			fix_flag = true;
			break;
		}

		/* Need some memory here - Allocate map state */
		if(s2 != 0){
			mapState = (INT_S*) CALLOC(s2, sizeof(INT_S));
			memset(mapState, -1, sizeof(INT_S)*(s2));
			mapState[0] = 0;
		}
		iso1(s2, s3, t2, t3, &fix_flag, mapState);
		free(mapState); mapState = NULL;
		if(fix_flag){
			break;
		}/*else{
		 init = 0L;
		 filedes(name4, s3, init, t3);
		 }*/
		freedes(s2, &t2);
		freedes(s3, &t3);
		s2 = s3 = 0; 
		t2 = t3 = NULL;
	}

	strcpy(long_name4, "");
	make_filename_ext(long_name4, name4, EXT_DES);
	if(exist(long_name4)){
		init = 0L;
		getdes(name4, &s3, &init, &t3);
		init = 0L;
		filedes(name3, s3, init, t3);
		remove(long_name4);
	}	

FREE_MEM:
	freedes(s1, &t1);
	freedes(s2, &t2);
	freedes(s3, &t3);
	freedes(s4, &t4);
	free(mapState);
	free(macro_c);
	free(macro_ab);

	return result;
}