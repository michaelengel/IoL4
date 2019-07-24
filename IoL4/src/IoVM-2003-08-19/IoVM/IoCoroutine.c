/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#include "IoCoroutine.h"
#include "IoState.h"
#include "IoObject.h"

IoCoroutine *IoCoroutine_new(void)
{
  IoCoroutine *self = (IoCoroutine *)calloc(1, sizeof(IoCoroutine));
  self->queue = Queue_new();
  self->exceptionCatchStack = Stack_new();
  Stack_push_(self->exceptionCatchStack, IoExceptionCatch_newWithCStringName_(""));
  return self;
}

void IoCoroutine_free(IoCoroutine *self)
{
  Stack_do_(self->exceptionCatchStack, (ListDoCallback *)IoExceptionCatch_free);
  Stack_free(self->exceptionCatchStack);
  Queue_free(self->queue);
  free(self);
}

void IoCoroutine_mark(IoCoroutine *self)
{ 
  Stack_do_(self->exceptionCatchStack, (ListDoCallback *)IoExceptionCatch_mark); 
  Queue_do_(self->queue, (QueueDoCallback *)IoObject_makeGrayIfWhite); 
}

size_t IoCoroutine_memorySize(IoCoroutine *self)
{
  size_t t = sizeof(IoCoroutine);
  t += Queue_memorySize(self->queue);
  t += Stack_memorySize(self->exceptionCatchStack);
  return t;
}

void IoCoroutine_compact(IoCoroutine *self)
{
  Queue_compact(self->queue);
  Stack_compact(self->exceptionCatchStack);
}

int IoCoroutine_retainingCount(IoCoroutine *self)
{
  int count = 0;
  Stack *s = Stack_newCopyWithoutMarks(self->exceptionCatchStack);
  IoExceptionCatch *ec = Stack_pop(s);
  while (ec)
  {
    count += Stack_totalSize(ec->retainedValues);
    ec = Stack_pop(s);
  }
  Stack_free(s);
  return count;
}

