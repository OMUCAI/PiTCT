#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
// #include <io.h>

#include "setup.h"
#include "des_data.h"
#include "des_proc.h"
#include "des_supp.h"
// #include "curses.h"
#include "mymalloc.h"
#include "ext_des_proc.h"
#include "supred.h"
#include "obs_check.h"
#include "higen.h"
#include "tct_proc.h"

/*Copy from other files*/


void ext_copy_part( INT_S *s_dest,part_node **t_dest,
              INT_S s_src,part_node *t_src   )
{
    INT_S i;//, jj;
    INT_T j;//, ee;
//    INT_B  ok;

    
    *t_dest = (part_node*)REALLOC(*t_dest, s_src * sizeof(part_node));

    if ((s_src !=0) && (*t_dest == NULL)) {
      mem_result = 1;
      return;
    }
    *s_dest = s_src;

    for (i=0; i < s_src; i++) {
      (*t_dest)[i].numelts = 0; 
      (*t_dest)[i].next = NULL;
      (*t_dest)[i].next = (INT_S*)REALLOC((*t_dest)[i].next, t_src[i].numelts * sizeof(INT_S));
      if((*t_dest)[i].next == NULL && (*t_dest)[i].numelts != 0){
         mem_result = 1;
         free_part(*s_dest, t_dest);
         return;
      }
      (*t_dest)[i].numelts = t_src[i].numelts;
      for (j=0; j < t_src[i].numelts; j++) {
         (*t_dest)[i].next[j] = t_src[i].next[j];
      }
    }
}


void local_coreach1(tran_node  *init,
              INT_S      s_init,
              INT_S      s1,
              state_node **t1,
              char*      un)
{
   INT_B  ok, ok2, found;
   INT_T cur;
   INT_S s;
   INT_S es;
   INT_S i;
   char* visited;
   t_stack ts;
   long num_hits = 0;

   visited = (char*) CALLOC(s1, sizeof(char));

   pstack_Init(&ts);

   cur = 0;
   s = s_init;

   /* To avoid selfloop, we mark the first state as being visited */
   visited[s] = 1;

   while (!( (cur >= (*t1)[s].numelts) && pstack_IsEmpty(&ts) )) {
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
      for (i=0; i < s1; i++) {
        if (visited[i] == 1)
           un[i] = 1;
      }
   }

   pstack_Done(&ts);
   free(visited);
}

void local_coreach2(INT_S s1,
              state_node **t1)
{
   INT_S state;
   char* un;

   if (!(*t1)[0].marked)
     (*t1)[0].coreach = false;

   /* Should always have "coreach" field in "t1" set to false
      to start */
   for (state=1; state < s1; state++)
     (*t1)[state].coreach = false;

   un = (char*) CALLOC(s1, sizeof(char));

   for (state=0; state < s1; state++) {
     if ((*t1)[state].marked || (*t1)[state].coreach) {
       (*t1)[state].coreach = true;
     } else if (un[state] == 1) {
       /* Already determined not a coreachable state */
     } else {
       /* Can we be coreached */
       local_coreach1((*t1)[state].next, state, s1, t1, un);
     }
   }

   free(un);
}
void sr_search(INT_S s_init, state_node **t1, state_node **t2, INT_S s1, INT_S end)
{
   INT_T cur;
   INT_S s;
   INT_S es,ee;
   t_queue tq;
   INT_B ok;

   /* Assume the "reached" field in state_node structure already
      set correctly.  In most cases, it means all false except,
      the zero state but not always. */

   queue_init(&tq);

   enqueue(&tq, s_init);

   while (!queue_empty(&tq)) {
      s = dequeue(&tq);
      for (cur=0; cur < (*t1)[s].numelts; cur++) {
         ee = (*t1)[s].next[cur].data1;
         es = (*t1)[s].next[cur].data2;
         if (!(*t1)[es].reached) {
            enqueue(&tq,es);
            (*t1)[es].reached = true;
            addordlist1((INT_T)ee, s, &(*t2)[es].next, (*t2)[es].numelts, &ok);
            if(ok) (*t2)[es].numelts ++;
         }
         if(es == end)
            goto FREE_QUEUE;
      }
   }
FREE_QUEUE:
   while(!queue_empty(&tq))
      dequeue(&tq);
   queue_done(&tq);
}
void combine_des(INT_S *s1, state_node **t1, INT_S s2, state_node *t2)
{
	INT_S i, jj;
	INT_T j, ee;
	INT_B ok;

	if(*s1 < s2){
		*t1 = (state_node *)REALLOC(*t1, s2 * sizeof(state_node));
		if(mem_result == 1)
			return;
		for(i = *s1; i < s2; i ++){
			(*t1)[i].reached = t2[i].reached;
			(*t1)[i].coreach = t2[i].coreach;
			(*t1)[i].marked = t2[i].marked;
			(*t1)[i].vocal = t2[i].vocal;
			(*t1)[i].numelts = 0;
			(*t1)[i].next = NULL;
		}
	}
	for(i = 0; i < s2; i ++){
		for(j = 0; j < t2[i].numelts; j ++){
			ee = t2[i].next[j].data1;
			jj = t2[i].next[j].data2;
			addordlist1(ee,jj, &(*t1)[i].next, (*t1)[i].numelts, &ok);
			if(ok) (*t1)[i].numelts ++;
			if(mem_result == 1)
				return;
		}
	}
}
INT_OS path2deadlock_proc(char* name2, char* name1)
{
	INT_S s1, s2, s3, s4, init;
	state_node *t1, *t2, *t3, *t4;
	INT_OS result;
	INT_S state, i, j;
	INT_S slist, *list;
	INT_B ok;

	result = 0;

	s1 = s2 = s3 = s4 = 0;
	t1 = t2 = t3 = t4 = NULL;
	slist = 0; list = NULL;

	init = 0L;
	if(!getdes(name1, &s1, &init, &t1)){
		result = -1;
		goto Free_memory;
	}

	if (s1 <= 0){
		result = 1;
		goto Free_memory;
	}


	// Step1: find the deadlock states
	for (state=0; state < s1; state++)
		t1[state].reached = false;

	t1[0].reached = true;
	b_reach(t1[0].next, 0L, &t1, s1);

	for(state = 0; state < s1; state ++){
		if(t1[state].reached){
			if(t1[state].numelts == 0 && t1[state].marked == false){
				addstatelist(state, &list, slist, &ok);
				if(ok) slist ++;
			}
		}
	}


	//Step 2: find shortest paths to each block state
	// store the paths into t4
	s4 = s1;
	t4 = newdes(s4);

	for(i = 0; i < slist; i ++){
		state = list[i];
		s2 = s1;
		t2 = newdes(s2);
		//find the shortest rountine
		for (j =0; j < s1; j++)
			t1[j].reached = false;
		t1[0].reached = true;
		sr_search(0, &t1, &t2, s1, state);
		for(j = 0; j < s2; j ++){
			t2[j].reached = false;
		}

		t2[state].reached = true;

		b_reach(t2[state].next, state, &t2, s2);  

		
		reverse_des(&s3, &t3, s2, t2);
		for(j = 0; j < s2; j ++){
			t3[j].reached = t2[j].reached;

		}
		freedes(s2, &t2);
		s2 = 0; t2 = NULL;

		purgebadstates(s3, &t3);

		combine_des(&s4, &t4, s3, t3);
		if(mem_result == 1){
			result = -1;
			goto Free_memory;
		}
		freedes(s3, &t3);
		s3 = 0; t3 = NULL;
	}
	init = 0L;
	filedes(name2, s4, init, t4);

Free_memory:
	if(result == 1){
		result = 0;
		filedes(name2, 0, 0, NULL);
	}
	freedes(s1, &t1);
	freedes(s2, &t2);
	freedes(s3, &t3);
	freedes(s4, &t4);
	free(list);

	return result;
}

INT_OS path2block_proc(char* name2, char* name1)
{
	INT_S s1, s2, s3, s4, init;
	state_node *t1, *t2, *t3, *t4;
	INT_OS result;
	INT_S state, i, j;
	INT_S slist, *list;
	INT_B ok;

	result = 0;

	s1 = s2 = s3 = s4 = 0;
	t1 = t2 = t3 = t4 = NULL;
	slist = 0; list = NULL;

	init = 0L;
	if(!getdes(name1, &s1, &init, &t1)){
		result = -1;
		goto Free_memory;
	}

	if (s1 <= 0){
		result = 1;
		goto Free_memory;
	}


	// Step1: find the block states
	for (state=0; state < s1; state++)
		t1[state].reached = false;

	t1[0].reached = true;
	b_reach(t1[0].next, 0L, &t1, s1);
	local_coreach2(s1, &t1);

	for (state = s1-1L; state >= 0; state--) {
		if (t1[state].reached && t1[state].coreach)
			s2++;
		else{
			addstatelist(state, &list, slist, &ok);
			if(ok) slist ++;
		}
	}
	if (s2 == s1) {
		result = 1;
		goto Free_memory;
	}

	//Step 2: find shortest paths to each block state
	// store the paths into t4
	s4 = s1;
	t4 = newdes(s4);

	for(i = 0; i < slist; i ++){
		state = list[i];
		s2 = s1;
		t2 = newdes(s2);
		//find the shortest rountine
		for (j =0; j < s1; j++)
			t1[j].reached = false;
		t1[0].reached = true;
		sr_search(0, &t1, &t2, s1, state);
		for(j = 0; j < s2; j ++){
			t2[j].reached = false;
		}

		t2[state].reached = true;

		b_reach(t2[state].next, state, &t2, s2);  


		reverse_des(&s3, &t3, s2, t2);
		for(j = 0; j < s2; j ++){
			t3[j].reached = t2[j].reached;

		}
		freedes(s2, &t2);
		s2 = 0; t2 = NULL;

		purgebadstates(s3, &t3);

		combine_des(&s4, &t4, s3, t3);
		if(mem_result == 1){
			result = -1;
			goto Free_memory;
		}
		freedes(s3, &t3);
		s3 = 0; t3 = NULL;
	}
	init = 0L;
	filedes(name2, s4, init, t4);

Free_memory:
	if(result == 1){
		result = 0;
		filedes(name2, 0, 0, NULL);
	}
	freedes(s1, &t1);
	freedes(s2, &t2);
	freedes(s3, &t3);
	freedes(s4, &t4);
	free(list);

	return result;
}

INT_OS ext_occ_proc(INT_S s1, state_node *t1, 
                 INT_T s_imagelist, INT_T *imagelist, 
                 INT_T *s_ext_imagelist, INT_T **ext_imagelist)
{
     INT_S s2;
     state_node  *t2;
     INT_S i,j;
     INT_OS result = 0;
     INT_T ee;//, k;
     INT_S s, es;
     t_queue tq;
     INT_B  occ_flag, ok;
     
     s2 = 0;  t2 = NULL;
     occ_flag = false;
     
     reverse_des(&s2, &t2, s1, t1);
     if(mem_result == 1){
        return -1;
     }
     
     for(i = 0; i < s1; i ++){
        t1[i].reached = t1[i].coreach = false;
        for(j = 0; j < t1[i].numelts; j ++){
           ee = t1[i].next[j].data1;
           if(ee %2 == 0 && inlist(ee, imagelist, s_imagelist)){
              t1[i].coreach = true;   // If event ee defined at i is observable and uncontrollable, set the coreach bit as true.
           }
        }
     }
     
     *s_ext_imagelist = s_imagelist;
     *ext_imagelist = (INT_T*)CALLOC(s_imagelist, sizeof(INT_T));
     for(i = 0; i < s_imagelist; i ++)
        (*ext_imagelist)[i] = imagelist[i];
     while(!occ_flag){
     occ_flag = true;
     for(i = 0; i < s2; i ++){
        if(t1[i].coreach){
           for(j = 0; j < s2; j ++){
              t2[j].reached = false;
           }
           queue_init(&tq);
           enqueue(&tq, i);
           while(!queue_empty(&tq)){
              s = dequeue(&tq);
              t2[s].reached = true;
              for(j = 0; j < t2[s].numelts; j ++){
                 ee = t2[s].next[j].data1;
                 es = t2[s].next[j].data2;
                 if(!inlist(ee, *ext_imagelist, *s_ext_imagelist)){
                    if(ee %2 == 0){
                       if(!t2[es].reached){
                          enqueue(&tq, es);
                       }
                    }else{
                        addordlist(ee, ext_imagelist, *s_ext_imagelist, &ok);
                        if(ok) (*s_ext_imagelist) ++;
                        occ_flag = false;
                    }
                 }
              }
           }
        }
     }
     }
     //init = 0L;
     //filedes(name2, s1, init, t1);

     freedes(s2, &t2);
     return result;
}

void rendez(INT_S *s3, state_node **t3, 
               INT_S s1, INT_S s2, 
               INT_S s_colist, INT_S *colist)
{
    INT_S i,cur;//macrostate,
    INT_B  ok, reach_flag; 
    INT_S s_curlist, s_prelist, *curlist, *prelist;
    INT_S es,ss; INT_T ee;
    
    reach_flag = false;
    s_curlist = s_prelist = 0;
    curlist = prelist = NULL;
    
    if(*s3 == 0)
       return;
    
    for(i = 0; i < *s3; i ++){
       (*t3)[i].marked = false;
       (*t3)[i].reached = false;
    }
       
    for(i = 0; i < s_colist; i ++)
       (*t3)[colist[i]].marked = true;
    
    ss = 0;
    addstatelist(ss, &curlist, s_curlist, &ok);
    if(ok) s_curlist ++;
    while(!reach_flag){
       for(i = 0; i < s_curlist; i ++){
          ss = curlist[i];
          (*t3)[ss].reached = true;
          for (cur=0; cur < (*t3)[ss].numelts; cur++) {
             ee = (*t3)[ss].next[cur].data1;
             es = (*t3)[ss].next[cur].data2;
             if((*t3)[es].reached){
                delete_ordlist1(ee,es,&((*t3)[ss].next), (*t3)[ss].numelts, &ok);
                if(ok) (*t3)[ss].numelts --;
                cur --;
                continue;
             }
             if ((*t3)[es].marked) {
                reach_flag = true;
                (*t3)[es].coreach = true;
             }
             addstatelist(es, &prelist, s_prelist, &ok);
             if(ok) s_prelist ++;
          }
       }
       s_curlist = s_prelist;
       curlist = (INT_S*)REALLOC(curlist, s_curlist * sizeof(INT_S));
       if(curlist == NULL){
          goto COOPERATE_LABEL;
          return;
       }
       for(i = 0; i < s_prelist; i++)
          curlist[i] = prelist[i];
       free(prelist);
       s_prelist = 0; prelist = NULL;
    }
    for(i = 0; i < *s3; i ++){
       if((*t3)[i].coreach){
          (*t3)[i].reached = true;
          free((*t3)[i].next);
          (*t3)[i].next = NULL;
          (*t3)[i].numelts = 0;
       }else
          (*t3)[i].marked = false;
       (*t3)[i].coreach = false;
    }
    purgebadstates(*s3, t3);
    
    local_coreach2(*s3, t3);
    
    for(i = 0; i < *s3; i ++){
       if((*t3)[i].coreach)
          (*t3)[i].reached = true;
       else
          (*t3)[i].reached = false;
    }
    
    purgebadstates(*s3, t3);
    
COOPERATE_LABEL:    
    free(curlist);
    free(prelist);
}

INT_OS rendez_proc(char *name1, INT_OS size, char (*names1)[MAX_FILENAME], INT_T*tranlist, INT_T s_tranlist, part_node *sp, INT_S s_sp)
{
    state_node *t1, *t2, *t3;
    INT_S s1, s2, s3, init;
    INT_S  *macro_ab, *macro_c, macrostate;
    INT_S ss1, ss2;
    INT_S i,j;//,k;
    
    INT_B  ok;
    INT_OS result;
    INT_S s_colist,*colist;
    
    INT_S s_par;
    part_node *par;
    
    s1 = s2 = s3 = 0;
    t1 = t2 = t3 = NULL;
    macro_ab = macro_c = NULL;
    s_colist = 0; 
    colist = NULL;
    s_par = 0; par = NULL;
    
    result = 0;
    ext_copy_part(&s_par, &par, s_sp, sp);

    //Synchronize the first two DES1 and DES2
    init = 0L;
    if(getdes(names1[0], &s1, &init, &t1) == false){
       result = -1;
       goto RENDEZ_LABEL;
    }
    init = 0L;
    if(getdes(names1[1], &s2, &init, &t2) == false){
       result = -1;
       goto RENDEZ_LABEL;
    }
    
    sync3(s1, t1, s2, t2, &s3, &t3, 0, s_tranlist, tranlist, &macro_ab, &macro_c);
    for(i = 0; i < s_par; i ++){
       ss1 = par[i].next[0];
       ss2 = par[i].next[1];
       macrostate = macro_ab[ss2*s1 + ss1];
       par[i].next[1] = macrostate;
    }
    
    for(i = 2; i < size; i ++){
       // Copy the prev result DES3 to new component DES1
       freedes(s1, &t1); s1 = 0; t1 = NULL;
       freedes(s2, &t2); s2 = 0; t2 = NULL;
       export_copy_des(&s1, &t1, s3, t3);
       freedes(s3, &t3); s3 = 0; t3 = NULL;
       free(macro_ab); free(macro_c);
       macro_ab = NULL; macro_c = NULL;
       // Get new component DES2
       init = 0L;
       if(getdes(names1[i], &s2, &init, &t2) == false){
          result = -1;
          goto RENDEZ_LABEL;
       }
       sync3(s1, t1, s2, t2, &s3, &t3, 1, s_tranlist, tranlist, &macro_ab, &macro_c);
       for(j = 0; j < s_par; j ++){
          ss1 = par[j].next[i-1];
          ss2 = par[j].next[i];
          if(ss1 > -1 && ss2 > -1){
              macrostate = macro_ab[ss2*s1 + ss1];              
          }else
             macrostate = -1;
          par[j].next[i] = macrostate;
       }
    }
    for(i = 0; i < s_par; i ++){
       macrostate = par[i].next[size - 1];
       if(macrostate >= 0 && macrostate < s3){
          addstatelist(par[i].next[size - 1], &colist, s_colist, &ok);
          if(ok) s_colist ++;
       }
    }
    
    rendez(&s3,&t3,s1,s2,s_colist, colist);
    reach(&s3,&t3);
    
    if(mem_result != 1){
       init = 0L;
       filedes(name1, s3, init, t3);
    }
    
RENDEZ_LABEL:    
    freedes(s1, &t1);
    freedes(s2, &t2);
    freedes(s3, &t3);
    free(macro_ab);
    free(macro_c);
    free(colist);
    free_part(s_par, &par);
    return 0;
}

INT_B index_list(INT_T elem, INT_T *list, INT_T s_list, INT_T *index)
{
    INT_T i;
    
    for(i = 0; i < s_list; i ++){
       if(elem == list[i]){
          *index = i;
          return true;
       }
    }
    return false;
}

INT_OS splitevent_proc(char *name1, char *name2, 
                         INT_T s_el, INT_T *el,
                         INT_S s_ep, part_node *ep)
{
    INT_S s1, s2, init;
    state_node *t1, *t2;
    INT_S i,j, k;
    INT_T event, index;
    INT_S state;
    INT_B  ok;   
    INT_OS result; 
    
    s1 = s2 = 0;
    t1 = t2 = NULL;
    
    result = 0;
    
    init = 0L;
    if(!getdes(name1, &s1, &init, &t1))
       return -1;
       
    export_copy_des(&s2, &t2, s1, t1);
    if(mem_result == 1){
       freedes(s1, &t1);
       return 3;
    }
    
    for(i = 0; i < s2; i ++){
       for(j = 0; j < t2[i].numelts; j ++){
          event = t2[i].next[j].data1;          
          if(index_list(event, el, s_el, &index)){
             state = t2[i].next[j].data2;
             delete_ordlist1(event, state, &t1[i].next, t1[i].numelts, &ok);
             if(ok) t1[i].numelts --;
             
             for(k = 0; k < ep[index].numelts; k ++){
                addordlist1((INT_T)ep[index].next[k], state, &t1[i].next, t1[i].numelts, &ok);
                if(ok) t1[i].numelts ++;
             }
          }
       }
    }
    
    if(mem_result != 1){
       init = 0L;
       filedes(name2, s1, init, t1);
    }else{
       result = 3;
    }
    
    freedes(s1, &t1);
    freedes(s2, &t2);
    
    return result;
}

void augment(INT_S *s2, state_node **t2, INT_T s_list, INT_T *list)
{
	state_node *t1;
	INT_S s1, ss;
	INT_S i,j,k;
	INT_B  ok;

	s1  = 0; t1  = NULL;

	export_copy_des(&s1, &t1, *s2, *t2);
	freedes(*s2, t2);
	*s2 = 0; *t2 = NULL;

	for(i = 0; i < s1; i ++)
		t1[i].marked = false;

	for(i = 0; i < s1; i ++){
		for(j = 0; j < s_list; j ++){
			if(!inordlist1(list[j], t1[i].next, t1[i].numelts)){
				addordlist1(list[j],s1, &t1[i].next, t1[i].numelts, &ok);
				if(ok) t1[i].numelts ++;
			}else{
				for(k = 0; k < t1[i].numelts; k ++){
					if(t1[i].next[k].data1 == list[j]){
						ss = t1[i].next[k].data2;
						t1[ss].marked = true;
					}
				}
			}
		}
	}
	s1 ++;
	t1 = (state_node *)REALLOC(t1, s1 * sizeof(state_node));
	if(mem_result == 1 || t1 == NULL){
		freedes(s1, &t1);
		return ;
	}

	t1[s1 - 1].next = NULL; t1[s1 - 1].numelts = 0;
	t1[s1- 1].vocal = 0; t1[s1].marked = true;

	export_copy_des(s2, t2, s1, t1);

	freedes(s1, &t1);
}

INT_OS augment_proc(char * name2, char * name1, INT_T s_list, INT_T *list)
{
    state_node *t1, *t2;
    INT_S s1, s2, init;//, ss;
//    INT_S i,j,k;
//    INT_B  ok;
    
    s1 = s2 = 0; t1 = t2 = NULL;
    
    init = 0L;
    if(getdes(name1, &s1, &init, &t1) == false)
       return -1;

	augment(&s1, &t1, s_list, list);
	/*for(i = 0; i < s1; i ++)
		t1[i].marked = false;
    
    for(i = 0; i < s1; i ++){
       for(j = 0; j < s_list; j ++){
          if(!inordlist1(list[j], t1[i].next, t1[i].numelts)){
             addordlist1(list[j],s1, &t1[i].next, t1[i].numelts, &ok);
             if(ok) t1[i].numelts ++;
          }else{
			  for(k = 0; k < t1[i].numelts; k ++){
				  if(t1[i].next[k].data1 == list[j]){
					  ss = t1[i].next[k].data2;
					  t1[ss].marked = true;
				  }
			  }
		  }
       }
    }
	s1 ++;
	t1 = (state_node *)REALLOC(t1, s1 * sizeof(state_node));
    if(mem_result == 1 || t1 == NULL){
       freedes(s1, &t1);
       return -1;
    }

	t1[s1 - 1].next = NULL; t1[s1 - 1].numelts = 0;
	t1[s1- 1].vocal = 0; t1[s1].marked = true;
	*/
    
    init = 0L;
    filedes(name2, s1, init, t1);
    
    freedes(s1, &t1);
    
    return 0;
}
void con_genlist(INT_T slist1,
             tran_node *list1,
             INT_T slist2,
             tran_node *list2,
             INT_S s,
             state_node *t)
{
   INT_T j1, j2;
   INT_B  ok;

   j1 = j2 = 0;
   while ((j1 < slist1) && (j2 < slist2)) {
     if (list1[j1].data1 == list2[j2].data1) {
        j1++; j2++;
     } else if (list1[j1].data1 > list2[j2].data1) {
        j2++;
     } else {
        addordlist1(list1[j1].data1, 0, &t[s].next, t[s].numelts, &ok);
        if (ok) t[s].numelts++;
        j1++;
     }
   }

   while (j1 < slist1) {
     addordlist1(list1[j1].data1, 0, &t[s].next, t[s].numelts, &ok);
     if (ok) t[s].numelts++;
     j1++;
   }
}

void sync_condat1(INT_S s1, state_node *t1,             
                  INT_S s2, state_node *t2,
                  INT_S s3,
                  INT_S s4, state_node *t4,
                  INT_S *s5, state_node **t5,
                  INT_S *macro_c,
                  INT_S *macro_d)
{
   INT_S state, state1, state2, state3;

   *s5 = s2;
   *t5 = newdes(*s5);

   for (state=0; state < s4; state++) {
     state1 = macro_d[state] % s3;   // Corresponding state in t3
     state2 = macro_c[state1] % s1;  // Corresponding state in t1
     state3 = macro_c[state1] / s1;  // Corresponding state in t2

     con_genlist(t1[state2].numelts, t1[state2].next,
             t2[state3].numelts, t2[state3].next, state3, *t5);
   }
}

INT_OS syncondat_proc1(char *name4, char * name1, char *name2, char *name3)
{
    state_node *t1, *t2, *t3, *t4, *t5, *t6;
    INT_S s1, s2, s3, s4, s5, s6, init;
    INT_S  *macro_ab, *macro_c, *macro_d;
    
    INT_OS result = 0;

    macro_ab = macro_c = macro_d = NULL;
    t1 = t2 = t3 = t4 = t5 = t6 = NULL;
    s1 = s2 = s3 = s4 = s5 = s6 = 0;
    
    init = 0L;    
    getdes(name1, &s1, &init, &t1);
    
    init = 0L;
    getdes(name2, &s2, &init, &t2);
    
    init = 0L;
    getdes(name3, &s3, &init, &t3);
    
    if(mem_result == 1){
       result = 1;
       goto SCONDAT_LABEL;
    }
    
    sync2(s1,t1,s2,t2,&s4,&t4,&macro_ab,&macro_c);
    //export_copy_des(&s5,&t5, s4, t4);
    free(macro_ab); macro_ab = NULL;
    sync2(s4,t4,s3,t3,&s5,&t5,&macro_ab,&macro_d);        
    
    freedes(s1, &t1); s1 = 0; t1 = NULL;
    freedes(s2, &t2); s2 = 0; t2 = NULL;
    
    init = 0L;    
    getdes(name1, &s1, &init, &t1);
    
    init = 0L;
    getdes(name2, &s2, &init, &t2);
    
    sync_condat1(s1,t1,s2,t2, s4, s5,t5, &s6, &t6, macro_c, macro_d);
   
    if (mem_result != 1)
    {
       filedes(name4, s6, -1L, t6);  
    }
SCONDAT_LABEL:    
    freedes(s1, &t1);
    freedes(s2, &t2);
    freedes(s3, &t3);
    freedes(s4, &t4);
    freedes(s5, &t5);
    freedes(s6, &t6);
        
    free(macro_ab);
    free(macro_c);
    free(macro_d);
    return result;
}

INT_OS syncondat_proc(char *name4, char * name1, char *name2, char *name3, char *name5)
{
    state_node *t1, *t2, *t3, *t4, *t5, *t6;
    INT_S s1, s2, s3, s4, s5, s6, init;
    INT_S  *macro_ab, *macro_c, *macro_d;
    INT_S i, j, state1, state2;
    INT_T slist, *list, event;
    INT_B  ok;
    
    INT_OS result = 0;

    macro_ab = macro_c = macro_d = NULL;
    t1 = t2 = t3 = t4 = t5 = t6 = NULL;
    s1 = s2 = s3 = s4 = s5 = s6 = 0;
    slist = 0; list = NULL;
    
    init = 0L;    
    getdes(name1, &s1, &init, &t1);
    
    init = -1L;
    getdes(name2, &s2, &init, &t2);
    
    init = 0L;
    getdes(name3, &s3, &init, &t3);
    
    init = 0L;
    getdes(name5, &s4, &init, &t4);
    gentranlist(s4, t4, &slist, &list);
    
    if(mem_result == 1){
       result = 1;
       goto SCONDAT_LABEL;
    }
    
    sync2(s1,t1,s3,t3,&s5,&t5,&macro_ab,&macro_c);    

    s6 = s3;
    t6 = newdes(s6);
    
    for(i = 0; i < s5; i ++){
       state1 = macro_c[i] % s1;
       state2 = macro_c[i] / s1;
       for(j = 0; j < t2[state1].numelts; j ++){
          event = t2[state1].next[j].data1;
          if(inlist(event, list, slist)){
             addordlist1(event, 0, &t6[state2].next, t6[state2].numelts, &ok);
             if(ok) t6[state2].numelts ++;
          }
       }
    }
   
    if (mem_result != 1)
    {
       filedes(name4, s6, -1L, t6);  
    }
SCONDAT_LABEL:    
    freedes(s1, &t1);
    freedes(s2, &t2);
    freedes(s3, &t3);
    freedes(s4, &t4);
    freedes(s5, &t5);
    freedes(s6, &t6);
        
    free(macro_ab);
    free(macro_c);
    return result;
}

INT_OS attach_proc(char * name3, char * name1, char * name2, INT_S state)
{
    state_node *t1, *t2;
    INT_S s1, s2;
    INT_S init;
    INT_S i, j;
    INT_S ss, s, ors;
    INT_OS result;
    INT_B  ok;
    
    t1 = t2 = NULL;
    s1 = s2 = 0;
    
    result = 0;
    
    init = 0L;
    if(!getdes(name1, &s1, &init, &t1))
       return 0;
    
    init = 0L;
    if(!getdes(name2, &s2, &init, &t2)){
       result = 1;
       goto ATTACH_FREE;
    }

    ors = s1;
    s1 = s1 + s2 - 1;
    t1 = (state_node *)REALLOC(t1,s1 * sizeof(state_node));
    for(i = 1; i < s2; i ++){
       t1[ors + i - 1].next = NULL;
       t1[ors + i - 1].numelts = 0;
    }
    if(t1 == NULL && s1 != 0){
       result = 1;
       goto ATTACH_FREE;
    }

    for(j = 0; j < t2[0].numelts; j ++){
       ss = t2[0].next[j].data2;
       if(ss == 0)  ss = state;
       else  ss = ors + ss - 1;
       addordlist1(t2[0].next[j].data1, ss, &t1[state].next, t1[state].numelts, &ok);
       if(ok) t1[state].numelts ++;
    }
    
   for(i = 1; i < s2; i ++){
       for(j = 0; j < t2[i].numelts; j ++){
          ss = t2[i].next[j].data2;
          if(ss == 0)  ss = state;
          else ss = ors + ss - 1;
          s = ors + i - 1;
          addordlist1(t2[i].next[j].data1, ss, &t1[s].next, t1[s].numelts, &ok);
          if(ok) t1[s].numelts ++;
          t1[s].marked = t2[i].marked;
          t1[s].vocal = t2[i].vocal;
       }
    }
    
    if(mem_result == 1){
       result = 1;
       goto ATTACH_FREE;
    }
    
    init = 0L;
    filedes(name3, s1, init, t1);

ATTACH_FREE:
    freedes(s1, &t1);
    freedes(s2, &t2);
    
    return result;
}

// extension of insertlist4 function: initialize the all the memory
// between s and max(src, dest)
void ext_insertlist(INT_S src,
	INT_T tran,
	INT_S dest,
	INT_S *s,
	state_node **t)
{
	INT_B  ok;
	INT_S i;
	state_node *temp;
	INT_S size;

	size = (INT_S)max(src, dest);

	if (size < *s) {
		addordlist1(tran, dest, &(*t)[src].next, (*t)[src].numelts, &ok);
		if (ok) (*t)[src].numelts++;
	} else {
		/* Need to increase the size of the array */
		if(size % 2 == 1)
			size = size - 1;
		temp = (state_node*) REALLOC(*t, sizeof(state_node)*(size + 2));

		if (temp == NULL) {
			mem_result = 1;
			return;
		}
		*t = temp;

		for (i=*s; i < size + 2; i++) {
			(*t)[i].marked       = false;
			(*t)[i].reached      = false;
			(*t)[i].coreach      = false;
			(*t)[i].vocal        = 0;
			(*t)[i].numelts      = 0;
			(*t)[i].next         = NULL;
		}

		*s = size + 2;
		addordlist1(tran, dest, &(*t)[src].next, (*t)[src].numelts, &ok);
		if (ok) (*t)[src].numelts++;
	}
}
// meet(DES1, DES2) based on some event set defined at each state of DES1
// Also identify the marked and unmarked state
void ext_meet(INT_S s1, state_node *t1,	INT_S s2, state_node *t2, INT_S s_par, part_node *par,
				INT_S *s3, state_node **t3, INT_S **macro_ab, INT_S **macro_c)
{
	INT_S t1i, t2j;
	INT_T colptr1, colptr2;
	INT_T tran1, tran2;
	INT_S srcstate, newstate, macrostate;
	INT_S a,b,c,i;

	if (s1 == 0L || s2 == 0L) {
		*s3 = 0;
		*t3 = NULL;
		return;
	}
	*s3  = 0;
	*t3  = NULL;

	*macro_ab = (INT_S*) CALLOC(2*s1*s2, sizeof(INT_S));
	*macro_c  = (INT_S*) CALLOC(2*s1*s2, sizeof(INT_S));

	if ( (*macro_ab == NULL) || (*macro_c == NULL) || s1 * s2 < 0) {
		mem_result = 1;
		return;
	}

	for (i = 0; i < 2* s1 * s2; i++){ 
		(*macro_ab)[i] = -1L;
		(*macro_c)[i]  = -1L;
	}

	(*macro_ab)[0] = (*macro_c)[0] = 0L;
	(*macro_ab)[1] = 1L;
	(*macro_c)[1] = 1;
	srcstate = macrostate = newstate = 0L;
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
			if((t1[t1i].marked == true) && (instatelist(tran1, par[t1i].next, par[t1i].numelts)))
				c = 1;
			else
				c = 0;
			macrostate = (*macro_ab)[b*s1*2 + a * 2 + c];     
			if (macrostate == -1L) {
				newstate += 2;
				(*macro_ab)[b*s1*2 + a * 2 ] = newstate;
				(*macro_c)[newstate] = b*s1*2 + a * 2 ;
				(*macro_ab)[b*s1*2 + a * 2 + 1] = newstate + 1;
				(*macro_c)[newstate + 1] = b*s1*2 + a * 2 + 1;
				ext_insertlist(srcstate, tran1, newstate + c, s3, t3);
				ext_insertlist(srcstate + 1, tran1, newstate + c, s3, t3);
				if (mem_result == 1) return;
			} else {
				ext_insertlist(srcstate, tran1, macrostate, s3, t3);
				ext_insertlist(srcstate + 1, tran1, macrostate, s3, t3);
				if (mem_result == 1) return;
			}
			colptr1++;
			colptr2++;
		}

		srcstate+=2;
		a = (*macro_c)[srcstate];
		if (a != -1L) {
			t1i = (a % (s1 * 2))/2;
			t2j = a / (s1 * 2);
		}

	} while (srcstate <= newstate);

	*s3 = newstate + 2;

	//(*t3)[0].reached = true;
	for (i=1; i < *s3; i+=2) {
		(*t3)[i].marked = true;
	}
	//(*t3)[0].marked = t1[0].marked;
	/* Should be safe because the size is smaller than before */
	*macro_c = (INT_S*) REALLOC(*macro_c, sizeof(INT_S)*(*s3));
}

// Realization of L\Sigma: = {s\sigma|s \in L, & \sigma \in \Sigma} 
// The key point is characterize the transitions leading to marked state and unmarked state 
// We search the DES by breadth first approach
void language_cat_event(INT_S *s1, state_node **t1, INT_S s_event_par, part_node *event_par)
{
 /*  INT_T cur;
   INT_S s;
   INT_S es;
   t_queue tq;
   long num_hits = 0;

  queue_init(&tq);

   enqueue(&tq, s_init);

   while (!queue_empty(&tq)) {
      s = dequeue(&tq);
      for (cur=0; cur < (*t1)[s].numelts; cur++) {
         es = (*t1)[s].next[cur].data2;
         if (!(*t1)[es].reached) {
            enqueue(&tq,es);
            (*t1)[es].reached = true;

            if (debug_mode) {
               if (num_hits % 10000L == 0L) {
                   move(23,0); clrtoeol();
                   printw("B_REACH: %ld", num_hits);
                   refresh();
               }
               num_hits++;
            }
         }
      }
   }

   queue_done(&tq);*/
}

void project_f_proc(INT_S s1, state_node *t1, INT_S *s2, state_node **t2, INT_T s_nullist,INT_T *nullist)
{
	state_map* macrosets; INT_S s_macrosets;
	INT_S *macro_ab, *macro_c;
	INT_S s3, s4, s5;
	state_node *t3, *t4, *t5;
	INT_S s_par1, s_par2;
	part_node *par1, *par2;
	INT_S i,j, k, state, ss;//, newstate;
	INT_T ee;
	INT_B ok;

	macrosets = NULL; s_macrosets = 0;
	macro_ab = macro_c = NULL;
	s3 = s4 = s5 = 0; 
	t3 = t4 = t5 = NULL;
	s_par1 = s_par2 = 0;
	par1 = par2 = NULL;

	//ext_meet(s1, t1, *s2, *t2, s_par2, par2, &s4, &t4, &macro_ab, &macro_c);

	//filedes("TEST", s4, 0, t4);

	export_copy_des(&s3, &t3, *s2, *t2);

	obs_project(&s3, &t3, s_nullist, nullist, &s_macrosets, &macrosets);

//	export_copy_des(&s5, &t5, s1, t1);
//	augment(&s5, &t5, s_nullist, nullist);
	//zprint_map(s_macrosets, macrosets);

	meet2(s1,t1, *s2, *t2, &s4, &t4, &macro_ab, &macro_c);

	s_par1 = *s2;
	par1 = (part_node *)CALLOC(s_par1, sizeof(part_node));
	if(par1 == NULL){
		mem_result = 1;
		goto FREE_MEM;
	}
	for(i = 0; i < s4; i ++){
		state = macro_c[i]/s1;
		for(j = 0; j < (*t2)[state].numelts; j ++){
			ee = (*t2)[state].next[j].data1;
			ss = (*t2)[state].next[j].data2;
			//if(inlist(ee, nullist, s_nullist) && ((*t2)[ss].marked == true)){
			if((*t2)[ss].marked == true){
				addstatelist(ee, &par1[state].next, par1[state].numelts, &ok);
				if(ok) par1[state].numelts ++;
			}
			//}
		}
	}
	//zprint_par(s_par1, par1);

	s_par2 = s_macrosets;
	par2 = (part_node *)CALLOC(s_par2, sizeof(part_node));
	if(par2 == NULL){
		mem_result = 1;
		goto FREE_MEM;
	}

	for(i = 0; i < s_macrosets; i ++){
		for(j = 0; j < macrosets[i].numelts; j ++){
			state = macrosets[i].next[j];
			for(k = 0; k < par1[state].numelts; k ++){
				addstatelist(par1[state].next[k], &par2[i].next, par2[i].numelts, &ok);
				if(ok) par2[i].numelts ++;
			}
		}
	}
	//zprint_par(s_par2, par2);

	for(i = 0; i < s3; i ++){
		for(j = 0; j < s_nullist; j ++){
			addordlist1(nullist[j], i, &t3[i].next, t3[i].numelts, &ok);
			if(ok)  t3[i].numelts ++;
		}
	}
	/*for(i = 0; i < s3; i ++){
		for(j = 0; j < par2[i].numelts; j ++){
			addordlist1(par2[i].next[j], i, &t3[i].next, t3[i].numelts, &ok);
			if(ok)  t3[i].numelts ++;
		}
	}*/

//	filedes("TEST", s3, 0, t3);

	freedes(s4, &t4); s4 = 0; t4 = NULL;
	free(macro_ab); free(macro_c);
	macro_ab = macro_c = NULL;

	ext_meet(s3, t3, s1, t1, s_par2, par2, &s4, &t4, &macro_ab, &macro_c);

	//export_copy_des(&s5, &t5, s4, t4);

	//reach(&s4, &t4);

	//filedes("TEST", s4, 0, t4);

	/*for(i = 0; i < s4; i ++){
		t4[i].marked = false;
	}

	newstate = s4;*/

	for(i = 1; i < s4; i +=2){
		state = macro_c[i]%(s3*2)/2;
		for(j = 0; j < par2[state].numelts; j ++){
			ee = (INT_T)par2[state].next[j];
			if(inordlist1(ee, t3[state].next, t3[state].numelts)){
				if(!inordlist1(ee, t4[i].next, t4[i].numelts)){
					addordlist1(ee, s4, &t4[i].next,t4[i].numelts, &ok);
					if(ok) t4[i].numelts ++;
				}
			}
		}
	}

	t4 = (state_node *)REALLOC(t4, (s4 + 1) * sizeof(state_node));
	if(t4 == NULL){
		mem_result = 1;
		goto FREE_MEM;
	}

	t4[s4].next = NULL; t4[s4].numelts = 0;
	t4[s4].vocal = 0; t4[s4].marked = true;
	s4 ++;

	filedes("TEST", s4, 0, t4);

	reach(&s4, &t4);

	/*for(i = 0; i < s4; i ++){
		for(j = 0;)
	}*/

	freedes(*s2, t2);
	*s2 = 0; *t2 = NULL;

	export_copy_des(s2, t2, s4, t4);
	
	//zprint_par(s_par2, par2);

FREE_MEM:
	free_part(s_par1, &par1);
	free_part(s_par2, &par2);
	freedes(s3, &t3);
	freedes(s4, &t4);
	free(macro_ab); 
	free(macro_c);
}

void dfs_reach(INT_S s1, state_node *t1, INT_S init, INT_S *s2, state_node **t2, INT_S *s_number, INT_S **number)
{
   INT_T ee;
   INT_S state, i,j;
   INT_S es;
   s_stack ss;
   INT_B flag,ok;
   INT_S traverse_order;
   INT_B *list;

   list = NULL;

   *s2 = s1;
   *t2 = newdes(*s2);
   if(*s2 != 0 && *t2 == NULL){
	   mem_result = 1;
	   return;
   }

   list = (INT_B*)CALLOC(s1, sizeof(INT_B));

   for(i = 0; i < s1; i ++){
	   (*t2)[i].reached = false;
	   list[i] = false;
   }

   stack_Init(&ss);

   state = init;
   traverse_order = init;
   t1[state].reached = true;  
   list[state] = true;
   (*number)[state] = traverse_order;
   stack_Push(&ss, state);

   while(state != -1){
	   flag = false;
	   for(i = 0; i < t1[state].numelts; i ++){
		   ee = t1[state].next[i].data1;
		   es = t1[state].next[i].data2;
		   if(!t1[es].reached){
			   t1[es].reached = true;
			   traverse_order ++;
			   (*number)[es] = traverse_order;
			   list[es] = true;
			   stack_Push(&ss, state);
			   addordlist1(ee,es,&(*t2)[state].next, (*t2)[state].numelts, &ok);
			   if(ok) (*t2)[state].numelts ++;
			   (*t2)[state].reached = (*t2)[es].reached = true;

			   state = es;
			   flag = true;
			   break;
		   }
	   }
	   if(!flag){
		   stack_Pop(&ss, &state,&ok);
		   if(ok) continue;
		   else break;
	   }

   }

   for(i = 0; i < s1; i ++){
	   if(list[i]){
		   for(j = 0; j < t1[i].numelts; j ++){
			   ee = t1[i].next[j].data1;
			   es = t1[i].next[j].data2;
			   if(list[es] && (!inordlist2(ee,es, (*t2)[i].next,(*t2)[i].numelts))){
				   if((*number)[i] > (*number)[es]){
					   addordlist1(ee,es, &(*t2)[i].next, (*t2)[i].numelts, &ok);
					   if(ok) (*t2)[i].numelts ++;
					   (*t2)[i].reached = (*t2)[es].reached = true;
				   }
			   }
		   }
	   }
   }

   stack_Done(&ss);
   free(list);
}

void dfs_visit(INT_S s1, state_node *t1, INT_S init, INT_S s_root_num, INT_S *root_num, INT_S s_number, INT_S *number, INT_B * in_component)
{
	INT_S i;
	s_stack sst;
	sp_stack spst;
	INT_S v;
	INT_S es;
	INT_B ok;

	stack_Init(&sst);
	v = init;

	sp_stack_Init(&spst);

START:
	root_num[v] = v;
	in_component[v] = false;
	stack_Push(&sst,v);
	t1[v].coreach = true;
	for(i = 0; i < t1[v].numelts; i ++){
		es = t1[v].next[i].data2;
		sp_stack_Push(&spst, v, i);
		if(!t1[es].coreach){
			v = es;
			goto START;
		}
BACK:
		sp_stack_Pop(&spst,&v,&i,&ok);
		if(!ok) return;
		es = t1[v].next[i].data2;
		if(!in_component[es]){
			root_num[v] = min(root_num[v],root_num[es]);
		}
	}
	if(root_num[v] == v){
		do{
			stack_Pop(&sst, &es, &ok);
			in_component[es] = true;
		}while(es != v);
	}
	goto BACK;

}

void melt_e_proc(INT_S s1, state_node *t1, INT_S *s2, state_node **t2, INT_T s_nullist,INT_T *nullist)
{
	INT_S s3, s4, s5;
	state_node *t3, *t4, *t5;
	INT_S s_par1, s_par2;
	part_node *par1, *par2;
	INT_S i,j, ii, jj;//, state;
	INT_T ee;
	INT_B ok;
	INT_S s_list, *list;
	INT_S s_number, *number, s_root_num, *root_num;
	INT_B *in_component;//, tq_flag;
//	t_queue tq;

	s3 = s4 = s5 = 0; 
	t3 = t4 = t5 = NULL;
	s_par1 = s_par2 = 0;
	par1 = par2 = NULL;
	s_list = 0; list = NULL;
	s_number = 0; number = NULL;
	in_component = NULL;

	s3 = s1;         
	t3 = newdes(s3);
	if ((s3 != 0) && (t3 == NULL))
	{		
		goto FREE_MEM;
	}      

	for (i=0; i < s1; i++) {
		//t3[i].marked = t1[i].marked;
		//t3[i].vocal  = t1[i].vocal;
		//t3[i].reached = t3[i].coreach = false;
		for (j=0; j < t1[i].numelts; j++) {
			ee = t1[i].next[j].data1;
			jj = t1[i].next[j].data2;          
			if (inlist(ee, nullist, s_nullist)){
				ee = EEE;  /* "e" = 1000 */
				addordlist1(ee, jj, &t3[i].next, t3[i].numelts, &ok);
				if (ok) t3[i].numelts++;
			}//else{
			//	t3[i].coreach = true;
			//}

		
		}    

		/* Add selfloop to marked states with label "m" = 1001 */
		//if (t1[i].marked)
		//{
		//	addordlist1(1001, i, &t3[i].next, t3[i].numelts, &ok);
		//	if (ok) t3[i].numelts++;
		//}   
	}    
	s5 = s1;         
	t5 = newdes(s5);

	for (i=0; i < s1; i++) {
		for (j=0; j < t1[i].numelts; j++) {
			ee = t1[i].next[j].data1;
			jj = t1[i].next[j].data2;          
			if (inlist(ee, nullist, s_nullist))
				ee = EEE;  /* "e" = 1000 */

			addordlist1(ee, jj, &t5[i].next, t5[i].numelts, &ok);
			if (ok) t5[i].numelts++;
		}    
		t5[i].marked = t1[i].marked;
		/* Add selfloop to marked states with label "m" = 1001 */
		//if (t1[i].marked)
		//{
		//	addordlist1(1001, i, &t5[i].next, t5[i].numelts, &ok);
		//	if (ok) t5[i].numelts++;
		//}   
	}  

/*	queue_init(&tq);

	enqueue(&tq, 0);

	s_list = s3;
	list = (INT_S*)CALLOC(s_list, sizeof(INT_S));
	for(i = 0; i < s_list; i ++)
		list[i] = -1;

	s_par1 = 1;
	par1 = (part_node *)CALLOC(s_par1, sizeof(part_node));
	par_index = 0;
	addstatelist(0, &par1[par_index].next, par1[par_index].numelts, &ok);
	par1[par_index].numelts ++;
	list[0] = 0;
	
	//t3[0].reached = true;
	while(!queue_empty(&tq)){
		//par_index ++;
		cur = dequeue(&tq);
		if(t3[cur].reached)
			continue;
		if(list[cur] == -1){
			par_index = s_par1;
			par1 = (part_node *)REALLOC(par1, (s_par1 + 1) * sizeof(part_node));
			if(par1 == NULL)
				goto FREE_MEM;
			par1[par_index].next = NULL; 
			par1[par_index].numelts = 0;
			s_par1 ++;

			addstatelist(cur, &par1[par_index].next, par1[par_index].numelts, &ok);
			par1[par_index].numelts ++;
			list[cur] = par_index;
		}
		par_index = list[cur];
		t3[cur].reached = true;

		for(j = 0; j < t3[cur].numelts; j ++){
			ee = t3[cur].next[j].data1;
			jj = t3[cur].next[j].data2;
			if(!t3[jj].reached)
				enqueue(&tq, jj);
			if((!t3[jj].coreach) && (ee == EEE)){
				addstatelist(jj, &par1[par_index].next, par1[par_index].numelts, &ok);
				if(ok) par1[par_index].numelts++;	
				list[jj] = par_index;
			}
		}
	}
	
	*s2 = s_par1;
	*t2 = newdes(*s2);
	for(i = 0; i < s3; i ++){
		for(j = 0; j < t3[i].numelts; j ++){
			cur = list[i];
			ee = t3[i].next[j].data1;
			jj = list[t3[i].next[j].data2];
			if(cur != jj){
				addordlist1(ee,jj,&(*t2)[cur].next, (*t2)[cur].numelts, &ok);
				if(ok) (*t2)[cur].numelts ++;
			}
		}
	}*/

	for(i = 0; i < s3; i ++){
		t3[i].reached = false;
	}

	s_root_num = s3;
	root_num = (INT_S*)CALLOC(s_root_num, sizeof(INT_S));
	for(i = 0; i < s_root_num; i ++)
		root_num[i] = -1;

	s_number = s3;
	number = (INT_S*)CALLOC(s_number, sizeof(INT_S));

	in_component = (INT_B*)CALLOC(s3, sizeof(INT_B));

	//filedes("TEST", s3, 0, t3);

	for(i = 0; i < s3; i ++){
		if(!t3[i].reached){
			for(j = 0; j < s3; j ++)
				number[j] = -1;
			dfs_reach(s3,t3, i, &s4, &t4, &s_number, &number);
		//	filedes("TEST1", s4, 0, t4);
		//	zprint_list(s_number, number);
			for(j = 0; j < s3; j ++){
				in_component[j] = false;
				t4[j].coreach = false;
			}
			dfs_visit(s4, t4, i, s_root_num, root_num, s_number, number, in_component);
			//zprint_list(s_root_num, root_num);
			freedes(s4, &t4);
			s4 = 0; t4 = NULL;
		}
	}

	//s_list = 0; list = NULL;
	//par2 = (part_node *)CALLOC(s_par2, sizeof(part_node));
	
/*	for(i = 0; i < s1; i ++)
		t1[i].reached = false;
	//state = 0;
	queue_init(&tq);
	//for(i = 0; i < s_root_num; i ++){
		//if(root_num[i] == i){
			//state ++;
			for(j = 0; j < s1; j ++){
				state = j;
				t1[state].reached = true;
				tq_flag = false;
				while(state != root_num[state]){
					tq_flag = true;
					enqueue(&tq, state);
					t1[state].reached = true;
					state = root_num[state];
					if(t1[state].reached){
						state = root_num[state];
						break;
					}
				}
				ii = state;
				if(tq_flag){
						while (!queue_empty(&tq)){
							state = dequeue(&tq);
							root_num[state] = ii;
						}
					
				}
				
			}
			//zprint_list(s_list, list);
			//free(list); s_list = 0; list = NULL;
		//}
	//}
	queue_done(&tq);*/

	//zprint_list(s_root_num, root_num);

	*s2 = s5;
	*t2 = newdes(*s2);

	for (i=0; i < s5; i++) {
		ii = root_num[i];
		if(t5[i].marked)
			(*t2)[ii].marked = true;
		for (j=0; j < t5[i].numelts; j++) {
			ee = t5[i].next[j].data1;
			jj = root_num[ t5[i].next[j].data2 ];          

			if ((ii == jj) && (ee == EEE)) { 

			} else {   
				addordlist1(ee, jj, &(*t2)[ii].next, (*t2)[ii].numelts, &ok);
				if (ok) (*t2)[ii].numelts++;
			}   
		}    
	}  

	trim1(s2, t2);

	//zprint_list(s_root_num, root_num);
	//export_copy_des(s2,t2, s4, t4);
	
	//zprint_list(s_rch_ord, rch_ord);


FREE_MEM:
	free_part(s_par1, &par1);
	free_part(s_par2, &par2);
	freedes(s3, &t3);
	freedes(s4, &t4);
	freedes(s5, &t5);
	free(list);
	free(root_num);
	free(number);
	free(in_component);
//	free(rch_ord);
}
/* Find state pair */
INT_B  instatepair1(INT_S e, INT_S *j, state_pair *L, INT_S size)
{
	INT_B  found;
	INT_S i;

	found = false;

	for(i = 0; i < size; i ++){
		if(L[i].data1 == e){
			*j = L[i].data2;
			found = true;
			break;
		}
	}	

	return found;
}

// map states to new states but keep the transition structure

INT_OS statemap_proc(char *name2, char* name1, INT_S s_sp, state_pair *sp)
{
	INT_S s1, s2, init;
	state_node *t1, *t2;
	INT_OS result;
	INT_S i, j, ss;
	INT_T ee;
	INT_S new_exit, new_entr;
	INT_B ok;

	s1 = s2 = 0;
	t1 = t2 = NULL;

	result = 0;

	init = 0L;
	if(getdes(name1, &s1, &init, &t1) == false){
		result = -1;
		goto FREE_MEM;
	}

	s2 = s1;
	t2 = newdes(s2);

	for(i = 0; i < s1; i ++){
		if(!instatepair1(i, &new_exit, sp, s_sp)){
			new_exit = i;
		}
		for(j = 0; j < t1[i].numelts; j ++){
			ee = t1[i].next[j].data1;
			ss = t1[i].next[j].data2;
			if(!instatepair1(ss, &new_entr, sp, s_sp)){
				new_entr = ss;
			}
			addordlist1(ee,new_entr, &t2[new_exit].next, t2[new_exit].numelts, &ok);
			if(ok) t2[new_exit].numelts ++;
			if(mem_result == 1){
				result = -1;
				goto FREE_MEM;
			}
		}
	}
	
	init = 0L;
	filedes(name2, s2, init, t2);

FREE_MEM:
	freedes(s1, &t1);
	freedes(s2, &t2);
	return result;
}
// Compute the supremal normal sublanugage by relabel function
// SupRR(M, E) = E - R_inv(R(M-E))

void inverse_relabel(INT_S s1, state_node *t1, INT_S *s2, state_node **t2, INT_S s_sp, state_pair *sp, INT_T s_list, INT_T *list)
{
	INT_S i, j, k;
	INT_S ee, ss;
	INT_B ok;

	*s2 = s1;
	*t2 = newdes(s1);
	
	for(i = 0; i < s1; i ++){
		for(j = 0; j < t1[i].numelts; j ++){
			ee = t1[i].next[j].data1;
			ss = t1[i].next[j].data2;
			for(k = 0; k < s_sp; k ++){
				if(sp[k].data2 == ee){
					addordlist1((INT_T)sp[k].data1, ss, &(*t2)[i].next, (*t2)[i].numelts, &ok);
					if(ok) (*t2)[i].numelts ++;
				}
			}
		}
		if(t1[i].marked)
			(*t2)[i].marked = true;
	}
	//selfloop_gentran(s_list, list, *s2, *t2);
}
INT_OS suprr_proc(char *name3, char* name1, char * name2, INT_S s_sp, state_pair *sp)
{
	INT_S s1, s2, s3, init;
	state_node *t1, *t2, *t3;
	INT_OS result;
	INT_B ok;

	INT_S _s1, _s2, _s3, _s4;
	state_node *_t1, *_t2, *_t3, *_t4;
	INT_T s_list1, *list1, s_list2, *list2, s_list3, *list3;

	INT_S *macro_ab, *macro_c; 

	s1 = s2 = s3 = 0;
	t1 = t2 = t3 = NULL;

	_s1 = _s2 = _s3 = _s4 = 0;
	_t1 = _t2 = _t3 = _t4 = NULL;

	s_list1 = s_list2 = s_list3 = 0; 
	list1 = list2 = list3 = NULL;
	macro_ab = macro_c = NULL;

	result = 0;

	init = 0L;
	if(getdes(name1, &s1, &init, &t1) == false){
		result = -1;
		goto FREE_MEM;
	}


	init = 0L;
	if(getdes(name2, &s3, &init, &t3) == false){
		result = -1;
		goto FREE_MEM;
	}	

	meet2(s1, t1, s3, t3, &s2, &t2, &macro_ab, &macro_c);
	free(macro_ab); free(macro_c); macro_ab = macro_c = NULL;
	freedes(s3, &t3); s3 = 0; t3 = NULL;

	allevent_des(&t1, &s1, &_t1, &_s1);  // store the all the events of M
	gentranlist(_s1, _t1, &s_list1, &list1);

	allevent_des(&t2, &s2, &_t2, &_s2);  // store the all the events of E
	gentranlist(_s2, _t2, &s_list2, &list2);

	// Step 1: Compute M - E = M /\ complement (E)
	export_copy_des(&_s3, &_t3, s2, t2);	
	complement1(&_s3, &_t3, s_list1, list1);
	reach(&_s3, &_t3);
	meet2(s1, t1, _s3, _t3, &_s4, &_t4, &macro_ab, &macro_c);
	freedes(_s3, &_t3); _s3 = 0; _t3 = NULL;
	free(macro_ab); free(macro_c); macro_ab = macro_c = NULL;

	//filedes("TEST4",_s4, 0, _t4);

	// Step 2: Compute R(M-E)
	eventmap_des(_t4, _s4, &_t3, &_s3, sp, s_sp, &list3,&s_list3,&ok);

	if (s_list2 != 0) 
		project0(&_s3, &_t3, s_list3, list3);
	freedes(_s4, &_t4); _s4 = 0; _t4 = NULL;

	//filedes("TEST0",_s3, 0, _t3);

	// Step 3: Compute R_inv(R(M-E))
	inverse_relabel(_s3, _t3, &_s4, &_t4, s_sp, sp, s_list1, list1);
	//sync2(_s3, _t3, _s2, _t2, &_s4, &_t4, &macro_ab, &macro_c);
	//free(macro_ab); free(macro_c); 	macro_ab = NULL; macro_c = NULL;   
	freedes(_s3, &_t3); _t3 = NULL; _s3 = 0;
	//filedes("TEST1",_s4, 0, _t4);

	// Step 4: Compute E - R_inv(R(M_E))
	complement1(&_s4, &_t4, s_list2, list2);
	//filedes("TEST2",_s4, 0, _t4);
	reach(&_s4, &_t4);
	//filedes("TEST3",_s4, 0, _t4);
	meet2(s2, t2, _s4, _t4, &s3, &t3, &macro_ab, &macro_c);

	// Clean up the obtained result
	trim1(&s3, &t3);

	init = 0L;
	filedes(name3, s3, init, t3);

FREE_MEM:
	freedes(s1, &t1);
	freedes(s2, &t2);
	freedes(s3, &t3);
	freedes(_s1, &_t1);
	freedes(_s2, &_t2);
	freedes(_s3, &_t3);
	freedes(_s4, &_t4);
	free(list1);
	free(list2);
	free(list3);
	free(macro_ab);
	free(macro_c);

	return result;
}

// relabel event labels to a list of new labels



INT_OS inv_relabel_proc(char *name2, char* name1, INT_S s_sm, state_map *sm)
{
	INT_S s1, s2, init;
	state_node *t1, *t2;
	INT_OS result;
	INT_S i, j, k, l;
	INT_S old_label, ee, ss;
	INT_B ok;

	s1 = s2 = 0;
	t1 = t2 = NULL;

	result = 0;

	init = 0L;
	if(getdes(name1, &s1, &init, &t1) == false){
		result = -1;
		goto FREE_MEM;
	}

	export_copy_des(&s2, &t2, s1, t1);

	for(i = 0; i < s_sm; i ++){
		old_label = sm[i].state;
		for(j = 0; j < s1; j ++){
			for(k = 0; k < t1[j].numelts; k ++){
				ee = t1[j].next[k].data1;
				if(ee == old_label){
					ss = t1[j].next[k].data2;
					delete_ordlist1((INT_T)ee, ss, &(t2[j].next), t2[j].numelts, &ok);
					if(ok) t2[j].numelts--;
					for(l = 0; l < sm[i].numelts; l ++){
						addordlist1((INT_T)sm[i].next[l], ss, &(t2[j].next), t2[j].numelts, &ok);
						if(ok) t2[j].numelts++;
					}
				}
			}
		}
		determinize(&t2, &s2);
		freedes(s1, &t1); s1 = 0; t1 = NULL;
		export_copy_des(&s1, &t1, s2, t2);		
	}

	init = 0L;
	filedes(name2, s2, init, t2);

FREE_MEM:
	freedes(s1, &t1);
	freedes(s2, &t2);
	return result;
}