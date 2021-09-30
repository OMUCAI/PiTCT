#ifndef _DES_SUPP_H
#define _DES_SUPP_H

#include "des_data.h"
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* state pair Stack data structure */
typedef struct sp_stack {
  state_pair *head;
  INT_S head_size;
  INT_S pstack_size;
} sp_stack;

/* Stack routine */
extern INT_B sp_stack_Push(sp_stack *, INT_S, INT_S);
extern void sp_stack_Pop(sp_stack *, INT_S *, INT_S *, INT_B *);
extern INT_B sp_stack_IsEmpty(sp_stack *);
extern void sp_stack_Init(sp_stack *);
extern void sp_stack_Done(sp_stack *);

/* Stack data structure for transitions*/
typedef struct t_stack {
  tran_node *head;
  INT_S head_size;
  INT_S pstack_size;
} t_stack;

/* Stack routine */
extern INT_B pstack_Push(t_stack *, INT_T, INT_S);
extern void pstack_Pop(t_stack *, INT_T *, INT_S *, INT_B *);
extern INT_B pstack_IsEmpty(t_stack *);
extern void pstack_Init(t_stack *);
extern void pstack_Done(t_stack *);

/* Stack data structure for state */
typedef struct s_stack {
  INT_S *head;
  INT_S head_size;
  INT_S pstack_size;
} s_stack;

/* Stack routine */
extern INT_B stack_Push(s_stack *, INT_S);
extern void stack_Pop(s_stack *, INT_S *, INT_B *);
extern INT_B stack_IsEmpty(s_stack *);
extern void stack_Init(s_stack *);
extern void stack_Done(s_stack *);

/* Queue data structure */
typedef struct t_queue {
  INT_S *ptr;
  INT_S head;
  INT_S tail;
  INT_S size;
} t_queue;

/* Queue routine */
extern INT_B enqueue(t_queue *, INT_S);
extern INT_S dequeue(t_queue *);
extern INT_B inqueue(t_queue *, INT_S);
extern INT_B queue_empty(t_queue *);
extern void queue_init(t_queue *);
extern void queue_done(t_queue *);

#ifdef __cplusplus
}
#endif

#endif /* _DES_SUPP_H */
