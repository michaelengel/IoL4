/* Copyright (c) 2002, Steve Dekorte
 * All rights reserved. See _BSDLicense.txt.
 */

#ifdef IOVALUEGROUP_C 
#define IO_IN_C_FILE
#endif
#include "base/Common_inline.h"
#ifdef IO_DECLARE_INLINES

IOINLINE void IoObjectGroup_isRemoving_(IoObjectGroup *self, IoObject *v)
{ 
  if (self->first == v) self->first = (IoObject *)(v->mark.next); 
  if (self->last == v)  self->last  = (IoObject *)(v->mark.previous); 
  /* unlink it */
  if (v->mark.previous) 
  { ((IoObject *)(v->mark.previous))->mark.next = v->mark.next; }
  if (v->mark.next)
  { ((IoObject *)(v->mark.next))->mark.previous = v->mark.previous; }
  v->mark.next = 0x0;
  /*v->previous = 0x0;*/
}

IOINLINE void IoObjectGroup_append_(IoObjectGroup *self, IoObject *v)
{
  /* append it to this one */
  v->mark.previous = self->last;
  v->mark.next = 0;
  if (self->last) self->last->mark.next = v;
  self->last = v;
  
  if (!self->first) self->first = v; 
  v->mark.color = self->mark.color;
}

IOINLINE void IoObjectGroup_appendGroup_(IoObjectGroup *self, IoObjectGroup *other)
{
  /* append a whole group to this one */
  if (other->first)
    other->first->mark.previous = self->last;
  if (self->last) self->last->mark.next = other->first;
  self->last = other->last;

  if (!self->first) self->first = other->first;
}

#undef IO_IN_C_FILE
#endif
