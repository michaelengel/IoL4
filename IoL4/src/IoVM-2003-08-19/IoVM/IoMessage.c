/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#include "IoObject.h"
#define IOMESSAGE_C
#include "IoMessage.h"
#undef IOMESSAGE_C
#include "IoNil.h"
#include "IoString.h"
#include "IoNumber.h"
#include "IoState.h"
#include "IoCFunction.h"
#include "IoBlock.h"
#include "IoList.h"
#ifndef IO_OS_L4
#include "IoDate.h"
#endif
#include "IoBuffer.h"
#include "IoMap.h"
#include <ctype.h>
#include <stdarg.h>
#include "IoMessage_parser.h"

#define DATA(self) ((IoMessageData *)self->data)

IoTag *IoMessage_tag(void *state)
{
  IoTag *tag = IoTag_newWithName_("Message");
  tag->state = state;
  tag->freeFunc  = (TagFreeFunc *)IoMessage_free;
  tag->cloneFunc = (TagCloneFunc *)IoMessage_rawClone;
  tag->markFunc  = (TagMarkFunc *)IoMessage_mark;
  return tag;
}

IoMessage *IoMessage_proto(void *state)
{
  IoObject *self = IoObject_new(state);
  IoMessageData *d;
  self->data = calloc(1, sizeof(IoMessageData));
  d = self->data;
  /*
  printf("message proto %p\n", self);
  printf("message data %p\n", d);
  */
  self->tag = IoMessage_tag(state);  
  d->args = List_new();
  d->method = IOSTRING("[unassigned method]");
  d->label = IOSTRING("[internal]");
  d->charNumber = -1;
  d->lineNumber = -1;
  IoState_registerProtoWithFunc_(state, self, IoMessage_proto);
    
  IoObject_addMethod_(self, IOSTRING("clone"), IoMessage_clone);
  IoObject_addMethod_(self, IOSTRING("name"), IoMessage_protoName);
  IoObject_addMethod_(self, IOSTRING("setName"), IoMessage_protoName_);
  IoObject_addMethod_(self, IOSTRING("nextMessage"), IoMessage_nextMessage);
  IoObject_addMethod_(self, IOSTRING("setNextMessage"), IoMessage_nextMessage_);
  IoObject_addMethod_(self, IOSTRING("attachedMessage"), IoMessage_attachedMessage);
  IoObject_addMethod_(self, IOSTRING("setAttachedMessage"), IoMessage_attachedMessage_);
  IoObject_addMethod_(self, IOSTRING("argAt"), IoMessage_argAt);
  IoObject_addMethod_(self, IOSTRING("arguments"), IoMessage_arguments);
  IoObject_addMethod_(self, IOSTRING("setArguments"), IoMessage_arguments_);
  IoObject_addMethod_(self, IOSTRING("cachedResult"), IoMessage_cachedResult);
  IoObject_addMethod_(self, IOSTRING("setCachedResult"), IoMessage_setCachedResult);
  IoObject_addMethod_(self, IOSTRING("code"), IoMessage_descriptionString);
  IoObject_addMethod_(self, IOSTRING("doInContext"), IoMessage_doInContext);
  IoObject_addMethod_(self, IOSTRING("fromString"), IoMessage_fromString);
  IoObject_addMethod_(self, IOSTRING("argsEvaluatedIn"), IoMessage_argsEvaluatedIn);
  return self;
}

IoMessage *IoMessage_rawClone(IoMessage *self) 
{ 
  IoObject *child = IoObject_rawClonePrimitive(self);
  IoMessageData *d = self->data;
  child->data = cpalloc(d, sizeof(IoMessageData));  
  DATA(child)->args = List_clone(d->args);
  return child; 
}

IoMessage *IoMessage_new(void *state)
{
  IoObject *proto = IoState_protoWithInitFunction_(state, IoMessage_proto);
  return IoMessage_rawClone(proto);
}

IoMessage *IoMessage_deepCopyOf_(IoMessage *self)
{
  IoMessage *child = IoMessage_new(IOSTATE);
  int i;
  /*printf("deep copying: %s\n", ByteArray_asCString(IoMessage_description(self)));*/
  for (i=0; i< IoMessage_argCount(self); i++)
  { List_add_(DATA(child)->args, IoMessage_deepCopyOf_(LIST_AT_(DATA(self)->args, i))); }
  DATA(child)->method = DATA(self)->method;
  IoMessage_cachedResult_(child, (IoObject *)DATA(self)->cachedResult);
  if (DATA(self)->nextMessage) DATA(child)->nextMessage = IoMessage_deepCopyOf_(DATA(self)->nextMessage);
  if (DATA(self)->attachedMessage) DATA(child)->attachedMessage = IoMessage_deepCopyOf_(DATA(self)->attachedMessage);
  /*printf("deep copy result: %s\n", ByteArray_asCString(IoMessage_description(child)));*/
  return child;
}

IoMessage *IoMessage_newWithName_(void *state, IoString *symbol)
{
  IoMessage *self = IoMessage_new(state);
  DATA(self)->method = IOREF(symbol);
  return self;
}

IoMessage *IoMessage_newWithName_label_(void *state, IoString *symbol, IoString *label)
{
  IoMessage *self = IoMessage_new(state);
  DATA(self)->method = IOREF(symbol);
  DATA(self)->label = IOREF(label);
  return self;
}


IoMessage *IoMessage_newWithName_returnsValue_(void *state, IoString *symbol, IoObject *v)
{
  IoMessage *self = IoMessage_new(state);
  DATA(self)->method = IOREF(symbol);
  DATA(self)->cachedResult = IOREF(v);
  return self;
}

/*
IoMessage *IoMessage_newWithName_andCachedArgs_(void *state, void *methodName, ...)
{
  IoMessage *self = IoMessage_newWithName_(state, methodName);
  void *arg;
  va_list ap;
  va_start(ap, methodName);
  arg = va_arg(ap, void *);
  while (arg)
  {
    IoMessage_addCachedArg_(self, arg);
    arg = va_arg(ap, void *);
  }
  va_end(ap);
  return self;
}
*/

void IoMessage_mark(IoMessage *self)
{
  if (DATA(self)->method) { IoObject_makeGrayIfWhite(DATA(self)->method); }
  List_do_(DATA(self)->args, (ListDoCallback *)IoObject_makeGrayIfWhite);
  if (DATA(self)->cachedResult)    { IoObject_makeGrayIfWhite(DATA(self)->cachedResult); } 
  if (DATA(self)->attachedMessage) { IoObject_makeGrayIfWhite((IoObject *)DATA(self)->attachedMessage); }
  if (DATA(self)->nextMessage)     { IoObject_makeGrayIfWhite((IoObject *)DATA(self)->nextMessage); }
  if (DATA(self)->label)           { IoObject_makeGrayIfWhite((IoObject *)DATA(self)->label); }
}

void IoMessage_free(IoMessage *self) 
{ 
  /*printf("freeing message %p\n", self);*/
  List_free(DATA(self)->args);
  free(self->data);
}

List *IoMessage_args(IoMessage *self) { return DATA(self)->args; }

void IoMessage_cachedResult_(IoMessage *self, IoObject *v)
{ 
  if (v)
  {
    if (ISNUMBER(v)) 
    {
      IoObject *cr = DATA(self)->cachedResult;
      if (cr && ISNUMBER(cr))
      { 
        IoNumber_rawSet(cr, IoNumber_asDouble(v)); 
        return; 
      }
      v = IoNumber_newCopyOf_(v);
    }
    DATA(self)->cachedResult = IOREF(v); 
  }
  else
  { DATA(self)->cachedResult = 0x0; }
}

void IoMessage_label_(IoMessage *self, IoString *ioString) /* sets label for children too */
{
  DATA(self)->label = IOREF(ioString);
  List_do_with_(DATA(self)->args, (ListDoWithCallback *)IoMessage_label_, ioString);
  if (DATA(self)->attachedMessage) IoMessage_label_(DATA(self)->attachedMessage, ioString); 
  if (DATA(self)->nextMessage) IoMessage_label_(DATA(self)->nextMessage, ioString); 
}

int IoMessage_rawLineNumber(IoMessage *self)
{ return DATA(self)->lineNumber; }

int IoMessage_rawCharNumber(IoMessage *self)
{ return DATA(self)->charNumber; }

List *IoMessage_rawArgList(IoMessage *self)
{ return DATA(self)->args; }

unsigned char IoMessage_isNotCached(IoMessage *self)
{ return !(DATA(self)->cachedResult); }

unsigned char IoMessage_needsEvaluation(IoMessage *self)
{
  List *args = DATA(self)->args;
  int a = List_detect_(args, (ListDetectCallback *)IoMessage_isNotCached) != 0x0;
  if (a) return 1;
  if (DATA(self)->attachedMessage && IoMessage_needsEvaluation(DATA(self)->attachedMessage)) return 1;
  if (DATA(self)->nextMessage     && IoMessage_needsEvaluation(DATA(self)->nextMessage)) return 1;
  return 0;
}

void IoMessage_addCachedArg_(IoMessage *self, IoObject *v)
{
  IoMessage *m = IoMessage_new(IOSTATE);
  IoMessage_cachedResult_(m, v);
  IoMessage_addArg_(self, m);
}

void IoMessage_setCachedArg_to_(IoMessage *self, int n, IoObject *v)
{
  IoMessage *arg;
  while (!(arg = List_at_(DATA(self)->args, n)))
  { IoMessage_addArg_(self, IoMessage_new(IOSTATE)); }
  IoMessage_cachedResult_(arg, v);
}

void IoMessage_setCachedArg_toInt_(IoMessage *self, int n, int anInt)
{
  /* optimized to avoid creating a number unless necessary */
  IoMessage *arg = 0x0;
  while (!(arg = List_at_(DATA(self)->args, n)))
  { List_add_(DATA(self)->args, IOREF(IoMessage_new(IOSTATE))); }
  
  if (DATA(arg)->cachedResult && ISNUMBER(DATA(arg)->cachedResult))
  { IoNumber_rawSet(DATA(arg)->cachedResult, (double)anInt); }
  else
  { DATA(arg)->cachedResult = IOREF(IONUMBER(anInt)); }
}

/* --- perform -------------------------------------------------------- */

IoObject *IoMessage_doInContext(IoMessage *self, IoObject *locals, IoMessage *m)
{
  IoObject *context = IoMessage_locals_objectArgAt_(m, (IoObject *)locals, 0);
  return IoMessage_locals_performOn_(self, context, context);
}

IoObject *IoMessage_locals_performOn_(IoMessage *self, IoObject *locals, IoObject *target)
{
  IoObject *result = NULL;
  IoState *state = IOSTATE;
  IoMessage *inner;
  IoMessage *outer;

  state->stopStatus = MESSAGE_STOP_STATUS_NORMAL;
  state->returnValue = IONIL(self);

  IoState_pushRetainPool(state);

  for (outer = self; outer; outer = DATA(outer)->nextMessage)
  {
    for (inner = outer; inner; inner = DATA(inner)->attachedMessage)
    {
      if (DATA(inner)->cachedResult)
      { result = DATA(inner)->cachedResult; }
      else
      {
        /* No cached value -- must perform the message. */
        IoState_pushRetainPool(state);
        result = (target->tag->performFunc)((void *)target, (void *)locals, (void *)inner);
        IoState_popRetainPool(state);
        IoState_stackRetain_(state, result);

        if (state->stopStatus != MESSAGE_STOP_STATUS_NORMAL)
        { result = state->returnValue; goto stop; }
      }

      /* The next attachedMessage uses this result as its target. */
      target = result;
    }

    /* The next nextMessage targets the local environment. */
    target = locals;
  }

stop:
  IoState_popRetainPool(state);
  IoState_stackRetain_(state, result);
  return result;
}

int IoMessage_argCount(IoMessage *self) { return List_count(DATA(self)->args); }

void IoMessage_assertArgCount_receiver_(IoMessage *self, int n, IoObject *receiver)
{
  if (List_count(DATA(self)->args) < n)
  {
    IoState_error_description_(IOSTATE, self, 
      "Io.missingArguments",  "[%s %s] requires %i arguments\n",
      IoObject_name(receiver), CSTRING(DATA(self)->method), n);
  }
}

IoObject *IoMessage_locals_valueArgAt_(IoMessage *self, IoObject *locals, int n)
{
  IoMessage *m = (IoMessage *)List_at_(DATA(self)->args, n);
  if (m) 
  { 
    IoObject *result = IoMessage_locals_performOn_(m, locals, locals); 
    IoState_stackRetain_(IOSTATE, result);
    return result;
  }
  return IONIL(self);
}

void IoMessage_locals_numberArgAt_errorForType_(
  IoMessage *self, IoObject *locals, int n, const char *typeName)
{
  IoObject *v = IoMessage_locals_valueArgAt_(self, locals, n);
  IoState_error_description_(IOSTATE, self, 
      "Io.CFunction.invalidArgument", "argument %i to method '%s' must be a %s, not a '%s'",
      n, CSTRING(DATA(self)->method), typeName, IoObject_name(v));
}

IoObject *IoMessage_locals_numberArgAt_(IoMessage *self, IoObject *locals, int n)
{
  IoObject *v = IoMessage_locals_valueArgAt_(self, locals, n);
  if (!ISNUMBER(v)) IoMessage_locals_numberArgAt_errorForType_(self, locals, n, "Number");
  return v;
}

int IoMessage_locals_intArgAt_(IoMessage *self, IoObject *locals, int n)
{ return IoNumber_asInt(IoMessage_locals_numberArgAt_(self, locals, n)); }

long IoMessage_locals_longArgAt_(IoMessage *self, IoObject *locals, int n)
{ return IoNumber_asLong(IoMessage_locals_numberArgAt_(self, locals, n)); }

double IoMessage_locals_doubleArgAt_(IoMessage *self, IoObject *locals, int n)
{ return IoNumber_asDouble(IoMessage_locals_numberArgAt_(self, locals, n)); }

IoObject *IoMessage_locals_stringArgAt_(IoMessage *self, IoObject *locals, int n)
{
  IoObject *v = IoMessage_locals_valueArgAt_(self, locals, n);
  if (!ISSTRING(v)) IoMessage_locals_numberArgAt_errorForType_(self, locals, n, "String");
  return v;
}

IoObject *IoMessage_locals_stringOrBufferArgAt_(IoMessage *self, IoObject *locals, int n)
{
  IoObject *v = IoMessage_locals_valueArgAt_(self, locals, n);
  if (ISNUMBER(v)) return IoNumber_asString((IoNumber *)v, (IoObject *)locals, self);
  if (!ISSTRING(v) && !ISBUFFER(v)) IoMessage_locals_numberArgAt_errorForType_(self, locals, n, "String or Buffer");
  return v;
}

IoObject *IoMessage_locals_bufferArgAt_(IoMessage *self, IoObject *locals, int n)
{
  IoObject *v = IoMessage_locals_valueArgAt_(self, locals, n);
  if (!ISBUFFER(v)) IoMessage_locals_numberArgAt_errorForType_(self, locals, n, "Buffer");
  return v;
}

IoObject *IoMessage_locals_blockArgAt_(IoMessage *self, IoObject *locals, int n)
{
  IoObject *v = IoMessage_locals_valueArgAt_(self, locals, n);
  if (!ISBLOCK(v)) IoMessage_locals_numberArgAt_errorForType_(self, locals, n, "Block");
  return v;
}

#ifndef IO_OS_L4
IoObject *IoMessage_locals_dateArgAt_(IoMessage *self, IoObject *locals, int n)
{
  IoObject *v = IoMessage_locals_valueArgAt_(self, locals, n);
  if (!ISDATE(v)) IoMessage_locals_numberArgAt_errorForType_(self, locals, n, "Date");
  return v;
}
#endif

IoObject *IoMessage_locals_messageArgAt_(IoMessage *self, IoObject *locals, int n)
{
  IoObject *v = IoMessage_locals_valueArgAt_(self, locals, n);
  if (!ISMESSAGE(v)) IoMessage_locals_numberArgAt_errorForType_(self, locals, n, "Message");
  return v;
}

IoObject *IoMessage_locals_listArgAt_(IoMessage *self, IoObject *locals, int n)
{
  IoObject *v = IoMessage_locals_valueArgAt_(self, locals, n);
  if (!ISLIST(v)) IoMessage_locals_numberArgAt_errorForType_(self, locals, n, "List");
  return v;
}

IoObject *IoMessage_locals_mapArgAt_(IoMessage *self, IoObject *locals, int n)
{
  IoObject *v = IoMessage_locals_valueArgAt_(self, locals, n);
  if (!ISMAP(v)) IoMessage_locals_numberArgAt_errorForType_(self, locals, n, "Buffer");
  return v;
}

IoObject *IoMessage_locals_objectArgAt_(IoMessage *self, IoObject *locals, int n)
{ return IoMessage_locals_valueArgAt_(self, locals, n); }

/* --- printing --------------------------------------------------------- */

void IoMessage_print(IoMessage *self)
{
  ByteArray *ba = IoMessage_description(self);
  IoState_print_(IOSTATE, ByteArray_asCString(ba));
  ByteArray_free(ba);
}

void IoMessage_printWithReturn(IoMessage *self)
{
  IoMessage_print(self);
  IoState_print_(IOSTATE, "\n");
}

ByteArray *IoMessage_description(IoMessage *self)
{
  ByteArray *ba = ByteArray_new();
  IoMessage_appendDescriptionTo_follow_(self, ba, 1);
  return ba;
}

ByteArray *IoMessage_descriptionJustSelfAndArgs(IoMessage *self)
{
  ByteArray *ba = ByteArray_new();
  IoMessage_appendDescriptionTo_follow_(self, ba, 0);
  return ba;
}

void IoMessage_appendDescriptionTo_follow_(IoMessage *self, ByteArray *ba, int follow)
{
  if (DATA(self)->cachedResult) 
  {
    IoState *state = IOSTATE;
    IoObject *debugParser = IoObject_getSlot_(state->lobby, IOSTRING("debugParser"));
    if (!ISNIL(debugParser)) ByteArray_appendAndEscapeCString_(ba, "#");
  }
  ByteArray_appendAndEscapeCString_(ba, CSTRING(DATA(self)->method));
  {
    int i;
    int max = List_count(DATA(self)->args);
    if (max>0) 
    {
      ByteArray_appendCString_(ba, "(");
      for (i=0; i<max; i++)
      {
        IoMessage *arg = List_at_(DATA(self)->args, i);
        IoMessage_appendDescriptionTo_follow_(arg, ba, 1);
        if (i!=max-1) { ByteArray_appendCString_(ba, ", "); }
      }
      ByteArray_appendCString_(ba, ")");
    }
  }
  if (!follow) return;
  if (DATA(self)->attachedMessage) 
  { 
    ByteArray_appendCString_(ba, " "); 
    IoMessage_appendDescriptionTo_follow_(DATA(self)->attachedMessage, ba, 1); 
  }
  if (DATA(self)->nextMessage) 
  { 
    ByteArray_appendCString_(ba, ";\n"); 
    /*
    ByteArray_appendCString_(ba, "["); 
    ByteArray_appendCString_(ba, CSTRING(DATA(self)->method)); 
    ByteArray_appendCString_(ba, "] "); 
    */
    IoMessage_appendDescriptionTo_follow_(DATA(self)->nextMessage, ba, 1); 
  }
}

/* --- flow control ------------------------------------------------------- */
/*
void IoMessage_end(IoMessage *self)
{ ((IoState *)IOSTATE)->stopStatus = MESSAGE_STOP_STATUS_NORMAL; }
*/
void IoMessage_break(IoMessage *self, IoObject *v)
{
  IoState *state = IOSTATE;
  state->stopStatus = MESSAGE_STOP_STATUS_BREAK;
  state->returnValue = v;
}

void IoMessage_continue(IoMessage *self)
{ ((IoState *)IOSTATE)->stopStatus = MESSAGE_STOP_STATUS_CONTINUE; }

void IoMessage_return_(IoMessage *self, IoObject *v)
{ 
  IoState *state = IOSTATE;
  state->stopStatus = MESSAGE_STOP_STATUS_RETURN; 
  state->returnValue = v;
}

int IoMessage_isStopped(IoMessage *self)
{ 
  int status = ((IoState *)IOSTATE)->stopStatus;
  return ((status == MESSAGE_STOP_STATUS_BREAK) || 
          (status == MESSAGE_STOP_STATUS_RETURN) || 
          (status == MESSAGE_STOP_STATUS_CONTINUE)); 
}

int IoMessage_isBrokenOrReturned(IoMessage *self)
{ 
  int status = ((IoState *)IOSTATE)->stopStatus;
  return ((status == MESSAGE_STOP_STATUS_BREAK) || 
          (status == MESSAGE_STOP_STATUS_RETURN)); 
}

int IoMessage_isBroken(IoMessage *self)
{ 
  int status = ((IoState *)IOSTATE)->stopStatus;
  return (status == MESSAGE_STOP_STATUS_BREAK); 
}

void IoMessage_resetStopStatus(IoMessage *self)
{  ((IoState *)IOSTATE)->stopStatus = MESSAGE_STOP_STATUS_NORMAL; }

int IoMessage_stopStatus(IoMessage *self)
{  return ((IoState *)IOSTATE)->stopStatus; }

/* --- primitive methods --------------------------------------------------- */

IoObject *IoMessage_clone(IoMessage *self, IoObject *locals, IoMessage *m)
{ return IoMessage_deepCopyOf_(self); }

IoObject *IoMessage_protoName(IoMessage *self, IoObject *locals, IoMessage *m)
{ return DATA(self)->method; }

IoObject *IoMessage_protoName_(IoMessage *self, IoObject *locals, IoMessage *m)
{ 
  DATA(self)->method = IOREF(IoMessage_locals_stringArgAt_(m, locals, 0));
  IoMessage_cacheIfPossible(self);
  return self; 
}

IoObject *IoMessage_descriptionString(IoMessage *self, IoObject *locals, IoMessage *m)
{ 
  ByteArray *ba = IoMessage_description(self); /* me must free the returned ByteArray */
  return IoState_stringWithByteArray_copy_(IOSTATE, ba, 0); 
}

IoObject *IoMessage_nextMessage(IoMessage *self, IoObject *locals, IoMessage *m)
{ return DATA(self)->nextMessage ? (IoObject *)DATA(self)->nextMessage : IONIL(self); }

IoObject *IoMessage_nextMessage_(IoMessage *self, IoObject *locals, IoMessage *m)
{ 
  DATA(self)->nextMessage = IOREF(IoMessage_locals_messageArgAt_(m , locals, 0));
  return self; 
}

IoObject *IoMessage_attachedMessage(IoMessage *self, IoObject *locals, IoMessage *m)
{ return DATA(self)->attachedMessage ? (IoObject *)DATA(self)->attachedMessage : IONIL(self); }

IoObject *IoMessage_attachedMessage_(IoMessage *self, IoObject *locals, IoMessage *m)
{ 
  DATA(self)->attachedMessage = IOREF(IoMessage_locals_messageArgAt_(m , locals, 0));
  return self; 
}

IoObject *IoMessage_argAt(IoMessage *self, IoObject *locals, IoMessage *m)
{
  int index =  IoNumber_asInt(IoMessage_locals_numberArgAt_(m , locals, 0));
  IoObject *v = List_at_(DATA(self)->args, index);
  return v ? v : IONIL(self);
}

IoObject *IoMessage_arguments(IoMessage *self, IoObject *locals, IoMessage *m)
{
  IoList *argsList = IoList_new(IOSTATE);
  List_copy_(IoList_rawList(argsList), DATA(self)->args);
  return argsList;
}

IoObject *IoMessage_arguments_(IoMessage *self, IoObject *locals, IoMessage *m)
{ 
  IoList *ioList = IoMessage_locals_listArgAt_(m, locals, 0);
  List *newArgs = IoList_rawList(ioList);
  int i;
  List_clear(DATA(self)->args);
  for (i=0; i < List_count(newArgs); i++)
  { 
    IoMessage *argMessage = LIST_AT_(newArgs, i);
    if (!ISMESSAGE(argMessage))
    {
      IoState_error_description_(IOSTATE, m, "Io.Message.arguments_", 
        "arguments_() takes a list containing only Message objects");    
    }
    List_add_(DATA(self)->args, IOREF(argMessage)); 
  }
  return self; 
}

IoObject *IoMessage_fromString(IoMessage *self, IoObject *locals, IoMessage *m)
{ 
  IoString *string = IoMessage_locals_stringArgAt_(m, locals, 0);
  IoString *label = DATA(m)->label;
  if (IoMessage_argCount(m)>1) { label = IoMessage_locals_stringArgAt_(m, locals, 1); }
  return IoMessage_newFromText_label_(IOSTATE,
      CSTRING(string), CSTRING(label));      
}

IoObject *IoMessage_cachedResult(IoMessage *self, IoObject *locals, IoMessage *m)
{ return (DATA(self)->cachedResult ? DATA(self)->cachedResult : IONIL(self)); }

IoObject *IoMessage_setCachedResult(IoMessage *self, IoObject *locals, IoMessage *m)
{
  DATA(self)->cachedResult = IOREF(IoMessage_locals_valueArgAt_(m , locals, 0));
  return self;
}

IoObject *IoMessage_argsEvaluatedIn(IoMessage *self, IoObject *locals, IoMessage *m)
{
  IoObject *context = IoMessage_locals_objectArgAt_(m, locals, 0);
  IoList *args = IoList_new(IOSTATE);
  int i;
  for (i = 0; i < List_count(DATA(self)->args); i++)
  { 
    IoObject *arg = IoMessage_locals_valueArgAt_(self, context, i);
    IoList_rawAdd_(args, arg);
  }
  return args;
}

/* ------------------------------ */
#define IOMESSAGEDATA(self) ((IoMessageData *)self->data)

IoString *IoMessage_rawMethod(IoMessage *self)
{ return IOMESSAGEDATA(self)->method; }

void IoMessage_addArg_(IoMessage *self, IoMessage *m)
{ List_add_(IOMESSAGEDATA(self)->args, IOREF(m)); }

IoMessage *IoMessage_rawArgAt_(IoMessage *self, int n)
{ return List_at_(IOMESSAGEDATA(self)->args, n); }

IoString *IoMessage_rawLabel(IoMessage *self)
{ return IOMESSAGEDATA(self)->label; }

List *IoMessage_rawArgs(IoMessage *self)
{ return IOMESSAGEDATA(self)->args; }

