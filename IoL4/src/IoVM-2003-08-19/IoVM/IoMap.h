/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#ifndef IOHASH_DEFINED
#define IOHASH_DEFINED 1

#include "base/Common.h"
#include "IoObject.h"
#include "base/Hash.h"

#define ISMAP(self) (self->tag->cloneFunc == (TagCloneFunc *)IoMap_rawClone)

typedef IoObject IoMap;

typedef struct
{
  Hash *hash;
} IoMapData;

IoMap *IoMap_proto(void *state);
IoMap *IoMap_rawClone(IoMap *self);
IoMap *IoMap_new(void *state);
void IoMap_free(IoMap *self);
void IoMap_mark(IoMap *self);
void IoMap_rawAtPut_(IoMap *self, IoString *k, IoObject *v);
Hash *IoMap_rawHash(IoMap *self);
/* ----------------------------------------------------------- */
IoObject *IoMap_type(IoMap *self, IoObject *locals, IoMessage *m);
IoObject *IoMap_clone(IoMap *self, IoObject *locals, IoMessage *m);
IoObject *IoMap_clear(IoMap *self, IoObject *locals, IoMessage *m);

IoObject *IoMap_at_(IoMap *self, IoObject *locals, IoMessage *m);
IoObject *IoMap_at_put_(IoMap *self, IoObject *locals, IoMessage *m);
IoObject *IoMap_atIfAbsentPut(IoMap *self, IoObject *locals, IoMessage *m);
IoObject *IoMap_remove_(IoMap *self, IoObject *locals, IoMessage *m);
IoObject *IoMap_count(IoMap *self, IoObject *locals, IoMessage *m);
/*
IoObject *IoMap_at_ifAbsent_(IoMap *self, IoObject *locals, IoMessage *m);
IoObject *IoMap_keyAtValue_(IoMap *self, IoObject *locals, IoMessage *m);
*/
IoObject *IoMap_hasKey_(IoMap *self, IoObject *locals, IoMessage *m);
IoObject *IoMap_hasValue_(IoMap *self, IoObject *locals, IoMessage *m);

IoObject *IoMap_keys(IoMap *self, IoObject *locals, IoMessage *m);
IoObject *IoMap_values(IoMap *self, IoObject *locals, IoMessage *m);
IoObject *IoMap_do_(IoMap *self, IoObject *locals, IoMessage *m);
IoObject *IoMap_foreach(IoMap *self, IoObject *locals, IoMessage *m);

#endif
