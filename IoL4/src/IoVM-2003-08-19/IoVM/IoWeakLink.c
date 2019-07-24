/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 *
 */

#include "IoWeakLink.h"
#include "IoString.h"
#include "IoState.h"
#include "IoObject.h"
#include "IoNil.h"
#include "IoNumber.h"

#define DATA(self) ((IoWeakLinkData *)self->data)

IoTag *IoWeakLink_tag(void *state)
{
  IoTag *tag = IoTag_newWithName_("WeakLink");
  tag->state = state;
  tag->cloneFunc   = (TagCloneFunc *)IoWeakLink_rawClone;
  tag->compareFunc = (TagCompareFunc *)IoObject_compare;
  tag->markFunc    = (TagMarkFunc *)IoWeakLink_mark;
  tag->freeFunc    = (TagFreeFunc *)IoWeakLink_free;
  return tag;
}

IoObject *IoWeakLink_proto(void *state) 
{ 
  IoObject *self = IoObject_new(state);
  
  self->data = calloc(1, sizeof(IoWeakLinkData));
  self->tag = IoWeakLink_tag(state);
  DATA(self)->link       = IONIL(self);
  DATA(self)->delegate   = IONIL(self);
  DATA(self)->identifier = IONIL(self);
  IoState_registerProtoWithFunc_(state, self, IoWeakLink_proto);
  
  IoState_addWeakLink_(IOSTATE, self);
  
  IoObject_addMethod_(self, IOSTRING("setLink"), IoWeakLink_setLink);
  IoObject_addMethod_(self, IOSTRING("link"), IoWeakLink_link);
  IoObject_addMethod_(self, IOSTRING("setDelegate"), IoWeakLink_setDelegate);
  IoObject_addMethod_(self, IOSTRING("delegate"), IoWeakLink_delegate);
  IoObject_addMethod_(self, IOSTRING("setIdentifier"), IoWeakLink_setIdentifier);
  IoObject_addMethod_(self, IOSTRING("identifier"), IoWeakLink_identifier);
  return self;
}

IoObject *IoWeakLink_rawClone(IoObject *self) 
{ 
  IoObject *child = IoObject_rawClonePrimitive(self);
  child->data = calloc(1, sizeof(IoWeakLinkData));
  child->tag	  = self->tag;
  DATA(child)->link       = IONIL(self);
  DATA(child)->delegate   = IONIL(self);
  DATA(child)->identifier = IONIL(self);
  return self;
}

IoObject *IoWeakLink_new(void *state)
{ 
  IoObject *proto = IoState_protoWithInitFunction_(state, IoWeakLink_proto);
  return IoWeakLink_rawClone(proto);
}

void IoWeakLink_free(IoObject *self)
{
  IoWeakLink_unlink(self);
  IoState_removeWeakLink_(IOSTATE, self);
  free(self->data);
}

IoObject *IoWeakLink_newWithValue_(void *state, IoObject *v)
{
  IoObject *self = IoWeakLink_new(state);
  DATA(self)->link = v;
  return self;
}

void IoWeakLink_mark(IoObject *self)
{ 
  IoObject_makeGrayIfWhite(DATA(self)->delegate);
  IoObject_makeGrayIfWhite(DATA(self)->identifier);
  /* we intentionaly don't mark 'link' so it will get collected despited our reference */
}

IoObject *IoWeakLink_rawLink(IoWeakLink *self)
{ return DATA(self)->link; }

IoObject *IoWeakLink_rawNextWeakLink(IoWeakLink *self)
{ return DATA(self)->nextWeakLink; }

/* --- special weaklink methods ----------------------------- */

void IoWeakLink_appendWeakLink_(IoObject *self, IoObject *other)
{
  if (DATA(self)->nextWeakLink) 
  { 
    DATA(other)->nextWeakLink = DATA(self)->nextWeakLink;
    DATA(other)->previousWeakLink = self;
  }
  DATA(self)->nextWeakLink = other;
}

void IoWeakLink_unlink(IoObject *self) 
{
  if (DATA(self)->nextWeakLink) DATA(DATA(self)->nextWeakLink)->previousWeakLink = DATA(self)->previousWeakLink;
  if (DATA(self)->previousWeakLink) DATA(DATA(self)->previousWeakLink)->nextWeakLink = DATA(self)->nextWeakLink;
}

void IoWeakLink_collected(IoObject *self) 
{
  IoState *state = IOSTATE;
  /* called by IoState when link is collected */
  /* link may already be freed */
  DATA(self)->link = IONIL(self);
  {
    IoMessage *m = IoMessage_newWithName_(state, IOSTRING("collectedWeakLink"));
    IoMessage_addArg_(m, IoMessage_newWithName_returnsValue_(state, IOSTRING("weaklink"), self));
    IoMessage_locals_performOn_(m, state->lobby, DATA(self)->delegate);
  }
  if (DATA(self)->nextWeakLink) IoWeakLink_collected(DATA(self)->nextWeakLink);
}

void IoWeakLink_unlinkItem_(IoObject *self, IoObject *other)
{
  if (self == other)
    IoWeakLink_unlink(self);
  else
    IoWeakLink_unlinkItem_(self, other);
}

/* ----------------------------------------------------------- */

IoObject *IoWeakLink_setLink(IoObject *self, IoObject *locals, IoMessage *m)
{ 
  IoObject *v = IoMessage_locals_valueArgAt_(m, locals, 0);
  IoState_removeWeakLink_(IOSTATE, self);
  IoWeakLink_unlink(self);
  DATA(self)->link = v; /* no IOREF because this is a weak link */
  if (!ISNIL(v)) v->mark.hasWeakLink = 1;
  IoState_addWeakLink_(IOSTATE, self);
  return self; 
}

IoObject *IoWeakLink_link(IoObject *self, IoObject *locals, IoMessage *m)
{ return DATA(self)->link; }

IoObject *IoWeakLink_setDelegate(IoObject *self, IoObject *locals, IoMessage *m)
{ 
  IoObject *v = IoMessage_locals_valueArgAt_(m, locals, 0);
  DATA(self)->delegate = IOREF(v);
  return self;
}

IoObject *IoWeakLink_delegate(IoObject *self, IoObject *locals, IoMessage *m)
{ return DATA(self)->delegate; }

IoObject *IoWeakLink_setIdentifier(IoObject *self, IoObject *locals, IoMessage *m)
{ 
  IoObject *v = IoMessage_locals_valueArgAt_(m, locals, 0);
  DATA(self)->identifier = IOREF(v);
  return self;
}

IoObject *IoWeakLink_identifier(IoObject *self, IoObject *locals, IoMessage *m)
{ return DATA(self)->identifier; }
