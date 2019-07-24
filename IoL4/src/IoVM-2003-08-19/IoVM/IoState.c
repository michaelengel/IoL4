/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#define IOSTATE_C
#include "IoState.h"
#undef IOSTATE_C

#include "IoObject.h"
#include "IoObject_actor.h"
#include "IoNil.h"
#include "IoString.h"
#include "IoNumber.h"
#include "IoCFunction.h" 
#include "IoBlock.h"
#include "IoList.h"
#include "IoMap.h"
#ifndef IO_OS_L4
#include "IoFile.h"
#include "IoDate.h"
#endif
#include "IoDuration.h"
#include "IoBuffer.h"
#include "IoMessage_parser.h"
#include "base/CLI.h"
#include "IoWeakLink.h"
#include "IoFuture.h"
#include "IoLinkedList.h"

#include <time.h>
#include <limits.h> 
#include <stdlib.h>

#define STATE_DEFAULT_COLECT_AFTER 10000

IoState *IoState_new(void)
{
  IoString *stringProto;
  IoState *self = (IoState *)calloc(1, sizeof(IoState));
  printf("%p\n", self);
  self->mainArgs = MainArgs_new();
  self->primitiveProtos = PHash_new();
  self->valueToWeakLink = Hash_new();
  /*self->collectedWeakLinks = Stack_new();*/
  
  self->freeGroup  = IoObjectGroup_newWithColor_(self, IOMARK_INITIAL_FREE);
  self->blackGroup = IoObjectGroup_newWithColor_(self, IOMARK_INITIAL_BLACK);
  self->grayGroup  = IoObjectGroup_newWithColor_(self, IOMARK_INITIAL_GRAY);
  self->whiteGroup = IoObjectGroup_newWithColor_(self, IOMARK_INITIAL_WHITE);
  self->recycledObjects = Stack_new();
  self->recycledObjectsSansSlots = Stack_new();
  self->newValueCount = 0;
  IoState_pauseGarbageCollector(self);
  
  self->literalStrings = SplayTree_new();
  SplayTree_compareFunction_(self->literalStrings, ioStringCompareFunc);
  
  self->retainedValues = List_new();

  self->scheduler = Scheduler_new(self);
  self->actors    = List_new();
  self->lookupStack = Stack_new();
  self->parentLookupQueue = Queue_new();

  /* 
   * Ok, there are some interdependencies here:
   * - creating instances requires a retain stack
   * - retain stack is in Object->coro->exceptionStack, 
   *   so we need a proto Object(mainActor) to use for
   *   our retainStack
   * - defining any primitive methods requires Strings and CFunctions
   *
   * Solution:
   * - create Object, CFunction and String protos sans methods. 
   * - add methods to Object, CFunction and String
   */
   
  self->mainActor = IoObject_proto(self); /* need to do this first, so we have a retain stack */
  
  IoObject_createCoroIfAbsent(self->mainActor);
  IoState_addActor_(self, self->mainActor);
  IoState_currentActor_(self, self->mainActor);
  IoState_retain_(self, self->mainActor);

  stringProto = IoString_proto(self);
  
  /* --- per state globals --- */
  self->activateString = IoState_retain_(self, SIOSTRING("activate"));
  self->blockString   = IoState_retain_(self, SIOSTRING("thisBlock"));
  self->cloneString   = IoState_retain_(self, SIOSTRING("clone"));
  self->debugString   = IoState_retain_(self, SIOSTRING("debug"));  
  self->debugCollectorString = IoState_retain_(self, SIOSTRING("debugCollector"));  
  self->debugParserString = IoState_retain_(self, SIOSTRING("debugParser"));  
  /*self->delegateString = IoState_retain_(self, SIOSTRING("delegate"));*/
  self->equalEqualString  = IoState_retain_(self, SIOSTRING("=="));  
  self->forwardString = IoState_retain_(self, SIOSTRING("forward"));
  self->initString    = IoState_retain_(self, SIOSTRING("init"));
  self->messageString = IoState_retain_(self, SIOSTRING("thisMessage"));
  self->objectString  = IoState_retain_(self, SIOSTRING("Object"));  
  self->parentString  = IoState_retain_(self, SIOSTRING("parent"));
  self->protoString   = IoState_retain_(self, SIOSTRING("proto"));
  self->returnString  = IoState_retain_(self, SIOSTRING("return"));
  self->selfString    = IoState_retain_(self, SIOSTRING("self"));
  self->senderString  = IoState_retain_(self, SIOSTRING("sender"));  
  self->setSlotString  = IoState_retain_(self, SIOSTRING("setSlot"));  
  self->targetString  = IoState_retain_(self, SIOSTRING("target"));  
  
  PHash_at_put_(stringProto->slots, self->protoString, self->mainActor);

  IoCFunction_proto(self);
  IoString_protoFinish(self);
  IoObject_protoFinish(self);
  IoCFunction_protoFinish(self);
  
  self->setSlotBlock  = IoState_retain_(self, 
    IoObject_getSlot_(self->mainActor, SIOSTRING("setSlot")));  
  
  /* --- set up the lobby --- */
  {
  IoObject *object = self->mainActor; /*IoObject_rawClone(self->mainActor);*/
  IoObject *protos = IoObject_rawClone(object);
  IoObject *vm     = IoObject_rawClone(object);
  self->lobby      = IoObject_rawClone(object);
  IoState_retain_(self, self->lobby);
  /*IoObject_initProto(self->mainActor, self);*/ /* need to do this after tags and lobby are ready */
  
  /* config namespace */
  IoObject_setSlot_to_(self->lobby, SIOSTRING("Lobby"), self->lobby);
  IoObject_setSlot_to_(self->lobby, SIOSTRING("Protos"), protos);  
  IoObject_setSlot_to_(protos, SIOSTRING("IoVM"), vm);  

  /* config proto chain */
  IoObject_setSlot_to_(object, self->protoString, self->lobby);  
  IoObject_setSlot_to_(self->lobby, self->protoString, protos);  
  IoObject_setSlot_to_(protos, self->protoString, vm);  
  
  /* add protos to namespace */
  IoObject_setSlot_to_(vm, SIOSTRING("Object"), object);
  IoObject_setSlot_to_(vm, SIOSTRING("String"), stringProto);
  PHash_at_put_(vm->slots, SIOSTRING("Number"), IoNumber_proto(self)); /* to avoid copy-on-write */
  
  self->ioNil = IoNil_proto(self);
  IoObject_setSlot_to_(vm, SIOSTRING("Nil"), self->ioNil);
  IoObject_setSlot_to_(vm, SIOSTRING("Message"), IoMessage_proto(self));
  
  self->nilMessage  = IoMessage_newWithName_(self, SIOSTRING("Nil"));
  IoMessage_cachedResult_(self->nilMessage, self->ioNil);
  IoState_retain_(self, self->nilMessage);
  self->printMessage  = IoMessage_newWithName_(self, SIOSTRING("print"));
  IoState_retain_(self, self->printMessage);
  self->forwardMessage  = IoMessage_newWithName_(self, SIOSTRING("forward"));
  IoState_retain_(self, self->forwardMessage);
  
  IoObject_setSlot_to_(vm, SIOSTRING("Block"), IoBlock_proto(self));
  IoObject_setSlot_to_(vm, SIOSTRING("List"), IoList_proto(self));
  IoObject_setSlot_to_(vm, SIOSTRING("LinkedList"), IoLinkedList_proto(self));
  IoObject_setSlot_to_(vm, SIOSTRING("Map"), IoMap_proto(self));
#ifndef IO_OS_L4
  IoObject_setSlot_to_(vm, SIOSTRING("File"), IoFile_proto(self));
  IoObject_setSlot_to_(vm, SIOSTRING("Date"), IoDate_proto(self));
#endif
  IoObject_setSlot_to_(vm, SIOSTRING("Duration"), IoDuration_proto(self));
  IoObject_setSlot_to_(vm, SIOSTRING("Buffer"), IoBuffer_proto(self));
  IoObject_setSlot_to_(vm, SIOSTRING("WeakLink"), IoWeakLink_proto(self));
  IoFuture_proto(self);
  

  /* define Nop object */
  { 
    self->ioNop = IoObject_new(self);
    IoObject_setSlot_to_(self->ioNop, self->forwardString,
      IoCFunction_newWithFunctionPointer_tag_(self, (IoUserFunction *)IoObject_nop, self->lobby->tag));
    IoObject_setSlot_to_(vm, SIOSTRING("Nop"), self->ioNop);
    IoState_retain_(self, self->ioNop);
  }
  
  IoObject_setSlot_to_(self->lobby, SIOSTRING("version"), IoNumber_newWithDouble_(self, IO_VERSION_NUMBER));
  IoObject_setSlot_to_(self->lobby, SIOSTRING("distribution"), SIOSTRING("IoVM"));
  }
  
  self->stopStatus = MESSAGE_STOP_STATUS_NORMAL;
  self->returnValue = (void *)0x0;
  

  IoState_doCString_(self, "Object list = method(thisMessage argsEvaluatedIn(sender))");
  
  /* temporary tests */
 
#ifndef IO_OS_L4 
  IoState_doCString_(self, "Lobby test = method(Lobby launchPath = \"../../../IoVM/_tests\"; Lobby doFile(\"../../../IoVM/_tests/test.io\"))");
  
  IoState_doCString_(self, "Lobby test2 = method(Lobby launchPath = \"../../../IoVM/_tests\"; Lobby doFile(\"../../../IoVM/_tests/performance/threads.io\"))");

  IoState_doCString_(self, "Lobby test3 = method(Lobby launchPath = \"../../../IoVM/_tests\"; Lobby doFile(\"../../../IoVM/_tests/Concurrency/ErlangChallenge.io\"))");
#endif
  
  /* set protos object to look into it's slots for lookups */
  /*IoState_doCString_(self, "Lobby protos forward = method(methodName, Lobby protos foreach(k, slot, v = slot ?getSlot(methodName); if(v, return v)); Nil)");*/
  
  IoState_gcInitPhase(self);
  IoState_resumeGarbageCollector(self);
  IoState_clearRetainStack(self);
  return self;
}

void IoState_registerProtoWithFunc_(IoState *self, IoObject *proto, StateProtoFunc *func)
{ 
  if (PHash_at_(self->primitiveProtos, func))
  {
    printf("IoState_registerProtoWithFunc_() Error: attempt to add the same proto twice\n");
    exit(1);
  }
  IoState_retain_(self, proto);
  PHash_at_put_(self->primitiveProtos, func, proto); 
}

IoObject *IoState_protoWithName_(IoState *self, const char *name)
{
  IoObject *proto = PHash_first(self->primitiveProtos);
  while (proto)
  {
    if ( strcmp(IoObject_name(proto), name) == 0 ) return proto;
    proto = PHash_next(self->primitiveProtos);
  }
  return 0x0;
}


void IoState_free(IoState *self)
{
  MainArgs_free(self->mainArgs);
  Scheduler_free(self->scheduler);
  List_clear(self->actors);
  if (self->currentActor) IoState_retain_(self, self->currentActor); /* leak */
  List_clear(self->retainedValues);

  {
    IoObject *v;
    while ((v = Stack_pop(self->recycledObjects))) IoObject_dealloc(v);
    Stack_free(self->recycledObjects);
  }
  {
    IoObject *v;
    while ((v = Stack_pop(self->recycledObjectsSansSlots))) IoObject_dealloc(v);
    Stack_free(self->recycledObjectsSansSlots);
  }

  IoState_collectGarbage(self);
  List_free(self->actors);
  SplayTree_free(self->literalStrings);
  List_free(self->retainedValues);
  PHash_do_(self->primitiveProtos, (ListDoCallback *)IoTag_free);
  PHash_free(self->primitiveProtos);
  Stack_free(self->lookupStack);
  Queue_free(self->parentLookupQueue);
  Hash_free(self->valueToWeakLink);
  
  /*Stack_free(self->collectedWeakLinks);*/
  
  IoObjectGroup_free(self->blackGroup);
  IoObjectGroup_free(self->grayGroup);
  IoObjectGroup_free(self->whiteGroup);
  free(self);
}

IoObject *IoState_lobby(IoState *self) { return self->lobby; }

IoObject *IoState_protoWithInitFunction_(IoState *self, StateProtoFunc *func)
{ 
  IoObject *proto = PHash_at_(self->primitiveProtos, (void *)func); 
  if (!proto)
  {
    printf("IoState_protoWithInitFunction_() Error: missing proto\n");
    exit(1);
  }
  return proto;
}

/* --- garbage collection ------------------------------------------------------- */

IoObject *IoState_retain_(IoState *self, IoObject *v)
{ 
  /*IoObject_makeGrayIfWhite(v);*/
  return (IoObject*)List_add_(self->retainedValues, v); 
}

int  IoState_garbageCollectorIsPaused(IoState *self)
{ return (self->newValueCountToUpdate == INT_MAX); }

void IoState_pauseGarbageCollector(IoState *self)
{
  self->newValueCountToUpdate = INT_MAX;
  self->newValueCountToSweep  = INT_MAX;
}

void IoState_resumeGarbageCollector(IoState *self)
{
  self->newValueCountToUpdate = 10; 
  self->newValueCountToSweep  = STATE_DEFAULT_COLECT_AFTER;
}

void IoState_gcInitPhase(IoState *self)
{
  /* assume all values currently white */
  IoObject_makeGrayIfWhite(self->lobby);
  IoObject_mark(self->lobby);
  /*List_do_(self->actors, (ListDoCallback *)IoObject_makeGrayIfWhite); */
}

int IoState_gcSweepPhase(IoState *self)
{
  int count;
  unsigned char debug = IoObject_rawGetSlot_(self->lobby, self->debugCollectorString) ? 1 : 0;
  if (debug)  printf("--- collector start ---\n");
  
  /* marking the actors will mark their stacks */
  /* retain active coroutines. Note: mainActor is always retained */
  /*
  IoObject_makeGrayIfWhite(self->mainActor);
  IoObject_makeGrayIfWhite(self->currentActor);
  */
  List_do_(self->retainedValues, (ListDoCallback *)IoObject_makeGrayIfWhite);
  List_do_(self->actors, (ListDoCallback *)IoObject_makeGrayIfWhite);
  List_do_(self->actors, (ListDoCallback *)IoObject_mark); /* need to make sure they mark the coros */
  {
    IoString *k = PHash_firstKey(self->primitiveProtos);
    while (k)
    {
      IoObject *v = PHash_at_(self->primitiveProtos, k);
      IoObject_makeGrayIfWhite(v);
      /*
      if (debug && strcmp(v->tag->name, "Future")==0)
      {
        printf("marked Future proto\n");
        printf("color = %s\n", IoObject_markColorName(v));
      }
      */
      /*printf("marking proto %s\n", v->tag->name);*/
      k = PHash_nextKey(self->primitiveProtos);
    }
  }
  /*PHash_do_(self->primitiveProtos, (PHashDoCallback *)IoObject_makeGrayIfWhite);*/
  if (debug)
  {
    printf("white: %i\n", IoObjectGroup_count(self->whiteGroup));
    printf("gray:  %i\n", IoObjectGroup_count(self->grayGroup));
    printf("black: %i\n", IoObjectGroup_count(self->blackGroup));
    /*
    if(IoObjectGroup_sharesWithGroup_(self->whiteGroup, self->grayGroup))
    { printf("IoObjectGroup_sharesWithGroup_(self->whiteGroup, self->grayGroup)\n"); }
  
    if(IoObjectGroup_sharesWithGroup_(self->whiteGroup, self->blackGroup))
    { printf("IoObjectGroup_sharesWithGroup_(self->whiteGroup, self->blackGroup)\n"); }
    
    if(IoObjectGroup_sharesWithGroup_(self->grayGroup, self->blackGroup))
    { printf("IoObjectGroup_sharesWithGroup_(self->grayGroup, self->blackGroup)\n"); }
    */
  }

  while (!IoObjectGroup_isEmpty(self->grayGroup)) IoObjectGroup_markValues(self->grayGroup);
  
  /* move white items into free group */
  IoObject_moveToGroup_(self->whiteGroup, self->freeGroup);
  /* swap black group with the now empty white group */
  {
    IoObjectGroup *vg = self->blackGroup;
    self->blackGroup = self->whiteGroup;
    self->whiteGroup = vg;
  }
  count = IoObjectGroup_freeValues(self->freeGroup);

  IoState_gcInitPhase(self);
  self->newValueSinceSweepCount = 0;
  if (debug)
  {
    printf("freed: %i\n", count);
    printf("left:  %i\n", IoObjectGroup_count(self->whiteGroup));
    printf("perm:  %i\n", List_count(self->retainedValues));
    printf("actors: %i\n", List_count(self->actors));
    printf("protos: %i\n", PHash_count(self->primitiveProtos));
    {
      int i, max = List_count(self->actors);
      for (i=0; i< max; i++) 
      {
        IoObject *actor = List_at_(self->actors, i);
        /*if (Queue_length(actor->coro->queue)) */
        IoObject_print(actor);
      }
    }
    printf("--- collector done  ---\n");
  }

  return count;
}

int IoState_collectGarbage(IoState *self) { return IoState_gcSweepPhase(self); }

void IoState_makeBlackFromGray_(IoState *self, IoObject *v)
{ 
  IoObjectGroup_isRemoving_(self->grayGroup, v);
  IoObjectGroup_append_(self->blackGroup, v); 
}

void IoState_makeGrayFromWhite_(IoState *self, IoObject *v)
{ 
  IoObjectGroup_isRemoving_(self->whiteGroup, v);
  IoObjectGroup_append_(self->grayGroup, v); 
}

void IoState_makeGrayFromBlack_(IoState *self, IoObject *v)
{ 
  IoObjectGroup_isRemoving_(self->blackGroup, v);
  IoObjectGroup_append_(self->grayGroup, v); 
}

void IoState_immediatelyFreeIfUnreferenced_(IoState *self, IoObject *v)
{ 
  if (!v->mark.isReferenced)
  {
    if (v->mark.color != self->whiteGroup->mark.color)
    {
      printf("IoState_immediatelyFreeIfUnreferenced_() v->mark.color != IOMARK_WHITE\n");
      exit(1);
    }
  IoObjectGroup_isRemoving_(self->whiteGroup, v);
  IoObject_free(v);
  }
}

void IoState_gcMarkPhase(IoState *self)
{
  IoObjectGroup_markValues(self->grayGroup);

  self->newValueSinceSweepCount += self->newValueCount;
  self->newValueCount = 0;
  if ( /*IoObjectGroup_isEmpty(self->grayGroup) ||*/
    self->newValueSinceSweepCount > self->newValueCountToSweep) 
  {
    IoState_gcSweepPhase(self);
    self->newValueSinceSweepCount = 0;
  }
}

IoObject *IoState_addValue_(IoState *self, IoObject *v)
{ 
  if (!(v->tag))
  {
    printf("IoState_addValue_() Error: missing tag\n");
    exit(1);
  }
  if (self->newValueCount > self->newValueCountToUpdate) IoState_gcMarkPhase(self);
  IoObjectGroup_append_(self->whiteGroup, v);
  IoState_stackRetain_(self, v);
  self->newValueCount ++;
  /*printf("new %s\n", v->tag->name);*/
  return v;
}

/*--- weak links ---------------------------------------------*/

void IoState_addWeakLink_(IoState *self, IoWeakLink *w)
{
  IoObject *link = IoWeakLink_rawLink(w);
  IoWeakLink *first = Hash_at_(self->valueToWeakLink, link);
  if (first)
    IoWeakLink_appendWeakLink_(first, w);
  else
    Hash_at_put_(self->valueToWeakLink, link, w);
}

void IoState_removeWeakLink_(IoState *self, IoWeakLink *w)
{
  /* the WeakLink will unlink itself, we just need to check if it's the first */
  IoWeakLink *first = Hash_at_(self->valueToWeakLink, IoWeakLink_rawLink(w));
  if (first == w)
    Hash_at_put_(self->valueToWeakLink, IoWeakLink_rawLink(first), IoWeakLink_rawNextWeakLink(first));
}

void IoState_willFreeWeakLinkedValue_(IoState *self, IoObject *value)
{
  IoWeakLink *first = Hash_at_(self->valueToWeakLink, value);
  if (first) 
  {
    IoWeakLink_collected(first);
    /*Stack_push_(self->collectedWeakLinks, first);*/
    Hash_at_put_(self->valueToWeakLink, value, 0x0);
  }
}


/*--- retain/release roots ---------------------------------------------*/

void IoState_removeLiteralString_(IoState *self, IoString *aString)
{ SplayTree_removeKey_(self->literalStrings, aString); }
/*{ SplayTree_removeKeyWithPointer_(self->literalStrings, aString); }*/

void IoState_release_(IoState *self, IoObject *v)
{ List_removeLast_(self->retainedValues, v); }


size_t IoState_memorySize(IoState *self)
{ 
  size_t total = 0;
  /*
  int i;
  for(i=0; i<List_count(self->allValues); i++)
  { total += IoObject_memorySize(List_at_(self->allValues, i)); }
  */
  return total;
}

void IoState_compact(IoState *self)
{ 
  /*
  int i;
  for(i=0; i<List_count(self->allValues); i++)
  { IoObject_compact(List_at_(self->allValues, i)); }
  */
}

/* --- literal strings ------------------------------------------------------- */

IoString *IoState_stringWithByteArray_copy_(IoState *self, ByteArray *ba, int copy)
{ 
  /* find a match without creating a new String object */
  IoString *existingString = SplayTree_findWithFunction_match_(
    self->literalStrings, ioStringFindFunc, ba->bytes);
  if (existingString) 
  { 
    if (!copy) ByteArray_free(ba);
    return existingString; 
  }
  return IoState_addLiteralString_(self, IoString_newWithByteArray_copy_(self, ba, copy));
}

IoString *IoState_stringWithCString_(IoState *self, char *s)
{ 
  /* find a match without creating a new String object */
  IoString *existingString = SplayTree_findWithFunction_match_(
    self->literalStrings, ioStringFindFunc, s);
  if (existingString) return existingString;
  return IoState_addLiteralString_(self, IoString_newWithCString_(self, s));
}

IoString *IoState_stringWithCString_length_(IoState *self, char *s,
    int length)
{
  ByteArray *ba = ByteArray_newWithData_size_copy_(s, length, 1);
  return IoState_stringWithByteArray_copy_(self, ba, 0);
}


IoString *IoState_addLiteralString_(IoState *self, IoString *s)
{
  SplayTree_key_value_(self->literalStrings, s, s);
  IoString_setIsSymbol_(s, 1);
  return s;
}

/* --- performing --- */
IoObject *IoState_catchWithin_arg_(IoState *self, StateCatchWithinCallback *callback, void *context, void *arg)
{
  volatile IoObject *result = self->ioNil;
  #if !defined(__SYMBIAN32__)
  volatile IoExceptionCatch *eCatch = IoState_pushExceptionCatchWithName_(self, "");
  if (!IOEXCEPTIONCATCH_SETJMP(*(eCatch->savedStack)))
  #endif
  
  result = (*callback)(context, arg); 
  
  #if !defined(__SYMBIAN32__)
  else
  { IoState_callErrorCallback(self, (IoExceptionCatch *)eCatch); }
  IoState_popExceptionCatch_(self, (IoExceptionCatch *)eCatch);
  #endif
  return (IoObject *)result;
}

IoObject *IoState_rawDoCString_(IoState *self, char *s)
{
  char *label = "[command line]";
  IoObject *debugParser = IoObject_getSlot_(self->lobby, self->debugParserString);
  int debug = !ISNIL(debugParser);
  IoObject *result = self->ioNil;
  IoMessage *m = IoMessage_newFromText_label_(self, s, label);
  IoState_pushRetainPool(self);
  /*IoState_stackRetain_(self, m);*/
  if (m)
  {
      if (debug)
      {
        IoState_print_(self, "parsed: "); 
        IoMessage_print((IoMessage *)m); 
        IoState_print_(self, "\n"); 
      }
      result = IoMessage_locals_performOn_((IoMessage *)m, self->lobby, self->lobby);
      /*
      while (Scheduler_coroCount(self->scheduler) > 1) 
      { IoObject_yield(self->mainActor); }
      */
  }
  IoState_popRetainPool(self);
  IoState_stackRetain_(self, result);
  return result;
}

unsigned char IoState_yieldIfNeeded(IoState *self)
{
  if (Scheduler_coroCount(self->scheduler) > 1) 
  { IoObject_yield(self->mainActor); return 1; }
  return 0;
}

IoObject *IoState_doCString_(IoState *self, char *s)
{
  return IoState_catchWithin_arg_(self, 
    (StateCatchWithinCallback *)IoState_rawDoCString_, self, (void *)s);
}

#ifndef IO_OS_L4
IoObject *IoState_rawDoFile_(IoState *self, char *path)
{
  IoFile *file = IoFile_newWithPath_(self, SIOSTRING(path));
  IoObject *result = IoFile_rawDo(file, self->lobby);
  while (Scheduler_coroCount(self->scheduler) > 1) 
  { IoObject_yield(self->mainActor); }
  return result;
}

IoObject *IoState_doFile_(IoState *self, char *path)
{
  return IoState_catchWithin_arg_(self, 
    (StateCatchWithinCallback *)IoState_rawDoFile_, self, path);
}
#endif

/* --- callbacks: print, error, exit --- */

void IoState_justPrint_(IoState *self, char *s)
{ 
  if (self->printCallback) { self->printCallback(self, s); }
  else { printf("%s", s); }
}

void IoState_print_(IoState *self, const char *format, ...)
{
  ByteArray *ba;
  va_list ap;
  va_start(ap, format);
  ba = ByteArray_newWithVargs_(format, ap);
  IoState_justPrint_(self, ByteArray_asCString(ba));
  ByteArray_free(ba);
  va_end(ap);
}

void IoState_error_description_(IoState *self, IoMessage *m, const char *name, const char *format, ...)
{
  IoString *exceptionDescription;
  ByteArray *ba;
  va_list ap;
  va_start(ap, format);
  ba = ByteArray_newWithVargs_(format, ap);
  exceptionDescription = IoString_newWithByteArray_copy_(self, ba, 0);
  va_end(ap);
  
  if (IoState_garbageCollectorIsPaused(self))
  { printf("%s\n", CSTRING(exceptionDescription)); }
  
  IoState_raiseExceptionWithName_andDescription_(
    self, SIOSTRING((char *)name), exceptionDescription, m);
}

void IoState_printCallback_(IoState *self, StatePrintCallback* callback)
{ self->printCallback = callback; }

void IoState_errorCallback_(IoState *self, StateErrorCallback* callback)
{ self->errorCallback = callback; }

void IoState_exitCallback_(IoState *self, StateExitCallback* callback)
{ self->exitCallback = callback; }

void IoState_exit(IoState *self)
{ if (self->exitCallback) { self->exitCallback(self); } else { exit(0); } }

/* --- exception handling ------------------------------------------ */

void IoState_syncRetainStack(IoState *self)
{
  IoExceptionCatch *topCatch = Stack_top(self->currentActor->coro->exceptionCatchStack);
  self->currentRetainStack = topCatch->retainedValues;
}

/* called by beginning of IoObject_exceptionCatch method */
IoExceptionCatch *IoState_pushExceptionCatchWithName_(IoState *self, char *exceptionName)
{
  IoExceptionCatch *eCatch = IoExceptionCatch_newWithCStringName_(exceptionName);
  Stack_push_(self->currentActor->coro->exceptionCatchStack, eCatch);
  IoState_syncRetainStack(self);
  return eCatch;
}

/* called by end of IoObject_exceptionCatch method */
void IoState_popExceptionCatch_(IoState *self, IoExceptionCatch *eCatch)
{
  if(Stack_top(self->currentActor->coro->exceptionCatchStack) == eCatch)
  { IoExceptionCatch_free(Stack_pop(self->currentActor->coro->exceptionCatchStack)); }
  IoState_syncRetainStack(self);
}

/* called by IoObject_error */
void IoState_raiseExceptionWithName_andDescription_(IoState *self, 
  IoString *exceptionName, IoString *description, IoMessage *m)
{
  IoExceptionCatch *eCatch = Stack_pop(self->currentActor->coro->exceptionCatchStack);
  
  /*printf("%s\n", CSTRING(description));*/ /* tmp hack --------------------------------------- */
  
  while (eCatch && !IoExceptionCatch_matchesId_(eCatch, exceptionName))
  { 
    IoExceptionCatch_free(eCatch);
    eCatch = Stack_pop(self->currentActor->coro->exceptionCatchStack); 
  }
  
  if (eCatch)
  {
    Stack_push_(self->currentActor->coro->exceptionCatchStack, eCatch);
    IoState_syncRetainStack(self);
    IoExceptionCatch_caughtName_(eCatch, exceptionName);
    IoExceptionCatch_caughtDescription_(eCatch, description);
    IoExceptionCatch_caughtMessage_(eCatch, m);
    IoExceptionCatch_jump(eCatch);
  }
  IoState_syncRetainStack(self);
}

void IoState_callErrorCallback(IoState *self, IoExceptionCatch *eCatch)
{
  if (self->errorCallback) 
  { self->errorCallback(self, CSTRING(eCatch->caughtName), CSTRING(eCatch->caughtDescription)); }
  else
  { IoState_printStackTrace(self, eCatch); }
}

/* --- stack trace ------------------------------------------ */

void IoState_printStackTrace(IoState *self, IoExceptionCatch *eCatch)
{
  IoMessage *topMessage = eCatch->caughtMessage;
  Stack *exceptions = self->currentActor->coro->exceptionCatchStack;
  int i;
  if (Stack_count(exceptions))
  {
    IoState_justPrint_(self, "\n<pre>\n");
    IoState_justPrint_(self, "Stack trace on current coroutine\n");
    IoState_justPrint_(self, "--------------------------------\n\n");
    IoState_print_(self, "ExceptionId: %s\n",   CSTRING(eCatch->caughtName)); 
    IoState_print_(self, "Description: %s\n\n", CSTRING(eCatch->caughtDescription)); 
    
    IoState_print_(self, "Label %# Line \t Char \t Message\n", 20-5, 20-6, 4);
    IoState_print_(self, "----- %# ---- \t ---- \t -------\n", 20-5, 20-6, 4);
   
    IoState_showStackEntryMessage_(self, topMessage);
   
    for(i=0; i < Stack_count(exceptions); i++)
    {
     IoExceptionCatch *eCatchItem = Stack_at_(exceptions, i);
     if (i!=0 && strlen(eCatchItem->exceptionName)) 
     { 
       IoState_print_(self, " catch \"%s\"\n", eCatchItem->exceptionName);
       if(getenv("QUERY_STRING")) IoState_print_(self, "<BR>");
     }
     Stack_do_on_(eCatchItem->retainedValues, (StackDoOnCallback *)IoState_showStackEntry_, self);
    }
    IoState_justPrint_(self, "\n");
  }
}

void IoState_showStackEntry_(IoState *self, void *blockLocals)
{
  IoMessage *m   = IoObject_rawGetSlot_(blockLocals, self->messageString); /* calling message */
  IoBlock *block = IoObject_rawGetSlot_(blockLocals, self->blockString);
  if (m && block) IoState_showStackEntryMessage_(self, m);
}

void IoState_showStackEntryMessage_(IoState *self, IoMessage *m)
{ 
  if (!m)
  {
    IoState_print_(self, "[internal] %# ? \t ? \t ?\n", 20 - 10);
    return;
  }
  else
  {
  ByteArray *description = IoMessage_descriptionJustSelfAndArgs(m);
  ByteArray *label = ByteArray_clone(IoString_rawByteArray(IoMessage_rawLabel(m)));
  ByteArray_removePathExtension(label);
  ByteArray_clipBeforeLastPathComponent(label);
  /* --- clip the description to something reasonable, if needed --- */
  {
    int ret = ByteArray_findCString_from_(description, "\n", 0);
    if (ret != -1)
    { 
      ByteArray_removeFrom_to_(description, ret, -1);
      ByteArray_appendCString_(description, "...");
    }
    if (ByteArray_length(description) > 40)
    { 
      ByteArray_removeFrom_to_(description, 40, -1);
      ByteArray_appendCString_(description, "...");
    }
  }
  
  {
  char line[16];
  char character[16];
  
  if (IoMessage_rawLineNumber(m) != -1) 
  { sprintf(line, "%i", IoMessage_rawLineNumber(m)); } else { strcpy(line, "-"); }
  
  if (IoMessage_rawCharNumber(m) != -1) 
  { sprintf(character, "%i", IoMessage_rawCharNumber(m)); } else { strcpy(character, "-"); }
  
  IoState_print_(self, "%s %# %s \t %s \t %s\n", 
           ByteArray_asCString(label), 
           20 - ByteArray_length(label),
           line, character,
           ByteArray_asCString(description));
           
  /*IoState_print_(self, "QUERY_STRING %s\n", getenv("QUERY_STRING"));*/
  /* if(getenv("QUERY_STRING")) IoState_print_(self, "<BR>");*/
      
  ByteArray_free(description);
  ByteArray_free(label);
  }
  }
}

/* --- actors ------------------------------------------------------ */
void IoState_addActor_(IoState *self, IoObject *actor) 
{ 
  /*IoObject_makeGrayIfWhite(actor);*/
  List_add_(self->actors, actor); 
}

void IoState_removeActor_(IoState *self, IoObject *actor) { List_remove_(self->actors, actor); }
IoObject *IoState_currentActor(IoState *self) { return self->currentActor; }

void IoState_currentActor_(IoState *self, IoObject *actor) 
{ 
  IoExceptionCatch *eCatch = Stack_top(actor->coro->exceptionCatchStack);
  self->currentActor = actor; 
  if (eCatch) self->currentRetainStack = eCatch->retainedValues;
}

void IoState_yield(IoState *self) { IoObject_yield(self->currentActor); }

/* --- command line ------------------------------------------------------ */

void IoState_argc_argv_(IoState *self, int argc, const char *argv[])
{ MainArgs_argc_argv_(self->mainArgs, argc, (char **)argv); }

int IoState_main(IoState *self, int argc, const char *argv[])
{
  MainArgs_argc_argv_(self->mainArgs, argc, (char **)argv);
  IoState_catchWithin_arg_(self, IoState_runCLI, self, (char *)0x0);
  return 0;
}

IoObject *IoState_runCLI(void *context, char *s)
{
  IoState *self = context;
  int argc = self->mainArgs->argc;
  char **argv = self->mainArgs->argv;

  if (argc < 2) 
  {
    IoNumber *version = IoObject_getSlot_(self->lobby, SIOSTRING("version"));
    IoString *distribution = IoObject_getSlot_(self->lobby, SIOSTRING("distribution"));
    CLI *cli = CLI_new();
    IoState_print_(self, "%s version %i\n", CSTRING(distribution), IoNumber_asInt(version)); 
    CLI_prompt_(cli, "Io> ");
    CLI_target_callback_(cli, self, (CLICallback *)IoState_cliInput);
   
#ifndef IO_OS_L4 
    {
      ByteArray *path = IoFile_CurrentWorkingDirectoryAsByteArray();
      IoObject_setSlot_to_(self->lobby, 
        SIOSTRING("launchPath"), 
        IoState_stringWithByteArray_copy_(self, path, 0));
    }
#endif
    CLI_run(cli);
    CLI_free(cli);
  }
  else
  { 
#ifndef IO_OS_L4
    IoList *argsList = IoList_new(self);
    int i;
    for (i=2; i < argc; i++)
    { IoList_rawAdd_(argsList, SIOSTRING((char *)argv[i])); }
    IoObject_setSlot_to_(self->lobby, SIOSTRING("args"), argsList);
    
    IoState_mainRunFile_(self, (char *)argv[1]);
#endif
  }
  return self->ioNil;
}

#ifndef IO_OS_L4
void IoState_mainRunFile_(IoState *self, char *filePath)
{
  /* --- set launch path --- */
  ByteArray *path;
  if (*filePath == '/')
  { path = ByteArray_newWithCString_(filePath); }
  else
  {
    path = IoFile_CurrentWorkingDirectoryAsByteArray();
    ByteArray_appendPathCString_(path, filePath);
  }
  
  ByteArray_removeLastPathComponent(path);  
  
  IoObject_setSlot_to_(self->lobby, 
        SIOSTRING("launchPath"), 
        IoState_stringWithByteArray_copy_(self, path, 0));
  IoState_doFile_(self, filePath); 
}
#endif

void IoState_cliInput(IoState *self, char *s) 
{
  IoObject *v = IoState_doCString_(self, s);
  IoState_justPrint_(self, "\n==> ");
  if (v) IoObject_print(v); 
  IoState_justPrint_(self, "\n");
  IoState_clearRetainStack(self);
}

/* --- user data -------------------------------------------------- */
void IoState_userData_(IoState *self, void *userData) { self->userData = userData; }
void *IoState_userData(IoState *self) { return self->userData; }

