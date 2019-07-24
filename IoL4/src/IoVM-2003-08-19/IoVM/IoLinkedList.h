/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#ifndef IOLINKEDLIST_DEFINED
#define IOLINKEDLIST_DEFINED 1

#include "base/Common.h"
#include "base/LinkedList.h"
#include "IoObject.h"

#define ISLINKEDLIST(v) \
  (self->tag->cloneFunc == (TagCloneFunc *)IoLinkedList_rawClone)

typedef IoObject IoLinkedList;

typedef struct
{
  LinkedList *list;
} IoLinkedListData;

IoLinkedList *IoLinkedList_proto(void *state);
IoLinkedList *IoLinkedList_rawClone(IoLinkedList *self);
IoLinkedList *IoLinkedList_new(void *state);
void IoLinkedList_free(IoLinkedList *self);
void IoLinkedList_mark(IoLinkedList *self);
int IoLinkedList_compare(IoLinkedList *self, IoLinkedList *otherList);
/* ----------------------------------------------------------- */
IoObject *IoLinkedList_type(IoLinkedList *self, IoObject *locals, IoMessage *m);
IoObject *IoLinkedList_clone(IoLinkedList *self, IoObject *locals, IoMessage *m);

IoObject *IoLinkedList_protoPrint(IoLinkedList *self, IoObject *locals, IoMessage *m);
IoObject *IoLinkedList_add_(IoLinkedList *self, IoObject *locals, IoMessage *m);
IoObject *IoLinkedList_remove_(IoLinkedList *self, IoObject *locals, IoMessage *m);
IoObject *IoLinkedList_indexOf_(IoLinkedList *self, IoObject *locals, IoMessage *m);
IoObject *IoLinkedList_contains(IoLinkedList *self, IoObject *locals, IoMessage *m);
IoObject *IoLinkedList_push_(IoLinkedList *self, IoObject *locals, IoMessage *m);
IoObject *IoLinkedList_pop(IoLinkedList *self, IoObject *locals, IoMessage *m);
IoObject *IoLinkedList_clear(IoLinkedList *self, IoObject *locals, IoMessage *m);
IoObject *IoLinkedList_count(IoLinkedList *self, IoObject *locals, IoMessage *m);
IoObject *IoLinkedList_insert_at_(IoLinkedList *self, IoObject *locals, IoMessage *m);
IoObject *IoLinkedList_removeAt_(IoLinkedList *self, IoObject *locals, IoMessage *m);
IoObject *IoLinkedList_at_(IoLinkedList *self, IoObject *locals, IoMessage *m);
IoObject *IoLinkedList_at_put_(IoLinkedList *self, IoObject *locals, IoMessage *m);
IoObject *IoLinkedList_clear(IoLinkedList *self, IoObject *locals, IoMessage *m);
IoObject *IoLinkedList_contains_(IoLinkedList *self, IoObject *locals, IoMessage *m);

IoObject *IoLinkedList_swapIndex_withIndex_(IoLinkedList *self, IoObject *locals, IoMessage *m);
IoObject *IoLinkedList_reverse(IoLinkedList *self, IoObject *locals, IoMessage *m);
IoObject *IoLinkedList_sort(IoLinkedList *self, IoObject *locals, IoMessage *m);
IoObject *IoLinkedList_sortBy_(IoLinkedList *self, IoObject *locals, IoMessage *m);
IoObject *IoLinkedList_do_(IoLinkedList *self, IoObject *locals, IoMessage *m);

IoObject *IoLinkedList_foreach(IoLinkedList *self, IoObject *locals, IoMessage *m);
IoObject *IoLinkedList_reverseForeach(IoLinkedList *self, IoObject *locals, IoMessage *m);
IoObject *IoLinkedList_random(IoLinkedList *self, IoObject *locals, IoMessage *m);
IoObject *IoLinkedList_randomize(IoLinkedList *self, IoObject *locals, IoMessage *m);
IoObject *IoLinkedList_translate(IoLinkedList *self, IoObject *locals, IoMessage *m);

#endif
