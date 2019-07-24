/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#ifndef IOSTRING_DEFINED
#define IOSTRING_DEFINED 1

#include "base/Common.h"
#include "base/ByteArray.h"
#include "IoObject.h"

#define IOSTRING(s)        IoState_stringWithCString_((IoState*)IOSTATE, s)
#define CSTRING(uString)  IoString_asCString(uString)
#define ISSTRING(self) (self->tag->cloneFunc == (TagCloneFunc *)IoString_rawClone)

#if !defined(IoString_DEFINED) 
  #define IoString_DEFINED
  typedef IoObject IoString;
#endif

typedef struct
{
  ByteArray *byteArray;
  unsigned char isSymbol; /* move into mark to save memory? */
} IoStringData;

int ioStringCompareFunc(void *s1, void *s2);
int ioStringFindFunc(void *s, void *ioString);

void IoString_addProtoMethods(IoObject *self); /* since this requires IoString */ 
IoString *IoString_proto(void *state);
void IoString_protoFinish(void *state);

void IoString_writeToStore_(IoString *self, IoStoreHandle *sh);
void *IoString_readFromStore_(IoString *self, IoStoreHandle *sh);

IoString *IoString_newWithCString_(void *state, char *s);
IoString *IoString_newWithCString_length_(void *state, char *s, int length);
IoString *IoString_newWithByteArray_copy_(void *state, ByteArray *ba, int copy);
IoString *IoString_newWithFormat_(void *state, const char *format, ...);
IoString *IoString_newFromFilePath_(void *state, char *path);
IoString *IoString_newCopy(IoObject *self);

IoString *IoString_rawClone(IoString *self);
void IoString_free(IoString *self);
int IoString_compare(IoString *self, IoString *v);
void IoString_print(IoString *self);
size_t IoString_memorySize(IoString *self);
void IoString_compact(IoString *self);

void IoString_setIsSymbol_(IoString *self, int i);
ByteArray *IoString_rawByteArray(IoString *self);
char *IoString_asCString(IoString *self);
size_t IoString_cStringLength(IoString *self);
double IoString_asDouble(IoString *self);
double IoString_asDoubleFromHex(IoString *self);
double IoString_asDoubleFromOctal(IoString *self);
int IoString_equalsCString_(IoString *self, char *s);

int IoString_isNotAlphaNumeric(IoObject *self);

/* ----------------------------------------------------------- */
char IoString_firstCharacter(IoObject *self);
int IoString_rawLength(IoObject *self);

IoString *IoString_quoted(IoObject *self);
IoString *IoString_rawUnquoted(IoObject *self);

IoString *IoString_rawEscape(IoObject *self);
IoString *IoString_rawUnescaped(IoObject *self);

IoObject *IoString_protoEscape(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoString_protoUnescape(IoObject *self, IoObject *locals, IoMessage *m);

/* ----------------------------------------------------------- */
IoObject *IoString_asString(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoString_clone(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoString_append_(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoString_insert_at_(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoString_removeFrom_to_(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoString_printString(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoString_linePrintString(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoString_length(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoString_replaceMap(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoString_replace_with_(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoString_substring(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoString_find(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoString_reverseFind(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoString_beginsWith(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoString_endsWith(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoString_removePrefix(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoString_removeSuffix(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoString_contains(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoString_containsAnyCase(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoString_isLowerCase(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoString_isUpperCase(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoString_lower(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoString_upper(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoString_capitalized(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoString_isEqualAnyCase(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoString_doFile(IoObject *self, IoObject *locals, IoMessage *m);
/*IoObject *IoString_doString(IoObject *self, IoObject *locals, IoMessage *m);*/
IoObject *IoString_asNumber(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoString_join(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoString_splitCount(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoString_split(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoString_splitAt(IoObject *self, IoObject *locals, IoMessage *m);
/* ----------------------------------------------------------- */
IoObject *IoString_asBuffer(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoString_characterAt(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoString_fromBase(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoString_toBase(IoObject *self, IoObject *locals, IoMessage *m);
/* --- path operations --------------------------------------- */
IoObject *IoString_appendPath_(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoString_removeLastPathComponent(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoString_lastPathComponent(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoString_pathExtension(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoString_leftJustified(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoString_rightJustified(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoString_centered(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoString_foreach(IoObject *self, void *locals, IoMessage *m);

#endif
