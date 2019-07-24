/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 *
 * PHash - Perfect Hash
 * keys and values are references (they are not copied or freed)
 * key pointers are assumed unique
 *
 * This is a perfect hash. When a new key collides with an
 * existing one, the storage size is increased by one and the records are
 * rehashed until no collisions are found. This means lookups are
 * fast but the number of keys must remain small(<100) - otherwise 
 * the storage size will get too big.
 */

#ifndef PHASH_DEFINED
#define PHASH_DEFINED 1

#include "Common.h"
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#define PHASHFUNCTION(v, sizeMinusOne) (int)((ptrdiff_t)v)%(sizeMinusOne)
#define PHASH_RECORDAT_(self, index) ((PHashRecord *)(self->records + (index*sizeof(PHashRecord))));

typedef struct 
{
  void *key;
  void *value;
} PHashRecord;

typedef struct 
{
  unsigned char *records;
  int size;
  int sizeMinusOne;
  int count;
  int index;
} PHash;

PHash *PHash_new(void);
void PHash_free(PHash *self);
PHash *PHash_clone(PHash *self);
void PHash_copy_(PHash *self, PHash *other);

int PHash_size(PHash *self);
size_t PHash_memorySize(PHash *self);
void PHash_compact(PHash *self);
void PHash_rehash(PHash *self);

void *PHash_first(PHash *self);
void *PHash_next(PHash *self);

void *PHash_firstKey(PHash *self);
void *PHash_nextKey(PHash *self);

int PHash_count(PHash *self);
int PHash_doCount(PHash *self);
int PHash_countRecords_size_(unsigned char *records, int size);

PHashRecord *PHash_recordAtIndex_(PHash *self, int index);
void *PHash_keyAt_(PHash *self, int index);
void *PHash_valueAt_(PHash *self, int index);
int PHash_indexForValue_(PHash *self, void *v);
void *PHash_firstKeyForValue_(PHash *self, void *v);

/* --- perform -------------------------------------------------- */

typedef void (PHashDoCallback)(void *);
void PHash_do_(PHash *self, PHashDoCallback *callback);
void PHash_doOnKeys_(PHash *self, PHashDoCallback *callback);

#include "PHash_inline.h"

#endif

