/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#ifndef IOLIST_DEFINED
#define IOLIST_DEFINED 1

#include "base/Common.h"
#include "IoObject.h"

#define ISLIST(self) \
  (self->tag->cloneFunc == (TagCloneFunc *)IoList_rawClone)

typedef IoObject IoList;

typedef struct
{
  List *list;
} IoListData;

IoList *IoList_proto(void *state);
IoList *IoList_rawClone(IoList *self);
IoList *IoList_new(void *state);
void IoList_free(IoList *self);
void IoList_mark(IoList *self);
int IoList_compare(IoList *self, IoList *otherList);

List *IoList_rawList(IoList *self);
IoObject *IoList_rawAt_(IoList *self, int i);
void IoList_rawAdd_(IoList *self, IoObject *v);
void IoList_rawAddList_(IoList *self, IoList *other);
size_t IoList_rawCount(IoList *self);

/* ----------------------------------------------------------- */
IoObject *IoList_type(IoList *self, IoObject *locals, IoMessage *m);
IoObject *IoList_clone(IoList *self, IoObject *locals, IoMessage *m);

IoObject *IoList_add_(IoList *self, IoObject *locals, IoMessage *m);
IoObject *IoList_remove_(IoList *self, IoObject *locals, IoMessage *m);
IoObject *IoList_indexOf_(IoList *self, IoObject *locals, IoMessage *m);
IoObject *IoList_contains(IoList *self, IoObject *locals, IoMessage *m);
IoObject *IoList_push_(IoList *self, IoObject *locals, IoMessage *m);
IoObject *IoList_pop(IoList *self, IoObject *locals, IoMessage *m);
IoObject *IoList_clear(IoList *self, IoObject *locals, IoMessage *m);
IoObject *IoList_count(IoList *self, IoObject *locals, IoMessage *m);
IoObject *IoList_insert_at_(IoList *self, IoObject *locals, IoMessage *m);
IoObject *IoList_removeAt_(IoList *self, IoObject *locals, IoMessage *m);
IoObject *IoList_at_(IoList *self, IoObject *locals, IoMessage *m);
IoObject *IoList_at_put_(IoList *self, IoObject *locals, IoMessage *m);
IoObject *IoList_clear(IoList *self, IoObject *locals, IoMessage *m);
IoObject *IoList_contains_(IoList *self, IoObject *locals, IoMessage *m);

IoObject *IoList_swapIndex_withIndex_(IoList *self, IoObject *locals, IoMessage *m);
IoObject *IoList_reverse(IoList *self, IoObject *locals, IoMessage *m);
IoObject *IoList_sort(IoList *self, IoObject *locals, IoMessage *m);
IoObject *IoList_sortBy_(IoList *self, IoObject *locals, IoMessage *m);
IoObject *IoList_do_(IoList *self, IoObject *locals, IoMessage *m);
IoObject *IoList_doBlock_(IoList *self, IoObject *locals, IoMessage *m);
IoObject *IoList_select(IoList *self, IoObject *locals, IoMessage *m);
IoObject *IoList_reject(IoList *self, IoObject *locals, IoMessage *m);
IoObject *IoList_detect(IoList *self, IoObject *locals, IoMessage *m);

IoObject *IoList_foreach(IoList *self, IoObject *locals, IoMessage *m);
IoObject *IoList_reverseForeach(IoList *self, IoObject *locals, IoMessage *m);
IoObject *IoList_random(IoList *self, IoObject *locals, IoMessage *m);
IoObject *IoList_randomize(IoList *self, IoObject *locals, IoMessage *m);

IoObject *IoList_sizeTo_(IoList *self, IoObject *locals, IoMessage *m);
IoObject *IoList_first(IoList *self, IoObject *locals, IoMessage *m);
IoObject *IoList_last(IoList *self, IoObject *locals, IoMessage *m);
IoObject *IoList_protoPrint(IoList *self, IoObject *locals, IoMessage *m);

IoObject *IoList_translateBlock(IoList *self, IoObject *locals, IoMessage *m);
IoObject *IoList_translate(IoList *self, IoObject *locals, IoMessage *m);

#endif
