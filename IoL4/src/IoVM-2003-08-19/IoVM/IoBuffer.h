/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#ifndef IOBUFFER_DEFINED
#define IOBUFFER_DEFINED 1

#include "base/Common.h"
#include "base/ByteArray.h"
#include "IoObject.h"
#include "IoNumber.h"

#define ISBUFFER(self) (self->tag->cloneFunc == (TagCloneFunc *)IoBuffer_rawClone)
#define IOBUFFER(data, size) IoBuffer_newWithData_length_((IoState*)IOSTATE, data, size)

typedef IoObject IoBuffer;

typedef struct
{
  ByteArray *byteArray;
  IoNumber *byteCache;
} IoBufferData;

IoBuffer *IoBuffer_proto(void *state);
IoBuffer *IoBuffer_rawClone(IoBuffer *self);
IoBuffer *IoBuffer_newWithByteArray_copy_(void *state, ByteArray *ba, int copy);
IoBuffer *IoBuffer_newWithData_length_(void *state, unsigned char *s, int length);
IoBuffer *IoBuffer_newWithCString_length_(void *state, char *s, int length);
IoBuffer *IoBuffer_newWithCString_(void *state, char *s);
IoBuffer *IoBuffer_newFromFilePath_(void *state, char *path);
IoBuffer *IoBuffer_newCopy(IoBuffer *self);

void IoBuffer_mark(IoBuffer *self);
void IoBuffer_free(IoBuffer *data);
int IoBuffer_compare(IoBuffer *self, IoBuffer *v);

void IoBuffer_writeToStore_(IoBuffer *self, IoStoreHandle *store);
void *IoBuffer_readFromStore_(IoBuffer *self, IoStoreHandle *store);

char *IoBuffer_asCString(IoBuffer *self);
unsigned char *IoBuffer_bytes(IoBuffer *self);
size_t IoBuffer_rawLength(IoBuffer *self);
ByteArray *IoBuffer_rawByteArray(IoBuffer *self);
double IoBuffer_asDouble(IoBuffer *self);
int IoBuffer_equalsCBuffer_(IoBuffer *self, char *s);

/* ----------------------------------------------------------- */
IoBuffer *IoBuffer_quoted(IoBuffer *self);
IoBuffer *IoBuffer_unquoted(IoBuffer *self);

IoBuffer *IoBuffer_escape(IoBuffer *self);
IoBuffer *IoBuffer_unescape(IoBuffer *self);

/* ----------------------------------------------------------- */
IoObject *IoBuffer_asNumber(IoBuffer *self, IoObject *locals, IoMessage *m);
IoObject *IoBuffer_asBinaryNumber(IoBuffer *self, IoObject *locals, IoMessage *m);

IoObject *IoBuffer_clone(IoBuffer *self, IoObject *locals, IoMessage *m);
IoObject *IoBuffer_append_(IoBuffer *self, IoObject *locals, IoMessage *m);
IoObject *IoBuffer_appendByte_(IoBuffer *self, IoObject *locals, IoMessage *m);
IoObject *IoBuffer_insert_at_(IoBuffer *self, IoObject *locals, IoMessage *m);
IoObject *IoBuffer_removeFrom_to_(IoBuffer *self, IoObject *locals, IoMessage *m);
IoObject *IoBuffer_print(IoBuffer *self, IoObject *locals, IoMessage *m);

IoObject *IoBuffer_length(IoBuffer *self, IoObject *locals, IoMessage *m);
IoObject *IoBuffer_setLength(IoBuffer *self, IoObject *locals, IoMessage *m);
IoObject *IoBuffer_sizeTo_(IoBuffer *self, IoObject *locals, IoMessage *m);
IoObject *IoBuffer_replace_with_(IoBuffer *self, IoObject *locals, IoMessage *m);
IoObject *IoBuffer_at_(IoBuffer *self, IoObject *locals, IoMessage *m);
IoObject *IoBuffer_at_put_(IoBuffer *self, IoObject *locals, IoMessage *m);
IoObject *IoBuffer_from_to_(IoBuffer *self, IoObject *locals, IoMessage *m);
IoObject *IoBuffer_between(IoBuffer *self, IoObject *locals, IoMessage *m);
IoObject *IoBuffer_find_from_(IoBuffer *self, IoObject *locals, IoMessage *m);
IoObject *IoBuffer_lower(IoBuffer *self, IoObject *locals, IoMessage *m);
IoObject *IoBuffer_upper(IoBuffer *self, IoObject *locals, IoMessage *m);

IoObject *IoBuffer_join(IoBuffer *self, IoObject *locals, IoMessage *m);
IoObject *IoBuffer_splitCount(IoBuffer *self, IoObject *locals, IoMessage *m);
IoObject *IoBuffer_split(IoBuffer *self, IoObject *locals, IoMessage *m);
IoObject *IoBuffer_asString(IoBuffer *self, IoObject *locals, IoMessage *m);
IoObject *IoBuffer_clear(IoBuffer *self, IoObject *locals, IoMessage *m);

#endif
