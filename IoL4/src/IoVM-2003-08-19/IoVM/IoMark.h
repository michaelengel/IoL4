/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#ifndef IOMARK_DEFINED
#define IOMARK_DEFINED 1

#include "base/Common.h"

#define IOMARK_INITIAL_FREE  0
#define IOMARK_INITIAL_WHITE 3
#define IOMARK_INITIAL_GRAY  2
#define IOMARK_INITIAL_BLACK 1

#define IOMARK_BLACK ((IoState *)IOSTATE)->blackGroup->mark.color
#define IOMARK_GRAY  IOMARK_INITIAL_GRAY
#define IOMARK_WHITE ((IoState *)IOSTATE)->whiteGroup->mark.color

typedef struct
{
  unsigned char color : 2;            /* for garbase collector */
  unsigned char hasWeakLink : 1;      /* marked if there's ever been a weak link to the value */
  unsigned char hasDoneLookup : 1;    /* used to avoid slot lookup loops */
  unsigned char ownsSlots : 1;        /* if true, free slots Hash when freeing object */
  unsigned char ownsData : 1;         /* if true, call freeFunc when freeing object */
  unsigned char isDirty : 1;          /* set to 1 when the object changes it's state */
  unsigned char isReferenced : 1;     /* set to 1 when the object is referenced by another 
                                        this can be used to do immediate recycling of locals objects */
  /*unsigned char canReachOutput : 1;*/   /* possible future use - actor gc use */
  /*unsigned char distributedColor : 2;*/ /* possible future use - distributed gc use */
  void *previous;                     /* an IoObject - next in gc color list */
  void *next;                         /* an IoObject - previous in gc color list */
} IoMark;

#endif
