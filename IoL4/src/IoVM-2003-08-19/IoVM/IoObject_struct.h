/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#ifndef OBJECT_STRUCT_DEFINED
#define OBJECT_STRUCT_DEFINED 1

#include "base/Common.h"
#include "base/PHash.h"
#include "IoTag.h"
#include "IoMark.h"
#include "IoCoroutine.h"

typedef struct IoObject IoObject;

/*
#define IOOBJECT_SANS_DATA_STRUCT \
  IoMark mark; \
  IoTag *tag; \
  PHash *slots; \
  IoObject *proto; \
  IoCoroutine *coro; \
  int persistentId;
*/

struct IoObject
{
  IoMark mark; 
  IoTag *tag; 
  PHash *slots; 
  IoObject *proto; 
  IoCoroutine *coro; 
  int persistentId;
  void *data; 
};


#endif
