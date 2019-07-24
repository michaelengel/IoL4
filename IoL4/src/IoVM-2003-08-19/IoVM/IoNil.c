/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#include "IoNil.h"
#include "IoState.h"
#include "IoObject.h"
#include "IoString.h"
#include "IoMessage.h"

IoTag *IoNil_tag(void *state)
{
  IoTag *tag = IoTag_newWithName_("Nil");
  tag->state = state;
  tag->cloneFunc = (TagCloneFunc *)IoNil_rawClone;
  tag->printFunc = (TagPrintFunc *)IoNil_printNil;
  return tag;
}

IoObject *IoNil_proto(void *state)
{
  IoObject *self = IoObject_new(state);
  self->tag = IoNil_tag(state);
  IoState_registerProtoWithFunc_(state, self, IoNil_proto);

  IoObject_addMethod_(self, IOSTRING("clone"), (void *)IoObject_self);
  IoObject_addMethod_(self, IOSTRING("and"), (void *)IoNil_and);
  IoObject_addMethod_(self, IOSTRING("or"), (void *)IoNil_or);
  IoObject_addMethod_(self, IOSTRING("print"), (void *)IoNil_print);
  IoObject_addMethod_(self, IOSTRING("=="), (void *)IoNil_isEqual);
  IoObject_addMethod_(self, IOSTRING("!="), (void *)IoNil_isNotEqual);
  IoObject_addMethod_(self, IOSTRING("then"), (void *)IoObject_nop);
  IoObject_addMethod_(self, IOSTRING("else"), (void *)IoObject_evalArg);
  IoObject_addMethod_(self, IOSTRING("elseif"), (void *)IoObject_protoIf);
  IoObject_addMethod_(self, IOSTRING("isNil"), (void *)IoNil_isNil);
  IoObject_addMethod_(self, IOSTRING("ifNil"), (void *)IoNil_ifNil);
  return self;
}

/* carefull - the compiler might not make this unique */
IoObject *IoNil_rawClone(IoObject *self) { return self; } 

IoObject *IoNil_new(void *state) 
{ return ((IoState *)state)->ioNil; }

void IoNil_printNil(IoObject *self) 
{ IoState_print_(IOSTATE, "Nil"); }

IoObject *IoNil_and(IoObject *self, IoObject *locals, IoMessage *m)
{ return self; }

IoObject *IoNil_or(IoObject *self, IoObject *locals, IoMessage *m)
{ 
  IoObject *other = IoMessage_locals_valueArgAt_(m, locals, 0);
  return (!ISNIL(other)) ? (IoObject *)other : self; 
}

IoObject *IoNil_print(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoState_print_(IOSTATE, "Nil");
  return self;
}

IoObject *IoNil_isEqual(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoObject *other = IoMessage_locals_valueArgAt_(m, locals, 0);
  if (other == self) { return ((IoState *)IOSTATE)->lobby; }
  return self;
}

IoObject *IoNil_isNotEqual(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoObject *other = IoMessage_locals_valueArgAt_(m, locals, 0);
  if (other != self) { return ((IoState *)IOSTATE)->lobby; }
  return self;
}

IoObject *IoNil_isNil(IoObject *self, IoObject *locals, IoMessage *m)
{ return ((IoState *)(IOSTATE))->ioNop; }

IoObject *IoNil_ifNil(IoObject *self, IoObject *locals, IoMessage *m)
{ return IoMessage_locals_valueArgAt_(m, locals, 0); }

IoObject *IoNil_ifTrue(IoObject *self, IoObject *locals, IoMessage *m)
{ return ((IoState *)(IOSTATE))->ioNop; }

IoObject *IoNil_ifFalse(IoObject *self, IoObject *locals, IoMessage *m)
{ return IoMessage_locals_valueArgAt_(m, locals, 0); }
