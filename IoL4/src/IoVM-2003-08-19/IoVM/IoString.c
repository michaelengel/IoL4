/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#include "IoString.h"
#include "IoState.h"
#include "IoCFunction.h"
#include "IoObject.h"
#include "IoNil.h"
#include "IoNumber.h"
#include "IoList.h"
#include "IoBuffer.h"
#include "IoMessage_parser.h"
#include "IoMap.h"
#include <ctype.h>
#include <errno.h>

#define DATA(self) ((IoStringData *)(self)->data)

int ioStringCompareFunc(void *s1, void *s2)
{
  return strcmp(
    DATA((IoString *)s1)->byteArray->bytes, 
    DATA((IoString *)s2)->byteArray->bytes);
}

int ioStringFindFunc(void *s, void *ioString)
{ return strcmp(s, DATA((IoString *)ioString)->byteArray->bytes); }

IoTag *IoString_tag(void *state)
{
  IoTag *tag = IoTag_newWithName_("String");
  tag->state = state;
  tag->cloneFunc      = (TagCloneFunc *)IoString_rawClone;
  tag->freeFunc       = (TagFreeFunc *)IoString_free;
  tag->compareFunc    = (TagCompareFunc *)IoString_compare;
  tag->printFunc      = (TagPrintFunc *)IoString_print;
  tag->memorySizeFunc = (TagMemorySizeFunc *)IoString_memorySize;
  tag->compactFunc    = (TagCompactFunc *)IoString_compact;
  tag->writeToStoreFunc  = (TagWriteToStoreFunc *)IoString_writeToStore_;
  tag->readFromStoreFunc = (TagReadFromStoreFunc *)IoString_readFromStore_;
  return tag;
}

IoString *IoString_rawClone(IoString *self)
{
  IoObject *child = IoObject_rawClonePrimitive(self);
  
  child->data = calloc(1, sizeof(IoStringData));
  child->tag= self->tag;
  DATA(child)->byteArray = ByteArray_new();
  DATA(child)->isSymbol = (unsigned char)0;
  return child;
}

IoString *IoString_proto(void *state)
{
  IoObject *self = IoObject_new(state);
  
  self->data = calloc(1, sizeof(IoStringData));
  self->tag = IoString_tag(state);
  DATA(self)->byteArray = ByteArray_new();
  DATA(self)->isSymbol = (unsigned char)0;
  IoState_registerProtoWithFunc_(state, self, IoString_proto);
  
  return self;
}

void IoString_writeToStore_(IoString *self, IoStoreHandle *sh)
{
  IoStoreDatum d = IOSTOREDATUM(&(DATA(self)->byteArray->bytes), DATA(self)->byteArray->length);
  sh->onObject_setData_(sh, d);
}

void *IoString_readFromStore_(IoString *self, IoStoreHandle *sh)
{
  IoStoreDatum d = sh->onObject_getData(sh, self);
  return IoState_stringWithCString_length_(IOSTATE, d.data, d.size);
}

void IoString_protoFinish(void *state)
{
  IoObject *self = IoState_protoWithInitFunction_(state, IoString_proto);
  /* accessors */
  IoObject_addMethod_(self, IOSTRING("clone"), IoString_clone);
  IoObject_addMethod_(self, IOSTRING("asNumber"), IoString_asNumber);
  IoObject_addMethod_(self, IOSTRING("asString"), IoString_asString);
  IoObject_addMethod_(self, IOSTRING("asBuffer"), IoString_asBuffer);
  IoObject_addMethod_(self, IOSTRING("at"), IoString_characterAt);
  IoObject_addMethod_(self, IOSTRING("find"), IoString_find);
  IoObject_addMethod_(self, IOSTRING("reverseFind"), IoString_reverseFind);
  IoObject_addMethod_(self, IOSTRING("beginsWith"), IoString_beginsWith);
  IoObject_addMethod_(self, IOSTRING("endsWith"), IoString_endsWith);
  IoObject_addMethod_(self, IOSTRING("removePrefix"), IoString_removePrefix);
  IoObject_addMethod_(self, IOSTRING("removeSuffix"), IoString_removeSuffix);
  IoObject_addMethod_(self, IOSTRING("contains"), IoString_contains);
  IoObject_addMethod_(self, IOSTRING("containsAnyCase"), IoString_containsAnyCase);
  IoObject_addMethod_(self, IOSTRING("length"), IoString_length);
  IoObject_addMethod_(self, IOSTRING("split"), IoString_split);
  IoObject_addMethod_(self, IOSTRING("splitAt"), IoString_splitAt);
  IoObject_addMethod_(self, IOSTRING("splitCount"), IoString_splitCount);
  IoObject_addMethod_(self, IOSTRING("substring"), IoString_substring);
  
  /* manipulation */
  IoObject_addMethod_(self, IOSTRING("append"), IoString_append_);
  IoObject_addMethod_(self, IOSTRING(".."), IoString_append_);
  IoObject_addMethod_(self, IOSTRING("+"), IoString_append_);
  IoObject_addMethod_(self, IOSTRING("insert"), IoString_insert_at_);
  IoObject_addMethod_(self, IOSTRING("removeFromTo"), IoString_removeFrom_to_);
  IoObject_addMethod_(self, IOSTRING("replaceMap"), IoString_replaceMap);
  IoObject_addMethod_(self, IOSTRING("replace"), IoString_replace_with_);
  IoObject_addMethod_(self, IOSTRING("isLowerCase"), IoString_isLowerCase);
  IoObject_addMethod_(self, IOSTRING("isUpperCase"), IoString_isUpperCase);
  IoObject_addMethod_(self, IOSTRING("lower"), IoString_lower);
  IoObject_addMethod_(self, IOSTRING("upper"), IoString_upper);
  IoObject_addMethod_(self, IOSTRING("capitalized"), IoString_capitalized);
  IoObject_addMethod_(self, IOSTRING("isEqualAnyCase"), IoString_isEqualAnyCase);
  IoObject_addMethod_(self, IOSTRING("escape"), IoString_protoEscape);
  IoObject_addMethod_(self, IOSTRING("unescape"), IoString_protoUnescape);

  /* utility */
  IoObject_addMethod_(self, IOSTRING("print"), IoString_printString);
  IoObject_addMethod_(self, IOSTRING("linePrint"), IoString_linePrintString);
  /*Tag_addMethod(tag, "doString"), IoString_doString);*/
  /*IoObject_addMethod_(self, IOSTRING("doFile"), IoString_doFile);*/
  IoObject_addMethod_(self, IOSTRING("join"), IoString_join);
  IoObject_addMethod_(self, IOSTRING("leftJustified"), IoString_leftJustified);
  IoObject_addMethod_(self, IOSTRING("rightJustified"), IoString_rightJustified);
  IoObject_addMethod_(self, IOSTRING("centered"), IoString_centered);
  
  /* base conversion */
  IoObject_addMethod_(self, IOSTRING("fromBase"), IoString_fromBase);
  IoObject_addMethod_(self, IOSTRING("toBase"), IoString_toBase);
  
  /* file paths */
  IoObject_addMethod_(self, IOSTRING("appendPath"), IoString_appendPath_);
  IoObject_addMethod_(self, IOSTRING("removeLastPathComponent"), IoString_removeLastPathComponent);
  IoObject_addMethod_(self, IOSTRING("lastPathComponent"), IoString_lastPathComponent);
  IoObject_addMethod_(self, IOSTRING("pathExtension"), IoString_pathExtension);
  
  /* loops */
  IoObject_addMethod_(self, IOSTRING("foreach"), IoString_foreach);
}

/* ----------------------------------------------------------- */

IoString *IoString_new(void *state)
{
  IoObject *proto = IoState_protoWithInitFunction_(state, IoString_proto);
  return IoString_rawClone(proto);
}
  
IoString *IoString_newWithCString_(void *state, char *s)
{ return IoString_newWithCString_length_(state, s, strlen(s)); }

IoString *IoString_newWithCString_length_(void *state, char *s, int length)
{
  IoObject *self = IoString_new(state);
  ByteArray_setData_length_(DATA(self)->byteArray, s, length);
  return self;
}

IoString *IoString_newWithByteArray_copy_(void *state, ByteArray *ba, int copy)
{
  IoObject *self = IoString_new(state);
  if (copy)
  { ByteArray_copy_(DATA(self)->byteArray, ba); }
  else
  {
    ByteArray_free(DATA(self)->byteArray);
    DATA(self)->byteArray = ba;
  }
  return self;
}

IoString *IoString_newWithFormat_(void *state, const char *format, ...)
{
  ByteArray *ba;
  va_list ap;
  va_start(ap, format);
  ba = ByteArray_newWithVargs_(format, ap);
  va_end(ap);
  return IoState_stringWithByteArray_copy_(state, ba, 0);
}

IoString *IoString_newFromFilePath_(void *state, char *path)
{
  ByteArray *ba = ByteArray_new();
  ByteArray_readFromFilePath_(ba, path);
  return IoState_stringWithByteArray_copy_(state, ba, 0);
}

IoString *IoString_newCopy(IoObject *self)
{ return IoString_newWithCString_(IOSTATE, DATA(self)->byteArray->bytes); }

void IoString_free(IoObject *self) 
{ 
  if (DATA(self)->isSymbol) { IoState_removeLiteralString_(IOSTATE, self); }
  ByteArray_free(DATA(self)->byteArray);
  free(self->data);
}

ByteArray *IoString_rawByteArray(IoString *self)
{ return DATA(self)->byteArray; }

int IoString_compare(IoObject *self, IoString *v) 
{ 
  if (ISSTRING(v)) { if (self == v) return 0; }
  if (ISSTRING(v)||ISBUFFER(v)) 
  { 
    int c = strcmp(DATA(self)->byteArray->bytes, DATA(v)->byteArray->bytes);
    /*printf("strcmp(%s, %s) = %i\n", CSTRING(self), CSTRING(v), c);*/
    return c; 
  }
  return ((ptrdiff_t)self->tag) - ((ptrdiff_t)v->tag);
}

void IoString_print(IoObject *self)
{ 
  ByteArray *ba = ByteArray_new();
  ByteArray_appendAndEscapeCString_(ba, CSTRING((IoString *)self));
  IoState_justPrint_(IOSTATE, "\""); 
  IoState_justPrint_(IOSTATE, ByteArray_asCString(ba)); 
  IoState_justPrint_(IOSTATE, "\""); 
  /*IoState_print_(IOSTATE, " %p", self); */
  /*IoState_print_(IOSTATE, "String %p \"%s\"", self, ByteArray_asCString(ba)); */
  ByteArray_free(ba);
}

size_t IoString_memorySize(IoObject *self)
{ return sizeof(IoString) + ByteArray_memorySize(DATA(self)->byteArray); }

void IoString_compact(IoObject *self) { ByteArray_compact(DATA(self)->byteArray); }
char *IoString_asCString(IoObject *self) { return (char *)(DATA(self)->byteArray->bytes); }
size_t IoString_cStringLength(IoObject *self) { return DATA(self)->byteArray->length; }
double IoString_asDouble(IoObject *self) { return strtod(DATA(self)->byteArray->bytes, NULL); }

double IoString_asDoubleFromHex(IoObject *self)
{
  char *s = IoString_asCString(self);
  int i;
  sscanf(s, "%x", &i);
  return (double)i;
}

double IoString_asDoubleFromOctal(IoObject *self)
{
  char *s = IoString_asCString(self);
  int i;
  sscanf(s, "%o", &i);
  return (double)i;
}

int IoString_equalsCString_(IoObject *self, char *s) 
{ return (strcmp(DATA(self)->byteArray->bytes, s) == 0 ); }

int IoString_isNotAlphaNumeric(IoObject *self)
{
  char *s = DATA(self)->byteArray->bytes;
  while (!isalnum(*s) && *s != 0x0) { s++; }
  return (*s == 0x0);
}

void IoString_setIsSymbol_(IoString *self, int i)
{ DATA(self)->isSymbol = (unsigned char)i; }

/* ----------------------------------------------------------- */
char IoString_firstCharacter(IoObject *self)
{ return DATA(self)->byteArray->bytes[0]; }

int IoString_rawLength(IoObject *self)
{ return DATA(self)->byteArray->length; }

IoString *IoString_quoted(IoObject *self)
{
  ByteArray *ba = ByteArray_clone(DATA(self)->byteArray);
  ByteArray_quote(ba);
  return IoState_stringWithByteArray_copy_(IOSTATE, ba, 0);
}

IoString *IoString_rawUnquoted(IoObject *self)
{
  ByteArray *ba = ByteArray_clone(DATA(self)->byteArray);
  ByteArray_unquote(ba);
  return IoState_stringWithByteArray_copy_(IOSTATE, ba, 0);
}

IoString *IoString_rawEscape(IoObject *self)
{
  ByteArray *ba = ByteArray_clone(DATA(self)->byteArray);
  ByteArray_escape(ba);
  return IoState_stringWithByteArray_copy_(IOSTATE, ba, 0);
}

IoString *IoString_rawUnescaped(IoObject *self)
{
  ByteArray *ba = ByteArray_clone(DATA(self)->byteArray);
  ByteArray_unescape(ba);
  return IoState_stringWithByteArray_copy_(IOSTATE, ba, 0);
}

IoObject *IoString_protoEscape(IoObject *self, IoObject *locals, IoMessage *m)
{ return IoString_rawEscape(self); }

IoObject *IoString_protoUnescape(IoObject *self, IoObject *locals, IoMessage *m)
{ return IoString_rawUnescaped(self); }

/* ----------------------------------------------------------- */
IoObject *IoString_asString(IoObject *self, IoObject *locals, IoMessage *m) { return self; }
IoObject *IoString_clone(IoObject *self, IoObject *locals, IoMessage *m) { return self; }

IoObject *IoString_append_(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoObject *other = IoMessage_locals_valueArgAt_(m, locals, 0);
  ByteArray *ba;
  if (ISNUMBER(other)) { other = IoNumber_justAsString((IoNumber *)other, (IoObject *)locals, m); }
  if (!ISSTRING(other))
  {
    IoState_error_description_(IOSTATE, m, 
      "Io.String.invalidArgument",
      "argument 0 to method '%s' must be a number or string, not a '%s'",
      CSTRING(IoMessage_rawMethod(m)), IoObject_name(other));
  }
  ba = ByteArray_clone(DATA(self)->byteArray);
  ByteArray_append_(ba, DATA(other)->byteArray);
  return IoState_stringWithByteArray_copy_(IOSTATE, ba, 0);
}

IoObject *IoString_insert_at_(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoString *otherString = IoMessage_locals_stringArgAt_(m, locals, 0);
  int n = IoMessage_locals_intArgAt_(m, locals, 1);
  ByteArray *ba = ByteArray_clone(DATA(self)->byteArray);
  ByteArray_insert_at_(ba, DATA(otherString)->byteArray, n);
  return IoState_stringWithByteArray_copy_(IOSTATE, ba, 0);
}

IoObject *IoString_removeFrom_to_(IoObject *self, IoObject *locals, IoMessage *m)
{
  int from = IoMessage_locals_intArgAt_(m, locals, 0);
  int to = IoMessage_locals_intArgAt_(m, locals, 1);
  ByteArray *ba = ByteArray_clone(DATA(self)->byteArray);
  ByteArray_removeFrom_to_(ba, from, to);
  return IoState_stringWithByteArray_copy_(IOSTATE, ba, 0);
}

IoObject *IoString_printString(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoState_justPrint_(IOSTATE, ByteArray_asCString(DATA(self)->byteArray));
  return self;
}

IoObject *IoString_linePrintString(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoState_justPrint_(IOSTATE, ByteArray_asCString(DATA(self)->byteArray));
  IoState_justPrint_(IOSTATE, "\n");
  return self;
}

IoObject *IoString_length(IoObject *self, IoObject *locals, IoMessage *m)
{ return IoNumber_newWithDouble_(IOSTATE, (double)DATA(self)->byteArray->length); }

IoObject *IoString_replaceMap(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoMap *map = IoMessage_locals_mapArgAt_(m, locals, 0);
  Hash *hash = IoMap_rawHash(map);
  IoString *subString = Hash_firstKey(hash);
  ByteArray *ba = ByteArray_clone(DATA(self)->byteArray);
  while (subString)
  {
    IoString *otherString = Hash_at_(hash, subString);
    ByteArray_replace_with_(ba, DATA(subString)->byteArray, DATA(otherString)->byteArray);
    subString = Hash_nextKey(hash);
  }
  return IoState_stringWithByteArray_copy_(IOSTATE, ba, 0);
}

IoObject *IoString_replace_with_(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoString *subString   = IoMessage_locals_stringOrBufferArgAt_(m, locals, 0);
  IoString *otherString = IoMessage_locals_stringOrBufferArgAt_(m, locals, 1);
  ByteArray *ba = ByteArray_clone(DATA(self)->byteArray);
  ByteArray_replace_with_(ba, DATA(subString)->byteArray, DATA(otherString)->byteArray);
  /*ByteArray_replace_with_output_(DATA(self)->byteArray, DATA(subString)->byteArray, DATA(otherString)->byteArray, DATA(result)->byteArray);*/
  return IoState_stringWithByteArray_copy_(IOSTATE, ba, 0);
}

IoObject *IoString_substring(IoObject *self, IoObject *locals, IoMessage *m)
{
  int fromIndex = IoMessage_locals_intArgAt_(m, locals, 0);
  int last = DATA(self)->byteArray->length;
  ByteArray *ba;
  if (IoMessage_argCount(m) > 1)
  { last = IoMessage_locals_intArgAt_(m, locals, 1); }
  ba = ByteArray_newWithBytesFrom_to_(DATA(self)->byteArray, fromIndex, last);
  return IoState_stringWithByteArray_copy_(IOSTATE, ba, 0);
}

IoObject *IoString_find(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoString *otherString = IoMessage_locals_stringArgAt_(m, locals, 0);
  int from = 0;
  int index;
  if (IoMessage_argCount(m) > 1)
  { from = IoMessage_locals_intArgAt_(m, locals, 1); }
  index = ByteArray_find_from_(DATA(self)->byteArray, DATA(otherString)->byteArray, from);
  if (index == -1) return IONIL(self);
  return IoNumber_newWithDouble_(IOSTATE, (double)index);
}

IoObject *IoString_reverseFind(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoString *otherString = IoMessage_locals_stringArgAt_(m, locals, 0);
  int from = ByteArray_length(DATA(self)->byteArray);
  int index;
  if (IoMessage_argCount(m) > 1)
  { from = IoMessage_locals_intArgAt_(m, locals, 1); }
  index = ByteArray_rFind_from_(DATA(self)->byteArray, DATA(otherString)->byteArray, from);
  if (index == -1) return IONIL(self);
  return IoNumber_newWithDouble_(IOSTATE, (double)index);
}

IoObject *IoString_beginsWith(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoString *otherString = IoMessage_locals_stringArgAt_(m, locals, 0);
  if (ByteArray_beginsWith_(DATA(self)->byteArray, DATA(otherString)->byteArray))
  { return self; } else { return IONIL(self); }
}

IoObject *IoString_endsWith(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoString *otherString = IoMessage_locals_stringArgAt_(m, locals, 0);
  if (ByteArray_endsWith_(DATA(self)->byteArray, DATA(otherString)->byteArray))
  { return self; } else { return IONIL(self); }
}

IoObject *IoString_removePrefix(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoString *otherString = IoMessage_locals_stringArgAt_(m, locals, 0);
  if (ByteArray_beginsWith_(DATA(self)->byteArray, DATA(otherString)->byteArray))
  { 
    ByteArray *ba = ByteArray_clone(DATA(self)->byteArray);
    ByteArray_removeFrom_to_(ba, 0, ByteArray_length(DATA(otherString)->byteArray)-1); 
    return IoState_stringWithByteArray_copy_(IOSTATE, ba, 0);
  }
  return self; 
}

IoObject *IoString_removeSuffix(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoString *otherString = IoMessage_locals_stringArgAt_(m, locals, 0);
  if (ByteArray_endsWith_(DATA(self)->byteArray, DATA(otherString)->byteArray))
  { 
    ByteArray *ba = ByteArray_clone(DATA(self)->byteArray);
    ByteArray_removeFrom_to_(ba, 
      ByteArray_length(ba) + 1 - ByteArray_length(DATA(otherString)->byteArray), 
      ByteArray_length(ba)); 
    return IoState_stringWithByteArray_copy_(IOSTATE, ba, 0);
  }
  return self; 
}

IoObject *IoString_contains(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoString *otherString = IoMessage_locals_stringArgAt_(m, locals, 0);
  if (ByteArray_contains_(DATA(self)->byteArray, DATA(otherString)->byteArray))
  { return self; } else { return IONIL(self); }
}

IoObject *IoString_containsAnyCase(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoString *otherString = IoMessage_locals_stringArgAt_(m, locals, 0);
  if (ByteArray_containsAnyCase_(DATA(self)->byteArray, DATA(otherString)->byteArray))
  { return self; } else { return IONIL(self); }
}

IoObject *IoString_isLowerCase(IoObject *self, IoObject *locals, IoMessage *m)
{ return (ByteArray_isLowerCase(DATA(self)->byteArray)) ? (IoObject *)self : IONIL(self); }

IoObject *IoString_isUpperCase(IoObject *self, IoObject *locals, IoMessage *m)
{ return (ByteArray_isUpperCase(DATA(self)->byteArray)) ? (IoObject *)self : IONIL(self); }

IoObject *IoString_lower(IoObject *self, IoObject *locals, IoMessage *m)
{ 
  if (ByteArray_isLowerCase(DATA(self)->byteArray)) { return self; }
  else
  {
    ByteArray *ba = ByteArray_clone(DATA(self)->byteArray);
    ByteArray_lower(ba); 
    return IoState_stringWithByteArray_copy_(IOSTATE, ba, 0);
  }
}

IoObject *IoString_upper(IoObject *self, IoObject *locals, IoMessage *m)
{ 
  if (ByteArray_isUpperCase(DATA(self)->byteArray)) { return self; }
  else
  {
    ByteArray *ba = ByteArray_clone(DATA(self)->byteArray);
    ByteArray_upper(ba); 
    return IoState_stringWithByteArray_copy_(IOSTATE, ba, 0);
  }
}

IoObject *IoString_capitalized(IoObject *self, IoObject *locals, IoMessage *m)
{ 
  char firstchar = ByteArray_at_(DATA(self)->byteArray, 0);
  if (firstchar == toupper(firstchar)) { return self; }
  else
  {
    ByteArray *ba = ByteArray_clone(DATA(self)->byteArray);
    ByteArray_at_put_(ba, 0, toupper(firstchar)); 
    return IoState_stringWithByteArray_copy_(IOSTATE, ba, 0);
  }
}


IoObject *IoString_isEqualAnyCase(IoObject *self, IoObject *locals, IoMessage *m)
{ 
  IoString *otherString = IoMessage_locals_stringArgAt_(m, locals, 0);
  if (ByteArray_equalsAnyCase_(DATA(self)->byteArray, DATA(otherString)->byteArray)) { return self; }
  return IONIL(self);
}

/*
IoObject *IoString_doFile(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoList *argsList = IoList_new(IOSTATE);
  int argn = 0;
  IoObject *arg = IoMessage_locals_valueArgAt_(m, locals, argn);
  while (arg && !ISNIL(arg))
  {
    IoList_rawAdd_(argsList, arg);
    argn++;
    arg = IoMessage_locals_valueArgAt_(m, locals, argn);
  }
  if (IoList_rawCount(argsList))
  { 
    IoObject *lobby = (IoObject *)IoObject_getSlot_((IoObject *)locals, IOSTRING("Lobby"));
    if (ISNIL(lobby))
    { IoState_error_description_(IOSTATE, m, "Io.String", "Unable to find Lobby slot for doFile context"); }
    IoObject_setSlot_to_(lobby, IOSTRING("args"), argsList); 
  }
  return IoState_doFile_(IOSTATE, CSTRING(self)); 
}
*/

IoObject *IoString_asNumber(IoObject *self, IoObject *locals, IoMessage *m)
{  
  char *s = DATA(self)->byteArray->bytes;
  char *endp;
  double d = strtod(s, &endp);
  if (errno == ERANGE || endp == s) { return IONIL(self); }
  return IoNumber_newWithDouble_(IOSTATE, d); 
}

IoObject *IoString_join(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoList *ioStrings = (IoList *)IoMessage_locals_valueArgAt_(m, locals, 0);
  IoString *separator = 0x0;
  IOASSERT(ISLIST(ioStrings), "String join() requires a list argument");
  if (IoMessage_argCount(m) == 2)
  { separator = IoMessage_locals_stringArgAt_(m, locals, 1); }
  
  {
    List *strings = IoList_rawList(ioStrings);
    int i;
    int size = 0;
    ByteArray *ba = ByteArray_new();
    for (i=0; i< List_count(strings); i++)
    {
      IoString *string = LIST_AT_(strings, i);
      IOASSERT(ISSTRING(string), "String join() requires all elements of it's list argument to be strings");
      size += ByteArray_length(DATA(string)->byteArray);
    }
    if (separator) { size += IoString_cStringLength(separator)*(List_count(strings)-1); }
    
    ByteArray_sizeTo_(ba, size);
    for (i=0; i< List_count(strings); i++)
    {
      IoString *string = LIST_AT_(strings, i);
      ByteArray_append_(ba, DATA(string)->byteArray);
      if (separator && i != List_count(strings)-1) { ByteArray_append_(ba, DATA(separator)->byteArray); }
    }
    return IoState_stringWithByteArray_copy_(IOSTATE, ba, 0);
  }
}

IoObject *IoString_splitCount(IoObject *self, IoObject *locals, IoMessage *m)
{
  int splitCount = 0;
  if (!IoMessage_argCount(m))
  {
      IoState_error_description_(IOSTATE, m, 
        "Io.String.invalidArgument", "String split() requires an argument");
  }
  else
  {
    List *strings = List_new();
    int argn = 0;
    IoObject *arg = IoMessage_locals_valueArgAt_(m, locals, argn);
    while (arg && !ISNIL(arg))
    {
      if (!ISSTRING(arg))
      {
        List_free(strings);
        IoState_error_description_(IOSTATE, m, "String.split.invalidArgument", 
          "String split() only takes strings arguments, not %ss", IoObject_name(arg));
      }
      List_add_(strings, arg);
      argn++;
      arg = IoMessage_locals_valueArgAt_(m, locals, argn);
    }

    {
    int i, n = 0;
    while (n < DATA(self)->byteArray->length)
    {
      int minNext = DATA(self)->byteArray->length;
      int matchLength = DATA(self)->byteArray->length - n;
      for (i=0; i < List_count(strings); i++)
      {
        IoString *string = LIST_AT_(strings, i);
        char *s = strstr(DATA(self)->byteArray->bytes + n, DATA(string)->byteArray->bytes);
        if (s)
        {
          int next = s - (char *)DATA(self)->byteArray->bytes;
          if (next < minNext) 
          { minNext = next; matchLength = DATA(string)->byteArray->length; }
        }
      }
      {
        splitCount++;
        n = minNext + matchLength;
      }
    }
    }
    List_free(strings);
  }
  return IoNumber_newWithDouble_(IOSTATE, splitCount);
}

IoObject *IoString_split(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoList *splitStrings = IoList_new(IOSTATE);
  if (!IoMessage_argCount(m))
  {
      IoState_error_description_(IOSTATE, m, 
        "Io.String.invalidArgument", "String split() requires an argument");
  }
  else
  {
    List *strings = List_new();
    int argn = 0;
    IoObject *arg = IoMessage_locals_valueArgAt_(m, locals, argn);
    while (arg && !ISNIL(arg))
    {
      if (!ISSTRING(arg))
      {
        List_free(strings);
        IoState_error_description_(IOSTATE, m, "String.split.invalidArgument", 
          "String split() only takes strings arguments, not %ss", IoObject_name(arg));
      }
      List_add_(strings, arg);
      argn++;
      arg = IoMessage_locals_valueArgAt_(m, locals, argn);
    }

    {
    int i, n = 0;
    while (n < DATA(self)->byteArray->length)
    {
      int minNext = DATA(self)->byteArray->length;
      int matchLength = DATA(self)->byteArray->length - n;
      for (i=0; i < List_count(strings); i++)
      {
        IoString *string = LIST_AT_(strings, i);
        char *s = strstr(DATA(self)->byteArray->bytes + n, DATA(string)->byteArray->bytes);
        if (s)
        {
          int next = s - (char *)DATA(self)->byteArray->bytes;
          if (next < minNext) 
          { 
            /*printf("match '%s' at %i\n", s, next);*/
            minNext = next; matchLength = DATA(string)->byteArray->length; 
          }
        }
      }
      {
        /*printf("string: '%s', length: %i\n", DATA(self)->byteArray->bytes + n, minNext - n);*/
        IoString *newString = IoState_stringWithCString_length_(IOSTATE, 
            DATA(self)->byteArray->bytes + n, minNext - n);
        IoList_rawAdd_(splitStrings, newString);
        n = minNext + matchLength;
      }
    }
    }
    List_free(strings);
  }
  return splitStrings;
}

IoObject *IoString_splitAt(IoObject *self, IoObject *locals, IoMessage *m)
{
  int index = IoMessage_locals_intArgAt_(m, locals, 0);
  IoList *splitStrings = IoList_new(IOSTATE);
  ByteArray_wrapPos_(DATA(self)->byteArray, index);
  {
  char *s = ByteArray_asCString(DATA(self)->byteArray);
  IoString *s1 = IoState_stringWithCString_length_(IOSTATE, s, index);
  IoString *s2 = IoState_stringWithCString_(IOSTATE, s + index);
  IoList_rawAdd_(splitStrings, (IoObject *)s1);
  IoList_rawAdd_(splitStrings, (IoObject *)s2);
  }
  return splitStrings;
}

IoObject *IoString_asBuffer(IoObject *self, IoObject *locals, IoMessage *m)
{ return IoBuffer_newWithByteArray_copy_(IOSTATE, DATA(self)->byteArray, 1); }

IoObject *IoString_characterAt(IoObject *self, IoObject *locals, IoMessage *m)
{
  int index = IoMessage_locals_intArgAt_(m, locals, 0);
  char c = (char)ByteArray_at_(DATA(self)->byteArray, index);
  return IONUMBER(c);
}

IoObject *IoString_leftJustified(IoObject *self, IoObject *locals, IoMessage *m)
{
  int width = IoMessage_locals_intArgAt_(m, locals, 0);
  int len = ByteArray_length(DATA(self)->byteArray);
  char withChar = ' ';
  ByteArray *result;

  if (width <= len)
  {
    /* Perhaps we should copy, even still? */
    return self;
  }
  else if (IoMessage_argCount(m) > 1)
  { withChar = (char)IoMessage_locals_intArgAt_(m, locals, 1); }

  result = ByteArray_newWithSize_(width);
  memcpy(result->bytes, DATA(self)->byteArray->bytes, len);
  memset(result->bytes + len, withChar, width - len);
  return IoState_stringWithByteArray_copy_(IOSTATE, result, 0);
}

IoObject *IoString_rightJustified(IoObject *self, IoObject *locals, IoMessage *m)
{
  int width = IoMessage_locals_intArgAt_(m, locals, 0);
  int len = ByteArray_length(DATA(self)->byteArray);
  char withChar = ' ';
  ByteArray *result;

  if (width <= len)
  {
    /* Perhaps we should copy, even still? */
    return self;
  }
  else if (IoMessage_argCount(m) > 1)
  { withChar = (char)IoMessage_locals_intArgAt_(m, locals, 1); }

  result = ByteArray_newWithSize_(width);
  memset(result->bytes, withChar, width - len);
  memcpy(result->bytes + width - len, DATA(self)->byteArray->bytes, len);
  return IoState_stringWithByteArray_copy_(IOSTATE, result, 0);
}

IoObject *IoString_centered(IoObject *self, IoObject *locals, IoMessage *m)
{
  int width = IoMessage_locals_intArgAt_(m, locals, 0);
  int len = ByteArray_length(DATA(self)->byteArray);
  char withChar = ' ';
  ByteArray *result;
  int leftPadding;

  if (width <= len)
  {
    /* Perhaps we should copy, even still? */
    return self;
  }
  else if (IoMessage_argCount(m) > 1)
  { withChar = (char)IoMessage_locals_intArgAt_(m, locals, 1); }

  result = ByteArray_newWithSize_(width);
  leftPadding = (width - len) / 1.9; /* Tend towards right edge. */
  memset(result->bytes, withChar, leftPadding);
  memcpy(result->bytes + leftPadding, DATA(self)->byteArray->bytes, len);

  len += leftPadding;
  memset(result->bytes + len, withChar, width - len);
  return IoState_stringWithByteArray_copy_(IOSTATE, result, 0);
}


IoObject *IoString_fromBase(IoObject *self, IoObject *locals, IoMessage *m)
{
  int base = IoMessage_locals_intArgAt_(m, locals, 0);
  char *s = CSTRING(self);
  unsigned long r = strtoul(s, NULL, base);

  if (errno == EINVAL)
  {
    errno = 0;
    IoState_error_description_(IOSTATE, m, 
      "Io.String.invalidArgument", "conversion from base %i not supported", base);
  }
  return IONUMBER(r);
}

IoObject *IoString_toBase(IoObject *self, IoObject *locals, IoMessage *m)
{
  const char * const table = "0123456789abcdefghijklmnopqrstuvwxyz";
  int base = IoMessage_locals_intArgAt_(m, locals, 0);
  unsigned long n = (unsigned long) IoString_asDouble(self);
  char buf[64], *ptr = buf + 64;

  if (base < 2 || base > 36)
  {
    IoState_error_description_(IOSTATE, m, 
      "Io.String.invalidArgument", "conversion to base %i not supported", base);
  }

  /* Build the converted string backwards. */
  *(--ptr) = '\0';

  if (n == 0)
    *(--ptr) = '0';
  else
  {
    do *(--ptr) = table[n % base];
    while ((n /= base) != 0);
  }

  return  IOSTRING(ptr);
}

/* --- path operations --------------------------------------- */
IoObject *IoString_appendPath_(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoString *component = IoMessage_locals_stringArgAt_(m, locals, 0);
  ByteArray *path = ByteArray_clone(DATA(self)->byteArray);
  ByteArray_appendPathCString_(path, DATA(component)->byteArray->bytes);
  return IoState_stringWithByteArray_copy_(IOSTATE, path, 0);
}

IoObject *IoString_removeLastPathComponent(IoObject *self, IoObject *locals, IoMessage *m)
{
  ByteArray *path = ByteArray_clone(DATA(self)->byteArray);
  ByteArray_removeLastPathComponent(path);
  return IoState_stringWithByteArray_copy_(IOSTATE, path, 0);
}

IoObject *IoString_lastPathComponent(IoObject *self, IoObject *locals, IoMessage *m)
{
  ByteArray *ba = ByteArray_lastPathComponent(DATA(self)->byteArray);
  return IoState_stringWithByteArray_copy_(IOSTATE, ba, 0);
}

IoObject *IoString_pathExtension(IoObject *self, IoObject *locals, IoMessage *m)
{
  ByteArray *path = ByteArray_pathExtension(DATA(self)->byteArray);
  return IoState_stringWithByteArray_copy_(IOSTATE, path, 0);
}

IoObject *IoString_foreach(IoObject *self, void *locals, IoMessage *m)
{
  IoMessage_assertArgCount_receiver_(m, 3, self);
  {
    IoObject *result = IONIL(self);
    IoMessage *indexMessage = IoMessage_rawArgAt_(m, 0);
    IoMessage *valueMessage = IoMessage_rawArgAt_(m, 1);
    IoMessage *doMessage    = IoMessage_rawArgAt_(m, 2);
    
    IoString *indexSlotName = IoMessage_rawMethod(indexMessage);
    IoNumber *index = IoObject_getSlot_(locals, indexSlotName);

    IoString *characterSlotName = IoMessage_rawMethod(valueMessage);
    IoNumber *character = IoObject_getSlot_(locals, characterSlotName);

    int i;
    if (!index || !ISNUMBER(index))
    {
      IoObject_setSlot_to_(locals, indexSlotName, IoNumber_newWithDouble_(IOSTATE, 0));
      index = IoObject_getSlot_(locals, indexSlotName); /* numbers  copied by value! */
    }
    if (!character || !ISNUMBER(character))
    {
      IoObject_setSlot_to_(locals, characterSlotName, IoNumber_newWithDouble_(IOSTATE, 0));
      character = IoObject_getSlot_(locals, characterSlotName); /* numbers  copied by value! */
    }
    for (i = 0; i < ByteArray_length(DATA(self)->byteArray); i++)
    {
      IoNumber_rawSet(character, (double)ByteArray_at_(DATA(self)->byteArray, i));
      IoNumber_rawSet(index, i);
      result = IoMessage_locals_performOn_(doMessage, locals, locals);\
      switch (IoMessage_stopStatus(doMessage))
      {
            case MESSAGE_STOP_STATUS_RETURN:
              return result;
            case MESSAGE_STOP_STATUS_BREAK:
              IoMessage_resetStopStatus(m); 
              break;
            case MESSAGE_STOP_STATUS_CONTINUE:
              IoMessage_resetStopStatus(m); 
      }
    }
    return result;
  }
}


