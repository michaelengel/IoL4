/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#include "IoTag.h"
#include "IoObject.h"
#include "IoState.h"
#include <string.h>

IoTag *IoTag_new()
{
  IoTag *self = (IoTag *)calloc(1, sizeof(IoTag));
  self->performFunc = (TagPerformFunc *)IoObject_perform;
  /*
  self->recyclableInstances = Stack_new();
  self->maxRecyclableInstances = 10000;
  */
  return self;
}

IoTag *IoTag_newWithName_(char *name)
{
  IoTag *self = IoTag_new();
  IoTag_name_(self, name);
  return self;
}

void IoTag_free(IoTag *self) 
{ 
  if (self->name) free(self->name); 
  /*Stack_free(self->recyclableInstances);*/
  free(self); 
}

void IoTag_name_(IoTag *self, char *name)
{ self->name = strcpy(realloc(self->name, strlen(name)+1), name); }

const char *IoTag_name(IoTag *self) { return self->name; }

void IoTag_mark(IoTag *self)
{
  /*
  if (Stack_count(self->recyclableInstances))
  { Stack_do_(self->recyclableInstances, (StackDoCallback *)IoObject_makeGrayIfWhite); }
  */
}


