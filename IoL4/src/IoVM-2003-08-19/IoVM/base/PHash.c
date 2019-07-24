/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */
 
#define PHASH_C
#include "PHash.h"
#undef PHASH_C
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define PHASH_DEFAULT_SIZE 8  
#define PHASH_RESIZE_FACTOR 1

PHash *PHash_new(void) 
{
  PHash *self = (PHash *)calloc(1, sizeof(PHash)); 
  self->size = PHASH_DEFAULT_SIZE;
  self->sizeMinusOne = self->size - 1;
  self->records = (void *)calloc(1, sizeof(PHashRecord)*self->size);
  self->count = 0;
  return self;
}

PHash *PHash_clone(PHash *self)
{
  PHash *child = PHash_new();
  PHash_copy_(child, self);
  return child;
}

void PHash_free(PHash *self)
{
  free(self->records);
  free(self);
}

int PHash_size(PHash *self) { return self->size; }

size_t PHash_memorySize(PHash *self)
{ return sizeof(PHash) + (self->size*sizeof(void *)); }

void PHash_compact(PHash *self)
{ printf("need to implement PHash_compact\n"); }

void PHash_copy_(PHash *self, PHash *other)
{
  self->size = other->size;
  self->sizeMinusOne = other->sizeMinusOne;
  self->count = other->count;
  self->records = (void *)realloc(self->records, sizeof(PHashRecord)*self->size);
  memcpy(self->records, other->records, sizeof(PHashRecord)*self->size);
}

void PHash_rehash(PHash *self)
{
  unsigned char *records = self->records;
  unsigned char *newRecords = (void *)NULL;
  int size = self->size;
  int i = 0;
  int moved = 0;
  
  while (moved < self->count && i < self->size) 
  {
    size = (size*PHASH_RESIZE_FACTOR) + 1;
    newRecords = (void *)realloc(newRecords, sizeof(PHashRecord)*size);
    memset(newRecords, 0x0, sizeof(PHashRecord)*size);
  
    for (i = 0; moved < self->count && i < self->size; i++)
    {
      PHashRecord *record = (PHashRecord *)(self->records + (i*sizeof(PHashRecord)));
      if (record->key)
      {
        unsigned int hval = PHASHFUNCTION(record->key, size - 1);
        PHashRecord *newRecord = (PHashRecord *)(newRecords + (hval*sizeof(PHashRecord)));
        if (newRecord->key) 
        { 
          /* printf("rehash collision, size = %i count = %i\n", size, self->count); */
          /*free(newRecords);*/
          moved = 0;
          i = 0;
          break; 
        }
        {
        newRecord->key   = record->key;
        newRecord->value = record->value;
        moved++;
/*
        printf("moved %i of %i to %p\n", moved, self->count, (void *)newRecord);
        {
          int m;
          int c = 0;
          printf("new count1 = %i\n", PHash_countRecords_size_(newRecords, size));
          printf("newRecords = %p\n", newRecords);
          for( m=0 ; m < size ; m++ )
          {
           PHashRecord *r = (PHashRecord *)(newRecords + (m*sizeof(PHashRecord)));
           if (r->key) { c++; }
          }
          printf("new count2 = %i\n", c);
          printf("newRecords = %p\n", newRecords);
          printf("new count3 = %i\n", PHash_countRecords_size_(newRecords, size));
        }
*/
        }
      }
    }
  }
/*
  printf("rehased to size:%i count:%i ratio:%f\n", 
    size, self->count, (float)size/(float)self->count);
  printf("old count:%i\n", PHash_doCount(self));
  printf("%p %i bytes [phash]\n", self, (int)(size*sizeof(void *)));
*/
  self->size = size;
  self->sizeMinusOne = self->size - 1;
  self->records = newRecords;
  free(records);
}

void *PHash_first(PHash *self)
{
  self->index = 0;
  return PHash_next(self);
}

void *PHash_next(PHash *self)
{
  PHashRecord *record;
  while (self->index < self->size)
  {
    record = PHASH_RECORDAT_(self, self->index);
    self->index++;
    if (record->key) return record->value;
  }
  return (void *)NULL;
}

void *PHash_firstKey(PHash *self)
{
  self->index = 0;
  return PHash_nextKey(self);
}

void *PHash_nextKey(PHash *self)
{
  PHashRecord *record;
  while (self->index < self->size)
  {
    record = PHASH_RECORDAT_(self, self->index);
    self->index++;
    if (record->key) return record->key;
  }
  return (void *)NULL;
}

int PHash_count(PHash *self) { return self->count; }

int PHash_countRecords_size_(unsigned char *newRecords, int size)
{
  int m;
  int c = 0;
  for (m=0 ; m < size ; m++)
  {
    PHashRecord *r = (PHashRecord *)(newRecords + (m*sizeof(PHashRecord)));
    if (r->key) { c++; }
  }
  return c;
}

int PHash_doCount(PHash *self)
{
  int m;
  int c = 0;
  for (m=0 ; m < self->size ; m++)
  {
    PHashRecord *r = (PHashRecord *)(self->records + (m*sizeof(PHashRecord)));
    if (r->key) { c++; }
  }
  return c;
}

PHashRecord *PHash_recordAtIndex_(PHash *self, int index)
{
  int i = 0;
  int count = 0;
  while (i < self->size)
  {
    PHashRecord *record = PHASH_RECORDAT_(self, i);
    if (record->key) 
    {
      if (index == count) { return record; }
      count++;
    }
    i++;
  }
  return (void *)NULL;
}

void *PHash_keyAt_(PHash *self, int i)
{
  PHashRecord *record = PHash_recordAtIndex_(self, i);
  return (record) ? record->key : (void *)NULL;
}

void *PHash_valueAt_(PHash *self, int i)
{
  PHashRecord *record = PHash_recordAtIndex_(self, i);
  return (record) ? record->value : (void *)NULL;
}

int PHash_indexForValue_(PHash *self, void *v)
{
  int i = 0;
  int count = 0;
  while (i < self->size)
  {
    PHashRecord *record = PHASH_RECORDAT_(self, i);
    if (record->key)
    {
      if (record->value == v) { return count; }
      count++;
    }
    i++;
  }
  return -1;
}

void *PHash_firstKeyForValue_(PHash *self, void *v)
{
  int i = 0;
  int count = 0;
  while (i < self->size)
  {
    PHashRecord *record = PHASH_RECORDAT_(self, i);
    if (record->key)
    {
      if (record->value == v) { return record->key; }
      count++;
    }
    i++;
  }
  return NULL;
}

/* --- perform -------------------------------------------------- */

void PHash_do_(PHash *self, PHashDoCallback *callback)
{
  PHashRecord *record;
  int size = self->size;
  int i;
  for(i=0; i < size; i++)
  {
    record = PHASH_RECORDAT_(self, i);
    if (record->key) { (*callback)(record->value); }
  }
}

void PHash_doOnKeys_(PHash *self, PHashDoCallback *callback)
{
  PHashRecord *record;
  int size = self->size;
  int i;
  for (i=0; i < size; i++)
  {
    record = PHASH_RECORDAT_(self, i);
    if (record->key) { (*callback)(record->key); }
  }
}

