/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#ifndef IONIL_DEFINED
#define IONIL_DEFINED 1

#include "base/Common.h"
#include "IoObject.h"

#define ISNIL(self) (self->tag->cloneFunc == (TagCloneFunc *)IoNil_rawClone)
#define IONIL(self) ((IoState *)IOSTATE)->ioNil


IoObject *IoNil_proto(void *state);
IoObject *IoNil_rawClone(IoObject *self);
IoObject *IoNil_new(void *state);
void IoNil_printNil(IoObject *self);

IoObject *IoNil_and(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoNil_or(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoNil_print(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoNil_isEqual(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoNil_isNotEqual(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoNil_isNil(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoNil_ifNil(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoNil_ifTrue(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoNil_ifFalse(IoObject *self, IoObject *locals, IoMessage *m);

#endif
