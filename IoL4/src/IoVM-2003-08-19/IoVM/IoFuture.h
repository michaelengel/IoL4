/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#ifndef IOFUTURE_DEFINED
#define IOFUTURE_DEFINED 1

#include "base/Common.h"
#include "IoObject.h"
#include "IoState.h"
#include "IoMessage.h"

#define ISFUTURE(self) (self->tag->cloneFunc == (TagCloneFunc *)IoFuture_rawClone)

typedef IoObject IoFuture;

typedef struct
{
  IoMessage *message;
  IoObject *value;
  IoObject *creatorObject;
  IoObject *pausedObject;
  /* for sendResultTo() */
  IoObject *target;
  IoString *methodName;
} IoFutureData;

IoFuture *IoFuture_proto(void *state);
IoFuture *IoFuture_rawClone(IoFuture *self);
IoFuture *IoFuture_new(void *state);
IoFuture *IoFuture_newWithMessage_creatorObject_(IoState *state, IoMessage *message, IoObject *creatorObject);
void IoFuture_mark(IoFuture *self);
void IoFuture_free(IoFuture *self);
IoMessage *IoFuture_rawMessage(IoFuture *self);

void IoFuture_rawSetValue_(IoFuture *self, IoObject *v);
unsigned char IoFuture_isPausing_(IoFuture *self, IoObject *v);

IoObject *IoFuture_value(IoFuture *self, IoObject *locals, IoMessage *m);
IoObject *IoFuture_setValue(IoFuture *self, IoObject *locals, IoMessage *m);
IoObject *IoFuture_isReady(IoFuture *self, IoObject *locals, IoMessage *m);
IoObject *IoFuture_sendResultTo(IoFuture *self, IoObject *locals, IoMessage *m);

#endif
