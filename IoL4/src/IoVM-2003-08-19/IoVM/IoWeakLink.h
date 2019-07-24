/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#ifndef IOWEAKLINK_DEFINED
#define IOWEAKLINK_DEFINED 1

#include "base/Common.h"
#include "IoObject.h"

#define ISWEAKLINK(self) (self->tag->cloneFunc == (TagCloneFunc *)IoWeakLink_rawClone)

typedef IoObject IoWeakLink;

typedef struct 
{
  IoWeakLink *previousWeakLink; /* linked list of weaklinks pointing to the same value */
  IoWeakLink *nextWeakLink;  
  IoObject *link;
  IoObject *delegate;
  IoObject *identifier;
} IoWeakLinkData;

IoObject *IoWeakLink_proto(void *state);
IoObject *IoWeakLink_new(void *state);

IoObject *IoWeakLink_rawClone(IoWeakLink *self);
void IoWeakLink_free(IoWeakLink *self);
void IoWeakLink_mark(IoWeakLink *self);

IoObject *IoWeakLink_rawLink(IoWeakLink *self);
IoObject *IoWeakLink_rawNextWeakLink(IoWeakLink *self);

/* --- special weaklink methods ----------------------------- */
void IoWeakLink_appendWeakLink_(IoWeakLink *self, IoObject *other);
void IoWeakLink_unlink(IoObject *self);
void IoWeakLink_collected(IoWeakLink *self);
void IoWeakLink_unlinkItem_(IoWeakLink *self, IoObject *other);

/* ----------------------------------------------------------- */
IoObject *IoWeakLink_type(IoWeakLink *self, IoObject *locals, IoMessage *m);
IoObject *IoWeakLink_clone(IoWeakLink *self, IoObject *locals, IoMessage *m);
void IoWeakLink_didCollect(IoWeakLink *self);

/* ----------------------------------------------------------- */
IoObject *IoWeakLink_setLink(IoWeakLink *self, IoObject *locals, IoMessage *m);
IoObject *IoWeakLink_link(IoWeakLink *self, IoObject *locals, IoMessage *m);

IoObject *IoWeakLink_setDelegate(IoWeakLink *self, IoObject *locals, IoMessage *m);
IoObject *IoWeakLink_delegate(IoWeakLink *self, IoObject *locals, IoMessage *m);

IoObject *IoWeakLink_setIdentifier(IoWeakLink *self, IoObject *locals, IoMessage *m);
IoObject *IoWeakLink_identifier(IoWeakLink *self, IoObject *locals, IoMessage *m);

#endif
