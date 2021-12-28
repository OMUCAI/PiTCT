#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <ctype.h>
// #include <io.h>

#include "setup.h"
#include "des_data.h"
#include "des_proc.h"
#include "des_supp.h"
// #include "curses.h"
#include "mymalloc.h"
#include "localize.h"
#include "supred.h"
// #include "ex_supred.h"
#include "tct_io.h"
// #include "cl_tct.h"
#include "minm.h"
#include "obs_check.h"
#include "program.h"

static INT_B mark = false;//used to check whether use mark information

// This structure is used to store the enablement/disablement information of a controllable event at a cell of states. 
//If control bit is 1, the controllable event is enabled; if it is 0, it is disabled; otherwise, it is undecided. 
//The middle two variables are used to store the states in a cell and the latter two variables the transitions occurred on 
//these states.
typedef struct x_state_node {
   INT_OS control;
   INT_S s_numelts;
   INT_S *s_next;
   INT_T t_numelts;
   tran_node *t_next;
} x_state_node;

void free_x_part(INT_S size, x_state_node **data)
{
    INT_S i;
    
    if (*data == NULL) return;
    if (size <= 0) return;
    
    for(i = 0; i < size; i ++){
       if ((*data)[i].s_next != NULL) {
          free((*data)[i].s_next);
          (*data)[i].s_next = NULL;
       }
       if ((*data)[i].t_next != NULL) {
          free((*data)[i].t_next);
          (*data)[i].t_next = NULL;
       }       
    }
    
    if (*data != NULL) 
       free(*data);
    *data = NULL;
}

void zprint_x_par(INT_S s, x_state_node *par)
{
     FILE *out;
     char tmp_result[MAX_PATH];
	 INT_S i,j;
     
     strcpy(tmp_result, "");
     strcat(tmp_result, prefix);
     strcat(tmp_result, "Ouput.txt");
     out = fopen(tmp_result,"a");
     
     
     for(i = 0; i < s; i ++){
         for(j = 0; j < par[i].s_numelts; j ++){
               fprintf(out,"%d ", par[i].s_next[j]);
         }
         fprintf(out, ": ");
         for(j = 0; j < par[i].t_numelts; j ++)
            fprintf(out, "(%d  %d) ", par[i].t_next[j].data1, par[i].t_next[j].data2);
         fprintf(out,"\n");
     }
     fclose(out);
}

void loc_copy_des( INT_S *s_dest,state_node **t_dest,
              INT_S s_src,state_node *t_src   )
{
    INT_S i, jj;
    INT_T j, ee;
    INT_B ok;

    *s_dest = s_src;
    *t_dest = newdes(s_src);

    if ((s_src !=0) && (*t_dest == NULL)) {
      mem_result = 1;
      return;
    }

    for (i=0; i < s_src; i++) {
      (*t_dest)[i].marked  = t_src[i].marked;
      (*t_dest)[i].reached = t_src[i].reached;
      (*t_dest)[i].coreach = t_src[i].coreach;
      (*t_dest)[i].vocal   = t_src[i].vocal;
      for (j=0; j < t_src[i].numelts; j++) {
         ee = t_src[i].next[j].data1;
         jj = t_src[i].next[j].data2;
         addordlist1(ee, jj, &(*t_dest)[i].next, (*t_dest)[i].numelts, &ok);
         if (ok) (*t_dest)[i].numelts++;
      }
    }
}
void loc_genconlist(INT_S s, state_node *t, INT_T *s_list, INT_T **list)
{
     /* Generate a list of all transition controllable labels used in DES */
     INT_S i,j;
     INT_B ok;
     
     ok = false;
     *s_list = 0;
     *list = NULL;
     if (s == 0) return;
     
     for(i = 0; i < s; i ++){
        for(j = 0; j < t[i].numelts; j ++){
            if(t[i].next[j].data1 % 2 != 0){
                addordlist(t[i].next[j].data1,list,*s_list,&ok);
                if(ok) (*s_list) ++;
            }
        }
     }
}
void loc_gentranlist(INT_S s1,
                 state_node *t1,
                 INT_T *s_t1,
                 INT_T **list1)
{
   /* Generate a list of all transition labels used in DES */
   INT_S i;
   INT_T j;
   INT_B ok;

   *s_t1 = 0;
   *list1 = NULL;
   if (s1 == 0L) return;

   for (i=0L; i < s1; i++) {
     for (j=0; j < t1[i].numelts; j++) {
       addordlist(t1[i].next[j].data1, list1, *s_t1, &ok);
       if (ok) (*s_t1)++;
     }
   }
}

void loc_meet_list(INT_T s1, INT_T *list1, INT_T s2, INT_T *list2, INT_T *s, INT_T ** list)
{
    /*get intersection of two lists: list = meet(list1,list2)*/
    
    INT_T i,j;
    INT_B ok;
    
    *s = 0;
    *list = NULL;
    
    for(i = 0; i < s1; i ++){
       for(j = 0; j < s2; j ++){
          if(list1[i] == list2[j]){
             addordlist(list1[i],list,*s,&ok);
             if(ok) (*s) ++;
          }
       }
    }
}
void loc_init_par(INT_S s, INT_S *s_par, part_node **par)
{
    INT_S i;
    INT_B ok;
    
    *s_par = 0; *par = NULL;
    
    *s_par = s;
    *par = (part_node*)CALLOC(*s_par,sizeof(part_node));
    if(*par == NULL){
        mem_result = 1;
        *s_par = 0;
        *par = NULL;
        return;
    } 
    for(i = 0; i < s; i ++){
        addstatelist(i,&(*par)[i].next,(*par)[i].numelts,&ok);
        if(ok) (*par)[i].numelts ++;
    }
}
INT_S loc_min_index(INT_S s_par, part_node *par, INT_S index)
{
    return par[index].next[0];
}

void loc_compute_vari(INT_S swl, state_pair *wl, INT_S index, part_node *par, INT_S *s_list, INT_S **list)
{
    INT_S i,j;
    INT_B ok;
    INT_S data1, data2;
    
    *s_list = 0; 
    *list = NULL;
    
    for(i = 0; i < swl; i ++){
       if(index == wl[i].data1){
          data2 = wl[i].data2;
          for(j = 0; j < par[data2].numelts; j ++){
             addstatelist(par[data2].next[j],list,*s_list,&ok);
             if(ok) (*s_list)++;
          }
       }
       if(index == wl[i].data2){
          data1 = wl[i].data1;
          for(j = 0; j < par[data1].numelts; j ++){
             addstatelist(par[data1].next[j],list,*s_list,&ok);
             if(ok) (*s_list)++;
          }
       }
    }
}
void loc_unionsets(INT_S *list1,
               INT_S size1,
               INT_S **list2,
               INT_S *size2)
{
   /* Form the union: list2 <- list1 + list2 */
   INT_S cur;
   INT_B ok;

   for (cur=0; cur < size1; cur++) {
      addstatelist(list1[cur],list2,*size2,&ok);
      if (ok) (*size2)++;
   }
}
void loc_unionsets1(INT_T *list1,
               INT_T size1,
               INT_T **list2,
               INT_T *size2)
{
   /* Form the union: list2 <- list1 + list2 */
   INT_T cur;
   INT_B ok;

   for (cur=0; cur < size1; cur++) {
      addordlist(list1[cur],list2,*size2,&ok);
      if (ok) (*size2)++;
   }
}
INT_B loc_is_mem(INT_S data1, INT_S data2, INT_S swl, state_pair *wl)
{
    INT_S i;
    
    for(i = 0; i < swl; i++){
       if((data1 == wl[i].data1 && data2 == wl[i].data2) ||
          (data1 == wl[i].data2 && data2 == wl[i].data1))
          return true;
    }
    return false;
}
INT_B loc_is_intersect(INT_S s1, INT_S *list1, INT_S s2, INT_S *list2)
{
    INT_S i,j;
    for(i = 0; i < s1; i ++){
       for(j = 0;j < s2; j ++){
          if(list1[i] == list2[j])
             return true;
       }
    }
    return false;
}
INT_B loc_is_cc(INT_S ptr1, INT_S ptr2, INT_S s_depar, part_node *epar, part_node *dpar, INT_B *tlist, INT_B *mlist)
{
    if(loc_is_intersect(epar[ptr1].numelts, epar[ptr1].next, dpar[ptr2].numelts, dpar[ptr2].next) ||
       loc_is_intersect(epar[ptr2].numelts, epar[ptr2].next, dpar[ptr1].numelts, dpar[ptr1].next))
       return false;
    if((tlist[ptr1] == tlist[ptr2])&& (mlist[ptr1] != mlist[ptr2]))
          return false;
    
    return true;
   /* if(mark){
       if(mlist[ptr1] == mlist[ptr2])
          return true;
    }else{
       if(dpar[ptr1].numelts == 0 && dpar[ptr2].numelts == 0)
          return true;
       if(loc_is_intersect(dpar[ptr1].numelts, dpar[ptr1].next, dpar[ptr2].numelts, dpar[ptr2].next))
          return true;
    }
    return false;*/
}
/* event in the transition, extrance state pair */
INT_S loc_inordlist(INT_T e,             /* event to find */
                   tran_node *L,        /* list to search */
                   INT_T size)
{
   INT_T pos;
   INT_T lower, upper;
   INT_S ss;
   INT_B found;

   /* Do a binary search. */
   ss = 2000;
   found = false;
   pos = 0;
   if (size > 1) {
     lower = 1;
     upper = size;
     while ( (found == false) && (lower <= upper) ) {
       pos = (lower + upper) / 2;
       if (e == L[pos-1].data1) {
          ss = L[pos-1].data2;
          found = true;
       } else if (e > L[pos-1].data1) {
          lower = pos+1;
       } else {
          upper = pos-1;
       }
     }
   } else if (size == 1) {
     if (e == L[0].data1) {
       ss = L[0].data2;
     }
   }

   return ss;                   
}  
                                                       
INT_B loc_equal_list(INT_S *list1, INT_S size1,
                       INT_S *list2, INT_S size2)
{
   INT_S i;
   if(size1 != size2)
      return false;
   for (i=0; i < size1; i++) {
      if (list1[i] != list2[i])
         return false;
   }
   return true;
}

INT_B loc_check_mergebility(INT_S si, INT_S sj, INT_S s_par, part_node *par, INT_S *swl, state_pair **wl, INT_S cnode,
                                    INT_T s_list, INT_T *list, INT_S s1, state_node *t1,
                                    INT_S s_depar, part_node *epar, part_node *dpar,
                                    INT_B *tlist, INT_B *mlist)
{
    INT_S i, j, k;
    INT_S s_list1,s_list2, *list1,*list2;
    INT_S ptr1, ptr2;
    INT_S ee, nptr1, nptr2;
    INT_B ok;
    INT_B check_flag;
    state_pair * twl; INT_S stwl;
    
    s_list1 = s_list2 = 0; list1 = list2 = NULL;
    check_flag = true;
    
    stwl = *swl;
    twl = (state_pair *)CALLOC(stwl,sizeof(state_pair));
    for(i = 0; i < stwl; i ++){
       twl[i].data1 = (*wl)[i].data1;
       twl[i].data2 = (*wl)[i].data2;
    }
     
    loc_compute_vari(stwl,twl,si,par,&s_list1,&list1);
    loc_unionsets(par[si].next, par[si].numelts, &list1, &s_list1);
    if(mem_result == 1)
       goto CHECK_LABEL;    
    for(i = 0; i < s_list1; i ++){
       ptr1 = list1[i];
       free(list2); list2 = NULL; s_list2 = 0;
       loc_compute_vari(stwl,twl,sj,par,&s_list2,&list2);
       loc_unionsets(par[sj].next,par[sj].numelts, &list2, &s_list2);
       if(mem_result == 1)
          goto CHECK_LABEL;
       for(j = 0; j < s_list2; j ++){
          ptr2 = list2[j];   
          if(loc_is_mem(ptr1,ptr2,stwl,twl))
             continue;
         /* if(!loc_is_cc(ptr1,ptr2,s_depar,epar,dpar,tlist,mlist)){
             check_flag = false;
             goto CHECK_LABEL;
          }         */
          addstatepair(ptr1,ptr2,wl,*swl,&ok);
          if(ok) (*swl)++;  
          for(k = 0; k < s_list; k ++){
             ee = list[k];
             nptr1 = loc_inordlist((INT_T)ee,t1[ptr1].next,t1[ptr1].numelts);
             nptr2 = loc_inordlist((INT_T)ee,t1[ptr2].next,t1[ptr2].numelts);          
             if(nptr1 != 2000 && nptr2 != 2000){
                if(loc_equal_list(par[nptr1].next, par[nptr1].numelts, par[nptr2].next, par[nptr2].numelts))
                   continue;
                if(loc_is_mem(nptr1,nptr2,stwl,twl))
                   continue;
                if((loc_min_index(s_par,par,nptr1) < cnode) || 
                   (loc_min_index(s_par,par,nptr2) < cnode)){
                   check_flag = false;
                   goto CHECK_LABEL;
                }
                            
                if(loc_check_mergebility(nptr1,nptr2,s_par,par,swl,wl,cnode,s_list,
                     list,s1,t1,s_depar,epar,dpar,tlist,mlist) == false) {  
                   check_flag = false;
                   goto CHECK_LABEL;
                }
             }
          }          
       }
    }
CHECK_LABEL:
    free(list1);
    free(list2);
    free(twl);
    return check_flag;
}
INT_B loc_check_mergebility1(INT_S si, INT_S sj, INT_S s_par, part_node *par, 
                                    INT_S *swl, state_pair **wl, 
                                    INT_S * macro_m, INT_S *macro_cc, INT_S cnode,
                                    INT_T s_list, INT_T *list, INT_S s1, state_node *t1,
                                    INT_S s_depar, part_node *epar, part_node *dpar,
                                    INT_B *tlist, INT_B *mlist)
{
    INT_S i, j, k;
    INT_S s_list1,s_list2,s_list3,s_list4, *list1,*list2,*list3,*list4;
    INT_S ptr1, ptr2;
    INT_S ee, nptr1, nptr2;
    INT_B ok;
    INT_B check_flag, restart;

    
    s_list1 = s_list2 = s_list3 = s_list4 = 0; 
    list1 = list2 = list3 = list4 = NULL;
    check_flag = true;
     
    loc_compute_vari(*swl,*wl,si,par,&s_list1,&list1);
    loc_unionsets(par[si].next, par[si].numelts, &list1, &s_list1);
    loc_compute_vari(*swl,*wl,sj,par,&s_list2,&list2);
    loc_unionsets(par[sj].next,par[sj].numelts, &list2, &s_list2);
    if(mem_result == 1)
       goto CHECK_LABEL;  
START: 
    restart = false;
    free(list3); free(list4);
    s_list3 = s_list4 = 0;
    list3 = list4 = NULL;
    for(i = 0; i < s_list1; i ++){
       ptr1 = list1[i];
       for(j = 0; j < s_list2; j ++){
          ptr2 = list2[j];   
          if(macro_m[ptr1 * s1 + ptr2] == 1 || macro_m[ptr2 * s1 + ptr1] == 1)
             continue;
         /* if(macro_cc[ptr1 * s1 + ptr2] == 0){
             if(!loc_is_cc(ptr1,ptr2,s_depar,epar,dpar,tlist,mlist)){
                check_flag = false;
                macro_cc[ptr1 * s1 + ptr2] = macro_cc[ptr2 * s1 + ptr1] = 1;
                goto CHECK_LABEL;
             }
             macro_cc[ptr1 * s1 + ptr2] = macro_cc[ptr2 * s1 + ptr1] = 2;
          } else if(macro_cc[ptr1 * s1 + ptr2] == 1){
             check_flag = false;
             goto CHECK_LABEL;
          }*/
          macro_m[ptr1 * s1 + ptr2] = 1;
                  
          addstatepair(ptr1,ptr2,wl,*swl,&ok);
          if(ok) {
             (*swl)++;
             restart = true;
             if(ptr1 == si){
               addstatelist(ptr2,&list3,s_list3,&ok);
               if(ok) s_list3++;             
             } else if(ptr1 == sj){
               addstatelist(ptr2,&list4,s_list4,&ok);
               if(ok) s_list4++;
             } else if(ptr2 == si){
               addstatelist(ptr1,&list3,s_list3,&ok);
               if(ok) s_list3++;
             } else if(ptr2 == sj){
               addstatelist(ptr1,&list4,s_list4,&ok);
               if(ok) s_list4++;
             } 
          }  
          
          for(k = 0; k < s_list; k ++){
             ee = list[k];
             nptr1 = loc_inordlist((INT_T)ee,t1[ptr1].next,t1[ptr1].numelts);
             nptr2 = loc_inordlist((INT_T)ee,t1[ptr2].next,t1[ptr2].numelts);          
             if(nptr1 != 2000 && nptr2 != 2000){
                if(loc_equal_list(par[nptr1].next, par[nptr1].numelts, par[nptr2].next, par[nptr2].numelts))
                   continue;
                if(macro_m[nptr1 * s1 + nptr2] == 1 || macro_m[nptr2 * s1 + nptr1] == 1)
                   continue;
                if((loc_min_index(s_par,par,nptr1) < cnode) || 
                   (loc_min_index(s_par,par,nptr2) < cnode)){
                   check_flag = false;
                   goto CHECK_LABEL;
                }                            
                if(loc_check_mergebility1(nptr1,nptr2,s_par,par,swl,wl,macro_m,macro_cc, cnode,s_list,
                     list,s1,t1,s_depar,epar,dpar,tlist,mlist) == false) {  
                   check_flag = false;
                   goto CHECK_LABEL;
                }
             }
          }         
       }
    }
    if(restart && s_list3 + s_list4 != 0) {
       loc_unionsets(list3,s_list3,&list1,&s_list1);
       loc_unionsets(list4,s_list4,&list2,&s_list2);
       goto START;
    }
CHECK_LABEL:
    free(list1);
    free(list2);
    free(list3);
    free(list4);
    return check_flag;
}

void loc_genlist(INT_T slist1, tran_node *list1, INT_T slist2, tran_node *list2, INT_T s_list, INT_T *list, INT_S *s_elist, INT_S **elist)
{
	INT_T j1, j2;
	INT_B ok;

	j1 = j2 = 0;
	while ((j1 < slist1) && (j2 < slist2)) {
		if (list1[j1].data1 == list2[j2].data1) {
			j1++; j2++;
		} else if (list1[j1].data1 > list2[j2].data1) {
			j2++;
		} else {
			if(inlist(list1[j1].data1,list,s_list)){
				addstatelist(list1[j1].data1, elist, *s_elist, &ok);
				if (ok) (*s_elist)++;
			}
			j1++;
		}
	}

	while (j1 < slist1) {
		if(inlist(list1[j1].data1,list,s_list)){
			addstatelist(list1[j1].data1, elist, *s_elist, &ok);
			if (ok) (*s_elist)++;
		}
		j1++;
	}
}
void loc_binary_cc(INT_S s1, state_node *t1, INT_S s2, state_node *t2,
                   INT_T s_conlist, INT_T *conlist,
                   INT_S *s_depar, part_node **epar, part_node **dpar,
                   INT_B **tlist,INT_B **mlist)
{
	INT_S i,j;
	INT_B ok;
	INT_S s3; state_node *t3;
	INT_S *macro_ab,*macro_c;
	INT_S state,state1,state2;
	INT_T ee;

    s3 = 0; t3 = NULL;
    macro_ab = macro_c = NULL;
    (*s_depar) = s2;

	/*Compute E(x) and M(x)*/
    *epar = (part_node*)CALLOC(*s_depar, sizeof(part_node));
    if(*epar == NULL){
       mem_result = 1;
       return;
    } 

    *mlist = (INT_B*)CALLOC(*s_depar,sizeof(INT_B));
	if(*mlist == NULL){
		mem_result = 1;
		return;
	}
    
    for(i = 0; i < s2; i ++){
       for(j = 0; j < t2[i].numelts; j ++){
		   ee = t2[i].next[j].data1;
		   if(inlist(ee, conlist, s_conlist)){
			addstatelist(ee,&(*epar)[i].next,(*epar)[i].numelts, &ok);
			if(ok) (*epar)[i].numelts ++;
		   }
       }
       (*mlist)[i] = 0;
       if(t2[i].marked){
          (*mlist)[i] = 1;
       }
    }    
    
	 /*Compute D(x) and T(x)*/
    *dpar = (part_node*)CALLOC(*s_depar, sizeof(part_node));
    if(*dpar == NULL){
		mem_result = 1;
		return;
    } 
    *tlist = (INT_B*)CALLOC(s1,sizeof(INT_B));
    if(*tlist == NULL){
		mem_result = 1;
		return;
    }  
    for(i = 0; i < s1; i ++)
       (*tlist)[i] = 0;

	meet2(s1,t1,s2,t2,&s3,&t3,&macro_ab,&macro_c);

	for(state = 0; state < s3; state ++){
		state1 = macro_c[state] % s1;
		state2 = macro_c[state] / s1;

		loc_genlist(t1[state1].numelts, t1[state1].next, t3[state].numelts, t3[state].next,s_conlist,conlist,&(*dpar)[state2].numelts,&(*dpar)[state2].next);
		if(t1[state1].marked)
			(*tlist)[state2] = 1;
	}  

	freedes(s3, &t3);
	free(macro_ab);
	free(macro_c);
}

void loc_merge_par(INT_S s_par, part_node *par, INT_S swl, state_pair *wl)
{
    INT_S i,j;
    INT_S ptr1;
    INT_S s_list1, s_list2, *list1, *list2;
    INT_B ok;
    
    s_list1 = s_list2 = 0;
    list1 = list2 = NULL;
    for(i = 0; i < s_par; i ++){
       for(j = 0; j < par[i].numelts; j ++){
          ptr1 = par[i].next[j];
          loc_compute_vari(swl,wl,ptr1,par,&s_list1,&list1);
          loc_unionsets(list1,s_list1,&list2,&s_list2);
          free(list1);
          s_list1 = 0; list1 = NULL;
       }
       for(j = 0; j < s_list2; j ++){
          addstatelist(list2[j],&par[i].next,par[i].numelts,&ok);
          if(ok) par[i].numelts ++;
       }
       free(list2);
       list2 = NULL; s_list2 = 0;
    }
    
}

INT_OS loc_gen_cover(INT_S s_par1, part_node *par1, INT_S *s_par2, part_node **par2)
{
	INT_S i,j;
	INT_S *list;
	INT_S ptr,cur;
	INT_B found, ok;

	list = (INT_S*)CALLOC(s_par1, sizeof(INT_S));
	for(i = 0; i < s_par1; i ++)
		list[i] = -1;

	cur = 0;
	for(i = 0; i < s_par1; i ++){
		found = false;
		for(j = 0; j < par1[i].numelts; j++){
			ptr = par1[i].next[j];
			if(list[ptr] == -1){
				list[ptr] = cur;
				found = true;
			}
		}
		if(found)
			cur ++;
	}
	*s_par2 = cur;
	*par2 = (part_node*)CALLOC(*s_par2,sizeof(part_node));
	if(*par2 == NULL){
		*s_par2 = 0;
		mem_result = 1;
		free(list);
		return -1;
	}
	for(i = 0; i < s_par1; i ++){
		addstatelist(i,&(*par2)[list[i]].next,(*par2)[list[i]].numelts,&ok);
		if(ok)  (*par2)[list[i]].numelts ++;
	}

	cur = 0;
	for(i = 0; i < *s_par2; i ++){
		for(j = 0; j < (*par2)[i].numelts; j ++)
			cur ++;
	}
	if(cur != s_par1){
		free(list);
		return -1;
	}

	free(list);
	return 0;
}
INT_S loc_get_pos(INT_S s, INT_S s_par, part_node *par)
{
	INT_S i,j;
	for(i = 0; i < s_par; i ++){
		for(j = 0; j < par[i].numelts; j ++){
			if(s == par[i].next[j])
				return i;
		}
	}
	return -1;
}

void loc_genlist1(INT_T slist1,
             tran_node *list1,
             INT_T slist2,
             tran_node *list2,
             INT_T s_list, INT_T *list,
             INT_S s,
             state_node *t)
{
   INT_T j1, j2;
   INT_B ok;

   j1 = j2 = 0;
   while ((j1 < slist1) && (j2 < slist2)) {
     if (list1[j1].data1 == list2[j2].data1) {
        j1++; j2++;
     } else if (list1[j1].data1 > list2[j2].data1) {
        j2++;
     } else {
        if(inlist(list1[j1].data1,list,s_list)){
           addordlist1(list1[j1].data1, 0, &t[s].next, t[s].numelts, &ok);
           if (ok) t[s].numelts++;
        }
        j1++;
     }
   }

   while (j1 < slist1) {
     if(inlist(list1[j1].data1,list,s_list)){
        addordlist1(list1[j1].data1, 0, &t[s].next, t[s].numelts, &ok);
        if (ok) t[s].numelts++;
     }
     j1++;
   }
}
void loc_condat1(state_node *t1,
             INT_S s1,
             INT_S s2,
             INT_S s3,
             state_node *t3,
             INT_S *s4,
             state_node **t4,
             INT_T s_list, INT_T *list,
             INT_S *macro_c)
{
   INT_S state, state1, state2;

   *s4 = s2;
   *t4 = newdes(*s4);
   
   for (state=0; state < s3; state++) {
     state1 = macro_c[state] % s1;
     state2 = macro_c[state] / s1;
     
     loc_genlist1(t1[state1].numelts, t1[state1].next,
             t3[state].numelts, t3[state].next, s_list,list,state2, *t4);
   }   
}

void reverse_tran(INT_S *s_dest, state_node **t_dest, INT_S s_src, state_node *t_src)
{    
     INT_S i, jj;
     INT_T j, ee;
     INT_B ok;

     *s_dest = s_src;
     *t_dest = newdes(s_src);

     if ((s_src !=0) && (*t_dest == NULL)) {
        mem_result = 1;
        return;
     }

     for (i=0; i < s_src; i++) {
        for (j=0; j < t_src[i].numelts; j++) {
           ee = t_src[i].next[j].data1;
           jj = t_src[i].next[j].data2;
           addordlist1(ee, i, &(*t_dest)[jj].next, (*t_dest)[jj].numelts, &ok);
           if (ok) (*t_dest)[jj].numelts++;
        }
     }
     
}

INT_OS localize_proc_orig(INT_S sfile, INT_S sloc, 
                        char* name1, char *name2, 
                        char (*names1)[MAX_FILENAME], 
                        char (*names2)[MAX_FILENAME],
                        INT_OS mode)
{
    INT_S s1, s2, s3, s4, s5, s6, s7;
    state_node *t1, *t2, *t3, *t4, *t5, *t6, *t7;
    INT_S init,i;
    INT_S *macro_ab, *macro_c;
    INT_OS result;
    INT_B ok;
    FILE * file;  
//    char temp[MAX_PATH];
    char long_temp_DES[MAX_PATH];
    char long_temp_DAT[MAX_PATH];
    char long_name1[MAX_PATH];
    char long_name2[MAX_PATH];
//    char ch;
    INT_OS x,y;
    char names3[MAX_DESS][MAX_PATH];
    char long_name3[MAX_PATH];
    //INT_S num_of_states1[sloc], num_of_states2[sloc];  
	INT_S *num_of_states1, *num_of_states2;
    INT_T slist, *list, sconlist, *conlist, sctemp, *ctemp;
	INT_S lb; float cr; 
	INT_S min, pos_min;

	INT_T ee; INT_S es; INT_S j, k;
    INT_B de_flag;
    
    t_part_node *par3; 
    INT_S  s_par3;
    
    s1 = s2 = s3 = s4 = s5 = s6 = s7 = 0;
    t1 = t2 = t3 = t4 = t5 = t6 = t7 = NULL;    
    
    macro_ab = macro_c = NULL;
    s_par3 = 0;  par3 = NULL;
    file = NULL;
    slist = sconlist = sctemp = 0; 
    list = conlist = ctemp = NULL;    
	num_of_states1 = (INT_S*)CALLOC(sloc, sizeof(INT_S));
	num_of_states2 = (INT_S*)CALLOC(sloc, sizeof(INT_S));
    
    /*Initial the temp names3 to store the temp DES file generated by Strong Localization*/
    for(i = 0; i < sloc; i ++){
       strcpy(names3[i],"");
       sprintf(names3[i],"###%d",i);
    }
    
    result = 0;

    //remove("G:\\result.txt");

    s_par3 = sfile;
    par3 = (t_part_node*)CALLOC(s_par3,sizeof(t_part_node));
    if(par3 == NULL){
       s_par3 = 0;
       mem_result = 1;
       goto LOC_LABEL;
    }
    
    init = 0L;
    if(getdes(name1,&s1,&init,&t1) == false)
       goto LOC_LABEL; 
    for(i = 0; i < sfile; i ++){
       init = 0L;
       if(getdes(names1[i],&s2,&init,&t2) == false)
          goto LOC_LABEL;
       loc_gentranlist(s2,t2,&par3[i].numelts,&par3[i].next);
       if(mem_result == 1)
          goto LOC_LABEL;
       freedes(s2,&t2);
       s2 = 0; t2 = NULL;
    }

    init = 0L;  
    if(getdes(name2,&s2,&init,&t2) == false)
       goto LOC_LABEL;

    meet2(s1,t1,s2,t2,&s3,&t3,&macro_ab,&macro_c);
    //printw("test");

    ok = false;
    lb = 0; cr = 0.0;
    for(i = 0; i < sloc; i ++){
       lb = 0; cr = 0.0;
       // Use Condat to generate DAT file at first
       strcpy(long_temp_DAT,"");
       make_filename_ext(long_temp_DAT,names2[i],".DAT");
	   /**   
       if(_wherey() > 19){
           clear();
		   printw("LOCALIZE");println();
		   println();
		   if(sfile <= 3){
			   printw("(");
			   for(i = 0; i < sfile; i ++){
				   printw(names2[i]);
				   if(i < sfile - 1)
					   printw(", ");
			   }
			   printw(") = LOCALIZE (%s, (", name1);
			   for(i = 0; i < sfile; i ++){
				   printw(names1[i]);
				   if(i < sfile - 1)
					   printw(", ");
			   }
			   printw("), %s)", name2);
		   }else{
			   printw("(%s, %s, ..., %s) = LOCALIZE(%s, (%s, %s, ..., %s), %s)", names2[0], 
				   names2[1], names2[sfile - 1], name1, names1[0], names1[1], names1[sfile - 1], name2);
		   }
		   println();
		   println();
           x = _wherex();
           y = _wherey();
           move(22,0); clrtoeol();
           move(23,0); clrtoeol();
           printw("Processing:  Please wait... ");
           refresh();
         
           move(y,x);
           refresh();
       }
       printw("Generating %s.DES...", names2[i]);
	   */
        
       loc_condat1(t1,s1,s2,s3,t3,&s4,&t4,par3[i].numelts,par3[i].next,macro_c);
       filedes(names2[i],s4,-1,t4);              
       if(!exist(long_temp_DAT)){
          result = -1;
          goto LOC_LABEL;
       }
       
       // Use Supreduce to implement Localize algorithm   
       strcpy(long_temp_DES,"");
       strcpy(long_name1,"");
       strcpy(long_name2,"");
       strcpy(long_name3,"");
       make_filename_ext(long_temp_DES,names2[i],".DES");       
       make_filename_ext(long_name1,name1,".DES");
       make_filename_ext(long_name2,name2,".DES");
       make_filename_ext(long_name3,names3[i],".DES");
       
       // Weak version
       supreduce1(long_name1,long_name2,long_temp_DAT,long_temp_DES,&lb,&cr,&num_of_states1[i]);	   

       // Strong version       
       supreduce2(long_name1,long_name2,long_temp_DAT,long_name3,&lb,&cr,&num_of_states2[i]);
	   
 
    //    println();
       if(exist(long_temp_DES) && exist(long_name3)) {  
        //    printw("%s.DES generated.\n",names2[i]); 
        } else {
		//    ex_supreduce1(long_name1,long_name2,long_temp_DAT,long_temp_DES,&lb,&cr,&num_of_states1[i]);
		   supreduce1(long_name1,long_name2,long_temp_DAT,long_temp_DES,&lb,&cr,&num_of_states1[i]);

		//    ex_supreduce2(long_name1,long_name2,long_temp_DAT,long_temp_DES,&lb,&cr,&num_of_states2[i]);
		   supreduce2(long_name1,long_name2,long_temp_DAT,long_temp_DES,&lb,&cr,&num_of_states2[i]);
		   if(exist(long_temp_DES) && exist(long_name3)) {   
			//    printw("%s.DES generated.\n",names2[i]); 
		   }else{
				// printw("\n%s.DES cannot be generated: possible error in data entry.\n",names2[i]);
				result = -1;
				goto LOC_LABEL;
		   }
       }
    //    println(); 
  
       remove(long_temp_DAT);
       freedes(s4,&t4);
       s4 = 0; t4 = NULL;
    }
   
    
	if(mode == 0){
		//if(!ok){
		min = abs((INT_OS)num_of_states2[0] - (INT_OS)num_of_states1[0]);
		pos_min = 0;
		for(i = 1; i < sloc; i ++){
			if(abs((INT_OS)num_of_states2[i] - (INT_OS)num_of_states1[i]) < min){
				pos_min = i;
			}
		}
		strcpy(long_name3,"");
		make_filename_ext(long_name3,names3[pos_min],".DES"); 
		init = 0L;
		if(exist(long_name3))
			getdes(names3[pos_min],&s4,&init,&t4);

		init = 0L;
		filedes(names2[pos_min],s4,init,t4);
		freedes(s4,&t4);
		//}
		for(i = 0; i < sloc; i ++){
			strcpy(long_name3,"");
			make_filename_ext(long_name3,names3[i],".DES");
			if(exist(long_name3))
				remove(long_name3);
		}

		//////////////////////////////////////////////////////////////////////////////////
		// clean_up the localized controllers
		if(sfile == 1)
			goto LOC_LABEL;

		for(i = 0; i < sloc; i ++){
			init = 0L;
			getdes(names2[i],&s4,&init,&t4);
			free(list); free(conlist);
			slist = sconlist = 0; list = conlist = NULL;
			for(k = 0; k < t4[0].numelts; k ++){
				if(t4[0].next[k].data2 == 0 && t4[0].next[k].data1 % 2 == 1){
					addordlist(t4[0].next[k].data1,&conlist,sconlist,&ok);
					if(ok) sconlist ++;
				}
			}

			for(j = 0; j < s4; j ++){
				for(k = 0; k < t4[j].numelts; k ++){
					ee = t4[j].next[k].data1;
					es = t4[j].next[k].data2;
					if(es != j){
						addordlist(ee,&list,slist,&ok);
						if(ok) slist++;
					} else if(ee %2 == 1){
						if(inlist(ee,conlist,sconlist)){
							addordlist(ee,&ctemp,sctemp,&ok);
							if(ok) sctemp ++;
						}
					}
				}  
				free(conlist); sconlist = 0; conlist = NULL;
				loc_unionsets1(ctemp,sctemp,&conlist,&sconlist);
				free(ctemp); sctemp = 0; ctemp = NULL;         
			}
			for(j = 0; j < s4; j ++){
				for(k = 0; k < t4[j].numelts; k ++){
					ee = t4[j].next[k].data1;
					es = t4[j].next[k].data2;
					de_flag = false;
					if(es != j)
						continue;            
					if(!inlist(ee,par3[i].next,par3[i].numelts) && (!inlist(ee,list,slist))){
						de_flag = true;
					}else{
						if(ee %2 == 0 && (!inlist(ee,list,slist))){
							de_flag = true;
						}else if(ee %2 == 1 && (inlist(ee,conlist,sconlist))){
							de_flag = true;
						}
					}
					if(de_flag == true){
						addordlist(ee,&ctemp,sctemp,&ok);
						if(ok) sctemp ++;
					}

				}
				if(sctemp != 0){
					for(k = 0; k < sctemp; k ++){
						delete_ordlist1(ctemp[k],j,&t4[j].next,t4[j].numelts,&ok);
						if(ok) t4[j].numelts --;
					}
				}
				free(ctemp); sctemp = 0; ctemp = NULL;
			}
			init = 0L;
			filedes(names2[i],s4,init,t4);
			freedes(s4,&t4); s4 = 0; t4 = NULL;
		}
	} else{
		for(i = 0; i < sloc; i ++){
			strcpy(long_name3,"");
			make_filename_ext(long_name3,names3[i],".DES");
			if(exist(long_name3)){
				init = 0L;
				getdes(names3[i], &s4, &init, &t4);

				init = 0L;
				filedes(names2[i], s4, init, t4);
				freedes(s4, &t4); s4 = 0; t4 = NULL;
				remove(long_name3);
			}
		}
	}
      
LOC_LABEL:
    
    freedes(s1,&t1);      
    freedes(s2,&t2);  
    freedes(s3,&t3); 
    freedes(s4,&t4);  
    freedes(s5,&t5);
    freedes(s6,&t6);
    freedes(s7,&t7);
    
    free_t_part(s_par3,&par3);
 
    free(macro_ab);
    free(macro_c); 
    
    free(list);
    free(conlist);
    free(ctemp);
	free(num_of_states1);
	free(num_of_states2);
    return result;
}
INT_OS localize_proc(INT_S sfile, INT_S sloc, 
	char* name1, char *name2, 
	char (*names1)[MAX_FILENAME], 
	char (*names2)[MAX_FILENAME],
	INT_OS clean_mode,
	INT_OS display_mode)
{
	INT_S s1, s2, s3, s4;
	state_node *t1, *t2, *t3, *t4;
	INT_S init,i;
	INT_S *macro_ab, *macro_c;
	INT_OS result;
	INT_B ok;
	//FILE * file;  
	//    char temp[MAX_PATH];
	//char long_temp_DES[MAX_PATH];
	char long_temp_DAT[MAX_PATH];
	char long_name1[MAX_PATH];
	char long_name2[MAX_PATH];
	//    char ch;
	INT_OS x,y;
//	char names3[MAX_DESS][MAX_PATH];
	char long_name3[MAX_PATH];
	//INT_S num_of_states1[sloc], num_of_states2[sloc];  
	INT_S *num_of_states2, num_of_states;
	INT_T slist, *list, slooplist, *looplist, sctemp, *ctemp;
	INT_S lb; float cr; 
	INT_S loc_flag;
//	INT_S min, pos_min;

	INT_T ee; INT_S es; INT_S j, k;
//	INT_B de_flag;

	t_part_node *par3; 
	INT_S  s_par3;

	s1 = s2 = s3 = s4 = 0;
	t1 = t2 = t3 = t4 = NULL;    

	macro_ab = macro_c = NULL;
	s_par3 = 0;  par3 = NULL;
//	file = NULL;
	slist = slooplist = sctemp = 0; 
	list = looplist = ctemp = NULL;  
	num_of_states2 = NULL;
	//num_of_states1 = (INT_S*)CALLOC(sloc, sizeof(INT_S));

	num_of_states2 = (INT_S*)CALLOC(sloc, sizeof(INT_S));

	result = 0;

	//remove("G:\\result.txt");

	s_par3 = sfile;
	par3 = (t_part_node*)CALLOC(s_par3,sizeof(t_part_node));
	if(par3 == NULL){
		s_par3 = 0;
		mem_result = 1;
		goto LOC_LABEL;
	}

	init = 0L;
	if(getdes(name1,&s1,&init,&t1) == false)
		goto LOC_LABEL; 
	for(i = 0; i < sfile; i ++){
		init = 0L;
		if(getdes(names1[i],&s2,&init,&t2) == false)
			goto LOC_LABEL;
		loc_gentranlist(s2,t2,&par3[i].numelts,&par3[i].next);
		if(mem_result == 1)
			goto LOC_LABEL;
		freedes(s2,&t2);
		s2 = 0; t2 = NULL;
	}

	init = 0L;  
	if(getdes(name2,&s2,&init,&t2) == false)
		goto LOC_LABEL;

	meet2(s1,t1,s2,t2,&s3,&t3,&macro_ab,&macro_c);
	//printw("test");

	ok = false;
	lb = 0; cr = 0.0;
	for(i = 0; i < sloc; i ++){
		lb = 0; cr = 0.0;
		// Use Condat to generate DAT file at first
		strcpy(long_temp_DAT,"");
		make_filename_ext(long_temp_DAT,names2[i],".DAT");
		/**  
		if(display_mode == 1){
			if(_wherey() > 19){
				clear();
				printw("LOCALIZE");println();
				println();
				if(sfile <= 3){
					printw("(");
					for(i = 0; i < sfile; i ++){
						printw(names2[i]);
						if(i < sfile - 1)
							printw(", ");
					}
					printw(") = LOCALIZE (%s, (", name1);
					for(i = 0; i < sfile; i ++){
						printw(names1[i]);
						if(i < sfile - 1)
							printw(", ");
					}
					printw("), %s)", name2);
				}else{
					printw("(%s, %s, ..., %s) = LOCALIZE(%s, (%s, %s, ..., %s), %s)", names2[0], 
						names2[1], names2[sfile - 1], name1, names1[0], names1[1], names1[sfile - 1], name2);
				}
				println();
				println();
				x = _wherex();
				y = _wherey();
				move(22,0); clrtoeol();
				move(23,0); clrtoeol();
				printw("Processing:  Please wait... ");
				refresh();

				move(y,x);
				refresh();
			}
			printw("Generating %s.DES...", names2[i]);    
		}
		*/

		loc_condat1(t1,s1,s2,s3,t3,&s4,&t4,par3[i].numelts,par3[i].next,macro_c);
		filedes(names2[i],s4,-1,t4);              
		if(!exist(long_temp_DAT)){
			result = -1;
			goto LOC_LABEL;
		}
		freedes(s4,&t4);
		s4 = 0; t4 = NULL;

		// Use Supreduce to implement Localize algorithm   
		strcpy(long_name1,"");
		strcpy(long_name2,"");
		strcpy(long_name3,"");     
		make_filename_ext(long_name1,name1,".DES");
		make_filename_ext(long_name2,name2,".DES");
		make_filename_ext(long_name3,names2[i],".DES");

		// loc_flag = ex_supreduce2(long_name1,long_name2,long_temp_DAT,long_name3,&lb,&cr,&num_of_states);		
		loc_flag = supreduce2(long_name1,long_name2,long_temp_DAT,long_name3,&lb,&cr,&num_of_states);		
		

		if(mem_result == 1)
			goto LOC_LABEL;
		if(loc_flag == -1){
			filedes(names2[i], 0, 0, NULL);
			num_of_states = 0;
		}else if(loc_flag == -2){
			init = 0L;
			getdes(name2, &s4, &init, &t4);
			filedes(names2[i], s4, 0, t4);
			num_of_states = s4;
			freedes(s4, &t4); s4 = 0; t4 = NULL;
		}
		if(loc_flag > 0){
			// if(display_mode == 1){
			// 	printw("\n%s.DES cannot be generated: possible error in data entry.\n",names2[i]);
			// }
			result = -1;
			goto LOC_LABEL;
		}

		num_of_states2[i] = num_of_states;
		/**
		if(display_mode == 1)
			println();
		if(exist(long_name3)) {   
			if(display_mode == 1)
				printw("%s.DES generated.\n",names2[i]); 
		}
		else {
			if(display_mode == 1){
				printw("\n%s.DES cannot be generated: possible error in data entry.\n",names2[i]);
				result = -1;
				goto LOC_LABEL;
			}
		}
		if(display_mode == 1)
			println();		
		*/
	}

	freedes(s1,&t1);       
	freedes(s2,&t2);  
	freedes(s3,&t3); 
	s1 = s2 = s3 = 0;
	t1 = t2 = t3 = NULL;

	if(clean_mode == 0){

		//////////////////////////////////////////////////////////////////////////////////
		// clean_up the localized controllers
		if(sfile == 1)
			goto LOC_LABEL;		

		for(i = 0; i < sloc; i ++){
			// First, compute the set of controllable events D_i that will be disabled in LOC_i
			init = -1L;
			getdes(names2[i],&s2,&init,&t2);
			for(j = 0; j < s2; j ++){
				for(k = 0; k < t2[j].numelts; k ++){
					addordlist(t2[j].next[k].data1,&list,slist,&ok);
					if(ok) slist ++;
				}
			}
			// Remove each condat file
			strcpy(long_temp_DAT,"");
			make_filename_ext(long_temp_DAT,names2[i],".DAT");    
			remove(long_temp_DAT);
			freedes(s2, &t2);
			s2 = 0; t2 = NULL;

			// Compute the set of events that cause a state change
			init = 0L;
			getdes(names2[i],&s4,&init,&t4);
			for(j = 0; j < s4; j ++){
				for(k = 0; k < t4[j].numelts; k ++){
					if(t4[j].next[k].data2 != j){
						addordlist(t4[j].next[k].data1,&looplist,slooplist,&ok);
						if(ok) slooplist ++;
					}
				}
			}
			// Combine the two computed sets together, we get the set of events 
			//of which the selfloops can not be removed. 

			loc_unionsets1(looplist,slooplist,&list,&slist);

			// In other words, the selfloops of any other events can be removed.
			for(j = 0; j < s4; j ++){
				for(k = 0; k < t4[j].numelts; k ++){
					ee = t4[j].next[k].data1;
					es = t4[j].next[k].data2;
					if((es == j)&&(!inlist(ee,list,slist))){
						addordlist(ee,&ctemp,sctemp,&ok);
						if(ok) sctemp ++;
					}
				}
				if(sctemp != 0){
					for(k = 0; k < sctemp; k ++){
						delete_ordlist1(ctemp[k],j,&t4[j].next,t4[j].numelts,&ok);
						if(ok) t4[j].numelts --;
					}
				}
				free(ctemp); sctemp = 0; ctemp = NULL;
			}
			init = 0L;
			filedes(names2[i],s4,init,t4);
			freedes(s4,&t4); s4 = 0; t4 = NULL;

			free(list); free(looplist);
			slist = slooplist = 0;
			list = looplist = NULL;

		}
	}

LOC_LABEL:

	freedes(s1,&t1);      
	freedes(s2,&t2);  
	freedes(s3,&t3); 
	freedes(s4,&t4);  


	free_t_part(s_par3,&par3);

	free(macro_ab);
	free(macro_c); 
	
	free(list);
	free(looplist);
//	free(conlist);
	free(ctemp);
	//free(num_of_states1);
	free(num_of_states2);
	return result;
}

void check_control_equ(INT_OS sfile, 
                             char *name1, 
                             char *name2, 
                             char (*names1)[MAX_FILENAME], 
                             char (*names2)[MAX_FILENAME],
                             INT_B *flag)
{
    INT_S s1, s2, s3, s4, s5, s6, s7;
    state_node *t1, *t2, *t3, *t4, *t5, *t6, *t7;
    INT_S init,i;
    INT_S *macro_ab, *macro_c;
//    INT_B ok;
	
    INT_S * mapstate;
    
    s1 = s2 = s3 = s4 = s5 = s6 = s7 = 0;
    t1 = t2 = t3 = t4 = t5 = t6 = t7 = NULL;    
    
    macro_ab = macro_c = NULL;

    init = 0L;  
    if(getdes(name2,&s2,&init,&t2) == false)
       goto CHECK_LABEL;
    ///////////////////////////////////////////////////////
    // Algorithm for checking localization
    //ALLSUPER = Allevents(SUPER)
    //EAGENTi = Sync(AGENTi,ALLSUPER)  [in general may contain globally prohibited events]
    //EAGENTiLOC = Sync(AGENTiLOC,ALLSUPER) [should not contain globally prohibited events]
    //ZAGENTi = Meet(EAGENTi,EAGENTiLOC) [will eliminate globally prohibited events]
    //ZSUPER = Meet(ZAGENT1,ZAGENT2,...) [should be isomorphic to SUPER]
    
    
    //  ALLSUPER = Allevents(SUPER)
    allevent_des(&t2,&s2,&t1,&s1);
    if(mem_result == 1){
       goto CHECK_LABEL;
    }
    
    for(i = 0; i < sfile; i ++){
       init = 0L;
       getdes(names1[i],&s4,&init,&t4);
       getdes(names2[i],&s5,&init,&t5);       
       
       //EAGENTi = Sync(AGENTi,ALLSUPER)
       free(macro_ab); free(macro_c);
       macro_ab = macro_c = NULL;
       loc_copy_des(&s7,&t7,s1,t1);
       sync2(s4,t4,s7,t7,&s6,&t6,&macro_ab,&macro_c);
       freedes(s4,&t4); s4 = 0; t4 = NULL;
       freedes(s7,&t7); s7 = 0; t7 = NULL;
       free(macro_ab); free(macro_c); macro_ab = macro_c = NULL;
       
       //EAGENTiLOC = Sync(AGENTiLOC,ALLSUPER)
       loc_copy_des(&s7,&t7,s1,t1);
       sync2(s5,t5,s7,t7,&s4,&t4,&macro_ab,&macro_c);
       freedes(s5,&t5); s5 = 0; t5 = NULL;
       freedes(s7,&t7); s7 = 0; t7 = NULL;
       free(macro_ab); free(macro_c); macro_ab = macro_c = NULL;
       
       //ZAGENTi = Meet(EAGENTi,EAGENTiLOC)
       meet2(s6,t6,s4,t4,&s5,&t5,&macro_ab,&macro_c);
       if(i == 0){
          loc_copy_des(&s3,&t3,s5,t5);
       } else{
          free(macro_ab); free(macro_c);
          macro_ab = macro_c = NULL;
          freedes(s4,&t4); s4 = 0; t4 = NULL;
          //ZSUPER = Meet(ZAGENT1,ZAGENT2,...)
          meet2(s3,t3,s5,t5,&s4,&t4,&macro_ab,&macro_c);
          freedes(s3,&t3); s3 = 0; t3 = NULL;
          loc_copy_des(&s3,&t3,s4,t4);
       }
       freedes(s4,&t4);
       freedes(s5,&t5);
       freedes(s6,&t6);
       s4 = s5 = s6 = 0;
       t4 = t5 = t6 = NULL;
    }
    
    *flag = false;
    mapstate = NULL;

	if(s3 == 0 && s2 == 0){
		*flag = true;
		goto CHECK_LABEL;
	}

	if(s3 == 0 || s2 == 0){
		*flag = false;
		goto CHECK_LABEL;
	}

       
    mapstate = (INT_S*)CALLOC(s3,sizeof(INT_S));
    if((s3!= 0)  && (mapstate == NULL)){
       mem_result = 1;
       goto CHECK_LABEL;
    }
    memset(mapstate,-1,s3 * sizeof(INT_S));
    mapstate[0] = 0;
    iso1(s3,s2,t3,t2,flag,mapstate);
      
    if(!(*flag)){  
        minimize(&s3,&t3);
        minimize(&s2,&t2);
        memset(mapstate,-1,s3 * sizeof(INT_S));
        mapstate[0] = 0;
        iso1(s3,s2,t3,t2,flag,mapstate);        
    }
    free(mapstate);
CHECK_LABEL:
    freedes(s1,&t1);      
    freedes(s2,&t2);  
    freedes(s3,&t3); 
    freedes(s4,&t4);  
    freedes(s5,&t5);
    freedes(s6,&t6);
    freedes(s7,&t7);
 
    free(macro_ab);
    free(macro_c);
}
/////////////////////////////////////////////////////////////////////////////////////////////////
// Variations of localize procedure
////////////////////////////////////////////////////////////////////////////////////////////////
// A structure to hold a list of bit

typedef struct code_part_node{
	INT_S numelts;
	INT_S *next;
	INT_S numcode;
	INT_B *code;
}code_part_node;
void free_code_part(INT_S s1,
	code_part_node **pn)
{
	INT_S i;

	for (i=0; i < s1; i++) {
		if ( (*pn)[i].next != NULL )
			free((*pn)[i].next);
		if ((*pn)[i].code != NULL)
			free((*pn)[i].code);
	}
	free(*pn);
}
void zprint_code_par(INT_S s, code_part_node *par)
{
	FILE *out;
	INT_S i; INT_S j;
	char tmp_result[_MAX_PATH];

	strcpy(tmp_result, "");
	strcat(tmp_result, prefix);
	strcat(tmp_result, "Ouput.txt");
	out = fopen(tmp_result,"a");


	for(i = 0; i < s; i ++){
		for(j = 0; j < par[i].numelts; j ++){
			fprintf(out,"%d ", par[i].next[j]);
		}
		fprintf(out,"   Code:");
		for(j = 0; j < par[i].numcode; j ++){
			fprintf(out,"%d ", par[i].code[j]);
		}
		fprintf(out,"\n");
	}
	fclose(out);
}
INT_OS par_to_des(INT_S s_par, part_node *par, INT_S s1, state_node *t1, INT_S *s2, state_node **t2)
{
    INT_S i,j,k;//,l;
    INT_S ptr1;//,ptr2;
    INT_T ee; INT_S es;
    INT_S cur1;//,cur2;
    INT_B ok;
    
    *s2 = s_par;
    *t2 = newdes(*s2);
    if(mem_result == 1){
       *s2 = 0;
       return -1;
    }
    /*Marker  new DES*/
    for(i = 0; i < s_par; i ++){
       (*t2)[i].marked = false;
       for(j = 0; j < par[i].numelts; j ++){
          if(t1[par[i].next[j]].marked){
             (*t2)[i].marked = true;
             break;
          }
       }
    }
    
    /*Add transitions to new DES*/    
    for(i = 0; i < s_par; i ++){
       for(j = 0; j < par[i].numelts; j ++){
          ptr1 = par[i].next[j];
          for(k = 0; k < t1[ptr1].numelts; k ++){
             ee = t1[ptr1].next[k].data1;
             es = t1[ptr1].next[k].data2;
             cur1 = loc_get_pos(es,s_par,par);
             if(cur1 == -1)
                return -1;
             addordlist1(ee,cur1,&(*t2)[i].next,(*t2)[i].numelts,&ok);
             if(ok) (*t2)[i].numelts ++;
          }
       }
    }
    return 0;
}
/*INT_OS exlocalize_proc3(char* name1, char *name2, char *name3, INT_T s_elist, INT_T *elist, INT_S s_slist, INT_S *slist)
{
    INT_S s1, s2, s3, s4;
    state_node *t1, *t2, *t3, *t4;
    INT_S init,i;//, j;//, k;    
    INT_OS result;
    INT_B ok;
//    INT_S newstate, srcstate;//, macrostate;
    INT_T  s_elist1, *elist1, s_elist2, *elist2;
    INT_S s_new_list, s_src_list, *new_list, *src_list;
    part_node *new_par, *src_par, *par;
    INT_S s_new_par, s_src_par, s_par;
	INT_T s_tmp_event, *tmp_event;
    INT_T s_tmp_state, *tmp_state;
//    INT_S  entr, state;
//    INT_B found;
    
    s1 = s2 = s3 = s4 = 0;
    t1 = t2 = t3 = t4 =  NULL;    
    
    s_new_list = s_src_list = 0; 
    new_list = src_list = NULL;
    
    s_new_par = s_src_par = s_par = 0;
    new_par = src_par = par = NULL;
   
    
    s_elist1 = s_elist2 = 0;
    elist1 = elist2 = NULL;
    
    result = 0;

    init = 0L;  
    if(getdes(name2,&s2,&init,&t2) == false){
       result = 1;
       goto EXLOC_LABEL;
    }
    
    reverse_tran(&s1, &t1, s2, t2);
    if(mem_result == 1){
       result = 1;
       goto EXLOC_LABEL;
    }
    for(i = 0; i < s1; i ++)
       t1[i].reached = false;
    
    s_src_list = s1 - 1;
    src_list = (INT_S *)CALLOC(s_src_list, sizeof(INT_S));
    if(src_list == NULL && s_src_list != 0){
       result = 1;
       goto EXLOC_LABEL;
    }
    ok = false;
    for(i = 0; i < s1; i ++){
       if(i == slist[0]){
          ok = true;
          continue;
       }
       if(ok)
          src_list[i - 1] = i;
       else
          src_list[i] = i;       
    }
    
    
    s_new_list = s_slist;
    new_list = (INT_S *)CALLOC(s_new_list, sizeof(INT_S));
    if(new_list == NULL){
       result = 1;
       goto EXLOC_LABEL;
    }
    memcpy(new_list, slist, s_new_list * sizeof(INT_S));

    
    s_par = 2;
    par = (part_node *)CALLOC(s_par, sizeof(part_node));
    if(par == NULL){
       result = 1;
       goto EXLOC_LABEL;    
    }

    for(i = 0; i < s_src_list; i ++){
       addstatelist(src_list[i], &par[0].next, par[0].numelts, &ok);
       if(ok) par[0].numelts ++;
    }

    addstatelist(slist[0], &par[1].next, par[1].numelts, &ok);
    if(ok) par[1].numelts ++;
    
    par_to_des(s_par, par, s2, t2, &s3, &t3);
    

    
    s_tmp_event = 0; tmp_event = NULL;
    s_tmp_state = 0; tmp_state = NULL;
    
    while (!is_deterministic(t3, s3)){
       for(i = 0; i < s_new_list; i ++){
          exit = new_list[i];
          t1[exit].reached = true;
          for(j = 0; j < t1[exit].numelts; j ++){
             event = t1[exit].next[j].data1;
             entr = t1[exit].next[j].data2;
             addordlist(event, &tmp_event, s_tmp_event, &ok);
             if(ok) s_tmp_event ++;
             addstatelist(entr, &tmp_state, s_tmp_state, &ok);
             if(ok) s_tmp_state ++;
          }
          s_new_par = 1;
          new_par = (part_node *)CALLOC(s_new_par, sizeof(part_node));
          if(new_par == NULL){
             s_new_par = 0;
             goto EXLOC_LABEL;
          }
          addstatelist(tmp_state[0], &new_par[0].next, new_par[0].numelts, &ok);
          if(ok) new_par[0].numelts ++;
          t1[tmp_state[0]].reached = true;
          
          for(j = 0; j < s_tmp_state; j ++){
             state = tmp_state[j];
              
          }
       }  
    }
    
    init = 0L;
    filedes(name3, s3, init, t3);
EXLOC_LABEL:
    
    freedes(s1,&t1);      
    freedes(s2,&t2);  
    freedes(s3,&t3); 
    freedes(s4,&t4);   
    
    free_part(s_par, &par);
    free_part(s_src_par, &src_par);
    free_part(s_new_par, &new_par);  

    
    free(elist1);
    free(elist2);
    free(src_list);
    free(new_list);
    
    return result;
}*/
void merge_states(x_state_node *par, INT_S pos1, INT_S pos2)
{
    INT_S i, j, ss;
    INT_B ok;
    
    // merge states
    for(i = 0; i < par[pos2].s_numelts; i ++){
       addstatelist(par[pos2].s_next[i], &par[pos1].s_next, par[pos1].s_numelts, &ok);
       if(ok) par[pos1].s_numelts ++;
    }    

    
    //merge transitions
    for(i = 0; i < par[pos2].t_numelts; i ++){
       addordlist1(par[pos2].t_next[i].data1, par[pos2].t_next[i].data2, &par[pos1].t_next, par[pos1].t_numelts, &ok);
       if(ok) par[pos1].t_numelts ++;
    }
    
    for(i = 0; i < par[pos1].s_numelts; i ++){
       ss = par[pos1].s_next[i];
       if(ss != pos1){
          // copy states of new cell pos1 to new cell pos2
          par[ss].s_numelts = par[pos1].s_numelts;
          par[ss].s_next = (INT_S*)REALLOC(par[ss].s_next, par[pos1].s_numelts * sizeof(INT_S));
          for(j = 0; j < par[ss].s_numelts; j ++)
             par[ss].s_next[j] = par[pos1].s_next[j];
          //copy transitions of pos1 to pos2  
          par[ss].t_numelts = par[pos1].t_numelts;
          par[ss].t_next = (tran_node*)REALLOC(par[ss].t_next, par[pos1].t_numelts * sizeof(tran_node));
          for(j = 0; j < par[ss].t_numelts; j ++){
             par[ss].t_next[j].data1 = par[pos1].t_next[j].data1;
             par[ss].t_next[j].data2 = par[pos1].t_next[j].data2;
          }          
       }
    }
}
INT_S search_ordlist(INT_T e,             /* event to find */
                   tran_node *L,        /* list to search */
                   INT_T size)
{
   INT_T pos;
   INT_T lower, upper;
   INT_B found;
   INT_S es;
   
   es = -1;

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
          es = L[pos-1].data2;
       } else if (e > L[pos-1].data1) {
          lower = pos+1;
       } else {
          upper = pos-1;
       }
     }
   } else if (size == 1) {
     if (e == L[0].data1) {
       found = true;
       es = L[0].data2;
     }
   }

   return es;                   
}                                                         
void loc_genlist2(INT_T slist1, tran_node *list1, INT_T slist2, tran_node *list2,
	             INT_T event, INT_S s, state_node **t)
{
   INT_B ok;
   /* relying on ordered list and TICK = 0*/
   if (slist1 > 0 && slist2 > 0 && 
       (list1[0].data1 == 0 && list2[0].data1 != 0)){
      if(inordlist1(event, list1, slist1) && inordlist1(event, list2, slist2)){
         addordlist1(event, 0, &(*t)[s].next, (*t)[s].numelts, &ok);
         if(ok) (*t)[s].numelts ++;
      }
   }
   
}
void loc_condat2(state_node *t1, INT_S s1, INT_S s2, INT_S s3, state_node *t3,
             INT_T event, INT_S *s5, state_node **t5, INT_S *macro_c)
{
   INT_S state, state1, state2;

   *s5 = s2;
   *t5 = newdes(*s5);
   
   for (state=0; state < s3; state++) {
     state1 = macro_c[state] % s1;
     state2 = macro_c[state] / s1;
     loc_genlist2(t1[state1].numelts, t1[state1].next,
             t3[state].numelts, t3[state].next, event, state2, t5);
   }

}
INT_S loc_getindex(INT_T e, INT_T *L, INT_T size)
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
	if(found == false) k = -1;

	return k;
}
void loc_genlist3(INT_T slist1, tran_node *list1, INT_T slist2, tran_node *list2, INT_T s_list, INT_T *list, INT_B *elist)
{
	INT_T j1, j2;
	INT_S index;
//	INT_B ok;

	j1 = j2 = 0;
	while ((j1 < slist1) && (j2 < slist2)) {
		if (list1[j1].data1 == list2[j2].data1) {
			j1++; j2++;
		} else if (list1[j1].data1 > list2[j2].data1) {
			j2++;
		} else {
			index = loc_getindex(list1[j1].data1,list,s_list);
			if(index != -1){
				elist[index] = 2;
			}
			j1++;
		}
	}

	while (j1 < slist1) {
		index = loc_getindex(list1[j1].data1,list,s_list);
		if(index != -1){
			elist[index] = 2;
		}
		j1++;
	}
}
void loc_code(INT_S s1, state_node *t1, INT_S s2, state_node *t2, INT_T s_conlist, INT_T *conlist, INT_S *s_depar, bt_part_node **depar)
{
	INT_S i,j;
	INT_S s3; state_node *t3;
	INT_S *macro_ab,*macro_c;
	INT_S state,state1,state2, numelts,index;


	s3 = 0; t3 = NULL;
	macro_ab = macro_c = NULL;
	*s_depar = s2;

	//Compute E(x) and M(x)
	*depar = (bt_part_node*)CALLOC(*s_depar, sizeof(bt_part_node));
	if(*depar == NULL){
		mem_result = 1;
		return;
	} 
	numelts = s_conlist + 2; // each controllable event corresponds to one bit in depar, the extra two bits are used to store M(x) and T(x)
	for(i = 0; i < (*s_depar); i ++){
		(*depar)[i].numelts = (INT_T)numelts;  
		(*depar)[i].next = (INT_B*)CALLOC((*depar)[i].numelts, sizeof(INT_B));
		if((*depar)[i].next == NULL){
			mem_result = 1;
			return;
		}
	}

	for(i = 0; i < s2; i ++){
		for(j = 0; j < t2[i].numelts; j ++){
			index = loc_getindex(t2[i].next[j].data1,conlist,s_conlist);
			if(index != -1){
				(*depar)[i].next[index] = 1;
			}
		}
		if(t2[i].marked){
			(*depar)[i].next[numelts - 2] = 1;
		}
	}    

	//Compute D(x) and T(x)
	meet2(s1,t1,s2,t2,&s3,&t3,&macro_ab,&macro_c);

	for(state = 0; state < s3; state ++){
		state1 = macro_c[state] % s1;
		state2 = macro_c[state] / s1;

		loc_genlist3(t1[state1].numelts, t1[state1].next, t3[state].numelts, t3[state].next,s_conlist,conlist,(*depar)[state2].next);
		if(t1[state1].marked)
			(*depar)[state2].next[numelts - 1] = 1;
	}  

	freedes(s3, &t3);
	free(macro_ab);
	free(macro_c);
}
INT_B loc_cc_check(INT_S ptr1, INT_S ptr2, INT_S enumelts, bt_part_node *depar)
{
	INT_B *code1, *code2;
	INT_S i;

	code1 = depar[ptr1].next;
	code2 = depar[ptr2].next;

   /* if(loc_is_intersect(epar[ptr1].numelts, epar[ptr1].next, dpar[ptr2].numelts, dpar[ptr2].next) ||
       loc_is_intersect(epar[ptr2].numelts, epar[ptr2].next, dpar[ptr1].numelts, dpar[ptr1].next))
       return false;*/
	// T(x1) = T(x2) => M(x1) = M(x2)
    if((code1[enumelts + 1] == code2[enumelts + 1])&& ((code1[enumelts] != code2[enumelts])))
          return false;

	// E(x1)/\D(x2) = E(x2)/\D(x1) = \empty
	for(i = 0; i < enumelts; i ++){
		if(code1[i] + code2[i] == 3)
			return false;
	}		
    
    return true;
   /* if(mark){
       if(mlist[ptr1] == mlist[ptr2])
          return true;
    }else{
       if(dpar[ptr1].numelts == 0 && dpar[ptr2].numelts == 0)
          return true;
       if(loc_is_intersect(dpar[ptr1].numelts, dpar[ptr1].next, dpar[ptr2].numelts, dpar[ptr2].next))
          return true;
    }
    return false;*/
}

void loc_get_congruence(INT_S size, cc_check_table *cross_table, part_node *par)
{
	INT_S i,j, p, q, index_i, index_j;
	INT_B merge_flag;

	for(i = 0; i < size - 1; i ++){
		if(i > par[i].next[0])
			continue;
		for(j = i + 1; j < size - 1; j ++){
			if(j > par[j].next[0])
				continue;
			merge_flag = true;
			for(p = 0; p < par[i].numelts; p ++){
				index_i = par[i].next[p];
				for(q = 0; q < par[j].numelts; q ++){
					index_j = par[j].next[q] - index_i - 1;
					if(cross_table[index_i].next[index_j].flag){
						merge_flag = false;
						break;
					}
				}
				if(!merge_flag)
					break;
			}
			if(!merge_flag)
				continue;

			//Merge cell [xi] and [xj]
			loc_unionsets(par[i].next, par[i].numelts,&par[j].next, &par[j].numelts);
			par[i].numelts = par[j].numelts;
			par[i].next = (INT_S*)REALLOC(par[i].next, par[i].numelts * sizeof(INT_S));
			if(par[i].next == NULL){
				mem_result = 1;
				return;
			}
			memcpy(par[i].next,par[j].next,par[i].numelts * sizeof(INT_S));
		}
	}

}
INT_OS rebuild_dat(INT_S s_par, part_node *par, INT_S s1, state_node *t1, INT_S *s2, state_node **t2, INT_OS mode)
{
	INT_S i,j, state;
	INT_T ee;
	INT_B  ok;

	*s2 = s_par;
	*t2 = newdes(*s2);
	if(mem_result == 1){
		*s2 = 0;
		return -1;
	}    

	/*Add transitions to new DES*/    
	for(i = 0; i < s1; i ++){
		for(j = 0; j < t1[i].numelts; j ++){
			ee = t1[i].next[j].data1;
			state = loc_get_pos(i,s_par,par);
			if(state == -1){
				freedes(*s2, t2);
				return -1;
			}
			if(mode == 0){
				addordlist1(ee,0, &(*t2)[state].next, (*t2)[state].numelts, &ok);
				if(ok) (*t2)[state].numelts ++;
			}else if(mode == 1){
				addordlist1(0,0, &(*t2)[state].next, (*t2)[state].numelts, &ok);
				if(ok) (*t2)[state].numelts ++;
			}
			if(mem_result == 1)
			{
				freedes(*s2, t2);
				return -1;
			}
		}
	}   

	return 0;
}

INT_OS exlocalize_proc1(char* name1, char *name2, char *name3, INT_T s_list, INT_T *list, INT_B display_mode)
{
	INT_S s1, s2, s3, s4;
	state_node *t1, *t2, *t3, *t4;
	INT_S init;//,i;
	INT_S *macro_ab, *macro_c;
	INT_OS result;
	INT_B ok, loc_flag;
	//    FILE * file;  
	char long_temp_DAT[MAX_PATH];
	char long_name1[MAX_PATH];
	char long_name2[MAX_PATH];
	char long_name3[MAX_PATH];
	INT_S num;
	float cr;INT_S lb;  

	s1 = s2 = s3 = s4 = 0;
	t1 = t2 = t3 = t4 =  NULL;    

	macro_ab = macro_c = NULL;     
	num = 0; 

	result = 0;
	loc_flag = 0;

	init = 0L;
	if(getdes(name1,&s1,&init,&t1) == false)
		goto EXLOC_LABEL; 

	init = 0L;  
	if(getdes(name2,&s2,&init,&t2) == false)
		goto EXLOC_LABEL;

	meet2(s1,t1,s2,t2,&s3,&t3,&macro_ab,&macro_c);
	if(mem_result == 1){
		goto EXLOC_LABEL;
	}


	ok = false;
	lb = 0; cr = 0.0;

	// Use Condat to generate DAT file at first
	strcpy(long_temp_DAT,"");
	make_filename_ext(long_temp_DAT,name2,EXT_DAT);

	// if(display_mode){
	// 	println();   
	// 	printw("Generating %s.DES...", name3);   println();
	// 	println(); 
	// }

	loc_condat1(t1,s1,s2,s3,t3,&s4,&t4,s_list,list,macro_c);

	init = -1;
	filedes(name2,s4,init,t4);              
	if(!exist(long_temp_DAT)){
		result = -1;
		goto EXLOC_LABEL;
	}

	// Use Supreduce to implement Localize algorithm   
	//strcpy(long_temp_DES,"");
	strcpy(long_name1,"");
	strcpy(long_name2,"");
	strcpy(long_name3,"");
	//make_filename_ext(long_temp_DES,names2[i],".DES");       
	make_filename_ext(long_name1,name1,EXT_DES);
	make_filename_ext(long_name2,name2,EXT_DES);
	make_filename_ext(long_name3,name3,EXT_DES);

	if(s_list == 0){
		// Construct local marker only consider the marking actions
		loc_flag = supreduce4(long_name1,long_name2,long_temp_DAT,long_name3,&lb,&cr,&num); 
	}else{
		// Construct local controller only consider the marking actions
	    loc_flag = supreduce1(long_name1,long_name2,long_temp_DAT,long_name3,&lb,&cr,&num); 
	}
	if(mem_result == 1){
		result = -1;
		goto EXLOC_LABEL;
	}
	if(loc_flag == -1){
		filedes(name3, 0, 0, NULL);
	}else if(loc_flag == -2){
		filedes(name3, s2, 0, t2);
	}
	// println();
	if(exist(long_name3))   {
		// if(display_mode)
		// 	printw("%s.DES is generated.\n",name3); 
	}
	else {
		// if(display_mode)
		// 	printw("\n%s.DES cannot be generated: possible error in data entry.\n",name3);
		result = -1;
		goto EXLOC_LABEL;
	}
	// println(); 

	remove(long_temp_DAT);
	freedes(s4,&t4);
	s4 = 0; t4 = NULL;

EXLOC_LABEL:

	freedes(s1,&t1);      
	freedes(s2,&t2);  
	freedes(s3,&t3); 
	freedes(s4,&t4);     

	free(macro_ab);
	free(macro_c); 

	return result;
}
//supervisor localization under partial observation
INT_OS exlocalize_proc2(char* name1, char *name2, char *name3, INT_T s_list, INT_T *list, INT_T s_nullist, INT_T *nullist)
{
	INT_S s1, s2, s3, s4, s5;
	state_node *t1, *t2, *t3, *t4, *t5;
	INT_S init, i, j, k;
	INT_S *macro_ab, *macro_c;
	INT_OS result;
	INT_B ok, loc_flag;
	//    FILE * file;  
	char long_temp_DAT[MAX_PATH];
	char long_name1[MAX_PATH];
	char long_name2[MAX_PATH];
	char long_name3[MAX_PATH];
	char temp_name1[MAX_PATH];
	char temp_name2[MAX_PATH];
	INT_S num;
	float cr;INT_S lb;  

	state_map * macrosets; INT_S s_macrosets, ss, ee;

	s1 = s2 = s3 = s4 = s5 = 0;
	t1 = t2 = t3 = t4 = t5 = NULL;    

	macro_ab = macro_c = NULL;     
	num = 0; 
	
	macrosets = NULL; s_macrosets = 0;

	result = 0;
	loc_flag = 0;

	init = 0L;
	if(getdes(name1,&s1,&init,&t1) == false)
		goto EXLOC_LABEL; 

	init = 0L;  
	if(getdes(name2,&s2,&init,&t2) == false)
		goto EXLOC_LABEL;

	meet2(s1,t1,s2,t2,&s3,&t3,&macro_ab,&macro_c);
	if(mem_result == 1){
		result = -1;
		goto EXLOC_LABEL;
	}


	ok = false;
	lb = 0; cr = 0.0;

	// Use Condat to generate DAT file at first
	strcpy(temp_name1, "#######");
	strcpy(temp_name2, "$$$$$$$");
	strcpy(long_temp_DAT,"");
	make_filename_ext(long_temp_DAT,temp_name2,EXT_DAT);
	
/**
	println();   
	printw("Generating %s.DES...", name3);   println();
	println(); */

	loc_condat1(t1,s1,s2,s3,t3,&s4,&t4,s_list,list,macro_c);

	export_copy_des(&s5, &t5, s1, t1);
	obs_project(&s1, &t1, s_nullist, nullist, &s_macrosets, &macrosets);
	for(i = 0; i < s_macrosets; i ++){
		for(j = 0; j < macrosets[i].numelts; j ++){
			ss = macrosets[i].next[j];
			for(k = 0; k < t5[ss].numelts; k ++){
				ee = t5[ss].next[k].data1;
				if(inlist((INT_T)ee, nullist, s_nullist)){
					addordlist1((INT_T)ee,i,&t1[i].next, t1[i].numelts,&ok);
					if(ok) t1[i].numelts ++;
				}
			}
		}
	}
	freedes(s5, &t5); s5 = 0; t5 = NULL;
	free_map(s_macrosets, &macrosets);
	s_macrosets = 0; macrosets = NULL;

	export_copy_des(&s5, &t5, s2, t2);
	obs_project(&s2, &t2, s_nullist, nullist, &s_macrosets, &macrosets);
	for(i = 0; i < s_macrosets; i ++){
		for(j = 0; j < macrosets[i].numelts; j ++){
			ss = macrosets[i].next[j];
			for(k = 0; k < t5[ss].numelts; k ++){
				ee = t5[ss].next[k].data1;
				if(inlist((INT_T)ee, nullist, s_nullist)){
					addordlist1((INT_T)ee,i,&t2[i].next, t2[i].numelts,&ok);
					if(ok) t2[i].numelts ++;
				}
			}
		}
	}
	freedes(s5, &t5); s5 = 0; t5 = NULL;

	//strore the new projected t1 and t2
	init = 0L;
	filedes(temp_name1, s1, init, t1);
	init = 0L;
	filedes(temp_name2, s2, init, t2);
	//generate the new condat file
	s5 = s2;
	t5 = newdes(s5);
	for(i = 0; i < s_macrosets; i ++){
		for(j = 0; j < macrosets[i].numelts; j ++){
			ss = macrosets[i].next[j];
			for(k = 0; k < t4[ss].numelts; k ++){
				addordlist1(t4[ss].next[k].data1,i,&t5[i].next, t5[i].numelts,&ok);
				if(ok) t5[i].numelts ++;
			}
		}
	}

	init = -1;
	filedes(temp_name2,s5,init,t5);              
	if(!exist(long_temp_DAT)){
		result = -1;
		goto EXLOC_LABEL;
	}

	// Use Supreduce to implement Localize algorithm   
	//strcpy(long_temp_DES,"");
	strcpy(long_name1,"");
	strcpy(long_name2,"");
	strcpy(long_name3,"");      
	make_filename_ext(long_name1,temp_name1,EXT_DES);
	make_filename_ext(long_name2,temp_name2,EXT_DES);
	make_filename_ext(long_name3,name3,EXT_DES);

	// Weak version
	//supreduce1(long_name1,long_name2,long_temp_DAT,long_temp_DES,&lb,&cr,&num_of_states1[i]);

	// Strong version
	//if(!ok)
	loc_flag = supreduce1(long_name1,long_name2,long_temp_DAT,long_name3,&lb,&cr,&num);
	if(mem_result == 1){
		result = -1;
		goto EXLOC_LABEL;
	}
	if(loc_flag == -1){
		filedes(name3, 0, 0, NULL);
	}else if(loc_flag == -2){
		filedes(name3, s2, 0, t2);
	}
	// println();
	if(exist(long_name3))  { 
		// printw("%s.DES is generated.\n",name3); 
	} else {
		// printw("\n%s.DES cannot be generated: possible error in data entry.\n",name3);
		result = -1;
		goto EXLOC_LABEL;
	}
	// println(); 

	remove(long_temp_DAT);
	remove(long_name1);
	remove(long_name2);
	freedes(s4,&t4);
	s4 = 0; t4 = NULL;

EXLOC_LABEL:

	freedes(s1,&t1);      
	freedes(s2,&t2);  
	freedes(s3,&t3); 
	freedes(s4,&t4);  
	freedes(s5,&t5);  

	free(macro_ab);
	free(macro_c); 

	free_map(s_macrosets, &macrosets);

	return result;
}

// Algorithm with quadratic complexity
INT_OS exlocalize_proc_new(char* name1, char *name2, char *name3, INT_T s_list, INT_T *list, INT_B display_mode)
{
	INT_S s1, s2, s3, s4;
	state_node *t1, *t2, *t3, *t4;
	INT_S init;//,i;
	INT_S *macro_ab, *macro_c;
	INT_OS result;
	INT_B ok;
	//    FILE * file;  
	char long_temp_DAT[MAX_PATH];
	char long_name1[MAX_PATH];
	char long_name2[MAX_PATH];
	char long_name3[MAX_PATH];
	INT_S num;
	float cr;INT_S lb;  
	INT_S loc_flag;

	s1 = s2 = s3 = s4 = 0;
	t1 = t2 = t3 = t4 =  NULL;    

	macro_ab = macro_c = NULL;     
	num = 0; 

	result = 0;
	loc_flag = 0;

	init = 0L;
	if(getdes(name1,&s1,&init,&t1) == false)
		goto EXLOC_LABEL; 

	init = 0L;  
	if(getdes(name2,&s2,&init,&t2) == false)
		goto EXLOC_LABEL;

	meet2(s1,t1,s2,t2,&s3,&t3,&macro_ab,&macro_c);


	ok = false;
	lb = 0; cr = 0.0;

	// Use Condat to generate DAT file at first
	strcpy(long_temp_DAT,"");
	make_filename_ext(long_temp_DAT,name2,EXT_DAT);

	// if(display_mode){
	// 	println();   
	// 	printw("Generating %s.DES...", name3);   println();
	// 	println(); 
	// }


	loc_condat1(t1,s1,s2,s3,t3,&s4,&t4,s_list,list,macro_c);

	init = -1;
	filedes(name2,s4,init,t4);              
	if(!exist(long_temp_DAT)){
		result = -1;
		goto EXLOC_LABEL;
	}

	// Use Supreduce to implement Localize algorithm   
	//strcpy(long_temp_DES,"");
	strcpy(long_name1,"");
	strcpy(long_name2,"");
	strcpy(long_name3,"");
	//make_filename_ext(long_temp_DES,names2[i],".DES");       
	make_filename_ext(long_name1,name1,EXT_DES);
	make_filename_ext(long_name2,name2,EXT_DES);
	make_filename_ext(long_name3,name3,EXT_DES);

	loc_flag = supreduce5(long_name1,long_name2,long_temp_DAT,long_name3,&lb,&cr);    
	if(mem_result == 1){
		result = -1;
		goto EXLOC_LABEL;
	}
	if(loc_flag == -1){
		filedes(name3, 0, 0, NULL);
	}else if(loc_flag == -2){
		filedes(name3, s2, 0, t2);
	}

	if(mem_result == 1){
		result = -1;
		goto EXLOC_LABEL;
	}

	// println();
	// if(exist(long_name3)) {
	// 	if(display_mode){
	// 		printw("%s.DES is generated.\n",name3); 
	// 	}
	// }
		
	else {
		// if(display_mode){
		// 	printw("\n%s.DES cannot be generated: possible error in data entry.\n",name3);
		// }
		
		result = -1;
		goto EXLOC_LABEL;
	}
	// println(); 

	remove(long_temp_DAT);
	freedes(s4,&t4);
	s4 = 0; t4 = NULL;

EXLOC_LABEL:

	freedes(s1,&t1);      
	freedes(s2,&t2);  
	freedes(s3,&t3); 
	freedes(s4,&t4);     

	free(macro_ab);
	free(macro_c); 

	return result;
}
//partition refinement algorithm with O(nlogn) time complexity
INT_OS exlocalize_proc_nlogn(char* name1, char *name2, char *name3, INT_T s_list, INT_T *list)
{
	INT_S s1, s2, s3, s4;
	state_node *t1, *t2, *t3, *t4;
	INT_S init;//,i;
	INT_S *macro_ab, *macro_c;
	INT_OS result;
	INT_B ok;
	//    FILE * file;  
	char long_temp_DAT[MAX_PATH];
//	char long_name1[MAX_PATH];
//	char long_name2[MAX_PATH];
	char long_name3[MAX_PATH];
	INT_S num;
	float cr;INT_S lb;  

	s1 = s2 = s3 = s4 = 0;
	t1 = t2 = t3 = t4 =  NULL;    

	macro_ab = macro_c = NULL;     
	num = 0; 

	result = 0;

	init = 0L;
	if(getdes(name1,&s1,&init,&t1) == false)
		goto EXLOC_LABEL; 

	init = 0L;  
	if(getdes(name2,&s2,&init,&t2) == false)
		goto EXLOC_LABEL;

	meet2(s1,t1,s2,t2,&s3,&t3,&macro_ab,&macro_c);


	ok = false;
	lb = 0; cr = 0.0;

	// Use Condat to generate DAT file at first
	strcpy(long_temp_DAT,"");
	make_filename_ext(long_temp_DAT,name2,EXT_DAT);

	// println();   
	// printw("Generating %s.DES...", name3);   println();
	// println(); 

	loc_condat1(t1,s1,s2,s3,t3,&s4,&t4,s_list,list,macro_c);

	/*init = -1;
	filedes(name2,s4,init,t4);              
	if(!exist(long_temp_DAT)){
		result = -1;
		goto EXLOC_LABEL;
	}

	// Use Supreduce to implement Localize algorithm   
	//strcpy(long_temp_DES,"");
	strcpy(long_name1,"");
	strcpy(long_name2,"");
	strcpy(long_name3,"");
	//make_filename_ext(long_temp_DES,names2[i],".DES");       
	make_filename_ext(long_name1,name1,EXT_DES);
	make_filename_ext(long_name2,name2,EXT_DES);
	make_filename_ext(long_name3,name3,EXT_DES);

	// Weak version
	//supreduce1(long_name1,long_name2,long_temp_DAT,long_temp_DES,&lb,&cr,&num_of_states1[i]);

	// Strong version
	//if(!ok)
	supreduce5(long_name1,long_name2,long_temp_DAT,long_name3,&lb,&cr);   */

	loc_refinement_disable(&s2,&t2,s4,t4);

	//loc_refinement_enable(&s2,&t2,s4,t4);

	if(mem_result == 1){
		result = -1;
		goto EXLOC_LABEL;
	}

	init = 0L;
	filedes(name3, s2, init, t2);

	// println();
	strcpy(long_name3,"");
	make_filename_ext(long_name3,name3,EXT_DES);
	if(exist(long_name3)) {
		// printw("%s.DES is generated.\n",name3); 
	} else {
		// printw("\n%s.DES cannot be generated: possible error in data entry.\n",name3);
		result = -1;
		goto EXLOC_LABEL;
	}
	// println(); 

	remove(long_temp_DAT);
	freedes(s4,&t4);
	s4 = 0; t4 = NULL;

EXLOC_LABEL:

	freedes(s1,&t1);      
	freedes(s2,&t2);  
	freedes(s3,&t3); 
	freedes(s4,&t4);     

	free(macro_ab);
	free(macro_c); 

	return result;
}
