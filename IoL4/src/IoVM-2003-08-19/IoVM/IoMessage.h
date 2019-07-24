/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#ifndef IOMESSAGE_DEFINED
#define IOMESSAGE_DEFINED 1

#include "base/Common.h"
#include "base/List.h"
#include "base/ByteArray.h"
#include "IoTag.h"
#include "IoMark.h"
#include "IoObject_struct.h"

#define MESSAGE_STOP_STATUS_NORMAL   0
#define MESSAGE_STOP_STATUS_BREAK    1
#define MESSAGE_STOP_STATUS_CONTINUE 2
#define MESSAGE_STOP_STATUS_RETURN   3

#define ISMESSAGE(self) (self->tag->cloneFunc == (TagCloneFunc *)IoMessage_rawClone)

#define IoMessage_end(self) IOSTATE->stopStatus = MESSAGE_STOP_STATUS_NORMAL;

#if !defined(IoString_DEFINED) 
  #define IoString_DEFINED
  typedef IoObject IoString;
#endif

typedef IoObject IoMessage;

typedef struct
{
  IoString *method; /* literal IoString */
  List *args;
  IoMessage *attachedMessage;
  IoMessage *nextMessage;
  IoObject *cachedResult;
  int charNumber;
  int lineNumber;
  IoString *label; /* literal IoString */
} IoMessageData;


/* --- basics -------------------------------------------------- */
IoMessage *IoMessage_proto(void *state);
IoMessage *IoMessage_rawClone(IoMessage *m);
IoMessage *IoMessage_new(void *state);
IoMessage *IoMessage_deepCopyOf_(IoMessage *m);
IoMessage *IoMessage_newWithName_(void *state, IoString *symbol);
IoMessage *IoMessage_newWithName_label_(void *state, IoString *symbol, IoString *label);
IoMessage *IoMessage_newWithName_returnsValue_(void *state, IoString *symbol, IoObject *v);
/*IoMessage *IoMessage_newWithName_andCachedArgs_(void *state, void *methodName, ...);*/

void IoMessage_mark(IoMessage *self);
void IoMessage_free(IoMessage *self);

void IoMessage_label_(IoMessage *self, IoString *ioString); /* sets label for children too */
int IoMessage_rawLineNumber(IoMessage *self);
int IoMessage_rawCharNumber(IoMessage *self);
List *IoMessage_rawArgList(IoMessage *self);
unsigned char IoMessage_needsEvaluation(IoMessage *self);


void IoMessage_addCachedArg_(IoMessage *self, IoObject *v);
void IoMessage_setCachedArg_to_(IoMessage *self, int n, IoObject *v);
void IoMessage_setCachedArg_toInt_(IoMessage *self, int n, int anInt);
void IoMessage_cachedResult_(IoMessage *self, IoObject *v);

/* --- perform ------------------------------------------------ */
IoObject *IoMessage_doInContext(IoMessage *self, IoObject *locals, IoMessage *m);
IoObject *IoMessage_locals_performOn_(IoMessage *self, IoObject *locals, IoObject *target);

/* --- args ------------------------------------------------ */
IoObject *IoMessage_locals_valueArgAt_(IoMessage *self, IoObject *locals, int n);
void IoMessage_assertArgCount_receiver_(IoMessage *self, int n, IoObject *receiver);
int IoMessage_argCount(IoMessage *self);
void IoMessage_locals_numberArgAt_errorForType_(
  IoMessage *self, IoObject *locals, int n, const char *typeName);
IoObject *IoMessage_locals_numberArgAt_(IoMessage *self, IoObject *locals, int n);
int IoMessage_locals_intArgAt_(IoMessage *self, IoObject *locals, int n);
long IoMessage_locals_longArgAt_(IoMessage *self, IoObject *locals, int n);
double IoMessage_locals_doubleArgAt_(IoMessage *self, IoObject *locals, int n);

IoObject *IoMessage_locals_stringArgAt_(IoMessage *self, IoObject *locals, int n);
IoObject *IoMessage_locals_stringOrBufferArgAt_(IoMessage *self, IoObject *locals, int n);
IoObject *IoMessage_locals_bufferArgAt_(IoMessage *self, IoObject *locals, int n);
IoObject *IoMessage_locals_blockArgAt_(IoMessage *self, IoObject *locals, int n);
IoObject *IoMessage_locals_dateArgAt_(IoMessage *self, IoObject *locals, int n);
IoObject *IoMessage_locals_mapArgAt_(IoMessage *self, IoObject *locals, int n);
IoObject *IoMessage_locals_messageArgAt_(IoMessage *self, IoObject *locals, int n);
IoObject *IoMessage_locals_listArgAt_(IoMessage *self, IoObject *locals, int n);
IoObject *IoMessage_locals_objectArgAt_(IoMessage *self, IoObject *locals, int n);

/* --- printing ----------------------------------------------- */
void IoMessage_print(IoMessage *self);
void IoMessage_printWithReturn(IoMessage *self);
ByteArray *IoMessage_description(IoMessage *self);
ByteArray *IoMessage_descriptionJustSelfAndArgs(IoMessage *self); /* caller must free returned */
void IoMessage_appendDescriptionTo_follow_(IoMessage *self, ByteArray *ba, int follow);  /* caller must free returned */

/* --- flow control ------------------------------------------- */
void IoMessage_break(IoMessage *self, IoObject *v);
void IoMessage_continue(IoMessage *self);
void IoMessage_return_(IoMessage *self, IoObject *v);
int IoMessage_isStopped(IoMessage *self);
int IoMessage_isBrokenOrReturned(IoMessage *self);
int IoMessage_isBroken(IoMessage *self);
void IoMessage_resetStopStatus(IoMessage *self);
int IoMessage_stopStatus(IoMessage *self);

/* --- primitive methods -------------------------------------- */
IoObject *IoMessage_clone(IoMessage *self, IoObject *locals, IoMessage *m);
IoObject *IoMessage_protoName(IoMessage *self, IoObject *locals, IoMessage *m);
IoObject *IoMessage_protoName_(IoMessage *self, IoObject *locals, IoMessage *m);
IoObject *IoMessage_descriptionString(IoMessage *self, IoObject *locals, IoMessage *m);
IoObject *IoMessage_nextMessage(IoMessage *self, IoObject *locals, IoMessage *m);
IoObject *IoMessage_nextMessage_(IoMessage *self, IoObject *locals, IoMessage *m);
IoObject *IoMessage_attachedMessage(IoMessage *self, IoObject *locals, IoMessage *m);
IoObject *IoMessage_attachedMessage_(IoMessage *self, IoObject *locals, IoMessage *m);
IoObject *IoMessage_argAt(IoMessage *self, IoObject *locals, IoMessage *m);
IoObject *IoMessage_arguments(IoMessage *self, IoObject *locals, IoMessage *m);
IoObject *IoMessage_arguments_(IoMessage *self, IoObject *locals, IoMessage *m);
IoObject *IoMessage_cachedResult(IoMessage *self, IoObject *locals, IoMessage *m);
IoObject *IoMessage_setCachedResult(IoMessage *self, IoObject *locals, IoMessage *m);

IoObject *IoMessage_fromString(IoMessage *self, IoObject *locals, IoMessage *m);
IoObject *IoMessage_argsEvaluatedIn(IoMessage *self, IoObject *locals, IoMessage *m);


/* ------------------------------ */
/*#include "IoMessage_inline.h"*/

IoString *IoMessage_rawMethod(IoMessage *self);
/*#define IoMessage_rawMethod(self)  (((IoMessageData *)self->data)->method);*/
void IoMessage_addArg_(IoMessage *self, IoMessage *m);
IoMessage *IoMessage_rawArgAt_(IoMessage *self, int n);
IoString *IoMessage_rawLabel(IoMessage *self);
List *IoMessage_rawArgs(IoMessage *self);

#endif
