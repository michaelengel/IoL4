/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#include "IoState.h"
#define IOOBJECT_C
#include "IoObject.h"
#undef IOOBJECT_C
#include "IoObject_actor.h"
#include "IoTag.h"
#include "IoCFunction.h"
#include "IoString.h"
#include "IoNumber.h"
#include "IoMessage.h"
#include "IoMessage_parser.h"
#include "IoCFunction.h"
#include "IoBlock.h"
#include "IoNil.h"
#include "IoList.h"
#include "IoObject.h"
#ifndef IO_OS_L4
#include "IoFile.h"
#endif
#include "IoFuture.h" /* for print */
#include <string.h>
#include <stddef.h>

IoTag *IoObject_tag(void *state)
{
  IoTag *tag = IoTag_newWithName_("Object");
  tag->state = state;
  tag->cloneFunc = (TagCloneFunc *)IoObject_rawClone;
  tag->printFunc = (TagPrintFunc *)IoObject_printFunc;
  /*tag->activateFunc = (TagActivateFunc *)IoObject_activateFunc;*/
  /* no hooks, as the Object methods are the ones that call the hooks */
  return tag;
}

IoObject *IoObject_proto(void *state)
{
  IoObject *self = calloc(1, sizeof(IoObject));
  self->slots = PHash_new();
  self->mark.ownsSlots = 1;
  self->coro = IoCoroutine_new(); /* for mainActor */
  
  self->tag = IoObject_tag(state);
  self->data = 0x0;
  IoState_registerProtoWithFunc_(state, self, IoObject_proto);
  return self;
}

IoObject *IoObject_protoFinish(void *state)
{
  IoNumber *self = IoState_protoWithInitFunction_(state, IoObject_proto);
  
  IoObject_addMethod_(self, IOSTRING("clone"), IoObject_clone);
  IoObject_addMethod_(self, IOSTRING("shallowCopy"), IoObject_shallowCopy);
  /*IoObject_addMethod_(self, IOSTRING("print"), IoObject_protoPrint);*/
  IoObject_addMethod_(self, IOSTRING("write"), IoObject_protoWrite);
  IoObject_addMethod_(self, IOSTRING("type"), IoObject_type);

  /* logic - need these since Object is a special primitive */
  IoObject_addMethod_(self, IOSTRING("and"), IoObject_and);
  IoObject_addMethod_(self, IOSTRING("or"), IoObject_or);
  
  IoObject_addMethod_(self, IOSTRING("<"), IoObject_isLessThan_);
  IoObject_addMethod_(self, IOSTRING(">"), IoObject_isGreaterThan_);
  IoObject_addMethod_(self, IOSTRING(">="), IoObject_isGreaterThanOrEqualTo_);
  IoObject_addMethod_(self, IOSTRING("<="), IoObject_isLessThanOrEqualTo_);
  
  /* comparison */
  IoObject_addMethod_(self, IOSTRING("=="), IoObject_equals);
  IoObject_addMethod_(self, IOSTRING("!="), IoObject_notEquals);
  
  /* introspection */
  /*IoObject_addMethod_(self, IOSTRING("self"), IoObject_self);*/
  IoObject_addMethod_(self, IOSTRING("setSlot"), IoObject_protoSet_to_);
  IoObject_addMethod_(self, IOSTRING("updateSlot"), IoObject_protoUpdateSlot_to_);
  IoObject_addMethod_(self, IOSTRING("getSlot"), IoObject_protoGet_);
  IoObject_addMethod_(self, IOSTRING("hasSlot"), IoObject_protoHasSlot);
  IoObject_addMethod_(self, IOSTRING("removeSlot"), IoObject_protoRemoveSlot);
  IoObject_addMethod_(self, IOSTRING("slotNames"), IoObject_protoSlotNames);
  
  /* method invocation */
  IoObject_addMethod_(self, IOSTRING("perform"), IoObject_protoPerform);
  IoObject_addMethod_(self, IOSTRING("performWithArgList"), IoObject_protoPerformWithArgList);
  IoObject_addMethod_(self, IOSTRING("?"), IoObject_try);
  /*IoObject_addMethod_(self, IOSTRING("forward"), IoObject_protoForward_);*/
  IoObject_addMethod_(self, IOSTRING("super"), IoObject_super);
  
  /* control */
  IoObject_addMethod_(self, IOSTRING("block"), IoBlock_lobbyBlock);
  IoObject_addMethod_(self, IOSTRING("method"), IoBlock_lobbyMethod);
  IoObject_addMethod_(self, IOSTRING("for"), IoObject_protoFor);
  IoObject_addMethod_(self, IOSTRING("if"), IoObject_protoIf);
  IoObject_addMethod_(self, IOSTRING("then"), IoObject_evalArg);
  IoObject_addMethod_(self, IOSTRING("else"), IoObject_nop);
  IoObject_addMethod_(self, IOSTRING("elseif"), IoObject_nop);
  
  IoObject_addMethod_(self, IOSTRING("return"), IoObject_return);
  IoObject_addMethod_(self, IOSTRING("while"), IoObject_protoWhile);
  IoObject_addMethod_(self, IOSTRING("break"), IoObject_break);
  IoObject_addMethod_(self, IOSTRING("continue"), IoObject_continue);
  IoObject_addMethod_(self, IOSTRING("exit"), IoObject_exit);

  /* utility */
  IoObject_addMethod_(self, IOSTRING("print"), IoObject_lobbyPrint);
  IoObject_addMethod_(self, IOSTRING("collectGarbage"), IoObject_collectGarbage);
  IoObject_addMethod_(self, IOSTRING("do"), IoObject_do);
  IoObject_addMethod_(self, IOSTRING("doMessage"), IoObject_doMessage);
  IoObject_addMethod_(self, IOSTRING("doString"), IoObject_doString);
#ifndef IO_OS_L4
  IoObject_addMethod_(self, IOSTRING("doFile"), IoObject_doFile);
#endif
  
  /* exceptions */
  IoObject_addMethod_(self, IOSTRING("catchException"), IoObject_catchException);
  IoObject_addMethod_(self, IOSTRING("raiseException"), IoObject_raiseException);
  
  /* actor */
  IoObject_addMethod_(self, IOSTRING("@"), IoObject_futurePerform);
  IoObject_addMethod_(self, IOSTRING("@@"), IoObject_asyncPerform);
  IoObject_addMethod_(self, IOSTRING("yield"), IoObject_actorYield);
  IoObject_addMethod_(self, IOSTRING("resume"), IoObject_actorResume);
  IoObject_addMethod_(self, IOSTRING("pause"), IoObject_actorPause);
  IoObject_addMethod_(self, IOSTRING("wait"), IoObject_actorWait);
  IoObject_addMethod_(self, IOSTRING("isActive"), IoObject_isActive);
  IoObject_addMethod_(self, IOSTRING("activeCoroCount"), IoObject_activeCoroCount);
  
  IoObject_addMethod_(self, IOSTRING("setSchedulerSleepSeconds"), IoObject_setSchedulerSleepSeconds);
  IoObject_addMethod_(self, IOSTRING("schedulerSleepSeconds"), IoObject_schedulerSleepSeconds);


  /* utility */
  IoObject_addMethod_(self, IOSTRING("uniqueId"), IoObject_uniqueId);
  /*
  IoObject_addMethod_(self, IOSTRING("memorySize"), IoObject_memorySizeMethod);
  IoObject_addMethod_(self, IOSTRING("compact"), IoObject_compactMethod);
  IoObject_addMethod_(self, IOSTRING("memorySizeOfState"), IoObject_memorySizeOfState);
  IoObject_addMethod_(self, IOSTRING("compactState"), IoObject_compactState);
  */
  IoObject_addMethod_(self, IOSTRING("getenv"), IoObject_getenv);
  
  /* loops */
  IoObject_addMethod_(self, IOSTRING("foreach"), IoObject_foreach);
  IoObject_addMethod_(self, IOSTRING("-"), IoObject_subtract);
  IoObject_addMethod_(self, IOSTRING("system"), IoObject_system);
  IoObject_addMethod_(self, IOSTRING("compileString"), IoObject_compileString);
  IoObject_addMethod_(self, IOSTRING("isNil"), IoObject_isNil);
  IoObject_addMethod_(self, IOSTRING("ifNil"), IoObject_ifNil);
  IoObject_addMethod_(self, IOSTRING("ifTrue"), IoObject_ifTrue);
  IoObject_addMethod_(self, IOSTRING("ifFalse"), IoObject_ifFalse);
  return self;
}

void IoObject_addMethod_(IoObject *self, IoString *slotName, void *fp)
{
  IoTag *t = self->tag;
  IoObject *proto = IoState_protoWithInitFunction_(IOSTATE, IoObject_proto);
  if (t == proto->tag) t = 0x0;
  IoObject_setSlot_to_(self, slotName, 
    IoCFunction_newWithFunctionPointer_tag_(IOSTATE, (IoUserFunction *)fp, t));
}

IoObject *IoObject_new(void *state)
{
  IoObject *proto = IoState_protoWithInitFunction_(state, IoObject_proto);
  return IoObject_rawClone(proto);
}

IoObject *IoObject_justClone(IoObject *self)
{ return (self->tag->cloneFunc)(self); }

/*#define IOOBJECT_RECYCLE*/
#ifdef IOOBJECT_RECYCLE

IoObject *IoObject_rawClone(IoObject *self)
{
  IoObject *child = Stack_pop(IOSTATE->recycledObjects);
  if (!child) 
  {
    child = calloc(1, sizeof(IoObject));
    child->slots = PHash_new();
    child->mark.ownsSlots = 1;
  }
  /*else { printf("recycling %p\n", child); }*/
  
  child->tag = self->tag;
  child->data = self->data;
  
  {
  IoString *protoString = ((IoState *)self->tag->state)->protoString;
  if (protoString) PHash_at_put_(child->slots, protoString, self);
  child->proto = self;
  }
    
  IoState_addValue_(IOSTATE, child);
  return child;
}

/*
IoObject *IoObject_rawClonePrimitive(IoObject *self)
{
  IoObject *child = calloc(1, sizeof(IoObject));
  child->tag = self->tag;
  child->slots = self->slots;
  child->data = 0x0;
  child->proto = self;
  IoState_addValue_(IOSTATE, child);
  return child;
}
*/

IoObject *IoObject_rawClonePrimitive(IoObject *self)
{
  IoObject *child = Stack_pop(IOSTATE->recycledObjectsSansSlots);
  if (!child) 
  { child = calloc(1, sizeof(IoObject)); }  
  else { memset(child, 0x0, sizeof(IoObject)); }

  child->tag = self->tag;
  child->slots = self->slots;
  child->data = 0x0;
  child->proto = self;
  IoState_addValue_(IOSTATE, child);
  return child;
}

void IoObject_dealloc(IoObject *self)
{  
  if (self->data)
  {
  TagFreeFunc *func = self->tag->freeFunc;
  if (func) { (*func)(self); } 
  }
  
  if (self->mark.ownsSlots) PHash_free(self->slots);
  if (self->coro) IoCoroutine_free(self->coro);  
  memset(self, 0x0, sizeof(IoObject)); /* temporary - for debugging */
  free(self);
}

void IoObject_free(IoObject *self)
{
  TagFreeFunc *func = self->tag->freeFunc;
  if (func) { (*func)(self); }
  self->data = 0x0; 
  
  if (self->mark.ownsSlots)
  {
    PHash_clean(self->slots);
    Stack_push_(IOSTATE->recycledObjects, self);
  }
  else
  { 
    Stack_push_(IOSTATE->recycledObjectsSansSlots, self); 
  }
  /*IoObject_dealloc(self);*/
}

#else

IoObject *IoObject_rawClone(IoObject *self)
{
  IoObject *child = calloc(1, sizeof(IoObject));
  IoString *protoString = ((IoState *)self->tag->state)->protoString;
  child->tag = self->tag;
  child->slots = PHash_new();
  child->mark.ownsSlots = 1;
  if (protoString) PHash_at_put_(child->slots, protoString, self);
  child->proto = self;
  child->data = self->data;
  IoState_addValue_(IOSTATE, child);
  return child;
}

IoObject *IoObject_rawClonePrimitive(IoObject *self)
{
  IoObject *child = calloc(1, sizeof(IoObject));
  child->tag = self->tag;
  child->slots = self->slots;
  child->proto = self;
  child->data = 0x0;
  IoState_addValue_(IOSTATE, child);
  return child;
}

void IoObject_dealloc(IoObject *self) { IoObject_free(self); }

void IoObject_free(IoObject *self)
{  
  TagFreeFunc *func = self->tag->freeFunc;
  if (func) { (*func)(self); } 
  
  if (self->mark.ownsSlots) PHash_free(self->slots);
  if (self->coro) IoCoroutine_free(self->coro);  
  memset(self, 0x0, sizeof(IoObject)); /* temp */
  free(self);
}

#endif

int IoObject_compareFunc(IoObject *self, IoObject *v)
{ 
  IoState *state = IOSTATE;
  if ((void *)self == (void *)v) return 0;
  {
    /* call a compare method instead? this is too slow */
    IoMessage *m = IoMessage_newWithName_(state, state->equalEqualString);
    IoMessage_addCachedArg_(m, v);
    return ISNIL(IoMessage_locals_performOn_(m, self, self)) ? -1 : 0; /* self as locals == hack */
  }
}

/* ------------------------------------------------------
 * slot lookups with lookup loop detection 
 * ------------------------------------------------------*/

inline IoObject *IoObject_getProtoChainSlot_(IoObject *self, IoString *slotName)
{ 
  /* walk up proto chain searching for slotName */
  IoObject *v;
  IoObject *obj = self;
  Stack *lookupStack = IOSTATE->lookupStack;
  IoString *parentString = IOSTATE->parentString;
  Queue *parentLookupQueue = IOSTATE->parentLookupQueue;
  /*int doPrint = 0;*/
  
  do
  {
    /*
    if (doPrint)
    {
      IoObject_printFunc(obj);
      printf("\n");
    }
    */
    if (obj->mark.hasDoneLookup) break;

    v = PHash_at_(obj->slots, slotName);
    if (v) return v;
    
    {
      v = PHash_at_(obj->slots, parentString);
      if (v) Queue_push_(parentLookupQueue, v);
    }
    Stack_push_(lookupStack, obj);
    obj->mark.hasDoneLookup = 1;
    obj = obj->proto;
  } while (obj);
  
  return 0x0;
}

IoObject *IoObject_rawGetSlotWithoutClear_(IoObject *self, IoString *slotName);

inline IoObject *IoObject_getParentChainSlot_(IoObject *self, IoString *slotName)
{ 
  IoObject *v = 0x0;
  Queue *parentLookupQueue = IOSTATE->parentLookupQueue;
  IoObject *parent;

  while ((parent = Queue_pop(parentLookupQueue)))
  {
    if (parent->mark.hasDoneLookup) continue;
    v = IoObject_rawGetSlotWithoutClear_(parent, slotName);
    if (v) return v;
  }
  return 0x0;
}

IoObject *IoObject_rawGetSlotWithoutClear_(IoObject *self, IoString *slotName)
{ 
  IoObject *v = IoObject_getProtoChainSlot_(self, slotName);
  if (!v) v = IoObject_getParentChainSlot_(self, slotName);
  return v;
}

inline void IoObject_clearLookupStack(IoObject *self)
{
  Stack *lookupStack = IOSTATE->lookupStack;
  IoObject *obj;
  while ((obj = Stack_pop(lookupStack))) { obj->mark.hasDoneLookup = 0; }
  Queue_clear(IOSTATE->parentLookupQueue);
}

IoObject *IoObject_rawGetSlot_(IoObject *self, IoString *slotName)
{
  /*printf("getSlot %s\n", CSTRING(slotName));*/
  {
  IoObject *v = IoObject_getProtoChainSlot_(self, slotName);
  if (!v) v = IoObject_getParentChainSlot_(self, slotName);
  IoObject_clearLookupStack(self);
  return v;
  }
}

/* ------------------------------------------------------ */

IoObject *IoObject_getSlot_(IoObject *self, IoString *slotName)
{ 
  IoObject *v = IoObject_rawGetSlot_(self, slotName);
  return v ? v : IONIL(self);
}

IoObject *IoObject_activateFunc(IoObject *self, IoObject *target, IoObject *locals, IoMessage *m)
{
  IoState *state = IOSTATE;
  IoObject *b = IoObject_rawGetSlot_(self, state->activateString);
  if (b && ISBLOCK(b)) { return IoBlock_target_locals_call_((IoBlock *)b, self, locals, m); }
  return self;
}

IoObject *IoObject_activate(IoObject *self, IoObject *target, IoObject *locals, IoMessage *m)
{
  TagActivateFunc *act = self->tag->activateFunc;
  if (act) { return (*act)(self, target, locals, m); }
  return self;
}

/* ----------------------------------------------------------- */

IoObject *IoObject_objectWithSlotValue_(IoObject *self, IoObject *slotValue)
{
  /* only the proto chain is followed */
  IoString *slotName = 0x0;
  IoObject *proto = self;
  Stack *lookupStack = IOSTATE->lookupStack;
  for (;;)
  {
    slotName = (IoString *)PHash_firstKeyForValue_(proto->slots, slotValue);
    if (slotName) { IoObject_clearLookupStack(self); return proto; }
    Stack_push_(lookupStack, proto);
    proto->mark.hasDoneLookup = 1;
    proto = proto->proto; 
    if ((!proto) | proto->mark.hasDoneLookup) 
    { IoObject_clearLookupStack(self); return (void *)0x0; }
  }
}

void IoObject_setSlot_to_(IoObject *self, IoString *slotName, IoObject *value)
{ 
  if (!self->mark.ownsSlots)
  {
    self->slots = PHash_new();
    self->mark.ownsSlots = 1;
    PHash_at_put_(self->slots, IOSTATE->protoString, self->proto);
  }
  /*if (!slotName->isSymbol) { printf("Io System Error: setSlot slotName not symbol\n"); exit(1); }*/
  if (ISNUMBER(value)) value = (void *)IoNumber_newCopyOf_((IoNumber *)value);
  if(PHash_at_put_(self->slots, IOREF(slotName), IOREF(value)))
  { self->mark.isDirty = 1; }
  
  if (slotName == IOSTATE->protoString) self->proto = value;
}

void IoObject_removeSlot_(IoObject *self, IoString *slotName)
{ 
  if (slotName == IOSTATE->protoString) 
  {
    printf("removing proto slot\n");
    self->proto = 0x0;
  }
  PHash_removeKey_(self->slots, slotName); 
}

IoObject *IoObject_perform(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoObject *slotValue = IoObject_rawGetSlot_(self, IoMessage_rawMethod(m));
  if (slotValue) return IoObject_activate(slotValue, self, locals, m);
  
  /* for locals */
  {
    IoObject *selfDelegate = IoObject_rawGetSlot_(self, IOSTATE->selfString);
    if (selfDelegate && selfDelegate != self) 
    { return IoObject_perform(selfDelegate, locals, m); }
  }
  return IoObject_forward(self, locals, m);
}

IoObject *IoObject_forward(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoState *state = IOSTATE;
  IoObject *forwardSlot = IoObject_rawGetSlot_(self, state->forwardString);
  IoExceptionCatch *eCatch = Stack_top(state->currentActor->coro->exceptionCatchStack);
  
  if (Stack_totalSize(eCatch->retainedValues) > 2000)
  {
    IoState_error_description_(state, 0x0,
      "Io.Object.forward", "stack overflow");
  }
  
  if (!IoMessage_rawMethod(m))
  {
    IoState_error_description_(state, 0x0,
      "Io.Object.doesNotRespond", "%s does not respond to message 0x0", 
      IoObject_name(self));
  }

  /*printf("%p\n", (void *)(IoMessage_rawMethod(m)));
  printf("%p forwarding '%s'\n", self, CSTRING(IoMessage_rawMethod(m)));
  */
  
  if (forwardSlot) return IoObject_activate(forwardSlot, self, locals, m);

  IoState_error_description_(state, m,
      "Io.Object.doesNotRespond", "%s does not respond to message '%s'", 
      IoObject_name(self), CSTRING(IoMessage_rawMethod(m)));
  return IONIL(self);
}

/* -- name ------------------------------------------------------ */

void IoObject_printFunc(IoObject *self) 
{
  IoState *state = IOSTATE;
  IoString *key = PHash_firstKey(self->slots);
  IoState_print_(IOSTATE, "Object_%p = Object clone do(\n", (void *)self);
  while (key)
  {
    IoObject *value = PHash_at_(self->slots, key);
    /*if (strpbrk(CSTRING(key), " "
    IoState_print_(state, "  '%s' = ", CSTRING(key));
    */
    IoState_print_(state, "  %s = ", CSTRING(key));
    if (value->data)
    { IoObject_print(value); }
    else
    {
      const char *name = IoObject_name(value);
      IoState_print_(state, "%s_%p", name, (void *)value);
    }
    IoState_print_(state, "\n");
    key = PHash_nextKey(self->slots);
  }
  if (self->coro /*&& Queue_length(self->coro->queue)*/)
  {
    Queue *q = self->coro->queue;
    int i;
    /*printf("  // aync queue length = %i\n", Queue_length(q));*/
    if (IoObject_isPaused(self))
    {
        printf("  // actor paused\n");
    }
    else
    {
    printf("  // aync queue:\n");
    for (i=0; i < Queue_length(q); i++)
    {
      IoFuture *f = Queue_at_(q, i);
      /*IoString *name = IoMessage_rawMethod(IoFuture_rawMessage(f));*/
      printf("  //   %i: ", i); 
      IoMessage_print(IoFuture_rawMessage(f));
      printf("\n"); 
    }
    }
    printf("  // coro retaining: %i\n", IoCoroutine_retainingCount(self->coro)); 
  }
  IoState_print_(state, ")\n");
}

size_t IoObject_memorySizeFunc(IoObject *self)
{
  size_t t = sizeof(IoObject) + PHash_memorySize(self->slots);
  if (self->coro) t += IoCoroutine_memorySize(self->coro);
  return t;
}

void IoObject_compactFunc(IoObject *self)
{
  PHash_compact(self->slots);
  if (self->coro) IoCoroutine_compact(self->coro);
}

/* -- proto methods ---------------------------------------------- */
IoObject *IoObject_protoPerform(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoString *slotName = IoMessage_locals_stringArgAt_(m, locals, 0);
  IoObject *v = IoObject_rawGetSlot_(self, slotName);
  if (v)
  {
    IoMessage *newMessage = IoMessage_deepCopyOf_(m);
    List *args = IoMessage_rawArgList(newMessage);
    List_removeIndex_(args, 0);
    return IoObject_activate(v, self, locals, newMessage);
  }
  return IoObject_forward(self, locals, m);
}

IoObject *IoObject_protoPerformWithArgList(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoString *slotName = IoMessage_locals_stringArgAt_(m, locals, 0);
  IoList *args = IoMessage_locals_listArgAt_(m, locals, 1);
  List *argList = IoList_rawList(args);
  IoObject *v = IoObject_rawGetSlot_(self, slotName);
  if (v)
  {
    IoMessage *newMessage = IoMessage_newWithName_(IOSTATE, slotName);
    int i, max = List_count(argList);
    for (i=0; i < max; i++)
    { IoMessage_addCachedArg_(newMessage, LIST_AT_(argList, i)); }
    return IoObject_activate(v, self, locals, newMessage);
  }
  return IoObject_forward(self, locals, m);
}

IoObject *IoObject_protoWrite(IoObject *self, IoObject *locals, IoMessage *m)
{
  int n, max = IoMessage_argCount(m);
  IoState *state = IOSTATE;
  for (n=0; n < max; n++)
  {
    IoObject *v = IoMessage_locals_valueArgAt_(m, locals, n);
    IoMessage_locals_performOn_(state->printMessage, locals, v);
  }
  return IONIL(self);
}

inline IoObject *IoObject_initClone_(IoObject *self, 
  IoObject *locals, IoMessage *m, IoObject *newObject)
{
  IoState *state = IOSTATE;
  IoObject *initSlotValue = IoObject_rawGetSlot_(newObject, state->initString);
  if (initSlotValue) IoObject_activate(initSlotValue, newObject, locals, m);
  return newObject;
}

IoObject *IoObject_clone(IoObject *self, IoObject *locals, IoMessage *m)
{
  TagCloneFunc *func = self->tag->cloneFunc;
  IoObject *newObject = (func)(self);
  /*IoObject *newObject = IoObject_rawClone(self);*/
  return IoObject_initClone_(self, locals, m, newObject);
}

IoObject *IoObject_shallowCopy(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoObject *newObject = IoObject_new(IOSTATE); 
  /* will own slots, since IoObject_new was called */
  PHash_copy_(newObject->slots, self->slots); 
  return newObject;
}

/* -- lobby methods ---------------------------------------------- */

IoObject *IoObject_protoSet_to_(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoString *slotName  = IoMessage_locals_stringArgAt_(m, locals, 0);
  IoObject *slotValue = IoMessage_locals_valueArgAt_(m, locals, 1);
  IoObject_setSlot_to_(self, slotName, slotValue);
  return slotValue;
}

IoObject *IoObject_protoUpdateSlot_to_(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoString *slotName  = IoMessage_locals_stringArgAt_(m, locals, 0);
  IoObject *slotValue = IoMessage_locals_valueArgAt_(m, locals, 1);
  
  if (!PHash_at_(self->slots, slotName))
  {
    IoString *description =  IoString_newWithFormat_(IOSTATE, "slot %s not found. Must define slot with := before using =.", CSTRING(slotName));
    description = IoState_addLiteralString_(IOSTATE, description);
    IoState_raiseExceptionWithName_andDescription_(IOSTATE, 
        IOSTRING("IoObject.updateSlot"), description, m);
  }
  else
  { IoObject_setSlot_to_(self, slotName, slotValue); }
  return slotValue;
}


IoObject *IoObject_protoGet_(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoString *slotName = IoMessage_locals_stringArgAt_(m, locals, 0);
  return IoObject_getSlot_(self, slotName);
}

IoObject *IoObject_protoHasSlot(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoString *slotName = IoMessage_locals_stringArgAt_(m, locals, 0);
  IoObject *v = PHash_at_(self->slots, slotName);
  return (v) ? self : IONIL(self);
}

IoObject *IoObject_protoRemoveSlot(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoString *slotName = IoMessage_locals_stringArgAt_(m, locals, 0);
  PHash_removeKey_(self->slots, slotName);
  return self;
}

IoObject *IoObject_protoSlotNames(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoList *slotNames = IoList_new(IOSTATE);
  IoString *slotName = PHash_firstKey(self->slots);
  while (slotName)
  { 
    IoList_rawAdd_(slotNames, slotName);
    slotName = PHash_nextKey(self->slots);
  }
  return slotNames;
}

/* --- loops --------------------------------------------------------------*/
IoObject *IoObject_protoWhile(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoMessage_assertArgCount_receiver_(m, 2, self);
  {
    IoMessage *condition = IoMessage_rawArgAt_(m, 0);
    IoMessage *doMessage = IoMessage_rawArgAt_(m, 1);
    IoObject *result = IONIL(self);
    unsigned char c;
    IoMessage_resetStopStatus(m); 
    for (;;)
    { 
      c = !ISNIL(IoMessage_locals_performOn_(condition, locals, self));
      IoState_retainPopOne(IOSTATE);
      if (!c) break;
      
      result = IoMessage_locals_performOn_(doMessage, locals, self);
      IoState_retainPopOne(IOSTATE);
      
      if (IoMessage_isStopped(doMessage)) 
      { 
          switch (IoMessage_stopStatus(doMessage))
          {
            case MESSAGE_STOP_STATUS_RETURN:
              goto done;
            case MESSAGE_STOP_STATUS_CONTINUE:
              IoMessage_resetStopStatus(m);
          }
          if (IoMessage_stopStatus(doMessage) == MESSAGE_STOP_STATUS_BREAK)
          {
            IoMessage_resetStopStatus(m); 
            break; /* won't work inside switch statement */
          }
      }
    }
    done:
    IoState_stackRetain_(IOSTATE, result);
    return result;
  }
}

IoObject *IoObject_protoFor(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoMessage_assertArgCount_receiver_(m, 4, self);
  {
    IoMessage *indexMessage = IoMessage_rawArgAt_(m, 0);
    IoMessage *doMessage = IoMessage_rawArgAt_(m, 3);
    IoObject *result = IONIL(self);
    int i;
    IoString *slotName = IoMessage_rawMethod(indexMessage);
    IoNumber *variable = IoObject_getSlot_(locals, slotName);
    int minValue = IoMessage_locals_intArgAt_(m, locals, 1);
    int maxValue = IoMessage_locals_intArgAt_(m, locals, 2);
    IoMessage_resetStopStatus(m); 
    if (!variable || !ISNUMBER(variable))
    {
      IoObject_setSlot_to_(locals, slotName, IONUMBER(minValue));
      variable = IoObject_getSlot_(locals, slotName); /* numbers copied by value! */
    }
    if (minValue < maxValue)
    {
      for (i=minValue; i < maxValue + 1; i++)
      {
        IoNumber_rawSet(variable, i);
        result = IoMessage_locals_performOn_(doMessage, locals, self);
        IoState_retainPopOne(IOSTATE);
        if (IoMessage_isStopped(doMessage)) 
        { 
          switch (IoMessage_stopStatus(doMessage))
          {
            case MESSAGE_STOP_STATUS_RETURN:
              goto done;
            case MESSAGE_STOP_STATUS_CONTINUE:
              IoMessage_resetStopStatus(m); 
          }
          if (IoMessage_stopStatus(doMessage) == MESSAGE_STOP_STATUS_BREAK)
          {
            IoMessage_resetStopStatus(m); 
            break; /* won't work inside switch statement */
          }
        }
      }
    }
    else
    {
      for (i=minValue; i > maxValue - 1; i--)
      { 
        IoNumber_rawSet(variable, i);
        result = IoMessage_locals_performOn_(doMessage, locals, self);
        IoState_retainPopOne(IOSTATE);
        if (IoMessage_isStopped(doMessage)) 
        { 
          switch (IoMessage_stopStatus(doMessage))
          {
            case MESSAGE_STOP_STATUS_RETURN:
              goto done;
            case MESSAGE_STOP_STATUS_CONTINUE:
              IoMessage_resetStopStatus(m); 
          }
          if (IoMessage_stopStatus(doMessage) == MESSAGE_STOP_STATUS_BREAK)
          {
            IoMessage_resetStopStatus(m); 
            break; /* won't work inside switch statement */
          }
        }
      }
    }
    done:
    if (result) 
    {
      IoState_stackRetain_(IOSTATE, result); 
      return result;
    }
    return IONIL(self);
  }
}

IoObject *IoObject_protoIf(IoObject *self, IoObject *locals, IoMessage *m)
{
  int argCount = IoMessage_argCount(m);
  IOASSERT(argCount > 0, "Error: 'if' needs at least 1 argument\n");
  
  if (argCount == 1)
  { return IoMessage_locals_valueArgAt_(m, locals, 0); }

  if (!ISNIL(IoMessage_locals_valueArgAt_(m, locals, 0)))
  { return IoMessage_locals_valueArgAt_(m, locals, 1); }
  else if (argCount == 3)
  { return IoMessage_locals_valueArgAt_(m, locals, 2); }
  
  return IONIL(self);
}

IoObject *IoObject_collectGarbage(IoObject *self, IoObject *locals, IoMessage *m)
{ return IONUMBER(IoState_collectGarbage(IOSTATE)); }

IoObject *IoObject_return(IoObject *self, IoObject *locals, IoMessage *m)
{ 
  IoObject *v = IoMessage_locals_valueArgAt_(m, locals, 0);
  IoMessage_return_(m, v);
  return self;
}

IoObject *IoObject_break(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoObject *v = IoMessage_locals_valueArgAt_(m, locals, 0);
  IoMessage_break(m, v);
  return self;
}

IoObject *IoObject_continue(IoObject *self, IoObject *locals, IoMessage *m)
{ IoMessage_continue(m); return self; }

IoObject *IoObject_exit(IoObject *self, IoObject *locals, IoMessage *m)
{ IoState_exit(IOSTATE); return self; }

IoObject *IoObject_protoForward_(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoState_error_description_(IOSTATE, m, 
      "Io.Object.doesNotRespond", "%s does not respond to message '%s'", 
      IoObject_name(self), CSTRING(IoMessage_rawMethod(m)));
  return IONIL(self);
}

IoObject *IoObject_super(IoObject *self, IoObject *locals, IoMessage *m)
{
  /* super - The hairiest bit of the runtime.
   * We need to call the argument message on the
   * proto of the object that is holding the current block.
   * Note: Only the proto chain is followed.
   *
   * Important to remember that super gets called on the locals, 
   * so we need to find the block's proto(the target object) first.
   */
  IoState *state = IOSTATE;
  if (IoMessage_argCount(m) != 0) 
  {
    IoMessage *superM = IoMessage_rawArgAt_(m, 0);
    IoBlock *block = PHash_at_(self->slots, state->blockString); /* self == locals */
    IoBlock *objSelf = PHash_at_(locals->slots, state->selfString); /* self == locals */
    if (block)
    {
      IoObject *object = self->proto;

      IoObject *objectWithBlock = IoObject_objectWithSlotValue_(object, block);
      if (objectWithBlock)
      {
        IoObject *proto = objectWithBlock->proto;

        if (proto)
        {
            IoObject *slotValue = IoObject_rawGetSlot_(proto, IoMessage_rawMethod(superM)); 
            if (slotValue) return IoObject_activate(slotValue, objSelf, locals, superM);
        }
      }
    }
  }
  return IONIL(self);
}

IoObject *IoObject_rawDoString_label_(IoObject *self, IoString *string, IoString *label)
{
  if(!ISSTRING(string))
  { IoState_error_description_(IOSTATE, 0x0, "Io.Object", "IoObject_rawDoString_label_ requires a string argument"); }
  
  {
    IoMessage *newMessage = IoMessage_newFromText_label_(IOSTATE,
      CSTRING(string), CSTRING(label));
    IoState *state = (IOSTATE);
    IoObject *debugParser = IoObject_getSlot_(state->lobby, state->debugParserString);
    int debug = !ISNIL(debugParser);
    if (debug)
    {
      IoState_print_(state, "parsed: "); 
      IoMessage_print((IoMessage *)newMessage); 
      IoState_print_(state, "\n"); 
    }
    if (newMessage) return IoMessage_locals_performOn_(newMessage, self, self); 
    IoState_print_(IOSTATE, "no message found for doRawString\n");
    return IONIL(self);
  }
}

IoObject *IoObject_doMessage(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoMessage *aMessage = IoMessage_locals_messageArgAt_(m, locals, 0);
  return IoMessage_locals_performOn_(aMessage, self, self);
}

IoObject *IoObject_doString(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoString *string = IoMessage_locals_stringArgAt_(m, locals, 0);
  IoObject *result;
  IoState_pushRetainPool(IOSTATE);
  result = IoObject_rawDoString_label_(self, string, IOSTRING("doString"));
  IoState_popRetainPool(IOSTATE);
  IoState_stackRetain_(IOSTATE, result);
  return result;
}

#ifndef IO_OS_L4
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

IoObject *IoObject_doFile(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoString *path = IoMessage_locals_stringArgAt_(m, locals, 0);
  IoFile *file = IoFile_newWithPath_(IOSTATE, path);
  IoString *string = (IoString *)IoFile_asString(file, locals, m);
  
  IoList *argsList = IoList_new(IOSTATE);
  int argn = 1;
  IoObject *arg = IoMessage_locals_valueArgAt_(m, locals, argn);
  while (arg && !ISNIL(arg))
  {
    IoList_rawAdd_(argsList, arg);
    argn++;
    arg = IoMessage_locals_valueArgAt_(m, locals, argn);
  }
  if (IoList_rawCount(argsList))
  { IoObject_setSlot_to_(self, IOSTRING("args"), argsList); }
  
  return IoObject_rawDoString_label_(self, string, path);
}
#endif

IoObject *IoObject_equals(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoObject *other = IoMessage_locals_valueArgAt_(m, locals, 0);
  return IoObject_compare(self, other) == 0 ? self : IONIL(self);
}

IoObject *IoObject_notEquals(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoObject *other = IoMessage_locals_valueArgAt_(m, locals, 0);
  return IoObject_compare(self, other) != 0 ? self : IONIL(self);
}

/* --- exceptions -------------------------------------------------------*/
#if !defined(__SYMBIAN32__)
IoObject *IoObject_catchException(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoObject *result = 0x0;
  IoMessage_assertArgCount_receiver_(m, 3, self);
  {
    IoString *exceptionName = IoMessage_locals_stringArgAt_(m, locals, 0);
    IoExceptionCatch *eCatch = IoState_pushExceptionCatchWithName_(IOSTATE, 
       CSTRING(exceptionName));

    if (!IOEXCEPTIONCATCH_SETJMP(*(eCatch->savedStack)))
    { 
      result = IoMessage_locals_valueArgAt_(m, locals, 1);
      IoState_popExceptionCatch_(IOSTATE, eCatch);
    }
    else
    { 
      IoString *d = eCatch->caughtDescription;
      if (!d) d = IOSTRING("<no description>");
      IoObject_setSlot_to_(locals, IOSTRING("exceptionName"), eCatch->caughtName);
      IoObject_setSlot_to_(locals, IOSTRING("exceptionDescription"), d); 
      IoState_popExceptionCatch_(IOSTATE, eCatch);
      result = IoMessage_locals_valueArgAt_(m, locals, 2);
    }
  }
  return result?result:IONIL(self);
}
#endif

IoObject *IoObject_raiseException(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoMessage_assertArgCount_receiver_(m, 2, self);
  {
    IoString *exceptionName = IoMessage_locals_stringArgAt_(m, locals, 0);
    IoString *exceptionDescription = IoMessage_locals_stringArgAt_(m, locals, 1);
    IoState_raiseExceptionWithName_andDescription_(IOSTATE, exceptionName, exceptionDescription, m);
  }
  return self;
}

/* --- actors ---------------------------------------- */

IoObject *IoObject_actorYield(IoObject *self, IoObject *locals, IoMessage *m)
{ 
  IoObject_yield(((IoState *)IOSTATE)->currentActor);
  return self;
}

IoObject *IoObject_actorResume(IoObject *self, IoObject *locals, IoMessage *m)
{ IoObject_resume(self); return self; }

IoObject *IoObject_actorPause(IoObject *self, IoObject *locals, IoMessage *m)
{ IoObject_pause(self); return self; }

IoObject *IoObject_memorySizeOfState(IoObject *self, IoObject *locals, IoMessage *m)
{ return IONUMBER(IoState_memorySize(IOSTATE)); }

IoObject *IoObject_compactState(IoObject *self, IoObject *locals, IoMessage *m)
{ IoState_compact(IOSTATE); return self; }

IoObject *IoObject_getenv(IoObject *self, IoObject *locals, IoMessage *m)
{ 
  IoString *key = IoMessage_locals_stringArgAt_(m, locals, 0);
  char *s = getenv(CSTRING(key));
  if (!s) return ((IoState *)IOSTATE)->ioNil;
  return IoState_stringWithCString_(IOSTATE, s);
}

IoObject *IoObject_foreach(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoMessage_assertArgCount_receiver_(m, 3, self);
  {
    IoMessage *keyMessage   = IoMessage_rawArgAt_(m, 0);
    IoMessage *valueMessage = IoMessage_rawArgAt_(m, 1);
    IoMessage *doMessage    = IoMessage_rawArgAt_(m, 2);
    IoObject *key = PHash_firstKey(self->slots);
    IoObject *result = IONIL(self);
    while (key)
    {
      IoObject *value = PHash_at_(self->slots, key);
      IoObject_setSlot_to_(locals, IoMessage_rawMethod(keyMessage), key);
      IoObject_setSlot_to_(locals, IoMessage_rawMethod(valueMessage), value);
      IoState_pushRetainPool(IOSTATE);
      result = IoMessage_locals_performOn_(doMessage, locals, locals);
      IoState_popRetainPool(IOSTATE);
      if (IoMessage_isStopped(doMessage)) 
      { 
          switch (IoMessage_stopStatus(doMessage))
          {
            case MESSAGE_STOP_STATUS_RETURN:
              return result;
            case MESSAGE_STOP_STATUS_CONTINUE:
              IoMessage_resetStopStatus(m); 
          }
          if (IoMessage_stopStatus(doMessage) == MESSAGE_STOP_STATUS_BREAK)
          {
            IoMessage_resetStopStatus(m); 
            break; /* won't work inside switch statement */
          }
      }
      key = PHash_nextKey(self->slots);
    }
    IoState_stackRetain_(IOSTATE, result);
    return result;
  }
}

/*IoObject *IoObject_try(IoObject *volatile self, IoObject *locals, IoMessage *m)*/
IoObject *IoObject_try(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoMessage_assertArgCount_receiver_(m, 1, self);
  {
    IoState *state = IOSTATE;
    IoMessage *firstArg = IoMessage_rawArgAt_(m, 0);
    IoObject *slotValue = IoObject_rawGetSlot_(self, IoMessage_rawMethod(firstArg));
    if (!slotValue) { return state->ioNil; }
    return IoMessage_locals_performOn_(firstArg, locals, self);
  }
}

IoObject *IoObject_subtract(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoNumber *num = IoMessage_locals_numberArgAt_(m, locals, 0);
  return IONUMBER(- IoNumber_asDouble(num));
}

IoObject *IoObject_system(IoObject *self, IoObject *locals, IoMessage *m)
{
  IoString *s = IoMessage_locals_stringArgAt_(m, locals, 0);
  int result = system(CSTRING(s));
  return IONUMBER(result);
}

IoObject *IoObject_self(IoObject *self, IoObject *locals, IoMessage *m)
{ return self; }

IoObject *IoObject_compileString(IoObject *self, IoObject *locals, IoMessage *m)
{ 
  volatile IoString *string = IoMessage_locals_stringArgAt_(m, locals, 0);
  volatile IoString *label = IoMessage_rawLabel(m);
  volatile IoObject *result = IONIL(self);

  #if !defined(__SYMBIAN32__)
  volatile IoExceptionCatch *eCatch = IoState_pushExceptionCatchWithName_(IOSTATE, "");
  if (!IOEXCEPTIONCATCH_SETJMP(*(eCatch->savedStack)))
  {
  #endif

  if (IoMessage_argCount(m)>1) { label = IoMessage_locals_stringArgAt_(m, locals, 1); }
  
  result = IoMessage_newFromText_label_(IOSTATE,
      CSTRING((IoString *)string), CSTRING((IoString *)label));
      
  #if !defined(__SYMBIAN32__)
  }
  else
  { result = (eCatch->caughtDescription); }
  IoState_popExceptionCatch_(IOSTATE, (IoExceptionCatch *)eCatch);
  #endif
  
  return (IoObject *)result;
}

/* --- stuff that used to be in IoValue ----------------------- */
/* --- message callbacks -------------------------------------- */

const char *IoObject_name(IoObject *self) 
{ return IoTag_name(self->tag); }

int IoObject_compare(IoObject *self, IoObject *v)
{
  if (self == v) return 0;
  if (self->tag->compareFunc) { return (self->tag->compareFunc)(self, v); }
  else
  {
    int d = -((ptrdiff_t)self->tag - (ptrdiff_t)v->tag);
    if (d == 0) { return ((ptrdiff_t)self) - ((ptrdiff_t)v); }
    return d;
  }
}

int IoObject_sortCompare(IoObject **self, IoObject **v) { return IoObject_compare(*self, *v); }

size_t IoObject_memorySize(IoObject *self)
{ return (self->tag->memorySizeFunc) ? (self->tag->memorySizeFunc)(self) : 0; }

void IoObject_compact(IoObject *self)
{ if (self->tag->compactFunc) { (self->tag->compactFunc)(self); } }


/* -- lobby methods ---------------------------------------------- */

IoObject *IoObject_memorySizeMethod(IoObject *self, IoObject *locals, IoMessage *m)
{ return IONUMBER(IoObject_memorySize(self)); }

IoObject *IoObject_compactMethod(IoObject *self, IoObject *locals, IoMessage *m)
{ IoObject_compact(self); return self; }

IoObject *IoObject_type(IoObject *self, IoObject *locals, IoMessage *m)
{ return IOSTRING((char *)IoObject_name(self)); }

IoObject *IoObject_lobbyPrint(IoObject *self, IoObject *locals, IoMessage *m)
{ IoObject_print(self); return IONIL(self); }

void IoObject_defaultPrint(IoObject *self)
{ IoState_print_(IOSTATE, "%s_%p", IoObject_name(self), self); }

void IoObject_print(IoObject *self) 
{ 
  if (self->tag->printFunc)
  { (self->tag->printFunc)(self); }
  else
  { IoObject_defaultPrint(self); }
}

IoObject *IoObject_evalArg(IoObject *self, IoObject *locals, IoMessage *m)
{
  IOASSERT(IoMessage_argCount(m) > 0, "Error: 'then' needs an argument\n");
  /* eval the arg and return a non-Nil so an attached else() won't get performed */
  IoMessage_locals_valueArgAt_(m, locals, 0);
  return (((IoState *)IOSTATE)->ioNop);
}

IoObject *IoObject_nop(IoObject *self, IoObject *locals, IoMessage *m)
{ /* do nothing */ return self; }


IoObject *IoObject_and(IoObject *self, IoObject *locals, IoMessage *m)
{ 
  IoObject *other = IoMessage_locals_valueArgAt_(m, locals, 0);
  return ISNIL(other) ? IONIL(self) : self; 
}

IoObject *IoObject_or(IoObject *self, IoObject *locals, IoMessage *m)
{ return self; }

IoObject *IoObject_isLessThan_(IoObject *self, IoObject *locals, IoMessage *m)
{ 
  IoObject *v = IoMessage_locals_valueArgAt_(m, locals, 0);
  return IoObject_compare(self, v) < 0 ? self : IONIL(self);
}

IoObject *IoObject_isLessThanOrEqualTo_(IoObject *self, IoObject *locals, IoMessage *m)
{ 
  IoObject *v = IoMessage_locals_valueArgAt_(m, locals, 0);
  return IoObject_compare(self, v) <= 0 ? self : IONIL(self);
}

IoObject *IoObject_isGreaterThan_(IoObject *self, IoObject *locals, IoMessage *m)
{ 
  IoObject *v = IoMessage_locals_valueArgAt_(m, locals, 0);
  return IoObject_compare(self, v) > 0 ? self : IONIL(self);
}

IoObject *IoObject_isGreaterThanOrEqualTo_(IoObject *self, IoObject *locals, IoMessage *m)
{ 
  IoObject *v = IoMessage_locals_valueArgAt_(m, locals, 0);
  return IoObject_compare(self, v) >= 0 ? self : IONIL(self);
}

IoObject *IoObject_uniqueId(IoObject *self, IoObject *locals, IoMessage *m)
{ return IoNumber_newWithDouble_(IOSTATE, (double)((long)self)); }

IoObject *IoObject_do(IoObject *self, IoObject *locals, IoMessage *m)
{
  if (IoMessage_argCount(m) != 0) 
  {
    IoMessage *argMessage = IoMessage_rawArgAt_(m, 0);
    IoMessage_locals_performOn_(argMessage, self, self);
  }
  return self;
}

IoObject *IoObject_isNil(IoObject *self, IoObject *locals, IoMessage *m)
{ return IONIL(self); }

IoObject *IoObject_ifNil(IoObject *self, IoObject *locals, IoMessage *m)
{ return self; }

IoObject *IoObject_ifTrue(IoObject *self, IoObject *locals, IoMessage *m)
{ return IoMessage_locals_valueArgAt_(m, locals, 0); }

IoObject *IoObject_ifFalse(IoObject *self, IoObject *locals, IoMessage *m)
{ return IONIL(self); }

/* --- inline these --- */

void IoObject_mark(IoObject *self)
{
  if (self->mark.ownsSlots)
  { PHash_doOnKeyAndValue_(self->slots, (PHashDoCallback *)IoObject_makeGrayIfWhite); }
  else
  { if (self->proto) IoObject_makeGrayIfWhite(self->proto); }
  
  if (self->coro) IoCoroutine_mark(self->coro); 
  
  if (self->mark.color == IOMARK_GRAY)
  {
    TagFreeFunc *func = self->tag->markFunc;
    if (func) (func)(self);
    IoState_makeBlackFromGray_(IOSTATE, self);
  }
}

void IoObject_makeGrayIfWhite(IoObject *self)
{
  if (self->mark.color == IOMARK_WHITE) 
  { IoState_makeGrayFromWhite_(IOSTATE, self); }
}

IoObject *IoObject_addingRef_(IoObject *self, IoObject *ref)
{
  unsigned char c = self->mark.color;
  if (ref && ((c == IOMARK_BLACK) || (c == IOMARK_GRAY)))
  { IoObject_makeGrayIfWhite(ref); }
  ref->mark.isReferenced = 1;
  return ref;
}

char *IoObject_markColorName(IoObject *self)
{
  unsigned char c = self->mark.color;
  if (c == IOMARK_WHITE) return "white";
  if (c == IOMARK_GRAY)  return "gray";
  if (c == IOMARK_BLACK) return "black";
  return "off-white";
}


