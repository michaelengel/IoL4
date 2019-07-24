/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#ifndef IOEXCEPTIONCATCH_DEFINED
#define IOEXCEPTIONCATCH_DEFINED 1

#include "base/Common.h"
#include "base/Stack.h"
#include "IoString.h"
#include <setjmp.h>

#ifdef _BSD_PPC_SETJMP_H_ 
  /* OSX */
  #define IOEXCEPTIONCATCH_SETJMP _setjmp
  #define IOEXCEPTIONCATCH_LONGJMP _longjmp
#else
  #define IOEXCEPTIONCATCH_SETJMP setjmp
  #define IOEXCEPTIONCATCH_LONGJMP longjmp
#endif

typedef struct
{
  jmp_buf *savedStack;
  char *exceptionName;
  IoString *caughtName;
  IoString *caughtDescription;
  IoMessage *caughtMessage;
  Stack *retainedValues;
} IoExceptionCatch;

IoExceptionCatch *IoExceptionCatch_new(void);
IoExceptionCatch *IoExceptionCatch_newWithCStringName_(char *exceptionName);
IoExceptionCatch *IoExceptionCatch_newWithName_(IoString *exceptionName);

void IoExceptionCatch_free(IoExceptionCatch *self);
void IoExceptionCatch_mark(IoExceptionCatch *self);

void IoExceptionCatch_caughtName_(IoExceptionCatch *self, IoString *s);
void IoExceptionCatch_caughtDescription_(IoExceptionCatch *self, IoString *s);
void IoExceptionCatch_caughtMessage_(IoExceptionCatch *self, IoMessage *m);

int IoExceptionCatch_matchesId_(IoExceptionCatch *self, IoString *exceptionName);
void IoExceptionCatch_jump(IoExceptionCatch *self);

#define IoExceptionCatch_pushRetainPool(self) Stack_pushMark(self->retainedValues);
#define IoExceptionCatch_retain_(self, value) Stack_push_(self->retainedValues, value);
#define IoExceptionCatch_popRetainPool(self) Stack_popMark(self->retainedValues);

/*
IOINLINE void IoExceptionCatch_pushRetainPool(IoExceptionCatch *self)
{ Stack_pushMark(self->retainedValues); }
  
IOINLINE void IoExceptionCatch_retain_(IoExceptionCatch *self, IoObject *v)
{ Stack_push_(self->retainedValues, v); }

IOINLINE void IoExceptionCatch_popRetainPool(IoExceptionCatch *self)
{ Stack_popMark(self->retainedValues); }
*/

#endif
