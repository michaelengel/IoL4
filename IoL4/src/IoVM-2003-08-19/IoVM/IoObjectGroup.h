/* Copyright (c) 2002, Steve Dekorte
 * All rights reserved. See _BSDLicense.txt.
 *
 * IoObjectGroup - holds references to first and last Values
 * in a group of linked Values
 */

#ifndef VALUEGROUP_DEFINED
#define VALUEGROUP_DEFINED 1

#include "base/Common.h"
#include "IoObject.h"
#include "IoWeakLink.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct
{
  IoObject *first;
  IoObject *last;
  IoMark mark;
  void *state;
} IoObjectGroup;

IoObjectGroup *IoObjectGroup_newWithColor_(void *state, unsigned char color);
void IoObjectGroup_free(IoObjectGroup *self);
int IoObjectGroup_count(IoObjectGroup *self);

void IoObject_moveToGroup_(IoObjectGroup *self, IoObjectGroup *other);

void IoObjectGroup_colorValues(IoObjectGroup *self);
void IoObjectGroup_markValues(IoObjectGroup *self);
int IoObjectGroup_freeValues(IoObjectGroup *self);
unsigned char IoObjectGroup_isEmpty(IoObjectGroup *self);

int IoObjectGroup_sharesWithGroup_(IoObjectGroup *self, IoObjectGroup *other);
int IoObjectGroup_hasValue_(IoObjectGroup *self, IoObject *value);

#include "IoObjectGroup_inline.h"

#endif
