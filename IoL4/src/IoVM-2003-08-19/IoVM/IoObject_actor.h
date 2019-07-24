/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#ifndef IOOBJECT_ACTOR_DEFINED
#define IOOBJECT_ACTOR_DEFINED 1

#include "base/Common.h"
#include "IoObject.h"
#include "IoCoroutine.h"

void IoObject_createCoroIfAbsent(IoObject *self);
IoObject *IoObject_asyncPerform(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_futurePerform(IoObject *self, IoObject *locals, IoMessage *m);
void IoObject_run(void *context);

void IoObject_yield(IoObject *self);
void IoObject_abort(IoObject *self);

void IoObject_pause(IoObject *self);
unsigned char IoObject_isPaused(IoObject *self);
void IoObject_resume(IoObject *self);

IoObject *IoObject_isActive(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_actorWait(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_activeCoroCount(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoObject_setSchedulerSleepSeconds(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_schedulerSleepSeconds(IoObject *self, IoObject *locals, IoMessage *m);

unsigned char IoObject_isPausing_(IoObject *self, IoObject *other);

#endif
