/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#ifndef IOFILE_DEFINED
#define IOFILE_DEFINED 1

#include "base/Common.h"
#include "IoObject.h"
#include "base/ByteArray.h"
#include "IoNumber.h"
#include "IoString.h"

#define ISFILE(self) \
  (self->tag->cloneFunc == (TagCloneFunc *)IoFile_rawClone)

typedef IoObject IoFile;

typedef struct
{
  FILE *stream;
  IoString *path;
  IoString *mode;
  unsigned char autoYield; /* not used yet */
  void *info; /* reserved for use in OS specific extensions */
} IoFileData;

IoFile *IoFile_proto(void *state);
IoFile *IoFile_rawClone(IoFile *self);
IoFile *IoFile_new(void *state);
IoFile *IoFile_newWithPath_(void *state, IoString *path);
void IoFile_free(IoFile *self);
void IoFile_mark(IoFile *self);

void IoFile_justClose(IoFile *self);
int IoFile_justExists(IoFile *self);
int IoFile_create(IoFile *self);

/* ----------------------------------------------------------- */
IoObject *IoFile_clone(IoFile *self, IoObject *locals, IoMessage *m);

IoObject *IoFile_standardInput(IoFile *self, IoObject *locals, IoMessage *m);
IoObject *IoFile_standardOutput(IoFile *self, IoObject *locals, IoMessage *m);

IoObject *IoFile_path_(IoFile *self, IoObject *locals, IoMessage *m);
IoObject *IoFile_path(IoFile *self, IoObject *locals, IoMessage *m);
IoObject *IoFile_lastPathComponent(IoFile *self, IoObject *locals, IoMessage *m);

IoObject *IoFile_mode(IoFile *self, IoObject *locals, IoMessage *m);

IoObject *IoFile_useTemporaryPath(IoFile *self, IoObject *locals, IoMessage *m);

IoObject *IoFile_openForReading(IoFile *self, IoObject *locals, IoMessage *m);
IoObject *IoFile_openForUpdating(IoFile *self, IoObject *locals, IoMessage *m);
IoObject *IoFile_openForAppending(IoFile *self, IoObject *locals, IoMessage *m);
IoObject *IoFile_open(IoFile *self, IoObject *locals, IoMessage *m);
IoObject *IoFile_close(IoFile *self, IoObject *locals, IoMessage *m);

IoObject *IoFile_flush(IoFile *self, IoObject *locals, IoMessage *m);
IoObject *IoFile_rawAsString(IoFile *self);
IoObject *IoFile_asString(IoFile *self, IoObject *locals, IoMessage *m);
IoObject *IoFile_asBuffer(IoFile *self, IoObject *locals, IoMessage *m);
IoObject *IoFile_exists(IoFile *self, IoObject *locals, IoMessage *m);
IoObject *IoFile_remove(IoFile *self, IoObject *locals, IoMessage *m);
IoObject *IoFile_moveTo_(IoFile *self, IoObject *locals, IoMessage *m);
IoObject *IoFile_write(IoFile *self, IoObject *locals, IoMessage *m);
IoObject *IoFile_readLine(IoFile *self, IoObject *locals, IoMessage *m);
IoObject *IoFile_readLines(IoFile *self, IoObject *locals, IoMessage *m);
IoObject *IoFile_readStringOfLength_(IoFile *self, IoObject *locals, IoMessage *m);
IoObject *IoFile_rewind(IoFile *self, IoObject *locals, IoMessage *m);
IoObject *IoFile_position_(IoFile *self, IoObject *locals, IoMessage *m);
IoObject *IoFile_position(IoFile *self, IoObject *locals, IoMessage *m);
IoObject *IoFile_size(IoFile *self, IoObject *locals, IoMessage *m);
IoObject *IoFile_isOpen(IoFile *self, IoObject *locals, IoMessage *m);
IoObject *IoFile_assertOpen(IoFile *self, IoObject *locals, IoMessage *m);

IoObject *IoFile_at(IoFile *self, IoObject *locals, IoMessage *m);
IoObject *IoFile_atPut(IoFile *self, IoObject *locals, IoMessage *m);
IoObject *IoFile_foreach(IoFile *self, IoObject *locals, IoMessage *m);

IoObject *IoFile_rawDo(IoFile *self, IoObject *context);
IoObject *IoFile_do(IoFile *self, IoObject *locals, IoMessage *m);

ByteArray *IoFile_CurrentWorkingDirectoryAsByteArray(void); /* caller must free returned ByteArray */
IoObject *IoFile_currentWorkingDirectory(IoFile *self, IoObject *locals, IoMessage *m);
/*IoObject *IoFile_setCurrentWorkingDirectory_(IoFile *self, IoObject *locals, IoMessage *m);*/

#endif
