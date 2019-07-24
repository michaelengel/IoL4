/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#ifndef IOCOROUTINE_DEFINED
#define IOCOROUTINE_DEFINED 1

#include "base/Common.h"
#include "base/Queue.h"
#include "base/Stack.h"

typedef struct
{
  void *tid;
  Queue *queue; /* for Actors - queue of async messages to be processed */
  Stack *exceptionCatchStack;
} IoCoroutine;

IoCoroutine *IoCoroutine_new(void);
void IoCoroutine_free(IoCoroutine *self);
void IoCoroutine_mark(IoCoroutine *self);

size_t IoCoroutine_memorySize(IoCoroutine *self);
void IoCoroutine_compact(IoCoroutine *self);

int IoCoroutine_retainingCount(IoCoroutine *self);

#endif
