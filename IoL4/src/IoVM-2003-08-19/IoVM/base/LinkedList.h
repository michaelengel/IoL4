/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 *
 * LinkedList - A  double linked list
 * each item holds a single value
 * a count is maintained so asking for a count is fast
 *
 */

#ifndef DLINKEDLIST_DEFINED
#define DLINKEDLIST_DEFINED 1

#include "Common.h"
#include <stdio.h>

typedef int (LinkedListSortCallback)(const void *, const void *);
typedef int (LinkedListCompareCallback)(const void *, const void *);

typedef struct LinkedListItem LinkedListItem;

struct LinkedListItem
{
  void *value;
  LinkedListItem *previous;
  LinkedListItem *next;
};

/* ----------------------------------------------------------- */

typedef void (LinkedListDoCallback)(void *);
typedef unsigned char (LinkedListFirstTrueForCallback)(void *);
typedef unsigned char (LinkedListFirstTrueForWithCallback)(void *, void *);

typedef struct
{
  LinkedListItem *first;
  LinkedListItem *last;
  int count;
} LinkedList;

LinkedList *LinkedList_new(void);
LinkedList *LinkedList_clone(LinkedList *self);
void LinkedList_copy_(LinkedList *self, LinkedList *other);

void LinkedList_free(LinkedList *self);
size_t LinkedList_memorySize(LinkedList *self);
void LinkedList_compact(LinkedList *self);

void *LinkedList_add_(LinkedList *self, void *value);

void *LinkedList_prepend_(LinkedList *self, void *value);
void *LinkedList_at_(LinkedList *self, int i);

/* removes first occurance, returns 1 if found, 0 otherwise */
int LinkedList_remove_(LinkedList *self, void *value); 

void *LinkedList_popFirst(LinkedList *self);
void *LinkedList_popLast(LinkedList *self);

void *LinkedList_first(LinkedList *self);
void *LinkedList_last(LinkedList *self);

void LinkedList_do_(LinkedList *self, LinkedListDoCallback *callback);
void *LinkedList_firstTrueFor_(LinkedList *self, LinkedListFirstTrueForCallback *callback);
void *LinkedList_firstTrueFor_with_(LinkedList *self, 
  LinkedListFirstTrueForWithCallback *callback, void *arg);
  
int LinkedList_indexOf_(LinkedList *self, void *value); 
void LinkedList_push_(LinkedList *self, void *value); 
void *LinkedList_pop(LinkedList *self); 
void *LinkedList_removeIndex_(LinkedList *self, int i); 
void LinkedList_at_put_(LinkedList *self, int i, void *value); 
void LinkedList_at_insert_(LinkedList *self, int i, void *value); 
void LinkedList_clear(LinkedList *self); 
void LinkedList_swap_with_(LinkedList *self, int index1, int index2); 
void LinkedList_reverse(LinkedList *self); 
void LinkedList_qsort(LinkedList *self, LinkedListSortCallback *callback); 
int LinkedList_compare_(LinkedList *self, LinkedList *other, LinkedListCompareCallback *compareFunc);

#include "LinkedList_inline.h"

#endif

