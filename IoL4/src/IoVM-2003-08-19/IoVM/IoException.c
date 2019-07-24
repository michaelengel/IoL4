/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#include "IoException.h"
#include "IoState.h"

IoException *IoException_new(void)
{
  IoException *self = (IoException *)calloc(1, sizeof(IoException));
  return self;
}

void IoException_free(IoException *self)
{
  free(self);
}

void IoException_mark(IoException *self)
{
  if (self->name) IoObject_makeGrayIfWhite((IoObject *)self->name);
  if (self->description) IoObject_makeGrayIfWhite((IoObject *)self->description);
  if (self->message) IoObject_makeGrayIfWhite((IoObject *)self->message);
}

void IoException_name_(IoException *self, IoString *s)
{ self->name = s; } /* no IOREF since IoException not a Value and actors get marked before GC */

void IoException_description_(IoException *self, IoString *s)
{ self->description = s; }

void IoException_message_(IoException *self, IoMessage *m)
{ self->message = m; }

