/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */
 
#define LIST_C
#include "List.h"
#undef LIST_C
#include <stdlib.h>

List *List_new(void)
{
  List *self = (List *)calloc(1, sizeof(List));
  self->count = 0;
  self->memSize = sizeof(void *)*LIST_START_SIZE;
  self->items = calloc(1, self->memSize);
  return self;
}

List *List_clone(List *self)
{
  /*
  List *child = List_new();
  List_copy_(child, self);
  */
  List *child = cpalloc(self, sizeof(List));
  child->items = cpalloc(self->items, self->memSize);
  return child;
}

void List_free(List *self)
{
  free(self->items);
  free(self);
}

void List_clear(List *self) { self->count = 0; }

void List_copy_(List *self, List *otherList)
{
  if (self == otherList || !otherList->count) { return; }
  List_sizeTo_(self, otherList->count);
  memmove(self->items, otherList->items, sizeof(void *)*(otherList->count));
  self->count = otherList->count;
}

int List_equals_(List *self, List *otherList)
{
  return (self->count == otherList->count &&
    memcmp(self->items, otherList->items, sizeof(void *)*self->count) == 0);
}

/* --- sizing ------------------------------------------------ */

void List_setLength_(List *self, int index)
{ self->count = index; }

void List_sizeTo_(List *self, int index)
{
  int s = index*sizeof(void *);
  if (s >= self->memSize) 
  {
    int newSize = self->memSize*LIST_RESIZE_FACTOR;
    if (s > newSize) { newSize = s; }
    self->items = realloc(self->items, newSize); 
    memset(self->items + self->count, 0x0, ((newSize/sizeof(void *)) - self->count));
    self->memSize = newSize;
  }
}

void List_compact(List *self)
{
  self->memSize = self->count*sizeof(void *);
  self->items = realloc(self->items, self->memSize); 
}

/* ----------------------------------------------------------- */

void List_removeItemsAfterLastNULL_(List *self)
{
  int i;
  void **items = self->items;
  for (i=self->count-1; i>-1; i--) 
  { if (!items[i]) { break; } }
  self->count = i;
}

void List_print(List *self)
{
  int i;
  printf("List <%ld> [%i bytes]\n", (long)self, (int)self->memSize);
  for (i=0; i<self->count; i++)
  { printf("%i: %ld\n", i, (long)self->items[i]); }
  printf("\n");
}

/* --- perform -------------------------------------------------- */

void List_do_(List *self, ListDoCallback *callback)
{
  int i;
  int count = self->count;
  void **items = self->items;
  for (i=0; i<count; i++) 
  { if (items[i]) (*callback)(items[i]); }
}

void List_do_with_(List *self, ListDoWithCallback *callback, void *arg)
{
  int i;
  int count = self->count;
  void **items = self->items;
  for (i=0; i<count; i++) { if (items[i]) (*callback)(items[i], arg); }
}

List *List_collect_(List *self, ListCollectCallback *callback)
{
  List *collection = List_new();
  int i;
  int count = self->count;
  void **items = self->items;
  for (i=0; i<count; i++) 
  { 
    void *item = items[i];
    if (item && (*callback)(item)) { if (item) List_add_(collection, item); }
  }
  return collection;
}

void *List_detect_(List *self, ListDetectCallback *callback)
{
  int i;
  int count = self->count;
  void **items = self->items;
  for (i=0; i<count; i++) 
  { 
    void *item = items[i];
    if (item && (*callback)(item)) return item;
  }
  return (void *)0x0;
}


void *List_random(List *self)
{ 
  int i;
  if (self->count == 0) return (void *)0x0; 
  if (self->count == 1) return LIST_AT_(self, 0); 
  i = (rand()>>4)%(self->count); /* without the shift, just get a sequence! */
  return LIST_AT_(self, i);
}

void List_randomize(List *self)
{ 
  int i, j;
  for (i = 0; i < self->count - 1; i++)
  {
    j = i + rand()%(self->count - i); 
    List_swap_with_(self, i, j);
  }
}

void *List_removeLast(List *self)
{
  void *item = List_at_(self, self->count-1);
  if (item) self->count --;
  return item;
}
