/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#ifndef IOBLOCK_DEFINED
#define IOBLOCK_DEFINED 1

#include "base/Common.h"
#include "base/List.h"
#include "IoObject.h"
#include "IoState.h"

#define ISBLOCK(self) (self->tag->cloneFunc == (TagCloneFunc *)IoBlock_rawClone)

typedef IoObject IoBlock;

typedef struct
{
  IoMessage *message;
  List *argNames;
  IoObject *scope; /* if 0x0, then use target as the locals proto */
} IoBlockData;

IoBlock *IoBlock_proto(void *state);
IoBlock *IoBlock_rawClone(IoBlock *self);
IoBlock *IoBlock_new(IoState *state);
void IoBlock_mark(IoBlock *self);
void IoBlock_free(IoBlock *self);
void IoBlock_message_(IoBlock *self, IoMessage *m);
IoBlock *IoBlock_newFromCString_(IoState *state, char *s);

void IoBlock_writeToStore_(IoBlock *self, IoStoreHandle *sh);
void *IoBlock_readFromStore_(IoBlock *self, IoStoreHandle *sh);

/* --- calling ----------------------------------------------- */
IoObject *IoBlock_target_locals_call_(IoBlock *self, IoObject *target, IoObject *locals, IoMessage *m);
IoObject *IoBlock_print(IoBlock *self, IoObject *locals, IoMessage *m);
ByteArray *IoBlock_justCode(IoBlock *self);
IoObject *IoBlock_code(IoBlock *self, IoObject *locals, IoMessage *m);
IoObject *IoBlock_code_(IoBlock *self, IoObject *locals, IoMessage *m);
IoObject *IoBlock_message(IoBlock *self, IoObject *locals, IoMessage *m);
IoObject *IoBlock_setMessage(IoBlock *self, IoObject *locals, IoMessage *m);
IoObject *IoBlock_argumentNames(IoBlock *self, IoObject *locals, IoMessage *m);
IoObject *IoBlock_argumentNames_(IoBlock *self, IoObject *locals, IoMessage *m);
IoObject *IoBlock_setScope_(IoBlock *self, IoObject *locals, IoMessage *m);
IoObject *IoBlock_scope(IoBlock *self, IoObject *locals, IoMessage *m);

/* --- lobby methods ---------------------------------------- */
IoObject *IoBlock_lobbyMethod(IoObject *target, IoObject *locals, IoMessage *m);
IoObject *IoBlock_lobbyBlock(IoObject *target, IoObject *locals, IoMessage *m);

/* --- jit -------------------------------------------------- */
IoObject *IoBlock_jitCompile(IoBlock *self, IoObject *locals, IoMessage *m);
void IoBlock_compile(IoBlock *self);

IoObject *IoBlock_ifTrue_(IoBlock *self, IoObject *locals, IoMessage *m);
IoObject *IoBlock_ifFalse_(IoBlock *self, IoObject *locals, IoMessage *m);
IoObject *IoBlock_whileTrue_(IoBlock *self, IoObject *locals, IoMessage *m);
IoObject *IoBlock_whileFalse_(IoBlock *self, IoObject *locals, IoMessage *m);

#endif
