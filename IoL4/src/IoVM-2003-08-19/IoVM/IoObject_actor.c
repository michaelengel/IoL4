/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#include "IoObject_actor.h"
#include "IoState.h"
#include "IoNil.h"
#include "IoNumber.h"
#include "IoFuture.h"
#include <string.h>

void IoObject_createCoroIfAbsent(IoObject *self)
{
  if (!self->coro) 
  {
    self->coro = IoCoroutine_new();
    IoState_addActor_(IOSTATE, self);
  }
}

IoObject *IoObject_actorPerform(IoObject *volatile self, IoObject *locals, IoMessage *m, 
  unsigned char returnFuture)
{
  /* add message to async message queue for this object */
  IoState *state = IOSTATE;
  IoMessage *sendMessage;
  IoFuture *future;
  
  /* setup or resume coro - need to do this first to ensure it's in 
     the State's list of actors, in case the collector frees when
     we're creating the message object to put on the queue
  */
  if (!self->coro) self->coro = IoCoroutine_new();

  if (!self->coro->tid)
  { 
    IoObject *current = IoState_currentActor(state);
    self->coro->tid = Scheduler_createCoro(state->scheduler, (void *)self, IoObject_run);
    /*printf("self %p coro %p tid %p\n", self, self->coro, self->coro->tid);*/
    IoState_currentActor_(state, current);
    IoState_addActor_(IOSTATE, self);
  }
  else
  {
    if (IoObject_isPaused(self)) IoObject_resume(self);
  }
  
  if (!IoMessage_argCount(m))
  {
    IoState_error_description_(state, m, 
      "Io.Object.asyncPerform", "argument is required for method '%s'", 
      CSTRING(IoMessage_rawMethod(m)));  
  }
  
  /* we need to pre-evaluate arguments since the locals will be 
    gone by the time the message is processed */
  {
    IoMessage *argMessage = IoMessage_rawArgAt_(m, 0);
    int i, max = IoMessage_argCount(argMessage);
    
    if (IoMessage_needsEvaluation(argMessage))
    { sendMessage = IoMessage_newWithName_(state, IoMessage_rawMethod(argMessage)); }
    else 
    { sendMessage = argMessage; }
    
    for (i = 0; i < max; i++)
    {
      IoMessage *arg = IoMessage_rawArgAt_(argMessage, i);
      IoObject *result = IoMessage_locals_performOn_(arg, locals, locals);
      IoMessage_setCachedArg_to_(sendMessage, i, result);
    }
  }
  
  future = IoFuture_newWithMessage_creatorObject_(state, sendMessage, self);
  Queue_push_(self->coro->queue, IOREF(future));

  if (returnFuture) return future;
  return IONIL(self);
}

IoObject *IoObject_asyncPerform(IoObject *self, IoObject *locals, IoMessage *m)
{ return IoObject_actorPerform(self, locals, m, 0); }

IoObject *IoObject_futurePerform(IoObject *self, IoObject *locals, IoMessage *m)
{ return IoObject_actorPerform(self, locals, m, 1); }

/* --- coroutine calls ----------------------------------------------- */

void IoObject_run(void *context)
{
  volatile IoObject *self = context;
  volatile IoState *state = IOSTATE;
  volatile IoObject *lobby = IoState_lobby((IoState *)state);
  volatile IoExceptionCatch *eCatch;
  volatile IoFuture *f = 0x0;

  IoObject_yield((IoObject *)self); /* yield so last coro can continue */
  for (;;)
  {
  eCatch = IoState_pushExceptionCatchWithName_((IoState *)state, "");
  if (!IOEXCEPTIONCATCH_SETJMP(*(eCatch->savedStack)))
  {
    for (;;)
    {
      while ((f = Queue_top(self->coro->queue)))
      {
        IoMessage *m = IoFuture_rawMessage((IoFuture *)f);
        IoObject *result;
        if (state->currentActor != self)
        {
          printf("wrong actor\n");
          exit(1);
        }
        if (Stack_totalSize(eCatch->retainedValues) > 100)
        {
          printf("fat actor stack\n");
          exit(1);        
        }
        
        IoState_pushRetainPool((IoState *)state);
        result = IoObject_perform((IoObject *)self, (IoObject *)lobby, m);
        IoFuture_rawSetValue_((IoFuture *)f, (IoObject *)result);
        IoState_popRetainPool((IoState *)state);
        Queue_pop(self->coro->queue);
        if (Queue_length(self->coro->queue) > 3)
        {
          printf("queue length = %i\n", Queue_length(self->coro->queue));
        }
        /*IoObject_yield((IoObject *)self);*/
      }
      IoObject_pause((IoObject *)self);
    }
  }
  else
  { 
    if (f) IoFuture_rawSetValue_((IoFuture *)f, (IoObject *)IONIL(self));
    Queue_pop(self->coro->queue);
    IoState_callErrorCallback((IoState *)state, (IoExceptionCatch *)eCatch); 
  }
  IoState_popExceptionCatch_((IoState *)state, (IoExceptionCatch *)eCatch);
  IoObject_pause((IoObject *)self);
  }
}

void IoObject_yield(IoObject *self)
{
  IoState *state = IOSTATE;
  /*printf("yielding %p\n", self);*/
  Scheduler_yield(state->scheduler);
  IoState_currentActor_(state, self);
}

void IoObject_abort(IoObject *self)
{ 
  if (self->coro)
  {
    IoState *state = IOSTATE;
    void *t = self->coro->tid;
    /*
    IoCoroutine_free(self->coro);
    self->coro = 0x0;
    */
    IoState_removeActor_(state, self);
    /*printf("aborting %p\n", p);*/
    Scheduler_killCoro(state->scheduler, t);
    self->coro->tid = 0x0;
  }
}

void IoObject_pause(IoObject *self)
{
  if (self->coro)
  {
    IoState *state = IOSTATE;
    /*printf("pausing %p\n", self);*/
    if (!Scheduler_hasCoro_(state->scheduler, self->coro->tid)) return;
    Scheduler_pauseCoro_(state->scheduler, self->coro->tid);
    /*IoState_removeActor_(state, self);*/ /* so object can be collected if not referenced */
    if (IoState_currentActor(state) == self) IoObject_yield(self);
  }
}

unsigned char IoObject_isPaused(IoObject *self)
{ 
  return (self->coro && !Scheduler_hasCoro_(IOSTATE->scheduler, (Coro *)self->coro->tid));
}

void IoObject_resume(IoObject *self)
{ 
  if (self->coro)
  {  
    IoState *state = IOSTATE;
    Coro *coro = self->coro->tid;
    if (Scheduler_hasCoro_(state->scheduler, coro)) return;
    /*printf("resuming %p\n", self);*/
    Scheduler_resumeCoro_(state->scheduler, coro);
    /*IoState_addActor_(state, self);*/
  }
}

IoObject *IoObject_isActive(IoObject *self, IoObject *locals, IoMessage *m)
{ 
  if (!self->coro) return ((IoState *)IOSTATE)->ioNil;
  return self;
}

IoObject *IoObject_actorWait(IoObject *self, IoObject *locals, IoMessage *m)
{ 
  double seconds = IoMessage_locals_doubleArgAt_(m, locals, 0);
  IoState *state = IOSTATE;
  IoObject *currentActor = IoState_currentActor(state);
  Scheduler_waitSeconds_(state->scheduler, seconds);
  IoState_currentActor_(state, currentActor);
  return self;
}

IoObject *IoObject_activeCoroCount(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoState *state = IOSTATE;
  return IONUMBER(Scheduler_coroCount(state->scheduler));
}

IoObject *IoObject_setSchedulerSleepSeconds(IoObject *self, IoObject *locals, IoMessage *m)
{
  double seconds = IoMessage_locals_doubleArgAt_(m, locals, 0);
  IoState *state = IOSTATE;
  if (seconds >= 1)
  {
    IoState_error_description_(state, m, 
      "Io.Object.setSchedulerSleepSeconds", "period of '%f' is too big - must be less than 1", seconds);  
  }
  Scheduler_nanoSleepPeriod_(state->scheduler, seconds*1000000000.0);
  return self;
}

IoObject *IoObject_schedulerSleepSeconds(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoState *state = IOSTATE;
  double p = Scheduler_nanoSleepPeriod(state->scheduler);
  return IONUMBER(p/1000000000.0);
}

unsigned char IoObject_isPausing_(IoObject *self, IoObject *other)
{
  if (self->coro)
  {
    if (Queue_firstTrueFor_with_(self->coro->queue, 
      (QueueFirstTrueForWithCallback *)IoFuture_isPausing_, other))
    { return 1; }
  }
  return 0;
}
