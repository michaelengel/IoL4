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

#ifdef PHASH_C 
#define IO_IN_C_FILE
#endif
#include "Common_inline.h"
#ifdef IO_DECLARE_INLINES

IOINLINE void PHash_clean(PHash *self)
{ memset(self->records, 0x0, sizeof(PHashRecord)*self->size); }

IOINLINE void *PHash_at_(PHash *self, void *key)
{
  PHashRecord *record = PHASH_RECORDAT_(self, PHASHFUNCTION(key, self->sizeMinusOne));
  return ( record->key == key ) ? record->value : (void *)NULL;
}

IOINLINE unsigned char PHash_at_put_(PHash *self, void *key, void *value)
{
  PHashRecord *record = PHASH_RECORDAT_(self, PHASHFUNCTION(key, self->sizeMinusOne));
  if (record->key)
  {
    /* already a matching key, replace it */
    if (record->key == key)
    {
      if (record->value == value) return 0; /* return 0 if no change */
      record->value = value; 
      return 1;
    }
    /* collision on non matching key, resize until no collision */
    while ( record->key ) 
    { 
      PHash_rehash(self);
      record = PHASH_RECORDAT_(self, PHASHFUNCTION(key, self->sizeMinusOne));
    }
  }
  record->key = key;
  record->value = value;
  self->count++;
  return 1;
}

IOINLINE void PHash_removeKey_(PHash *self, void *key)
{
  PHashRecord *record = PHASH_RECORDAT_(self, PHASHFUNCTION(key, self->sizeMinusOne));
  if (record->key)
  {
    self->count--;
    memset(record, 0x0, sizeof(PHashRecord));
  }
}

IOINLINE void PHash_doOnKeyAndValue_(PHash *self, PHashDoCallback *callback)
{
  PHashRecord *record;
  int size = self->size;
  int i;
  for(i=0; i < size; i++)
  {
    record = PHASH_RECORDAT_(self, i);
    if (record->key) 
    { 
      (*callback)(record->key); 
      (*callback)(record->value); 
    }
  }
}

#undef IO_IN_C_FILE
#endif
