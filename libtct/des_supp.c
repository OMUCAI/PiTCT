#include "des_supp.h"
#include "des_data.h"
#include "mymalloc.h"
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* static state_pair* head = NULL;
static INT_S      head_size;
static INT_S      pstack_size = 0L; */

INT_B sp_stack_Push(sp_stack *ts, INT_S value1, INT_S value2) {
  if (ts->head_size >= ts->pstack_size) {
    ts->head = (state_pair *)REALLOC(ts->head, sizeof(state_pair) *
                                                   (ts->pstack_size + 512L));
    if (ts->head == NULL) {
      mem_result = 1;
      return false;
    }
    ts->pstack_size += 512L;
  }
  ts->head[ts->head_size].data1 = value1;
  ts->head[ts->head_size].data2 = value2;
  ts->head_size++;
  return true;
}

void sp_stack_Pop(sp_stack *ts, INT_S *value1, INT_S *value2, INT_B *ok) {
  if (ts->head_size == 0L) {
    *ok = false;
    return;
  }

  ts->head_size--;
  *value1 = ts->head[ts->head_size].data1;
  *value2 = ts->head[ts->head_size].data2;
  *ok = true;
}

INT_B sp_stack_IsEmpty(sp_stack *ts) {
  if (ts->head_size == 0L)
    return true;
  else
    return false;
}

void sp_stack_Init(sp_stack *ts) {
  ts->head_size = 0L;
  ts->pstack_size = 0L;
  ts->head = NULL;
}

void sp_stack_Done(sp_stack *ts) {
  free(ts->head);
  ts->head_size = 0L;
  ts->pstack_size = 0L;
  ts->head = NULL;
}

/* static tran_node* head = NULL;
static INT_S      head_size;
static INT_S      pstack_size = 0L; */

INT_B pstack_Push(t_stack *ts, INT_T value1, INT_S value2) {
  if (ts->head_size >= ts->pstack_size) {
    ts->head = (tran_node *)REALLOC(ts->head, sizeof(tran_node) *
                                                  (ts->pstack_size + 512L));
    if (ts->head == NULL) {
      mem_result = 1;
      return false;
    }
    ts->pstack_size += 512L;
  }
  ts->head[ts->head_size].data1 = value1;
  ts->head[ts->head_size].data2 = value2;
  ts->head_size++;
  return true;
}

void pstack_Pop(t_stack *ts, INT_T *value1, INT_S *value2, INT_B *ok) {
  if (ts->head_size == 0L) {
    *ok = false;
    return;
  }

  ts->head_size--;
  *value1 = ts->head[ts->head_size].data1;
  *value2 = ts->head[ts->head_size].data2;
  *ok = true;
}

INT_B pstack_IsEmpty(t_stack *ts) {
  if (ts->head_size == 0L)
    return true;
  else
    return false;
}

void pstack_Init(t_stack *ts) {
  ts->head_size = 0L;
  ts->pstack_size = 0L;
  ts->head = NULL;
}

void pstack_Done(t_stack *ts) {
  free(ts->head);
  ts->head_size = 0L;
  ts->pstack_size = 0L;
  ts->head = NULL;
}
///////////////////////////////////

INT_B stack_Push(s_stack *ts, INT_S value) {
  if (ts->head_size >= ts->pstack_size) {
    ts->head =
        (INT_S *)REALLOC(ts->head, sizeof(INT_S) * (ts->pstack_size + 512L));
    if (ts->head == NULL) {
      mem_result = 1;
      return false;
    }
    ts->pstack_size += 512L;
  }
  ts->head[ts->head_size] = value;
  ts->head_size++;
  return true;
}

void stack_Pop(s_stack *ts, INT_S *value, INT_B *ok) {
  if (ts->head_size == 0L) {
    *ok = false;
    return;
  }

  ts->head_size--;
  *value = ts->head[ts->head_size];
  *ok = true;
}

INT_B stack_IsEmpty(s_stack *ts) {
  if (ts->head_size == 0L)
    return true;
  else
    return false;
}

void stack_Init(s_stack *ts) {
  ts->head_size = 0L;
  ts->pstack_size = 0L;
  ts->head = NULL;
}

void stack_Done(s_stack *ts) {
  free(ts->head);
  ts->head_size = 0L;
  ts->pstack_size = 0L;
  ts->head = NULL;
}

/* static INT_S* queue_ptr = NULL;
static INT_S  queue_head = 0L;
static INT_S  queue_tail = 0L;
static INT_S  queue_size = 0L;
*/

INT_B enqueue(t_queue *tq, INT_S value) {
  INT_S new_queue_head;

  new_queue_head = tq->head + 1;
  if (new_queue_head > tq->size) {
    new_queue_head = 0L;
    if (new_queue_head == tq->tail) {
      tq->ptr = (INT_S *)REALLOC(tq->ptr, sizeof(INT_S) * (tq->size + 512L));
      if (tq->ptr == NULL) {
        mem_result = 1;
        return false;
      }
      tq->size += 512L;
    } else {
      tq->head = 0L;
    }
  } else {
    if (new_queue_head == tq->tail) {
      tq->ptr = (INT_S *)REALLOC(tq->ptr, sizeof(INT_S) * (tq->size + 512L));
      if (tq->ptr == NULL) {
        mem_result = 1;
        return false;
      }
      /* Shift the memory down */
      memmove(&tq->ptr[tq->tail + 512L], &tq->ptr[tq->tail],
              sizeof(INT_S) * (tq->size - tq->tail));

      tq->size += 512L;
      tq->tail += 512L;
    }
  }

  tq->ptr[tq->head] = value;
  tq->head++;

  return true;
}

INT_S dequeue(t_queue *tq) {
  INT_S value;

  if (tq->head == tq->tail) {
    return 0L;
  }

  value = tq->ptr[tq->tail];
  tq->tail++;
  if (tq->tail >= tq->size) {
    tq->tail = 0L;
  }
  return value;
}

INT_B inqueue(t_queue *tq, INT_S value) {
  INT_S i;

  for (i = tq->tail; i < tq->head; i++) {
    if (tq->ptr[i] == value)
      return true;
  }

  return false;
}

INT_B queue_empty(t_queue *tq) { return ((tq->head % tq->size) == tq->tail); }

void queue_init(t_queue *tq) {
  tq->head = 0L;
  tq->tail = 0L;
  tq->size = 0L;
  tq->ptr = NULL;
}

void queue_done(t_queue *tq) {
  tq->head = 0L;
  tq->tail = 0L;
  tq->size = 0L;
  free(tq->ptr);
  tq->ptr = NULL;
}

#ifdef __cplusplus
}
#endif
