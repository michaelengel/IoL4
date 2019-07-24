/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#include "IoFuture.h"
#include "IoObject.h"
#include "IoObject_actor.h"
#include "IoState.h"
#include "IoString.h"
#include "IoNil.h"

#define DATA(self) ((IoFutureData *)self->data)

IoTag *IoFuture_tag(void *state)
{
  IoTag *tag = IoTag_newWithName_("Future");
  tag->state = state;
  tag->cloneFunc = (TagCloneFunc *)IoFuture_rawClone;
  tag->markFunc  = (TagMarkFunc *)IoFuture_mark;
  tag->freeFunc  = (TagFreeFunc *)IoFuture_free;
  return tag;
}

IoFuture *IoFuture_proto(void *state)
{
  IoObject *self = IoObject_new(state);

  self->data = calloc(1, sizeof(IoFutureData));
  self->tag = IoFuture_tag(state);
  /* rest of data expected to be NULL */
  IoState_registerProtoWithFunc_(state, self, IoFuture_proto);
      
  IoObject_addMethod_(self, IOSTRING("value"), (void *)IoFuture_value);
  /*Tag_addMethod(tag, "setValue"), (void *)IoFuture_setValue);*/
  IoObject_addMethod_(self, IOSTRING("isReady"), (void *)IoFuture_isReady);
  IoObject_addMethod_(self, IOSTRING("sendResultTo"), (void *)IoFuture_sendResultTo);
  return self;
}

IoFuture *IoFuture_rawClone(IoFuture *self) 
{ 
  IoFuture *child = IoObject_rawClonePrimitive(self);
  child->data = cpalloc(self->data, sizeof(IoFutureData));
  return child; 
}

IoFuture *IoFuture_new(void *state)
{
  IoFuture *proto = IoState_protoWithInitFunction_(state, IoFuture_proto);
  return IoFuture_rawClone(proto);
}

void IoFuture_free(IoFuture *self)
{
  /*IoFuture *proto = IoState_protoWithInitFunction_(IOSTATE, IoFuture_proto);*/
  free(DATA(self));
}


IoFuture *IoFuture_newWithMessage_creatorObject_(IoState *state, 
  IoMessage *message, IoObject *creatorObject)
{
  IoFuture *self = IoFuture_new(state);
  DATA(self)->message = IOREF(message);
  DATA(self)->creatorObject = IOREF(creatorObject);
  return self;
}

IoMessage *IoFuture_rawMessage(IoFuture *self)
{ return DATA(self)->message; }


void IoFuture_mark(IoFuture *self)
{ 
  if (DATA(self)->value) IoObject_makeGrayIfWhite((IoObject *)(DATA(self)->value));
  if (DATA(self)->creatorObject) IoObject_makeGrayIfWhite((IoObject *)(DATA(self)->creatorObject));
  if (DATA(self)->pausedObject) IoObject_makeGrayIfWhite((IoObject *)(DATA(self)->pausedObject));
  if (DATA(self)->message) IoObject_makeGrayIfWhite((IoObject *)(DATA(self)->message));
  if (DATA(self)->target) IoObject_makeGrayIfWhite((IoObject *)(DATA(self)->target));
  if (DATA(self)->methodName) IoObject_makeGrayIfWhite((IoObject *)(DATA(self)->methodName));
}

void IoFuture_rawSetValue_(IoFuture *self, IoObject *v) 
{ 
  DATA(self)->value = IOREF(v); 
  if (DATA(self)->pausedObject) 
  { 
    IoObject_resume(DATA(self)->pausedObject);
    DATA(self)->pausedObject = 0x0;
    DATA(self)->creatorObject = 0x0;
  }
  if (DATA(self)->target)
  {
    IoMessage *m = IoMessage_newWithName_(IOSTATE, DATA(self)->methodName);
    IoMessage_setCachedArg_to_(m, 0, v);
    IoObject_asyncPerform((IoObject *)(DATA(self)->target), (IoObject *)(DATA(self)->target) /* hack */, m);
  }
}

unsigned char IoFuture_isPausing_(IoFuture *self, IoObject *v)
{
  if (DATA(self)->pausedObject) { return IoObject_isPausing_(DATA(self)->pausedObject, v); } 
  return 0;
}

/* ----------------------------------------------------------- */

IoObject *IoFuture_setValue(IoFuture *self, IoObject *locals, IoMessage *m)
{ 
  IoObject *value = IoMessage_locals_valueArgAt_(m, locals, 0);
  IoFuture_rawSetValue_(self, value); 
  return self; 
}

IoObject *IoFuture_value(IoFuture *self, IoObject *locals, IoMessage *m)
{ 
  if (!DATA(self)->value)
  {
    IoState *state = IOSTATE;
    
    /* auto deadlock detection */
    if (IoObject_isPausing_(IoState_currentActor(state), DATA(self)->creatorObject))
    { 
      IoState_error_description_(state, m, 
        "Io.Future.value", "attempt to access this Future value would create a deadlock");
      return IONIL(self);
    }
    DATA(self)->pausedObject = IOREF(IoState_currentActor(state));
    IoObject_pause(DATA(self)->pausedObject); /* this doesn't always work? why? */
  }
  while (!DATA(self)->value) { IoObject_pause(DATA(self)->pausedObject); } /* in case pause fails? */
  return DATA(self)->value;
}

IoObject *IoFuture_isReady(IoFuture *self, IoObject *locals, IoMessage *m)
{ return DATA(self)->value ? (IoObject *)self : IONIL(self); }


IoObject *IoFuture_sendResultTo(IoFuture *self, IoObject *locals, IoMessage *m)
{ 
  DATA(self)->target = IOREF(IoMessage_locals_valueArgAt_(m, locals, 0));
  DATA(self)->methodName = IOREF(IoMessage_locals_stringArgAt_(m, locals, 1));
  return self; 
}
