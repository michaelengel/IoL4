/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#define LINKEDLIST_C
#include "LinkedList.h"
#undef LINKEDLIST_C
#include <stdlib.h>
#include <stdio.h>
#include <string.h> 

LinkedListItem *LinkedListItem_newWithValue_(void *v)
{
  LinkedListItem *self = (LinkedListItem *)calloc(1, sizeof(LinkedListItem));
  self->value = v;
  return self;
}

void LinkedListItem_free(LinkedListItem *self)
{ 
  memset(self, 0x0, sizeof(LinkedListItem));
  free(self); 
}

void LinkedListItem_reverse(LinkedListItem *self)
{ 
  LinkedListItem *item = self->next;
  self->next = self->previous;
  self->previous = item;
}

void LinkedListItem_next_(LinkedListItem *self, LinkedListItem *item)
{ 
  if (self->next) self->next->previous = item;
  self->next = item;
  item->previous = self;
}

void LinkedListItem_previous_(LinkedListItem *self, LinkedListItem *item)
{ 
  if (self->previous) self->previous->next = item;
  self->previous = item;
  item->next = self;
}


/* ----------------------------------------------------------- */

LinkedList *LinkedList_new(void)
{
  LinkedList *self = (LinkedList *)calloc(1, sizeof(LinkedList));
  return self;
}

LinkedList *LinkedList_clone(LinkedList *self)
{
  LinkedList *child = LinkedList_new();
  LinkedList_copy_(child, self);
  return child;
}

void LinkedList_freeItems(LinkedList *self)
{
  LinkedListItem *item = self->first;
  while (item)
  {
    LinkedListItem *next = item->next;
    LinkedListItem_free(item); 
    item = next;
  }
  self->first = 0x0;
  self->last = 0x0;
  self->count = 0;
}

void LinkedList_free(LinkedList *self)
{
  LinkedList_freeItems(self);
  free(self);
}

size_t LinkedList_memorySize(LinkedList *self)
{ return sizeof(LinkedList) + (self->count*sizeof(LinkedListItem)); }

void LinkedList_compact(LinkedList *self) {}


void *LinkedList_add_(LinkedList *self, void *value)
{
  LinkedListItem *item = LinkedListItem_newWithValue_(value);
  if (self->last) 
  { 
    LinkedListItem_next_(self->last, item); 
    self->last = item; 
  }
  else 
  { self->last = item; }
  if (!self->first) self->first = item;
  self->count++;
  return value;
}

void *LinkedList_prepend_(LinkedList *self, void *value)
{
  LinkedListItem *item = LinkedListItem_newWithValue_(value);
  if (self->first) 
  { 
    LinkedListItem_previous_(self->first, item); 
    self->first = item; 
  }
  else 
  { self->first = item; }
  if (!self->last) self->last = item;
  self->count++;
  return value;
}

void LinkedList_removeItem_(LinkedList *self, LinkedListItem *item)
{
  if (item == self->first) 
  {
    self->first = item->next;
    if (self->first) self->first->previous = 0x0;
  }
  if (item == self->last) 
  {
    self->last = item->previous;
    if (self->last) self->last->next = 0x0;
  }
  if (item->previous) item->previous->next = item->next;
  if (item->next) item->previous = item->previous;
  LinkedListItem_free(item);
  self->count--;
  /*
  if (self->first && self->first->previous)
  { printf("self->first->previous?\n"); }
  if (self->last && self->last->next)
  { printf("self->last->next?\n"); }
  */
  /*printf("%p remove %i\n", self, self->count);*/
}

int LinkedList_remove_(LinkedList *self, void *value)
{
  LinkedListItem *item = self->first;
  while (item)
  {
    if (item->value == value)
    { 
      LinkedList_removeItem_(self, item);
      return 1;
    }
    item = item->next;
  }
  return 0;
}

LinkedListItem *LinkedList_itemAt_(LinkedList *self, int index)
{
  LinkedListItem *item = self->first;
  int i = 0;
  if (index >= self->count) return 0x0;
  while (item && i <= index)
  {
    if (i == index) { return item; }
    i++;
    item = item->next;
  }
  return 0x0;
}

void *LinkedList_at_(LinkedList *self, int i)
{
  LinkedListItem *item = LinkedList_itemAt_(self, i);
  if (item) return item->value;
  return 0x0;
}

void *LinkedList_popFirst(LinkedList *self)
{
  void *v;
  if (!self->first) return 0x0;
  v = self->first->value;
  LinkedList_removeItem_(self, self->first);
  return v;
}

void *LinkedList_popLast(LinkedList *self)
{
  void *v;
  if (!self->last) return 0x0;
  v = self->last->value;
  LinkedList_removeItem_(self, self->last);
  return v;
}

void *LinkedList_first(LinkedList *self) 
{
  if (!self->first) return 0x0;
  return self->first->value;
}

void *LinkedList_last(LinkedList *self)
{
  if (!self->last) return 0x0;
  return self->last->value;
}

void LinkedList_do_(LinkedList *self, LinkedListDoCallback *callback)
{
  LinkedListItem *item = self->first;
  while (item) 
  { 
    (*callback)(item->value);
    item = item->next; 
  }
}

void *LinkedList_firstTrueFor_(LinkedList *self, LinkedListFirstTrueForCallback *callback)
{
  LinkedListItem *item = self->first;
  while (item) 
  { 
    if ((*callback)(item->value)) return item;
    item = item->next; 
  }
  return 0x0;
}

void *LinkedList_firstTrueFor_with_(LinkedList *self, 
  LinkedListFirstTrueForWithCallback *callback, void *arg)
{
  LinkedListItem *item = self->first;
  while (item) 
  { 
    if ((*callback)(item->value, arg)) return item;
    item = item->next; 
  }
  return 0x0;
}


/* ------------------------------------------------ */

int LinkedList_indexOf_(LinkedList *self, void *value)
{
  LinkedListItem *item = self->first;
  int i = 0;
  while (item)
  {
    if (item->value == value) return i;
    item = item->next;
    i++;
  }
  return -1;
}

void LinkedList_push_(LinkedList *self, void *value)
{ LinkedList_add_(self, value); }

void *LinkedList_pop(LinkedList *self)
{ return LinkedList_popLast(self); }

void *LinkedList_removeIndex_(LinkedList *self, int i)
{
  LinkedListItem *item = LinkedList_itemAt_(self, i);
  void *v = 0x0;
  if (item)
  {
    v = item->value;
    LinkedList_removeItem_(self, item);
  }
  return v;
}

void LinkedList_at_put_(LinkedList *self, int i, void *value)
{
  LinkedListItem *item = LinkedList_itemAt_(self, i);
  if (item) item->value = value;
}

void LinkedList_at_insert_(LinkedList *self, int i, void *value)
{
  LinkedListItem *item = LinkedList_itemAt_(self, i);
  if (item) 
  {
    LinkedListItem *newItem = LinkedListItem_newWithValue_(value);
    LinkedListItem_next_(item, newItem);
    self->count++;
  }
}

void LinkedList_clear(LinkedList *self)
{ LinkedList_freeItems(self); }
 
void LinkedList_swap_with_(LinkedList *self, int index1, int index2)
{
  LinkedListItem *item1 = LinkedList_itemAt_(self, index1);
  LinkedListItem *item2 = LinkedList_itemAt_(self, index2);
  if (item1 && item2)
  {
    void *v = item1->value;
    item1->value = item2->value;
    item2->value = v;
  }
}

void LinkedList_reverse(LinkedList *self)
{
  LinkedListItem *item = self->first;
  while (item)
  {
    LinkedListItem *next = item->next;
    LinkedListItem_reverse(item);
    item = next;
  }
  item = self->first;
  self->first = self->last;
  self->last = item;
}


void LinkedList_qsort(LinkedList *self, LinkedListSortCallback *callback)
{
  printf("LinkedList_qsort not implemented\n");
  exit(1);
}

void LinkedList_copy_(LinkedList *self, LinkedList *other)
{
  LinkedListItem *otherItem = LinkedList_itemAt_(other, 0);
  LinkedList_clear(self);  
  while (otherItem)
  {
    LinkedList_add_(self, otherItem->value);
    otherItem = otherItem->next;
  }
}

int LinkedList_compare_(LinkedList *self, LinkedList *other, LinkedListCompareCallback *compareFunc)
{
  LinkedListItem *item = self->first;
  LinkedListItem *otherItem = other->first;
  while (item && otherItem)
  {
    int c = (compareFunc)(item->value, otherItem->value);
    if (c) 
    { return c; }
    item = item->next;
    otherItem = otherItem->next;
  }
  if ((item != self->last) || (otherItem != other->last)) 
  { return (self->count - other->count); }
  return 0;
}



