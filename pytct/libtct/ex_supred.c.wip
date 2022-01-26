//In this version, we move data requiring large memories in the old version to the hard disk files.
// Then it intend to compute a larger supervisor localization


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <io.h>
#include <tct_io.h>
// #include <windows.h>
// #include <direct.h>

#include "des_data.h"
#include "ex_supred.h"
#include "mymalloc.h"
#include "supred.h"
#include "math.h"
#include "setup.h"
#include "des_supp.h"
#include "supred2.h"


#ifdef __cplusplus
extern "C" {
#endif

#define TmpData "TmpData"
#define equal_set_dat "TmpData\\equal_set.dat"
#define control_tree_dat "TmpData\\control_tree.dat"
#define waitlist_dat "TmpData\\waitlist"
#define mergetable_dat "TmpData\\merge_table"
#define sim_control_dat "TmpData\\simple_controller.dat"
#define bit_num 10    //a string with 8 character can represent any number smaller than 10 billion

// static char signature[8] = {"Z8^0L;1"};
// static INT_OS  signature_length = 7;
// static INT_OS endian = 0xFF00AA55;
INT_T *forcible_event;

INT_S *controller,*controller_tree, *simpler_controller, *plant,*c_marked_states,*p_marked_states;
char **merging_table;

FILE *output;
HANDLE *hWaitlist, hMapWaitlist;
char *pvWaitlist;
INT_S nFileSize_limit;
INT_S nWaitlist_limit;
INT_S nTmpuStack_limit;
INT_S nCurIndex, nViewIndex = 0, nFileIndex = 0; 
INT_S nLengthWaitlist;

//int *record;

//int data_length = 32;


INT_S *tmpu_stack;

INT_S tran_number, num_states;

typedef struct equivalent_state_set equivalent_state_set;
typedef struct transitions transitions;
struct node *root_node;
struct virtual_stack{
       INT_S state_1;
       INT_S state_2;
       INT_S node_1,node_2,c_flag, flag, r_flag_1,r_flag_2;
       INT_S tmpu_point_1,tmpu_point_2;
       INT_S record_point_1, record_point_2;
       struct equivalent_state_set *temp6, *temp7;
       struct transitions *temp3, *temp4;
       INT_S tmp_state_3, tmp_state_4;
       INT_S tmp_state_1, tmp_state_2;
       struct virtual_stack *last_state_pair;
       struct virtual_stack *next_state_pair;
} *stack;
/////////////////////////////////
// stack for storing transitions
typedef struct env_transitions{
	transitions *data1;
	transitions *data2;
}env_transitions;
typedef struct tran_stack {
	env_transitions* head;
	INT_S      head_size;
	INT_S      pstack_size;
} tran_stack;

/* Stack routine */

INT_B  tran_stack_Push(tran_stack *ts,
	transitions *value1, transitions *value2)
{
	if (ts->head_size >= ts->pstack_size) {
		ts->head = (env_transitions*) REALLOC(ts->head,
			sizeof(env_transitions)*(ts->pstack_size+512L));
		if (ts->head == NULL) {
			mem_result = 1;
			return false;
		}
		ts->pstack_size +=512L;
	}
	ts->head[ts->head_size].data1 = value1;
	ts->head[ts->head_size].data2 = value2;
	ts->head_size++;
	return true;
}

void tran_stack_Pop(tran_stack *ts,
	transitions **value1,
	transitions **value2,
	INT_B *ok)
{
	if (ts->head_size == 0L) {
		*ok = false;
		return;
	}

	ts->head_size--;
	*value1 = (transitions*)ts->head[ts->head_size].data1;
	*value2 = (transitions*)ts->head[ts->head_size].data2;
	*ok = true;
}

INT_B  tran_stack_IsEmpty(tran_stack *ts)
{
	if (ts->head_size == 0L)
		return true;
	else
		return false;
}

void tran_stack_Init(tran_stack *ts)
{
	ts->head_size   = 0L;
	ts->pstack_size = 0L;
	ts->head = NULL;
}

void tran_stack_Done(tran_stack *ts)
{
	free(ts->head);
	ts->head_size   = 0L;
	ts->pstack_size = 0L;
	ts->head        = NULL;
}
//////////////////////////////
//stack for storing equivalent_state_set
typedef struct env_equivalent_state_set{
	equivalent_state_set * data1;
    equivalent_state_set *data2;
}env_equivalent_state_set;
typedef struct ess_stack {
	env_equivalent_state_set* head;
	INT_S      head_size;
	INT_S      pstack_size;
} ess_stack;

/* Stack routine */

INT_B  ess_stack_Push(ess_stack *ts,
	equivalent_state_set *value1, equivalent_state_set *value2)
{
	if (ts->head_size >= ts->pstack_size) {
		ts->head = (env_equivalent_state_set*) REALLOC(ts->head,
			sizeof(env_equivalent_state_set)*(ts->pstack_size+512L));
		if (ts->head == NULL) {
			mem_result = 1;
			return false;
		}
		ts->pstack_size +=512L;
	}
	ts->head[ts->head_size].data1 = value1;
	ts->head[ts->head_size].data2 = value2;
	ts->head_size++;
	return true;
}

void ess_stack_Pop(ess_stack *ts,
	equivalent_state_set **value1,
	equivalent_state_set **value2,
	INT_B *ok)
{
	if (ts->head_size == 0L) {
		*ok = false;
		return;
	}

	ts->head_size--;
	*value1 = (equivalent_state_set*)ts->head[ts->head_size].data1;
	*value2 = (equivalent_state_set*)ts->head[ts->head_size].data2;
	*ok = true;
}

INT_B  ess_stack_IsEmpty(ess_stack *ts)
{
	if (ts->head_size == 0L)
		return true;
	else
		return false;
}

void ess_stack_Init(ess_stack *ts)
{
	ts->head_size   = 0L;
	ts->pstack_size = 0L;
	ts->head = NULL;
}

void ess_stack_Done(ess_stack *ts)
{
	free(ts->head);
	ts->head_size   = 0L;
	ts->pstack_size = 0L;
	ts->head        = NULL;
}

FILE *out;

extern INT_OS Get_DES(INT_S *, INT_S *, INT_OS, char*);
extern INT_B Forbidden_Event(char*);
extern void Final_Result();
extern INT_OS Txt_DES(INT_S);
extern void Controller_Tree();
extern INT_B Combined_Tree();
extern void Tree_Structure_Conversion(char*);
extern INT_OS Selfloop_Node(INT_S, INT_S, INT_S, INT_OS);
extern void Reduction();
extern INT_S Refinement();

void SetMapView(INT_S nIndex)
{
    INT_S nTemp, i;
    DWORD nLength;
    char *tmpBuf;
    
    nViewIndex = nIndex;
    UnmapViewOfFile(pvWaitlist);
    CloseHandle(hMapWaitlist);
    if(nViewIndex > nFileIndex){
       nTemp = nFileIndex + 1;
       nFileIndex = nViewIndex;
       tmpBuf = (char*)CALLOC(nFileSize_limit, 1);
       for(i = nTemp; i < nFileIndex + 1; i ++){
          WriteFile(hWaitlist[i], tmpBuf, (DWORD)nFileSize_limit, &nLength, NULL);
       }
       free(tmpBuf);
    }
    hMapWaitlist = CreateFileMapping(hWaitlist[nViewIndex], NULL, PAGE_READWRITE, 0, 
                                      GetFileSize(hWaitlist[nViewIndex],0), NULL);
    pvWaitlist = (char*)MapViewOfFile(hMapWaitlist, FILE_MAP_WRITE, 0, 0, 0);
}

void Ex_father_node_in_controller_tree(INT_S base_point)
{
	INT_S relative_point,son_base_point,son_node,no_of_son_nodes;
	/* modification starts here */
	/* assign father-node-number to each node in controller_tree */
	sp_stack sp;
	INT_B ok;

	sp_stack_Init(&sp);     
	//stack_Push(&sq, base_point);

START:
	// point = dequeue(&tq);
	//stack_Pop(&sq, &point,&ok);
	relative_point = 0;
	for(relative_point=0;relative_point<*(controller_tree+base_point+14);relative_point++)
	{
		son_node = *(controller_tree+base_point+2*relative_point+16);
		son_base_point = 0;
		while(*(controller_tree+son_base_point)!=son_node && *(controller_tree+son_base_point)!=-1)
		{
			no_of_son_nodes = *(controller_tree+son_base_point+14);
			son_base_point +=15+2*no_of_son_nodes;
		}
		if(*(controller_tree+son_base_point)==-1)
		{
			/*cout << "something wrong 20, program halt"; */
			/*return 1; */
		}
		sp_stack_Push(&sp, relative_point, base_point);
		if(*(controller_tree+son_base_point+13)==-1 && *(controller_tree+son_base_point)!=0)
		{
			*(controller_tree+son_base_point+13) = *(controller_tree + base_point);			
			base_point = son_base_point;
			goto START;
		}
CONTINUE:
		sp_stack_Pop(&sp,&relative_point, &base_point, &ok);
	}
// 
// 	for(relative_point= point + 1;relative_point<*(controller_tree+base_point+14);relative_point++)
// 	{
// 		son_node = *(controller_tree+base_point+2*relative_point+16);
// 		son_base_point = 0;
// 		while(*(controller_tree+son_base_point)!=son_node && *(controller_tree+son_base_point)!=-1)
// 		{
// 			no_of_son_nodes = *(controller_tree+son_base_point+14);
// 			son_base_point +=15+2*no_of_son_nodes;
// 		}
// 		if(*(controller_tree+son_base_point)==-1)
// 		{
// 			/*cout << "something wrong 20, program halt"; */
// 			/*return 1; */
// 		}
// 		if(*(controller_tree+son_base_point+13)==-1 && *(controller_tree+son_base_point)!=0)
// 		{
// 			*(controller_tree+son_base_point+13) = *(controller_tree + base_point);
// 			sp_stack_Push(&sp, relative_point, base_point);
// 			base_point = son_base_point;
// 			goto START;
// 		}
// 	}
	if(!sp_stack_IsEmpty(&sp))
		goto CONTINUE;

	sp_stack_Done(&sp);
	/* end of modification */
}
void Ex_Controller_Tree()
{
	INT_S index,tran_number=0,trace_father_node=0,base_point=0,relative_point=0;
	INT_S stno_in_controller,stno_in_plant=-1,forbidden_event=-1;
	INT_S father_node=-1,no_of_son_nodes=0,forbidden_index;
	index=*controller;
	while(index!=-1)
	{
		stno_in_controller=*(controller+3*tran_number);
		/*the following is to find the father_node.*/
		if(stno_in_controller!=0)
		{
			while(*(controller+3*trace_father_node+2)!=stno_in_controller)
			{
				trace_father_node +=1;
			}
			trace_father_node=0;
		}
		/*the following is to find the number of the son_nodes and the corresponding*/
		/*branches which are connected between the current node and its son_nodes.*/
		while(*(controller+3*tran_number)==stno_in_controller)
		{
			*(controller_tree+base_point+relative_point+15)=*(controller+3*tran_number+1);
			*(controller_tree+base_point+relative_point+16)=*(controller+3*tran_number+2);
			relative_point +=2;
			no_of_son_nodes +=1;
			tran_number +=1;
		}
		*(controller_tree+base_point)=stno_in_controller;
		*(controller_tree+base_point+2)=stno_in_plant;
		for(forbidden_index=0;forbidden_index<10;forbidden_index++)
			*(controller_tree+base_point+3+forbidden_index)=forbidden_event;
		*(controller_tree+base_point+13)=father_node;
		*(controller_tree+base_point+14)=no_of_son_nodes;
		base_point +=15+2*no_of_son_nodes;
		relative_point=0;
		no_of_son_nodes=0;
		index=*(controller+3*tran_number);
	}
	*(controller_tree+base_point)=-1;

	Ex_father_node_in_controller_tree(0);

}

INT_OS Ex_Set_State_No_of_Plant(INT_S c_st_no_of_con)
{
  INT_S base_point=0,father_base_point=0,father_relative_point=0,trace_in_plant=0;
  INT_S n_base_point, n_father_base_point;
  INT_B  ok;
  //s_stack np;
  sp_stack sp;

  sp_stack_Init(&sp);
  
  //stack_Init(&np);
  //stack_Push(&np, c_st_no_of_con);
  
LOOP1:  
 // stack_Pop(&np, &c_st_no_of_con, &ok);
  base_point = 0;
  while(*(controller_tree+base_point)!=c_st_no_of_con)
  {
    base_point +=15+2*(*(controller_tree+base_point+14));
    if(*(controller_tree+base_point)==-1)
    {
      goto CONDITION;
    }
  }
  father_base_point = 0;
  while(*(controller_tree+father_base_point)!=*(controller_tree+base_point+13))
  {
    father_base_point +=15+2*(*(controller_tree+father_base_point+14));
    if(*(controller_tree+father_base_point)==-1)
    {
      goto CONDITION;
    }
  }
  sp_stack_Push(&sp, father_base_point, base_point);  //store father_base_point and base_point in current cycle
  
  if(*(controller_tree+father_base_point+2)==-1){
	  c_st_no_of_con = *(controller_tree+father_base_point);
      goto LOOP1;
  }
    //Set_State_No_of_Plant(*(controller_tree+father_base_point));
LOOP2:    
  sp_stack_Pop(&sp, &n_father_base_point, &n_base_point, &ok);
  father_relative_point = 0;
  while(*(controller_tree+n_father_base_point+16+2*father_relative_point)!=*(controller_tree+n_base_point))
  {
    father_relative_point +=1;
    if(father_relative_point > *(controller_tree+n_father_base_point+14))
    {
      goto CONDITION;
    }
  }
  trace_in_plant = 0;
  while((*(plant+3*trace_in_plant)!=*(controller_tree+n_father_base_point+2))||(*(plant+3*trace_in_plant+1)!=*(controller_tree+n_father_base_point+15+2*father_relative_point)))
  {
    trace_in_plant +=1;
    if(*(plant+3*trace_in_plant)==-1)
    {
      goto CONDITION;
    }
  }
  *(controller_tree + n_base_point+2)=*(plant+3*trace_in_plant+2);
  
CONDITION:  
  if(!sp_stack_IsEmpty(&sp))
    goto LOOP2;

 // if(!stack_IsEmpty(&np))
//	  goto LOOP1;
    
  //stack_Done(&sp);
  sp_stack_Done(&sp);
  return 0;
}

INT_OS Ex_Set_State_No_of_Plant1(INT_S c_st_no_of_con)
{
	INT_S base_point=0,father_base_point=0,father_relative_point=0,trace_in_plant=0;
	INT_S n_base_point, n_father_base_point;
	INT_B  ok;
	sp_stack sp;

	sp_stack_Init(&sp);

LOOP1:  
	base_point = 0;
	while(*(controller_tree+base_point)!=c_st_no_of_con)
	{
		base_point +=15+2*(*(controller_tree+base_point+14));
		if(*(controller_tree+base_point)==-1)
		{
			goto CONDITION;
		}
	}
	father_base_point = 0;
	while(*(controller_tree+father_base_point)!=*(controller_tree+base_point+13))
	{
		father_base_point +=15+2*(*(controller_tree+father_base_point+14));
		if(*(controller_tree+father_base_point)==-1)
		{
			goto CONDITION;
		}
	}
	sp_stack_Push(&sp, father_base_point, base_point);  //store father_base_point and base_point in current cycle

	if(*(controller_tree+father_base_point+2)==-1){
		goto LOOP1;
	}
	//Set_State_No_of_Plant(*(controller_tree+father_base_point));
LOOP2:    
	sp_stack_Pop(&sp, &n_father_base_point, &n_base_point, &ok);
	father_relative_point = 0;
	while(*(controller_tree+n_father_base_point+16+2*father_relative_point)!=*(controller_tree+n_base_point))
	{
		father_relative_point +=1;
		if(father_relative_point > *(controller_tree+n_father_base_point+14))
		{
			goto CONDITION;
		}
	}
	trace_in_plant = 0;
	while((*(plant+3*trace_in_plant)!=*(controller_tree+n_father_base_point+2))||(*(plant+3*trace_in_plant+1)!=*(controller_tree+n_father_base_point+15+2*father_relative_point)))
	{
		trace_in_plant +=1;
		if(*(plant+3*trace_in_plant)==-1)
		{
			goto CONDITION;
		}
	}
	*(controller_tree + n_base_point+2)=*(plant+3*trace_in_plant+2);

CONDITION:  
	if(!sp_stack_IsEmpty(&sp))
		goto LOOP2;

	sp_stack_Done(&sp);
	return 0;
}

/*I use this module to add any possible forbidden event at each node of controller_tree.*/
/*This kind of information is very important.*/
INT_B Ex_Combined_Tree()
{
  INT_S flag,c_st_no_of_con,base_point=0;
  *(controller_tree+2)=0;
  while(*(controller_tree+base_point)!=-1)
  {
    c_st_no_of_con=*(controller_tree+base_point);
    if(c_st_no_of_con!=0)
    {
       flag = Ex_Set_State_No_of_Plant(c_st_no_of_con);
       if(flag==1) return 1;
    }
    base_point +=15+2*(*(controller_tree+base_point+14));
  }
  return 0;
}
/*This module is part of the main module Reduction1.*/
INT_OS Ex1_Selfloop_Node(INT_S base_state,INT_S tmp_state,INT_S nodel, INT_OS indexx)
{
  INT_S node_1,node_2,c_flag, flag, r_flag_1,r_flag_2;
  INT_S tmpu_point,tmpu_point_1,tmpu_point_2,k;
  struct forbidden_event_set *temp1;
  struct transitions *temp2, *temp22;
  struct equivalent_state_set *temp6, *temp7;
  struct transitions *temp3, *temp4;
  INT_S tmp_state_3, tmp_state_4;
  INT_S tmp_state_1, tmp_state_2;
  sp_stack s_node,s_flag,s_tmpu_point,s_tmp_state;
  tran_stack s_tmp34; ess_stack s_tmp67;
  INT_B ok;
  INT_OS return_code;

  sp_stack_Init(&s_node);
  sp_stack_Init(&s_flag);
  tran_stack_Init(&s_tmp34);
  ess_stack_Init(&s_tmp67);
  sp_stack_Init(&s_tmpu_point);
  sp_stack_Init(&s_tmp_state);

  node_1=base_state;
  node_2=tmp_state;
  
//  while(1){
START:
  temp1 = 0;
  temp2 = 0;
  temp6=root_node[root_node[node_1].equal_set->state_number].equal_set;
  r_flag_1=0;
  tmpu_point_1=0;
  while(r_flag_1==0)
  {
   if(indexx == 0)
   {
    if(temp6 != NULL)
    {
        tmp_state_1 = temp6->state_number;
        temp6 = temp6->next_node;
    }
    else
    {
        tmp_state_1=-1;
        while(*(tmpu_stack+2*tmpu_point_1)!=-1)
        {
            if(*(tmpu_stack+2*tmpu_point_1)==node_1)
            {
                tmp_state_1=*(tmpu_stack+2*tmpu_point_1+1);
                tmpu_point_1 +=1;
                break;
            }
            if(*(tmpu_stack+2*tmpu_point_1+1)==node_1)
            {
                tmp_state_1=*(tmpu_stack+2*tmpu_point_1);
                tmpu_point_1 +=1;
                break;
            }
            tmpu_point_1 +=1;
        }
        if(tmp_state_1==-1) r_flag_1=1;;
    }
   }
   else
   {
        r_flag_1 = 1;
        tmp_state_1 = node_1;
   }

    if(tmp_state_1!= -1)
    {
        temp7=root_node[root_node[node_2].equal_set->state_number].equal_set;
        r_flag_2=0;
        tmpu_point_2=0;
        while(r_flag_2==0)
        {
          if(indexx == 0)
          {
           if(temp7 != NULL)
           {
                 tmp_state_2 = temp7->state_number;
                 temp7 = temp7->next_node;
           }
           else
           {
                 tmp_state_2=-1;
                 while(*(tmpu_stack+2*tmpu_point_2)!=-1)
                 {
                     if(*(tmpu_stack+2*tmpu_point_2)==node_2)
                     {
                         tmp_state_2=*(tmpu_stack+2*tmpu_point_2+1);
                         tmpu_point_2 +=1;
                         break;
                     }
                     if(*(tmpu_stack+2*tmpu_point_2+1)==node_2)
                     {
                         tmp_state_2=*(tmpu_stack+2*tmpu_point_2);
                         tmpu_point_2 +=1;
                         break;
                     }
                     tmpu_point_2 +=1;
                 }
                 if(tmp_state_2==-1) r_flag_2=1;;
           }
          }
          else
          {
             r_flag_2 = 1;
             tmp_state_2 = node_2;
          }
           if(tmp_state_2!= -1)
           {
               c_flag=0;
               if((tmp_state_1!=node_1 || tmp_state_2!=node_2) && (tmp_state_1!=node_2 || tmp_state_2!=node_1))
               {
                   if(indexx == 1)
                   {
                      if(merging_table[tmp_state_1][tmp_state_2] == 0) continue;
                      if(merging_table[tmp_state_1][tmp_state_2] == 1 && tmp_state_1 != tmp_state_2){
						  return_code = 10;
						  goto CONDITION;
                      }//return 10;
                   }
                   if(root_node[tmp_state_1].equal_set->state_number == root_node[tmp_state_2].equal_set->state_number) continue;


                 tmpu_point=0;
                 while(*(tmpu_stack+2*tmpu_point)!=-1)
                 {
                    if(*(tmpu_stack+2*tmpu_point)==tmp_state_1 && *(tmpu_stack+2*tmpu_point+1)==tmp_state_2)
                    {
                        c_flag=8;
                        break;
                    }
                    if(*(tmpu_stack+2*tmpu_point+1)==tmp_state_1 && *(tmpu_stack+2*tmpu_point)==tmp_state_2)
                    {
                        c_flag=8;
                        break;
                    }
                    tmpu_point +=1;
                 }
                 if(c_flag ==8) continue;


                 if(root_node[tmp_state_1].marked_in_plant==root_node[tmp_state_2].marked_in_plant && root_node[tmp_state_1].marked_in_controller!=root_node[tmp_state_2].marked_in_controller)
                  {  
					 return_code = 10;
					 goto CONDITION;
                  }     //  return 10;

                 for(k=1;k<3;k++)
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
                         if(temp1->event == temp2->event){
							 return_code = 10;
							 goto CONDITION;
                         }
                              //return 10;
                         temp2 = temp2->next_transition;
                     }
                     temp1 = temp1->next_event;
                   }
                 }
                 if(c_flag!=8)
                 {
                    *(tmpu_stack+2*tmpu_point)=tmp_state_1;
                    *(tmpu_stack+2*tmpu_point+1)=tmp_state_2;
                    *(tmpu_stack+2*tmpu_point+2)=-1;
                 }
               }
               if(c_flag==0)
               {

                  temp3 = root_node[tmp_state_1].tran_set;
                  while(temp3 != NULL)
                  {
                     temp4 = root_node[tmp_state_2].tran_set;
                     while(temp4 != NULL)
                     {
                        if(temp3->event == temp4->event)
                        {
                           flag = 0;
                           tmp_state_3 = temp3->target_state_number;
                           tmp_state_4 = temp4->target_state_number;
                           if(indexx == 1)
                           {
                              if(merging_table[tmp_state_3][tmp_state_4] == 0)
                              {
                                  temp4 = temp4->next_transition;
                                  continue;
                              }
                              if(merging_table[tmp_state_3][tmp_state_4] == 1 && tmp_state_3 != tmp_state_4){
								  return_code = 10;
								  goto CONDITION;
                              } //return 10;
                           }
                           if((root_node[tmp_state_3].equal_set)->state_number != (root_node[tmp_state_4].equal_set)->state_number)
                           {

			     /*****************************************************************/

                               if((root_node[tmp_state_3].equal_set)->state_number<nodel || (root_node[tmp_state_4].equal_set)->state_number<nodel)
                               { //flag = 10; goto LABEL;
								   return_code = 10;
								   goto CONDITION;
                               }
                               // return 10;
			     /*****************************************************************/


                               tmpu_point=0;
                               while(*(tmpu_stack+2*tmpu_point)!=-1)
                               {
                                  if(*(tmpu_stack+2*tmpu_point)==tmp_state_3 && *(tmpu_stack+2*tmpu_point+1)==tmp_state_4)
                                  {
                                      flag=8;
                                      break;
                                  }
                                  if(*(tmpu_stack+2*tmpu_point+1)==tmp_state_3 && *(tmpu_stack+2*tmpu_point)==tmp_state_4)
                                  {
                                      flag=8;
                                      break;
                                  }
                                  tmpu_point +=1;
                               }
                               if(flag==8)
                               {
                                   temp4=temp4->next_transition;
                                   continue;
                               }


                               if(root_node[tmp_state_3].marked_in_plant==root_node[tmp_state_4].marked_in_plant && root_node[tmp_state_3].marked_in_controller!=root_node[tmp_state_4].marked_in_controller)
                               {  // flag = 10; goto LABEL;
								   return_code = 10;
								   goto CONDITION;
                               } //   return 10;

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
                                          if(temp1->event == temp2->event){
                                             /*flag = 10;
                                             goto LABEL;*/
											  return_code = 10;
											  goto CONDITION;
                                          }
                                               //return 10;
                                          temp2 = temp2->next_transition;
                                      }
                                      temp1 = temp1->next_event;
                                  }
                               }
							   //store variables before restart the loop
							   sp_stack_Push(&s_node, node_1, node_2);
							   sp_stack_Push(&s_flag, r_flag_1, r_flag_2);
							   tran_stack_Push(&s_tmp34, temp3, temp4);
							   ess_stack_Push(&s_tmp67, temp6, temp7);
							   sp_stack_Push(&s_tmpu_point, tmpu_point_1, tmpu_point_2);
							   sp_stack_Push(&s_tmp_state, tmp_state_1, tmp_state_2);
                               if(flag==0)
                               {
                                  *(tmpu_stack+2*tmpu_point)=tmp_state_3;
                                  *(tmpu_stack+2*tmpu_point+1)=tmp_state_4;
                                  *(tmpu_stack+2*tmpu_point+2)=-1;
                                  //  fprintf(output, "test1"); fflush(output);
                                  //flag = Ex2_Selfloop_Node(tmp_state_3,tmp_state_4,nodel,indexx);
                                  node_1 = tmp_state_3;
                                  node_2 = tmp_state_4;
                                  goto START;
                                  // fprintf(output, "test2\n"); fflush(output);
                                  /*if(flag==10)
                                  {
                                     if(indexx == 1)
                                     {
                                       merging_table[tmp_state_3][tmp_state_4] = 1;
                                       merging_table[tmp_state_4][tmp_state_3] = 1;
                                     }
                                     return 10;
                                  }*/
                              }
CONTINUE:
							   sp_stack_Pop(&s_node, &node_1, &node_2, &ok);
							   sp_stack_Pop(&s_flag, &r_flag_1, &r_flag_2, &ok);
							   tran_stack_Pop(&s_tmp34, &temp3, &temp4, &ok);
							   ess_stack_Pop(&s_tmp67, &temp6, &temp7, &ok);
							   sp_stack_Pop(&s_tmpu_point, &tmpu_point_1, &tmpu_point_2, &ok);
							   sp_stack_Pop(&s_tmp_state, &tmp_state_1, &tmp_state_2, &ok);
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
  return_code = 9;
  if(!sp_stack_IsEmpty(&s_node)){
	  flag = return_code;
	  goto CONTINUE;	  
  }
CONDITION:
  while(!sp_stack_IsEmpty(&s_node)){
	  sp_stack_Pop(&s_node, &node_1, &node_2, &ok);
	  sp_stack_Pop(&s_flag, &r_flag_1, &r_flag_2, &ok);
	  tran_stack_Pop(&s_tmp34, &temp3, &temp4, &ok);
	  ess_stack_Pop(&s_tmp67, &temp6, &temp7, &ok);
	  sp_stack_Pop(&s_tmpu_point, &tmpu_point_1, &tmpu_point_2, &ok);
	  sp_stack_Pop(&s_tmp_state, &tmp_state_1, &tmp_state_2, &ok);
  }

  sp_stack_Done(&s_node);
  sp_stack_Done(&s_flag);
  tran_stack_Done(&s_tmp34);
  ess_stack_Done(&s_tmp67);
  sp_stack_Done(&s_tmpu_point);
  sp_stack_Done(&s_tmp_state);

  return return_code;
}

INT_OS Weak_Ex1_Selfloop_Node(INT_S base_state,INT_S tmp_state,INT_S nodel, INT_OS indexx)
{
  INT_S node_1,node_2,c_flag, flag, r_flag_1,r_flag_2;
  INT_S tmpu_point,tmpu_point_1,tmpu_point_2,k;
  struct forbidden_event_set *temp1;
  struct transitions *temp2, *temp22;
  struct equivalent_state_set *temp6, *temp7;
  struct transitions *temp3, *temp4;
  INT_S tmp_state_3, tmp_state_4;
  INT_S tmp_state_1, tmp_state_2;
  sp_stack s_node,s_flag,s_tmpu_point,s_tmp_state;
  tran_stack s_tmp34; ess_stack s_tmp67;
  INT_B ok;
  INT_OS return_code;

  sp_stack_Init(&s_node);
  sp_stack_Init(&s_flag);
  tran_stack_Init(&s_tmp34);
  ess_stack_Init(&s_tmp67);
  sp_stack_Init(&s_tmpu_point);
  sp_stack_Init(&s_tmp_state);

  node_1=base_state;
  node_2=tmp_state;
  
//  while(1){
START:
  temp1 = 0;
  temp2 = 0;
  temp6=root_node[root_node[node_1].equal_set->state_number].equal_set;
  r_flag_1=0;
  tmpu_point_1=0;
  while(r_flag_1==0)
  {
   if(indexx == 0)
   {
    if(temp6 != NULL)
    {
        tmp_state_1 = temp6->state_number;
        temp6 = temp6->next_node;
    }
    else
    {
        tmp_state_1=-1;
        while(*(tmpu_stack+2*tmpu_point_1)!=-1)
        {
            if(*(tmpu_stack+2*tmpu_point_1)==node_1)
            {
                tmp_state_1=*(tmpu_stack+2*tmpu_point_1+1);
                tmpu_point_1 +=1;
                break;
            }
            if(*(tmpu_stack+2*tmpu_point_1+1)==node_1)
            {
                tmp_state_1=*(tmpu_stack+2*tmpu_point_1);
                tmpu_point_1 +=1;
                break;
            }
            tmpu_point_1 +=1;
        }
        if(tmp_state_1==-1) r_flag_1=1;;
    }
   }
   else
   {
        r_flag_1 = 1;
        tmp_state_1 = node_1;
   }

    if(tmp_state_1!= -1)
    {
        temp7=root_node[root_node[node_2].equal_set->state_number].equal_set;
        r_flag_2=0;
        tmpu_point_2=0;
        while(r_flag_2==0)
        {
          if(indexx == 0)
          {
           if(temp7 != NULL)
           {
                 tmp_state_2 = temp7->state_number;
                 temp7 = temp7->next_node;
           }
           else
           {
                 tmp_state_2=-1;
                 while(*(tmpu_stack+2*tmpu_point_2)!=-1)
                 {
                     if(*(tmpu_stack+2*tmpu_point_2)==node_2)
                     {
                         tmp_state_2=*(tmpu_stack+2*tmpu_point_2+1);
                         tmpu_point_2 +=1;
                         break;
                     }
                     if(*(tmpu_stack+2*tmpu_point_2+1)==node_2)
                     {
                         tmp_state_2=*(tmpu_stack+2*tmpu_point_2);
                         tmpu_point_2 +=1;
                         break;
                     }
                     tmpu_point_2 +=1;
                 }
                 if(tmp_state_2==-1) r_flag_2=1;;
           }
          }
          else
          {
             r_flag_2 = 1;
             tmp_state_2 = node_2;
          }
           if(tmp_state_2!= -1)
           {
               c_flag=0;
               if((tmp_state_1!=node_1 || tmp_state_2!=node_2) && (tmp_state_1!=node_2 || tmp_state_2!=node_1))
               {
                   if(indexx == 1)
                   {
                      if(merging_table[tmp_state_1][tmp_state_2] == 0) continue;
                      if(merging_table[tmp_state_1][tmp_state_2] == 1 && tmp_state_1 != tmp_state_2){
						  return_code = 10;
						  goto CONDITION;
                      }//return 10;
                   }
                   if(root_node[tmp_state_1].equal_set->state_number == root_node[tmp_state_2].equal_set->state_number) continue;


                 tmpu_point=0;
                 while(*(tmpu_stack+2*tmpu_point)!=-1)
                 {
                    if(*(tmpu_stack+2*tmpu_point)==tmp_state_1 && *(tmpu_stack+2*tmpu_point+1)==tmp_state_2)
                    {
                        c_flag=8;
                        break;
                    }
                    if(*(tmpu_stack+2*tmpu_point+1)==tmp_state_1 && *(tmpu_stack+2*tmpu_point)==tmp_state_2)
                    {
                        c_flag=8;
                        break;
                    }
                    tmpu_point +=1;
                 }
                 if(c_flag ==8) continue;


                /* if(root_node[tmp_state_1].marked_in_plant==root_node[tmp_state_2].marked_in_plant && root_node[tmp_state_1].marked_in_controller!=root_node[tmp_state_2].marked_in_controller)
                  {  
					 return_code = 10;
					 goto CONDITION;
                  }     //  return 10;
				  */
                 for(k=1;k<3;k++)
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
                         if(temp1->event == temp2->event){
							 return_code = 10;
							 goto CONDITION;
                         }
                              //return 10;
                         temp2 = temp2->next_transition;
                     }
                     temp1 = temp1->next_event;
                   }
                 }
                 if(c_flag!=8)
                 {
                    *(tmpu_stack+2*tmpu_point)=tmp_state_1;
                    *(tmpu_stack+2*tmpu_point+1)=tmp_state_2;
                    *(tmpu_stack+2*tmpu_point+2)=-1;
                 }
               }
               if(c_flag==0)
               {

                  temp3 = root_node[tmp_state_1].tran_set;
                  while(temp3 != NULL)
                  {
                     temp4 = root_node[tmp_state_2].tran_set;
                     while(temp4 != NULL)
                     {
                        if(temp3->event == temp4->event)
                        {
                           flag = 0;
                           tmp_state_3 = temp3->target_state_number;
                           tmp_state_4 = temp4->target_state_number;
                           if(indexx == 1)
                           {
                              if(merging_table[tmp_state_3][tmp_state_4] == 0)
                              {
                                  temp4 = temp4->next_transition;
                                  continue;
                              }
                              if(merging_table[tmp_state_3][tmp_state_4] == 1 && tmp_state_3 != tmp_state_4){
								  return_code = 10;
								  goto CONDITION;
                              } //return 10;
                           }
                           if((root_node[tmp_state_3].equal_set)->state_number != (root_node[tmp_state_4].equal_set)->state_number)
                           {

			     /*****************************************************************/

                               if((root_node[tmp_state_3].equal_set)->state_number<nodel || (root_node[tmp_state_4].equal_set)->state_number<nodel)
                               { //flag = 10; goto LABEL;
								   return_code = 10;
								   goto CONDITION;
                               }
                               // return 10;
			     /*****************************************************************/


                               tmpu_point=0;
                               while(*(tmpu_stack+2*tmpu_point)!=-1)
                               {
                                  if(*(tmpu_stack+2*tmpu_point)==tmp_state_3 && *(tmpu_stack+2*tmpu_point+1)==tmp_state_4)
                                  {
                                      flag=8;
                                      break;
                                  }
                                  if(*(tmpu_stack+2*tmpu_point+1)==tmp_state_3 && *(tmpu_stack+2*tmpu_point)==tmp_state_4)
                                  {
                                      flag=8;
                                      break;
                                  }
                                  tmpu_point +=1;
                               }
                               if(flag==8)
                               {
                                   temp4=temp4->next_transition;
                                   continue;
                               }


                           /*    if(root_node[tmp_state_3].marked_in_plant==root_node[tmp_state_4].marked_in_plant && root_node[tmp_state_3].marked_in_controller!=root_node[tmp_state_4].marked_in_controller)
                               {  // flag = 10; goto LABEL;
								   return_code = 10;
								   goto CONDITION;
                               } //   return 10;
							   */
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
                                          if(temp1->event == temp2->event){
                                             /*flag = 10;
                                             goto LABEL;*/
											  return_code = 10;
											  goto CONDITION;
                                          }
                                               //return 10;
                                          temp2 = temp2->next_transition;
                                      }
                                      temp1 = temp1->next_event;
                                  }
                               }
							   //store variables before restart the loop
							   sp_stack_Push(&s_node, node_1, node_2);
							   sp_stack_Push(&s_flag, r_flag_1, r_flag_2);
							   tran_stack_Push(&s_tmp34, temp3, temp4);
							   ess_stack_Push(&s_tmp67, temp6, temp7);
							   sp_stack_Push(&s_tmpu_point, tmpu_point_1, tmpu_point_2);
							   sp_stack_Push(&s_tmp_state, tmp_state_1, tmp_state_2);
                               if(flag==0)
                               {
                                  *(tmpu_stack+2*tmpu_point)=tmp_state_3;
                                  *(tmpu_stack+2*tmpu_point+1)=tmp_state_4;
                                  *(tmpu_stack+2*tmpu_point+2)=-1;
                                  //  fprintf(output, "test1"); fflush(output);
                                  //flag = Ex2_Selfloop_Node(tmp_state_3,tmp_state_4,nodel,indexx);
                                  node_1 = tmp_state_3;
                                  node_2 = tmp_state_4;
                                  goto START;
                                  // fprintf(output, "test2\n"); fflush(output);
                                  /*if(flag==10)
                                  {
                                     if(indexx == 1)
                                     {
                                       merging_table[tmp_state_3][tmp_state_4] = 1;
                                       merging_table[tmp_state_4][tmp_state_3] = 1;
                                     }
                                     return 10;
                                  }*/
                              }
CONTINUE:
							   sp_stack_Pop(&s_node, &node_1, &node_2, &ok);
							   sp_stack_Pop(&s_flag, &r_flag_1, &r_flag_2, &ok);
							   tran_stack_Pop(&s_tmp34, &temp3, &temp4, &ok);
							   ess_stack_Pop(&s_tmp67, &temp6, &temp7, &ok);
							   sp_stack_Pop(&s_tmpu_point, &tmpu_point_1, &tmpu_point_2, &ok);
							   sp_stack_Pop(&s_tmp_state, &tmp_state_1, &tmp_state_2, &ok);
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
  return_code = 9;
  if(!sp_stack_IsEmpty(&s_node)){
	  flag = return_code;
	  goto CONTINUE;	  
  }
CONDITION:
  while(!sp_stack_IsEmpty(&s_node)){
	  sp_stack_Pop(&s_node, &node_1, &node_2, &ok);
	  sp_stack_Pop(&s_flag, &r_flag_1, &r_flag_2, &ok);
	  tran_stack_Pop(&s_tmp34, &temp3, &temp4, &ok);
	  ess_stack_Pop(&s_tmp67, &temp6, &temp7, &ok);
	  sp_stack_Pop(&s_tmpu_point, &tmpu_point_1, &tmpu_point_2, &ok);
	  sp_stack_Pop(&s_tmp_state, &tmp_state_1, &tmp_state_2, &ok);
  }

  sp_stack_Done(&s_node);
  sp_stack_Done(&s_flag);
  tran_stack_Done(&s_tmp34);
  ess_stack_Done(&s_tmp67);
  sp_stack_Done(&s_tmpu_point);
  sp_stack_Done(&s_tmp_state);

  return return_code;
}

/*This module is part of the main module Reduction2.*/
INT_OS Ex2_Selfloop_Node(INT_S base_state,INT_S tmp_state,INT_S nodel, INT_OS indexx)
{
  INT_S node_1,node_2,c_flag, flag, r_flag_1,r_flag_2;
  INT_S tmpu_point,tmpu_point_1,tmpu_point_2; INT_S k;
  struct forbidden_event_set *temp1;
  struct transitions *temp2, *temp22;
  struct equivalent_state_set *temp6, *temp7;
  INT_S tmp_state_1, tmp_state_2;
  INT_S num1, num2;
  char pReadBuf[50];
  char pWriteBuf[50];
  struct transitions *temp3, *temp4;
  INT_S tmp_state_3, tmp_state_4;
  sp_stack s_node,s_flag,s_tmpu_point,s_tmp_state;
  tran_stack s_tmp34; ess_stack s_tmp67;
  INT_B ok;
  INT_OS return_code;

  sp_stack_Init(&s_node);
  sp_stack_Init(&s_flag);
  tran_stack_Init(&s_tmp34);
  ess_stack_Init(&s_tmp67);
  sp_stack_Init(&s_tmpu_point);
  sp_stack_Init(&s_tmp_state);


  node_1 = base_state;
  node_2 = tmp_state;

  
START:

  temp1 = 0;
  temp2 = 0;

  temp6=root_node[root_node[node_1].equal_set->state_number].equal_set;
  r_flag_1=0;
  tmpu_point_1 = 1;
  while(r_flag_1==0)
  {
   if(indexx == 0)
   {
    if(temp6 != NULL)
    {
        tmp_state_1 = temp6->state_number;
        temp6 = temp6->next_node;
    }
    else
    {
        tmp_state_1=-1;   
        while(tmpu_point_1 <= nLengthWaitlist)
        {   //fprintf(output, "1read 1");
            nCurIndex = (INT_S)(tmpu_point_1 / nWaitlist_limit);
            if(nCurIndex != nViewIndex){
                  SetMapView(nCurIndex);                
            }
            memcpy(pReadBuf, pvWaitlist + (INT_S)(tmpu_point_1 - 1 - nViewIndex * ((INT_S)nWaitlist_limit) ) * 8, 8);
            num1 = atoi(pReadBuf);
            memcpy(pReadBuf, pvWaitlist + (INT_S)(tmpu_point_1 - nViewIndex * ((INT_S)nWaitlist_limit)) * 8, 8);
            num2 = atoi(pReadBuf);
            //fprintf(output, "1read 2\n");
            if(num1 == node_1)
            {   
                tmp_state_1 = num2;
                tmpu_point_1 += 2;
                break;
            }
            if(num2 == node_1)
            {
                tmp_state_1 = num1;
                tmpu_point_1 +=2;
                break;
            }
            tmpu_point_1 += 2;
        }        
        if(tmp_state_1==-1) r_flag_1=1;;
    }
   }
   else
   {
        r_flag_1 = 1;
        tmp_state_1 = node_1;
   }

    if(tmp_state_1!= -1)
    {
        temp7=root_node[root_node[node_2].equal_set->state_number].equal_set;
        r_flag_2=0;
        tmpu_point_2 = 1;
        while(r_flag_2==0)
        {
          if(indexx == 0)
          {
           if(temp7 != NULL)
           {
                 tmp_state_2 = temp7->state_number;
                 temp7 = temp7->next_node;
           }
           else
           {
                 tmp_state_2=-1;
                 while(tmpu_point_2 <= nLengthWaitlist)
                 {   
                     nCurIndex = (INT_S)(tmpu_point_2 / nWaitlist_limit);
                     if(nCurIndex != nViewIndex)
                        SetMapView(nCurIndex);
                     memcpy(pReadBuf, pvWaitlist + (INT_S)(tmpu_point_2 - 1 - nViewIndex * ((INT_S)nWaitlist_limit)) * 8, 8);
                     num1 = atoi(pReadBuf);
                     memcpy(pReadBuf, pvWaitlist + (INT_S)(tmpu_point_2 - nViewIndex * ((INT_S)nWaitlist_limit)) * 8, 8);
                     num2 = atoi(pReadBuf);
                     if(num1 == node_2)
                     {
                         tmp_state_2 = num2;
                         tmpu_point_2 += 2;
                         break;
                     }
                     if(num2 == node_2)
                     {
                         tmp_state_2 = num1;
                         tmpu_point_2 += 2;
                         break;
                     }
                     tmpu_point_2 += 2;
                 }
                 if(tmp_state_2==-1) r_flag_2=1;;
           }
          }
          else
          {
             r_flag_2 = 1;
             tmp_state_2 = node_2;
          }
          
           if(tmp_state_2!= -1)
           {
               c_flag=0;
               if((tmp_state_1!=node_1 || tmp_state_2!=node_2) && (tmp_state_1!=node_2 || tmp_state_2!=node_1))
               {
                   if(indexx == 1)
                   {
                      if(merging_table[tmp_state_1][tmp_state_2] == 0) continue;
					  if(merging_table[tmp_state_1][tmp_state_2] == 1 && tmp_state_1 != tmp_state_2){ 
						  return_code = 10;
						  goto CONDITION;
					  }
                   }
                   if(root_node[tmp_state_1].equal_set->state_number == root_node[tmp_state_2].equal_set->state_number) continue;
 
                 tmpu_point = 1;
                 while(tmpu_point <= nLengthWaitlist)
                 {
                    nCurIndex = (INT_S)(tmpu_point / nWaitlist_limit);
                    if(nCurIndex != nViewIndex)
                       SetMapView(nCurIndex);
                    memcpy(pReadBuf, pvWaitlist + (INT_S)(tmpu_point - 1 - nViewIndex * ((INT_S)nWaitlist_limit)) * 8, 8);
                    num1 = atoi(pReadBuf);
                    memcpy(pReadBuf, pvWaitlist + (INT_S)(tmpu_point - nViewIndex * ((INT_S)nWaitlist_limit)) * 8, 8);
                    num2 = atoi(pReadBuf);
                    //fprintf(output, "3read 2\n");
                    if(num1 == tmp_state_1 && num2 == tmp_state_2)
                    {
                          c_flag=8;
                          break;
                    }
                    if(num2 == tmp_state_1 && num1 == tmp_state_2)
                    {
                        c_flag=8;
                        break;
                    }
                    tmpu_point += 2;
                 }
                 if(c_flag ==8) continue;


				 if(root_node[tmp_state_1].marked_in_plant==root_node[tmp_state_2].marked_in_plant && root_node[tmp_state_1].marked_in_controller!=root_node[tmp_state_2].marked_in_controller){
					 return_code = 10;
					 goto CONDITION;
				 }

                 for(k=1;k<3;k++)
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
						 if(temp1->event == temp2->event){
							 return_code = 10;
							 goto CONDITION;
						 }
                         temp2 = temp2->next_transition;
                     }
                     temp1 = temp1->next_event;
                   }
                 }
                 if(c_flag!=8)
                 {
                    //fprintf(output, "Write 0");
                    nCurIndex = (INT_S)(tmpu_point / nWaitlist_limit);
                    if(nCurIndex != nViewIndex)
                       SetMapView(nCurIndex);
                    sprintf(pWriteBuf, "%8d%8d", tmp_state_1, tmp_state_2);
                    memcpy(pvWaitlist + (INT_S)(tmpu_point - 1 - nViewIndex * ((INT_S)nWaitlist_limit)) * 8, pWriteBuf, 16);
                    nLengthWaitlist += 2;
                    //fprintf(output, "Write 1\n");
                 }
               }
               if(c_flag==0)
               {
                  temp3 = root_node[tmp_state_1].tran_set;
                  while(temp3 != NULL)
                  {
                     temp4 = root_node[tmp_state_2].tran_set;
                     while(temp4 != NULL)
                     {
                        if(temp3->event == temp4->event)
                        {
                           flag = 0;
                           tmp_state_3 = temp3->target_state_number;
                           tmp_state_4 = temp4->target_state_number;
                           if(indexx == 1)
                           {
                              if(merging_table[tmp_state_3][tmp_state_4] == 0)
                              {
                                  temp4 = temp4->next_transition;
                                  continue;
                              }
                              if(merging_table[tmp_state_3][tmp_state_4] == 1 && tmp_state_3 != tmp_state_4){
								  return_code = 10;
								  goto CONDITION;
							  }
                           }
                           if((root_node[tmp_state_3].equal_set)->state_number != (root_node[tmp_state_4].equal_set)->state_number)
                           {

			     /*****************************************************************/

                               if((root_node[tmp_state_3].equal_set)->state_number<nodel || (root_node[tmp_state_4].equal_set)->state_number<nodel){
								   return_code = 10;
								   goto CONDITION;
							   }
			     /*****************************************************************/


                               tmpu_point = 1;
                               while(tmpu_point <= nLengthWaitlist)
                               {
                                  //fprintf(output, "4read 1");
                                  nCurIndex = (INT_S)(tmpu_point / nWaitlist_limit);
                                  if(nCurIndex != nViewIndex)
                                     SetMapView(nCurIndex);
                                  memcpy(pReadBuf, pvWaitlist + (INT_S)(tmpu_point - 1 - nViewIndex * ((INT_S)nWaitlist_limit)) * 8, 8);
                                  num1 = atoi(pReadBuf);
                                  memcpy(pReadBuf, pvWaitlist + (INT_S)(tmpu_point - nViewIndex * ((INT_S)nWaitlist_limit)) * 8, 8);
                                  num2 = atoi(pReadBuf);
                                  //fprintf(output, "4read 2\n");
                                  if(num1 == tmp_state_3 && num2 == tmp_state_4)
                                  {
                                        flag=8;
                                        break;
                                  }
                                  if(num2 == tmp_state_3 && num1 == tmp_state_4)
                                  {
                                        flag=8;
                                        break;
                                  }
                                  tmpu_point += 2;
                               }
                               if(flag==8)
                               {
                                   temp4=temp4->next_transition;
                                   continue;
                               }


                               if(root_node[tmp_state_3].marked_in_plant==root_node[tmp_state_4].marked_in_plant && root_node[tmp_state_3].marked_in_controller!=root_node[tmp_state_4].marked_in_controller)
							   {
								   return_code = 10;
								   goto CONDITION;
							   }

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
										  if(temp1->event == temp2->event){
											  return_code = 10;
											  goto CONDITION;
										  }
                                          temp2 = temp2->next_transition;
                                      }
                                      temp1 = temp1->next_event;
                                  }
                               }
							   //store variables before restart the loop
							   sp_stack_Push(&s_node, node_1, node_2);
							   sp_stack_Push(&s_flag, r_flag_1, r_flag_2);
							   tran_stack_Push(&s_tmp34, temp3, temp4);
							   ess_stack_Push(&s_tmp67, temp6, temp7);
							   sp_stack_Push(&s_tmpu_point, tmpu_point_1, tmpu_point_2);
							   sp_stack_Push(&s_tmp_state, tmp_state_1, tmp_state_2);
                               if(flag==0)
                               {
                                  //fprintf(output, "Write 1");
                                  nCurIndex = (INT_S)(tmpu_point / nWaitlist_limit);
                                  if(nCurIndex != nViewIndex)
                                     SetMapView(nCurIndex);
                                  sprintf(pWriteBuf, "%8d%8d", tmp_state_3, tmp_state_4);
                                  memcpy(pvWaitlist + (INT_S)(tmpu_point - 1 - nViewIndex * ((INT_S)nWaitlist_limit)) * 8, pWriteBuf, 16);
                                  nLengthWaitlist += 2;
                                  //fprintf(output, "Write 2");
                                  
                                  node_1 = tmp_state_3;
                                  node_2 = tmp_state_4;
                                  goto START;
                                  
                                  //flag = Ex_Selfloop_Node(tmp_state_3,tmp_state_4,nodel,indexx);
                                  /*if(flag==10)
                                  {
                                     if(indexx == 1)
                                     {
                                        merge_table[tmp_state_3][tmp_state_4] = 1;
                                        merge_table[tmp_state_4][tmp_state_3] = 1;
                                     }
                                     return 10;
                                  }*/
                              }
CONTINUE:
							   sp_stack_Pop(&s_node, &node_1, &node_2, &ok);
							   sp_stack_Pop(&s_flag, &r_flag_1, &r_flag_2, &ok);
							   tran_stack_Pop(&s_tmp34, &temp3, &temp4, &ok);
							   ess_stack_Pop(&s_tmp67, &temp6, &temp7, &ok);
							   sp_stack_Pop(&s_tmpu_point, &tmpu_point_1, &tmpu_point_2, &ok);
							   sp_stack_Pop(&s_tmp_state, &tmp_state_1, &tmp_state_2, &ok);
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
  return_code = 9;
  if(!sp_stack_IsEmpty(&s_node)){
	  flag = return_code;
	  goto CONTINUE;	  
  }
CONDITION:
  while(!sp_stack_IsEmpty(&s_node)){
	  sp_stack_Pop(&s_node, &node_1, &node_2, &ok);
	  sp_stack_Pop(&s_flag, &r_flag_1, &r_flag_2, &ok);
	  tran_stack_Pop(&s_tmp34, &temp3, &temp4, &ok);
	  ess_stack_Pop(&s_tmp67, &temp6, &temp7, &ok);
	  sp_stack_Pop(&s_tmpu_point, &tmpu_point_1, &tmpu_point_2, &ok);
	  sp_stack_Pop(&s_tmp_state, &tmp_state_1, &tmp_state_2, &ok);
  }

  sp_stack_Done(&s_node);
  sp_stack_Done(&s_flag);
  tran_stack_Done(&s_tmp34);
  ess_stack_Done(&s_tmp67);
  sp_stack_Done(&s_tmpu_point);
  sp_stack_Done(&s_tmp_state);

 // strcpy(pWriteBuf, "");

  return return_code;
}
/*This module is part of the main module Reduction3.*/
INT_OS Ex3_Selfloop_Node(INT_S base_state,INT_S tmp_state,INT_S nodel, INT_OS indexx)
{
  INT_S node_1,node_2,c_flag, flag, r_flag_1,r_flag_2;
  INT_S tmpu_point,tmpu_point_1,tmpu_point_2,k;
  struct forbidden_event_set *temp1;
  struct transitions *temp2, *temp22;
  struct transitions *temp3, *temp4;
  INT_S tmp_state_3, tmp_state_4;
  struct equivalent_state_set *temp6, *temp7;
  INT_S tmp_state_1, tmp_state_2;
  sp_stack s_node,s_flag,s_tmpu_point,s_tmp_state;
  tran_stack s_tmp34; ess_stack s_tmp67;
  INT_B ok;
  INT_OS return_code;

  sp_stack_Init(&s_node);
  sp_stack_Init(&s_flag);
  tran_stack_Init(&s_tmp34);
  ess_stack_Init(&s_tmp67);
  sp_stack_Init(&s_tmpu_point);
  sp_stack_Init(&s_tmp_state);
 
  node_1=base_state;
  node_2=tmp_state;
  temp1 = 0;
  temp2 = 0;
START:
  temp6=root_node[root_node[node_1].equal_set->state_number].equal_set;
  r_flag_1=0;
  tmpu_point_1=0;
  while(r_flag_1==0)
  {
   if(indexx == 0)
   {
    if(temp6 != NULL)
    {
        tmp_state_1 = temp6->state_number;
        temp6 = temp6->next_node;
    }
    else
    {
        tmp_state_1=-1;
        while( 2 * tmpu_point_1 < nLengthWaitlist)
        {
            if(*(tmpu_stack+2*tmpu_point_1)==node_1)
            {
                tmp_state_1=*(tmpu_stack+2*tmpu_point_1+1);
                tmpu_point_1 +=1;
                break;
            }
            if(*(tmpu_stack+2*tmpu_point_1+1)==node_1)
            {
                tmp_state_1=*(tmpu_stack+2*tmpu_point_1);
                tmpu_point_1 +=1;
                break;
            }
            tmpu_point_1 +=1;
        }
        if(tmp_state_1==-1) r_flag_1=1;;
    }
   }
   else
   {
        r_flag_1 = 1;
        tmp_state_1 = node_1;
   }

    if(tmp_state_1!= -1)
    {
        temp7=root_node[root_node[node_2].equal_set->state_number].equal_set;
        r_flag_2=0;
        tmpu_point_2=0;
        while(r_flag_2==0)
        {
          if(indexx == 0)
          {
           if(temp7 != NULL)
           {
                 tmp_state_2 = temp7->state_number;
                 temp7 = temp7->next_node;
           }
           else
           {
                 tmp_state_2=-1;
                 while(2 * tmpu_point_2 < nLengthWaitlist)
                 {
                     if(*(tmpu_stack+2*tmpu_point_2)==node_2)
                     {
                         tmp_state_2=*(tmpu_stack+2*tmpu_point_2+1);
                         tmpu_point_2 +=1;
                         break;
                     }
                     if(*(tmpu_stack+2*tmpu_point_2+1)==node_2)
                     {
                         tmp_state_2=*(tmpu_stack+2*tmpu_point_2);
                         tmpu_point_2 +=1;
                         break;
                     }
                     tmpu_point_2 +=1;
                 }
                 if(tmp_state_2==-1) r_flag_2=1;;
           }
          }
          else
          {
             r_flag_2 = 1;
             tmp_state_2 = node_2;
          }
           if(tmp_state_2!= -1)
           {
               c_flag=0;
               if((tmp_state_1!=node_1 || tmp_state_2!=node_2) && (tmp_state_1!=node_2 || tmp_state_2!=node_1))
               {
                   
                   if(root_node[tmp_state_1].equal_set->state_number == root_node[tmp_state_2].equal_set->state_number) continue;


                 tmpu_point=0;
                 while(2 * tmpu_point < nLengthWaitlist)
                 {
                    if(*(tmpu_stack+2*tmpu_point)==tmp_state_1 && *(tmpu_stack+2*tmpu_point+1)==tmp_state_2)
                    {
                        c_flag=8;
                        break;
                    }
                    if(*(tmpu_stack+2*tmpu_point+1)==tmp_state_1 && *(tmpu_stack+2*tmpu_point)==tmp_state_2)
                    {
                        c_flag=8;
                        break;
                    }
                    tmpu_point +=1;
                 }
                 if(c_flag ==8) continue;


                 if(root_node[tmp_state_1].marked_in_plant==root_node[tmp_state_2].marked_in_plant && root_node[tmp_state_1].marked_in_controller!=root_node[tmp_state_2].marked_in_controller)
                  {  
					  return_code = 10;
                     goto CONDITION;
                  }     //  return 10;

                 for(k=1;k<3;k++)
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
                         if(temp1->event == temp2->event){
							 return_code = 10;
							 goto CONDITION;
                         }
                              //return 10;
                         temp2 = temp2->next_transition;
                     }
                     temp1 = temp1->next_event;
                   }
                 }
                 if(c_flag!=8)
                 {
                    if(2 * tmpu_point >= nTmpuStack_limit){
						return_code = 12;
						goto CONDITION;
                    }
                    *(tmpu_stack+2*tmpu_point)=tmp_state_1;
                    *(tmpu_stack+2*tmpu_point+1)=tmp_state_2;
                    nLengthWaitlist += 2;
                 }
               }
               if(c_flag==0)
               {

                  temp3 = root_node[tmp_state_1].tran_set;
                  while(temp3 != NULL)
                  {
                     temp4 = root_node[tmp_state_2].tran_set;
                     while(temp4 != NULL)
                     {
                        if(temp3->event == temp4->event)
                        {
                           flag = 0;
                           tmp_state_3 = temp3->target_state_number;
                           tmp_state_4 = temp4->target_state_number;

                           if((root_node[tmp_state_3].equal_set)->state_number != (root_node[tmp_state_4].equal_set)->state_number)
                           {

			     /*****************************************************************/

                               if((root_node[tmp_state_3].equal_set)->state_number<nodel || (root_node[tmp_state_4].equal_set)->state_number<nodel)
							   { 		
								   return_code = 10;
								   goto CONDITION;
                               }
                               // return 10;
			     /*****************************************************************/


                               tmpu_point=0;
                               while(2 * tmpu_point < nLengthWaitlist)
                               {
                                  if(*(tmpu_stack+2*tmpu_point)==tmp_state_3 && *(tmpu_stack+2*tmpu_point+1)==tmp_state_4)
                                  {
                                      flag=8;
                                      break;
                                  }
                                  if(*(tmpu_stack+2*tmpu_point+1)==tmp_state_3 && *(tmpu_stack+2*tmpu_point)==tmp_state_4)
                                  {
                                      flag=8;
                                      break;
                                  }
                                  tmpu_point +=1;
                               }
                               if(flag==8)
                               {
                                   temp4=temp4->next_transition;
                                   continue;
                               }


                               if(root_node[tmp_state_3].marked_in_plant==root_node[tmp_state_4].marked_in_plant && root_node[tmp_state_3].marked_in_controller!=root_node[tmp_state_4].marked_in_controller)
                               {  // flag = 10; goto LABEL;
								   return_code = 10;
								   goto CONDITION;
                               } //   return 10;

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
                                          if(temp1->event == temp2->event){
											  return_code = 10;
											  goto CONDITION;
                                          }
                                               //return 10;
                                          temp2 = temp2->next_transition;
                                      }
                                      temp1 = temp1->next_event;
                                  }
                               }
							   //store variables before restart the loop
							   sp_stack_Push(&s_node, node_1, node_2);
							   sp_stack_Push(&s_flag, r_flag_1, r_flag_2);
							   tran_stack_Push(&s_tmp34, temp3, temp4);
							   ess_stack_Push(&s_tmp67, temp6, temp7);
							   sp_stack_Push(&s_tmpu_point, tmpu_point_1, tmpu_point_2);
							   sp_stack_Push(&s_tmp_state, tmp_state_1, tmp_state_2);
                               if(flag==0)
                               {
                                  if( 2 * tmpu_point >= nTmpuStack_limit){
                                     return_code = 12;
									 goto CONDITION;
                                  }
                                  *(tmpu_stack+2*tmpu_point)=tmp_state_3;
                                  *(tmpu_stack+2*tmpu_point+1)=tmp_state_4;
                                  nLengthWaitlist += 2; 
                                  node_1 = tmp_state_3;
                                  node_2 = tmp_state_4;
                                  goto START;
                              }
CONTINUE:
							  sp_stack_Pop(&s_node, &node_1, &node_2, &ok);
							  sp_stack_Pop(&s_flag, &r_flag_1, &r_flag_2, &ok);
							  tran_stack_Pop(&s_tmp34, &temp3, &temp4, &ok);
							  ess_stack_Pop(&s_tmp67, &temp6, &temp7, &ok);
							  sp_stack_Pop(&s_tmpu_point, &tmpu_point_1, &tmpu_point_2, &ok);
							  sp_stack_Pop(&s_tmp_state, &tmp_state_1, &tmp_state_2, &ok);
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
  return_code = 9;
  if(!sp_stack_IsEmpty(&s_node)){
	  flag = return_code;
	  goto CONTINUE;	  
  }
CONDITION:
  while(!sp_stack_IsEmpty(&s_node)){
	  sp_stack_Pop(&s_node, &node_1, &node_2, &ok);
	  sp_stack_Pop(&s_flag, &r_flag_1, &r_flag_2, &ok);
	  tran_stack_Pop(&s_tmp34, &temp3, &temp4, &ok);
	  ess_stack_Pop(&s_tmp67, &temp6, &temp7, &ok);
	  sp_stack_Pop(&s_tmpu_point, &tmpu_point_1, &tmpu_point_2, &ok);
	  sp_stack_Pop(&s_tmp_state, &tmp_state_1, &tmp_state_2, &ok);
  }

  sp_stack_Done(&s_node);
  sp_stack_Done(&s_flag);
  tran_stack_Done(&s_tmp34);
  ess_stack_Done(&s_tmp67);
  sp_stack_Done(&s_tmpu_point);
  sp_stack_Done(&s_tmp_state);

  return return_code;
}
INT_OS Weak_Ex3_Selfloop_Node(INT_S base_state,INT_S tmp_state,INT_S nodel, INT_OS indexx)
{
	INT_S node_1,node_2,c_flag, flag, r_flag_1,r_flag_2;
	INT_S tmpu_point,tmpu_point_1,tmpu_point_2,k;
	struct forbidden_event_set *temp1;
	struct transitions *temp2, *temp22;
	struct transitions *temp3, *temp4;
	INT_S tmp_state_3, tmp_state_4;
	struct equivalent_state_set *temp6, *temp7;
	INT_S tmp_state_1, tmp_state_2;
	sp_stack s_node,s_flag,s_tmpu_point,s_tmp_state;
	tran_stack s_tmp34; ess_stack s_tmp67;
	INT_B ok;
	INT_OS return_code;

	sp_stack_Init(&s_node);
	sp_stack_Init(&s_flag);
	tran_stack_Init(&s_tmp34);
	ess_stack_Init(&s_tmp67);
	sp_stack_Init(&s_tmpu_point);
	sp_stack_Init(&s_tmp_state);

	node_1=base_state;
	node_2=tmp_state;
	temp1 = 0;
	temp2 = 0;
START:
	temp6=root_node[root_node[node_1].equal_set->state_number].equal_set;
	r_flag_1=0;
	tmpu_point_1=0;
	while(r_flag_1==0)
	{
		if(indexx == 0)
		{
			if(temp6 != NULL)
			{
				tmp_state_1 = temp6->state_number;
				temp6 = temp6->next_node;
			}
			else
			{
				tmp_state_1=-1;
				while( 2 * tmpu_point_1 < nLengthWaitlist)
				{
					if(*(tmpu_stack+2*tmpu_point_1)==node_1)
					{
						tmp_state_1=*(tmpu_stack+2*tmpu_point_1+1);
						tmpu_point_1 +=1;
						break;
					}
					if(*(tmpu_stack+2*tmpu_point_1+1)==node_1)
					{
						tmp_state_1=*(tmpu_stack+2*tmpu_point_1);
						tmpu_point_1 +=1;
						break;
					}
					tmpu_point_1 +=1;
				}
				if(tmp_state_1==-1) r_flag_1=1;;
			}
		}
		else
		{
			r_flag_1 = 1;
			tmp_state_1 = node_1;
		}

		if(tmp_state_1!= -1)
		{
			temp7=root_node[root_node[node_2].equal_set->state_number].equal_set;
			r_flag_2=0;
			tmpu_point_2=0;
			while(r_flag_2==0)
			{
				if(indexx == 0)
				{
					if(temp7 != NULL)
					{
						tmp_state_2 = temp7->state_number;
						temp7 = temp7->next_node;
					}
					else
					{
						tmp_state_2=-1;
						while(2 * tmpu_point_2 < nLengthWaitlist)
						{
							if(*(tmpu_stack+2*tmpu_point_2)==node_2)
							{
								tmp_state_2=*(tmpu_stack+2*tmpu_point_2+1);
								tmpu_point_2 +=1;
								break;
							}
							if(*(tmpu_stack+2*tmpu_point_2+1)==node_2)
							{
								tmp_state_2=*(tmpu_stack+2*tmpu_point_2);
								tmpu_point_2 +=1;
								break;
							}
							tmpu_point_2 +=1;
						}
						if(tmp_state_2==-1) r_flag_2=1;;
					}
				}
				else
				{
					r_flag_2 = 1;
					tmp_state_2 = node_2;
				}
				if(tmp_state_2!= -1)
				{
					c_flag=0;
					if((tmp_state_1!=node_1 || tmp_state_2!=node_2) && (tmp_state_1!=node_2 || tmp_state_2!=node_1))
					{

						if(root_node[tmp_state_1].equal_set->state_number == root_node[tmp_state_2].equal_set->state_number) continue;


						tmpu_point=0;
						while(2 * tmpu_point < nLengthWaitlist)
						{
							if(*(tmpu_stack+2*tmpu_point)==tmp_state_1 && *(tmpu_stack+2*tmpu_point+1)==tmp_state_2)
							{
								c_flag=8;
								break;
							}
							if(*(tmpu_stack+2*tmpu_point+1)==tmp_state_1 && *(tmpu_stack+2*tmpu_point)==tmp_state_2)
							{
								c_flag=8;
								break;
							}
							tmpu_point +=1;
						}
						if(c_flag ==8) continue;


						/*if(root_node[tmp_state_1].marked_in_plant==root_node[tmp_state_2].marked_in_plant && root_node[tmp_state_1].marked_in_controller!=root_node[tmp_state_2].marked_in_controller)
						{  
							return_code = 10;
							goto CONDITION;
						}     //  return 10;
						*/
						for(k=1;k<3;k++)
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
									if(temp1->event == temp2->event){
										return_code = 10;
										goto CONDITION;
									}
									//return 10;
									temp2 = temp2->next_transition;
								}
								temp1 = temp1->next_event;
							}
						}
						if(c_flag!=8)
						{
							if(2 * tmpu_point >= nTmpuStack_limit){
								return_code = 12;
								goto CONDITION;
							}
							*(tmpu_stack+2*tmpu_point)=tmp_state_1;
							*(tmpu_stack+2*tmpu_point+1)=tmp_state_2;
							nLengthWaitlist += 2;
						}
					}
					if(c_flag==0)
					{

						temp3 = root_node[tmp_state_1].tran_set;
						while(temp3 != NULL)
						{
							temp4 = root_node[tmp_state_2].tran_set;
							while(temp4 != NULL)
							{
								if(temp3->event == temp4->event)
								{
									flag = 0;
									tmp_state_3 = temp3->target_state_number;
									tmp_state_4 = temp4->target_state_number;

									if((root_node[tmp_state_3].equal_set)->state_number != (root_node[tmp_state_4].equal_set)->state_number)
									{

										/*****************************************************************/

										if((root_node[tmp_state_3].equal_set)->state_number<nodel || (root_node[tmp_state_4].equal_set)->state_number<nodel)
										{ 		
											return_code = 10;
											goto CONDITION;
										}
										// return 10;
										/*****************************************************************/


										tmpu_point=0;
										while(2 * tmpu_point < nLengthWaitlist)
										{
											if(*(tmpu_stack+2*tmpu_point)==tmp_state_3 && *(tmpu_stack+2*tmpu_point+1)==tmp_state_4)
											{
												flag=8;
												break;
											}
											if(*(tmpu_stack+2*tmpu_point+1)==tmp_state_3 && *(tmpu_stack+2*tmpu_point)==tmp_state_4)
											{
												flag=8;
												break;
											}
											tmpu_point +=1;
										}
										if(flag==8)
										{
											temp4=temp4->next_transition;
											continue;
										}


									/*	if(root_node[tmp_state_3].marked_in_plant==root_node[tmp_state_4].marked_in_plant && root_node[tmp_state_3].marked_in_controller!=root_node[tmp_state_4].marked_in_controller)
										{  // flag = 10; goto LABEL;
											return_code = 10;
											goto CONDITION;
										} //   return 10;
										*/
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
													if(temp1->event == temp2->event){
														return_code = 10;
														goto CONDITION;
													}
													//return 10;
													temp2 = temp2->next_transition;
												}
												temp1 = temp1->next_event;
											}
										}
										//store variables before restart the loop
										sp_stack_Push(&s_node, node_1, node_2);
										sp_stack_Push(&s_flag, r_flag_1, r_flag_2);
										tran_stack_Push(&s_tmp34, temp3, temp4);
										ess_stack_Push(&s_tmp67, temp6, temp7);
										sp_stack_Push(&s_tmpu_point, tmpu_point_1, tmpu_point_2);
										sp_stack_Push(&s_tmp_state, tmp_state_1, tmp_state_2);
										if(flag==0)
										{
											if( 2 * tmpu_point >= nTmpuStack_limit){
												return_code = 12;
												goto CONDITION;
											}
											*(tmpu_stack+2*tmpu_point)=tmp_state_3;
											*(tmpu_stack+2*tmpu_point+1)=tmp_state_4;
											nLengthWaitlist += 2; 
											node_1 = tmp_state_3;
											node_2 = tmp_state_4;
											goto START;
										}
CONTINUE:
										sp_stack_Pop(&s_node, &node_1, &node_2, &ok);
										sp_stack_Pop(&s_flag, &r_flag_1, &r_flag_2, &ok);
										tran_stack_Pop(&s_tmp34, &temp3, &temp4, &ok);
										ess_stack_Pop(&s_tmp67, &temp6, &temp7, &ok);
										sp_stack_Pop(&s_tmpu_point, &tmpu_point_1, &tmpu_point_2, &ok);
										sp_stack_Pop(&s_tmp_state, &tmp_state_1, &tmp_state_2, &ok);
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
	return_code = 9;
	if(!sp_stack_IsEmpty(&s_node)){
		flag = return_code;
		goto CONTINUE;	  
	}
CONDITION:
	while(!sp_stack_IsEmpty(&s_node)){
		sp_stack_Pop(&s_node, &node_1, &node_2, &ok);
		sp_stack_Pop(&s_flag, &r_flag_1, &r_flag_2, &ok);
		tran_stack_Pop(&s_tmp34, &temp3, &temp4, &ok);
		ess_stack_Pop(&s_tmp67, &temp6, &temp7, &ok);
		sp_stack_Pop(&s_tmpu_point, &tmpu_point_1, &tmpu_point_2, &ok);
		sp_stack_Pop(&s_tmp_state, &tmp_state_1, &tmp_state_2, &ok);
	}

	sp_stack_Done(&s_node);
	sp_stack_Done(&s_flag);
	tran_stack_Done(&s_tmp34);
	ess_stack_Done(&s_tmp67);
	sp_stack_Done(&s_tmpu_point);
	sp_stack_Done(&s_tmp_state);

	return return_code;
}

/*This module is used to reduce the transition structure of optimal controller.*/
void Ex1_Reduction()
{
  struct forbidden_event_set *temp1;
  struct transitions *temp2, *temp22;
  INT_S temp3, temp4, temp_state;
  struct equivalent_state_set **temp5, *temp6;
  INT_S tmpu_point;
  INT_OS flag;
  INT_S i,j;
  INT_S state_1, state_2;
  *tmpu_stack=-1;

  

//  if (temp5) {} /* Remove compiler warning */
//  if (temp6) {} /* Remove compiler warning */

  for(i=0;i<num_states;i++)
  {
    if((root_node[i].equal_set)->state_number == i)
    {
      for(j=i+1;j<num_states;j++)
      {
        if((root_node[j].equal_set)->state_number==j)
        {
           flag=0;
           if(flag!=10)
           {
              if(root_node[i].marked_in_plant==root_node[j].marked_in_plant && root_node[i].marked_in_controller!=root_node[j].marked_in_controller)
                  flag = 10;
           }
           temp1 = root_node[i].forb_set;
           temp2 = root_node[j].tran_set;
           if(flag != 10)
           {
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
           }
           if(flag != 10)
           {             
             *tmpu_stack=i;
             *(tmpu_stack+1)=j;
             *(tmpu_stack+2)=-1;
             flag = Ex1_Selfloop_Node(i,j,i,0);
           }
           if(flag == 10)
           {
             *tmpu_stack = -1;
           }
           if(flag != 10)
           {
             tmpu_point=0;
             while(*(tmpu_stack+2*tmpu_point)!=-1)
             {
               state_1 = *(tmpu_stack+2*tmpu_point);
               state_2 = *(tmpu_stack+2*tmpu_point+1);
               temp3 = root_node[state_1].equal_set->state_number;
               temp4 = root_node[state_2].equal_set->state_number;
               if(temp3 < temp4)
               {
                  temp5=&(root_node[temp3].equal_set); /*acceptor*/
                  temp6=root_node[temp4].equal_set; /*merger*/
               }
               else if(temp3 > temp4)
               {
                  temp5=&(root_node[temp4].equal_set); /*acceptor*/
                  temp6=root_node[temp3].equal_set; /*merger*/
               }
               temp_state = (*temp5)->state_number;
               if(temp3==temp4)
               {
                  tmpu_point +=1;
                  continue;
               }
               while(*temp5!=NULL)
                  temp5 = &((*temp5)->next_node);
               while(temp6!=NULL)
               {
                  //fprintf(output, "test1");
                  //fflush(output);
                  *temp5 = (struct equivalent_state_set *) CALLOC(1, sizeof(struct equivalent_state_set));
                  if(*temp5 == NULL){
                     mem_result = 1;
                     return;
                  }
                  (*temp5)->state_number = temp6->state_number;
                  root_node[temp6->state_number].equal_set->state_number=temp_state;
                  temp6=temp6->next_node;
                  if(temp6!=NULL)
                     temp5 = &((*temp5)->next_node);
                  else
                     (*temp5)->next_node=NULL;
                  //fprintf(output, "test2\n");
                  //fflush(output);
               }
               tmpu_point += 1;
             }
             *tmpu_stack=-1;
           }
        }
      }
    }
  }
  simpler_controller = (INT_S *) CALLOC((3*tran_number+1), sizeof(INT_S));
  if(simpler_controller == NULL){
     mem_result = 1;
     return;
  }
  Final_Result();
}
/*This module is used to reduce the transition structure of optimal controller.*/
void Weak_Ex1_Reduction()
{
	struct forbidden_event_set *temp1;
	struct transitions *temp2, *temp22;
	INT_S temp3, temp4, temp_state;
	struct equivalent_state_set **temp5, *temp6;
	INT_S tmpu_point;
	INT_OS flag;
	INT_S i,j;
	INT_S state_1, state_2;
	*tmpu_stack=-1;

	//  if (temp5) {} /* Remove compiler warning */
	//  if (temp6) {} /* Remove compiler warning */

	for(i=0;i<num_states;i++)
	{
		if((root_node[i].equal_set)->state_number == i)
		{
			for(j=i+1;j<num_states;j++)
			{
				if((root_node[j].equal_set)->state_number==j)
				{
					flag=0;
					/*if(flag!=10)
					{
						if(root_node[i].marked_in_plant==root_node[j].marked_in_plant && root_node[i].marked_in_controller!=root_node[j].marked_in_controller)
							flag = 10;
					}*/
					temp1 = root_node[i].forb_set;
					temp2 = root_node[j].tran_set;
					if(flag != 10)
					{
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
					}
					if(flag != 10)
					{             
						*tmpu_stack=i;
						*(tmpu_stack+1)=j;
						*(tmpu_stack+2)=-1;
						flag = Weak_Ex1_Selfloop_Node(i,j,i,0);
					}
					if(flag == 10)
					{
						*tmpu_stack = -1;
					}
					if(flag != 10)
					{
						tmpu_point=0;
						while(*(tmpu_stack+2*tmpu_point)!=-1)
						{
							state_1 = *(tmpu_stack+2*tmpu_point);
							state_2 = *(tmpu_stack+2*tmpu_point+1);
							temp3 = root_node[state_1].equal_set->state_number;
							temp4 = root_node[state_2].equal_set->state_number;
							if(temp3 < temp4)
							{
								temp5=&(root_node[temp3].equal_set); /*acceptor*/
								temp6=root_node[temp4].equal_set; /*merger*/
							}
							else if(temp3 > temp4)
							{
								temp5=&(root_node[temp4].equal_set); /*acceptor*/
								temp6=root_node[temp3].equal_set; /*merger*/
							}
							temp_state = (*temp5)->state_number;
							if(temp3==temp4)
							{
								tmpu_point +=1;
								continue;
							}
							while(*temp5!=NULL)
								temp5 = &((*temp5)->next_node);
							while(temp6!=NULL)
							{
								//fprintf(output, "test1");
								//fflush(output);
								*temp5 = (struct equivalent_state_set *) CALLOC(1, sizeof(struct equivalent_state_set));
								if(*temp5 == NULL){
									mem_result = 1;
									return;
								}
								(*temp5)->state_number = temp6->state_number;
								root_node[temp6->state_number].equal_set->state_number=temp_state;
								temp6=temp6->next_node;
								if(temp6!=NULL)
									temp5 = &((*temp5)->next_node);
								else
									(*temp5)->next_node=NULL;
								//fprintf(output, "test2\n");
								//fflush(output);
							}
							tmpu_point += 1;
						}
						*tmpu_stack=-1;
					}
				}
			}
		}
	}
	simpler_controller = (INT_S *) CALLOC((3*tran_number+1), sizeof(INT_S));
	if(simpler_controller == NULL){
		mem_result = 1;
		return;
	}
	Final_Result();
}

/*This module is used to reduce the transition structure of optimal controller.*/
void Ex2_Reduction()
{
  struct forbidden_event_set *temp1;
  struct transitions *temp2, *temp22;
  INT_S temp3, temp4, temp_state;
  struct equivalent_state_set **temp5, *temp6;
  double tmpu_point;
  INT_OS flag;
  INT_S i,j;
  INT_S state_1, state_2;
  INT_B  bMapFlag = false;
  char pWriteBuf[50];
  char pReadBuf[50];
  
  //*tmpu_stack=-1;
  nLengthWaitlist = 0;
  
//  if (temp5) {} /* Remove compiler warning */
//  if (temp6) {} /* Remove compiler warning */

  for(i=0;i<num_states;i++)
  {
    if((root_node[i].equal_set)->state_number == i)
    {
      for(j=i+1;j<num_states;j++)
      {
        if((root_node[j].equal_set)->state_number==j)
        {
           flag=0;
           if(flag!=10)
           {
              if(root_node[i].marked_in_plant==root_node[j].marked_in_plant && root_node[i].marked_in_controller!=root_node[j].marked_in_controller)
                  flag = 10;
           }
           temp1 = root_node[i].forb_set;
           temp2 = root_node[j].tran_set;
           if(flag != 10)
           {
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
           }
           if(flag != 10)
           {
             SetMapView(0);
             sprintf(pWriteBuf, "%8d%8d",i,j);
             memcpy(pvWaitlist, pWriteBuf, 16);
             nLengthWaitlist = 2;
             flag = Ex2_Selfloop_Node(i,j,i,0);
           }
           if(flag != 10)
           {
             tmpu_point = 1;
             while(tmpu_point <= nLengthWaitlist)
             {
               //Read pair from Mapped Memory
               nCurIndex = (INT_S)(tmpu_point / nWaitlist_limit);
               if(nCurIndex != nViewIndex)
                  SetMapView(nCurIndex);
               memcpy(pReadBuf, pvWaitlist + (INT_S)(tmpu_point -1 - nViewIndex * ((INT_S)nWaitlist_limit)) * 8, 8);
               state_1 = atoi(pReadBuf);
               memcpy(pReadBuf, pvWaitlist + (INT_S)(tmpu_point - nViewIndex * ((INT_S)nWaitlist_limit)) * 8, 8);
               state_2 = atoi(pReadBuf);
               temp3 = root_node[state_1].equal_set->state_number;
               temp4 = root_node[state_2].equal_set->state_number;
               if(temp3 < temp4)
               {
                  temp5=&(root_node[temp3].equal_set); /*acceptor*/
                  temp6=root_node[temp4].equal_set; /*merger*/
               }
               else if(temp3 > temp4)
               {
                  temp5=&(root_node[temp4].equal_set); /*acceptor*/
                  temp6=root_node[temp3].equal_set; /*merger*/
               }
               temp_state = (*temp5)->state_number;
               if(temp3==temp4)
               {
                  tmpu_point +=2;
                  continue;
               }
               while(*temp5!=NULL)
                  temp5 = &((*temp5)->next_node);
               while(temp6!=NULL)
               {
                  *temp5 = (struct equivalent_state_set *) CALLOC(1, sizeof(struct equivalent_state_set));
                  (*temp5)->state_number = temp6->state_number;
                  root_node[temp6->state_number].equal_set->state_number=temp_state;
                  temp6=temp6->next_node;
                  if(temp6!=NULL)
                     temp5 = &((*temp5)->next_node);
                  else
                     (*temp5)->next_node=NULL;
               }
               tmpu_point += 2;
             }
             
           }
           nLengthWaitlist = 0;          
        }
      }
    }
  }
  simpler_controller=(INT_S *) CALLOC((3*tran_number+1), sizeof(INT_S));

  Final_Result();
}

/*This module is used to reduce the transition structure of optimal controller.*/
void Ex3_Reduction()
{
  struct forbidden_event_set *temp1;
  struct transitions *temp2, *temp22;
  INT_S temp3, temp4, temp_state;
  struct equivalent_state_set **temp5, *temp6;
  INT_S tmpu_point;
  INT_OS flag;
  INT_S i,j;
  INT_S state_1, state_2;
  
  nLengthWaitlist = 0;

//  if (temp5) {} /* Remove compiler warning */
//  if (temp6) {} /* Remove compiler warning */

  for(i=0;i<num_states;i++)
  {
    if((root_node[i].equal_set)->state_number == i)
    {
      for(j=i+1;j<num_states;j++)
      {
        if((root_node[j].equal_set)->state_number==j)
        {
           flag=0;
           if(flag!=10)
           {
              if(root_node[i].marked_in_plant==root_node[j].marked_in_plant && root_node[i].marked_in_controller!=root_node[j].marked_in_controller)
                  flag = 10;
           }
           temp1 = root_node[i].forb_set;
           temp2 = root_node[j].tran_set;
           if(flag != 10)
           {
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
           }
           if(flag != 10)
           {             
             *tmpu_stack=i;
             *(tmpu_stack+1)=j;
             nLengthWaitlist = 2;
             flag = Ex3_Selfloop_Node(i,j,i,0);
             if(flag == 12){
                mem_result = 1;
                return;
             }
           }
           if(flag == 10)
           {
             nLengthWaitlist = 0;
           }
           if(flag != 10)
           {
             tmpu_point=0;
             while(2 * tmpu_point < nLengthWaitlist)
             {
               state_1 = *(tmpu_stack+2*tmpu_point);
               state_2 = *(tmpu_stack+2*tmpu_point+1);
               temp3 = root_node[state_1].equal_set->state_number;
               temp4 = root_node[state_2].equal_set->state_number;
               if(temp3 < temp4)
               {
                  temp5=&(root_node[temp3].equal_set); /*acceptor*/
                  temp6=root_node[temp4].equal_set; /*merger*/
               }
               else if(temp3 > temp4)
               {
                  temp5=&(root_node[temp4].equal_set); /*acceptor*/
                  temp6=root_node[temp3].equal_set; /*merger*/
               }
               temp_state = (*temp5)->state_number;
               if(temp3==temp4)
               {
                  tmpu_point +=1;
                  continue;
               }
               while(*temp5!=NULL)
                  temp5 = &((*temp5)->next_node);
               while(temp6!=NULL)
               {
                  *temp5 = (struct equivalent_state_set *) CALLOC(1, sizeof(struct equivalent_state_set));
                  if(*temp5 == NULL){
                     mem_result = 1;
                     return;
                  }
                  (*temp5)->state_number = temp6->state_number;
                  root_node[temp6->state_number].equal_set->state_number=temp_state;
                  temp6=temp6->next_node;
                  if(temp6!=NULL)
                     temp5 = &((*temp5)->next_node);
                  else
                     (*temp5)->next_node=NULL;
               }
               tmpu_point += 1;
             }
             nLengthWaitlist = 0;
           }
        }
      }
    }
  }
  simpler_controller = (INT_S *) CALLOC((3*tran_number+1), sizeof(INT_S));
  if(simpler_controller == NULL){
     mem_result = 1;
     return;
  }
  Final_Result();
}

/*This module is used to reduce the transition structure of optimal controller.*/
void Weak_Ex3_Reduction()
{
	struct forbidden_event_set *temp1;
	struct transitions *temp2, *temp22;
	INT_S temp3, temp4, temp_state;
	struct equivalent_state_set **temp5, *temp6;
	INT_S tmpu_point;
	INT_OS flag;
	INT_S i,j;
	INT_S state_1, state_2;

	nLengthWaitlist = 0;

	//  if (temp5) {} /* Remove compiler warning */
	//  if (temp6) {} /* Remove compiler warning */

	for(i=0;i<num_states;i++)
	{
		if((root_node[i].equal_set)->state_number == i)
		{
			for(j=i+1;j<num_states;j++)
			{
				if((root_node[j].equal_set)->state_number==j)
				{
					flag=0;
					/*if(flag!=10)
					{
						if(root_node[i].marked_in_plant==root_node[j].marked_in_plant && root_node[i].marked_in_controller!=root_node[j].marked_in_controller)
							flag = 10;
					}*/
					temp1 = root_node[i].forb_set;
					temp2 = root_node[j].tran_set;
					if(flag != 10)
					{
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
					}
					if(flag != 10)
					{             
						*tmpu_stack=i;
						*(tmpu_stack+1)=j;
						nLengthWaitlist = 2;
						flag = Weak_Ex3_Selfloop_Node(i,j,i,0);
						if(flag == 12){
							mem_result = 1;
							return;
						}
					}
					if(flag == 10)
					{
						nLengthWaitlist = 0;
					}
					if(flag != 10)
					{
						tmpu_point=0;
						while(2 * tmpu_point < nLengthWaitlist)
						{
							state_1 = *(tmpu_stack+2*tmpu_point);
							state_2 = *(tmpu_stack+2*tmpu_point+1);
							temp3 = root_node[state_1].equal_set->state_number;
							temp4 = root_node[state_2].equal_set->state_number;
							if(temp3 < temp4)
							{
								temp5=&(root_node[temp3].equal_set); /*acceptor*/
								temp6=root_node[temp4].equal_set; /*merger*/
							}
							else if(temp3 > temp4)
							{
								temp5=&(root_node[temp4].equal_set); /*acceptor*/
								temp6=root_node[temp3].equal_set; /*merger*/
							}
							temp_state = (*temp5)->state_number;
							if(temp3==temp4)
							{
								tmpu_point +=1;
								continue;
							}
							while(*temp5!=NULL)
								temp5 = &((*temp5)->next_node);
							while(temp6!=NULL)
							{
								*temp5 = (struct equivalent_state_set *) CALLOC(1, sizeof(struct equivalent_state_set));
								if(*temp5 == NULL){
									mem_result = 1;
									return;
								}
								(*temp5)->state_number = temp6->state_number;
								root_node[temp6->state_number].equal_set->state_number=temp_state;
								temp6=temp6->next_node;
								if(temp6!=NULL)
									temp5 = &((*temp5)->next_node);
								else
									(*temp5)->next_node=NULL;
							}
							tmpu_point += 1;
						}
						nLengthWaitlist = 0;
					}
				}
			}
		}
	}
	simpler_controller = (INT_S *) CALLOC((3*tran_number+1), sizeof(INT_S));
	if(simpler_controller == NULL){
		mem_result = 1;
		return;
	}
	Final_Result();
}

INT_OS ex_supreduce(char *name1,
              char *name2,
              char *name3,
              char *name4,
              INT_S* lb,
              float *cr,
			  INT_B slb_flag)
{
   INT_S min_exit,tmp_data,tmp_trace_node,trace_node,index,min_tran_point;
   INT_S base_point,number_of_transitions,tmp_data_1;
   float compress_ratio;
   INT_S trace_mark;
   INT_OS return_code = 0, flag;
   INT_S i;//, j;// k;
   INT_S num_states_2 = 0;
   char path[MAX_PATH];
   INT_B  approach_flag = false;
   INT_S nSizeWaitlist;
   INT_S nNumWaitlist;
//   int nLastPos;
   char *tmpBuf;
   DWORD nLength;
//   int nLengthWaitlist;
   struct _finddata_t fileinfo; 
   long handle;  
   INT_B reduce_type;
   
   output = NULL;
  
   /* Initial all arrays to NULL */
   c_marked_states    = NULL;
   p_marked_states    = NULL;
   controller         = NULL;
   plant              = NULL;
   root_node          = NULL;
   controller_tree    = NULL;
   simpler_controller = NULL;
   tmpu_stack         = NULL;
   hWaitlist          = NULL;
   //record             = NULL;

   nFileSize_limit = (INT_S)pow(2.0, 25);
   nWaitlist_limit = (INT_S)pow(2.0, 22);
   nTmpuStack_limit = (INT_S)pow(2.0, 27);

  // sprintf(path, "%s%s\\out.txt", prefix, TmpData);
  // output = fopen(path, "w");

   flag = Get_DES(&tran_number,&num_states, 1, name1);
   if (flag != 0) return flag;
   if (num_states == 0) return -1;

   flag = Get_DES(&tran_number,&num_states, 0, name2);
   if (flag != 0) return flag+10;
   if (num_states == 0) return -1;
   if (tran_number == 1)
   {
      return_code = -2;
      goto FREEMEM;
   }   

   root_node = (struct node *) CALLOC(num_states,sizeof(struct node));
   controller_tree=(INT_S *) CALLOC((2*tran_number+15*num_states+1),sizeof(INT_S));
   
   if(root_node == NULL || controller_tree == NULL)
   {
      mem_result = 1;
      return_code = 30;
      goto FREEMEM;
   }

   Ex_Controller_Tree();
   free(controller); controller = NULL;

   /* generate combined_tree */
   flag = Ex_Combined_Tree();
   free(plant); plant = NULL;
   if(flag==1) {
      return_code = 40;
      goto FREEMEM;
   }
   
   Tree_Structure_Conversion(name3);
   free(controller_tree);  controller_tree = NULL;
   

   approach_flag = false;
   reduce_type = 0;

   tmpu_stack=(INT_S *) CALLOC((2*num_states*num_states+1), sizeof(INT_S));
   if(tmpu_stack == NULL){
	   mem_result = 1;
   }else{
	   Reduction_Faster(root_node, num_states, tran_number);
	   reduce_type = 1;
   }
   if(mem_result == 1){
	   mem_result = 0;
	   tmpu_stack=(INT_S *) CALLOC(nTmpuStack_limit, sizeof(INT_S));
	   if(tmpu_stack == NULL){
		   mem_result = 1;
	   }else{
		   Ex3_Reduction();
		   reduce_type = 3;
	   }
   }
   //free(tmpu_stack); tmpu_stack = NULL;
   //mem_result = 1;
   if(mem_result == 1){ 
      mem_result = 0;
      approach_flag = true;
	  nSizeWaitlist = 2 * num_states * num_states;
	  nNumWaitlist =  (INT_S)((float)num_states / nWaitlist_limit)*2 *num_states  + 1;

      hWaitlist = (HANDLE*)CALLOC(nNumWaitlist, sizeof(HANDLE));
      if(hWaitlist == NULL){
         mem_result = 1;
         return_code = 30;
         goto FREEMEM;
      }
      tmpBuf = (char*)CALLOC(nFileSize_limit, 1);// 1 is the size of char
      if(tmpBuf == NULL){
         mem_result = 1;
         return_code = 30;
         goto FREEMEM;
      }
	  //Create folder TempData to store temporary files
	  sprintf(path, "%s%s", prefix, TmpData);
	  if(_access(path, 0) == -1){
		  if(_mkdir(path))  
			  return -1;
	  }   
      for(i = 0; i < nNumWaitlist; i ++){
         sprintf(path, "%s%s%d.dat", prefix, waitlist_dat, i);
         hWaitlist[i] = CreateFile(path, GENERIC_READ | GENERIC_WRITE, 0, NULL,
               OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
      }
      if(WriteFile(hWaitlist[0], tmpBuf, (DWORD)nFileSize_limit, &nLength, NULL) == 0){
         mem_result = 1;
         return_code = 30;
         goto FREEMEM;
      }
      free(tmpBuf); tmpBuf = NULL;

      hMapWaitlist = CreateFileMapping(hWaitlist[0], NULL, PAGE_READWRITE, 0, (DWORD)nFileSize_limit, NULL);
      if(hMapWaitlist == NULL){
		  mem_result = 1;
         return_code = 30;
         goto FREEMEM;
      }

      pvWaitlist = (char*)MapViewOfFile(hMapWaitlist, FILE_MAP_WRITE, 0, 0, 0);
      if(pvWaitlist == NULL){
		  mem_result = 1;
         return_code = 30;
         goto FREEMEM;
      }
      nFileIndex = nViewIndex = 0;

      Ex2_Reduction();
	  reduce_type = 2;
   }
   
   /* refine simpler_controller to generate the final text version transition structure of the reduced supervisor */
   base_point=0;
   while(*(simpler_controller+3*base_point)!=-1)
   {
     min_exit=base_point;
     trace_node=base_point+1;
     while(*(simpler_controller+3*trace_node)!=-1)     
     {
       if(*(simpler_controller+3*trace_node)<*(simpler_controller+3*min_exit))       
           min_exit=trace_node;
       trace_node +=1;
     }
     tmp_data = *(simpler_controller+3*base_point);
     *(simpler_controller+3*base_point)=*(simpler_controller+3*min_exit);
     *(simpler_controller+3*min_exit)=tmp_data;
     tmp_data=*(simpler_controller+3*base_point+1);
     *(simpler_controller+3*base_point+1)=*(simpler_controller+3*min_exit+1);
     *(simpler_controller+3*min_exit+1)=tmp_data;
     tmp_data=*(simpler_controller+3*base_point+2);
     *(simpler_controller+3*base_point+2)=*(simpler_controller+3*min_exit+2);
     *(simpler_controller+3*min_exit+2)=tmp_data;
     base_point +=1;
   }
   base_point=0;
   while(*(simpler_controller+3*base_point)!=-1)
   {
     trace_node=base_point+1;
     while(*(simpler_controller+3*trace_node)!=-1)
     {
       if(*(simpler_controller+3*trace_node)>*(simpler_controller+3*base_point))
          break;
       else
       {
          if((*(simpler_controller+3*trace_node+1)==*(simpler_controller+3*base_point+1))&&(*(simpler_controller+3*trace_node+2)==*(simpler_controller+3*base_point+2)))
          {
              tmp_trace_node=trace_node+1;
              while(*(simpler_controller+3*tmp_trace_node)!=-1)
              {
                 *(simpler_controller+3*tmp_trace_node-3)=*(simpler_controller+3*tmp_trace_node);
                 *(simpler_controller+3*tmp_trace_node-2)=*(simpler_controller+3*tmp_trace_node+1);
                 *(simpler_controller+3*tmp_trace_node-1)=*(simpler_controller+3*tmp_trace_node+2);
                 tmp_trace_node +=1;
              }
              *(simpler_controller+3*tmp_trace_node-3)=-1;
              trace_node -=1;
          }
       }
       trace_node +=1;
     }
     base_point +=1;
   }
   base_point=0;
   index=1;
   tmp_data=0;
   while(*(simpler_controller+3*base_point)!=-1)
   {
      while(*(simpler_controller+3*base_point)>tmp_data)
      {
          if(*(simpler_controller+3*base_point)==-1) break;
          tmp_data=*(simpler_controller+3*base_point);
          trace_mark=0;
          while(*(c_marked_states+trace_mark)!=-1)
          {
             if(*(c_marked_states+trace_mark)==tmp_data) *(c_marked_states+trace_mark)=index;
             trace_mark +=1;
          }
          while((*(simpler_controller+3*base_point)==tmp_data)&&(*(simpler_controller+3*base_point)!=-1))
          {
            *(simpler_controller+3*base_point)=index;
            base_point +=1;
          }
          tmp_trace_node=0;
          while(*(simpler_controller+3*tmp_trace_node)!=-1)
          {
            if(*(simpler_controller+3*tmp_trace_node+2)==tmp_data) *(simpler_controller+3*tmp_trace_node+2)=index;
            tmp_trace_node +=1;
          }
          index +=1;
     }
     if(*(simpler_controller+3*base_point)!=-1) base_point +=1;
   }
   base_point=0;
   while(*(simpler_controller+3*base_point)!=-1)
   {
     tmp_data=*(simpler_controller+3*base_point);
     trace_node=base_point;
     while(*(simpler_controller+3*base_point)==tmp_data) base_point +=1;
     while(trace_node<base_point)
     {
       min_tran_point=trace_node;
       for(tmp_trace_node=trace_node;tmp_trace_node<base_point;tmp_trace_node++)
       {
          if(*(simpler_controller+3*tmp_trace_node+1)<*(simpler_controller+3*min_tran_point+1))
          {
              min_tran_point=tmp_trace_node;
          }
       }
       tmp_data_1=*(simpler_controller+3*trace_node+1);
       *(simpler_controller+3*trace_node+1)=*(simpler_controller+3*min_tran_point+1);
       *(simpler_controller+3*min_tran_point+1)=tmp_data_1;
       tmp_data_1=*(simpler_controller+3*trace_node+2);
       *(simpler_controller+3*trace_node+2)=*(simpler_controller+3*min_tran_point+2);
       *(simpler_controller+3*min_tran_point+2)=tmp_data_1;
       trace_node +=1;
     }
   }

   base_point=0;
   number_of_transitions=0;
   while(*(simpler_controller+3*base_point)!=-1)
   {
     number_of_transitions +=1;
     if(*(simpler_controller+3*base_point+1)>=1000) 
        number_of_transitions -=1;
     base_point +=1;
   }

   /* output simsup.des */
   tmp_data=*(simpler_controller+3*base_point-3)+1;
   compress_ratio = ((float) num_states)/((float) tmp_data);
   *cr = compress_ratio;

   out=fopen(name4, "wb");
   if (out == NULL) return 1;
   flag = Txt_DES(tmp_data);
   fclose(out);
   //goto FREEMEM;

   free(simpler_controller); simpler_controller = NULL;
   free(c_marked_states); c_marked_states = NULL;
   free(p_marked_states); p_marked_states = NULL;
   
   if(flag!=1)
   {
      return_code = 50;
      goto FREEMEM;
   }
   /* lower bound estimate 
      In the extended version, we don't estimate the lower bound
   */
   if((slb_flag == true) && (reduce_type == 1)){
	   *lb = Refinement();
   }else
	   *lb = 0;

FREEMEM:      
   if(approach_flag){
	   if(pvWaitlist != NULL)
		   UnmapViewOfFile(pvWaitlist);
	   if(hMapWaitlist != NULL)
		   CloseHandle(hMapWaitlist);
	   for(i = 0; i < nNumWaitlist; i ++){
		   if(hWaitlist[i] != NULL)
			   CloseHandle(hWaitlist[i]);
	   }
	   free(hWaitlist);

	   // Clear the file in the folder TmpData

	   sprintf(path, "%s%s\\*.*", prefix, TmpData);
	   handle = (long)_findfirst(path,&fileinfo); 
	   if(handle != -1){
		   while (_findnext(handle, &fileinfo) == 0) 
		   { 
			   sprintf(path, "%s%s\\%s", prefix, TmpData, fileinfo.name);
			   remove(path); 
		   } 
	   }
	   _findclose(handle);
	   sprintf(path, "%s%s", prefix, TmpData);
	   _rmdir(path);    
	   
   }
   if (root_node != NULL)
   {
       for (i=0; i < num_states; i++) {
          struct equivalent_state_set *temp1, *temp11;
          struct forbidden_event_set *temp2, *temp22;
          struct transitions *temp3, *temp33;

          temp1 = root_node[i].equal_set;
          while (temp1 != NULL)
          {
             temp11 = temp1->next_node;
             free(temp1);
             temp1 = temp11;
          }

          temp2 = root_node[i].forb_set;
          while (temp2 != NULL)
          {
             temp22 = temp2->next_event;
             free(temp2);
             temp2 = temp22;
          }

          temp3 = root_node[i].tran_set;
          while (temp3 != NULL)
          {
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
   
   return return_code;
}
//Modefied by ZRY to implement Localization algorithm
INT_OS ex_supreduce1(char *name1,
              char *name2,
              char *name3,
              char *name4,
              INT_S* lb,
              float *cr,
			  INT_S *fnum)
{
   INT_S min_exit,tmp_data,tmp_trace_node,trace_node,index,min_tran_point;
   INT_S base_point,number_of_transitions,tmp_data_1;
   float compress_ratio;
   INT_S trace_mark;
   INT_OS return_code = 0, flag;
   INT_S i;//, j;// k;
   INT_S num_states_2 = 0;
//   char path[MAX_PATH];
 //  INT_B  approach_flag = false;
//   INT_S nSizeWaitlist;
 //  INT_S nNumWaitlist;
//   int nLastPos;
//   char *tmpBuf;
//   DWORD nLength;
//   int nLengthWaitlist;
 //  struct _finddata_t fileinfo; 
 //  long handle;  
   
   output = NULL;
  
   /* Initial all arrays to NULL */
   c_marked_states    = NULL;
   p_marked_states    = NULL;
   controller         = NULL;
   plant              = NULL;
   root_node          = NULL;
   controller_tree    = NULL;
   simpler_controller = NULL;
   tmpu_stack         = NULL;
   hWaitlist          = NULL;
   //record             = NULL;

   //nFileSize_limit = (INT_S)pow(2.0, 25);
   //nWaitlist_limit = (INT_S)pow(2.0, 22);
   nTmpuStack_limit = (INT_S)pow(2.0, 27);

  // sprintf(path, "%s%s\\out.txt", prefix, TmpData);
  // output = fopen(path, "w");

   flag = Get_DES(&tran_number,&num_states, 1, name1);
   if (flag != 0) return flag;
   if (num_states == 0) return -1;

   flag = Get_DES(&tran_number,&num_states, 0, name2);
   if (flag != 0) return flag+10;
   if (num_states == 0) return -1;
   if (tran_number == 1)
   {
      return_code = -2;
      goto FREEMEM;
   }   

   root_node = (struct node *) CALLOC(num_states,sizeof(struct node));
   controller_tree=(INT_S *) CALLOC((2*tran_number+15*num_states+1),sizeof(INT_S));
   
   if(root_node == NULL || controller_tree == NULL)
   {
      mem_result = 1;
      return_code = 30;
      goto FREEMEM;
   }

   Ex_Controller_Tree();
   free(controller); controller = NULL;

   /* generate combined_tree */
   flag = Ex_Combined_Tree();
   free(plant); plant = NULL;
   if(flag==1) {
      return_code = 40;
      goto FREEMEM;
   }
   
   Tree_Structure_Conversion(name3);
   free(controller_tree);  controller_tree = NULL;
   
 //  nSizeWaitlist = 2 * num_states * num_states;
 //  nNumWaitlist =  (nSizeWaitlist / nWaitlist_limit) + 1;
 //  approach_flag = false;

   tmpu_stack=(INT_S *) CALLOC((2*num_states*num_states+1), sizeof(INT_S));
   if(tmpu_stack == NULL){
	   mem_result = 1;
   }else{
	   Weak_Ex1_Reduction();
   }
   if(mem_result == 1){
	   mem_result = 0;
	   tmpu_stack=(INT_S *) CALLOC(nTmpuStack_limit, sizeof(INT_S));
	   if(tmpu_stack == NULL){
		   mem_result = 1;
	   }else{
		   Weak_Ex3_Reduction();
	   }
   }
   free(tmpu_stack); tmpu_stack = NULL;
   if(mem_result == 1){
	   return_code = 30;
	   goto FREEMEM;
   }
   //mem_result = 1;
/*   if(mem_result == 1){ 
      mem_result = 0;
      approach_flag = true;
      hWaitlist = (HANDLE*)CALLOC(nNumWaitlist, sizeof(HANDLE));
      if(hWaitlist == NULL){
         mem_result = 1;
         return_code = 30;
         goto FREEMEM;
      }
      tmpBuf = (char*)CALLOC(nFileSize_limit, 1);// 1 is the size of char
      if(tmpBuf == NULL){
         mem_result = 1;
         return_code = 30;
         goto FREEMEM;
      }
	  //Create folder TempData to store temporary files
	  sprintf(path, "%s%s", prefix, TmpData);
	  if(_access(path, 0) == -1){
		  if(_mkdir(path))  
			  return -1;
	  }   
      for(i = 0; i < nNumWaitlist; i ++){
         sprintf(path, "%s%s%d.dat", prefix, waitlist_dat, i);
         hWaitlist[i] = CreateFile(path, GENERIC_READ | GENERIC_WRITE, 0, NULL,
               OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
      }
      if(WriteFile(hWaitlist[0], tmpBuf, (DWORD)nFileSize_limit, &nLength, NULL) == 0){
         mem_result = 1;
         return_code = 30;
         goto FREEMEM;
      }
      free(tmpBuf); tmpBuf = NULL;

      hMapWaitlist = CreateFileMapping(hWaitlist[0], NULL, PAGE_READWRITE, 0, (DWORD)nFileSize_limit, NULL);
      if(hMapWaitlist == NULL){
		  mem_result = 1;
         return_code = 30;
         goto FREEMEM;
      }

      pvWaitlist = (char*)MapViewOfFile(hMapWaitlist, FILE_MAP_WRITE, 0, 0, 0);
      if(pvWaitlist == NULL){
		  mem_result = 1;
         return_code = 30;
         goto FREEMEM;
      }
      nFileIndex = nViewIndex = 0;

      Ex2_Reduction();
   }*/
   
   /* refine simpler_controller to generate the final text version transition structure of the reduced supervisor */
   base_point=0;
   while(*(simpler_controller+3*base_point)!=-1)
   {
     min_exit=base_point;
     trace_node=base_point+1;
     while(*(simpler_controller+3*trace_node)!=-1)     
     {
       if(*(simpler_controller+3*trace_node)<*(simpler_controller+3*min_exit))       
           min_exit=trace_node;
       trace_node +=1;
     }
     tmp_data = *(simpler_controller+3*base_point);
     *(simpler_controller+3*base_point)=*(simpler_controller+3*min_exit);
     *(simpler_controller+3*min_exit)=tmp_data;
     tmp_data=*(simpler_controller+3*base_point+1);
     *(simpler_controller+3*base_point+1)=*(simpler_controller+3*min_exit+1);
     *(simpler_controller+3*min_exit+1)=tmp_data;
     tmp_data=*(simpler_controller+3*base_point+2);
     *(simpler_controller+3*base_point+2)=*(simpler_controller+3*min_exit+2);
     *(simpler_controller+3*min_exit+2)=tmp_data;
     base_point +=1;
   }
   base_point=0;
   while(*(simpler_controller+3*base_point)!=-1)
   {
     trace_node=base_point+1;
     while(*(simpler_controller+3*trace_node)!=-1)
     {
       if(*(simpler_controller+3*trace_node)>*(simpler_controller+3*base_point))
          break;
       else
       {
          if((*(simpler_controller+3*trace_node+1)==*(simpler_controller+3*base_point+1))&&(*(simpler_controller+3*trace_node+2)==*(simpler_controller+3*base_point+2)))
          {
              tmp_trace_node=trace_node+1;
              while(*(simpler_controller+3*tmp_trace_node)!=-1)
              {
                 *(simpler_controller+3*tmp_trace_node-3)=*(simpler_controller+3*tmp_trace_node);
                 *(simpler_controller+3*tmp_trace_node-2)=*(simpler_controller+3*tmp_trace_node+1);
                 *(simpler_controller+3*tmp_trace_node-1)=*(simpler_controller+3*tmp_trace_node+2);
                 tmp_trace_node +=1;
              }
              *(simpler_controller+3*tmp_trace_node-3)=-1;
              trace_node -=1;
          }
       }
       trace_node +=1;
     }
     base_point +=1;
   }
   base_point=0;
   index=1;
   tmp_data=0;
   while(*(simpler_controller+3*base_point)!=-1)
   {
      while(*(simpler_controller+3*base_point)>tmp_data)
      {
          if(*(simpler_controller+3*base_point)==-1) break;
          tmp_data=*(simpler_controller+3*base_point);
          trace_mark=0;
          while(*(c_marked_states+trace_mark)!=-1)
          {
             if(*(c_marked_states+trace_mark)==tmp_data) *(c_marked_states+trace_mark)=index;
             trace_mark +=1;
          }
          while((*(simpler_controller+3*base_point)==tmp_data)&&(*(simpler_controller+3*base_point)!=-1))
          {
            *(simpler_controller+3*base_point)=index;
            base_point +=1;
          }
          tmp_trace_node=0;
          while(*(simpler_controller+3*tmp_trace_node)!=-1)
          {
            if(*(simpler_controller+3*tmp_trace_node+2)==tmp_data) *(simpler_controller+3*tmp_trace_node+2)=index;
            tmp_trace_node +=1;
          }
          index +=1;
     }
     if(*(simpler_controller+3*base_point)!=-1) base_point +=1;
   }
   base_point=0;
   while(*(simpler_controller+3*base_point)!=-1)
   {
     tmp_data=*(simpler_controller+3*base_point);
     trace_node=base_point;
     while(*(simpler_controller+3*base_point)==tmp_data) base_point +=1;
     while(trace_node<base_point)
     {
       min_tran_point=trace_node;
       for(tmp_trace_node=trace_node;tmp_trace_node<base_point;tmp_trace_node++)
       {
          if(*(simpler_controller+3*tmp_trace_node+1)<*(simpler_controller+3*min_tran_point+1))
          {
              min_tran_point=tmp_trace_node;
          }
       }
       tmp_data_1=*(simpler_controller+3*trace_node+1);
       *(simpler_controller+3*trace_node+1)=*(simpler_controller+3*min_tran_point+1);
       *(simpler_controller+3*min_tran_point+1)=tmp_data_1;
       tmp_data_1=*(simpler_controller+3*trace_node+2);
       *(simpler_controller+3*trace_node+2)=*(simpler_controller+3*min_tran_point+2);
       *(simpler_controller+3*min_tran_point+2)=tmp_data_1;
       trace_node +=1;
     }
   }

   base_point=0;
   number_of_transitions=0;
   while(*(simpler_controller+3*base_point)!=-1)
   {
     number_of_transitions +=1;
     if(*(simpler_controller+3*base_point+1)>=1000) 
        number_of_transitions -=1;
     base_point +=1;
   }

   /* output simsup.des */
   tmp_data=*(simpler_controller+3*base_point-3)+1;
   compress_ratio = ((float) num_states)/((float) tmp_data);
   *cr = compress_ratio;

   out=fopen(name4, "wb");
   if (out == NULL) return 1;
   flag = Txt_DES(tmp_data);
   *fnum = tmp_data;
   fclose(out);
   goto FREEMEM;

   free(simpler_controller); simpler_controller = NULL;
   free(c_marked_states); c_marked_states = NULL;
   free(p_marked_states); p_marked_states = NULL;
   
   if(flag!=1)
   {
      return_code = 50;
      goto FREEMEM;
   }
   /* lower bound estimate 
      In the extended version, we don't estimate the lower bound
   */
   *lb = 0;

FREEMEM:      
  /* if(approach_flag){
	   if(pvWaitlist != NULL)
		   UnmapViewOfFile(pvWaitlist);
	   if(hMapWaitlist != NULL)
		   CloseHandle(hMapWaitlist);
	   for(i = 0; i < nNumWaitlist; i ++){
		   if(hWaitlist[i] != NULL)
			   CloseHandle(hWaitlist[i]);
	   }
	   free(hWaitlist);

	   // Clear the file in the folder TmpData

	   sprintf(path, "%s%s\\*.*", prefix, TmpData);
	   handle = (long)_findfirst(path,&fileinfo); 
	   if(handle != -1){
		   while (_findnext(handle, &fileinfo) == 0) 
		   { 
			   sprintf(path, "%s%s\\%s", prefix, TmpData, fileinfo.name);
			   remove(path); 
		   } 
	   }
	   _findclose(handle);
	   sprintf(path, "%s%s", prefix, TmpData);
	   _rmdir(path);    
	   
   }*/
   if (root_node != NULL)
   {
       for (i=0; i < num_states; i++) {
          struct equivalent_state_set *temp1, *temp11;
          struct forbidden_event_set *temp2, *temp22;
          struct transitions *temp3, *temp33;

          temp1 = root_node[i].equal_set;
          while (temp1 != NULL)
          {
             temp11 = temp1->next_node;
             free(temp1);
             temp1 = temp11;
          }

          temp2 = root_node[i].forb_set;
          while (temp2 != NULL)
          {
             temp22 = temp2->next_event;
             free(temp2);
             temp2 = temp22;
          }

          temp3 = root_node[i].tran_set;
          while (temp3 != NULL)
          {
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
   
   return return_code;
}
//Modefied by ZRY to implement Localization algorithm
INT_OS ex_supreduce2(char *name1,
              char *name2,
              char *name3,
              char *name4,
              INT_S* lb,
              float *cr,
			  INT_S *fnum)
{
   INT_S min_exit,tmp_data,tmp_trace_node,trace_node,index,min_tran_point;
   INT_S base_point,number_of_transitions,tmp_data_1;
   float compress_ratio;
   INT_S trace_mark;
   INT_OS return_code = 0, flag;
   INT_S i;//, j;// k;
   INT_S num_states_2 = 0;
   
   output = NULL;
  
   /* Initial all arrays to NULL */
   c_marked_states    = NULL;
   p_marked_states    = NULL;
   controller         = NULL;
   plant              = NULL;
   root_node          = NULL;
   controller_tree    = NULL;
   simpler_controller = NULL;
   tmpu_stack         = NULL;
   hWaitlist          = NULL;
   //record             = NULL;

   //nFileSize_limit = (INT_S)pow(2.0, 25);
   //nWaitlist_limit = (INT_S)pow(2.0, 22);
   nTmpuStack_limit = (INT_S)pow(2.0, 27);

  // sprintf(path, "%s%s\\out.txt", prefix, TmpData);
  // output = fopen(path, "w");

   tran_number = num_states = 0;
   flag = Get_DES(&tran_number,&num_states, 1, name1);
   if (flag != 0) {return_code = flag; goto FREEMEM;}
   if (num_states == 0) {return_code = -1; goto FREEMEM;}

    tran_number = num_states = 0;
   flag = Get_DES(&tran_number,&num_states, 0, name2);
   if (flag != 0) {return_code = flag+10; goto FREEMEM;}
   if (num_states == 0) {return_code = -1; goto FREEMEM;}
   if (tran_number == 1)
   {
      return_code = -2;
      goto FREEMEM;
   }   

   root_node = (struct node *) CALLOC(num_states,sizeof(struct node));
   controller_tree=(INT_S *) CALLOC((2*tran_number+15*num_states+1),sizeof(INT_S));
   
   if(root_node == NULL || controller_tree == NULL)
   {
      mem_result = 1;
      return_code = 30;
      goto FREEMEM;
   }

   Ex_Controller_Tree();
   free(controller); controller = NULL;

   /* generate combined_tree */
   flag = Ex_Combined_Tree();
   free(plant); plant = NULL;
   if(flag==1) {
      return_code = 40;
      goto FREEMEM;
   }
   
   Tree_Structure_Conversion(name3);
   free(controller_tree);  controller_tree = NULL;
  

   tmpu_stack=(INT_S *) CALLOC((2*num_states*num_states+1), sizeof(INT_S));
   if(tmpu_stack == NULL){
	   mem_result = 1;
   }else{
	   Reduction_Faster(root_node, num_states, tran_number);
   }
   if(mem_result == 1){
	   mem_result = 0;
	   tmpu_stack=(INT_S *) CALLOC(nTmpuStack_limit, sizeof(INT_S));
	   if(tmpu_stack == NULL){
		   mem_result = 1;
	   }else{
		   Ex3_Reduction();
	   }
   }
   free(tmpu_stack); tmpu_stack = NULL;
   if(mem_result == 1){
	   return_code = 30;
	   goto FREEMEM;
   }

   
   /* refine simpler_controller to generate the final text version transition structure of the reduced supervisor */
   base_point=0;
   while(*(simpler_controller+3*base_point)!=-1)
   {
     min_exit=base_point;
     trace_node=base_point+1;
     while(*(simpler_controller+3*trace_node)!=-1)     
     {
       if(*(simpler_controller+3*trace_node)<*(simpler_controller+3*min_exit))       
           min_exit=trace_node;
       trace_node +=1;
     }
     tmp_data = *(simpler_controller+3*base_point);
     *(simpler_controller+3*base_point)=*(simpler_controller+3*min_exit);
     *(simpler_controller+3*min_exit)=tmp_data;
     tmp_data=*(simpler_controller+3*base_point+1);
     *(simpler_controller+3*base_point+1)=*(simpler_controller+3*min_exit+1);
     *(simpler_controller+3*min_exit+1)=tmp_data;
     tmp_data=*(simpler_controller+3*base_point+2);
     *(simpler_controller+3*base_point+2)=*(simpler_controller+3*min_exit+2);
     *(simpler_controller+3*min_exit+2)=tmp_data;
     base_point +=1;
   }
   base_point=0;
   while(*(simpler_controller+3*base_point)!=-1)
   {
     trace_node=base_point+1;
     while(*(simpler_controller+3*trace_node)!=-1)
     {
       if(*(simpler_controller+3*trace_node)>*(simpler_controller+3*base_point))
          break;
       else
       {
          if((*(simpler_controller+3*trace_node+1)==*(simpler_controller+3*base_point+1))&&(*(simpler_controller+3*trace_node+2)==*(simpler_controller+3*base_point+2)))
          {
              tmp_trace_node=trace_node+1;
              while(*(simpler_controller+3*tmp_trace_node)!=-1)
              {
                 *(simpler_controller+3*tmp_trace_node-3)=*(simpler_controller+3*tmp_trace_node);
                 *(simpler_controller+3*tmp_trace_node-2)=*(simpler_controller+3*tmp_trace_node+1);
                 *(simpler_controller+3*tmp_trace_node-1)=*(simpler_controller+3*tmp_trace_node+2);
                 tmp_trace_node +=1;
              }
              *(simpler_controller+3*tmp_trace_node-3)=-1;
              trace_node -=1;
          }
       }
       trace_node +=1;
     }
     base_point +=1;
   }
   base_point=0;
   index=1;
   tmp_data=0;
   while(*(simpler_controller+3*base_point)!=-1)
   {
      while(*(simpler_controller+3*base_point)>tmp_data)
      {
          if(*(simpler_controller+3*base_point)==-1) break;
          tmp_data=*(simpler_controller+3*base_point);
          trace_mark=0;
          while(*(c_marked_states+trace_mark)!=-1)
          {
             if(*(c_marked_states+trace_mark)==tmp_data) *(c_marked_states+trace_mark)=index;
             trace_mark +=1;
          }
          while((*(simpler_controller+3*base_point)==tmp_data)&&(*(simpler_controller+3*base_point)!=-1))
          {
            *(simpler_controller+3*base_point)=index;
            base_point +=1;
          }
          tmp_trace_node=0;
          while(*(simpler_controller+3*tmp_trace_node)!=-1)
          {
            if(*(simpler_controller+3*tmp_trace_node+2)==tmp_data) *(simpler_controller+3*tmp_trace_node+2)=index;
            tmp_trace_node +=1;
          }
          index +=1;
     }
     if(*(simpler_controller+3*base_point)!=-1) base_point +=1;
   }
   base_point=0;
   while(*(simpler_controller+3*base_point)!=-1)
   {
     tmp_data=*(simpler_controller+3*base_point);
     trace_node=base_point;
     while(*(simpler_controller+3*base_point)==tmp_data) base_point +=1;
     while(trace_node<base_point)
     {
       min_tran_point=trace_node;
       for(tmp_trace_node=trace_node;tmp_trace_node<base_point;tmp_trace_node++)
       {
          if(*(simpler_controller+3*tmp_trace_node+1)<*(simpler_controller+3*min_tran_point+1))
          {
              min_tran_point=tmp_trace_node;
          }
       }
       tmp_data_1=*(simpler_controller+3*trace_node+1);
       *(simpler_controller+3*trace_node+1)=*(simpler_controller+3*min_tran_point+1);
       *(simpler_controller+3*min_tran_point+1)=tmp_data_1;
       tmp_data_1=*(simpler_controller+3*trace_node+2);
       *(simpler_controller+3*trace_node+2)=*(simpler_controller+3*min_tran_point+2);
       *(simpler_controller+3*min_tran_point+2)=tmp_data_1;
       trace_node +=1;
     }
   }

   base_point=0;
   number_of_transitions=0;
   while(*(simpler_controller+3*base_point)!=-1)
   {
     number_of_transitions +=1;
     if(*(simpler_controller+3*base_point+1)>=1000) 
        number_of_transitions -=1;
     base_point +=1;
   }

   /* output simsup.des */
   tmp_data=*(simpler_controller+3*base_point-3)+1;
   compress_ratio = ((float) num_states)/((float) tmp_data);
   *cr = compress_ratio;

   out=fopen(name4, "wb");
   if (out == NULL) return 1;
   flag = Txt_DES(tmp_data);
   *fnum = tmp_data;
   fclose(out);
   goto FREEMEM;

   free(simpler_controller); simpler_controller = NULL;
   free(c_marked_states); c_marked_states = NULL;
   free(p_marked_states); p_marked_states = NULL;
   
   if(flag!=1)
   {
      return_code = 50;
      goto FREEMEM;
   }
   /* lower bound estimate 
      In the extended version, we don't estimate the lower bound
   */
   *lb = 0;

FREEMEM:      
  /* if(approach_flag){
	   if(pvWaitlist != NULL)
		   UnmapViewOfFile(pvWaitlist);
	   if(hMapWaitlist != NULL)
		   CloseHandle(hMapWaitlist);
	   for(i = 0; i < nNumWaitlist; i ++){
		   if(hWaitlist[i] != NULL)
			   CloseHandle(hWaitlist[i]);
	   }
	   free(hWaitlist);

	   // Clear the file in the folder TmpData

	   sprintf(path, "%s%s\\*.*", prefix, TmpData);
	   handle = (long)_findfirst(path,&fileinfo); 
	   if(handle != -1){
		   while (_findnext(handle, &fileinfo) == 0) 
		   { 
			   sprintf(path, "%s%s\\%s", prefix, TmpData, fileinfo.name);
			   remove(path); 
		   } 
	   }
	   _findclose(handle);
	   sprintf(path, "%s%s", prefix, TmpData);
	   _rmdir(path);    
	   
   }*/
   if (root_node != NULL)
   {
       for (i=0; i < num_states; i++) {
          struct equivalent_state_set *temp1, *temp11;
          struct forbidden_event_set *temp2, *temp22;
          struct transitions *temp3, *temp33;

          temp1 = root_node[i].equal_set;
          while (temp1 != NULL)
          {
             temp11 = temp1->next_node;
             free(temp1);
             temp1 = temp11;
          }

          temp2 = root_node[i].forb_set;
          while (temp2 != NULL)
          {
             temp22 = temp2->next_event;
             free(temp2);
             temp2 = temp22;
          }

          temp3 = root_node[i].tran_set;
          while (temp3 != NULL)
          {
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
   
   return return_code;
}


#ifdef __cplusplus
}
#endif
