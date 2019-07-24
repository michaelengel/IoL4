/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 *
 * List - an array of void pointers
 * User is responsible for freeing items
 */

#ifndef LIST_DEFINED
#define LIST_DEFINED 1

#include "Common.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef LOW_MEMORY_SYSTEM
#define LIST_START_SIZE 128*4
#define LIST_RESIZE_FACTOR 1.5
#else
#define LIST_START_SIZE 128*4
#define LIST_RESIZE_FACTOR 2
#endif

#define LIST_AT_(self, n) self->items[n]

typedef void (ListDoCallback)(void *);
typedef void (ListDoWithCallback)(void *, void *);
typedef int (ListCollectCallback)(void *);
typedef int (ListDetectCallback)(void *);
typedef int (ListSortCallback)(const void *, const void *);

typedef struct
{
  void **items;
  int memSize;
  int count;
} List;

List *List_new(void);
List *List_clone(List *self);
void List_free(List *self);
void List_clear(List *self);
void List_copy_(List *self, List *otherList);
int List_equals_(List *self, List *otherList);

/* --- sizing ------------------------------------------------ */
void List_sizeTo_(List *self, int index);
void List_setLength_(List *self, int index);
void List_compact(List *self);

/* ----------------------------------------------------------- */
void List_print(List *self);
void List_removeItemsAfterLastNULL_(List *self);

/* --- perform -------------------------------------------------- */
void List_do_(List *self, ListDoCallback *callback);
void List_do_with_(List *self, ListDoWithCallback *callback, void *arg);
List *List_collect_(List *self, ListCollectCallback *callback);
void *List_detect_(List *self, ListDetectCallback *callback);

void *List_random(List *self);
void List_randomize(List *self);
void *List_removeLast(List *self);

#include "List_inline.h"

#endif
