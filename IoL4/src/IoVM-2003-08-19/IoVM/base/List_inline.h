/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 *
 * List - an array of void pointers
 * User is responsible for freeing items
 */

#ifdef LIST_C 
#define IO_IN_C_FILE
#endif
#include "Common_inline.h"
#ifdef IO_DECLARE_INLINES

IOINLINE int List_count(List *self) { return self->count; }

IOINLINE void *List_at_(List *self, int index)
{
  if ( index > -1 && index < self->count)
  { return self->items[index]; }
  else
  { return (void *)0x0; }
}

IOINLINE int List_indexOf_(List *self, void *item)
{
  int index = 0;
  while (index < self->count)
  {
    if (self->items[index] == item) { return index; }
    index++;
  }
  return -1;
}

IOINLINE int List_contains_(List *self, void *item)
{
  int index = 0;
  while (index < self->count)
  {
    if (self->items[index] == item) { return 1; }
    index++;
  }
  return 0;
}

IOINLINE void *List_add_(List *self, void *item)
{
  List_sizeTo_(self, self->count+1);
  self->items[self->count] = item;
  self->count++;
  return item;
}

IOINLINE void List_addList_(List *self, List *otherList)
{
  int i;
  for (i=0; i < otherList->count; i++)
  { List_add_(self, LIST_AT_(otherList, i)); }
}

IOINLINE void List_removeIndex_(List *self, int index)
{
  if (index >= 0 && index < self->count)
  {
    if ( index != self->count -1)
    {
      memmove(&self->items[index], &self->items[index+1], 
        (self->count - 1 - index)*sizeof(void *));
    }
    self->count--;
  }
}

IOINLINE void List_remove_(List *self, void *item)
{
  int index;
  for (index = 0; index < self->count; index++)
  {
    if (self->items[index] == item) 
    { List_removeIndex_(self, index); }
  }
}

IOINLINE void List_removeLast_(List *self, void *item)
{
  int index = self->count - 1;
  for (index = self->count - 1; index > -1; index--)
  {
    if (self->items[index] == item) 
    { List_removeIndex_(self, index); break; }
  }
}

IOINLINE void List_removeItems_(List *self, List *other)
{
  int i;
  int count = other->count;
  void **items = other->items;
  for (i=0; i<count; i++) 
  { 
    void *item = items[i];
    List_remove_(self, item);
  }
}

IOINLINE void List_at_insert_(List *self, int index, void *item)
{
  if (index < 0) { return; }
  if (index > self->count) 
  { List_sizeTo_(self, index+1); }
  else
  { List_sizeTo_(self, self->count+1); }
  
  memmove(&self->items[index+1], &self->items[index], 
    (self->count - index)*sizeof(void *));
  self->items[index] = item;
  self->count++;
}

IOINLINE void List_at_put_(List *self, int index, void *item)
{
  if (index < 0) { return; }
  List_sizeTo_(self, index);
  self->items[index] = item;
  if (index+1 > self->count) { self->count = index+1; }
}

IOINLINE void List_swap_with_(List *self, int index1, int index2)
{
  if (index1 < 0 || index2 < 0) { return; }
  if (index1 != index2)
  {
    void **items = self->items;
    void *v1 = items[index1];
    items[index1] = items[index2];
    items[index2] = v1;
  }
}

IOINLINE void List_reverse(List *self)
{
  void **i = self->items;
  void **j = i + (self->count-1);
  void *iv;
  while (j > i)
  {
    iv = *i;
    *i = *j;
    *j = iv;
    j--;
    i++;
  }
}

/* --- stack -------------------------------------------------- */

IOINLINE void List_push_(List *self, void *item)
{
  List_sizeTo_(self, self->count+1);
  self->items[self->count] = item;
  self->count++;
}

IOINLINE void *List_pop(List *self)
{
  if (!self->count) { return (void *)NULL; }
  self->count--;
  return self->items[self->count];
}

IOINLINE void *List_top(List *self)
{ 
  if (!self->count) return 0x0;
  return self->items[self->count-1]; 
}

/* --- perform -------------------------------------------------- */

IOINLINE int List_removeTrueFor_(List *self, ListCollectCallback* callback)
{
  int getIndex = 0;
  int putIndex = 0;
  int count = self->count;
  void **items = self->items;
  while (getIndex<count) 
  { 
    void *item = items[getIndex];
    if ( item && !((*callback)(item)) ) 
    {
      if (getIndex!=putIndex)
      { items[putIndex] = item; }
      putIndex++; 
    }
    getIndex++;
  }
  self->count = putIndex;
  return getIndex - putIndex;
}

IOINLINE void List_qsort(List *self, ListSortCallback *callback)
{ qsort(self->items, self->count, sizeof(void *), *callback); }

IOINLINE void *List_bsearch(List *self, const void *key, ListSortCallback *callback)
{ return bsearch(key, self->items, self->count, sizeof(void *), callback); }

#undef IO_IN_C_FILE
#endif


