/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#ifndef IOSTATE_DEFINED
#define IOSTATE_DEFINED 1

#include "base/Common.h"
#include "base/Queue.h"
#include "base/SplayTree.h"
#include "base/Stack.h"
#include "base/Hash.h"
#include "base/MainArgs.h"
#include "base/Scheduler.h"
#include "IoObject.h"
#include "IoString.h"
#include "IoExceptionCatch.h"
#include "IoObjectGroup.h"
#include "IoVersion.h"

#define SIOSTRING(stringConstant) IoState_stringWithCString_(self, stringConstant)

#define IOASSERT(value, message) \
  if (!(value)) \
  { IoState_error_description_(IOSTATE, m, "Io.Assertion", message); }

typedef void (StatePrintCallback)(void *, char *);
typedef void (StateErrorCallback)(void *, char *, char *);
typedef void (StateExitCallback)(void *);
typedef IoObject *(StateProtoFunc)(void *);
typedef IoObject *(StateCatchWithinCallback)(void *, char *);

#define IOSTATE ((IoState *)(self->tag->state))

typedef struct
{
  MainArgs *mainArgs;
  PHash *primitiveProtos;

  /* -- gc --- */
  IoObject *lobby;
  List *retainedValues;
   
  IoObjectGroup *blackGroup;
  IoObjectGroup *grayGroup;
  IoObjectGroup *whiteGroup;
  IoObjectGroup *freeGroup;
  
  int newValueCount;           /* count of values added since last mark phase */
  int newValueCountToUpdate;   /* do a mark when reached */
  int newValueSinceSweepCount; /* count of values added since last sweep phase */
  int newValueCountToSweep;    /* do a sweep when reached */

  /* -- literals --- */
  SplayTree *literalStrings;
  
  /* -- actors --- */
  Scheduler *scheduler;
  List *actors; /* to prevent active actors from being collected */
  IoObject *mainActor;
  IoObject *currentActor; /* move this into Coro structure? */
  Stack *currentRetainStack;
  Stack *lookupStack;
  Queue *parentLookupQueue;
  
  /* --- current execution state --- */
  unsigned char stopStatus;
  void *returnValue;
  
  /* --- quick access strings --- */
  IoString *activateString;
  IoString *blockString;
  IoString *cloneString;
  IoString *debugString;
  IoString *debugCollectorString;
  IoString *debugParserString;
  /*IoString *delegateString;*/
  IoString *equalEqualString;
  IoString *forwardString;
  IoString *initString;
  IoString *messageString;
  IoString *objectString;
  IoString *parentString;
  IoString *protoString;
  IoString *returnString;
  IoObject *selfString;
  IoString *senderString;
  IoObject *setSlotBlock;
  IoObject *setSlotString;
  IoString *targetString;

  IoMessage *nilMessage;
  IoMessage *printMessage;
  IoMessage *forwardMessage;
  
  /* --- global objects --- */
  IoObject *ioNil;
  IoObject *ioNop; /* object that returns itself for any message */

  /* -- embedding --- */
  void *userData;
  StatePrintCallback *printCallback;
  StateErrorCallback *errorCallback;
  StateExitCallback  *exitCallback;

  /* -- weaklinks --- */
  Hash *valueToWeakLink;
  /*Stack *collectedWeakLinks;*/
  Stack *recycledObjects;
  Stack *recycledObjectsSansSlots;
} IoState;

/* --- setup ---*/
IoState *IoState_new(void);
IoObject *IoState_finishInit(IoState *self);

/* --- setup tags ---*/
void IoState_registerProtoWithFunc_(IoState *self, IoObject *proto, StateProtoFunc *func);
IoObject *IoState_protoWithInitFunction_(IoState *self, StateProtoFunc *func);
IoObject *IoState_protoWithName_(IoState *self, const char *name);

void IoState_free(IoState *self);
IoObject *IoState_lobby(IoState *self);

/* --- garbage collection ---*/
IoObject *IoState_retain_(IoState *self, IoObject *v);

int  IoState_garbageCollectorIsPaused(IoState *self);
void IoState_pauseGarbageCollector(IoState *self);
void IoState_resumeGarbageCollector(IoState *self);

void IoState_gcInitPhase(IoState *self);
int IoState_gcSweepPhase(IoState *self);

void IoState_gcMarkPhase(IoState *self);
IoObject *IoState_addValue_(IoState *self, IoObject *v);

void IoState_makeBlackFromGray_(IoState *self, IoObject *v);
void IoState_makeGrayFromWhite_(IoState *self, IoObject *v);
void IoState_makeGrayFromBlack_(IoState *self, IoObject *v);
void IoState_immediatelyFreeIfUnreferenced_(IoState *self, IoObject *v);

/* --- weak links -------------------------------- */
#ifdef IOWEAKLINK_DEFINED
void IoState_addWeakLink_(IoState *self, IoWeakLink *v);
void IoState_removeWeakLink_(IoState *self, IoWeakLink *v);
void IoState_willFreeWeakLinkedValue_(IoState *self, IoObject *v);
#endif
/* ----------------------------------------------- */

int IoState_collectGarbage(IoState *self);
void IoState_release_(IoState *self, IoObject *v);

/* --- stack retained values --------------------- */
size_t IoState_memorySize(IoState *self);
void IoState_compact(IoState *self);

/* --- literals ---------------------------------- */
void IoState_removeLiteralString_(IoState *self, IoString *aString);
IoString *IoState_stringWithByteArray_copy_(IoState *self, ByteArray *ba, int copy);
IoString *IoState_stringWithCString_(IoState *self, char *s);
IoString *IoState_stringWithCString_length_(IoState *self, char *s, int length);
IoString *IoState_addLiteralString_(IoState *self, IoString *s);

/* --- performing --------------------------------- */
IoObject *IoState_catchWithin_arg_(IoState *self, StateCatchWithinCallback *callback, void *context, void *s);
IoObject *IoState_doCString_(IoState *self, char *s);
IoObject *IoState_doFile_(IoState *self, char *fileName);

/* --- embedding callbacks ------------------------ */
void IoState_justPrint_(IoState *self, char *s);
void IoState_print_(IoState *self, const char *format, ...);
void IoState_printCallback_(IoState *self, StatePrintCallback* callback);
void IoState_errorCallback_(IoState *self, StateErrorCallback* callback);
void IoState_exitCallback_(IoState *self, StateExitCallback* callback);
void IoState_exit(IoState *self);
void IoState_callErrorCallback(IoState *self, IoExceptionCatch *eCatch);

/* --- exceptions --------------------------------- */
void IoState_syncRetainStack(IoState *self);
void IoState_error_description_(IoState *self, IoMessage *m, const char *name, const char *format, ...);
IoExceptionCatch *IoState_pushExceptionCatchWithName_(IoState *self, char *exceptionName);
void IoState_popExceptionCatch_(IoState *self, IoExceptionCatch *eCatch);
void IoState_raiseExceptionWithName_andDescription_(IoState *self, 
  IoString *exceptionName, IoString *description, IoMessage *m);

/* --- stack trace -------------------------------- */
void IoState_printStackTrace(IoState *self, IoExceptionCatch *eCatch);
void IoState_showStackEntry_(IoState *self, void *blockLocals);
void IoState_showStackEntryMessage_(IoState *self, IoMessage *m);

/* --- actors ------------------------------------- */
void IoState_addActor_(IoState *self, IoObject *actor);
void IoState_removeActor_(IoState *self, IoObject *actor);
IoObject *IoState_currentActor(IoState *actor);
void IoState_currentActor_(IoState *self, IoObject *actor);
void IoState_yield(IoState *self);

/* --- command line ------------------------------- */
void IoState_argc_argv_(IoState *st, int argc, const char *argv[]);
int IoState_main(IoState *st, int argc, const char *argv[]);
void IoState_mainRunFile_(IoState *self, char *s);
IoObject *IoState_runCLI(void *context, char *s);
void IoState_cliInput(IoState *st, char *s);

/* --- user data ---------------------------------- */
void IoState_userData_(IoState *st, void *userData);
void *IoState_userData(IoState *st);

#include "IoState_inline.h"

#endif
