/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 *
 * Hash - Hash table with buckets
 * keys and values are references (they are not copied or freed)
 * key pointers are assumed unique
 *
 * Hash can store a large number of values with a reasonable
 * trade off between lookup time and memory usage.
 *
 * Collisions are put in a linked list. The storage sized is increased
 * and the records rehashed if the ratio of records/storage size exceeds
 * a defined amount. The storage size (currently), never shrinks.
 *
 * Note: not optimized
 * should probably use open addressing instead for better use of CPU cache
 */

#ifndef HASH_DEFINED
#define HASH_DEFINED 1

#include "Common.h"

typedef struct HashRecord HashRecord;

struct HashRecord
{
  void *key;
  void *value;
  HashRecord *next; /* next record in the bucket */
  HashRecord *nextRecord; /* link to next record in entire record list */
  HashRecord *previousRecord; /* link to previous record in entire record list */
};

/* --------------------------------------------------------------------- */

typedef struct 
{
    HashRecord **records;
    int size;
    int count;
    HashRecord *first;
    HashRecord *current;
} Hash;

Hash *Hash_new(void);
Hash *Hash_clone(Hash *self);
void Hash_copy_(Hash *self, Hash *other);

void Hash_free(Hash *self);
void Hash_freeRecords(Hash *self);
void Hash_clear(Hash *self);

void Hash_rehash(Hash *self);

void *Hash_firstKey(Hash *self);
void *Hash_nextKey(Hash *self);

void Hash_verify(Hash *self);
int Hash_count(Hash *self);

HashRecord *Hash_recordAt_(Hash *self, int index);
void *Hash_keyAt_(Hash *self, int index);
void *Hash_valueAt_(Hash *self, int index);
int Hash_indexForValue_(Hash *self, void *v);


void *Hash_at_(Hash *self, void *w);
void Hash_at_put_(Hash *self, void *w, void *v);
void Hash_removeKey_(Hash *self, void *w);


/* --- perform -------------------------------------------------- */
typedef void (HashDoCallback)(void *);
void Hash_do_(Hash *self, HashDoCallback *callback);
/* void Hash_doOnKeyAndValue_(Hash *self, HashDoCallback *callback);*/

void Hash_doOnKey_(Hash *self, HashDoCallback *callback);

#include "Hash_inline.h"

#endif
