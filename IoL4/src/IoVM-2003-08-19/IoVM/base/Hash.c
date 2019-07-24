/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */
 

#define HASH_C
#include "Hash.h"
#undef HASH_C
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef LOW_MEMORY_SYSTEM
  #define HASH_DEFAULT_SIZE 128
  #define HASH_RESIZE_FACTOR 1.2
  #define HASH_COUNT_TO_SIZE_RATIO 5
#else
  #define HASH_DEFAULT_SIZE 128
  #define HASH_RESIZE_FACTOR 1.2
  #define HASH_COUNT_TO_SIZE_RATIO 10
#endif

#define HASHFUNCTION(v, tsize) (unsigned int)((ptrdiff_t)v)%(tsize-1)

/*
enum { HASH_NUMBER_OF_PRIMES = 28 };

static unsigned long Hash_primeList[HASH_NUMBER_OF_PRIMES] = {
    53ul,         97ul,         193ul,       389ul,       769ul,
    1543ul,       3079ul,       6151ul,      12289ul,     24593ul,
    49157ul,      98317ul,      196613ul,    393241ul,    786433ul,
    1572869ul,    3145739ul,    6291469ul,   12582917ul,  25165843ul,
    50331653ul,   100663319ul,  201326611ul, 402653189ul, 805306457ul, 
    1610612741ul, 3221225473ul, 4294967291ul
};
*/

/* --------------------------------------------------------------------- */

Hash *Hash_new(void)
{
  Hash *self = (Hash *)calloc(1, sizeof(Hash));
  self->size = HASH_DEFAULT_SIZE;
  self->records = (HashRecord **)calloc(1, sizeof(HashRecord *)*self->size);
  self->count = 0;
  self->first = (HashRecord *)NULL;
  return self;
}

Hash *Hash_clone(Hash *self)
{
  Hash *child = Hash_new();
  Hash_copy_(child, self);
  return child;
}

void Hash_copy_(Hash *self, Hash *other)
{
  HashRecord *record = other->first;
  while (record)
  {
    Hash_at_put_(self, record->key, record->value);
    record = record->nextRecord;
  }
}

void Hash_free(Hash *self)
{
  Hash_freeRecords(self);
  free(self->records);
  free(self);
}

void Hash_freeRecords(Hash *self)
{
  HashRecord *record = self->first;
  while (record)
  {
    HashRecord *nextRecord = record->nextRecord;
    HashRecord_free(record);
    record = nextRecord;
  }
}

void Hash_clear(Hash *self)
{
  Hash_freeRecords(self);
  self->first = (HashRecord *)NULL;
  memset(self->records, (unsigned char)0x0, sizeof(HashRecord *)*self->size);
}

void Hash_rehash(Hash *self)
{
  HashRecord *record = self->first;
  
  /* clear old bucket links */
  while (record)
  {
    record->next = (HashRecord *)NULL;
    record = record->nextRecord;
  }
    
  self->size = (self->size*HASH_RESIZE_FACTOR) + 1;
  self->records = (HashRecord **)realloc(self->records, sizeof(HashRecord *)*self->size);
  memset(self->records, (unsigned char)0x0, sizeof(HashRecord *)*self->size);
  
  record = self->first;
  while (record)
  {
      unsigned int hval = HASHFUNCTION(record->key, self->size);
      HashRecord *r = self->records[hval];
      if (!r) { self->records[hval] = record; }
      else
      {
        for (;;)
        {
          if (!r->next) { r->next = record; break; }
          r = r->next;
        }
      }
      record = record->nextRecord;
  }
/*
  printf("rehased to size:%i count:%i ratio:%f\n", 
    self->size, self->count, (float)self->size/(float)self->count);
  Hash_verify(self);*/
}

void *Hash_at_(Hash *self, void *key)
{
  unsigned int hval = HASHFUNCTION(key, self->size);
  HashRecord *record = self->records[hval];
  if (record)
  {
    if (record->key == key) { return record->value; }
    {
      HashRecord *lastRecord = record;
      record = record->next;
      while (record)
      {
        if (record->key == key) 
        { 
          /* move to front of bucket */
          lastRecord->next = record->next;
          record->next = self->records[hval];
          self->records[hval] = record;
          return record->value; 
        }
        lastRecord = record;
        record = record->next;
      }
    }
  }
  /*Hash_verify(self);*/
  return (void *)NULL;
}

void Hash_at_put_(Hash *self, void *key, void *value)
{
  unsigned int hval = HASHFUNCTION(key, self->size);
  HashRecord *record = self->records[hval];
  while (record)
  {
    if (record->key == key) 
    { 
      record->value = value; /* don't increment count */
      return; 
    }
    record = record->next;
  }
  /* no match, add new hash record */
  record = HashRecord_newWithKey_value_(key, value);
  
  /* put it in front of bucket */
  if (self->records[hval])
  { record->next = self->records[hval]; }
  self->records[hval] = record;
  
  /* put it in front of enumeration chain */
  record->nextRecord = self->first;
  if (self->first) { self->first->previousRecord = record; }
  self->first = record;

  self->count++;
  if ( self->count > self->size * HASH_COUNT_TO_SIZE_RATIO)
  { Hash_rehash(self); }
  /*Hash_verify(self);*/
}

void Hash_removeKey_(Hash *self, void *key)
{ 
  unsigned int hval = HASHFUNCTION(key, self->size);
  HashRecord *record = self->records[hval];
  HashRecord *lastRecord = (HashRecord *)NULL;
  while (record)
  {
    if (record->key == key) 
    { 
      HashRecord *previousRecord = record->previousRecord;
      HashRecord *nextRecord = record->nextRecord;
      
      /* remove from enumeration chain */
      if (previousRecord)
      { previousRecord-> nextRecord = nextRecord; }
      else
      { self->first = nextRecord; }
      if (nextRecord)
      { nextRecord->previousRecord = previousRecord; }
    
      /* remove from bucket chain */
      if (lastRecord) 
      { lastRecord->next = record->next; }
      else
      { self->records[hval] = record->next; } 
      
      HashRecord_free(record);
      self->count--;
      return;
    }
    record = record->next;
  }
}

void *Hash_firstKey(Hash *self)
{
  self->current = self->first;
  return (self->current) ? self->current->key : (void *)NULL;
}

void *Hash_nextKey(Hash *self)
{
  if (self->current)
  { 
    self->current = self->current->nextRecord; 
    if (self->current)
    { return self->current->key; }
  }
  return (void *)NULL;
}

void Hash_verify(Hash *self)
{
  HashRecord *r = self->first;
  int c = 0;
  
  while (r)
  {
    c++;
    if (!Hash_at_(self, r->key))
    {
      printf("Hash_verify() Hash_at_ failed\n");
      exit(1);
    }      
    r = r->nextRecord;
  }
  
  if (c != self->count)
  {
    printf("Hash_verify() failed - next chain missing items or count wrong\n");
    exit(1);
  }
}

int Hash_count(Hash *self) { return self->count; }

HashRecord *Hash_recordAt_(Hash *self, int index)
{
  int i = 0;
  HashRecord *record = self->first;
  while (record)
  {
    if (i == index) return record; 
    record = record->nextRecord;
    i++;
  }
  return (HashRecord *)NULL;
}

void *Hash_keyAt_(Hash *self, int index)
{
  HashRecord *record = Hash_recordAt_(self, index);
  if (record) { return record->key; }
  return (void *)NULL;
}

void *Hash_valueAt_(Hash *self, int index)
{
  HashRecord *record = Hash_recordAt_(self, index);
  if (record) { return record->value; }
  return (void *)NULL;
}

int Hash_indexForValue_(Hash *self, void *v)
{
  int index = 0;
  HashRecord *record = self->first;
  while (record)
  {
    if (record->value == v) return index;
    record = record->nextRecord;
    index++;
  }
  return -1;
}

/* --- perform -------------------------------------------------- */

void Hash_do_(Hash *self, HashDoCallback *callback)
{
  HashRecord *record = self->first;
  while (record)
  {
    (*callback)(record->value);
    record = record->nextRecord;
  }
} 

void Hash_doOnKey_(Hash *self, HashDoCallback *callback)
{
  HashRecord *record = self->first;
  while (record)
  {
    (*callback)(record->key);
    record = record->nextRecord;
  }
}

