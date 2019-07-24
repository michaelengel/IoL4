/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#include "IoBlock.h"
#include "IoObject.h"
#include "IoState.h"
#include "IoMessage.h"
#include "IoMessage_parser.h"
#include "IoCFunction.h"
#include "IoString.h"
#include "IoNumber.h"
#include "IoNil.h"
#include "IoList.h"
#include "base/ByteArray.h"

#define DATA(self) ((IoBlockData *)self->data)

IoTag *IoBlock_tag(void *state)
{
  IoTag *tag = IoTag_newWithName_("Block");
  tag->state = state;
  tag->cloneFunc = (TagCloneFunc *)IoBlock_rawClone;
  tag->markFunc = (TagMarkFunc *)IoBlock_mark;
  tag->freeFunc = (TagFreeFunc *)IoBlock_free;
  tag->activateFunc = (TagActivateFunc *)IoBlock_target_locals_call_;
  tag->writeToStoreFunc = (TagWriteToStoreFunc *)IoBlock_writeToStore_;
  tag->readFromStoreFunc = (TagReadFromStoreFunc *)IoBlock_readFromStore_;
  return tag;
}

void IoBlock_writeToStore_(IoBlock *self, IoStoreHandle *sh)
{
  ByteArray *ba = IoBlock_justCode(self);
  IoStoreDatum d = IOSTOREDATUM(ba->bytes, ba->length+1);
  sh->onObject_setData_(sh, d);
  ByteArray_free(ba);
}

void *IoBlock_readFromStore_(IoBlock *self, IoStoreHandle *sh)
{
/*
  IoStoreDatum d = sh->onObject_getData(sh, self);
  IoMessage *m = IoMessage_newFromText_label_(IOSTATE, d.data, "[store]");
  
  IoBlock_justCode_(self, d.data, d.size);
  
  IoMessage *newM = IoMessage_newFromText_label_(IOSTATE, s, "[store]");
*/
  return self;
}

IoBlock *IoBlock_proto(void *state)
{
  IoObject *self = IoObject_new(state);

  self->data = calloc(1, sizeof(IoBlockData));
  self->tag = IoBlock_tag(state);
  DATA(self)->message  = ((IoState *)state)->nilMessage;
  DATA(self)->argNames = List_new();
  DATA(self)->scope    = 0x0;
  IoState_registerProtoWithFunc_(state, self, IoBlock_proto);

  IoObject_addMethod_(self, IOSTRING("print"), IoBlock_print);
  IoObject_addMethod_(self, IOSTRING("code"), IoBlock_code);
  IoObject_addMethod_(self, IOSTRING("message"), IoBlock_message);
  IoObject_addMethod_(self, IOSTRING("setMessage"), IoBlock_setMessage);
  IoObject_addMethod_(self, IOSTRING("argumentNames"), IoBlock_argumentNames);
  IoObject_addMethod_(self, IOSTRING("setArgumentNames"), IoBlock_argumentNames_);
  IoObject_addMethod_(self, IOSTRING("setScope"), IoBlock_setScope_);
  IoObject_addMethod_(self, IOSTRING("scope"), IoBlock_scope);
  
  /* expiremental Smalltalky stuff */
  IoObject_addMethod_(self, IOSTRING("ifTrue"), IoBlock_ifTrue_); 
  IoObject_addMethod_(self, IOSTRING("ifFalse"), IoBlock_ifFalse_);
  IoObject_addMethod_(self, IOSTRING("whileTrue"), IoBlock_whileTrue_);
  IoObject_addMethod_(self, IOSTRING("whileFalse"), IoBlock_whileFalse_);
  /*Tag_addMethod(tag, "jitCompile"), IoBlock_jitCompile);*/
  return self;
}

IoBlock *IoBlock_rawClone(IoBlock *self)
{
  IoObject *child = IoObject_rawClonePrimitive(self);
  child->data = calloc(1, sizeof(IoBlockData));
  child->tag     = self->tag;
  DATA(child)->message  = IOSTATE->nilMessage;
  DATA(child)->argNames = List_clone(DATA(self)->argNames);
  DATA(child)->scope    = DATA(self)->scope;
  return child;
}

IoBlock *IoBlock_new(IoState *state)
{
  IoObject *proto = IoState_protoWithInitFunction_(state, IoBlock_proto);
  return IoBlock_rawClone(proto);
}

void IoBlock_mark(IoBlock *self)
{ 
  IoObject_makeGrayIfWhite(DATA(self)->message);
  if (DATA(self)->scope) IoObject_makeGrayIfWhite(DATA(self)->scope);
  List_do_(DATA(self)->argNames, (ListDoCallback *)IoObject_makeGrayIfWhite);
}

void IoBlock_free(IoBlock *self)
{
  List_free(DATA(self)->argNames);
  free(self->data);
}

void IoBlock_message_(IoBlock *self, IoMessage *m) { DATA(self)->message = IOREF(m); }

/* --- parsing -------------------------------------------------------- */
IoBlock *IoBlock_newFromCString_(IoState *state, char *s)
{
  IoBlock *self = IoBlock_new(state);
  IoBlock_message_(self, IoMessage_newFromText_label_(state, s, "[IoBlock_newFromCString_]"));
  return self;
}
  
/* --- calling -------------------------------------------------------- */

IoObject *IoBlock_target_locals_call_(IoBlock *self, 
  IoObject *target, IoObject *locals, IoMessage *m)
{
  IoState *state = IOSTATE;
  IoObject *result;
  IoObject *blockLocals;
  IoObject *scope = DATA(self)->scope;
  
  IoState_pushRetainPool(state);
  if (scope)
  {
    blockLocals = IoObject_rawClone(scope);
    IoObject_setSlot_to_(blockLocals, state->selfString, scope);
    IoObject_setSlot_to_(blockLocals, state->targetString, target);  
  }
  else
  {
    /*blockLocals = IoObject_rawClone(target);*/
    blockLocals = IoObject_rawClone(state->lobby); /* new locals handling */
    IoObject_setSlot_to_(blockLocals, state->selfString, target);
  }
  
  IoObject_setSlot_to_(blockLocals, state->messageString, m); /* for the debugger */
  IoObject_setSlot_to_(blockLocals, state->blockString, self);
  IoObject_setSlot_to_(blockLocals, state->senderString, locals);
  
  /* eval args and put into locals */
  {
    int max = List_count(DATA(self)->argNames);
    IoString *name;
    IoObject *arg;
    int i;
    for (i=0; i<max; i++)  
    {
      arg = IoMessage_locals_valueArgAt_(m, locals, i);
      name = LIST_AT_(DATA(self)->argNames, i);
      IoObject_setSlot_to_(blockLocals, name, arg); /* check for proto slotName? */
    }
  }  

  result = IoMessage_locals_performOn_(DATA(self)->message, blockLocals, blockLocals);  
  IoMessage_end(DATA(self)->message);
  IoState_popRetainPool(state);
  /*IoState_immediatelyFreeIfUnreferenced_(state, blockLocals);*/ /* assuming it's not on the retain stack... */
  return result;
}

IoObject *IoBlock_lobbyMethod(IoObject *target, IoObject *locals, IoMessage *m)
{
  IoBlock *self = IoBlock_new(target->tag->state);
  int nargs = IoMessage_argCount(m);
  int i;
  if (nargs == 0)
  { 
    IoState_error_description_(IOSTATE, m, 
      "Io.Block.argumentMissing", "requires at least one argument"); 
  }
  for (i=0; i<nargs-1; i++)
  {
    IoMessage *argMessage = IoMessage_rawArgAt_(m, i);
    IoString *name = IoMessage_rawMethod(argMessage);
    List_add_(DATA(self)->argNames, IOREF(name));
  }
  DATA(self)->message = (IoMessage *)IOREF(IoMessage_rawArgAt_(m, nargs-1));
  /*IoBlock_compile(self);*/
  return self;
}

IoObject *IoBlock_lobbyBlock(IoObject *target, IoObject *locals, IoMessage *m)
{
  IoBlock *self = (IoBlock *)IoBlock_lobbyMethod(target, locals, m);
  DATA(self)->scope = IOREF(locals);
  return self;
}

IoObject *IoBlock_print(IoBlock *self, IoObject *locals, IoMessage *m)
{ 
  if (!ISNIL(DATA(self)->scope))
  { IoState_print_(IOSTATE, "block("); }
  else
  { IoState_print_(IOSTATE, "method("); }
  IoMessage_print(DATA(self)->message); 
  IoState_print_(IOSTATE, ")");
  return IONIL(self); 
}

ByteArray *IoBlock_justCode(IoBlock *self)
{ 
  ByteArray *ba = ByteArray_newWithCString_("block(");
  {
    int i;
    for (i=0; i < List_count(DATA(self)->argNames); i++)
    {
      IoString *argName = List_at_(DATA(self)->argNames, i);
      ByteArray_append_(ba, IoString_rawByteArray(argName));
      ByteArray_appendCString_(ba, ", ");
    }
  }
  ByteArray_append_(ba, IoMessage_description(DATA(self)->message));

  ByteArray_appendCString_(ba, ")");
  return ba; 
}

IoObject *IoBlock_code(IoBlock *self, IoObject *locals, IoMessage *m)
{ 
  return IoString_newWithByteArray_copy_(IOSTATE, IoBlock_justCode(self), 0); 
}

IoObject *IoBlock_code_(IoBlock *self, IoObject *locals, IoMessage *m)
{ 
  IoString *string = IoMessage_locals_stringArgAt_(m, locals, 0);
  char *s = CSTRING(string);
  IoMessage *newM = IoMessage_newFromText_label_(IOSTATE, s, "[IoBlock_code_]");
  if (newM) { DATA(self)->message = IOREF(newM); }
  else
  { 
    IoState_error_description_(IOSTATE, m, 
      "Io.Block.compileError", "no messages found in compile string"); 
  }
  return self;
}

IoObject *IoBlock_message(IoBlock *self, IoObject *locals, IoMessage *m)
{ return DATA(self)->message ? (IoObject *)DATA(self)->message : IONIL(self); }

IoObject *IoBlock_setMessage(IoBlock *self, IoObject *locals, IoMessage *m)
{ 
  IoMessage *message = IoMessage_locals_messageArgAt_(m, locals, 0);
  DATA(self)->message = IOREF(message);
  return self;
}

IoObject *IoBlock_argumentNames(IoBlock *self, IoObject *locals, IoMessage *m)
{
  IoList *argsList = IoList_new(IOSTATE);
  int i;
  for (i=0; i < List_count(DATA(self)->argNames); i++)
  { IoList_rawAdd_(argsList, LIST_AT_(DATA(self)->argNames, i)); }
  return argsList;
}

IoObject *IoBlock_argumentNames_(IoBlock *self, IoObject *locals, IoMessage *m)
{
  IoList *newArgNames = IoMessage_locals_listArgAt_(m, locals, 0);
  IOASSERT(ISLIST(newArgNames), "argument must be a List");
  {
    unsigned int i;
    for (i=0; i < IoList_rawCount(newArgNames); i++)
    { IOASSERT(ISSTRING(IoList_rawAt_(newArgNames, i)), "all argument names must be Strings"); }
  }
  List_copy_(DATA(self)->argNames, IoList_rawList(newArgNames));
  return self;
}

IoObject *IoBlock_scope(IoBlock *self, IoObject *locals, IoMessage *m)
{ 
  IoObject *scope = DATA(self)->scope;
  return scope ? scope : IONIL(self); 
}

IoObject *IoBlock_setScope_(IoBlock *self, IoObject *locals, IoMessage *m)
{
  IoObject *scope = IoMessage_locals_valueArgAt_(m, locals, 0);
  DATA(self)->scope = ISNIL(scope) ? 0x0 : IOREF(scope);
  return self;
}

IoObject *IoBlock_ifTrue_(IoBlock *self, IoObject *locals, IoMessage *m)
{
  IOASSERT(IoMessage_argCount(m)==1, "method requires a single argument");
  if (!ISNIL(IoBlock_target_locals_call_(self, locals, locals, DATA(self)->message)))
  { IoMessage_locals_performOn_(IoMessage_rawArgAt_(m, 0), locals, locals); }
  return self;
}

IoObject *IoBlock_ifFalse_(IoBlock *self, IoObject *locals, IoMessage *m)
{
  IOASSERT(IoMessage_argCount(m)==1, "method requires a single argument");
  if (ISNIL(IoBlock_target_locals_call_(self, locals, locals, DATA(self)->message)))
  { IoMessage_locals_performOn_(IoMessage_rawArgAt_(m, 0), locals, locals); }
  return self;
}

IoObject *IoBlock_whileTrue_(IoBlock *self, IoObject *locals, IoMessage *m)
{
  IOASSERT(IoMessage_argCount(m)==1, "method requires a single argument");
  while (!ISNIL(IoBlock_target_locals_call_(self, locals, locals, DATA(self)->message)))
  { IoMessage_locals_performOn_(IoMessage_rawArgAt_(m, 0), locals, locals); }
  return self;
}

IoObject *IoBlock_whileFalse_(IoBlock *self, IoObject *locals, IoMessage *m)
{
  IOASSERT(IoMessage_argCount(m)==1, "method requires a single argument");
  while (ISNIL(IoBlock_target_locals_call_(self, locals, locals, DATA(self)->message)))
  { IoMessage_locals_performOn_(IoMessage_rawArgAt_(m, 0), locals, locals); }
  return self;
}

