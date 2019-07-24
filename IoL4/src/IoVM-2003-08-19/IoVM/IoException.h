/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#ifndef IOEXCEPTION_DEFINED
#define IOEXCEPTION_DEFINED 1

#include "IoString.h"
#include "IoMessage.h"

typedef struct
{
  IoString *name;
  IoString *description;
  IoMessage *message;
} IoException;

IoException *IoException_new(void);
void IoException_free(IoException *self);

void IoException_name_(IoException *self, IoString *s);
void IoException_description_(IoException *self, IoString *s);
void IoException_message_(IoException *self, IoMessage *m);

#endif
