/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#define IOEXCEPTIONCATCH_C
#include "IoExceptionCatch.h"
#undef IOEXCEPTIONCATCH_C
#include "IoState.h"

IoExceptionCatch *IoExceptionCatch_new(void)
{
  IoExceptionCatch *self = (IoExceptionCatch *)calloc(1, sizeof(IoExceptionCatch));
  self->savedStack = (jmp_buf *)malloc(sizeof(jmp_buf));
  self->retainedValues = Stack_new();
  return self;
}

IoExceptionCatch *IoExceptionCatch_exceptionNameCString_(IoExceptionCatch *self, char *s)
{
  self->exceptionName = strcpy(realloc(self->exceptionName, strlen(s)+1), s);
  return self;
}

IoExceptionCatch *IoExceptionCatch_newWithCStringName_(char *exceptionName)
{
  IoExceptionCatch *self = IoExceptionCatch_new();
  IoExceptionCatch_exceptionNameCString_(self, exceptionName);
  return self;
}

IoExceptionCatch *IoExceptionCatch_newWithName_(IoString *s)
{
  IoExceptionCatch *self = IoExceptionCatch_new();
  IoExceptionCatch_exceptionNameCString_(self, CSTRING(s));
  return self;
}

void IoExceptionCatch_free(IoExceptionCatch *self)
{
  /*
  if (Stack_count(recyclableExceptions) < 100)
  { Stack_push_(recyclableExceptions, self); }
  else
  { 
  */
    free(self->exceptionName);
    free(self->savedStack);
    Stack_free(self->retainedValues);
    free(self);
  /*}*/
}

void IoExceptionCatch_mark(IoExceptionCatch *self)
{
  /*printf("IoExceptionCatch_mark('%s')\n", self->exceptionName);*/
  if (self->caughtName) IoObject_makeGrayIfWhite((IoObject *)self->caughtName);
  if (self->caughtDescription) IoObject_makeGrayIfWhite((IoObject *)self->caughtDescription);
  if (self->caughtMessage) IoObject_makeGrayIfWhite((IoObject *)self->caughtMessage);
  Stack_do_(self->retainedValues, (ListDoCallback *)IoObject_makeGrayIfWhite);
  /*printf("IoExceptionCatch_mark('%s') done\n", self->exceptionName);*/
}

void IoExceptionCatch_caughtName_(IoExceptionCatch *self, IoString *s)
{ self->caughtName = s; } /* no IOREF since IoException not a Value and actors get marked before GC */

void IoExceptionCatch_caughtDescription_(IoExceptionCatch *self, IoString *s)
{ self->caughtDescription = s; }

void IoExceptionCatch_caughtMessage_(IoExceptionCatch *self, IoMessage *m)
{ self->caughtMessage = m; }

int IoExceptionCatch_matchesId_(IoExceptionCatch *self, IoString *s)
{ 
  char *e = self->exceptionName;
  return (strncmp(e, CSTRING(s), strlen(e)) == 0);
}

#if !defined(__SYMBIAN32__)
void IoExceptionCatch_jump(IoExceptionCatch *self)
{ 
  if (*(self->savedStack)) IOEXCEPTIONCATCH_LONGJMP(*(self->savedStack), 1); 
}
#endif
