/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */
 
#define QUEUE_C
#include "Queue.h"
#undef QUEUE_C
#include <stdlib.h>

Queue *Queue_new(void)
{
  Queue *self = (Queue *)calloc(1, sizeof(Queue));
  self->size = QUEUE_START_SIZE;
  self->items = calloc(1, self->size * sizeof(void *));
  return self;
}

Queue *Queue_clone(Queue *self) 
{
  Queue *child = cpalloc(self, sizeof(Queue));
  child->items = cpalloc(self->items, self->size*sizeof(void *));
  return child;
}

void Queue_free(Queue *self)
{
  free(self->items);
  free(self);
}

int Queue_length(Queue *self) { return self->bottom - self->top; }

void *Queue_at_(Queue *self, int i)
{
  if (i < 0 || i >= Queue_length(self)) return (void *)0x0;
  return self->items[self->top+i];
}


void Queue_compact(Queue *self)
{
  if (self->top)
  {
    memmove(self->items, self->items + self->top, self->bottom - self->top);
    self->bottom -= self->top;
    self->top = 0;
    self->size = self->bottom + 1;
    self->items = realloc(self->items, self->size);
  }
}

size_t Queue_memorySize(Queue *self)
{ return sizeof(Queue) + (self->size*sizeof(void *)); }

void Queue_print(Queue *self)
{
  printf("Queue: \n");
  printf("size    = %i\n", self->size);
  printf("top     = %i\n", self->top);
  printf("bottom  = %i\n", self->bottom);
  {
    int i;
    for (i=0; i < self->size; i++)
    { printf(" %i : %p\n", i, self->items[i]); }
  }
  printf("\n");
}

void Queue_do_(Queue *self, QueueDoCallback *callback)
{
  int i, max = self->bottom;
  for (i=self->top; i<max; i++) (*callback)(self->items[i]);
}

void *Queue_firstTrueFor_with_(Queue *self, 
  QueueFirstTrueForWithCallback *callback, void *arg)
{
  int i, max = self->bottom;
  for (i=self->top; i<max; i++)
  {
    void *item = self->items[i];
    if ((*callback)(item, arg)) return item;
  }
  return 0x0;
}

#ifdef QUEUE_TEST
void main(void)
{
  Queue *q = Queue_new();
  Queue_push_(q, (void *)0x1); Queue_print(q);
  Queue_push_(q, (void *)0x2); Queue_print(q);

  printf("pop %p\n", Queue_pop(q)); Queue_print(q);
  printf("pop %p\n", Queue_pop(q)); Queue_print(q);
  printf("pop %p\n", Queue_pop(q)); Queue_print(q);
  
  Queue_push_(q, (void *)0x3); Queue_print(q);
  Queue_push_(q, (void *)0x4); Queue_print(q);
  Queue_push_(q, (void *)0x5); Queue_print(q);

  printf("pop %p\n", Queue_pop(q)); Queue_print(q);
  printf("pop %p\n", Queue_pop(q)); Queue_print(q);
  printf("pop %p\n", Queue_pop(q)); Queue_print(q);
}
#endif

