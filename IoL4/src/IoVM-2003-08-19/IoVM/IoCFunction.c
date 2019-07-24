/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */
#include "IoCFunction.h"

#include "IoState.h"
#include "IoNumber.h"
#include "IoNil.h"
#include <stddef.h>

#define DATA(self) ((IoCFunctionData *)self->data)

IoTag *IoCFunction_tag(void *state)
{
  IoTag *tag = IoTag_newWithName_("CFunction");
  tag->state = state;
  tag->cloneFunc = (TagCloneFunc *)IoCFunction_rawClone;
  tag->markFunc  = (TagMarkFunc *)IoCFunction_mark;
  tag->freeFunc  = (TagFreeFunc *)IoCFunction_free;
  tag->activateFunc = (TagActivateFunc *)IoCFunction_target_locals_call_;
  return tag;
}

IoCFunction *IoCFunction_proto(void *state)
{
  IoObject *self = IoObject_new(state);
  self->tag = IoCFunction_tag(state);

  self->data = calloc(1, sizeof(IoCFunctionData));
  IoState_registerProtoWithFunc_(state, self, IoCFunction_proto);
  return self;
}

void IoCFunction_protoFinish(void *state)
{
  IoObject *self = IoState_protoWithInitFunction_(state, IoString_proto);
  
  IoObject_addMethod_(self, IOSTRING("id"), IoCFunction_id);
  IoObject_addMethod_(self, IOSTRING("=="), IoCFunction_equals);
}

IoCFunction *IoCFunction_rawClone(IoCFunction *self)
{
  IoObject *child = IoObject_rawClonePrimitive(self);
  child->data = cpalloc(DATA(self), sizeof(IoCFunctionData));
  return child;
}

void IoCFunction_mark(IoCFunction *self)
{
  if (DATA(self)->uniqueName) { IoObject_makeGrayIfWhite(DATA(self)->uniqueName); }
}

void IoCFunction_free(IoCFunction *self)
{ 
  free(DATA(self));
}

IoCFunction *IoCFunction_newWithFunctionPointer_tag_(void *state, IoUserFunction *func, IoTag *typeTag)
{
  IoCFunction *proto = IoState_protoWithInitFunction_(state, IoCFunction_proto);
  IoCFunction *self = IoCFunction_rawClone(proto);
  DATA(self)->typeTag = typeTag;
  DATA(self)->func = func;
  /*DATA(self)->uniqueName = IoState_stringWithCString_(state, funcName); 
  IoState_registerCFunction_(state, self);
  */
  /*printf("adding method to %s\n",  typeTag->name);*/
  return self;
}

IoObject *IoCFunction_id(IoCFunction *self, void *locals, IoMessage *m)
{ return IoNumber_newWithDouble_(IOSTATE, (double)((ptrdiff_t)self)); }

IoObject *IoCFunction_equals(IoCFunction *self, void *locals, IoMessage *m)
{ 
  IoObject *v = IoMessage_locals_valueArgAt_(m, locals, 0);
  if (ISCFUNCTION(v) && (DATA(self)->func == DATA(v)->func)) return self;
  return IONIL(self); 
}

IoObject *IoCFunction_target_locals_call_(IoCFunction *self, IoObject *target, IoObject *locals, IoMessage *m)
{ 
  IoTag *t = DATA(self)->typeTag;
  /*printf("%s ?= %s\n", v->name, t->name);*/
  IoObject *result;
  if (t && t != target->tag)
  { 
    char *a = (char *)IoTag_name(t);
    char *b = (char *)IoTag_name(target->tag);
    IoState_error_description_(IOSTATE, m, "CFunction.activate", 
      "CFunction defined for type %s but called on type %s", a, b);
  }
  result =(*(IoUserFunction *)(DATA(self)->func))(target, (void *)locals, m); 
  return result;
}

