/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 *
 * ByteArray - A mutable array of unsigned chars
 * supports basic operations, searching and 
 * reading/writing to a file
 */
 
#ifndef BYTEARRAY_DEFINED
#define BYTEARRAY_DEFINED 1

#include "Common.h"
#include <stdarg.h>
#include <stdio.h>
#include <stddef.h>

#define BYTEARRAY_BYTES_PER_CHARACTER 1

/* put defines for tolower, etc here */

typedef struct
{
  unsigned char *bytes;
  long length;
  long memSize;
} ByteArray ;

ByteArray *ByteArray_new(void);
ByteArray *ByteArray_newWithData_size_copy_(unsigned char *buf, int size, char copy);
ByteArray *ByteArray_newWithCString_length_(char *s, int length);
ByteArray *ByteArray_newWithCString_(char *s);
ByteArray *ByteArray_newWithSize_(int size);
ByteArray *ByteArray_newWithBytesFrom_to_(ByteArray *self, int startpos, int endpos);
ByteArray *ByteArray_clone(ByteArray *self);

void ByteArray_free(ByteArray *self);
unsigned char * ByteArray_bytes(ByteArray *self);
int ByteArray_length(ByteArray *self);
size_t ByteArray_memorySize(ByteArray *self);
void ByteArray_compact(ByteArray *self);

void ByteArray_clear(ByteArray *self);
void ByteArray_sizeTo_(ByteArray *self, int size);
void ByteArray_setLength_(ByteArray *self, int length);
void ByteArray_copy_(ByteArray *self, ByteArray *other);
void ByteArray_setData_length_(ByteArray *self, unsigned char *data, size_t length);

int ByteArray_compare_(ByteArray *self, ByteArray *other);

/* --- character operations ------------- */
int ByteArray_hasDigit(ByteArray *self);
unsigned char ByteArray_at_(ByteArray *self, int pos);
int ByteArray_at_put_(ByteArray *self, int pos, unsigned char c);
void ByteArray_dropLastByte(ByteArray *self);
void ByteArray_removeByteAt_(ByteArray *self, int pos);
void ByteArray_removeCharAt_(ByteArray *self, int pos);
void ByteArray_removeFrom_to_(ByteArray *self, int from, int to);

/* --- access operations ---------------- */
char *ByteArray_asCString(ByteArray *self);

/* --- escape & quote ------------------- */
void ByteArray_escape(ByteArray *self);
void ByteArray_unescape(ByteArray *self);

void ByteArray_quote(ByteArray *self);
void ByteArray_unquote(ByteArray *self);

/* --- modification operations ---------- */
void ByteArray_appendChar_(ByteArray *self, char c);
void ByteArray_append_(ByteArray *self, ByteArray *other);
void ByteArray_appendCString_(ByteArray *self, char *s);
void ByteArray_appendAndEscapeCString_(ByteArray *self, char *s);
void ByteArray_appendBytes_length_(ByteArray *self, unsigned char *bytes, long length);

void ByteArray_prepend_(ByteArray *self, ByteArray *other);
void ByteArray_prependCString_(ByteArray *self, char *s);
void ByteArray_prependBytes_length_(ByteArray *self, unsigned char *bytes, long length);

void ByteArray_insert_at_(ByteArray *self, ByteArray *other, int pos);
void ByteArray_insertCString_at_(ByteArray *self, char *s, int pos);
void ByteArray_insertBytes_length_at_(ByteArray *self, unsigned char *bytes, long length, int pos);

int ByteArray_isLowerCase(ByteArray *self);
int ByteArray_isUpperCase(ByteArray *self);
void ByteArray_lower(ByteArray *self);
void ByteArray_upper(ByteArray *self);

/* --- string comparision operations ------- */
int ByteArray_equals_(ByteArray *self, ByteArray *other);
int ByteArray_equalsAnyCase_(ByteArray *self, ByteArray *other);
int ByteArray_contains_(ByteArray *self, ByteArray *other);
int ByteArray_containsAnyCase_(ByteArray *self, ByteArray *other);
int ByteArray_find_(ByteArray *self, ByteArray *other);
int ByteArray_beginsWith_(ByteArray *self, ByteArray *other);
int ByteArray_endsWith_(ByteArray *self, ByteArray *other);

/* --- search operations ------------------- */
int ByteArray_findAnyCase_(ByteArray *self, ByteArray *other);
int ByteArray_find_from_(ByteArray *self, ByteArray *other, int from);
int ByteArray_findCString_from_(ByteArray *self, char *other, int from);
int ByteArray_rFind_from_(ByteArray *self, ByteArray *other, int from);
int ByteArray_findAnyCase_from_(ByteArray *self, ByteArray *other, int from);
int ByteArray_count_(ByteArray *self, ByteArray *other);

/* --- replace ------------------------------ */
void ByteArray_replaceCString_withCString_(ByteArray *self, char *s1, char *s2);
/* for effiency, replacement is done without allocating a seperate byte array */
void ByteArray_replace_with_(ByteArray *self, ByteArray *substring, ByteArray *other);
void ByteArray_replace_with_output_(ByteArray *self, ByteArray *substring, ByteArray *other, ByteArray *output);

/* --- I/O ---------------------------------- */
void ByteArray_print(ByteArray *self);
size_t ByteArray_writeToCStream_(ByteArray *self, FILE *stream);
int ByteArray_readFromFilePath_(ByteArray *self, char *path); /* returns 0 on success */
unsigned char ByteArray_readLineFromCStream_(ByteArray *self, FILE *stream);
int ByteArray_readFromCStream_(ByteArray *self, FILE *stream);
size_t ByteArray_readNumberOfBytes_fromCStream_(ByteArray *self, long length, FILE *stream);

/* --- private utility functions ------------ */
int ByteArray_wrapPos_(ByteArray *self, int pos);

ByteArray *ByteArray_newWithFormat_(const char *format, ...);
ByteArray *ByteArray_newWithVargs_(const char *format, va_list ap);
ByteArray *ByteArray_fromFormat_(ByteArray *self, const char *format, ...);
void ByteArray_fromVargs_(ByteArray *self, const char *format, va_list ap);

/* --- hex conversion ------------------------ */
ByteArray *ByteArray_asNewHexStringByteArray(ByteArray *self);

/* --- file paths ---------------------------- */
void ByteArray_appendPathCString_(ByteArray *self, char *path);
void ByteArray_removeLastPathComponent(ByteArray *self);
void ByteArray_clipBeforeLastPathComponent(ByteArray *self);
ByteArray *ByteArray_lastPathComponent(ByteArray *self);
char *ByteArray_lastPathComponentAsCString(ByteArray *self);
void ByteArray_removePathExtension(ByteArray *self);
ByteArray *ByteArray_pathExtension(ByteArray *self);

#endif

