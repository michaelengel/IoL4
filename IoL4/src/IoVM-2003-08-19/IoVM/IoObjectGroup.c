/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#define IOVALUEGROUP_C
#include "IoObjectGroup.h"
#undef IOVALUEGROUP_C
#include "IoState.h"
#include "IoWeakLink.h"

IoObjectGroup *IoObjectGroup_newWithColor_(void *state, unsigned char color)
{
  IoObjectGroup *self = (IoObjectGroup *)calloc(1, sizeof(IoObjectGroup));
  self->mark.color = color;
  self->state = state;
  return self;
}

void IoObjectGroup_free(IoObjectGroup *self) { free(self); }

void IoObject_moveToGroup_(IoObjectGroup *self, IoObjectGroup *other)
{
  if (self->first)
  {
    IoObjectGroup_appendGroup_(other, self);
    self->first = 0x0;
    self->last  = 0x0;
  }
}

void IoObjectGroup_colorValues(IoObjectGroup *self)
{
  IoObject *v = self->first;
  unsigned char color = self->mark.color;
  while (v) { v->mark.color = color; v = v->mark.next; }
}

int IoObjectGroup_count(IoObjectGroup *self)
{
  int count = 0;
  IoObject *v = self->first;
  while (v) { count++; v = v->mark.next; }
  return count;
}

void IoObjectGroup_markValues(IoObjectGroup *self)
{
  IoObject *v = self->first;
  IoObject *next;
  while (v)
  { 
    next = v->mark.next;
    IoObject_mark(v); 
    v = next; 
  }
}

int IoObjectGroup_freeValues(IoObjectGroup *self)
{
  int count = 0;
  IoObject *v = self->first;
  IoObject *next;
  while (v) 
  {
    count++;
    next = v->mark.next;
    if (v->mark.hasWeakLink) IoState_willFreeWeakLinkedValue_(self->state, v);
    IoObject_free(v); 
    v = next;
  }
  self->first = 0x0;
  self->last = 0x0;
  return count;
}

unsigned char IoObjectGroup_isEmpty(IoObjectGroup *self)
{ return (self->first == 0x0); }

int IoObjectGroup_sharesWithGroup_(IoObjectGroup *self, IoObjectGroup *other)
{
  IoObject *v = self->first;
  while (v)
  { 
    if (IoObjectGroup_hasValue_(other, v)) return 1; 
    v = v->mark.next; 
  }
  return 0;
}

int IoObjectGroup_hasValue_(IoObjectGroup *self, IoObject *value)
{
  IoObject *v = self->first;
  while (v)
  { 
    if (v == value) return 1; 
    v = v->mark.next; 
  }
  return 0;
}

