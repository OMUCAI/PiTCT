#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "canqc.h"
#include "setup.h"
#include "des_data.h"
#include "des_proc.h"
#include "des_supp.h"
// #include "curses.h"
#include "mymalloc.h"
#include "tct_proc.h"
// #include "ext_des_proc.h"

INT_B  CANQC_DEBUG = false;
FILE *out_canqc_debug;

/* 
 * Print out the partition for debugging.
 */

// void print_partition(INT_S size, part_node *par)
// {
//    INT_S i,j;

//    fprintf(out_canqc_debug, "[");
     
//    for (i=0; i < size; i++)
//    {
//      fprintf(out_canqc_debug, "[");
//      for (j=0; j < par[i].numelts; j++)
//      {
//          fprintf(out_canqc_debug, "%d", par[i].next[j]);
//          if ((j+1) < par[i].numelts)
//              fprintf(out_canqc_debug, ",");
//       }    
//       fprintf(out_canqc_debug, "]"); 
//       if ((i+1) < size)
//          fprintf(out_canqc_debug, ",");
//    }  
//    fprintf(out_canqc_debug, "]");   
// }   

/* 
 * Print out the partition for debugging.
 */
// void print_par(INT_S size, part_node *par)
// {
//    INT_S i,j;

//    // clrscr();
//    printf("[");
     
//    for (i=0; i < size; i++)
//    {
//      printf("[");
//      for (j=0; j < par[i].numelts; j++)
//      {
//          printf("%d", par[i].next[j]);
//          if ((j+1) < par[i].numelts)
//              printf(",");
//       }    
//       printf("]"); 
//       if ((i+1) < size)
//          printf(",");
//    }  
//    printf("]");   
//    printf("\n");
// }   


/*
 *  Generate "Par1"
 */ 
void Generate_Par1(INT_S *size, part_node **par)
{
   *size = 6;
   
   *par = (part_node*) CALLOC(*size, sizeof(part_node));
   
   (*par)[0].numelts = 2;
   (*par)[0].next = (INT_S*) CALLOC(2, sizeof(INT_S));
   (*par)[0].next[0] = 9;
   (*par)[0].next[1] = 5;
   
   (*par)[1].numelts = 1;
   (*par)[1].next = (INT_S*) CALLOC(1, sizeof(INT_S));   
   (*par)[1].next[0] = 2;
   
   (*par)[2].numelts = 1;
   (*par)[2].next = (INT_S*) CALLOC(1, sizeof(INT_S));   
   (*par)[2].next[0] = 8;
   
   (*par)[3].numelts = 3;
   (*par)[3].next = (INT_S*) CALLOC(3, sizeof(INT_S));   
   (*par)[3].next[0] = 6;
   (*par)[3].next[1] = 0;
   (*par)[3].next[2] = 3;
   
   (*par)[4].numelts = 2;
   (*par)[4].next = (INT_S*) CALLOC(2, sizeof(INT_S));
   (*par)[4].next[0] = 7;
   (*par)[4].next[1] = 1;
   
   (*par)[5].numelts = 2;
   (*par)[5].next = (INT_S*) CALLOC(2, sizeof(INT_S));
   (*par)[5].next[0] = 4;
   (*par)[5].next[1] = 10;
}      

/*
 *  Generate "Par2"
 */ 
void Generate_Par2(INT_S *size, part_node **par)
{
   *size = 4;
   
   *par = (part_node*) CALLOC(*size, sizeof(part_node));
   
   (*par)[0].numelts = 2;
   (*par)[0].next = (INT_S*) CALLOC(2, sizeof(INT_S));
   (*par)[0].next[0] = 0;
   (*par)[0].next[1] = 2;
   
   (*par)[1].numelts = 4;
   (*par)[1].next = (INT_S*) CALLOC(4, sizeof(INT_S));   
   (*par)[1].next[0] = 1;
   (*par)[1].next[1] = 3;
   (*par)[1].next[2] = 6;
   (*par)[1].next[3] = 7;
   
   (*par)[2].numelts = 4;
   (*par)[2].next = (INT_S*) CALLOC(4, sizeof(INT_S));   
   (*par)[2].next[0] = 4;
   (*par)[2].next[1] = 5;
   (*par)[2].next[2] = 8;
   (*par)[2].next[3] = 9;
   
   (*par)[3].numelts = 1;
   (*par)[3].next = (INT_S*) CALLOC(1, sizeof(INT_S));   
   (*par)[3].next[0] = 10;
}      

/*
 * Par2 = Arrange (Par1)
 */

void Arrange(INT_S *s_par2, part_node **par2, INT_S *s_par1, part_node **par1, INT_S nStates)
{
   INT_S i,j,k,m;
   INT_B  ok;
   INT_B  *parbool;
    
   *s_par2 = *s_par1;  
   *par2 = (part_node*) CALLOC(*s_par2, sizeof(part_node));
   if (*par2 == NULL) {
     mem_result = 1;
     return;
   }
   
   parbool = (INT_B *) CALLOC(nStates, sizeof(INT_B ));
   if (parbool == NULL) {
     mem_result = 1;
     free(*par2); *par2= NULL;  
     *s_par2 = 0;
     return;
   }  

   m = 0;
   for (i=0; i < nStates; i++) {
     for (j=0; (j < *s_par1) && (parbool[i] == false); j++) {
        for (k=0; k < (*par1)[j].numelts; k++) {
           if ((*par1)[j].next[k] == i)
              parbool[i] = true;
        }   
        
        if (parbool[i] == true) {
           for (k=0; k < (*par1)[j].numelts; k++)
           {
              addstatelist( (*par1)[j].next[k], &(*par2)[m].next, (*par2)[m].numelts, &ok);
              if (ok) {
                 (*par2)[m].numelts++;
                 parbool[(*par1)[j].next[k]] = true;                  
              }   
           }   
           m++;
        }   
     }             
   }    
   
   free(parbool);
   
   /*
    * Debugging
    */ 
   // if (CANQC_DEBUG == true)
   // {
   //    fprintf(out_canqc_debug, "Par2 = Arrange(Par1)\n");
   //    fprintf(out_canqc_debug, "Par1 = ");
   //    print_partition(*s_par1, *par1);
   //    fprintf(out_canqc_debug, "\n");
   //    fprintf(out_canqc_debug, "Par2 = ");
   //    print_partition(*s_par2, *par2);
   //    fprintf(out_canqc_debug, "\n");
   //    fprintf(out_canqc_debug, "\n");
   // }   
}    

/*
 * List3 = Intersection(List1, List2)
 * Assume the lists are not ordered for now.
 */
void old_Intersection(INT_S *s_list3, INT_S **list3,
                  INT_S  s_list1, INT_S  *list1,
                  INT_S  s_list2, INT_S  *list2)
{                  
   INT_S i,j;
   
   for (i=0; i < s_list1; i++) {
      for (j=0; j < s_list2; j++) {
          if (list1[i] == list2[j])
          {
              *list3 = (INT_S*) REALLOC(*list3, ((*s_list3)+1)*sizeof(INT_S));
              if (*list3 == NULL)
              {
                 mem_result = 1;
                 return;
              }   
              (*list3)[*s_list3] = list1[i];
              (*s_list3)++;
          }                    
      }    
   }      
}                  

void Intersection(INT_S *s_list3, INT_S **list3,
                  INT_S  s_list1, INT_S  *list1,
                  INT_S  s_list2, INT_S  *list2)
{                  
   INT_S i;//,j;

   /* Optimize union based on size */

   if (s_list1 < s_list2) {
   
      for (i=0; i < s_list1; i++) {     
         if (instatelist(list1[i], list2, s_list2))
         {
            *list3 = (INT_S*) REALLOC(*list3, ((*s_list3)+1)*sizeof(INT_S));
            if (*list3 == NULL)
            {
               mem_result = 1;
               return;
            }   
            (*list3)[*s_list3] = list1[i];
            (*s_list3)++;
         }   
      }      
   
   } else {
          
      for (i=0; i < s_list2; i++) {     
         if (instatelist(list2[i], list1, s_list1))
         {
            *list3 = (INT_S*) REALLOC(*list3, ((*s_list3)+1)*sizeof(INT_S));
            if (*list3 == NULL)
            {
               mem_result = 1;
               return;
            }   
            (*list3)[*s_list3] = list2[i];
            (*s_list3)++;
         }   
      }         
          
   }       
}                  

/*
 * Par3 = ParMeet(Par1, Par2)
 */
void ParMeet(INT_S *s_par3, part_node **par3,
             INT_S  s_par1, part_node  *par1, 
             INT_S  s_par2, part_node  *par2)
{
   INT_S i,j;
   INT_S s_list3;
   INT_S *list3;
   
   for (i=0; i < s_par1; i++) { 
      for (j=0; j < s_par2; j++) {
          s_list3 = 0;  
          list3 = NULL; 
          
          Intersection(&s_list3, &list3, 
                       par1[i].numelts, par1[i].next,
                       par2[j].numelts, par2[j].next);
                       
          if (s_list3 != 0)             
          {
             *par3 = (part_node*) REALLOC(*par3, ((*s_par3)+1)*sizeof(part_node));
             (*par3)[*s_par3].numelts = s_list3;
             (*par3)[*s_par3].next = list3;
             (*s_par3)++;
          }
      }
   }      
   
   /*
    * Debugging
    */ 
   // if (CANQC_DEBUG == true)
   // {
   //    fprintf(out_canqc_debug, "Par3 = ParMeet(Par1, Par2)\n");
   //    fprintf(out_canqc_debug, "Par1 = ");
   //    print_partition(s_par1, par1);
   //    fprintf(out_canqc_debug, "\n");
   //    fprintf(out_canqc_debug, "Par2 = ");
   //    print_partition(s_par2, par2);
   //    fprintf(out_canqc_debug, "\n");
   //    fprintf(out_canqc_debug, "Par3 = ");
   //    print_partition(*s_par3, *par3);
   //    fprintf(out_canqc_debug, "\n");
   //    fprintf(out_canqc_debug, "\n");
   // }                    
}     

INT_S test_suite()
{
   INT_S i;       
   INT_S s1, s2, init;
   state_node *t1, *t2;  
   
   INT_S s_par1, s_par2, s_par2a, s_par3, s_par4;
   part_node *par1, *par2, *par2a, *par3, *par4;
   
   s1 = s2 = init = 0;
   t1 = t2 = NULL;
   
   s_par1 = s_par2 = s_par2a = s_par3 = s_par4 = 0;
   par1 = par2 = par2a = par3 = par4 = NULL;            
   
   if (CANQC_DEBUG == true)
      out_canqc_debug = fopen("qc_debug.txt", "w");
   
   Generate_Par1(&s_par1, &par1);
   Arrange(&s_par2, &par2, &s_par1, &par1, 11);   
   
   Generate_Par2(&s_par2a, &par2a);
   ParMeet(&s_par3, &par3, s_par1, par1, s_par2a, par2a);
   Arrange(&s_par4, &par4, &s_par3, &par3, 11); 

   if (CANQC_DEBUG == true)
      fclose(out_canqc_debug);      

   for (i=0; i < s_par1; i++)
     free(par1[i].next);
   free(par1);  
   for (i=0; i < s_par2; i++)
     free(par2[i].next);
   free(par2);  
   for (i=0; i < s_par2a; i++)
     free(par2a[i].next);
   free(par2a);  
   for (i=0; i < s_par3; i++)
     free(par3[i].next);
   free(par3);  
   for (i=0; i < s_par4; i++)
     free(par4[i].next);         
   free(par4);
   return 0;
}    

/*
 * addcolumn1
 *   Add state in partition list.
 * return:
 *   INT_B  -- true  : new entry added as not previously present
 *              false : already present        
 */
INT_B  old_addcolumn1(INT_S i, INT_S j, part_node *pn)
{
   INT_S pos;
   INT_S ii;
   INT_B  found = false;
   
   /* TODO -- faster if this is already sorted */
   
   /* Check if "j" exists already before appending it */
   for (ii=0; ii < pn[i].numelts; ii++)
   {
      if (pn[i].next[ii] == j)
          found = true;
   }   

   if (found == false) 
   {
      /* Append to the end */
      pos = pn[i].numelts;
      pn[i].numelts++;
      pn[i].next = (INT_S*) REALLOC(pn[i].next, sizeof(INT_S)*pn[i].numelts);
      pn[i].next[pos] = j;
   }   
   
   return !found;
}

INT_B  addcolumn1(INT_S i, INT_S j, part_node *pn)
{
   INT_B  ok = false;

   addstatelist(j, &(pn[i].next), pn[i].numelts, &ok);
   if (ok) pn[i].numelts++;
      
   return ok;   
}

void Parbinary(INT_S s_par, part_node *par, INT_S size) 
{
   INT_S i;//,k;
   INT_B  found;
   
   for (i=0; i < size; i++) {
/*      found = false; 
      for (k=0; k < par[0].numelts; k++) {
         if (par[0].next[k] == i)
            found = true;
      }    
*/
      found = instatelist(i, par[0].next, par[0].numelts); 
      
      if (found == false)
         addcolumn1(1, i, par);
   }       
}                              

INT_B  existing_state(INT_S state, t_stack *ts)
{
   INT_S i;
        
   for (i=0; i < ts->head_size; i++)
   {
       if (ts->head[i].data2 == state)
          return true;
   }    
   return false;
}

void eta3(INT_S state, INT_S *nStates, INT_S **stateList, INT_S s_temp, state_node *temp, INT_T x)
{
   INT_T j, ee;
   INT_S ss;
   INT_B  ok;

   /* Just mark states that are next to it */   
   for (j=0; j < temp[state].numelts; j++)
   {
       ee = temp[state].next[j].data1;
              
       if (ee == EEE) {   /* "e" = 1000 */
          ss = temp[state].next[j].data2;
          addstatelist(ss, stateList, *nStates, &ok);
          if (ok) (*nStates)++;
       }
   }    
} 

void eta121(INT_S *nStates, INT_S **stateList, INT_S s_temp, state_node *temp, INT_T x)
{
   INT_S i, nElems;  
     
   do {
      nElems = *nStates;         
      for (i=0; i < *nStates; i++) 
         eta3((*stateList)[i], nStates, stateList, s_temp, temp, x);       
   } while (nElems < *nStates);
}        

/*
 * Original 
 * Better for large states and small NULL list (i.e. large imagelist)
 * Worst for large NULL list (i.e. small imagelist)
 */
void eta11(INT_S* s2, state_node **t2, 
          INT_S state, INT_T x,
          INT_S s_temp, state_node *temp)
{
   INT_S i, ss;
   INT_T j, ee, dummy = 0;
   INT_B  ok, action;
   t_stack ts, ts2;   
   
   INT_S match      = 0;
   INT_S nElems     = 0;
   INT_S nStates    = 0;
   INT_S* stateList = NULL; 
   
   INT_OS count = 0; 
   
   pstack_Init(&ts);
   pstack_Init(&ts2);
   
   i = state;
   j = 0;
      
   do {
      if (j < temp[i].numelts) {
         ee = temp[i].next[j].data1;
         ss = temp[i].next[j].data2; 
         action = false;

         if (ee == x) {
            if (match == 0) {
                addstatelist(ss, &stateList, nStates, &ok);
                if (ok) nStates++;
                
                action = true;
                
                if (i != ss) match++;
            } else {
                pstack_Pop(&ts2, &dummy, &match, &ok);
                pstack_Pop(&ts, &j, &i, &ok);

                ee = temp[i].next[j].data1;                
                if (ee == x) match--;                  
                j++;
            }    
         } else if (ee == EEE) { /* "e" = 1000 */
            if (match == 0) {
                action = true;
            } else {  
                addstatelist(ss, &stateList, nStates, &ok);
                if (ok) nStates++;
                
                action = true; 
            }    
         }                       
         
         if (action) {
           if (ss == i) {  /* self-loop */
              ee = temp[i].next[j].data1;
              if (ee == x) match--;
              j++; 
           } else if (existing_state(ss, &ts)) {    /* Return to an existing state */
              ee = temp[i].next[j].data1;
              if (ee == x) match--;
              j++;
           } else {
              pstack_Push(&ts, j, i);
              pstack_Push(&ts2, dummy, match);
              i = ss; j = 0;
           }     
         }
            
      } else {
          if (!pstack_IsEmpty(&ts)) {
             pstack_Pop(&ts2, &dummy, &match, &ok);
             pstack_Pop(&ts, &j, &i, &ok);
             
             ee = temp[i].next[j].data1;
             if (ee == x) match--;
          }   
          j++;
      }             
      
      count++;
      
   } while ( (j < temp[i].numelts) || (!pstack_IsEmpty(&ts)) );
   
   pstack_Done(&ts);
   pstack_Done(&ts2);
   
   /* With this initial list of states, find the "x..e...e".
      Iterate until no new states can be found */    
   do 
   {
      nElems = nStates;         
      for (i=0; i < nStates; i++) 
         eta3(stateList[i], &nStates, &stateList, s_temp, temp, x);       
   } while (nElems < nStates);   
        
   for (i=0; i < nStates; i++) { 
      addordlist1(x, stateList[i], &(*t2)[state].next, (*t2)[state].numelts, &ok);
      if (ok) (*t2)[state].numelts++;    
   }    
   
   free(stateList);
}                    

/*
 * This the newer better one.
 * This version is bounded by O(n*m) n=state, m=transitions
 */

void eta12(INT_S* s2, state_node **t2, 
          INT_S state, INT_T x,
          INT_S s_temp, state_node *temp)
{
   INT_S i, ss, ii;
   INT_T j, ee, dummy = 0;
   INT_B  ok, action;
   t_stack ts, ts2;   
   
   INT_S match      = 0;
   INT_S nStates    = 0;
   INT_S* stateList = NULL; 
   INT_B  *rlist   = NULL;

   if (temp[state].numelts == 0)
      return;
   else
      rlist = (INT_B *) CALLOC(s_temp, sizeof(INT_B ));

   /* Find all e...x states */   
   pstack_Init(&ts);
   pstack_Init(&ts2);
   
   i = state;
   j = 0;
      
   do {
      if (j < temp[i].numelts) {
         ee = temp[i].next[j].data1;
         ss = temp[i].next[j].data2; 
         action = false;
         
         if (ee == x) {
            if (match == 0) {
               addstatelist(ss, &stateList, nStates, &ok);
               if (ok) nStates++;
               j++;
            } else {
               pstack_Pop(&ts2, &dummy, &match, &ok);
               pstack_Pop(&ts, &j, &i, &ok);

               ee = temp[i].next[j].data1;                
               if (ee == x) match--;                  
               j++;
            }    
         } else if (ee == EEE) { /* "e" = 1000 */
            if (match == 0) {
               action = true;
            } else {  
               addstatelist(ss, &stateList, nStates, &ok);
               if (ok) nStates++;
                
               action = true; 
            }    
         }  
                     
         if (action) {            
           if (ss == i) {  /* self-loop */
               ee = temp[i].next[j].data1;
               if (ee == x) match--;
               j++; 
           } else if (existing_state(ss, &ts)) {    /* Return to an existing state */
              ee = temp[i].next[j].data1;
              if (ee == x) match--;
              j++;
           } else {
              if (!rlist[ss]) {    
                 pstack_Push(&ts, j, i);
                 pstack_Push(&ts2, dummy, match);
                 i = ss; j = 0;
              }   
              else
              {
                 if (ee == x) match--;
                 j++;
              }   
           }     
         }
         
         rlist[ss] = true;
            
      } else {
          if (!pstack_IsEmpty(&ts)) {
             pstack_Pop(&ts2, &dummy, &match, &ok);
             pstack_Pop(&ts, &j, &i, &ok);
             
             ee = temp[i].next[j].data1;
             if (ee == x) match--;
          }   
          j++;
      }          
         
   } while ( (j < temp[i].numelts) || (!pstack_IsEmpty(&ts)) );
   
   pstack_Done(&ts);
   pstack_Done(&ts2);
   
   /* With this initial list of states, find the "x..e". */ 

   /* Clear the "reached" states info from rlist */
   memset(rlist, 0, sizeof(INT_B ) * s_temp);
   
   /* Mark with the states that are already identified */
   for (i=0; i < nStates; i++)
      rlist[stateList[i]] = true;
   
   for (ii=0; ii < nStates; ii++)
   {
      pstack_Init(&ts);
      
      i = stateList[ii];
      j = 0;
      
      do {
         if (j < temp[i].numelts) {
            ee = temp[i].next[j].data1;
            ss = temp[i].next[j].data2; 
            
            if (rlist[ss]) {
               j++;
            } else {
            
               if (ee == EEE) {
                  rlist[ss] = true;
                  
                  pstack_Push(&ts, j, i);
                  i = ss; j = 0;
               } else {
                  j++;   
               }
            }       
            
         } else {
            if (!pstack_IsEmpty(&ts)) {
               pstack_Pop(&ts, &j, &i, &ok);
            }   
            j++;    
         }  
                 
      } while ( (j < temp[i].numelts) || (!pstack_IsEmpty(&ts)) );
       
      pstack_Done(&ts);            
   }   

   for (i=0; i < s_temp; i++) { 
      if (rlist[i]) {       
         addordlist1(x, i, &(*t2)[state].next, (*t2)[state].numelts, &ok);
         if (ok) (*t2)[state].numelts++;    
      }   
   }    
   
   free(stateList);
   free(rlist);
}       

void eta2(INT_S* s2, state_node **t2, 
          INT_S state, 
          INT_S s_temp, state_node *temp)
{
   INT_S i;
   INT_B  ok;
   
   INT_S nElems     = 0;
   INT_S nStates    = 0;
   INT_S* stateList = NULL; 
   
   /* This routine is the same as a list of states reacheable from "state" */
   for (i=0; i < s_temp; i++) 
      temp[i].reached = false;
      
   temp[state].reached = true;
   b_reach(temp[state].next, state, &temp, s_temp);   
             
   for (i=0; i < s_temp; i++) { 
      if (temp[i].reached == true) 
      {
         addordlist1(EEE, i, &(*t2)[state].next, (*t2)[state].numelts, &ok);
         if (ok) (*t2)[state].numelts++;    
      }   
   }       
} 

void eta(INT_S  s1, state_node *t1, 
         INT_S* s2, state_node **t2,
         INT_T s_imagelist2, INT_T *imagelist2)
{
   INT_S i, ii, ss;  
   INT_T j, jj, ee;
   INT_S s_temp;
   state_node *temp;
   INT_B  ok;
   long num_hits = 0;
     
   *s2 = s1;   
   *t2 = newdes(*s2);
   if ((*s2 != 0) && (*t2 == NULL))
   {
      mem_result = 1;
      return;
   }

   s_temp = 0;
   temp = NULL;
      
   for (j=0; j < s_imagelist2; j++)
   {
      s_temp = s1;
      temp = newdes(s_temp); 
      if ((s_temp != 0) && (temp == NULL))
      {
         mem_result = 1;
         return;
      } 
   
      /* Construct a version of DES (s1, t1) only with transition "e" or "x" only */
      for (ii=0; ii < s1; ii++) {   
         for (jj=0; jj < t1[ii].numelts; jj++) {
            ee = t1[ii].next[jj].data1;

            if ((ee == imagelist2[j]) || (ee == EEE))    /* "e" = 1000 */
            {
               ss = t1[ii].next[jj].data2;
               addordlist1(ee, ss, &temp[ii].next, temp[ii].numelts, &ok);
               if (ok) temp[ii].numelts++;
            }   
         }   
      }            
       
      // if (debug_mode)
      // {
      //    move(22,0); clrtoeol();
      //    printw("ETA: %ld %d", s_imagelist2, s1);
      // }   
       
      for (i=0; i < s1; i++) {
          
         // if (debug_mode) {
         //    if ((num_hits % 10000L) == 0L) {
         //       move(23,0); clrtoeol();
         //       printw("ETA: %ld %d", j, i);
         //       refresh();
         //    }   
         //    num_hits++;
         // } 
                   
         if (imagelist2[j] == EEE)
            eta2(s2, t2, i, s_temp, temp);
         else   
            eta12(s2, t2, i, imagelist2[j], s_temp, temp);  
      }    
          
      freedes(s_temp, &temp);
   }       
   
   return;
   
/* clrscr();
   for (i=0; i < *s2; i++) {
       for (j=0; j < (*t2)[i].numelts; j++) {
           printf("%d %d %d\n", i, (*t2)[i].next[j].data1, (*t2)[i].next[j].data2);
       }
   }               
   clrscr(); */
}     

INT_S par_mapping(INT_S state, INT_S s_par, part_node *par)
{
   INT_S i;//, k;
   
   for (i=0; i < s_par; i++)
   {
/*      for (k=0; k < par[i].numelts; k++)    
      {
         if (par[i].next[k] == state)
            return i;
      }  */
      
      if (instatelist(state, par[i].next, par[i].numelts))
         return i; 
   }   
   
   return 0;  /* This is an error condition actually */
}      

void eta_to_table(INT_S s1, state_node *t1, 
                  INT_S* s2, state_node **t2,
                  INT_S s_par, part_node *par)
{
   INT_S i, jj, k, state;
   INT_T j, ee;
   INT_B  ok;
   
   INT_S s_remap;
   INT_S *remap;
                  
   *s2 = s1;
   *t2 = newdes(*s2);
   
   s_remap = s1;
   remap = (INT_S*) MALLOC(s_remap * sizeof(INT_S)); 
   if ((s_remap !=0) && (remap == NULL))
   {
      mem_result = -1;
      return;
   }   
         
   for (i=0; i < s_par; i++) {
      for (k=0; k < par[i].numelts; k++) {
          state = par[i].next[k];
          remap[state] = i;
      }   
   }    
   
   
   for (i=0; i < *s2; i++)
   {
      for (j=0; j < t1[i].numelts; j++)
      {
          ee = t1[i].next[j].data1;
          jj = t1[i].next[j].data2;
          
/*          jj = par_mapping(jj, s_par, par); */
          jj = remap[jj];
          
          addordlist1(ee, jj, &(*t2)[i].next, (*t2)[i].numelts, &ok);
          if (ok) (*t2)[i].numelts++;
      }    
   }   

   free(remap);
   
/* clrscr();
   for (i=0; i < *s2; i++) {
       for (j=0; j < (*t2)[i].numelts; j++) {
           printf("%d %d %d\n", i, (*t2)[i].next[j].data1, (*t2)[i].next[j].data2);
       }
   }               
   clrscr(); */
}                  

void table_to_par(INT_S s, state_node *t, INT_S *s_par, part_node **par)
{
   INT_S i, k;//, pos;
   INT_T j;
   INT_B  match;
   
   /* Clear out reached to indicate we have not process it already */
   for (i=0; i < s; i++)
     t[i].reached = false;

   *s_par = 0;
   *par = NULL;
     
   for (i=0; i < s; i++)
   {
      if (t[i].reached == false)
      {      
         *par = (part_node*) REALLOC(*par, ((*s_par)+1)*sizeof(part_node));
   
         (*par)[*s_par].numelts = 1;
         (*par)[*s_par].next = (INT_S*) CALLOC(1, sizeof(INT_S));
         (*par)[*s_par].next[0] = i; 
         (*s_par)++;           
         t[i].reached = true;
       
         for (k=i+1; k < s; k++) 
         {
            if ( (t[k].reached == false) && (t[i].numelts == t[k].numelts) )
            {
               /* Look for all matching transitions */
               match = true;
               for (j=0; (j < t[i].numelts) && (match == true); j++)
               {
                  if (! ((t[i].next[j].data1 == t[k].next[j].data1) &&
                         (t[i].next[j].data2 == t[k].next[j].data2)) )
                     match = false;
               }        
             
               if (match == true)
               {
                  t[k].reached = true;
                
                  /* Append to the end */
                  addcolumn1((*s_par)-1, k, *par);
               }  
            } /* if */
         } /* for k */          
      }  /*if */ 
   } /* for i */ 
}     

// void print_eta(INT_S s, state_node *t, INT_T s_imagelist, INT_T *imagelist)
// {
//    INT_S i,k;
//    INT_T j;
     
//    clrscr(); 
//    for (i=0; i < s; i++) {
//       printf("state %d:\n", i);
//       for (k=0; k < s_imagelist; k++) {    
//          printf("  transition   %d: ", imagelist[k]);
//          for (j=0; j < t[i].numelts; j++) {
//             if (t[i].next[j].data1 == imagelist[k])
//                printf("%d ", t[i].next[j].data2);
//          }      
//          printf("\n");
//      }    

//      printf("  transition 'm': ");
//      for (j=0; j < t[i].numelts; j++) {
//         if (t[i].next[j].data1 == 1001)    /* "m" = 1001 */
//             printf("%d ", t[i].next[j].data2);
//      }      
//      printf("\n");
     
//      printf("  transition 'e': ");
//      for (j=0; j < t[i].numelts; j++) {
//         if (t[i].next[j].data1 == EEE)    /* "e" = 1000 */
//             printf("%d ", t[i].next[j].data2);
//      }      
//      printf("\n");     
//    }   
// }

void ParGen(INT_T event, INT_S s2, state_node *t2, 
                         INT_S s_par, part_node* par)
{
   INT_S i,k;
   INT_T j;
   INT_B  found;
                            
   /* Transitions [q,xi,_] */  
   for (i=0; i < s2; i++) {
/*      for (j=0; j < t2[i].numelts; j++) {
         if (t2[i].next[j].data1 == event)
             addcolumn1(0, i, par);
      } */
      
      if (inordlist1(event, t2[i].next, t2[i].numelts))
         addcolumn1(0, i, par);
   }    
   
   /* Transitions [q,xi,q'] */
   found = true;
   while(found) {
     found = false;   
     for (i=0; i < s2; i++) {
        for (j=0; j < t2[i].numelts; j++) {
           if (t2[i].next[j].data1 == EEE) {    /* "e" = 1000 */
              for (k=0; k < par[0].numelts; k++) {
                 if (t2[i].next[j].data2 == par[0].next[k])
                 {
                    if (addcolumn1(0, i, par)) 
                       found = true;
                 }    
              }   
           }   
        }      
     }    
   }   
   Parbinary(s_par, par, s2);
}   

void free_par(INT_S *s_par, part_node** par)
{
   INT_S i;
     
   for (i=0; i < *s_par; i++)
      free((*par)[i].next);
   free(*par); 
   *par = NULL;
   *s_par = 0;   
}     
INT_B is_lcc(INT_S state, INT_T uncon_event, INT_S s1, state_node *t1, INT_S s2, state_node *t2)
{
	INT_S i, es, ss, cur;
	t_queue tq;

	/* Breadth first search */

	for(i = 0; i < s2; i ++)
		t2[i].reached = false;

	queue_init(&tq);

	enqueue(&tq, state);
	t2[state].reached = true;

	while (!queue_empty(&tq)) {
		ss = dequeue(&tq);
		if(inordlist1(uncon_event, t1[ss].next, t1[ss].numelts)){
			return true;
		}
		for (cur = 0; cur < t2[ss].numelts; cur++) {
			es = t2[ss].next[cur].data2;
			if (!t2[es].reached) {
				enqueue(&tq,es);
				t2[es].reached = true;
			}
		}
	}
	queue_done(&tq);

	return false;

}

void eta_lcc(INT_S s1, state_node *t1, INT_S s2, state_node *t2, INT_T s_imagelist, INT_T * imagelist)
{
	INT_S i, j, k, s3, s4, es;
	INT_T ee;
	state_node *t3, *t4;
	INT_B ok;

	s3 = s4 = 0; t3 = t4 = NULL;

	// Use (s4, t4) to store all the uncontrollable and unobservable paths
	s4 = s1;         
	t4 = newdes(s4);   
	for (i=0; i < s1; i++) {
		for (j=0; j < t1[i].numelts; j++) {
			ee = t1[i].next[j].data1;
			es = t1[i].next[j].data2;          
			if (!inlist(ee, imagelist, s_imagelist) && ee %2 == 0){
				addordlist1(EEE, es, &t4[i].next, t4[i].numelts, &ok);
				if (ok) t4[i].numelts++;
			}
		}     
	} 

	export_copy_des(&s3, &t3, s2, t2);

	for(i = 0; i < s3; i ++){
		for(j = 0; j < t3[i].numelts; j ++){
			ee = t3[i].next[j].data1;
			if(ee % 2 == 0){
				if(is_lcc(i, ee, s1, t1, s4, t4)){
					for(k = 0; k < t3[i].numelts; k ++){
						es = t3[i].next[k].data2;
						addordlist1(ee, es, &t2[i].next, t2[i].numelts, &ok);
						if(ok) t2[i].numelts ++;
					}
				}
			}
		}
	}
	freedes(s3, &t3);
	freedes(s4, &t4);
}
//Original version
INT_OS CanQC_proc1(char *name2, char *name1,
                INT_T s_nulllist, INT_T *nulllist,
                INT_T s_imagelist, INT_T *imagelist,
                INT_S *s_statemap, INT_S **statemap,
                INT_OS mode)
{
   INT_S s1, s2, s3, s4, s5, init;
   state_node *t1, *t2, *t3, *t4, *t5;    
   INT_B  ok;
   INT_S i, k, state, ii;
   INT_T j, jj, ee;
   INT_OS result = 0;   
   
   INT_S s_par1, s_par2, s_par3, s_par4;
   part_node *par1, *par2, *par3;
   
   INT_S s_remap;
   INT_S *remap;
   
   INT_T s_imagelist2, *imagelist2;
   
   s1 = s2 = s3 = s4 = s5 = init = 0;
   t1 = t2 = t3 = t4 = t5 = NULL;
   
   s_imagelist2 = 0;
   imagelist2 = NULL;
   
   s_par1 = s_par2 = s_par3 = s_par4 = 0;
   par1 = par2 = par3 = NULL;
   
   init = 0L;
   if (getdes(name1, &s1, &init, &t1) == false) {
      return -1;
   }  
   
   /* NULL */
   if (s1 == 0)
   {
      init = 0L;
      filedes(name2, s1, init, t1);
      goto CANLABEL1;
   }   
   
   /* Look for case everything is NULL */
   if (s_imagelist == 0) 
   {
      s2 = 1;
      t2 = newdes(s2);
      
      t2[0].marked = t1[0].marked;  
      
      init = 0L;
      filedes(name2, s2, init, t2);
      goto CANLABEL1;
   }  

   s2 = s1;         
   t2 = newdes(s2);
   if ((s2 != 0) && (t2 == NULL))
   {
      result = -1;
      goto CANLABEL1;
   }            

   /* Step 3 */          
   /* Create t2 with partition states and mark NULL event as "e" = 1000 */
   for (i=0; i < s1; i++) {
      t2[i].marked = t1[i].marked;
      t2[i].vocal  = t1[i].vocal;
      for (j=0; j < t1[i].numelts; j++) {
          ee = t1[i].next[j].data1;
          ii = t1[i].next[j].data2;          
          if (inlist(ee, nulllist, s_nulllist))
             ee = EEE;  /* "e" = 1000 */
             
          addordlist1(ee, ii, &t2[i].next, t2[i].numelts, &ok);
          if (ok) t2[i].numelts++;
      }    
      
      /* Add selfloop to marked states with label "m" = 1001 */
      if (t1[i].marked)
      {
         addordlist1(1001, i, &t2[i].next, t2[i].numelts, &ok);
         if (ok) t2[i].numelts++;
      }   
   }     
   
   /* Can delete t1 at this point */
   freedes(s1, &t1);
   s1 = 0;
   t1 = NULL;
         
   /* Step 4 */
   s_par2 = 2;
   par2 = (part_node*) CALLOC(s_par2, sizeof(part_node));
   if (par2 == NULL) {
      result = -1;
      goto CANLABEL1;
   }
   
   s_par3 = 2;
   par3 = (part_node*) CALLOC(s_par3, sizeof(part_node));
   if (par3 == NULL) {
      result = -1;
      goto CANLABEL1;
   }        
   
   /* Construct Alph' = Image U {m} */
   for (j=0; j < s_imagelist; j++)
   {
      addordlist(imagelist[j], &imagelist2, s_imagelist2, &ok);
      if (ok) s_imagelist2++;
   }    
   addordlist(1001, &imagelist2, s_imagelist2, &ok);   /* "m" = 1001 */
   if (ok) s_imagelist2++;
   
   ParGen(imagelist2[0], s2, t2, s_par3, par3);
   for (j=1; j < s_imagelist2; j++)
   {
      ParGen(imagelist2[j], s2, t2, s_par2, par2);
      
      s_par1 = s_par3;
      par1   = par3;          
      s_par3 = 0;
      par3   = NULL;
         
      ParMeet(&s_par3, &par3, s_par1, par1, s_par2, par2);
     
      /* Zero out content of par2 */
      for (i=0; i < s_par2; i++)
      {
        free(par2[i].next);
        par2[i].next = NULL;
        par2[i].numelts = 0;
      }   
   }  
   
   free_par(&s_par1, &par1);
   free_par(&s_par2, &par2);
   
   /* Put the answer back into par1 for now */   
   Arrange(&s_par1, &par1, &s_par3, &par3, s2);
   free_par(&s_par3, &par3);     

   /*** DEDUG ***/   
/* print_par(s_par1, par1); */
   /*** DEBUG ***/
   
   /* Step 5 -- it is integrated with step 4 */
   
   /* Step 6 */
   
   if (mode == 2){
     addordlist(EEE, &imagelist2, s_imagelist2, &ok);   /* "e" = 1000 */
     if(ok) s_imagelist2 ++;
   }
       
   eta(s2, t2, &s3, &t3, s_imagelist2, imagelist2);

   /*** DEBUG ***/
   /* print_eta(s3, t3, s_imagelist, imagelist); */
   /*** DEBUG ***/

   /* Step 7 */
   eta_to_table(s3, t3, &s4, &t4, s_par1, par1);

   /* Step 8 */
   table_to_par(s4, t4, &s_par2, &par2);
   
   freedes(s4, &t4);
   t4 = NULL;
   s4 = 0;
   
   /* Step 9 */
   ParMeet(&s_par3, &par3, s_par1, par1, s_par2, par2);   

   free_par(&s_par1, &par1);
   free_par(&s_par2, &par2);

   /* Step 10 */
   do {
      s_par4 = s_par3; 
      
      s_par1 = s_par3;
      par1   = par3;
      s_par3 = 0;
      par3   = NULL;

      eta_to_table(s3, t3, &s4, &t4, s_par1, par1);
      table_to_par(s4, t4, &s_par2, &par2);
      freedes(s4, &t4);
      t4 = NULL;
      s4 = 0;
      
      ParMeet(&s_par3, &par3, s_par1, par1, s_par2, par2);
      
      /* Free up space used by s_par1, s_par2 */
      free_par(&s_par1, &par1);
      free_par(&s_par2, &par2);
   } while (s_par4 != s_par3);

   /* Step 11 */
   s5 = s_par3;
   t5 = newdes(s5);   
   if ((s5 != 0) && (t5 == NULL))
   {
      result = -1;
      goto CANLABEL1;
   }    

   /* Create statemap and remap list from (par3, s_par3) */  
   *s_statemap = s2;
   *statemap = (INT_S*) MALLOC((*s_statemap) * sizeof(INT_S));
   if ((*s_statemap != 0) && (*statemap == NULL))
   {
      result = -1;
      goto CANLABEL1;
   }      
   
   s_remap = s2;
   remap = (INT_S*) MALLOC(s_remap * sizeof(INT_S)); 
   if ((s_remap !=0) && (remap == NULL))
   {
      result = -1;
      goto CANLABEL1;
   }   
         
   for (i=0; i < s_par3; i++) {
      for (k=0; k < par3[i].numelts; k++) {
          state = par3[i].next[k];
          (*statemap)[state] = par3[i].next[0]; 
          remap[state] = i;
      }   
   }    
     
   for (i=0; i < s2; i++) {
      ii = remap[i];
      for (j=0; j < t2[i].numelts; j++) {
          ee = t2[i].next[j].data1;
          jj = (INT_T)remap[ t2[i].next[j].data2 ];          
             
          if ((ii == jj) && (ee == 1001)) {  /* "m" = 1001 */
             t5[ii].marked = true;
          } else if ((ii == jj) && (ee == EEE)) { /* "e" = 1000 */
             /* Remove selfloop for non-observable events only */
          } else {   
             addordlist1(ee, jj, &t5[ii].next, t5[ii].numelts, &ok);
             if (ok) t5[ii].numelts++;
          }   
      }    
   }     
                  
   /* Step 12 */
   init = 0L;
   filedes(name2, s5, init, t5);         
   
CANLABEL1:   
   freedes(s1, &t1);
   freedes(s2, &t2); 
   freedes(s3, &t3);
   freedes(s4, &t4);
   freedes(s5, &t5);

   free_par(&s_par1, &par1);  
   free_par(&s_par2, &par2);
   free_par(&s_par3, &par3); 

   free(remap);
   free(imagelist2);
   
   return result;
}


// Used for Ext_Observer
INT_OS CanQC_proc3(char *name2, char *name1,
                INT_T s_nulllist, INT_T *nulllist,
                INT_T s_imagelist, INT_T *imagelist,
                INT_S *s_statemap, INT_S **statemap,
                INT_S *s_par, part_node **par,
                INT_OS mode)
{
   INT_S s1, s2, s3, s4, s5, init;
   state_node *t1, *t2, *t3, *t4, *t5;    
   INT_B  ok;
   INT_S i, k, state, ii;
   INT_T j, jj, ee;
   INT_OS result = 0;   
   
   INT_S s_par1, s_par2, s_par3, s_par4;
   part_node *par1, *par2, *par3;
   
   INT_S s_remap;
   INT_S *remap;
   
   INT_T s_imagelist2, *imagelist2;
   
   s1 = s2 = s3 = s4 = s5 = init = 0;
   t1 = t2 = t3 = t4 = t5 = NULL;
   
   s_imagelist2 = 0;
   imagelist2 = NULL;
   
   s_par1 = s_par2 = s_par3 = s_par4 = *s_par = 0;
   par1 = par2 = par3 = *par = NULL;

   s_remap = 0; remap = NULL;
   
   init = 0L;
   if (getdes(name1, &s1, &init, &t1) == false) {
      return -1;
   }  
   
   /* NULL */
   if (s1 == 0)
   {
      init = 0L;
      filedes(name2, s1, init, t1);
      goto CANLABEL1;
   }   
   
   /* Look for case everything is NULL */
   if (s_imagelist == 0) 
   {
      s2 = 1;
      t2 = newdes(s2);
      
      t2[0].marked = t1[0].marked;  
      
      init = 0L;
      filedes(name2, s2, init, t2);
      goto CANLABEL1;
   }  

   s2 = s1;         
   t2 = newdes(s2);
   if ((s2 != 0) && (t2 == NULL))
   {
      result = -1;
      goto CANLABEL1;
   }            

   /* Step 3 */          
   /* Create t2 with partition states and mark NULL event as "e" = 1000 */
   for (i=0; i < s1; i++) {
      t2[i].marked = t1[i].marked;
      t2[i].vocal  = t1[i].vocal;
      for (j=0; j < t1[i].numelts; j++) {
          ee = t1[i].next[j].data1;
          ii = t1[i].next[j].data2;          
          if (inlist(ee, nulllist, s_nulllist))
             ee = EEE;  /* "e" = 1000 */
             
          addordlist1(ee, ii, &t2[i].next, t2[i].numelts, &ok);
          if (ok) t2[i].numelts++;
      }    
      
      /* Add selfloop to marked states with label "m" = 1001 */
      if (t1[i].marked)
      {
         addordlist1(1001, i, &t2[i].next, t2[i].numelts, &ok);
         if (ok) t2[i].numelts++;
      }   
   }     
   
   /* Can delete t1 at this point */
  // freedes(s1, &t1);
  // s1 = 0;
  // t1 = NULL;
         
   /* Step 4 */
   s_par2 = 2;
   par2 = (part_node*) CALLOC(s_par2, sizeof(part_node));
   if (par2 == NULL) {
      result = -1;
      goto CANLABEL1;
   }
   
   s_par3 = 2;
   par3 = (part_node*) CALLOC(s_par3, sizeof(part_node));
   if (par3 == NULL) {
      result = -1;
      goto CANLABEL1;
   }        
   
   /* Construct Alph' = Image U {m} */
   for (j=0; j < s_imagelist; j++)
   {
      addordlist(imagelist[j], &imagelist2, s_imagelist2, &ok);
      if (ok) s_imagelist2++;
   }    
   addordlist(1001, &imagelist2, s_imagelist2, &ok);   /* "m" = 1001 */
   if (ok) s_imagelist2++;
   
   ParGen(imagelist2[0], s2, t2, s_par3, par3);
   for (j=1; j < s_imagelist2; j++)
   {
      ParGen(imagelist2[j], s2, t2, s_par2, par2);
      
      s_par1 = s_par3;
      par1   = par3;          
      s_par3 = 0;
      par3   = NULL;
         
      ParMeet(&s_par3, &par3, s_par1, par1, s_par2, par2);
     
      /* Zero out content of par2 */
      for (i=0; i < s_par2; i++)
      {
        free(par2[i].next);
        par2[i].next = NULL;
        par2[i].numelts = 0;
      }   
   }  
   
   free_par(&s_par1, &par1);
   free_par(&s_par2, &par2);
   
   /* Put the answer back into par1 for now */   
   Arrange(&s_par1, &par1, &s_par3, &par3, s2);
   free_par(&s_par3, &par3);     

   /*** DEDUG ***/   
/* print_par(s_par1, par1); */
   /*** DEBUG ***/
   
   /* Step 5 -- it is integrated with step 4 */
   
   /* Step 6 */
   
  // if (mode == 2){
  //   addordlist(EEE, &imagelist2, s_imagelist2, &ok);   /* "e" = 1000 */
  //   if(ok) s_imagelist2 ++;
  // }
       
   eta(s2, t2, &s3, &t3, s_imagelist2, imagelist2);
   if(mode == 2)
      eta_lcc(s1, t1, s3, t3, s_imagelist, imagelist);

   /*** DEBUG ***/
   /* print_eta(s3, t3, s_imagelist, imagelist); */
   /*** DEBUG ***/

   /* Step 7 */
   eta_to_table(s3, t3, &s4, &t4, s_par1, par1);

   /* Step 8 */
   table_to_par(s4, t4, &s_par2, &par2);
   
   freedes(s4, &t4);
   t4 = NULL;
   s4 = 0;
   
   /* Step 9 */
   ParMeet(&s_par3, &par3, s_par1, par1, s_par2, par2);   

   free_par(&s_par1, &par1);
   free_par(&s_par2, &par2);

   /* Step 10 */
   do {
      s_par4 = s_par3; 
      
      s_par1 = s_par3;
      par1   = par3;
      s_par3 = 0;
      par3   = NULL;

      eta_to_table(s3, t3, &s4, &t4, s_par1, par1);
      table_to_par(s4, t4, &s_par2, &par2);
      freedes(s4, &t4);
      t4 = NULL;
      s4 = 0;
      
      ParMeet(&s_par3, &par3, s_par1, par1, s_par2, par2);
      
      /* Free up space used by s_par1, s_par2 */
      free_par(&s_par1, &par1);
      free_par(&s_par2, &par2);
   } while (s_par4 != s_par3);
   
   /*Store Par3 into Par*/
   *s_par = s_par3;
   *par = (part_node*)CALLOC(*s_par, sizeof(part_node));
   if(*par == NULL){
      *s_par = 0;
      mem_result = 1;
      result = -1;
      goto CANLABEL1;
   }
   for(i = 0; i < s_par3; i ++){
      (*par)[i].next = (INT_S*)REALLOC((*par)[i].next , par3[i].numelts * sizeof(INT_S));
      if((*par)[i].next == NULL){
         (*par)[i].numelts = 0;
         mem_result = 1;
         result = -1;
         goto CANLABEL1;
      }
      (*par)[i].numelts = par3[i].numelts;
      memcpy((*par)[i].next, par3[i].next, par3[i].numelts * sizeof(INT_S));
   }

   /* Step 11 */
   s5 = s_par3;
   t5 = newdes(s5);   
   if ((s5 != 0) && (t5 == NULL))
   {
      result = -1;
      goto CANLABEL1;
   }    

   /* Create statemap and remap list from (par3, s_par3) */  
   *s_statemap = s2;
   *statemap = (INT_S*) MALLOC((*s_statemap) * sizeof(INT_S));
   if ((*s_statemap != 0) && (*statemap == NULL))
   {
      result = -1;
      goto CANLABEL1;
   }      
   
   s_remap = s2;
   remap = (INT_S*) MALLOC(s_remap * sizeof(INT_S)); 
   if ((s_remap !=0) && (remap == NULL))
   {
      result = -1;
      goto CANLABEL1;
   }   
         
   for (i=0; i < s_par3; i++) {
      for (k=0; k < par3[i].numelts; k++) {
          state = par3[i].next[k];
          (*statemap)[state] = par3[i].next[0]; 
          remap[state] = i;
      }   
   }    

   for (i=0; i < s2; i++) {
      ii = remap[i];
      for (j=0; j < t2[i].numelts; j++) {
          ee = t2[i].next[j].data1;
          jj = (INT_T)remap[ t2[i].next[j].data2 ];          
 
          if ((ii == jj) && (ee == 1001)) {  /* "m" = 1001 */
             t5[ii].marked = true;
          } else if ((ii == jj) && (ee == EEE)) { /* "e" = 1000 */
             /* Remove selfloop for non-observable events only */
          } else {   
             addordlist1(ee, jj, &t5[ii].next, t5[ii].numelts, &ok);
             if (ok) t5[ii].numelts++;
          }   
      }    
   }     
                  
   /* Step 12 */
   init = 0L;
   filedes(name2, s5, init, t5);         
   
CANLABEL1:   
   freedes(s1, &t1);
   freedes(s2, &t2); 
   freedes(s3, &t3);
   freedes(s4, &t4);
   freedes(s5, &t5);

   free_par(&s_par1, &par1);  
   free_par(&s_par2, &par2);
   free_par(&s_par3, &par3); 

   free(remap);
   free(imagelist2);
   
   return result;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//Natural observer extension algorithm
//////////////////////////////////////////////////////////////////////////////////////////////

void list_copy(INT_T *slist1, INT_T **list1, INT_T slist2, INT_T *list2)
{
   INT_T i;
   
   *slist1 = slist2;
   *list1 = (INT_T*)REALLOC(*list1, (*slist1) * sizeof(INT_T));
   if(*list1 == NULL){
      mem_result = 1;
      return;
   }
   for(i = 0; i < slist2; i ++)
      (*list1)[i] = list2[i];
}
INT_B  ext_is_deterministic(state_node *t, INT_S s)
{
   INT_S i;
   INT_T j, ee;
   
   for (i=0; i < s; i++) {       
       if (t[i].numelts > 0) {
          ee = t[i].next[0].data1;
          
          if (ee == EEE) 
            return false;
       }
       
       for (j=1; j < t[i].numelts; j++) {
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
void inv_project(INT_S ss, INT_T ee, INT_S es, 
                  state_node *t1, part_node *par1, 
                  INT_T s_imagelist, INT_T *imagelist,
                  INT_S *s_trip1, triple ** trip1)
{
   INT_S i, j, ii;
   INT_T event; INT_S state;
   INT_B  ok;
   if(ee == EEE && ss != es){
      for(i = 0; i < par1[ss].numelts; i ++){
         ii = par1[ss].next[i];
         for(j = 0; j < t1[ii].numelts; j ++){
            event = t1[ii].next[j].data1;
            state = t1[ii].next[j].data2;
            if(!inlist(event, imagelist,s_imagelist) && 
                 instatelist(state,par1[es].next,par1[es].numelts)){
               addtriple(ii, event, state , trip1, *s_trip1, &ok);
               if(ok) (*s_trip1) ++;
            }
         }
      }
   } else if(ee != EEE){
      for(i = 0; i < par1[ss].numelts; i ++){        
         ii = par1[ss].next[i];         
         for(j = 0; j < t1[ii].numelts; j ++){
            event = t1[ii].next[j].data1;
            state = t1[ii].next[j].data2;
            if((event == ee) && 
                 instatelist(state,par1[es].next,par1[es].numelts)){
               addtriple(ii, event, state , trip1, *s_trip1, &ok);
               if(ok) (*s_trip1) ++;
            }
         }
      }   
   }
}

void event_unionsets(INT_T *list1,
               INT_T size1,
               INT_T **list2,
               INT_T *size2)
{
   /* Form the union: list2 <- list1 + list2 */
   INT_T cur;
   INT_B  ok;

   for (cur=0; cur < size1; cur++) {
      addordlist(list1[cur],list2,*size2,&ok);
      if (ok) (*size2)++;
   }
}
// list3 = list2 - list1
void complement_sets(INT_T *list1, INT_T size1,
                     INT_T *list2, INT_T size2,
                     INT_T **list3, INT_T *size3)
{
   INT_T i;
   INT_B  ok;
   
   for(i = 0; i < size2; i ++){
      if(!inlist(list2[i],list1,size1)){
         addordlist(list2[i],list3,*size3,&ok);
         if(ok) (*size3) ++;
      }
   }
}

void delete_ordlist(INT_T e, INT_T **L, INT_T size, INT_B  *ok)
{
   INT_T pos;
   INT_T lower, upper;
   INT_B  found;
   
   *ok = false;
   // Do a binary search
   found = false;
   pos = 0;
   if(size > 1){
      lower = 1;
      upper = size;
      while(found == false && lower <= upper){
         pos = (lower + upper)/2;
         if(e == (*L)[pos - 1]){
            found = true;
         } else if(e > (*L)[pos - 1]){
            lower = pos + 1;
         } else if(e < (*L)[pos - 1]){
            upper = pos - 1;
         }
      }
   }else if(size == 1){
      if(e == (*L)[0])
         found = true;
   }
   if(found == false)
      return;
   if((size - pos) > 0){
      memmove(&((*L)[pos - 1]), &((*L)[pos]), (size - pos) * sizeof(INT_T));
   }
   *L = (INT_T *)REALLOC(*L, sizeof(INT_T) * (size - 1));
   if(size > 1){
      if(*L == NULL){
         mem_result = 1;
         return;
      }
   } else
     *L = NULL;
   
   //printw("%d %d %d",e, pos, size);
   
   *ok = true;
}

INT_B  split_reach(INT_S s, state_node **t, 
                    INT_S ss, 
                    INT_T s_colist, INT_T *colist, 
                    INT_S slist, INT_S *list)
{
   INT_T cur, ee;
   INT_S es,se;
   INT_S i;
   t_queue tq;
   
   queue_init(&tq);
   
   enqueue(&tq, ss);   
   
   for(i = 0; i < s; i ++)
      (*t)[i].reached = false;
   (*t)[ss].reached = true;
   
   while(!queue_empty(&tq)){
      se = dequeue(&tq);
      for(cur = 0; cur < (*t)[se].numelts; cur ++){
         ee = (*t)[se].next[cur].data1;
         es = (*t)[se].next[cur].data2;
         if(inlist(ee,colist,s_colist)){
            if(instatelist(es,list,slist)){
               while(!queue_empty(&tq))
                  dequeue(&tq);
               queue_done(&tq);
               return false;
            }
            if(!(*t)[es].reached){
               enqueue(&tq,es);
               (*t)[es].reached = true;
            }
         }
      }
   }
   queue_done(&tq);
   return true;
}
INT_B  split(INT_S y, 
              INT_T slist, INT_T *list,
              INT_T s_imagelist, INT_T *imagelist,
              INT_S s1, state_node *t1,
              INT_S s2, state_node *t2,
              part_node *par)
{
   INT_B  result;
   INT_S i,j, k, m;
   INT_T ee;
   INT_S ss;
   INT_S s_l1, *l1;
   INT_B  ok;
   triple * trip1; INT_S s_trip1;
   part_node * par1; INT_S s_par1;
   INT_T s_colist, *colist;
   
   s_l1 = 0; l1 = NULL;
   s_trip1 = 0; trip1 = NULL;
   s_par1 = 0; par1 = NULL;
   s_colist = 0; colist = NULL;
   
   result = true;
   //printw("%d ", imagelist[0]);
   //zprint_list1(slist,list);
   gen_complement_list(t1,s1, list, slist, &colist, &s_colist);
   if(mem_result == 1){
      return false;
   }
   //printw("test");
   for(i = 0; i< s_imagelist; i++){
      //printw("%d ", i);
      ee = imagelist[i];
      free(l1); s_l1 = 0; l1 = NULL;
      for(j = 0; j < t2[y].numelts; j ++){
         if(ee == t2[y].next[j].data1){
            addstatelist(t2[y].next[j].data2, &l1, s_l1, &ok);
            if(ok) s_l1 ++;
         }
      }
      
      if(s_l1 <= 1)
         continue;
      s_par1 = s_l1;
      par1 = (part_node*)CALLOC(s_par1 , sizeof(part_node));
      if(par1 == NULL){
         mem_result = 1;
         result = false;
         goto SPLIT_FREE;         
      }      
      for(j = 0; j < s_l1; j ++){
         //printw("%d %d %d\n",y, ee, l1[j]);
         //zprint_list(par[y].numelts, par[y].next);
         //zprint_list(par[l1[j]].numelts, par[l1[j]].next);
         inv_project(y,ee,l1[j],t1,par,s_imagelist, imagelist, &s_trip1, &trip1);
         //printw("%d ",j);
         for(k = 0; k < s_trip1; k ++){
            addstatelist(trip1[k].i, &par1[j].next, par1[j].numelts, &ok);
            if(ok) par1[j].numelts ++;
         }
         free(trip1);
         s_trip1 = 0; trip1 = NULL;
      }
      //system("pause");
      //zprints("trip:");
      //zprint_triple(s_trip1, trip1);
      //zprints("split: \n");
      for(j = 0; j < s_par1; j ++){
         //zprint_list(par1[j].numelts, par1[j].next);
         for(k = 0; k < par1[j].numelts; k ++){
            ss = par1[j].next[k];
            for(m = 0; m < s_par1; m ++){
               if(j == m)
                  continue;
               if(!split_reach(s1,&t1, ss, s_colist, colist, par1[m].numelts, par1[m].next)){
                  result = false;
                  goto SPLIT_FREE;
               }
            }
         }
      }
      free_par(&s_par1, &par1);
      s_par1 = 0; par1 = NULL;
      
   }
   
   //printw("test");
SPLIT_FREE:
   free(l1);
   free(trip1);
   free_par(&s_par1, &par1);
   return result; 
}

/*Event extension algorithm*/
INT_OS event_extend(char *name1, char *name2, 
                  INT_T s_imagelist, INT_T *imagelist,
                  INT_S s_par1, part_node *par1,
                  INT_T *s_ext_imagelist, INT_T **ext_imagelist)
{
   INT_S s1, s2;  
   state_node *t1, *t2;
   INT_S init, i, j, ii, jj, k;

   INT_S es; INT_T ee;
   INT_S s_trip1;
   triple * trip1; 
   INT_T slist1, *list1, slist2, *list2, slist3, *list3;
   INT_S s_nlist, *nlist;
//   INT_S max;
   INT_OS result;
   INT_B  ok;//, prev;
   
   INT_B  split_flag;
   
   s1 = s2 = 0;
   t1 = t2 = NULL;
   s_trip1 = 0; trip1 = NULL;
   slist1 = slist2 = slist3 = 0; 
   list1 = list2 = list3 = NULL;
   s_nlist = 0; nlist = NULL;
   
   result = 0;
   
   init = 0L;
   if(getdes(name1,&s1,&init,&t1) == false){
      result = -1;
      goto EXTEND_LABEL;
   }
   
   init = 0L;
   if(getdes(name2,&s2,&init, &t2) == false){
      result = -1;
      goto EXTEND_LABEL;
   }
   //printw("Start\n");
/*   s_par1 = s2;
   par1 = (part_node*)CALLOC(s_par1, sizeof(part_node));
   if(par1 == NULL){
      result = -1;
      mem_result = 1;
      goto EXTEND_LABEL;
   }
   jj = -1;
   max = -1;
   //zprint_list(s_statemap,statemap);
   for( i = 0; i < s_statemap; i ++){
      ii = statemap[i];
      if(ii > max){
         max = ii;
         jj ++;         
         addstatelist(i, &par1[jj].next, par1[jj].numelts, &ok);
         if(ok) par1[jj].numelts ++;
      } else
         continue;

      for(j = i + 1; j < s_statemap; j ++){
         if(statemap[j] == ii){
            addstatelist(j, &par1[jj].next, par1[jj].numelts, &ok);
            if(ok) par1[jj].numelts ++;
         }
      }
      
   }*/
   //printw("computing B\n");
   //Step 1 Compute B = union(_,_), the enlargement from the silent transitions in name2
   for( i = 0; i < s2; i ++){
      for(j = 0; j < t2[i].numelts; j ++){
         ee = t2[i].next[j].data1;
         if(ee == EEE ){
            es = t2[i].next[j].data2;
            inv_project(i, ee, es, t1, par1, s_imagelist, imagelist, &s_trip1, &trip1);
            if(mem_result == 1){
               result = -1;
               goto EXTEND_LABEL;
            }
         }
      }
   }
   for(i = 0; i < s_trip1; i++){
      addordlist(trip1[i].e, &list1, slist1, &ok);
      if(ok) slist1 ++;
   }
   free(trip1);
   s_trip1 = 0; trip1 = NULL;
   // B = list1
   event_unionsets(imagelist,s_imagelist,&list1,&slist1);

   if(mem_result == 1){
      result = -1;
      goto EXTEND_LABEL;
   }
   //printw("computing N\n");
   //Step 2: Compute N, the set of states where some observable event leads to more than one state
   // N = nlist
   for(i = 0; i < s2; i ++){
      if(t2[i].numelts > 0)
         ee = t2[i].next[0].data1;
      for(j = 1; j < t2[i].numelts; j ++){
         if(t2[i].next[j].data1 == EEE)
            continue;
         else{
            if(t2[i].next[j].data1 == ee){
               addstatelist(i, &nlist, s_nlist, &ok);
               if(ok) s_nlist ++;
               break;
            } else{
               ee = t2[i].next[j].data1;
            }
         }
      }
   }
   // Step 3: Check whether N is empty
   if(s_nlist == 0){
      list_copy(s_ext_imagelist,ext_imagelist,slist1,list1);
      if(mem_result == 1)
         result = -1;
      goto EXTEND_LABEL;
   }
   //printw("Computing H\n");
   // Step 4 and 5: compute H, the events hidden in the cosets 
   //corresponding to the states in Set N
   //H = list2
   for(i = 0; i < s_nlist; i ++){
      ii = nlist[i];
      for(j = 0; j < par1[ii].numelts; j ++){
         jj = par1[ii].next[j];
         for(k = 0; k < t1[jj].numelts; k ++){
            ee = t1[jj].next[k].data1;
            if(!inlist(ee,imagelist,s_imagelist) && (ee != EEE) &&
                 instatelist(t1[jj].next[k].data2, par1[ii].next, par1[ii].numelts))
            {
               addordlist(ee, &list2, slist2, &ok);
               if(ok) slist2 ++;
            }
         }
      }
   }
   //Step 6 and 7: Compute Alphabet = union(B,H), 
   //and delete some events by condition in Step 7
   
   // Store list1 in ext_imagelist
   list_copy(s_ext_imagelist,ext_imagelist,slist1,list1);
   
   // list1 = union(B,H)
   event_unionsets(list2,slist2,&list1,&slist1);
   
   // list3 = H - B
   complement_sets(*ext_imagelist, *s_ext_imagelist, list2, slist2, &list3, &slist3);
   
   // store union(B,H) in ext_imagelist
   list_copy(s_ext_imagelist, ext_imagelist, slist1, list1);
   if(mem_result == 1){
      result = -1;
      goto EXTEND_LABEL;
   }

   //printw("Final\n");
   //zprint_list(s_nlist, nlist);
   for(i = 0; i < slist3; i ++){
      list_copy(&slist1,&list1,*s_ext_imagelist, *ext_imagelist);
      //zprint_list1(slist1,list1);
      //printw("%d ", list3[i]);
      delete_ordlist(list3[i], &list1, slist1, &ok);
      if(ok) slist1 --;
      //zprints("list1:");
      //zprint_list1(slist1,list1);
      split_flag = true;
      for(j = 0; j < s_nlist; j ++){
         if(!split(nlist[j], slist1, list1, s_imagelist, imagelist, s1, t1, s2, t2, par1)){
            split_flag = false;
            break;
         }
         if(mem_result == 1){
            result = -1;
            goto EXTEND_LABEL;
         }
      }
      if(split_flag){
         delete_ordlist(list3[i], ext_imagelist, *s_ext_imagelist, &ok);
         if(ok) (*s_ext_imagelist)--;
      }
   }
   //printw("End");
EXTEND_LABEL:
   //printw("Free");
   freedes(s1, &t1);
   freedes(s2, &t2);
   
   free(trip1);
   free(list1);
   free(list2);
   free(list3);
   free(nlist);
   return result;
}

// void ext_obs_header_continue(char *name3, char*name4, char*name1, char*name2, INT_OS entry_type)
// {
// 	printw("NATURAL OBSERVER"); println();
// 	println();
// 	switch(entry_type){
// 	case 1:  printw("(%s,%s) = NATOBS (%s, %s)", name3, name4, name1, name2); break;
// 	case 2:  printw("(%s,%s) = NATOBS (%s, %s)", name3, name4, name1, name2); break;
// 	case 3:  printw("(%s,%s) = NATOBS (%s, [EVENTLIST])", name3, name4, name1); break;
// 	}	
// 	println();
// 	println();
// }
 
INT_OS ext_obs_proc(char *name3, char *name4, char *name1, char *name2,
                INT_T *s_nulllist, INT_T **nulllist,
                INT_T *s_imagelist, INT_T **imagelist,
                INT_T *s_ext_imagelist, INT_T ** ext_imagelist,
                INT_S *s_statemap, INT_S **statemap,
                INT_OS mode,
				INT_B print_flag,
				INT_OS entry_type)
{
   INT_S s1, s2; 
   state_node *t1, *t2;
   INT_T s_nulist, *nulist;
   INT_S s_par; part_node *par;
   INT_S i, init;
   INT_OS result;
   INT_B  is_det = false;
   
   s1 = s2 = 0; t1 = t2 = NULL;
   *s_ext_imagelist = 0;
   *ext_imagelist = NULL;
   s_nulist = 0; nulist = NULL;
   s_par = 0; par = NULL;
   
   // if(print_flag){
	//    if(_wherey() > 16){
	// 	   clear();
	// 	   ext_obs_header_continue(name3, name4, name1, name2, entry_type);
	//    } 

	//    printw("DES2 (Seed) image: "); println();
	//    print_eventlist(*s_imagelist, *imagelist);  println();
	//    println();
	//    print_flag = false;

   // }
   
   i = 0;
   while(!is_det){

       result = CanQC_proc3(name3, name1, *s_nulllist, *nulllist, *s_imagelist, *imagelist,
                         s_statemap, statemap, &s_par, &par, mode);  
                      
       if(result != 0)
          goto EXT_OBS_LABEL;
       i ++;
       init = 0L;   
       if(getdes(name3,&s2,&init, &t2) == false){
          result = -1;
          goto EXT_OBS_LABEL;
       }
       if(ext_is_deterministic(t2,s2)){
		   // if(print_flag){
			//    if(_wherey() > 16){
			// 	   clear();
			// 	   ext_obs_header_continue(name3, name4, name1, name2, entry_type);
			// 	   println();
			//    } 
			//    printw("DES4 (Natural observer) image: "); println();
			//    print_eventlist(*s_imagelist, *imagelist); println();
			//    println();
		   // }

          is_det = true;
          *s_ext_imagelist = *s_imagelist;
          *ext_imagelist = (INT_T*)REALLOC(*ext_imagelist, (*s_ext_imagelist) * sizeof(INT_T));
          if(*ext_imagelist == NULL){
             result = -1;
             goto EXT_OBS_LABEL;
          }
          memcpy(*ext_imagelist, *imagelist, (*s_ext_imagelist) * sizeof(INT_T));
       } else{
          result = event_extend(name1,name3,*s_imagelist,*imagelist, s_par, par, s_ext_imagelist, ext_imagelist);
          if(result != 0)  goto EXT_OBS_LABEL;
          *s_imagelist = *s_ext_imagelist;
          *imagelist = (INT_T*)REALLOC(*imagelist, (*s_imagelist) * sizeof(INT_T));
          if(*imagelist == NULL){
             result = -1;
             goto EXT_OBS_LABEL;
          }
          memcpy(*imagelist, *ext_imagelist, (*s_imagelist) * sizeof(INT_T));
          init = 0L;
          if(getdes(name1,&s1,&init,&t1) == false){
             mem_result = 1;
             result = -1;
             goto EXT_OBS_LABEL;
          }
          gen_complement_list(t1,s1,*imagelist,*s_imagelist, &nulist,&s_nulist);
          *s_nulllist = s_nulist;
          *nulllist = (INT_T*)REALLOC(*nulllist, (*s_nulllist) * sizeof(INT_T));
          if(s_nulist != 0){
             if(*nulllist == NULL){
                mem_result = 1;
                result = -1;
                goto EXT_OBS_LABEL;
             }
             memcpy(*nulllist, nulist, (*s_nulllist) * sizeof(INT_T));
          }
          free(nulist);
          s_nulist = 0; nulist = NULL;
          freedes(s1,&t1);
          s1 = 0; t1 = NULL;
          free(*ext_imagelist);
          *s_ext_imagelist = 0; *ext_imagelist = NULL;
          free(*statemap);
          *s_statemap = 0; *statemap = NULL;
       }
       free_part(s_par, &par);
       s_par = 0; par = NULL;
       freedes(s2,&t2);
       s2 = 0; t2 = NULL;
      
   }

EXT_OBS_LABEL:
   freedes(s2, &t2);
   freedes(s1, &t1);
   free_part(s_par, &par);
   return result;
}

//used to compute the equivalence (quasi-congruence & control consistency)
//zry
INT_OS supqc_cc_proc(char *name3, char *name1, char *name2,
                INT_T s_nulllist, INT_T *nulllist,
                INT_T s_imagelist, INT_T *imagelist,
                INT_S *s_statemap, INT_S **statemap,
                INT_OS mode)
{
   INT_S s1, s2, s3, s4, s5, init, ss;
   state_node *t1, *t2, *t3, *t4, *t5;    
   INT_B  ok;
   INT_S i, k, state, ii;
   INT_T j, jj, ee;
   INT_T s_conlist, *conlist, event;
   INT_OS result = 0;   
   
   INT_S s_par1, s_par2, s_par3, s_par4;
   part_node *par1, *par2, *par3;
   
   INT_S s_remap;
   INT_S *remap;
   
   INT_T s_imagelist2, *imagelist2;
   
   s1 = s2 = s3 = s4 = s5 = init = 0;
   t1 = t2 = t3 = t4 = t5 = NULL;
   
   s_imagelist2 = 0;
   imagelist2 = NULL;
   
   s_par1 = s_par2 = s_par3 = s_par4 = 0;
   par1 = par2 = par3 = NULL;
   
   s_conlist = 0;
   conlist = NULL;
   
   init = 0L;
   if (getdes(name1, &s1, &init, &t1) == false) {
      return -1;
   }  
   
   init = -1L;
   if (getdes(name2, &s3, &init, &t3) == false) {
      return -1;
   }  
   if(s1 != s3){
       result = -2;
       goto CANLABEL1;
   }
   
   /* NULL */
   /*if (s1 == 0)
   {
      init = 0L;
      filedes(name3, s1, init, t1);
      goto CANLABEL1;
   }   */
   
   /* Look for case everything is NULL */
   /*if (s_imagelist == 0) 
   {
      s2 = 1;
      t2 = newdes(s2);
      
      t2[0].marked = t1[0].marked;  
      
      init = 0L;
      filedes(name3, s2, init, t2);
      goto CANLABEL1;
   }  */
   
   event = imagelist[0];
   addordlist(event, &imagelist2, s_imagelist2, &ok);
   if(ok) s_imagelist2 ++;
   //generate observable event set from enable/disable information
   for(i = 0; i < s1; i ++){
      if(inordlist1(event, t1[i].next, t1[i].numelts)){
         for(j = 0; j < t1[i].numelts; j ++){
            ee = t1[i].next[j].data1;
            ss = t1[i].next[j].data2;
            if(inordlist1(ee, t3[ss].next, t3[ss].numelts)){
               
               addordlist(ee, &imagelist2, s_imagelist2, &ok);
               if(ok) s_imagelist2 ++;
            }
         }
      }
      if(inordlist1(event, t3[i].next, t3[i].numelts)){         
         for(j = 0; j < t1[i].numelts; j ++){
            ee = t1[i].next[j].data1;
            ss = t1[i].next[j].data2;
            if(inordlist1(event, t1[ss].next, t1[ss].numelts)){
               addordlist(ee, &imagelist2, s_imagelist2, &ok);
               if(ok) s_imagelist2 ++;
            }
         }
      }
   }

   s2 = s1;         
   t2 = newdes(s2);
   if ((s2 != 0) && (t2 == NULL))
   {
      result = -1;
      goto CANLABEL1;
   }            

   /* Step 3 */          
   /* Create t2 with partition states and mark NULL event as "e" = 1000 */
   for (i=0; i < s1; i++) {
      t2[i].marked = t1[i].marked;
      t2[i].vocal  = t1[i].vocal;
      for (j=0; j < t1[i].numelts; j++) {
          ee = t1[i].next[j].data1;
          jj = (INT_T)t1[i].next[j].data2;          
          if (!inlist(ee, imagelist2, s_imagelist2))
             ee = EEE;  /* "e" = 1000 */
             
          addordlist1(ee, jj, &t2[i].next, t2[i].numelts, &ok);
          if (ok) t2[i].numelts++;
      }    
      
      /* Add selfloop to marked states with label "m" = 1001 */
      if (t1[i].marked)
      {
         addordlist1(1001, i, &t2[i].next, t2[i].numelts, &ok);
         if (ok) t2[i].numelts++;
      }   
   }     
   
   /* Can delete t1 at this point */
   //freedes(s1, &t1);
   //s1 = 0;
   //t1 = NULL;
         
   /* Step 4 */
   s_par2 = 2;
   par2 = (part_node*) CALLOC(s_par2, sizeof(part_node));
   if (par2 == NULL) {
      result = -1;
      goto CANLABEL1;
   }
   
   s_par3 = 2;
   par3 = (part_node*) CALLOC(s_par3, sizeof(part_node));
   if (par3 == NULL) {
      result = -1;
      goto CANLABEL1;
   }        
   
   /* Construct Alph' = Image U {m} */
   /*for (j=0; j < s_imagelist; j++)
   {
      addordlist(imagelist[j], &imagelist2, s_imagelist2, &ok);
      if (ok) s_imagelist2++;
   }  */  
   addordlist(1001, &imagelist2, s_imagelist2, &ok);   /* "m" = 1001 */
   if (ok) s_imagelist2++;
   
   //zprint_list1(s_imagelist2, imagelist2);
   ParGen(imagelist2[0], s2, t2, s_par3, par3);
   for (j=1; j < s_imagelist2; j++)
   {
      ParGen(imagelist2[j], s2, t2, s_par2, par2);
      
      s_par1 = s_par3;
      par1   = par3;          
      s_par3 = 0;
      par3   = NULL;
         
      ParMeet(&s_par3, &par3, s_par1, par1, s_par2, par2);
     
      /* Zero out content of par2 */
      for (i=0; i < s_par2; i++)
      {
        free(par2[i].next);
        par2[i].next = NULL;
        par2[i].numelts = 0;
      }   
   }  
   //zprint_par(s_par3, par3);
   free_par(&s_par1, &par1);
   free_par(&s_par2, &par2);
   
   /*Compute the meet of initial partion and control congruence*/   
   /*for(i = 0; i < s1; i ++){
      for(j = 0; j < t1[i].numelts; j ++){
          event = t1[i].next[j].data1;
          if((event %2 == 1) && inlist(event, imagelist, s_imagelist)){
             addordlist(event, &conlist, s_conlist, &ok);
             if(ok) s_conlist ++;
          }
      }
   }*/
   
   //for(i = 0; i < s_conlist; i ++){
       //event = conlist[i];
       s_par1 = 2;
       par1 = (part_node*)CALLOC(s_par1, sizeof(part_node));
       for(j = 0; j < s3; j ++){
           if(inordlist1(event, t3[j].next, t3[j].numelts)){
              addstatelist(j, &par1[0].next,par1[0].numelts, &ok);
              if(ok) par1[0].numelts ++;
           }else{
              addstatelist(j, &par1[1].next,par1[1].numelts, &ok);
              if(ok) par1[1].numelts ++;
           }
       }
       ParMeet(&s_par2, &par2, s_par1, par1, s_par3, par3);
       Arrange(&s_par3, &par3, &s_par2, &par2, s3);
       free_par(&s_par1, &par1);
       free_par(&s_par2, &par2);
       s_par1 = s_par2 = 0;
       par1 = par2 = NULL;
   //}
   //zprint_par(s_par3, par3);
   /* Put the answer back into par1 for now */   
   Arrange(&s_par1, &par1, &s_par3, &par3, s2);
   
   /*delte unnecessary part and states*/
   free_par(&s_par3, &par3);     
   freedes(s1, &t1);
   freedes(s3, &t3);
   s1 = s3 = 0;
   t1 = t3 = NULL;
   /*** DEDUG ***/   
/* print_par(s_par1, par1); */
   /*** DEBUG ***/
   
   /* Step 5 -- it is integrated with step 4 */
   
   /* Step 6 */
   
   if (mode == 2){
     addordlist(EEE, &imagelist2, s_imagelist2, &ok);   /* "e" = 1000 */
     if(ok) s_imagelist2 ++;
   }
       
   eta(s2, t2, &s3, &t3, s_imagelist2, imagelist2);

   /*** DEBUG ***/
   /* print_eta(s3, t3, s_imagelist, imagelist); */
   /*** DEBUG ***/

   /* Step 7 */
   eta_to_table(s3, t3, &s4, &t4, s_par1, par1);

   /* Step 8 */
   table_to_par(s4, t4, &s_par2, &par2);
   
   freedes(s4, &t4);
   t4 = NULL;
   s4 = 0;
   
   /* Step 9 */
   ParMeet(&s_par3, &par3, s_par1, par1, s_par2, par2);   

   free_par(&s_par1, &par1);
   free_par(&s_par2, &par2);

   /* Step 10 */
   do {
      s_par4 = s_par3; 
      
      s_par1 = s_par3;
      par1   = par3;
      s_par3 = 0;
      par3   = NULL;

      eta_to_table(s3, t3, &s4, &t4, s_par1, par1);
      table_to_par(s4, t4, &s_par2, &par2);
      freedes(s4, &t4);
      t4 = NULL;
      s4 = 0;
      
      ParMeet(&s_par3, &par3, s_par1, par1, s_par2, par2);
      
      /* Free up space used by s_par1, s_par2 */
      free_par(&s_par1, &par1);
      free_par(&s_par2, &par2);
   } while (s_par4 != s_par3);
   
   /* Step 11 */
   s5 = s_par3;
   t5 = newdes(s5);   
   if ((s5 != 0) && (t5 == NULL))
   {
      result = -1;
      goto CANLABEL1;
   }    

   /* Create statemap and remap list from (par3, s_par3) */  
   *s_statemap = s2;
   *statemap = (INT_S*) MALLOC((*s_statemap) * sizeof(INT_S));
   if ((*s_statemap != 0) && (*statemap == NULL))
   {
      result = -1;
      goto CANLABEL1;
   }      
   
   s_remap = s2;
   remap = (INT_S*) MALLOC(s_remap * sizeof(INT_S)); 
   if ((s_remap !=0) && (remap == NULL))
   {
      result = -1;
      goto CANLABEL1;
   }   
         
   for (i=0; i < s_par3; i++) {
      for (k=0; k < par3[i].numelts; k++) {
          state = par3[i].next[k];
          (*statemap)[state] = par3[i].next[0]; 
          remap[state] = i;
      }   
   }    
     
   for (i=0; i < s2; i++) {
      ii = remap[i];
      for (j=0; j < t2[i].numelts; j++) {
          ee = t2[i].next[j].data1;
          jj = (INT_T)remap[ t2[i].next[j].data2 ];          
             
          if ((ii == jj) && (ee == 1001)) {  /* "m" = 1001 */
             t5[ii].marked = true;
          } else if ((ii == jj) && (ee == EEE)) { /* "e" = 1000 */
             /* Remove selfloop for non-observable events only */
          } else {   
             addordlist1(ee, jj, &t5[ii].next, t5[ii].numelts, &ok);
             if (ok) t5[ii].numelts++;
          }   
      }    
   }     
                  
   /* Step 12 */
   init = 0L;
   filedes(name3, s5, init, t5);         
   
CANLABEL1:   
   freedes(s1, &t1);
   freedes(s2, &t2); 
   freedes(s3, &t3);
   freedes(s4, &t4);
   freedes(s5, &t5);

   free_par(&s_par1, &par1);  
   free_par(&s_par2, &par2);
   free_par(&s_par3, &par3); 

   free(remap);
   free(imagelist2);
   
   return result;
}

//used to compute (quasi-congruence & local control consistency)
//zry

INT_OS supqc_lcc_proc(char *name2, char *name1,
                INT_T s_nulllist, INT_T *nulllist,
                INT_T s_imagelist, INT_T *imagelist,
                INT_S *s_statemap, INT_S **statemap,
                INT_OS mode)
{
	INT_S s1, s2, s3, s4, s5, init;
	state_node *t1, *t2, *t3, *t4, *t5;    
	INT_B  ok;
	INT_S i, k, state, ii;
	INT_T j, jj, ee;
	INT_OS result = 0;   

	INT_S s_par1, s_par2, s_par3, s_par4;
	part_node *par1, *par2, *par3;

	INT_S s_remap;
	INT_S *remap;

	INT_T s_imagelist2, *imagelist2;

	s1 = s2 = s3 = s4 = s5 = init = 0;
	t1 = t2 = t3 = t4 = t5 = NULL;

	s_imagelist2 = 0;
	imagelist2 = NULL;

	s_par1 = s_par2 = s_par3 = s_par4 = 0;
	par1 = par2 = par3 = NULL;

	init = 0L;
	if (getdes(name1, &s1, &init, &t1) == false) {
		return -1;
	}  

	/* NULL */
	if (s1 == 0)
	{
		init = 0L;
		filedes(name2, s1, init, t1);
		goto CANLABEL1;
	}   

	/* Look for case everything is NULL */
	if (s_imagelist == 0) 
	{
		s2 = 1;
		t2 = newdes(s2);

		t2[0].marked = t1[0].marked;  

		init = 0L;
		filedes(name2, s2, init, t2);
		goto CANLABEL1;
	}  

	s2 = s1;         
	t2 = newdes(s2);
	if ((s2 != 0) && (t2 == NULL))
	{
		result = -1;
		goto CANLABEL1;
	}            

	/* Step 3 */          
	/* Create t2 with partition states and mark NULL event as "e" = 1000 */
	for (i=0; i < s1; i++) {
		t2[i].marked = t1[i].marked;
		t2[i].vocal  = t1[i].vocal;
		for (j=0; j < t1[i].numelts; j++) {
			ee = t1[i].next[j].data1;
			ii = t1[i].next[j].data2;          
			if (inlist(ee, nulllist, s_nulllist))
				ee = EEE;  /* "e" = 1000 */

			addordlist1(ee, ii, &t2[i].next, t2[i].numelts, &ok);
			if (ok) t2[i].numelts++;
		}    

		/* Add selfloop to marked states with label "m" = 1001 */
		if (t1[i].marked)
		{
			addordlist1(1001, i, &t2[i].next, t2[i].numelts, &ok);
			if (ok) t2[i].numelts++;
		}   
	}     

	/* Can delete t1 at this point 
	freedes(s1, &t1);
	s1 = 0;
	t1 = NULL;*/

	/* Step 4 */
	s_par2 = 2;
	par2 = (part_node*) CALLOC(s_par2, sizeof(part_node));
	if (par2 == NULL) {
		result = -1;
		goto CANLABEL1;
	}

	s_par3 = 2;
	par3 = (part_node*) CALLOC(s_par3, sizeof(part_node));
	if (par3 == NULL) {
		result = -1;
		goto CANLABEL1;
	}        

	/* Construct Alph' = Image U {m} */
	for (j=0; j < s_imagelist; j++)
	{
		addordlist(imagelist[j], &imagelist2, s_imagelist2, &ok);
		if (ok) s_imagelist2++;
	}    
	addordlist(1001, &imagelist2, s_imagelist2, &ok);   /* "m" = 1001 */
	if (ok) s_imagelist2++;

	ParGen(imagelist2[0], s2, t2, s_par3, par3);
	for (j=1; j < s_imagelist2; j++)
	{
		ParGen(imagelist2[j], s2, t2, s_par2, par2);

		s_par1 = s_par3;
		par1   = par3;          
		s_par3 = 0;
		par3   = NULL;

		ParMeet(&s_par3, &par3, s_par1, par1, s_par2, par2);

		/* Zero out content of par2 */
		for (i=0; i < s_par2; i++)
		{
			free(par2[i].next);
			par2[i].next = NULL;
			par2[i].numelts = 0;
		}   
	}  

	free_par(&s_par1, &par1);
	free_par(&s_par2, &par2);

	/* Put the answer back into par1 for now */   
	Arrange(&s_par1, &par1, &s_par3, &par3, s2);
	free_par(&s_par3, &par3);     

	/*** DEDUG ***/   
	/* print_par(s_par1, par1); */
	/*** DEBUG ***/

	/* Step 5 -- it is integrated with step 4 */

	/* Step 6 */

	if (mode == 2){
		addordlist(EEE, &imagelist2, s_imagelist2, &ok);   /* "e" = 1000 */
		if(ok) s_imagelist2 ++;
	}

	eta(s2, t2, &s3, &t3, s_imagelist2, imagelist2);
	eta_lcc(s1, t1, s3, t3, s_imagelist, imagelist);

	//filedes("TEST", s3, 0, t3);

	/*** DEBUG ***/
	/* print_eta(s3, t3, s_imagelist, imagelist); */
	/*** DEBUG ***/

	/* Step 7 */
	eta_to_table(s3, t3, &s4, &t4, s_par1, par1);

	/* Step 8 */
	table_to_par(s4, t4, &s_par2, &par2);

	freedes(s4, &t4);
	t4 = NULL;
	s4 = 0;

	/* Step 9 */
	ParMeet(&s_par3, &par3, s_par1, par1, s_par2, par2);   

	free_par(&s_par1, &par1);
	free_par(&s_par2, &par2);

	/* Step 10 */
	do {
		s_par4 = s_par3; 

		s_par1 = s_par3;
		par1   = par3;
		s_par3 = 0;
		par3   = NULL;

		eta_to_table(s3, t3, &s4, &t4, s_par1, par1);
		table_to_par(s4, t4, &s_par2, &par2);
		freedes(s4, &t4);
		t4 = NULL;
		s4 = 0;

		ParMeet(&s_par3, &par3, s_par1, par1, s_par2, par2);

		/* Free up space used by s_par1, s_par2 */
		free_par(&s_par1, &par1);
		free_par(&s_par2, &par2);
	} while (s_par4 != s_par3);

	/* Step 11 */
	s5 = s_par3;
	t5 = newdes(s5);   
	if ((s5 != 0) && (t5 == NULL))
	{
		result = -1;
		goto CANLABEL1;
	}    

	/* Create statemap and remap list from (par3, s_par3) */  
	*s_statemap = s2;
	*statemap = (INT_S*) MALLOC((*s_statemap) * sizeof(INT_S));
	if ((*s_statemap != 0) && (*statemap == NULL))
	{
		result = -1;
		goto CANLABEL1;
	}      

	s_remap = s2;
	remap = (INT_S*) MALLOC(s_remap * sizeof(INT_S)); 
	if ((s_remap !=0) && (remap == NULL))
	{
		result = -1;
		goto CANLABEL1;
	}   

	for (i=0; i < s_par3; i++) {
		for (k=0; k < par3[i].numelts; k++) {
			state = par3[i].next[k];
			(*statemap)[state] = par3[i].next[0]; 
			remap[state] = i;
		}   
	}    

	for (i=0; i < s2; i++) {
		ii = remap[i];
		for (j=0; j < t2[i].numelts; j++) {
			ee = t2[i].next[j].data1;
			jj = (INT_T)remap[ t2[i].next[j].data2 ];          

			if ((ii == jj) && (ee == 1001)) {  /* "m" = 1001 */
				t5[ii].marked = true;
			} else if ((ii == jj) && (ee == EEE)) { /* "e" = 1000 */
				/* Remove selfloop for non-observable events only */
			} else {   
				addordlist1(ee, jj, &t5[ii].next, t5[ii].numelts, &ok);
				if (ok) t5[ii].numelts++;
			}   
		}    
	}     

	/* Step 12 */
	init = 0L;
	filedes(name2, s5, init, t5);         

CANLABEL1:   
	freedes(s1, &t1);
	freedes(s2, &t2); 
	freedes(s3, &t3);
	freedes(s4, &t4);
	freedes(s5, &t5);

	free_par(&s_par1, &par1);  
	free_par(&s_par2, &par2);
	free_par(&s_par3, &par3); 

	free(remap);
	free(imagelist2);

	return result;
}


// Ferandez's algorithm
//Updated by ZRY

typedef struct ext_part_node {
	INT_S numelts;
	part_node *data;
} ext_part_node;

void free_ext_part(INT_S s1,
	ext_part_node **pn)
{
	INT_S i;

	for (i=0; i < s1; i++) {
		if ( (*pn)[i].data != NULL )
			free_part((*pn)[i].numelts, &(*pn)[i].data);
	}
	free(*pn);
}
// void zprint_ext_par(INT_S s, ext_part_node *ext_par, INT_T *imagelist)
// {
// 	FILE *out;
// 	INT_S i, j, k;
// 	char tmp_result[MAX_PATH];

// 	strcpy(tmp_result, "");
// 	strcat(tmp_result, prefix);
// 	strcat(tmp_result, "Ouput.txt");
// 	out = fopen(tmp_result,"a");


// 	for(i = 0; i < s; i ++){
// 		fprintf(out, "state - %d\n", i);
// 		for(j = 0; j < ext_par[i].numelts; j ++){
// 			fprintf(out,"%d: ", imagelist[j]);
// 			for(k = 0; k < ext_par[i].data[j].numelts; k ++)
// 				fprintf(out,"%d ", ext_par[i].data[j].next[k]);
// 			fprintf(out,"\n");
// 		}
// 		fprintf(out,"\n\n");
// 	}
// 	fprintf(out,"\n");

// 	fclose(out);
// }
// Find event index
INT_S getindex(INT_T e, INT_T *L, INT_T size)
{
	INT_S k;
	INT_S pos;
	INT_S lower, upper;
	INT_B found;
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
void eta_to_tran(INT_S s1, state_node *t1, INT_S *s_ext_par, ext_part_node **ext_par, INT_T s_imagelist, INT_T * imagelist)
{
	INT_S i,j, k;
	INT_T event;
	part_node * temp;
	INT_B ok;

	*s_ext_par = s1;
	*ext_par = (ext_part_node *)CALLOC(*s_ext_par, sizeof(ext_part_node));
	if(ext_par == NULL){
		mem_result = 1;
		return;
	}
	for(i = 0; i < s1; i ++){
		(*ext_par)[i].numelts = s_imagelist;
		(*ext_par)[i].data = (part_node *)CALLOC(s_imagelist, sizeof (part_node));
	}
	for(i = 0; i < s1; i ++){		
		for(j = 0; j < t1[i].numelts;j ++){
			event = t1[i].next[j].data1;
			k = getindex(event, imagelist, s_imagelist);
			//ss = t1[i].next[j].data2;
			temp = (*ext_par)[i].data;			
			addstatelist(t1[i].next[j].data2, &temp[k].next, temp[k].numelts, &ok);
			if (ok) temp[k].numelts ++;
		}
	}

}
void eta_to_invtran(INT_S s1, state_node *t1, INT_S *s_ext_par, ext_part_node **ext_par, INT_T s_imagelist, INT_T * imagelist)
{
	INT_S i,j, k;
	INT_T event; INT_S ss;
	part_node * temp;
	INT_B ok;

	*s_ext_par = s1;
	*ext_par = (ext_part_node *)CALLOC(*s_ext_par, sizeof(ext_part_node));
	if(*ext_par == NULL){
		mem_result = 1;
		return;
	}
	for(i = 0; i < s1; i ++){
		(*ext_par)[i].numelts = s_imagelist;
		(*ext_par)[i].data = (part_node *)CALLOC(s_imagelist, sizeof (part_node));
	}
	for(i = 0; i < s1; i ++){		
		for(j = 0; j < t1[i].numelts;j ++){
			event = t1[i].next[j].data1;
			k = getindex(event, imagelist, s_imagelist);
			ss = t1[i].next[j].data2;
			temp = (*ext_par)[ss].data;			
			addstatelist(i, &temp[k].next, temp[k].numelts, &ok);
			if (ok) temp[k].numelts ++;
		}
	}

}

INT_S compute_trans(part_node par1, INT_S index, ext_part_node *par2, INT_S *s_list, INT_S **list)
{
	INT_S i, j, state;
	INT_S s_tmp, *tmp;
	INT_B ok;

	*s_list = 0; *list = NULL;

	for(i = 0; i < par1.numelts; i ++){
		state = par1.next[i];
		s_tmp = par2[state].data[index].numelts;
		tmp = par2[state].data[index].next;
		for(j = 0; j < s_tmp; j ++){
			addstatelist(tmp[j], list, *s_list, &ok);
			if(ok) (*s_list) ++;
		}

	}
	
	return *s_list;
}
INT_B qc_equal_list(INT_S *list1, INT_S *list2, INT_S numelts)
{
	INT_S i;

	for (i=0; i < numelts; i++) {
		if (list1[i] != list2[i])
			return false;
	}
	return true;
}

/*
 * Par2 = Arrange (Par1)
 */

void New_Arrange(INT_S *s_par2, part_node **par2, INT_S *s_par1, part_node **par1, INT_S nStates)
{
   INT_S i,j,k,m;
   INT_B ok;
   INT_B *parbool;
    
   *s_par2 = *s_par1;  
   *par2 = (part_node*) CALLOC(*s_par2, sizeof(part_node));
   if (*par2 == NULL) {
     mem_result = 1;
     return;
   }
   
   parbool = (INT_B*) CALLOC(nStates, sizeof(INT_B));
   if (parbool == NULL) {
     mem_result = 1;
     free(*par2); *par2= NULL;  
     *s_par2 = 0;
     return;
   }  

   m = 0;
   for (i=0; i < nStates; i++) {
     for (j=0; (j < *s_par1) && (parbool[i] == false); j++) {
        for (k=0; k < (*par1)[j].numelts; k++) {
           if ((*par1)[j].next[k] == i)
              parbool[i] = true;
        }   
        
        if (parbool[i] == true) {
           for (k=0; k < (*par1)[j].numelts; k++)
           {
              addstatelist( (*par1)[j].next[k], &(*par2)[m].next, (*par2)[m].numelts, &ok);
              if (ok) {
                 (*par2)[m].numelts++;
                 parbool[(*par1)[j].next[k]] = true;                  
              }   
           }   
           m++;
        }   
     }             
   }    
   
   free(parbool);

   j = 0;
   for(i = 0; i < *s_par2; i ++){
	   if((*par2)[i].numelts != 0)
		   j ++;
	   else{
		   if((*par2)[i].next != NULL){
			   free((*par2)[i].next);
			   (*par2)[i].next = NULL;
		   }
	   }
   }
   *s_par2 = j;
   *par2 = (part_node*)REALLOC(*par2, j * sizeof(part_node));
}    


INT_OS back_CanQC_proc2(char *name2, char *name1,
					INT_T s_nulllist, INT_T *nulllist,
					INT_T s_imagelist, INT_T *imagelist,
					INT_S *s_statemap, INT_S **statemap,
					INT_OS mode)
{
	INT_S s1, s2, s3, s4, s5, init;
	state_node *t1, *t2, *t3, *t4, *t5;    
	INT_B  ok, equal_flag;
	INT_S i, k, l, state, ii;
	INT_T j, jj, ee;
	INT_OS result, f1, f2, f3;
	INT_S split_index;

	INT_S s_par1, s_par2, s_par3, s_par4;
	part_node *par1, *par2, *par3, *temp, *spliter;

	INT_S s_ext_par1, s_ext_par2;
	ext_part_node *ext_par1, *ext_par2;

	INT_S s_remap;
	INT_S *remap;

	INT_T s_imagelist2, *imagelist2;
	INT_S s_list, *list, s_lA, s_lB, *lA, *lB, s_l1, s_l2, s_l3, *l1, *l2, *l3;

	result = 0;

	s1 = s2 = s3 = s4 = s5 = init = 0;
	t1 = t2 = t3 = t4 = t5 = NULL;

	s_imagelist2 = 0;
	imagelist2 = NULL;

	s_par1 = s_par2 = s_par3 = s_par4 = 0;
	par1 = par2 = par3 = NULL;

	s_ext_par1 = s_ext_par2 = 0;
	ext_par1 = ext_par2 = NULL;

	s_list = s_lA = s_lB = 0; 
	list = lA = lB = NULL;

	init = 0L;
	if (getdes(name1, &s1, &init, &t1) == false) {
		return -1;
	}  

	/* NULL */
	if (s1 == 0)
	{
		init = 0L;
		filedes(name2, s1, init, t1);
		goto CANLABEL1;
	}   

	/* Look for case everything is NULL */
	if (s_imagelist == 0) 
	{
		s2 = 1;
		t2 = newdes(s2);

		t2[0].marked = t1[0].marked;  

		init = 0L;
		filedes(name2, s2, init, t2);
		goto CANLABEL1;
	}  

	//melt_e_p(s1, t1, &s3, &t3, s_imagelist, imagelist);

	s2 = s1;         
	t2 = newdes(s2);
	if ((s2 != 0) && (t2 == NULL))
	{
		result = -1;
		goto CANLABEL1;
	}            

	/* Step 3 */          
	/* Create t2 with partition states and mark NULL event as "e" = 1000 */
	for (i=0; i < s1; i++) {
		t2[i].marked = t1[i].marked;
		t2[i].vocal  = t1[i].vocal;
		for (j=0; j < t1[i].numelts; j++) {
			ee = t1[i].next[j].data1;
			jj = (INT_T)t1[i].next[j].data2;          
			if (inlist(ee, nulllist, s_nulllist))
				ee = EEE;  /* "e" = 1000 */

			addordlist1(ee, jj, &t2[i].next, t2[i].numelts, &ok);
			if (ok) t2[i].numelts++;
		}    

		/* Add selfloop to marked states with label "m" = 1001 */
		if (t1[i].marked)
		{
			addordlist1(1001, i, &t2[i].next, t2[i].numelts, &ok);
			if (ok) t2[i].numelts++;
		}   
	}     

	/* Can delete t1 at this point */
	freedes(s1, &t1);
	s1 = 0;
	t1 = NULL;

	/* Step 4 */
	s_par2 = 2;
	par2 = (part_node*) CALLOC(s_par2, sizeof(part_node));
	if (par2 == NULL) {
		result = -1;
		goto CANLABEL1;
	}

	s_par3 = 2;
	par3 = (part_node*) CALLOC(s_par3, sizeof(part_node));
	if (par3 == NULL) {
		result = -1;
		goto CANLABEL1;
	}        

	/* Construct Alph' = Image U {m} */
	for (j=0; j < s_imagelist; j++)
	{
		addordlist(imagelist[j], &imagelist2, s_imagelist2, &ok);
		if (ok) s_imagelist2++;
	}    
	addordlist(1001, &imagelist2, s_imagelist2, &ok);   /* "m" = 1001 */
	if (ok) s_imagelist2++;

	if (mode == 2){
		addordlist(EEE, &imagelist2, s_imagelist2, &ok);   /* "e" = 1000 */
		if(ok) s_imagelist2 ++;
	}

	eta(s2, t2, &s3, &t3, s_imagelist2, imagelist2);

	// Store transition list into ext_par
	eta_to_tran(s3, t3, &s_ext_par1, &ext_par1, s_imagelist2, imagelist2);

//	zprint_ext_par(s_ext_par1, ext_par1, imagelist2);

	// Initialize the partition and the splitters
	s_list = s3;
	list = (INT_S *)CALLOC(s_list, sizeof(INT_S));
	for(i = 0; i < s3; i ++){
		list[i] = i;
	}

	s_par1 = 1;
	par1 = (part_node *)CALLOC(s_par1, sizeof(part_node));
	if(par1 == NULL){
		mem_result = 1;
		goto CANLABEL1;
	}
	par1[0].numelts = s_list;
	par1[0].next = (INT_S*)CALLOC(s_list, sizeof(INT_S));
	memcpy(par1[0].next, list, s_list * sizeof(INT_S));

	s_ext_par2 = 1;
	ext_par2 = (ext_part_node*)CALLOC(1, sizeof(ext_part_node));
	if(ext_par2 == NULL){
		mem_result = 1;
		goto CANLABEL1;
	}
	ext_par2[0].numelts = 1;
	ext_par2[0].data = (part_node*)CALLOC(1, sizeof(part_node));
	temp = ext_par2[0].data;
	temp[0].numelts = s_list;
	temp[0].next = (INT_S*)CALLOC(s_list, sizeof(INT_S));
	memcpy(temp[0].next, list, s_list * sizeof(INT_S));

	free(list); list = NULL; s_list = 0;

	split_index = 0;
	while(split_index < s_ext_par2){	
		spliter = ext_par2[split_index].data;
		//num_par = s_par1;
		if(ext_par2[split_index].numelts == 1){
			for(i = 0; i < s_imagelist2; i ++){
				// Compute Ta^{-1}B
				if(compute_trans(spliter[0], i, ext_par1, &s_list, &list) == 0)
					continue;
				for(j = 0; j < s_par1; j ++){
					if(par1[j].numelts <= 1)
						continue;
					s_l1 = s_l2 = 0;
					l1 = l2 = NULL;
					for(k = 0; k < par1[j].numelts; k ++){	
						state = par1[j].next[k];
						if(instatelist(state, list, s_list)){
							addstatelist(state, &l1, s_l1, &ok);
							if(ok) s_l1 ++;
						}else{
							addstatelist(state, &l2, s_l2, &ok);
							if(ok) s_l2 ++;
						}
					}
					if(s_l1 == 0 || s_l2 == 0){
						continue;
					}else{ 
						//Update the splitter
						equal_flag = false;
						for(l = split_index + 1; l < s_ext_par2; l ++){
							if(ext_par2[l].data[0].numelts != s_l2)
								continue;
							else if(qc_equal_list(ext_par2[l].data[0].next, l2, s_l2)){
								equal_flag = true;
								break;
							}
						}
						ext_par2 = (ext_part_node *)REALLOC(ext_par2, (s_ext_par2 + 1)* sizeof(ext_part_node));
						if(ext_par2 == NULL){
							mem_result = 1;
							goto CANLABEL1;
						}
						if(equal_flag){
							ext_par2[s_ext_par2].numelts = 1;
							ext_par2[s_ext_par2].data = (part_node*)CALLOC(1, sizeof(part_node));
							temp = ext_par2[s_ext_par2].data;
							temp[0].numelts = s_l1;
							temp[0].next = (INT_S *)CALLOC(s_l1, sizeof(INT_S));
							memcpy(temp[0].next, l1, s_l1 * sizeof(INT_S));

						}else{
							ext_par2[s_ext_par2].numelts = 3;
							ext_par2[s_ext_par2].data = (part_node*)CALLOC(3, sizeof(part_node));
							temp = ext_par2[s_ext_par2].data;
							temp[0].numelts = par1[j].numelts;
							temp[0].next = (INT_S *)CALLOC(par1[j].numelts, sizeof(INT_S));
							memcpy(temp[0].next,  par1[j].next, par1[j].numelts * sizeof(INT_S));
							temp[1].numelts = s_l1;
							temp[1].next = (INT_S *)CALLOC(s_l1, sizeof(INT_S));
							memcpy(temp[1].next, l1, s_l1 * sizeof(INT_S));
							temp[2].numelts = s_l2;
							temp[2].next = (INT_S *)CALLOC(s_l2, sizeof(INT_S));
							memcpy(temp[2].next, l2, s_l2 * sizeof(INT_S));
						}
						s_ext_par2 ++;

						// Update the partition						
						par1 = (part_node*)REALLOC(par1, (s_par1 + 2) * sizeof(part_node));
						if(par1 == NULL)
						{
							mem_result = 1;
							goto CANLABEL1;
						}
						free(par1[j].next);
						par1[j].numelts = 0; par1[j].next = NULL;
						
						par1[s_par1].numelts = s_l1;
						par1[s_par1].next = (INT_S *)CALLOC(s_l1, sizeof(INT_S));
						memcpy(par1[s_par1].next, l1, s_l1 * sizeof(INT_S));
						s_par1 ++;
						par1[s_par1].numelts = s_l2;
						par1[s_par1].next = (INT_S *)CALLOC(s_l2, sizeof(INT_S));
						memcpy(par1[s_par1].next, l2, s_l2 * sizeof(INT_S)); 
						s_par1 ++;

					}
					free(l1); free(l2);
					s_l1 = s_l2 = 0;
					l1 = l2 = NULL;

				}
			}
			free(list);
			s_list = 0; list = NULL;
		}else{
			for(i = 0; i < s_imagelist2; i ++){
				// Compute Ta^{-1}B
				if(compute_trans(spliter[0], i, ext_par1, &s_list, &list) <= 1)
					continue;
				compute_trans(spliter[1], i, ext_par1, &s_lA, &lA);
				compute_trans(spliter[2], i, ext_par1, &s_lB, &lB);
				for(j = 0; j < s_par1; j ++){
					if(par1[j].numelts <= 1)
						continue;
					if(!instatelist(par1[j].next[0], list, s_list))
						continue;
					s_l1 = s_l2 = s_l3 = 0;
					l1 = l2 = l3 = NULL;
					f1 = f2 = f3 = 0;
					for(k = 0; k < par1[j].numelts; k ++){	
						state = par1[j].next[k];
						if(instatelist(state, lA, s_lA)){
							if(instatelist(state, lB, s_lB)){
								addstatelist(state, &l1, s_l1, &ok);
								if(ok) s_l1 ++;
								f1 = 1;
							}else{
								addstatelist(state, &l2, s_l2, &ok);
								if(ok) s_l2 ++;
								f2 = 1;
							}
						}else{
							addstatelist(state, &l3, s_l3, &ok);
							if(ok) s_l3 ++;
							f3 = 1;
						}
					}

					if(f1 + f2 + f3 == 3){
						//Update the splitter
						ext_par2 = (ext_part_node *)REALLOC(ext_par2, (s_ext_par2 + 2)* sizeof(ext_part_node));
						if(ext_par2 == NULL){
							mem_result = 1;
							goto CANLABEL1;
						}
						ext_par2[s_ext_par2].numelts = 3;
						ext_par2[s_ext_par2].data = (part_node*)CALLOC(3, sizeof(part_node));
						temp = ext_par2[s_ext_par2].data;
						temp[0].numelts = par1[j].numelts;
						temp[0].next = (INT_S *)CALLOC(par1[j].numelts, sizeof(INT_S));
						memcpy(temp[0].next,  par1[j].next, par1[j].numelts * sizeof(INT_S));
						temp[1].numelts = s_l1;
						temp[1].next = (INT_S *)CALLOC(s_l1, sizeof(INT_S));
						memcpy(temp[1].next, l1, s_l1 * sizeof(INT_S));
						temp[2].numelts = s_l2;
						temp[2].next = (INT_S *)CALLOC(s_l2, sizeof(INT_S));
						memcpy(temp[2].next, l2, s_l2 * sizeof(INT_S));
						for(k = 0; k < s_l3; k ++){
							addstatelist(l3[k], &temp[2].next, temp[2].numelts, &ok);
							if(ok) temp[2].numelts ++;
						}
						s_ext_par2 += 1;

						ext_par2[s_ext_par2].numelts = 3;
						ext_par2[s_ext_par2].data = (part_node*)CALLOC(3, sizeof(part_node));
						temp = ext_par2[s_ext_par2].data;
						temp[0].numelts = s_l2;
						temp[0].next = (INT_S *)CALLOC(s_l2, sizeof(INT_S));
						memcpy(temp[1].next, l2, s_l2 * sizeof(INT_S));
						for(k = 0; k < s_l3; k ++){
							addstatelist(l3[k], &temp[0].next, temp[0].numelts, &ok);
							if(ok) temp[0].numelts ++;
						}
						temp[1].numelts = s_l2;
						temp[1].next = (INT_S *)CALLOC(s_l2, sizeof(INT_S));
						memcpy(temp[1].next, l2, s_l2 * sizeof(INT_S));
						temp[2].numelts = s_l3;
						temp[2].next = (INT_S *)CALLOC(s_l3, sizeof(INT_S));
						memcpy(temp[2].next, l3, s_l3 * sizeof(INT_S));
						s_ext_par2 += 1;

						// Update the partition
						par1 = (part_node*)REALLOC(par1, (s_par1 + 3) * sizeof(part_node));
						if(par1 == NULL)
						{
							mem_result = 1;
							goto CANLABEL1;
						}
						free(par1[j].next);
						par1[j].numelts = 0;
						par1[j].next = NULL;
						par1[s_par1].numelts = s_l1;
						par1[s_par1].next = (INT_S *)CALLOC(s_l1, sizeof(INT_S));
						memcpy(par1[s_par1].next, l1, s_l1 * sizeof(INT_S));
						s_par1 ++;
						par1[s_par1].numelts = s_l2;
						par1[s_par1].next = (INT_S *)CALLOC(s_l2, sizeof(INT_S));
						memcpy(par1[s_par1].next, l2, s_l2 * sizeof(INT_S));
						s_par1 ++;
						par1[s_par1].numelts = s_l3;
						par1[s_par1].next = (INT_S *)CALLOC(s_l3, sizeof(INT_S));
						memcpy(par1[s_par1].next, l3, s_l3 * sizeof(INT_S));
						s_par1 ++;

					}else if(f1 + f2 + f3 == 2){
						ext_par2 = (ext_part_node *)REALLOC(ext_par2, (s_ext_par2 + 1)* sizeof(ext_part_node));
						if(ext_par2 == NULL){
							mem_result = 1;
							goto CANLABEL1;
						}
						ext_par2[s_ext_par2].numelts = 3;
						ext_par2[s_ext_par2].data = (part_node*)CALLOC(3, sizeof(part_node));
						temp = ext_par2[s_ext_par2].data;
						s_ext_par2 += 1;
						if(f1 == 0){
							//Update the splitter
							temp[0].numelts = par1[j].numelts;
							temp[0].next = (INT_S *)CALLOC(par1[j].numelts, sizeof(INT_S));
							memcpy(temp[0].next,  par1[j].next, par1[j].numelts * sizeof(INT_S));
							temp[1].numelts = s_l2;
							temp[1].next = (INT_S *)CALLOC(s_l2, sizeof(INT_S));
							memcpy(temp[1].next, l2, s_l2 * sizeof(INT_S));
							temp[2].numelts = s_l3;
							temp[2].next = (INT_S *)CALLOC(s_l3, sizeof(INT_S));
							memcpy(temp[2].next, l3, s_l3 * sizeof(INT_S));
							
							// Update the partition
							par1 = (part_node*)REALLOC(par1, (s_par1 + 2) * sizeof(part_node));
							if(par1 == NULL)
							{
								mem_result = 1;
								goto CANLABEL1;
							}
							free(par1[j].next);
							par1[j].numelts = 0;
							par1[j].next = NULL;
							par1[s_par1].numelts = s_l2;
							par1[s_par1].next = (INT_S *)CALLOC(s_l2, sizeof(INT_S));
							memcpy(par1[s_par1].next, l2, s_l2 * sizeof(INT_S));
							s_par1 ++;
							par1[s_par1].numelts = s_l3;
							par1[s_par1].next = (INT_S *)CALLOC(s_l3, sizeof(INT_S));
							memcpy(par1[s_par1].next, l3, s_l3 * sizeof(INT_S));
							s_par1 ++;
						}else if (f2 == 0){
							//Update the splitter
							temp[0].numelts = par1[j].numelts;
							temp[0].next = (INT_S *)CALLOC(par1[j].numelts, sizeof(INT_S));
							memcpy(temp[0].next,  par1[j].next, par1[j].numelts * sizeof(INT_S));
							temp[1].numelts = s_l1;
							temp[1].next = (INT_S *)CALLOC(s_l1, sizeof(INT_S));
							memcpy(temp[1].next, l1, s_l1 * sizeof(INT_S));
							temp[2].numelts = s_l3;
							temp[2].next = (INT_S *)CALLOC(s_l3, sizeof(INT_S));
							memcpy(temp[2].next, l3, s_l3 * sizeof(INT_S));

							// Update the partition
							par1 = (part_node*)REALLOC(par1, (s_par1 + 2) * sizeof(part_node));
							if(par1 == NULL)
							{
								mem_result = 1;
								goto CANLABEL1;
							}
							free(par1[j].next);
							par1[j].numelts = 0;
							par1[j].next = NULL;
							par1[s_par1].numelts = s_l1;
							par1[s_par1].next = (INT_S *)CALLOC(s_l1, sizeof(INT_S));
							memcpy(par1[s_par1].next, l1, s_l1 * sizeof(INT_S));
							s_par1 ++;
							par1[s_par1].numelts = s_l3;
							par1[s_par1].next = (INT_S *)CALLOC(s_l3, sizeof(INT_S));
							memcpy(par1[s_par1].next, l3, s_l3 * sizeof(INT_S));
							s_par1 ++;
						} else if(f3 == 0){
							//Update the splitter
							temp[0].numelts = par1[j].numelts;
							temp[0].next = (INT_S *)CALLOC(par1[j].numelts, sizeof(INT_S));
							memcpy(temp[0].next,  par1[j].next, par1[j].numelts * sizeof(INT_S));
							temp[1].numelts = s_l1;
							temp[1].next = (INT_S *)CALLOC(s_l1, sizeof(INT_S));
							memcpy(temp[1].next, l1, s_l1 * sizeof(INT_S));
							temp[2].numelts = s_l2;
							temp[2].next = (INT_S *)CALLOC(s_l2, sizeof(INT_S));
							memcpy(temp[2].next, l2, s_l2 * sizeof(INT_S));

							// Update the partition
							par1 = (part_node*)REALLOC(par1, (s_par1 + 2) * sizeof(part_node));
							if(par1 == NULL)
							{
								mem_result = 1;
								goto CANLABEL1;
							}
							free(par1[j].next);
							par1[j].numelts = 0;
							par1[j].next = NULL;
							par1[s_par1].numelts = s_l1;
							par1[s_par1].next = (INT_S *)CALLOC(s_l1, sizeof(INT_S));
							memcpy(par1[s_par1].next, l1, s_l1 * sizeof(INT_S));
							s_par1 ++;
							par1[s_par1].numelts = s_l2;
							par1[s_par1].next = (INT_S *)CALLOC(s_l2, sizeof(INT_S));
							memcpy(par1[s_par1].next, l2, s_l2 * sizeof(INT_S));
							s_par1 ++;
						}
					}
						 
					free(l1); free(l2); free(l3);
					s_l1 = s_l2 = s_l3 = 0;
					l1 = l2 = l3 = NULL;

				}
				free(lA); free(lB);
				s_lA = s_lB = 0;
				lA = lB = NULL;
			}
			free(list);
			s_list = 0; list = NULL;

		}
		split_index ++;
	}

	New_Arrange(&s_par3, &par3, &s_par1, &par1, s2);

	//zprint_ext_par(s_ext_par2, ext_par2, imagelist2);
	//zprint_par(s_par3, par3);

	s5 = s_par3;
	t5 = newdes(s5);   
	if ((s5 != 0) && (t5 == NULL))
	{
		result = -1;
		goto CANLABEL1;
	}    

	/* Create statemap and remap list from (par3, s_par3) */  
	*s_statemap = s2;
	*statemap = (INT_S*) MALLOC((*s_statemap) * sizeof(INT_S));
	if ((*s_statemap != 0) && (*statemap == NULL))
	{
		result = -1;
		goto CANLABEL1;
	}      

	s_remap = s2;
	remap = (INT_S*) MALLOC(s_remap * sizeof(INT_S)); 
	if ((s_remap !=0) && (remap == NULL))
	{
		result = -1;
		goto CANLABEL1;
	}   

	for (i=0; i < s_par3; i++) {
		for (k=0; k < par3[i].numelts; k++) {
			state = par3[i].next[k];
			(*statemap)[state] = par3[i].next[0]; 
			remap[state] = i;
		}   
	}    

	for (i=0; i < s2; i++) {
		ii = remap[i];
		for (j=0; j < t2[i].numelts; j++) {
			ee = t2[i].next[j].data1;
			jj = (INT_T)remap[ t2[i].next[j].data2 ];          

			if ((ii == jj) && (ee == 1001)) {  /* "m" = 1001 */
				t5[ii].marked = true;
			} else if ((ii == jj) && (ee == EEE)) { /* "e" = 1000 */
				/* Remove selfloop for non-observable events only */
			} else {   
				addordlist1(ee, jj, &t5[ii].next, t5[ii].numelts, &ok);
				if (ok) t5[ii].numelts++;
			}   
		}    
	}     

	/* Step 12 */
	init = 0L;
	filedes(name2, s5, init, t5);               

CANLABEL1:   
	freedes(s1, &t1);
	freedes(s2, &t2); 
	freedes(s3, &t3);
	freedes(s4, &t4);
	freedes(s5, &t5);

	free_par(&s_par1, &par1);  
	free_par(&s_par2, &par2);
	free_par(&s_par3, &par3); 

	free_ext_part(s_ext_par1, &ext_par1);
	free_ext_part(s_ext_par2, &ext_par2);

	//free(remap);
	free(imagelist2);

	return result;
}

INT_OS CanQC_proc2(char *name2, char *name1,
	INT_T s_nulllist, INT_T *nulllist,
	INT_T s_imagelist, INT_T *imagelist,
	INT_S *s_statemap, INT_S **statemap,
	INT_OS mode)
{
	INT_S s1, s2, s3, s4, s5, init;
	state_node *t1, *t2, *t3, *t4, *t5;    
	INT_B  ok;
	INT_S i, k, l, state, ii, count;
	INT_T j, jj, ee;
	INT_OS result, f1, f2, f3;
	INT_S split_index;

	INT_S s_par1, s_par2, s_par3, s_par4, num;
	part_node *par1, *par2, *par3, *temp, *spliter;

	INT_S s_ext_par1, s_ext_par2, s_ext_par3;
	ext_part_node *ext_par1, *ext_par2, *ext_par3;

	INT_S s_remap;
	INT_S *remap;

	INT_T s_imagelist2, *imagelist2;
	INT_S s_list, *list, s_list1, *list1, s_lA, s_lB, *lA, *lB, s_l1, s_l2, s_l3, *l1, *l2, *l3;

	result = 0;

	s1 = s2 = s3 = s4 = s5 = init = 0;
	t1 = t2 = t3 = t4 = t5 = NULL;

	s_imagelist2 = 0;
	imagelist2 = NULL;

	s_par1 = s_par2 = s_par3 = s_par4 = 0;
	par1 = par2 = par3 = NULL;

	s_ext_par1 = s_ext_par2 = s_ext_par3 = 0;
	ext_par1 = ext_par2 = ext_par3 = NULL;

	s_list = s_list1 = s_lA = s_lB = 0; 
	list = list1 = lA = lB = NULL;

	init = 0L;
	if (getdes(name1, &s1, &init, &t1) == false) {
		return -1;
	}  

	/* NULL */
	if (s1 == 0)
	{
		init = 0L;
		filedes(name2, s1, init, t1);
		goto CANLABEL1;
	}   

	/* Look for case everything is NULL */
	if (s_imagelist == 0) 
	{
		s2 = 1;
		t2 = newdes(s2);

		t2[0].marked = t1[0].marked;  

		init = 0L;
		filedes(name2, s2, init, t2);
		goto CANLABEL1;
	}  

	//melt_e_p(s1, t1, &s3, &t3, s_imagelist, imagelist);

	s2 = s1;         
	t2 = newdes(s2);
	if ((s2 != 0) && (t2 == NULL))
	{
		result = -1;
		goto CANLABEL1;
	}            

	/* Step 3 */          
	/* Create t2 with partition states and mark NULL event as "e" = 1000 */
	for (i=0; i < s1; i++) {
		t2[i].marked = t1[i].marked;
		t2[i].vocal  = t1[i].vocal;
		for (j=0; j < t1[i].numelts; j++) {
			ee = t1[i].next[j].data1;
			jj = (INT_T)t1[i].next[j].data2;          
			if (inlist(ee, nulllist, s_nulllist))
				ee = EEE;  /* "e" = 1000 */

			addordlist1(ee, jj, &t2[i].next, t2[i].numelts, &ok);
			if (ok) t2[i].numelts++;
		}    

		/* Add selfloop to marked states with label "m" = 1001 */
		if (t1[i].marked)
		{
			addordlist1(1001, i, &t2[i].next, t2[i].numelts, &ok);
			if (ok) t2[i].numelts++;
		}   
	}     

	/* Can delete t1 at this point */
	freedes(s1, &t1);
	s1 = 0;
	t1 = NULL;

	/* Step 4 */
	s_par2 = 2;
	par2 = (part_node*) CALLOC(s_par2, sizeof(part_node));
	if (par2 == NULL) {
		result = -1;
		goto CANLABEL1;
	}

	s_par3 = 2;
	par3 = (part_node*) CALLOC(s_par3, sizeof(part_node));
	if (par3 == NULL) {
		result = -1;
		goto CANLABEL1;
	}        

	/* Construct Alph' = Image U {m} */
	for (j=0; j < s_imagelist; j++)
	{
		addordlist(imagelist[j], &imagelist2, s_imagelist2, &ok);
		if (ok) s_imagelist2++;
	}    
	addordlist(1001, &imagelist2, s_imagelist2, &ok);   /* "m" = 1001 */
	if (ok) s_imagelist2++;

	if (mode == 2){
		addordlist(EEE, &imagelist2, s_imagelist2, &ok);   /* "e" = 1000 */
		if(ok) s_imagelist2 ++;
	}

	eta(s2, t2, &s3, &t3, s_imagelist2, imagelist2);

	// Store transition list into ext_par
	eta_to_invtran(s3, t3, &s_ext_par1, &ext_par1, s_imagelist2, imagelist2);
	eta_to_tran(s3, t3, &s_ext_par3, &ext_par3, s_imagelist2, imagelist2);

	//	zprint_ext_par(s_ext_par1, ext_par1, imagelist2);

	// Initialize the partition and the splitters
	s_list = s3;
	list = (INT_S *)CALLOC(s_list, sizeof(INT_S));
	for(i = 0; i < s3; i ++){
		list[i] = i;
	}

	s_par1 = 1;
	par1 = (part_node *)CALLOC(s_par1, sizeof(part_node));
	if(par1 == NULL){
		mem_result = 1;
		goto CANLABEL1;
	}
	par1[0].numelts = s_list;
	par1[0].next = (INT_S*)CALLOC(s_list, sizeof(INT_S));
	memcpy(par1[0].next, list, s_list * sizeof(INT_S));

	s_ext_par2 = 1;
	ext_par2 = (ext_part_node*)CALLOC(1, sizeof(ext_part_node));
	if(ext_par2 == NULL){
		mem_result = 1;
		goto CANLABEL1;
	}
	ext_par2[0].numelts = 1;
	ext_par2[0].data = (part_node*)CALLOC(1, sizeof(part_node));
	temp = ext_par2[0].data;
	temp[0].numelts = s_list;
	temp[0].next = (INT_S*)CALLOC(s_list, sizeof(INT_S));
	memcpy(temp[0].next, list, s_list * sizeof(INT_S));

	free(list); list = NULL; s_list = 0;

	split_index = 0;
	while(split_index < s_ext_par2){	
		spliter = ext_par2[split_index].data;
		//num_par = s_par1;
		if(ext_par2[split_index].numelts == 1){
			for(i = 0; i < s_imagelist2; i ++){
				// Compute Ta^{-1}B
				if(compute_trans(spliter[0], i, ext_par1, &s_list, &list) == 0)
					continue;
				num = s_par1;
				for(j = 0; j < num; j ++){
					if(par1[j].numelts <= 1)
						continue;
					s_l1 = s_l2 = 0;
					l1 = l2 = NULL;
					for(k = 0; k < par1[j].numelts; k ++){	
						state = par1[j].next[k];
						if(instatelist(state, list, s_list)){
							addstatelist(state, &l1, s_l1, &ok);
							if(ok) s_l1 ++;
						}else{
							addstatelist(state, &l2, s_l2, &ok);
							if(ok) s_l2 ++;
						}
					}
					if(s_l1 == 0 || s_l2 == 0){
						continue;
/*						ext_par2 = (ext_part_node *)REALLOC(ext_par2, (s_ext_par2 + 1)* sizeof(ext_part_node));
						if(ext_par2 == NULL){
							mem_result = 1;
							goto CANLABEL1;
						}
						ext_par2[s_ext_par2].numelts = 1;
						ext_par2[s_ext_par2].data = (part_node*)CALLOC(1, sizeof(part_node));
						temp = ext_par2[s_ext_par2].data;
						temp[0].numelts = s_l2;
						temp[0].next = (INT_S *)CALLOC(s_l2, sizeof(INT_S));
						memcpy(temp[0].next, l2, s_l2 * sizeof(INT_S));

						s_ext_par2 ++;

					}else if (s_l2 == 0){
						ext_par2 = (ext_part_node *)REALLOC(ext_par2, (s_ext_par2 + 1)* sizeof(ext_part_node));
						if(ext_par2 == NULL){
							mem_result = 1;
							goto CANLABEL1;
						}
						ext_par2[s_ext_par2].numelts = 1;
						ext_par2[s_ext_par2].data = (part_node*)CALLOC(1, sizeof(part_node));
						temp = ext_par2[s_ext_par2].data;
						temp[0].numelts = s_l1;
						temp[0].next = (INT_S *)CALLOC(s_l1, sizeof(INT_S));
						memcpy(temp[0].next, l1, s_l1 * sizeof(INT_S));

						s_ext_par2 ++;*/
					}else{ 
						//Update the splitter
						/*//equal_flag = false;
						for(l = split_index + 1; l < s_ext_par2; l ++){
							if(ext_par2[l].numelts != 1)
								continue;
							else if(ext_par2[l].data[0].numelts != s_l2)
								continue;
							else if(qc_equal_list(ext_par2[l].data[0].next, par1[j].next, par1[j].numelts)){
								equal_flag = true;
								break;
							}
						}*/
						ext_par2 = (ext_part_node *)REALLOC(ext_par2, (s_ext_par2 + 1)* sizeof(ext_part_node));
						if(ext_par2 == NULL){
							mem_result = 1;
							goto CANLABEL1;
						}
						/*if(equal_flag){
							ext_par2[s_ext_par2].numelts = 1;
							ext_par2[s_ext_par2].data = (part_node*)CALLOC(1, sizeof(part_node));
							temp = ext_par2[s_ext_par2].data;
							temp[0].numelts = s_l1;
							temp[0].next = (INT_S *)CALLOC(s_l1, sizeof(INT_S));
							memcpy(temp[0].next, l1, s_l1 * sizeof(INT_S));

						}else{*/
							ext_par2[s_ext_par2].numelts = 2;
							ext_par2[s_ext_par2].data = (part_node*)CALLOC(2, sizeof(part_node));
							temp = ext_par2[s_ext_par2].data;
							temp[0].numelts = par1[j].numelts;
							temp[0].next = (INT_S *)CALLOC(par1[j].numelts, sizeof(INT_S));
							memcpy(temp[0].next,  par1[j].next, par1[j].numelts * sizeof(INT_S));
							if(s_l1 <= s_l2){
								temp[1].numelts = s_l1;
								temp[1].next = (INT_S *)CALLOC(s_l1, sizeof(INT_S));
								memcpy(temp[1].next, l1, s_l1 * sizeof(INT_S));
							}else{
								temp[1].numelts = s_l2;
								temp[1].next = (INT_S *)CALLOC(s_l2, sizeof(INT_S));
								memcpy(temp[1].next, l2, s_l2 * sizeof(INT_S));
							}
						//}
						s_ext_par2 ++;

						// Update the partition						
						par1 = (part_node*)REALLOC(par1, (s_par1 + 1) * sizeof(part_node));
						if(par1 == NULL)
						{
							mem_result = 1;
							goto CANLABEL1;
						}
						//free(par1[j].next);
						//par1[j].numelts = 0; par1[j].next = NULL;

						par1[j].numelts = s_l2;
						par1[j].next = (INT_S *)REALLOC(par1[j].next, s_l2 * sizeof(INT_S));
						memcpy(par1[j].next, l2, s_l2 * sizeof(INT_S));

						par1[s_par1].numelts = s_l1;
						par1[s_par1].next = (INT_S *)CALLOC(s_l1, sizeof(INT_S));
						memcpy(par1[s_par1].next, l1, s_l1 * sizeof(INT_S));
						s_par1 ++;
						//par1[s_par1].numelts = s_l2;
						//par1[s_par1].next = (INT_S *)CALLOC(s_l2, sizeof(INT_S));
						//memcpy(par1[s_par1].next, l2, s_l2 * sizeof(INT_S)); 
						//s_par1 ++;

					}
					free(l1); free(l2);
					s_l1 = s_l2 = 0;
					l1 = l2 = NULL;

				}
			}
			free(list);
			s_list = 0; list = NULL;
		}else{
			for(i = 0; i < s_imagelist2; i ++){
				// Compute Ta^{-1}B
				if(compute_trans(spliter[0], i, ext_par1, &s_list1, &list1) == 0)
					continue;
				//compute_trans(spliter[1], i, ext_par1, &s_lA, &lA);
				//compute_trans(spliter[2], i, ext_par1, &s_lB, &lB);
				num = s_par1;
				for(j = 0; j < num; j ++){
					if(par1[j].numelts <= 1)
						continue;					
					state = par1[j].next[0];
					if(!instatelist(state, list1, s_list1))
						continue;
					s_list = ext_par3[state].data[i].numelts;
					list = ext_par3[state].data[i].next;
					//if((s_list == 0) || !instatelist(list[0], spliter[0].next, spliter[0].numelts))
					//	continue;
					if(s_list == 0)
						continue;
					s_l1 = s_l2 = s_l3 = 0;
					l1 = l2 = l3 = NULL;
					f1 = f2 = f3 = 0;
					for(k = 0; k < par1[j].numelts; k ++){	
						state = par1[j].next[k];
						s_list = ext_par3[state].data[i].numelts;
						list = ext_par3[state].data[i].next;
						s_lA = spliter[1].numelts;
						lA = spliter[1].next;
						count = 0;
						for(l = 0; l < s_list; l ++){
							if(instatelist(list[l],lA, s_lA)){
								count ++;
							}
						}
						if(count == s_list){
							addstatelist(state, &l1, s_l1, &ok);
							if(ok) s_l1 ++;
							f1 = 1;
						}else if(count == 0){
							addstatelist(state, &l2, s_l2, &ok);
							if(ok) s_l2 ++;
							f2 = 1;
						}else{
							addstatelist(state, &l3, s_l3, &ok);
							if(ok) s_l3 ++;
							f3 = 1;
						}

						/*if(instatelist(state, lA, s_lA)){
							if(instatelist(state, lB, s_lB)){
								addstatelist(state, &l1, s_l1, &ok);
								if(ok) s_l1 ++;
								f1 = 1;
							}else{
								addstatelist(state, &l2, s_l2, &ok);
								if(ok) s_l2 ++;
								f2 = 1;
							}
						}else{
							addstatelist(state, &l3, s_l3, &ok);
							if(ok) s_l3 ++;
							f3 = 1;
						}*/
					}

					if(f1 + f2 + f3 == 3){
						//Update the splitter
						ext_par2 = (ext_part_node *)REALLOC(ext_par2, (s_ext_par2 + 2)* sizeof(ext_part_node));
						if(ext_par2 == NULL){
							mem_result = 1;
							goto CANLABEL1;
						}
						ext_par2[s_ext_par2].numelts = 2;
						ext_par2[s_ext_par2].data = (part_node*)CALLOC(2, sizeof(part_node));
						temp = ext_par2[s_ext_par2].data;
						temp[0].numelts = par1[j].numelts;
						temp[0].next = (INT_S *)CALLOC(par1[j].numelts, sizeof(INT_S));
						memcpy(temp[0].next,  par1[j].next, par1[j].numelts * sizeof(INT_S));
						if(s_l1 <= s_l2 + s_l3){
							temp[1].numelts = s_l1;
							temp[1].next = (INT_S *)CALLOC(s_l1, sizeof(INT_S));
							memcpy(temp[1].next, l1, s_l1 * sizeof(INT_S));

							s_ext_par2 += 1;

							ext_par2[s_ext_par2].numelts = 2;
							ext_par2[s_ext_par2].data = (part_node*)CALLOC(2, sizeof(part_node));
							temp = ext_par2[s_ext_par2].data;
							temp[0].numelts = s_l2;
							temp[0].next = (INT_S *)CALLOC(s_l2, sizeof(INT_S));
							memcpy(temp[0].next, l2, s_l2 * sizeof(INT_S));
							for(k = 0; k < s_l3; k ++){
								addstatelist(l3[k], &temp[0].next, temp[0].numelts, &ok);
								if(ok) temp[0].numelts ++;
							}
							if(s_l2 <= s_l3){
								temp[1].numelts = s_l2;
								temp[1].next = (INT_S *)CALLOC(s_l2, sizeof(INT_S));
								memcpy(temp[1].next, l2, s_l2 * sizeof(INT_S));
							}else{
								temp[1].numelts = s_l3;
								temp[1].next = (INT_S *)CALLOC(s_l3, sizeof(INT_S));
								memcpy(temp[1].next, l3, s_l3 * sizeof(INT_S));
							}
							s_ext_par2 += 1;
						} else {
							temp[1].numelts = s_l2;
							temp[1].next = (INT_S *)CALLOC(s_l2, sizeof(INT_S));
							memcpy(temp[1].next, l2, s_l2 * sizeof(INT_S));

							s_ext_par2 += 1;

							ext_par2[s_ext_par2].numelts = 2;
							ext_par2[s_ext_par2].data = (part_node*)CALLOC(2, sizeof(part_node));
							temp = ext_par2[s_ext_par2].data;
							temp[0].numelts = s_l1;
							temp[0].next = (INT_S *)CALLOC(s_l1, sizeof(INT_S));
							memcpy(temp[0].next, l1, s_l1 * sizeof(INT_S));
							for(k = 0; k < s_l3; k ++){
								addstatelist(l3[k], &temp[0].next, temp[0].numelts, &ok);
								if(ok) temp[0].numelts ++;
							}
							if(s_l1 <= s_l3){
								temp[1].numelts = s_l1;
								temp[1].next = (INT_S *)CALLOC(s_l1, sizeof(INT_S));
								memcpy(temp[1].next, l1, s_l1 * sizeof(INT_S));
							}else{
								temp[1].numelts = s_l3;
								temp[1].next = (INT_S *)CALLOC(s_l3, sizeof(INT_S));
								memcpy(temp[1].next, l3, s_l3 * sizeof(INT_S));
							}
							s_ext_par2 += 1;
						}

						// Update the partition
						par1 = (part_node*)REALLOC(par1, (s_par1 + 2) * sizeof(part_node));
						if(par1 == NULL)
						{
							mem_result = 1;
							goto CANLABEL1;
						}
						//free(par1[j].next);
						//par1[j].numelts = 0;
						//par1[j].next = NULL;
						par1[j].numelts = s_l1;
						par1[j].next = (INT_S *)REALLOC(par1[j].next, s_l1* sizeof(INT_S));
						memcpy(par1[j].next, l1, s_l1 * sizeof(INT_S));
						//s_par1 ++;
						par1[s_par1].numelts = s_l2;
						par1[s_par1].next = (INT_S *)CALLOC(s_l2, sizeof(INT_S));
						memcpy(par1[s_par1].next, l2, s_l2 * sizeof(INT_S));
						s_par1 ++;
						par1[s_par1].numelts = s_l3;
						par1[s_par1].next = (INT_S *)CALLOC(s_l3, sizeof(INT_S));
						memcpy(par1[s_par1].next, l3, s_l3 * sizeof(INT_S));
						s_par1 ++;

					}else if(f1 + f2 + f3 == 2){
						ext_par2 = (ext_part_node *)REALLOC(ext_par2, (s_ext_par2 + 1)* sizeof(ext_part_node));
						if(ext_par2 == NULL){
							mem_result = 1;
							goto CANLABEL1;
						}
						ext_par2[s_ext_par2].numelts = 2;
						ext_par2[s_ext_par2].data = (part_node*)CALLOC(2, sizeof(part_node));
						temp = ext_par2[s_ext_par2].data;
						temp[0].numelts = par1[j].numelts;
						temp[0].next = (INT_S *)CALLOC(par1[j].numelts, sizeof(INT_S));
						memcpy(temp[0].next,  par1[j].next, par1[j].numelts * sizeof(INT_S));
						s_ext_par2 += 1;
						if(f1 == 0){
							//Update the splitter
							if(s_l2 <= s_l3){
								temp[1].numelts = s_l2;
								temp[1].next = (INT_S *)CALLOC(s_l2, sizeof(INT_S));
								memcpy(temp[1].next, l2, s_l2 * sizeof(INT_S));
							}else{
								temp[1].numelts = s_l3;
								temp[1].next = (INT_S *)CALLOC(s_l3, sizeof(INT_S));
								memcpy(temp[1].next, l3, s_l3 * sizeof(INT_S));
							}

							// Update the partition
							par1 = (part_node*)REALLOC(par1, (s_par1 + 1) * sizeof(part_node));
							if(par1 == NULL)
							{
								mem_result = 1;
								goto CANLABEL1;
							}
							//free(par1[j].next);
							//par1[j].numelts = 0;
							//par1[j].next = NULL;
							par1[j].numelts = s_l2;
							par1[j].next = (INT_S *)REALLOC(par1[j].next, s_l2* sizeof(INT_S));
							memcpy(par1[j].next, l2, s_l2 * sizeof(INT_S));
							//s_par1 ++;
							par1[s_par1].numelts = s_l3;
							par1[s_par1].next = (INT_S *)CALLOC(s_l3, sizeof(INT_S));
							memcpy(par1[s_par1].next, l3, s_l3 * sizeof(INT_S));
							s_par1 ++;
						}else if (f2 == 0){
							//Update the splitter
							if(s_l1 <= s_l3){
								temp[1].numelts = s_l1;
								temp[1].next = (INT_S *)CALLOC(s_l1, sizeof(INT_S));
								memcpy(temp[1].next, l1, s_l1 * sizeof(INT_S));
							}else{
								temp[1].numelts = s_l3;
								temp[1].next = (INT_S *)CALLOC(s_l3, sizeof(INT_S));
								memcpy(temp[1].next, l3, s_l3 * sizeof(INT_S));
							}

							// Update the partition
							par1 = (part_node*)REALLOC(par1, (s_par1 + 1) * sizeof(part_node));
							if(par1 == NULL)
							{
								mem_result = 1;
								goto CANLABEL1;
							}
							//free(par1[j].next);
							//par1[j].numelts = 0;
							//par1[j].next = NULL;
							par1[j].numelts = s_l1;
							par1[j].next = (INT_S *)REALLOC(par1[j].next, s_l1 * sizeof(INT_S));
							memcpy(par1[j].next, l1, s_l1 * sizeof(INT_S));
							//s_par1 ++;
							par1[s_par1].numelts = s_l3;
							par1[s_par1].next = (INT_S *)CALLOC(s_l3, sizeof(INT_S));
							memcpy(par1[s_par1].next, l3, s_l3 * sizeof(INT_S));
							s_par1 ++;
						} else{
							//Update the splitter
							if(s_l1 <= s_l2){
								temp[1].numelts = s_l1;
								temp[1].next = (INT_S *)CALLOC(s_l1, sizeof(INT_S));
								memcpy(temp[1].next, l1, s_l1 * sizeof(INT_S));
							}else{
								temp[1].numelts = s_l2;
								temp[1].next = (INT_S *)CALLOC(s_l2, sizeof(INT_S));
								memcpy(temp[1].next, l2, s_l2 * sizeof(INT_S));
							}

							// Update the partition
							par1 = (part_node*)REALLOC(par1, (s_par1 + 1) * sizeof(part_node));
							if(par1 == NULL)
							{
								mem_result = 1;
								goto CANLABEL1;
							}
							//free(par1[j].next);
							//par1[j].numelts = 0;
							//par1[j].next = NULL;
							par1[j].numelts = s_l1;
							par1[j].next = (INT_S *)REALLOC(par1[j].next, s_l1*sizeof(INT_S));
							memcpy(par1[j].next, l1, s_l1 * sizeof(INT_S));
							//s_par1 ++;
							par1[s_par1].numelts = s_l2;
							par1[s_par1].next = (INT_S *)CALLOC(s_l2, sizeof(INT_S));
							memcpy(par1[s_par1].next, l2, s_l2 * sizeof(INT_S));
							s_par1 ++;
						}
					}

					free(l1); free(l2); free(l3);
					s_l1 = s_l2 = s_l3 = 0;
					l1 = l2 = l3 = NULL;

				}
			//	free(lA); free(lB);
			//	s_lA = s_lB = 0;
			//	lA = lB = NULL;
				free(list1);
				list1 = NULL; s_list1 = 0;
			}
			//free(list);
			//s_list = 0; list = NULL;

		}
		free_par(&ext_par2[split_index].numelts, &ext_par2[split_index].data);
		split_index ++;
	}
	//zprintn(s_ext_par2);
	//zprint_ext_par(s_ext_par2,ext_par2,imagelist2);
	//zprint_par(s_par1, par1);

	New_Arrange(&s_par3, &par3, &s_par1, &par1, s2);

	s5 = s_par3;
	t5 = newdes(s5);   
	if ((s5 != 0) && (t5 == NULL))
	{
		result = -1;
		goto CANLABEL1;
	}    

	/* Create statemap and remap list from (par3, s_par3) */  
	*s_statemap = s2;
	*statemap = (INT_S*) MALLOC((*s_statemap) * sizeof(INT_S));
	if ((*s_statemap != 0) && (*statemap == NULL))
	{
		result = -1;
		goto CANLABEL1;
	}      

	s_remap = s2;
	remap = (INT_S*) MALLOC(s_remap * sizeof(INT_S)); 
	if ((s_remap !=0) && (remap == NULL))
	{
		result = -1;
		goto CANLABEL1;
	}   

	for (i=0; i < s_par3; i++) {
		for (k=0; k < par3[i].numelts; k++) {
			state = par3[i].next[k];
			(*statemap)[state] = par3[i].next[0]; 
			remap[state] = i;
		}   
	}    

	for (i=0; i < s2; i++) {
		ii = remap[i];
		for (j=0; j < t2[i].numelts; j++) {
			ee = t2[i].next[j].data1;
			jj = (INT_T)remap[ t2[i].next[j].data2 ];          

			if ((ii == jj) && (ee == 1001)) { 
				t5[ii].marked = true;
			} else if ((ii == jj) && (ee == EEE)) { 
				
			} else {   
				addordlist1(ee, jj, &t5[ii].next, t5[ii].numelts, &ok);
				if (ok) t5[ii].numelts++;
			}   
		}    
	}     

	/* Step 12 */
	init = 0L;
	filedes(name2, s5, init, t5);               

CANLABEL1:   
	freedes(s1, &t1);
	freedes(s2, &t2); 
	freedes(s3, &t3);
	freedes(s4, &t4);
	freedes(s5, &t5);

	free_par(&s_par1, &par1);  
	free_par(&s_par2, &par2);
	free_par(&s_par3, &par3); 

	free_ext_part(s_ext_par1, &ext_par1);
	free_ext_part(s_ext_par2, &ext_par2);
	free_ext_part(s_ext_par3, &ext_par3);

	//free(remap);
	free(imagelist2);

	return result;
}

