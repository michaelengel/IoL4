/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#include "IoDate.h"
#include "IoFile.h"
#include "IoString.h"
#include "IoState.h"
#include "IoCFunction.h"
#include "IoObject.h"
#include "IoNil.h"
#include "IoList.h"
#include "IoBuffer.h"
#include "base/ByteArray.h"
#include <errno.h>
#include <stdio.h>

#ifndef _MSC_VER
#include <unistd.h> /* ok, this isn't ANSI */
#endif

#ifdef _MSC_VER
#include <direct.h>
#define getcwd _getcwd
#endif

#define DATA(self) ((IoFileData *)self->data)

IoTag *IoFile_tag(void *state)
{
  IoTag *tag = IoTag_newWithName_("File");
  tag->state = state;
  tag->cloneFunc = (TagCloneFunc *)IoFile_rawClone;
  tag->markFunc  = (TagMarkFunc *)IoFile_mark;
  tag->freeFunc  = (TagFreeFunc *)IoFile_free;
  return tag;
}

IoFile *IoFile_proto(void *state)
{
  IoObject *self = IoObject_new(state);
  self->tag = IoFile_tag(state);

  self->data = calloc(1, sizeof(IoFileData));
  DATA(self)->path = IOSTRING("");
  DATA(self)->mode = IOSTRING("r+b"); 
  IoState_registerProtoWithFunc_(state, self, IoFile_proto);
    
  /* standard I/O */
  IoObject_addMethod_(self, IOSTRING("standardInput"), IoFile_standardInput);
  IoObject_addMethod_(self, IOSTRING("standardOutput"), IoFile_standardOutput);
  
  /* path */
  IoObject_addMethod_(self, IOSTRING("setPath"), IoFile_path_);
  IoObject_addMethod_(self, IOSTRING("path"), IoFile_path);
  IoObject_addMethod_(self, IOSTRING("name"), IoFile_lastPathComponent);
  IoObject_addMethod_(self, IOSTRING("useTemporaryPath"), IoFile_useTemporaryPath);

  /* info */
  IoObject_addMethod_(self, IOSTRING("exists"), IoFile_exists);
  IoObject_addMethod_(self, IOSTRING("size"), IoFile_size);
  
  /* open and close */
  IoObject_addMethod_(self, IOSTRING("openForReading"), IoFile_openForReading);
  IoObject_addMethod_(self, IOSTRING("openForUpdating"), IoFile_openForUpdating);
  IoObject_addMethod_(self, IOSTRING("openForAppending"), IoFile_openForAppending);

  IoObject_addMethod_(self, IOSTRING("open"), IoFile_open);
  IoObject_addMethod_(self, IOSTRING("close"), IoFile_close);
  
  /* reading */
  IoObject_addMethod_(self, IOSTRING("asString"), IoFile_asString);
  IoObject_addMethod_(self, IOSTRING("asBuffer"), IoFile_asBuffer);
  IoObject_addMethod_(self, IOSTRING("readLine"), IoFile_readLine);
  IoObject_addMethod_(self, IOSTRING("readLines"), IoFile_readLines);
  IoObject_addMethod_(self, IOSTRING("readStringOfLength"), IoFile_readStringOfLength_);
  IoObject_addMethod_(self, IOSTRING("at"), IoFile_at);
  IoObject_addMethod_(self, IOSTRING("foreach"), IoFile_foreach);
   
  /* writing */
  IoObject_addMethod_(self, IOSTRING("write"), IoFile_write);
  IoObject_addMethod_(self, IOSTRING("atPut"), IoFile_atPut);
  IoObject_addMethod_(self, IOSTRING("flush"), IoFile_flush);

  /* positioning */
  IoObject_addMethod_(self, IOSTRING("rewind"), IoFile_rewind);
  IoObject_addMethod_(self, IOSTRING("setPosition"), IoFile_position_);
  IoObject_addMethod_(self, IOSTRING("position"), IoFile_position);
  
  /* other */
  IoObject_addMethod_(self, IOSTRING("remove"), IoFile_remove);
  IoObject_addMethod_(self, IOSTRING("moveTo"), IoFile_moveTo_);

  /*
  This is too dangerous since it sets a process level variable which can
  potentially affect other IoStates in the same process
  IoObject_addMethod_(self, IOSTRING("setCurrentWorkingDirectory", IoFile_setCurrentWorkingDirectory_);
  */
  IoObject_addMethod_(self, IOSTRING("currentWorkingDirectory"), IoFile_currentWorkingDirectory);

  return self;
}

IoFile *IoFile_rawClone(IoFile *self) 
{ 
  IoObject *child = IoObject_rawClonePrimitive(self);
  child->data = cpalloc(self->data, sizeof(IoFileData));
  return child; 
}

IoFile *IoFile_new(void *state)
{
  IoObject *proto = IoState_protoWithInitFunction_(state, IoFile_proto);
  return IoFile_rawClone(proto);
}

IoFile *IoFile_newWithPath_(void *state, IoString *path)
{
  IoFile *self = IoFile_new(state);
  DATA(self)->path = path;
  return self;
}

void IoFile_mark(IoFile *self)
{ 
  if (DATA(self)->path) { IoObject_makeGrayIfWhite(DATA(self)->path); }
  if (DATA(self)->mode) { IoObject_makeGrayIfWhite(DATA(self)->mode); }
}

void IoFile_free(IoFile *self) 
{ 
  IoFile_justClose(self);
  if (DATA(self)->info) free(DATA(self)->info);
  free(self->data);
}

void IoFile_justClose(IoFile *self)
{
  if (DATA(self)->stream) 
  { 
    fclose(DATA(self)->stream); 
    DATA(self)->stream = (FILE *)NULL;
  }
}

int IoFile_justExists(IoFile *self)
{
  FILE *fp = fopen(CSTRING(DATA(self)->path), "rb");
  if (fp) { fclose(fp); return 1; }
  return 0;
}

int IoFile_create(IoFile *self)
{
  FILE *fp = fopen(CSTRING(DATA(self)->path), "wb");
  if (fp) { fclose(fp); return 1; }
  return 0;
}

/* ----------------------------------------------------------- */

IoObject *IoFile_standardInput(IoFile *self, IoObject *locals, IoMessage *m)
{
  IoFile *newFile = IoFile_new(IOSTATE);
  DATA(newFile)->path = IOREF(IOSTRING("<standard input>"));
  DATA(newFile)->mode = IOREF(IOSTRING("rb"));
  DATA(newFile)->stream = stdin;
  return newFile;
}

IoObject *IoFile_standardOutput(IoFile *self, IoObject *locals, IoMessage *m)
{
  IoFile *newFile = IoFile_new(IOSTATE);
  DATA(newFile)->path = IOREF(IOSTRING("<standard output>"));
  DATA(newFile)->mode = IOREF(IOSTRING("wb"));
  DATA(newFile)->stream = stdout;
  return newFile;
}


IoObject *IoFile_path_(IoFile *self, IoObject *locals, IoMessage *m)
{
  DATA(self)->path = IOREF(IoMessage_locals_stringArgAt_(m, locals, 0));
  return self;
}

IoObject *IoFile_path(IoFile *self, IoObject *locals, IoMessage *m)
{ return DATA(self)->path; }

IoObject *IoFile_lastPathComponent(IoFile *self, IoObject *locals, IoMessage *m)
{ return IoString_lastPathComponent(DATA(self)->path, locals, m); }

IoObject *IoFile_mode(IoFile *self, IoObject *locals, IoMessage *m)
{ 
  char *mode = IoString_asCString(DATA(self)->mode);
  if (!strcmp(mode, "rb"))  { return IOSTRING("read"); }
  if (!strcmp(mode, "r+b")) { return IOSTRING("update"); }
  if (!strcmp(mode, "a+b")) { return IOSTRING("append"); }
  return IONIL(self); 
}

IoObject *IoFile_useTemporaryPath(IoFile *self, IoObject *locals, IoMessage *m)
{
  char s[TMP_MAX];
  #ifdef _UNISTD_H_
    strcpy(s, "Io_XXXXXX");
    mkstemp(s);
  #else 
    tmpnam(s);
  #endif
  DATA(self)->path = IOSTRING(s);
  return self;
}

IoObject *IoFile_openForReading(IoFile *self, IoObject *locals, IoMessage *m)
{
  DATA(self)->mode = IOREF(IOSTRING("rb"));
  return IoFile_open(self, locals, m);
}

IoObject *IoFile_openForUpdating(IoFile *self, IoObject *locals, IoMessage *m)
{
  DATA(self)->mode = IOREF(IOSTRING("r+b"));
  return IoFile_open(self, locals, m);
}

IoObject *IoFile_openForAppending(IoFile *self, IoObject *locals, IoMessage *m)
{
  DATA(self)->mode = IOREF(IOSTRING("a+b"));
  return IoFile_open(self, locals, m);
}


IoObject *IoFile_open(IoFile *self, IoObject *locals, IoMessage *m)
{
  char *mode = CSTRING(DATA(self)->mode);
  if (IoMessage_argCount(m) > 0)
  { DATA(self)->path = IOREF(IoMessage_locals_stringArgAt_(m, locals, 0)); }

  if (DATA(self)->stream) { IoFile_justClose(self); }
  
  if (!IoFile_justExists(self) && strcmp(mode, "rb") != 0 )
  { IoFile_create(self); }
  
  DATA(self)->stream = fopen(CSTRING(DATA(self)->path), mode);
  if (DATA(self)->stream == NULL)
  {
    IoState_error_description_(IOSTATE, m, "File.open", 
      "error opening file path '%s'", CSTRING(DATA(self)->path));
  }
  return self;
}

IoObject *IoFile_close(IoFile *self, IoObject *locals, IoMessage *m)
{ IoFile_justClose(self); return self; }

IoObject *IoFile_flush(IoFile *self, IoObject *locals, IoMessage *m)
{ fflush(DATA(self)->stream); return self; }

IoObject *IoFile_rawAsString(IoFile *self)
{
  ByteArray *ba = ByteArray_new();
  
  if (ByteArray_readFromFilePath_(ba, CSTRING(DATA(self)->path)) == 0)
  { return IoString_newWithByteArray_copy_(IOSTATE, ba, 0); }
  else
  {
    ByteArray_free(ba);
    IoState_error_description_(IOSTATE, 0x0, "File.asString", 
      "unable to read file '%s'", CSTRING(DATA(self)->path));
  }
  return IONIL(self);
}

IoObject *IoFile_asString(IoFile *self, IoObject *locals, IoMessage *m)
{
  ByteArray *ba = ByteArray_new();
  char *path = CSTRING(DATA(self)->path);
  FILE *fp;
  if (DATA(self)->stream == stdin) 
  { fp = DATA(self)->stream; }
  else 
  { fp = fopen(path, "rb"); }
  if (ByteArray_readFromCStream_(ba, fp) != -1)
  { return IoString_newWithByteArray_copy_(IOSTATE, ba, 0); }
  else
  {
    ByteArray_free(ba);
    IoState_error_description_(IOSTATE, m, "File.asString", 
      "unable to read file '%s'", CSTRING(DATA(self)->path));
  }
  return IONIL(self);
}

IoObject *IoFile_asBuffer(IoFile *self, IoObject *locals, IoMessage *m)
{
  ByteArray *ba = ByteArray_new();
  if (ByteArray_readFromFilePath_(ba, CSTRING(DATA(self)->path)) == 0)
  { return IoBuffer_newWithByteArray_copy_(IOSTATE, ba, 0); }
  else
  {
    ByteArray_free(ba);
    IoState_error_description_(IOSTATE, m, "File.asBuffer", 
      "unable to read file '%s'", CSTRING(DATA(self)->path));
  }
  return IONIL(self);
}

IoObject *IoFile_exists(IoFile *self, IoObject *locals, IoMessage *m)
{ return (IoFile_justExists(self)) ? (IoObject *)self : IONIL(self); }



IoObject *IoFile_remove(IoFile *self, IoObject *locals, IoMessage *m)
{
#if !defined(__SYMBIAN32__)
  int error = remove(CSTRING(DATA(self)->path));
#else
  int error = -1;
#endif
  if (error && IoFile_justExists(self))
  {
    IoState_error_description_(IOSTATE, m, "File.remove", 
      "error removing file '%s'", CSTRING(DATA(self)->path));
  }
  return self;
}

IoObject *IoFile_moveTo_(IoFile *self, IoObject *locals, IoMessage *m)
{
  IoString *newPath = IoMessage_locals_stringArgAt_(m, locals, 0);
  int error = rename(CSTRING(DATA(self)->path), CSTRING(newPath));
  if (error)
  {
      IoState_error_description_(IOSTATE, m, "File.moveTo", 
        "error moving file '%s' to '%s'", CSTRING(DATA(self)->path), CSTRING(newPath));
  }
  return self;
}

IoObject *IoFile_write(IoFile *self, IoObject *locals, IoMessage *m)
{
  int i;
  IoFile_assertOpen(self, locals, m);
  for (i=0; i < IoMessage_argCount(m); i++)
  {
    IoString *string = IoMessage_locals_stringOrBufferArgAt_(m, locals, i);
    ByteArray_writeToCStream_(IoString_rawByteArray(string), DATA(self)->stream);
    if (ferror(DATA(self)->stream) != 0)
    {
      IoState_error_description_(IOSTATE, m, "File.write", 
        "error writing to file '%s'", CSTRING(DATA(self)->path));
    }
  }
  return self;
}

IoObject *IoFile_readLines(IoFile *self, IoObject *locals, IoMessage *m)
{
  IoFile_assertOpen(self, locals, m);
  {
  IoList *lines = IoList_new(IOSTATE);
  IoObject *newLine = IoFile_readLine(self, locals, m);
  while (!ISNIL(newLine))
  {
    IoList_rawAdd_(lines, newLine);
    newLine = IoFile_readLine(self, locals, m);
  }
  return lines;
  }
}

IoObject *IoFile_readLine(IoFile *self, IoObject *locals, IoMessage *m)
{
  IoFile_assertOpen(self, locals, m);
  if (feof(DATA(self)->stream) != 0) return IONIL(self);
  {
    ByteArray *ba = ByteArray_new();
    int error;
    unsigned char didRead = ByteArray_readLineFromCStream_(ba, DATA(self)->stream);
    if (!didRead) { ByteArray_free(ba); return IONIL(self); }
    error = ferror(DATA(self)->stream);
    if (error != 0)
    {
      ByteArray_free(ba);
      clearerr(DATA(self)->stream);
      IoState_error_description_(IOSTATE, m, "File.readLine", 
        "error reading from file '%s'", CSTRING(DATA(self)->path));
    }
    return IoState_stringWithByteArray_copy_(IOSTATE, ba, 0);
  }
}

IoObject *IoFile_readStringOfLength_(IoFile *self, IoObject *locals, IoMessage *m)
{
  int length = IoMessage_locals_intArgAt_(m, locals, 0);
  ByteArray *ba = ByteArray_new();
  IoFile_assertOpen(self, locals, m);

  ByteArray_readNumberOfBytes_fromCStream_(ba, length, DATA(self)->stream);
  if (ferror(DATA(self)->stream) != 0)
  {
    clearerr(DATA(self)->stream);
    ByteArray_free(ba);
    IoState_error_description_(IOSTATE, m, "File.readStringOfLength", 
      "error reading file '%s'", CSTRING(DATA(self)->path));
  }
  if (!ByteArray_length(ba))
  {
    ByteArray_free(ba);
    return IONIL(self);
  }
  return IoString_newWithByteArray_copy_(IOSTATE, ba, 0);
}

IoObject *IoFile_rewind(IoFile *self, IoObject *locals, IoMessage *m)
{ if (DATA(self)->stream) rewind(DATA(self)->stream); return self; }

IoObject *IoFile_position_(IoFile *self, IoObject *locals, IoMessage *m)
{
  long pos = IoMessage_locals_longArgAt_(m, locals, 0);
  IoFile_assertOpen(self, locals, m);
  if (fseek(DATA(self)->stream, pos, 0) != 0)
  {
    IoState_error_description_(IOSTATE, m, "File.position_", 
      "unable to set position %i file path '%s'", 
      (int)pos, CSTRING(DATA(self)->path));
  }
  return self;
}

IoObject *IoFile_position(IoFile *self, IoObject *locals, IoMessage *m)
{
  IoFile_assertOpen(self, locals, m);
  return IoNumber_newWithDouble_(IOSTATE, ftell(DATA(self)->stream));
}

IoObject *IoFile_size(IoFile *self, IoObject *locals, IoMessage *m)
{
  FILE *fp = fopen(CSTRING(DATA(self)->path), "rb");
  if (fp)
  {
    int fileSize;
    fseek(fp, 0, SEEK_END);
    fileSize = ftell(fp);
    fclose(fp);
    return IoNumber_newWithDouble_(IOSTATE, fileSize);
  }
  else
  {
    IoState_error_description_(IOSTATE, m, "File.size", 
        "unable to open file '%s'", CSTRING(DATA(self)->path));
  }
  return IONIL(self);
}

IoObject *IoFile_isOpen(IoFile *self, IoObject *locals, IoMessage *m)
{ return (DATA(self)->stream) ? (IoObject *)self : IONIL(self); }

IoObject *IoFile_assertOpen(IoFile *self, IoObject *locals, IoMessage *m)
{
  if (!DATA(self)->stream)
  {
    IoState_error_description_(IOSTATE, m, "File.notOpen", 
      "file '%s' not yet open", CSTRING(DATA(self)->path));
  }
  return self;
}

IoObject *IoFile_at(IoFile *self, IoObject *locals, IoMessage *m)
{
  int byte;
  IoFile_position_(self, locals, m); /* works since first arg is the same */
  byte = fgetc(DATA(self)->stream);
  if (byte == EOF) return IONIL(self); 
  return IONUMBER(byte);
}

IoObject *IoFile_atPut(IoFile *self, IoObject *locals, IoMessage *m)
{
  int c = IoMessage_locals_intArgAt_(m, locals, 1);
  IoFile_position_(self, locals, m); /* works since first arg is the same */
  if (fputc(c, DATA(self)->stream) == EOF)
  {
    int pos = IoMessage_locals_intArgAt_(m, locals, 0); /* BUG - this may not be the same when evaled */
    IoState_error_description_(IOSTATE, m, "File.atPut", 
      "error writing to position %i in file '%s'", pos, CSTRING(DATA(self)->path));  
  }
  return self;
}

IoObject *IoFile_foreach(IoFile *self, IoObject *locals, IoMessage *m)
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

    int i = 0;
    if (!index || !ISNUMBER(index))
    {
      IoObject_setSlot_to_(locals, indexSlotName, IONUMBER(0));
      index = IoObject_getSlot_(locals, indexSlotName); /* numbers  copied by value! */
    }
    if (!character || !ISNUMBER(character))
    {
      IoObject_setSlot_to_(locals, characterSlotName, IONUMBER(0));
      character = IoObject_getSlot_(locals, characterSlotName); /* numbers copied by value! */
    }
    for (;;) 
    {
      int c = getc(DATA(self)->stream);
      if (c == EOF) break;
      IoNumber_rawSet(character, (double)c);
      IoNumber_rawSet(index, (double)i);
      result = IoMessage_locals_performOn_(doMessage, locals, locals);
      if (IoMessage_isBrokenOrReturned(doMessage)) 
      { 
        if (IoMessage_isBroken(doMessage)) IoMessage_resetStopStatus(m); 
        return result;
      }
      i++;
    } 
    return result;
  }
}

IoObject *IoFile_rawDo(IoFile *self, IoObject *context)
{ return IoObject_rawDoString_label_(context, (IoString *)IoFile_rawAsString(self), DATA(self)->path); }

IoObject *IoFile_do(IoFile *self, IoObject *locals, IoMessage *m)
{
  IoList *argsList = IoList_new(IOSTATE);
  IoObject *context = IoMessage_locals_valueArgAt_(m, locals, 0);
  int argn = 1;
  IoObject *arg = IoMessage_locals_valueArgAt_(m, locals, argn);
  while (arg && !ISNIL(arg))
  {
    IoList_rawAdd_(argsList, arg);
    argn++;
    arg = IoMessage_locals_valueArgAt_(m, locals, argn);
  }
  if (IoList_rawCount(argsList))
  { IoObject_setSlot_to_(context, IOSTRING("args"), argsList); }
  return IoObject_rawDoString_label_(context, (IoString *)IoFile_rawAsString(self), DATA(self)->path);
}

ByteArray *IoFile_CurrentWorkingDirectoryAsByteArray(void)
{
  char *buf = getcwd(NULL, 0);
  return ByteArray_newWithData_size_copy_(buf, strlen(buf), 0);
}

IoObject *IoFile_currentWorkingDirectory(IoFile *self, IoObject *locals, IoMessage *m)
{
  char *buf = getcwd(NULL, 0);
  IoString *s = IOSTRING(buf);
  free(buf);
  return s;
}

/*
int IoFile_SetCurrentWorkingDirectory_(char *p)
{ return chdir(p); }

IoObject *IoFile_setCurrentWorkingDirectory_(IoFile *self, IoObject *locals, IoMessage *m)
{
  IoString *path = IoMessage_locals_stringArgAt_(m, locals, 0);
  if(chdir(CSTRING(path)) == -1) return IONIL(self);
  return self;
}
*/

