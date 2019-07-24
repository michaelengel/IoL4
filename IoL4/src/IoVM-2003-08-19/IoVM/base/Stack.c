/*  Copyright (c) 2002, Steve Dekorte
 *  All rights reserved. See _BSDLicense.txt.
 *  Notes: first element of items is always 0x0
 */
 
#define STACK_C
#include "Stack.h" 
#undef STACK_C

Stack *Stack_new(void)
{
  int size = sizeof(void *)*STACK_START_SIZE;
  Stack *self = (Stack *)calloc(1, sizeof(Stack));
  self->items = calloc(1, size);
  self->memEnd = self->items + (size/sizeof(void *));
  self->top = self->items;
  /*self->lastMark = self->items;*/
  return self;
}

void Stack_free(Stack *self)
{
  free(self->items);
  free(self);
}

/* --- stack -------------------------------------------------- */

size_t Stack_memorySize(Stack *self)
{ return sizeof(Stack) + (self->memEnd - self->items); }

void Stack_compact(Stack *self) 
{
  int oldSize = (1 + self->top - self->items)*sizeof(void *);
  self->items = realloc(self->items, oldSize);
}

void Stack_resize(Stack *self)
{
  int oldSize = (self->memEnd - self->items)*sizeof(void *);
  int newSize = oldSize*STACK_RESIZE_FACTOR;
  int i = self->top - self->items;
  self->items = realloc(self->items, newSize);
  self->top = self->items + i;
  self->memEnd = self->items + (newSize/sizeof(void *));
}

void Stack_clear(Stack *self) { self->top = self->items; }
size_t Stack_totalSize(Stack *self) { return (self->top - self->items); }

/* --- sizing ------------------------------------------------ */

void Stack_do_on_(Stack *self, StackDoOnCallback *callback, void *target)
{
  Stack *stack = Stack_newCopyWithoutMarks(self);
  int i;
  for(i = 0; i < Stack_count(stack); i++)
  { (*callback)(target, Stack_at_(stack, i)); }
  Stack_free(stack);
}

Stack *Stack_newCopyWithoutMarks(Stack *self)
{
  Stack *newStack = Stack_new();
  void **itemP = self->top;
  ptrdiff_t mark = self->lastMark;
  while (itemP > self->items)
  { 
    if (itemP - self->items == mark) 
    { mark = (ptrdiff_t)(*itemP); } 
    else 
    { Stack_push_(newStack,*itemP); }
    itemP --;
  }
  return newStack;
}

