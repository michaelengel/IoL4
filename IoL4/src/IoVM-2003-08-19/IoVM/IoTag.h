/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#ifndef IOTAG_DEFINED
#define IOTAG_DEFINED 1

#include "base/Common.h"
#include "base/Stack.h"

typedef void * (TagCloneFunc)(void *); /* self */
typedef void   (TagFreeFunc)(void *); /* self */
typedef void   (TagMarkFunc)(void *); /* self */
typedef void * (TagPerformFunc)(void *, void *, void *); /* self, locals, message */
typedef void * (TagActivateFunc)(void *, void *, void *, void *); /* self, target, locals, message */
typedef unsigned char (TagRespondsToFunc)(void *, void *); /* self, IoString */
typedef int    (TagCompareFunc)(void *, void *); /* self and another IoObject */
typedef void   (TagPrintFunc)(void *); /* self */
typedef size_t (TagMemorySizeFunc)(void *); /* self */
typedef void   (TagCompactFunc)(void *); /* self */

typedef void   (TagWriteToStoreFunc)(void *, void *);  /* self */
typedef void * (TagReadFromStoreFunc)(void *, void *); /* self */

typedef struct
{
  void *state;
  char *name;
  TagCloneFunc *cloneFunc;
  TagPerformFunc *performFunc; /* perform means: look up the slot specificed by the message and activate it with the message, return result */
  TagActivateFunc *activateFunc; /* return the receiver or compute and return a value */
  TagRespondsToFunc *respondsToFunc; /* depricated? */
  TagFreeFunc *freeFunc;
  TagMarkFunc *markFunc;
  TagCompareFunc *compareFunc;
  TagPrintFunc *printFunc;
  
  TagWriteToStoreFunc *writeToStoreFunc;
  TagReadFromStoreFunc *readFromStoreFunc;
  
  TagMemorySizeFunc *memorySizeFunc;
  TagCompactFunc *compactFunc;
  /*
  Stack *recyclableInstances;
  int maxRecyclableInstances;
  */
} IoTag;

IoTag *IoTag_new(void);
IoTag *IoTag_newWithName_(char *name);
void IoTag_free(IoTag *self);
void IoTag_name_(IoTag *self, char *name);
const char *IoTag_name(IoTag *self);

void IoTag_mark(IoTag *self);

#endif
