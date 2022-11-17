#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "des_proc.h"
#include "cnorm.h"
#include "mymalloc.h"
#include "setup.h"

#ifdef __cplusplus
extern "C" {
#endif

/* This procedures is the same as "gentranlist" in DES_PROC.C */
void get_event_list(INT_S s1, state_node *t1, INT_T **list1, INT_T *s_t1)
{
   /* Generate a list of all transition labels used in DES */
   INT_S i;
   INT_T j;
   INT_B  ok;

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

/* Is this the same as "gendifflist" in DES_PROC.C */
void get_event_diff(INT_T **aux, INT_T *s_aux,
                    INT_T *list2, INT_T s_list2,
                    INT_T *list0, INT_T s_list0)
{
    INT_T ii;
    INT_B  ok;

    for (ii=0; ii < s_list2; ii++) {
       if (!inlist(list2[ii], list0, s_list0)) {
          addordlist(list2[ii], aux, *s_aux, &ok);
          if (ok) (*s_aux)++;
       }
    }
}

void copy_des(state_node **t_dest, INT_S *s_dest,
              state_node *t_src,   INT_S s_src)
{
    INT_S i, jj;
    INT_T j, ee;
    INT_B  ok;

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

void suprema_normal(state_node*  t1, INT_S s1,
                    state_node*  t2, INT_S s2,
                    state_node** t3, INT_S *s3,
                    INT_T *nullist, INT_T s_nullist)
{
   state_node *_t0, *_t1, *_t2, *_t3, *_t4, *_t5, *_t6, *_t7, *_t8, *_t9;
   INT_S _s0, _s1, _s2, _s3, _s4, _s5, _s6, _s7, _s8, _s9;

   INT_T *list0, *list1, *aux1, *aux2;
   INT_T s_list0, s_list1, s_aux1, s_aux2;
   INT_S i;

   INT_S *macro_ab; INT_S *macro_c;

#if defined(_x64_)
   unsigned long long *macro64_c;
   macro64_c = NULL;
#endif

   /* 0.  Initialize tempory data structures */
   _t0 = NULL;   _t1 = NULL;   _t2 = NULL;   _t3 = NULL;  _t4 = NULL;   
   _t5 = NULL;   _t6 = NULL;   _t7 = NULL;   _t8 = NULL;  _t9 = NULL;

   _s0 = 0; _s1 = 0; _s2 = 0; _s3 = 0; _s4 = 0;
   _s5 = 0; _s6 = 0; _s7 = 0; _s8 = 0; _s9 = 0;

   list0 = NULL; list1 = NULL; aux1 = NULL; aux2 = NULL;
   s_list0 = 0;  s_list1 = 0;  s_aux1 = 0;  s_aux2 = 0;

   macro_ab = NULL; macro_c = NULL;

   /* 1. _t0 = meet(t1, t2) */
#if defined(_x64_)
   meet_x64(s1, t1, s2, t2, &_s0, &_t0, &macro64_c); 
   free(macro64_c); macro64_c = NULL;
#else
   meet2(s1, t1, s2, t2, &_s0, &_t0, &macro_ab,&macro_c); 
   free(macro_ab); free(macro_c);
   macro_ab = NULL; macro_c = NULL;
#endif
   //meet2(s1, t1, s2, t2, &_s0, &_t0, &macro_ab, &macro_c);

   /* 2. _t1 = ALLEVENT(_t0) */
   allevent_des(&_t0, &_s0, &_t1, &_s1);
   
   /* 3. _t2 = ALLEVENT(t2) */
   allevent_des(&t2, &s2, &_t2, &_s2);
   
   /* 4a. LIST0 = [all event labels that appear in _t2] */
   get_event_list(_s1, _t1, &list0, &s_list0);
   
   /* 4b. LIST1 = [all event labels that appear in t2] */
   get_event_list(_s2, _t2, &list1, &s_list1);
   freedes(_s1, &_t1); _t1 = NULL; _s1 = 0;

   /* 4c. aux1 = [all event labels which appear in LIST1, but which
                 do not appear in LIST0] */
   get_event_diff(&aux1, &s_aux1, list1, s_list1, list0, s_list0);
   free(list1); list1 = NULL; s_list1 = 0;

   /* 5. _t3 = COMPLEMENT(_t0, aux1) */
   /* Copy _t3 <- _t0 */
   copy_des(&_t3,&_s3,_t0,_s0);
   complement1(&_s3, &_t3, s_aux1, aux1);
   reach(&_s3, &_t3);
   free(aux1); aux1 = NULL; s_aux1 = 0;

   /* 6. _t4 = meet(t2, _t3) */
#if defined(_x64_)
   meet_x64(s2, t2, _s3, _t3, &_s4, &_t4, &macro64_c); 
   free(macro64_c); macro64_c = NULL;
#else
   meet2(s2, t2, _s3, _t3, &_s4, &_t4, &macro_ab,&macro_c); 
   free(macro_ab); free(macro_c);
   macro_ab = NULL; macro_c = NULL;
#endif
   //meet2(s2, t2, _s3, _t3, &_s4, &_t4, &macro_ab, &macro_c);
   //free(macro_ab); free(macro_c);
   //macro_ab = NULL; macro_c = NULL;
   freedes(_s3, &_t3);

   /* 7. _t5 = project(_t4, null_event_list) */
   _t5 = _t4;
   _s5 = _s4;
   _t4 = NULL;
   _s4 = 0;
   project0(&_s5, &_t5, s_nullist, nullist);
 
   /* 8. _t6 = sync(_t5, _t2) */
   sync2(_s5, _t5, _s2, _t2, &_s6, &_t6, &macro_ab, &macro_c);
   free(macro_ab); free(macro_c);
   macro_ab = NULL; macro_c = NULL;   
   freedes(_s2, &_t2); _t2 = NULL; _s2 = 0;
   freedes(_s5, &_t5); _t5 = NULL; _s5 = 0;

   /* 9. _t7 = allevent(_t6) */
   allevent_des(&_t6, &_s6, &_t7, &_s7);
   
   /* 10a. LIST0 = [all event labels that appear in _t2] */
   /* This operation previously done already.
      get_event_list(_s2, _t2, &list0, &s_list0); */
   
   /* 10b. LIST1 = [all event labels that appear in _t7] */
   get_event_list(_s7, _t7, &list1, &s_list1);
   freedes(_s7, &_t7);

   /* 10c. aux2 = [all event labels which appear in LIST1, but which
                 do not appear in LIST0] */
   get_event_diff(&aux2, &s_aux2, list0, s_list0, list1, s_list1);
   free(list0); list0 = NULL; s_list0 = 0;
   free(list1); list1 = NULL; s_list1 = 0;

   /* 11. _t8 = COMPLEMENT(_t6, aux2) */
   _t8 = _t6;
   _s8 = _s6;
   _t6 = NULL;
   _s6 = 0;
   complement1(&_s8, &_t8, s_aux2, aux2);
   reach(&_s8, &_t8);
   free(aux2); aux2 = NULL; s_aux2 = 0;
 
   /* 12. _t9 = meet(_t0, _t8) */

#if defined(_x64_)
   meet_x64(_s0, _t0, _s8, _t8, &_s9, &_t9, &macro64_c); 
   free(macro64_c); macro64_c = NULL;
#else
   meet2(_s0, _t0, _s8, _t8, &_s9, &_t9, &macro_ab,&macro_c); 
   free(macro_ab); free(macro_c);
   macro_ab = NULL; macro_c = NULL;
#endif
   //meet2(_s0, _t0, _s8, _t8, &_s9, &_t9, &macro_ab, &macro_c);
  // free(macro_ab); free(macro_c);
  // macro_ab = NULL; macro_c = NULL;

   /* 13. t3 = trim(_t9) */
   *t3 = _t9;
   *s3 = _s9;
   _t9 = NULL;
   _s9 = 0;

   /* Clear all reach */
   for (i=0; i < *s3; i++)
      (*t3)[i].reached = false;
   trim1(s3, t3);

   for (i=1; i < *s3; i++)
     (*t3)[i].reached = false;
   if (*s3 > 0)
     (*t3)[0].reached = true;
   reach(s3, t3);
   minimize(s3, t3);  
   
   /* Cleanup the procedure of the intermediate data structures */
   freedes(_s0, &_t0);  
   freedes(_s1, &_t1);
   freedes(_s2, &_t2);
   freedes(_s3, &_t3);
   freedes(_s4, &_t4);
   freedes(_s5, &_t5); 
   freedes(_s6, &_t6);
   freedes(_s7, &_t7);
   freedes(_s8, &_t8);
   freedes(_s9, &_t9);
   free(list0); free(list1);  
   free(aux1);  free(aux2);  
#if defined(_x64_)
   free(macro64_c);	
#endif
}

// supremal normal without minimizing any intermediate DES
void suprema_normal1(state_node*  t1, INT_S s1,
                    state_node*  t2, INT_S s2,
                    state_node** t3, INT_S *s3,
                    INT_T *nullist, INT_T s_nullist)
{
   state_node *_t0, *_t1, *_t2, *_t3, *_t4, *_t5, *_t6, *_t7, *_t8, *_t9;
   INT_S _s0, _s1, _s2, _s3, _s4, _s5, _s6, _s7, _s8, _s9;

   INT_T *list0, *list1, *aux1, *aux2;
   INT_T s_list0, s_list1, s_aux1, s_aux2;
   INT_S i;

   INT_S *macro_ab; INT_S *macro_c;

   /* 0.  Initialize tempory data structures */
   _t0 = NULL;   _t1 = NULL;   _t2 = NULL;   _t3 = NULL;  _t4 = NULL;   
   _t5 = NULL;   _t6 = NULL;   _t7 = NULL;   _t8 = NULL;  _t9 = NULL;

   _s0 = 0; _s1 = 0; _s2 = 0; _s3 = 0; _s4 = 0;
   _s5 = 0; _s6 = 0; _s7 = 0; _s8 = 0; _s9 = 0;

   list0 = NULL; list1 = NULL; aux1 = NULL; aux2 = NULL;
   s_list0 = 0;  s_list1 = 0;  s_aux1 = 0;  s_aux2 = 0;

   macro_ab = NULL; macro_c = NULL;

   /* 1. _t0 = meet(t1, t2) */
   meet2(s1, t1, s2, t2, &_s0, &_t0, &macro_ab, &macro_c);
   free(macro_ab); free(macro_c);
   macro_ab = NULL; macro_c = NULL;

   /* 2. _t1 = ALLEVENT(_t0) */
   allevent_des(&_t0, &_s0, &_t1, &_s1);
   
   /* 3. _t2 = ALLEVENT(t2) */
   allevent_des(&t2, &s2, &_t2, &_s2);
   
   /* 4a. LIST0 = [all event labels that appear in _t2] */
   get_event_list(_s1, _t1, &list0, &s_list0);
   
   /* 4b. LIST1 = [all event labels that appear in t2] */
   get_event_list(_s2, _t2, &list1, &s_list1);
   freedes(_s1, &_t1); _t1 = NULL; _s1 = 0;

   /* 4c. aux1 = [all event labels which appear in LIST1, but which
                 do not appear in LIST0] */
   get_event_diff(&aux1, &s_aux1, list1, s_list1, list0, s_list0);
   free(list1); list1 = NULL; s_list1 = 0;

   /* 5. _t3 = COMPLEMENT(_t0, aux1) */
   /* Copy _t3 <- _t0 */
   copy_des(&_t3,&_s3,_t0,_s0);
   complement1(&_s3, &_t3, s_aux1, aux1);
   reach(&_s3, &_t3);
   free(aux1); aux1 = NULL; s_aux1 = 0;

   /* 6. _t4 = meet(t2, _t3) */
   meet2(s2, t2, _s3, _t3, &_s4, &_t4, &macro_ab, &macro_c);
   free(macro_ab); free(macro_c);
   macro_ab = NULL; macro_c = NULL;
   freedes(_s3, &_t3);

   /* 7. _t5 = project(_t4, null_event_list) */
   _t5 = _t4;
   _s5 = _s4;
   _t4 = NULL;
   _s4 = 0;
   plain_project_proc(&_s5, &_t5, s_nullist, nullist);
 
   /* 8. _t6 = sync(_t5, _t2) */
   sync2(_s5, _t5, _s2, _t2, &_s6, &_t6, &macro_ab, &macro_c);
   free(macro_ab); free(macro_c);
   macro_ab = NULL; macro_c = NULL;   
   freedes(_s2, &_t2); _t2 = NULL; _s2 = 0;
   freedes(_s5, &_t5); _t5 = NULL; _s5 = 0;

   /* 9. _t7 = allevent(_t6) */
   allevent_des(&_t6, &_s6, &_t7, &_s7);
   
   /* 10a. LIST0 = [all event labels that appear in _t2] */
   /* This operation previously done already.
      get_event_list(_s2, _t2, &list0, &s_list0); */
   
   /* 10b. LIST1 = [all event labels that appear in _t7] */
   get_event_list(_s7, _t7, &list1, &s_list1);
   freedes(_s7, &_t7);

   /* 10c. aux2 = [all event labels which appear in LIST1, but which
                 do not appear in LIST0] */
   get_event_diff(&aux2, &s_aux2, list0, s_list0, list1, s_list1);
   free(list0); list0 = NULL; s_list0 = 0;
   free(list1); list1 = NULL; s_list1 = 0;

   /* 11. _t8 = COMPLEMENT(_t6, aux2) */
   _t8 = _t6;
   _s8 = _s6;
   _t6 = NULL;
   _s6 = 0;
   complement1(&_s8, &_t8, s_aux2, aux2);
   reach(&_s8, &_t8);
   free(aux2); aux2 = NULL; s_aux2 = 0;
 
   /* 12. _t9 = meet(_t0, _t8) */
   meet2(_s0, _t0, _s8, _t8, &_s9, &_t9, &macro_ab, &macro_c);
   free(macro_ab); free(macro_c);
   macro_ab = NULL; macro_c = NULL;

   /* 13. t3 = trim(_t9) */
   *t3 = _t9;
   *s3 = _s9;
   _t9 = NULL;
   _s9 = 0;

   /* Clear all reach */
   for (i=0; i < *s3; i++)
      (*t3)[i].reached = false;
   trim1(s3, t3);

   for (i=1; i < *s3; i++)
     (*t3)[i].reached = false;
   if (*s3 > 0)
     (*t3)[0].reached = true;
   reach(s3, t3);
  // minimize(s3, t3);  
   
   /* Cleanup the procedure of the intermediate data structures */
   freedes(_s0, &_t0);  
   freedes(_s1, &_t1);
   freedes(_s2, &_t2);
   freedes(_s3, &_t3);
   freedes(_s4, &_t4);
   freedes(_s5, &_t5); 
   freedes(_s6, &_t6);
   freedes(_s7, &_t7);
   freedes(_s8, &_t8);
   freedes(_s9, &_t9);
   free(list0); free(list1);  
   free(aux1);  free(aux2);  
}
void suprema_normal_clo(state_node*  t1, INT_S s1,
                        state_node*  t2, INT_S s2,
                        state_node** t3, INT_S *s3,
                        INT_T *nullist, INT_T s_nullist)
{
   state_node *t_gg, *t_j, *t_k, *t_k1, *t_k2, *t_k3, *t_k4, *t_k5;
   INT_S       s_gg,  s_j,  s_k,  s_k1,  s_k2,  s_k3,  s_k4,  s_k5;
   INT_S              c_j,  c_k,  c_k1,  c_k2,  c_k3,  c_k4,  c_k5;
   INT_S i;
   INT_B  iso;
   INT_S  *macro_ab, *macro_c;
   INT_S    *mapState;

   macro_ab = NULL;  macro_c  = NULL;
   mapState = NULL;
   
   t_gg = t_j = t_k = t_k1 = t_k2 = t_k3 = t_k4 = t_k5 = NULL;
   s_gg = s_j = s_k = s_k1 = s_k2 = s_k3 = s_k4 = s_k5 = 0;
   
   /* GG := Edit(DES2, Markall) (Mark all states of DES2); */
   /* Copy t_gg <- t2 */
   copy_des(&t_gg,&s_gg,t2,s2);
   for (i = 0; i < s_gg; i++) 
      t_gg[i].marked = true;  
   
   /* J := DES1 */
   /* Copy t_j <- t1 */
   copy_des(&t_j,&s_j,t1,s1);   
   
   do {
      /* K := J */
      copy_des(&t_k, &s_k, t_j, s_j);
      c_k = count_tran(t_k, s_k);
      freedes(s_j, &t_j);  s_j = 0; t_j = NULL;
      
      /* K1 := Edit(K, Markall) */
      copy_des(&t_k1, &s_k1, t_k, s_k);
      c_k1 = count_tran(t_k1, s_k1);
      for (i=0; i < s_k1; i++)
         t_k1[i].marked = true;
      
      /* K2 := Supnorm(K1, GG, NULL); (wipe K1) */
      suprema_normal(t_k1, s_k1, t_gg, s_gg, &t_k2, &s_k2, nullist, s_nullist);
      c_k2 = count_tran(t_k2, s_k2);
      freedes(s_k1, &t_k1);  s_k1 = 0; t_k1 = NULL;
      
      /* K3 := Supclo(K2); (wipe K2) */
      supclo_des(&t_k2, &s_k2, &t_k3, &s_k3);
      c_k3 = count_tran(t_k3, s_k3);
      freedes(s_k2, &t_k2);  s_k2 = 0; t_k2 = NULL;
      
      /* K4 := Meet(K,K3); (wipe K3) */
      meet2(s_k,t_k,s_k3,t_k3,&s_k4,&t_k4,&macro_ab,&macro_c); 
      c_k4 = count_tran(t_k4, s_k4);
      freedes(s_k3, &t_k3);  s_k3 = 0; t_k3 = NULL;
      free(macro_ab); free(macro_c);
      macro_ab = NULL; macro_c = NULL;
      
      /* K5 = Trim(K4); (wipe K4) */
      t_k5 = t_k4;
      s_k5 = s_k4;
      s_k4 = 0;
      t_k4 = NULL;
      trim1(&s_k5, &t_k5);
      c_k5 = count_tran(t_k5, s_k5);
      
      /* J := Minstate(K5); (wipe K5) */
      t_j = t_k5;
      s_j = s_k5;
      t_k5 = NULL;
      s_k5 = 0;
      reach(&s_j, &t_j);
      minimize(&s_j, &t_j);
      
      c_j = count_tran(t_j, s_j);
   
      /* Isomorph(J,K) */
      iso   = false;
      mapState = NULL; 

      /* Zero out the reached variable -- should be done in iso1 */
      for (i=0; i < s_j; i++)
         t_j[i].reached = false;
      for (i=0; i < s_k; i++)
         t_k[i].reached = false;

      if ( (s_j == 0) && (s_k == 0) ) {
        iso = true;
      } else {
        /* Need some memory here - Allocate map state */
        mapState = (INT_S*) CALLOC(s_j, sizeof(INT_S));

        memset(mapState, -1, sizeof(INT_S)*(s_j));

        iso = true;
        iso1(s_j, s_k, t_j, t_k, &iso, mapState);
      }     
      free(mapState); mapState = NULL;
   } while (iso == false);
   
   *t3 = t_j;
   *s3 = s_j;
   
   freedes(s_k, &t_k);
}


void suprema_normal_scop(state_node*  t1, INT_S s1,
                         state_node*  t2, INT_S s2,
                         state_node** t3, INT_S *s3,
                         INT_T *nullist, INT_T s_nullist)
{
   state_node *t_g, *t_j, *t_k, *t_k1, *t_k2, *t_k3;
   INT_S       s_g,  s_j,  s_k,  s_k1,  s_k2,  s_k3;
   INT_S             c_j,  c_k,  c_k1,  c_k2,  c_k3;
   INT_S i;
   INT_B  iso;
   INT_S  *macro_ab, *macro_c;
   INT_S  *mapState;

   macro_ab = NULL;  macro_c  = NULL;
   mapState = NULL;
   
   t_g = t_j = t_k = t_k1 = t_k2 = t_k3 = NULL;
   s_g = s_j = s_k = s_k1 = s_k2 = s_k3 = 0;
   
   /* J = DES1 */
   /* Copy t_j <- t1 */
   copy_des(&t_j,&s_j,t1,s1);   
   
   /* G = DES2 */
   /* Copy t_g <- t2 */
   copy_des(&t_g,&s_g,t2,s2);
   
   do {
      /* K := J */
      copy_des(&t_k, &s_k, t_j, s_j);
      c_k = count_tran(t_k, s_k);
      freedes(s_j, &t_j);  s_j = 0; t_j = NULL;
      
      /* K1 := Supnormclo(K, G, NULL event list); */
      suprema_normal_clo(t_k, s_k, t_g, s_g, &t_k1, &s_k1, nullist, s_nullist);
      c_k1 = count_tran(t_k1, s_k1);
      
      /* K2 := Supnorm(K1,G, Null event list) */
      suprema_normal(t_k1, s_k1, t_g, s_g, &t_k2, &s_k2, nullist, s_nullist);
      c_k2 = count_tran(t_k2, s_k2);
      freedes(s_k1, &t_k1); s_k1 = 0; t_k1 = NULL;
      
      /* K3 := Supcon(G,K2); (wipe K2) */
      meet2(s_g,t_g,s_k2,t_k2,&s_k3,&t_k3,&macro_ab,&macro_c); 
      freedes(s_k2,&t_k2); s_k2 = 0; t_k2 = NULL;
      trim2(&s_k3,&t_k3,macro_c);
      shave1(s_g,t_g,&s_k3,&t_k3,macro_c);
      free(macro_ab); free(macro_c);
      macro_ab = NULL;  macro_c = NULL;
      c_k3 = count_tran(t_k3, s_k3);
      
      /* J := Minstate(K3); (wipe) K3 */
      t_j = t_k3;
      s_j = s_k3;
      t_k3 = NULL;
      s_k3 = 0;
      reach(&s_j, &t_j);
      minimize(&s_j, &t_j);
      c_j = count_tran(t_j, s_j);      
         
      /* Isomorph(J,K) */
      iso   = false;
      mapState = NULL; 

      /* Zero out the reached variable -- should be done in iso1 */
      for (i=0; i < s_j; i++)
         t_j[i].reached = false;
      for (i=0; i < s_k; i++)
         t_k[i].reached = false;

      if ( (s_j == 0) && (s_k == 0) ) {
        iso = true;
      } else {
        /* Need some memory here - Allocate map state */
        mapState = (INT_S*) CALLOC(s_j, sizeof(INT_S));

        memset(mapState, -1, sizeof(INT_S)*(s_j));

        iso = true;
        iso1(s_j, s_k, t_j, t_k, &iso, mapState);
      }     
      free(mapState); mapState = NULL;
   } while (iso == false);
   
   *t3 = t_j;
   *s3 = s_j;
   
   freedes(s_k, &t_k);
}

/*
 * iso_sizecount 
 */
INT_B  iso_sizecount(INT_S s1, state_node *t1, 
                      INT_S s2, state_node *t2)
{
   INT_S m_s1, m_s2, c_t1, c_t2;
   INT_S i;
   
   m_s1 = m_s2 = 0;
   
   if (s1 != s2) return false;
   
   for (i=0; i < s1; i++)
   {
      if (t1[i].marked == true) m_s1++;
      if (t2[i].marked == true) m_s2++;      
   }              
   if (m_s1 != m_s2) return false;
      
   c_t1 = count_tran(t1, s1);
   c_t2 = count_tran(t2, s2);
   
   if (c_t1 != c_t2) return false;
   
   return true;
}                  

/***/

void suprema_normal_clo1(state_node*  t1, INT_S s1,
                         state_node*  t2, INT_S s2,
                         state_node** t3, INT_S *s3,
                         INT_T *nullist, INT_T s_nullist)
{
   state_node *t_gg, *t_j, *t_k, *t_k1, *t_k2, *t_k3, *t_k4, *t_k5;
   INT_S       s_gg,  s_j,  s_k,  s_k1,  s_k2,  s_k3,  s_k4,  s_k5;
   INT_S              c_j,  c_k,  c_k1,  c_k2,  c_k3,  c_k4,  c_k5;
   INT_S i;
   INT_B  iso;
   INT_S  *macro_ab, *macro_c;
   INT_S  *mapState;

   macro_ab = NULL;  macro_c  = NULL;
   mapState = NULL;
   
   t_gg = t_j = t_k = t_k1 = t_k2 = t_k3 = t_k4 = t_k5 = NULL;
   s_gg = s_j = s_k = s_k1 = s_k2 = s_k3 = s_k4 = s_k5 = 0;
   
   /* GG := Edit(DES2, Markall) (Mark all states of DES2); */
   /* Copy t_gg <- t2 */
   copy_des(&t_gg,&s_gg,t2,s2);
   for (i = 0; i < s_gg; i++) 
      t_gg[i].marked = true;  
   
   /* J := DES1 */
   /* Copy t_j <- t1 */
   copy_des(&t_j,&s_j,t1,s1);   
   
   do {
      /* K := J */
      copy_des(&t_k, &s_k, t_j, s_j);
      c_k = count_tran(t_k, s_k);
      freedes(s_j, &t_j);  s_j = 0; t_j = NULL;
      
      /* K1 := Edit(K, Markall) */
      copy_des(&t_k1, &s_k1, t_k, s_k);
      c_k1 = count_tran(t_k1, s_k1);
      for (i=0; i < s_k1; i++)
         t_k1[i].marked = true;
      
      /* K2 := Supnorm(K1, GG, NULL); (wipe K1) */
      suprema_normal(t_k1, s_k1, t_gg, s_gg, &t_k2, &s_k2, nullist, s_nullist);
      c_k2 = count_tran(t_k2, s_k2);
      freedes(s_k1, &t_k1);  s_k1 = 0; t_k1 = NULL;
      
      /* K3 := Supclo(K2); (wipe K2) */
      supclo_des(&t_k2, &s_k2, &t_k3, &s_k3);
      c_k3 = count_tran(t_k3, s_k3);
      freedes(s_k2, &t_k2);  s_k2 = 0; t_k2 = NULL;
      
      /* K4 := Meet(K,K3); (wipe K3) */
      meet2(s_k,t_k,s_k3,t_k3,&s_k4,&t_k4,&macro_ab,&macro_c); 
      c_k4 = count_tran(t_k4, s_k4);
      freedes(s_k3, &t_k3);  s_k3 = 0; t_k3 = NULL;
      free(macro_ab); free(macro_c);
      macro_ab = NULL; macro_c = NULL;
      
      /* K5 = Trim(K4); (wipe K4) */
      t_k5 = t_k4;
      s_k5 = s_k4;
      s_k4 = 0;
      t_k4 = NULL;
      trim1(&s_k5, &t_k5);
      c_k5 = count_tran(t_k5, s_k5);
      
      /* J := Minstate(K5); (wipe K5) */
      t_j = t_k5;
      s_j = s_k5;
      t_k5 = NULL;
      s_k5 = 0;
      reach(&s_j, &t_j);
      minimize(&s_j, &t_j);
      
      c_j = count_tran(t_j, s_j);
   
      /* Iso_sizecount(J,K) */
      iso   = iso_sizecount(s_j, t_j, s_k, t_k);      
      
   } while (iso == false);
   
   *t3 = t_j;
   *s3 = s_j;
   
   freedes(s_k, &t_k);
}


void suprema_normal_scop1(state_node*  t1, INT_S s1,
                         state_node*  t2, INT_S s2,
                         state_node** t3, INT_S *s3,
                         INT_T *nullist, INT_T s_nullist)
{
   state_node *t_g, *t_j, *t_k, *t_k1, *t_k2, *t_k3;
   INT_S       s_g,  s_j,  s_k,  s_k1,  s_k2,  s_k3;
   INT_S             c_j,  c_k,  c_k1,  c_k2,  c_k3;
   INT_B  iso;
   INT_S  *macro_ab, *macro_c;
   INT_S  *mapState;

   macro_ab = NULL;  macro_c  = NULL;
   mapState = NULL;
   
   t_g = t_j = t_k = t_k1 = t_k2 = t_k3 = NULL;
   s_g = s_j = s_k = s_k1 = s_k2 = s_k3 = 0;
   
   /* J = DES1 */
   /* Copy t_j <- t1 */
   copy_des(&t_j,&s_j,t1,s1);   
   
   /* G = DES2 */
   /* Copy t_g <- t2 */
   copy_des(&t_g,&s_g,t2,s2);
   
   do {
      /* K := J */
      copy_des(&t_k, &s_k, t_j, s_j);
      c_k = count_tran(t_k, s_k);
      freedes(s_j, &t_j);  s_j = 0; t_j = NULL;
      
      /* K1 := Supnormclo(K, G, NULL event list); */
      suprema_normal_clo(t_k, s_k, t_g, s_g, &t_k1, &s_k1, nullist, s_nullist);
      c_k1 = count_tran(t_k1, s_k1);
      
      /* K2 := Supnorm(K1,G, Null event list) */
      suprema_normal(t_k1, s_k1, t_g, s_g, &t_k2, &s_k2, nullist, s_nullist);
      c_k2 = count_tran(t_k2, s_k2);
      freedes(s_k1, &t_k1); s_k1 = 0; t_k1 = NULL;
      
      /* K3 := Supcon(G,K2); (wipe K2) */
      meet2(s_g,t_g,s_k2,t_k2,&s_k3,&t_k3,&macro_ab,&macro_c); 
      freedes(s_k2,&t_k2); s_k2 = 0; t_k2 = NULL;
      trim2(&s_k3,&t_k3,macro_c);
      shave1(s_g,t_g,&s_k3,&t_k3,macro_c);
      free(macro_ab); free(macro_c);
      macro_ab = NULL;  macro_c = NULL;
      c_k3 = count_tran(t_k3, s_k3);
      
      /* J := Minstate(K3); (wipe) K3 */
      t_j = t_k3;
      s_j = s_k3;
      t_k3 = NULL;
      s_k3 = 0;
      reach(&s_j, &t_j);
      minimize(&s_j, &t_j);
      c_j = count_tran(t_j, s_j);      
         
      /* Iso_sizecount(J,K) */
      iso   = iso_sizecount(s_j, t_j, s_k, t_k);  
      
   } while (iso == false);
   
   *t3 = t_j;
   *s3 = s_j;
   
   freedes(s_k, &t_k);
}


#ifdef __cplusplus
}
#endif

