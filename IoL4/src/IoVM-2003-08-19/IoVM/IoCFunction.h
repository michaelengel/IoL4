/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#ifndef IOCFUNCTION_DEFINED
#define IOCFUNCTION_DEFINED 1

#include "base/Common.h"
#include "IoObject.h"

#define ISCFUNCTION(self) \
  (self->tag->cloneFunc == (TagCloneFunc *)IoCFunction_rawClone)
#define IOCFUNCTION(func, tag) IoCFunction_newWithFunctionPointer_tag_(IOSTATE, (IoUserFunction *)func, tag)

typedef IoObject *(IoUserFunction)(IoObject *, IoObject *, IoMessage *);

typedef IoObject IoCFunction;

typedef struct
{
  IoTag *typeTag; /* pointer to tag of type excepted for self value to have as data */
  IoUserFunction *func;
  IoString *uniqueName;
} IoCFunctionData;

IoCFunction *IoCFunction_proto(void *state);
void IoCFunction_protoFinish(void *state);
IoCFunction *IoCFunction_rawClone(IoCFunction *self);
IoCFunction *IoCFunction_newWithFunctionPointer_tag_(void *state, IoUserFunction *s, IoTag *typeTag);

void IoCFunction_mark(IoCFunction *self);
void IoCFunction_free(IoCFunction *self);

IoObject *IoCFunction_id(IoCFunction *self, void *locals, IoMessage *m);
IoObject *IoCFunction_equals(IoCFunction *self, void *locals, IoMessage *m);
IoObject *IoCFunction_target_locals_call_(IoCFunction *self, IoObject *target, IoObject *locals, IoMessage *m);

#endif

