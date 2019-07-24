/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#ifdef QUEUE_C 
#define IO_IN_C_FILE
#endif
#include "Common_inline.h"
#ifdef IO_DECLARE_INLINES


IOINLINE void Queue_clear(Queue *self) 
{ 
  self->top = 0; 
  self->bottom = 0; 
  self->items[0] = 0x0;
}

IOINLINE void Queue_push_(Queue *self, void *v)
{
  if (self->bottom == self->size)
  {
    if (self->top > self->size * .25)
    {
      memmove(self->items, self->items + self->top, self->bottom - self->top);
      self->bottom -= self->top;
      self->top = 0;
    }
    else
    {
      self->size *= QUEUE_RESIZE_FACTOR;
      self->items = realloc(self->items, self->size*sizeof(void *));
    }
  }
  self->items[self->bottom] = v;
  self->bottom++;
}

IOINLINE void *Queue_top(Queue *self) { return self->items[self->top]; }

IOINLINE void *Queue_pop(Queue *self)
{
  void *v;
  if (self->top == self->bottom)
  {
    if (self->top != 0) { self->top = 0; self->bottom = 0; self->items[0] = 0x0; }
    return 0x0;
  }
  v = self->items[self->top];
  self->items[self->top] = 0x0; /* temp */
  self->top++;
  if (self->top == self->bottom) { self->top = 0; self->bottom = 0; }
  return v;
}

#undef IO_IN_C_FILE
#endif


