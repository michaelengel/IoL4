/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#ifdef HASH_C 
#define IO_IN_C_FILE
#endif
#include "Common_inline.h"
#ifdef IO_DECLARE_INLINES

#include <stdlib.h>
#include <stddef.h>

IOINLINE HashRecord *HashRecord_newWithKey_value_(void *key, void *value)
{
  HashRecord *self = (HashRecord *)calloc(1, sizeof(HashRecord));
  self->key = key;
  self->value = value;
  return self;
}

IOINLINE void HashRecord_free(HashRecord *self) { free(self); }

IOINLINE void Hash_doOnKeyAndValue_(Hash *self, HashDoCallback *callback)
{
  HashRecord *record = self->first;
  int count = 0;
  while ( record )
  {
    (*callback)(record->key);
    (*callback)(record->value);
    record = record->nextRecord;
    count++;
  }
}

#undef IO_IN_C_FILE
#endif
