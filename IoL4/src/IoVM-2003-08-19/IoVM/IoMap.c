/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */
 
#include "IoMap.h"
#include "IoObject.h"
#include "IoState.h"
#include "IoCFunction.h"
#include "IoNil.h"
#include "IoString.h"
#include "IoState.h"
#include "IoNumber.h"
#include "IoList.h"
#include "IoBlock.h"

#define DATA(self) ((IoMapData *)self->data)

IoTag *IoMap_tag(void *state)
{
  IoTag *tag = IoTag_newWithName_("Map");
  tag->state = state;
  tag->freeFunc  = (TagFreeFunc *)IoMap_free;
  tag->cloneFunc = (TagCloneFunc *)IoMap_rawClone;
  tag->markFunc  = (TagMarkFunc *)IoMap_mark;
  return tag;
}

IoMap *IoMap_proto(void *state)
{
  IoObject *self = IoObject_new(state);

  self->data = calloc(1, sizeof(IoMapData));
  self->tag = IoMap_tag(state);
  self->data = calloc(1, sizeof(IoMapData));
  DATA(self)->hash = Hash_new();
  
  IoState_registerProtoWithFunc_(state, self, IoMap_proto);

  IoObject_addMethod_(self, IOSTRING("empty"), IoMap_clear);
  IoObject_addMethod_(self, IOSTRING("at"), IoMap_at_);
  IoObject_addMethod_(self, IOSTRING("atPut"), IoMap_at_put_);
  IoObject_addMethod_(self, IOSTRING("atIfAbsentPut"), IoMap_atIfAbsentPut);
  IoObject_addMethod_(self, IOSTRING("count"), IoMap_count);
  IoObject_addMethod_(self, IOSTRING("keys"), IoMap_keys);
  IoObject_addMethod_(self, IOSTRING("values"), IoMap_values);
  IoObject_addMethod_(self, IOSTRING("doBlock"), IoMap_do_);
  /*Tag_addMethod(tag, "do"), IoObject_do);*/
  IoObject_addMethod_(self, IOSTRING("foreach"), IoMap_foreach);
  IoObject_addMethod_(self, IOSTRING("hasKey"), IoMap_hasKey_);
  IoObject_addMethod_(self, IOSTRING("hasValue"), IoMap_hasValue_);
  IoObject_addMethod_(self, IOSTRING("remove"), IoMap_remove_);
  return self;
}

IoMap *IoMap_rawClone(IoMap *self) 
{ 
  IoObject *child = IoObject_rawClonePrimitive(self);
  child->data = cpalloc(self->data, sizeof(IoMapData));
  DATA(child)->hash = Hash_clone(DATA(self)->hash);
  return child; 
}

IoMap *IoMap_new(void *state)
{
  IoObject *proto = IoState_protoWithInitFunction_(state, IoMap_proto);
  return IoMap_rawClone(proto);
}

void IoMap_free(IoMap *self) 
{ 
  Hash_free(DATA(self)->hash);
  free(self->data);
}

void IoMap_mark(IoMap *self) 
{ Hash_doOnKeyAndValue_(DATA(self)->hash, (ListDoCallback *)IoObject_makeGrayIfWhite); }

void IoMap_rawAtPut_(IoMap *self, IoString *k, IoObject *v)
{
  if (ISNUMBER(v)) { v = (IoObject *)IoNumber_newCopyOf_((IoNumber *)v); }
  Hash_at_put_(DATA(self)->hash, IOREF(k), IOREF(v));
}

Hash *IoMap_rawHash(IoMap *self)
{ return DATA(self)->hash; }

/* ----------------------------------------------------------- */

IoObject *IoMap_clear(IoMap *self, IoObject *locals, IoMessage *m)
{ Hash_clear(DATA(self)->hash); return self; }

IoObject *IoMap_at_(IoMap *self, IoObject *locals, IoMessage *m)
{
  IoString *k = IoMessage_locals_stringArgAt_(m, locals, 0);
  void *result = Hash_at_(DATA(self)->hash, k);
  if (!result && IoMessage_argCount(m) > 1) 
  { return IoMessage_locals_valueArgAt_(m, locals, 1); }
  return (result) ? (IoObject *)result : IONIL(self);
}

IoObject *IoMap_at_put_(IoMap *self, IoObject *locals, IoMessage *m)
{
  IoString *k = IoMessage_locals_stringArgAt_(m, locals, 0);
  IoObject *v  = IoMessage_locals_valueArgAt_(m, locals, 1);
  IoMap_rawAtPut_(self, k, v);
  return self;
}

IoObject *IoMap_atIfAbsentPut(IoMap *self, IoObject *locals, IoMessage *m)
{
  IoString *k = IoMessage_locals_stringArgAt_(m, locals, 0);
  void *result = Hash_at_(DATA(self)->hash, k);
  if (result) return result;
  else
  {
    IoObject *v = IoMessage_locals_valueArgAt_(m, locals, 1);
    IoMap_rawAtPut_(self, k, v);
    return v;
  }
}

IoObject *IoMap_count(IoMap *self, IoObject *locals, IoMessage *m)
{ return IoNumber_newWithDouble_(IOSTATE, Hash_count(DATA(self)->hash)); }

IoObject *IoMap_hasKey_(IoMap *self, IoObject *locals, IoMessage *m)
{
  IoString *k = IoMessage_locals_stringArgAt_(m, locals, 0);
  return Hash_at_(DATA(self)->hash, k) ? IONIL(self) : (IoObject *)self;
}

IoObject *IoMap_remove_(IoMap *self, IoObject *locals, IoMessage *m)
{
  IoString *k = IoMessage_locals_stringArgAt_(m, locals, 0);
  Hash_removeKey_(DATA(self)->hash, k);
  return self;
}

IoObject *IoMap_hasValue_(IoMap *self, IoObject *locals, IoMessage *m)
{
  IoState *state = IOSTATE;
  IoString *value = IoMessage_locals_valueArgAt_(m, locals, 0);
  IoObject *key = (IoObject *)Hash_firstKey(DATA(self)->hash);
  IoMessage *equalsMessage = IoMessage_newWithName_(state, state->equalEqualString);
  IoMessage *equalsMessageArg = IoMessage_newWithName_(state, IOSTRING("<setarg>"));
  while (key)
  {
    void *v = Hash_at_(DATA(self)->hash, key);
    IoMessage_cachedResult_(equalsMessageArg, v);
    if (!ISNIL(IoObject_perform((IoObject *)value, (void *)locals, equalsMessage)))
    { return self; }
    key = Hash_nextKey(DATA(self)->hash);
  }
  return IONIL(self);
}

IoObject *IoMap_keys(IoMap *self, IoObject *locals, IoMessage *m)
{
  IoList *list = IoList_new(IOSTATE);
  IoObject *key = (IoObject *)Hash_firstKey(DATA(self)->hash);
  while (key)
  {
    IoList_rawAdd_(list, key);
    key = Hash_nextKey(DATA(self)->hash);
  }
  return list;
}

IoObject *IoMap_values(IoMap *self, IoObject *locals, IoMessage *m)
{
  IoList *list = IoList_new(IOSTATE);
  IoObject *key = (IoObject *)Hash_firstKey(DATA(self)->hash);
  while (key)
  {
    IoList_rawAdd_(list, (IoObject *)Hash_at_(DATA(self)->hash, key));
    key = Hash_nextKey(DATA(self)->hash);
  }
  return list;
}

IoObject *IoMap_do_(IoMap *self, IoObject *locals, IoMessage *m)
{
  IoBlock *block = IoMessage_locals_blockArgAt_(m, locals, 0);
  IoMessage *blockMsg    = IoMessage_new(IOSTATE);
  IoMessage *keyArgMsg   = IoMessage_new(IOSTATE);
  IoMessage *valueArgMsg = IoMessage_new(IOSTATE);
  IoObject *key = (IoObject *)Hash_firstKey(DATA(self)->hash);
  IoMessage_addArg_(blockMsg, keyArgMsg);
  IoMessage_addArg_(blockMsg, valueArgMsg);
  while (key)
  {
    IoObject *v = (IoObject *)Hash_at_(DATA(self)->hash, key);
    IoMessage_cachedResult_(keyArgMsg, key);
    IoMessage_cachedResult_(valueArgMsg, v);
    IoBlock_target_locals_call_(block, locals, locals, blockMsg);
    key = Hash_nextKey(DATA(self)->hash);
  }
  return self;
}

IoObject *IoMap_foreach(IoMap *self, IoObject *locals, IoMessage *m)
{
  IoMessage_assertArgCount_receiver_(m, 3, self);
  {
    IoMessage *keyMessage = IoMessage_rawArgAt_(m, 0);
    IoMessage *valueMessage = IoMessage_rawArgAt_(m, 1);
    IoMessage *doMessage = IoMessage_rawArgAt_(m, 2);
    IoObject *key = (IoObject *)Hash_firstKey(DATA(self)->hash);
    IoObject *result = IONIL(self);    
    while (key)
    {
      IoObject *value = (IoObject *)Hash_at_(DATA(self)->hash, key);
      IoObject_setSlot_to_(locals, IoMessage_rawMethod(keyMessage), key);
      IoObject_setSlot_to_(locals, IoMessage_rawMethod(valueMessage), value);
      IoMessage_locals_performOn_(doMessage, locals, locals);
      key = Hash_nextKey(DATA(self)->hash);
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
