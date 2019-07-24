/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#ifdef IOSTATE_C 
#define IO_IN_C_FILE
#endif
#include "base/Common_inline.h"
#ifdef IO_DECLARE_INLINES

/*
#define RETAIN_DEBUG
*/
IOINLINE void IoState_stackRetain_(IoState *self, IoObject *v)
{ 
  Stack_push_(self->currentRetainStack, v); 
  #ifdef RETAIN_DEBUG
  printf("rpush %i\n", (int)Stack_totalSize(self->currentRetainStack));
  #endif
}

IOINLINE void IoState_retainPopOne(IoState *self)
{ 
  Stack_pop(self->currentRetainStack); 
  #ifdef RETAIN_DEBUG
  printf("r-pop-one\n");
  #endif
}

IOINLINE void IoState_clearRetainStack(IoState *self)
{ 
  Stack_clear(((IoState *)self)->currentRetainStack); 
  #ifdef RETAIN_DEBUG
  printf("rclear %i\n", (int)Stack_totalSize(self->currentRetainStack));
  #endif
}

IOINLINE void IoState_pushRetainPool(void *self)
{ 
  Stack_pushMark(((IoState *)self)->currentRetainStack); 
  #ifdef RETAIN_DEBUG
  printf("rmark push\n");
  #endif
}

IOINLINE void IoState_popRetainPool(void *self)
{ 
  Stack_popMark(((IoState *)self)->currentRetainStack); 
  #ifdef RETAIN_DEBUG
  printf("rmark pop\n");
  #endif
}

#undef IO_IN_C_FILE
#endif

