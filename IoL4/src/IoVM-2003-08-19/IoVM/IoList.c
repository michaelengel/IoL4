/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#include "IoList.h"
#include "IoObject.h"
#include "IoState.h"
#include "IoCFunction.h"
#include "IoNil.h"
#include "IoString.h"
#include "IoState.h"
#include "IoNumber.h"
#include "IoBlock.h"
#include "base/SDSort.h"
#include <math.h>

#define DATA(self) ((IoListData *)self->data)

IoTag *IoList_tag(void *state)
{
  IoTag *tag = IoTag_newWithName_("List");
  tag->state = state;
  tag->freeFunc = (TagFreeFunc *)IoList_free;
  tag->cloneFunc = (TagCloneFunc *)IoList_rawClone;
  tag->markFunc = (TagMarkFunc *)IoList_mark;
  tag->compareFunc = (TagCompareFunc *)IoList_compare;
  return tag;
}  
  
IoList *IoList_proto(void *state)
{
  IoObject *self = IoObject_new(state);
  self->tag = IoList_tag(state);

  self->data = calloc(1, sizeof(IoListData));
  DATA(self)->list = List_new();
  IoState_registerProtoWithFunc_(state, self, IoList_proto);
  
  IoObject_addMethod_(self, IOSTRING("clone"), IoList_clone);
  IoObject_addMethod_(self, IOSTRING("add"), IoList_add_);
  IoObject_addMethod_(self, IOSTRING("remove"), IoList_remove_);
  IoObject_addMethod_(self, IOSTRING("indexOf"), IoList_indexOf_);
  IoObject_addMethod_(self, IOSTRING("contains"), IoList_contains_);
  IoObject_addMethod_(self, IOSTRING("push"), IoList_push_);
  IoObject_addMethod_(self, IOSTRING("pop"), IoList_pop);
  IoObject_addMethod_(self, IOSTRING("empty"), IoList_clear);
  IoObject_addMethod_(self, IOSTRING("count"), IoList_count);
  IoObject_addMethod_(self, IOSTRING("insertAt"), IoList_insert_at_);
  IoObject_addMethod_(self, IOSTRING("removeAt"), IoList_removeAt_);
  IoObject_addMethod_(self, IOSTRING("at"), IoList_at_);
  IoObject_addMethod_(self, IOSTRING("atPut"), IoList_at_put_);
  IoObject_addMethod_(self, IOSTRING("swapIndexWithIndex"), IoList_swapIndex_withIndex_);
  IoObject_addMethod_(self, IOSTRING("doBlock"), IoList_doBlock_);
  IoObject_addMethod_(self, IOSTRING("do"), IoList_do_);
  IoObject_addMethod_(self, IOSTRING("select"), IoList_select);
  IoObject_addMethod_(self, IOSTRING("reject"), IoList_reject);
  IoObject_addMethod_(self, IOSTRING("detect"), IoList_detect);
  IoObject_addMethod_(self, IOSTRING("sort"), IoList_sort);
  IoObject_addMethod_(self, IOSTRING("sortBy"), IoList_sortBy_);
  IoObject_addMethod_(self, IOSTRING("foreach"), IoList_foreach);
  IoObject_addMethod_(self, IOSTRING("reverse"), IoList_reverse);
  IoObject_addMethod_(self, IOSTRING("reverseForeach"), IoList_reverseForeach);
  IoObject_addMethod_(self, IOSTRING("random"), IoList_random);
  IoObject_addMethod_(self, IOSTRING("randomize"), IoList_randomize);
  IoObject_addMethod_(self, IOSTRING("sizeTo"), IoList_sizeTo_);
  IoObject_addMethod_(self, IOSTRING("first"), IoList_first);
  IoObject_addMethod_(self, IOSTRING("last"), IoList_last);
  IoObject_addMethod_(self, IOSTRING("print"), IoList_protoPrint);
  IoObject_addMethod_(self, IOSTRING("translateBlock"), IoList_translateBlock);
  IoObject_addMethod_(self, IOSTRING("translate"), IoList_translate);
  return self;
}

IoList *IoList_rawClone(IoList *self) 
{ 
  IoObject *child = IoObject_rawClonePrimitive(self);
  child->data = calloc(1, sizeof(IoListData));
  child->tag= self->tag;
  DATA(child)->list = List_clone(DATA(self)->list);
  return child; 
}

IoList *IoList_new(void *state)
{
  IoObject *proto = IoState_protoWithInitFunction_(state, IoList_proto);
  return IoList_rawClone(proto);
}

void IoList_free(IoList *self) 
{ 
  /*printf("freeing IoList %p\n", self);*/
  List_free(DATA(self)->list);
  free(self->data);
}

void IoList_mark(IoList *self) 
{ List_do_(DATA(self)->list, (ListDoCallback *)IoObject_makeGrayIfWhite); }

int IoList_compare(IoList *self, IoList *otherList)
{
  if(!ISLIST(otherList)) 
  { return -((ptrdiff_t)self->tag - (ptrdiff_t)otherList->tag); }
  {
    int i;
    int sizeDiff;
    sizeDiff = List_count(DATA(self)->list) -  List_count(DATA(otherList)->list);
    if (sizeDiff) { return sizeDiff; }
    for (i=0; i < List_count(DATA(self)->list); i++)
    {
      IoObject *v1 = LIST_AT_(DATA(self)->list, i);
      IoObject *v2 = LIST_AT_(DATA(otherList)->list, i);
      int c = IoObject_compare(v1, v2);
      if (c) { return c; }
    }
  }
  return 0;
}

List *IoList_rawList(IoList *self)
{ return DATA(self)->list; }

IoObject *IoList_rawAt_(IoList *self, int i)
{ return List_at_(DATA(self)->list, i); }

void IoList_rawAdd_(IoList *self, IoObject *v)
{ List_add_(DATA(self)->list, IOREF(v)); }

void IoList_rawAddList_(IoList *self, IoList *other)
{
  int i;
  for (i=0; i< List_count(DATA(other)->list); i++)
  {
    IoObject *v = List_at_(DATA(other)->list, i);
    List_add_(DATA(self)->list, IOREF(v));
  }
}

size_t IoList_rawCount(IoList *self)
{ return List_count(DATA(self)->list); }

int IoList_rawIndexOf_(IoList *self, IoObject *v)
{
  int i;
  for (i=0; i< List_count(DATA(self)->list); i++)
  {
    IoObject *item = List_at_(DATA(self)->list, i);
    if (IoObject_compare(v, item) == 0) return i;
  }
  return -1;
}

/* ----------------------------------------------------------- */

IoObject *IoList_clone(IoList *self, IoObject *locals, IoMessage *m)
{
  IoList *newIoList = IoList_new(IOSTATE);
  List_copy_(IoList_rawList(newIoList), DATA(self)->list);
  {
    int i;
    for (i=0; i < IoMessage_argCount(m); i++)
    {
      IoObject *v = IoMessage_locals_valueArgAt_(m, locals, i);
      IoList_rawAdd_(newIoList, v);
    }
  }
  return newIoList;
}

IoObject *IoList_add_(IoList *self, IoObject *locals, IoMessage *m)
{
  int n;
  for (n=0; n < IoMessage_argCount(m); n++)
  {
    IoMessage *argm = IoMessage_rawArgAt_(m, n);
    IoObject *v = IoMessage_locals_performOn_(argm, locals, locals); 
    IoState_stackRetain_(IOSTATE, v);
  
    if (ISNUMBER(v))
    { List_add_(DATA(self)->list, IOREF(IoNumber_newCopyOf_((IoNumber *)v))); }
    else
    { List_add_(DATA(self)->list, IOREF(v)); }
  }
  return self;
}

IoObject *IoList_remove_(IoList *self, IoObject *locals, IoMessage *m)
{
  IoObject *v = IoMessage_locals_valueArgAt_(m, locals, 0);
  int i = IoList_rawIndexOf_(self, v);
  if (i != -1) List_removeIndex_(DATA(self)->list, i);
  return self;
}

IoObject *IoList_indexOf_(IoList *self, IoObject *locals, IoMessage *m)
{
  IoObject *v = IoMessage_locals_valueArgAt_(m, locals, 0);
  int i = IoList_rawIndexOf_(self, v);
  return i == -1 ? IONIL(self) : 
    (IoObject *)IoNumber_newWithDouble_(IOSTATE, IoList_rawIndexOf_(self, v));
}

IoObject *IoList_contains_(IoList *self, IoObject *locals, IoMessage *m)
{
  IoObject *v = IoMessage_locals_valueArgAt_(m, locals, 0);
  return IoList_rawIndexOf_(self, v) == -1 ? IONIL(self) : (IoObject *)self;
}

IoObject *IoList_push_(IoList *self, IoObject *locals, IoMessage *m)
{
  IoObject *v = IoMessage_locals_valueArgAt_(m, locals, 0);
  List_push_(DATA(self)->list, IOREF(v));
  return v;
}

IoObject *IoList_pop(IoList *self, IoObject *locals, IoMessage *m)
{
  IoObject *v = List_pop(DATA(self)->list);
  return (v) ? (IoObject *)v : IONIL(self);
}

IoObject *IoList_count(IoList *self, IoObject *locals, IoMessage *m)
{ return IoNumber_newWithDouble_(IOSTATE, List_count(DATA(self)->list)); }

IoObject *IoList_insert_at_(IoList *self, IoObject *locals, IoMessage *m)
{
  IoObject *v = IoMessage_locals_valueArgAt_(m, locals, 0);
  int index = IoMessage_locals_intArgAt_(m, locals, 1);
  List_at_insert_(DATA(self)->list, index, IOREF(v));
  return v;
}

IoObject *IoList_removeAt_(IoList *self, IoObject *locals, IoMessage *m)
{
  int index = IoMessage_locals_intArgAt_(m, locals, 0);
  IoObject *v = List_at_(DATA(self)->list, index);
  List_removeIndex_(DATA(self)->list, index);
  return (v) ? (IoObject *)v : IONIL(self);
}

IoObject *IoList_at_(IoList *self, IoObject *locals, IoMessage *m)
{
  int index = IoMessage_locals_intArgAt_(m, locals, 0);
  IoObject *v = List_at_(DATA(self)->list, index);
  return (v) ? (IoObject *)v : IONIL(self);
}

inline void IoList_rawAtPut(IoList *self, int i, IoObject *v)
{
  if (ISNUMBER(v))
  { List_at_put_(DATA(self)->list, i, IOREF(IoNumber_newCopyOf_((IoNumber *)v))); }
  else
  { List_at_put_(DATA(self)->list, i, IOREF(v)); }
}

IoObject *IoList_at_put_(IoList *self, IoObject *locals, IoMessage *m)
{
  int i = IoMessage_locals_intArgAt_(m, locals, 0);
  IoObject *v = IoMessage_locals_valueArgAt_(m, locals, 1);
  IoList_rawAtPut(self, i, v);
  return v;
}

IoObject *IoList_clear(IoList *self, IoObject *locals, IoMessage *m)
{ List_clear(DATA(self)->list); return self; }

IoObject *IoList_swapIndex_withIndex_(IoList *self, IoObject *locals, IoMessage *m)
{
  int i = IoMessage_locals_intArgAt_(m, locals, 0);
  int j = IoMessage_locals_intArgAt_(m, locals, 1);
  List_swap_with_(DATA(self)->list, i, j);
  return self;
}

IoObject *IoList_reverse(IoList *self, IoObject *locals, IoMessage *m)
{ List_reverse(DATA(self)->list); return self; }

IoObject *IoList_sort(IoList *self, IoObject *locals, IoMessage *m)
{
  List_qsort(DATA(self)->list, (ListSortCallback *)IoObject_sortCompare);
  return self;
}

/* --- sorting ----------------------------------------------- */

typedef struct
{
  IoObject *locals;
  IoBlock *block;
  IoMessage *blockMsg;
  IoMessage *argMsg1;
  IoMessage *argMsg2;
  List *list;
} SortContext;

int SortContext_compareForSort(SortContext *self, int i, int j)
{
  IoObject *cr;
  IoState_pushRetainPool(self->block->tag->state);
  /*
  IoMessage_setCachedArg_to_(self->blockMsg, 0, LIST_AT_(DATA(self)->list, i));
  IoMessage_setCachedArg_to_(self->blockMsg, 1, LIST_AT_(DATA(self)->list, j));
  */
  IoMessage_cachedResult_(self->argMsg1, LIST_AT_(self->list, i));
  IoMessage_cachedResult_(self->argMsg2, LIST_AT_(self->list, j));
  cr = IoBlock_target_locals_call_(self->block, self->locals, self->locals, self->blockMsg);
  /*cr = IoMessage_locals_performOn_(self->block->message, self->locals, self->locals);*/

  IoState_popRetainPool(self->block->tag->state);
  return ISNIL(cr) ? 1 : -1;
}

void SortContext_swapForSort(SortContext *self, int i, int j)
{ List_swap_with_(self->list, i, j); }

IoObject *IoList_sortBy_(IoList *self, IoObject *locals, IoMessage *m)
{
  SortContext sc;
  SortContext *sortContext = &sc; 

  sortContext->list = DATA(self)->list;
  sortContext->locals = locals;
  sortContext->block = IoMessage_locals_blockArgAt_(m, locals, 0);
  sortContext->blockMsg = IoMessage_new(IOSTATE);
  sortContext->argMsg1  = IoMessage_new(IOSTATE);
  sortContext->argMsg2  = IoMessage_new(IOSTATE);

  IoMessage_addArg_(sortContext->blockMsg, sortContext->argMsg1);
  IoMessage_addArg_(sortContext->blockMsg, sortContext->argMsg2);

  SDSort_sortContext_comp_swap_size_type_(sortContext, 
    (SDSortCompareCallback *)SortContext_compareForSort, 
    (SDSortSwapCallback *)SortContext_swapForSort, 
    List_count(DATA(self)->list), SDQuickSort);

  return self;
}

IoObject *IoList_doBlock_(IoList *self, IoObject *locals, IoMessage *m)
{
  IoBlock *block = IoMessage_locals_blockArgAt_(m, locals, 0);
  IoMessage *blockMsg = IoMessage_new(IOSTATE);
  IoMessage *argMsg   = IoMessage_new(IOSTATE);
  int i;
  IoMessage_addArg_(blockMsg, argMsg);
  for (i = 0; i < List_count(DATA(self)->list); i++)
  {
    IoObject *v = LIST_AT_(DATA(self)->list, i);
    IoMessage_cachedResult_(argMsg, v);
    IoBlock_target_locals_call_(block, locals, locals, blockMsg);
  }
  return self;
}

IoObject *IoList_do_(IoList *self, IoObject *locals, IoMessage *m)
{
  IoMessage *msg = IoMessage_rawArgAt_(m, 0);
  int i;
  if (!msg)
  {
    IoState_error_description_(IOSTATE, self, 
      "Io.List.do",  "requires an argument\n");
  }
  for (i = 0; i < List_count(DATA(self)->list); i++)
  {
    IoObject *v = LIST_AT_(DATA(self)->list, i);
    IoMessage_locals_performOn_(msg, locals, v);
  }
  return self;
}

inline IoObject *IoList_foreachv(IoList *self, IoObject *locals, IoMessage *m)
{
  IoObject *result = IONIL(self);
  IoMessage *valueMessage = IoMessage_rawArgAt_(m, 0);
  IoMessage *doMessage    = IoMessage_rawArgAt_(m, 1);
    
  int i;
  for (i = 0; i < List_count(DATA(self)->list); i++)
  {
      IoObject *value = (IoObject *)LIST_AT_(DATA(self)->list, i);
      IoObject_setSlot_to_(locals, IoMessage_rawMethod(valueMessage), value);
      result = IoMessage_locals_performOn_(doMessage, locals, locals);
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

IoObject *IoList_foreach(IoList *self, IoObject *locals, IoMessage *m)
{
  if (IoMessage_argCount(m)==2) { return IoList_foreachv(self, locals, m); }
  IoMessage_assertArgCount_receiver_(m, 3, self);
  {
    IoObject *result = IONIL(self);
    IoMessage *indexMessage = IoMessage_rawArgAt_(m, 0);
    IoMessage *valueMessage = IoMessage_rawArgAt_(m, 1);
    IoMessage *doMessage    = IoMessage_rawArgAt_(m, 2);
    
    IoString *slotName = IoMessage_rawMethod(indexMessage);
    IoNumber *index = IoObject_getSlot_(locals, slotName);
    int i;
    if (!index || !ISNUMBER(index))
    {
      IoObject_setSlot_to_(locals, slotName, IoNumber_newWithDouble_(IOSTATE, 0));
      index = IoObject_getSlot_(locals, slotName); /* numbers copied by value! */
    }
    for (i = 0; i < List_count(DATA(self)->list); i++)
    {
      IoObject *value = (IoObject *)LIST_AT_(DATA(self)->list, i);
      IoNumber_rawSet(index, i);
      IoObject_setSlot_to_(locals, IoMessage_rawMethod(valueMessage), value);
      IoState_pushRetainPool(IOSTATE);
      result = IoMessage_locals_performOn_(doMessage, locals, locals);
      IoState_popRetainPool(IOSTATE);
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
    IoState_stackRetain_(IOSTATE, result);
    return result;
  }
}

IoObject *IoList_reverseForeach(IoList *self, IoObject *locals, IoMessage *m)
{
  if (IoMessage_argCount(m)==2) { return IoList_foreachv(self, locals, m); }
  IoMessage_assertArgCount_receiver_(m, 3, self);
  {
    IoObject *result = IONIL(self);
    IoMessage *indexMessage = IoMessage_rawArgAt_(m, 0);
    IoMessage *valueMessage = IoMessage_rawArgAt_(m, 1);
    IoMessage *doMessage    = IoMessage_rawArgAt_(m, 2);
    
    IoString *slotName = IoMessage_rawMethod(indexMessage);
    IoNumber *index = IoObject_getSlot_(locals, slotName);
    int i;
    if (!index || !ISNUMBER(index))
    {
      IoObject_setSlot_to_(locals, slotName, IoNumber_newWithDouble_(IOSTATE, 0));
      index = IoObject_getSlot_(locals, slotName); /* numbers copied by value! */
    }
    for (i = List_count(DATA(self)->list)-1; i >=0; i--)
    {
      IoObject *value = (IoObject *)LIST_AT_(DATA(self)->list, i);
      IoNumber_rawSet(index, i);
      IoObject_setSlot_to_(locals, IoMessage_rawMethod(valueMessage), value);
      IoState_pushRetainPool(IOSTATE);
      result = IoMessage_locals_performOn_(doMessage, locals, locals);
      IoState_popRetainPool(IOSTATE);
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
    IoState_stackRetain_(IOSTATE, result);
    return result;
  }
}

IoObject *IoList_random(IoList *self, IoObject *locals, IoMessage *m)
{ 
  IoObject *r = List_random(DATA(self)->list);
  return r?r:IONIL(self);
}

IoObject *IoList_randomize(IoList *self, IoObject *locals, IoMessage *m)
{ 
  List_randomize(DATA(self)->list);
  return self;
}

IoObject *IoList_sizeTo_(IoList *self, IoObject *locals, IoMessage *m)
{
  int newSize = IoMessage_locals_intArgAt_(m, locals, 0);
  List_sizeTo_(DATA(self)->list, newSize);
  return self;
}

IoObject *IoList_first(IoList *self, IoObject *locals, IoMessage *m)
{
  IoObject *result = List_at_(DATA(self)->list, 0);
  return result ? result : ((IoState *)IOSTATE)->ioNil;
}

IoObject *IoList_last(IoList *self, IoObject *locals, IoMessage *m)
{
  IoObject *result = List_at_(DATA(self)->list, List_count(DATA(self)->list)-1);
  return result ? result : ((IoState *)IOSTATE)->ioNil;
}

IoObject *IoList_protoPrint(IoList *self, IoObject *locals, IoMessage *m)
{
  IoString *separator = IOSTRING(", ");
  int i;
  
  if (IoMessage_argCount(m) > 0)
  { separator = IoMessage_locals_stringArgAt_(m, locals, 0); }

  for (i=0; i < List_count(DATA(self)->list); i++)
  {
    IoObject *item = List_at_(DATA(self)->list, i);
    IoObject_print(item);
    if (i < List_count(DATA(self)->list) - 1) 
      IoState_justPrint_(IOSTATE, CSTRING(separator)); 
  }
  return self;
}

IoObject *IoList_select(IoList *self, IoObject *locals, IoMessage *m)
{
  IoBlock *block = IoMessage_locals_blockArgAt_(m, locals, 0);
  IoMessage *blockMsg = IoMessage_new(IOSTATE);
  IoMessage *argMsg   = IoMessage_new(IOSTATE);
  int i;
  int putIndex = 0;
  IoMessage_addArg_(blockMsg, argMsg);
  for (i = 0; i < List_count(DATA(self)->list); i++)
  {
    IoObject *result;
    IoObject *v = LIST_AT_(DATA(self)->list, i);
    IoMessage_cachedResult_(argMsg, v);
    IoState_pushRetainPool(IOSTATE);
    result = IoBlock_target_locals_call_(block, locals, locals, blockMsg);
    IoState_popRetainPool(IOSTATE);
    if (i != putIndex) List_at_put_(DATA(self)->list, putIndex, v);
    if (!ISNIL(result)) putIndex++;
  }
  List_setLength_(DATA(self)->list, putIndex);
  return self;
}

IoObject *IoList_reject(IoList *self, IoObject *locals, IoMessage *m)
{
  IoBlock *block = IoMessage_locals_blockArgAt_(m, locals, 0);
  IoMessage *blockMsg = IoMessage_new(IOSTATE);
  IoMessage *argMsg   = IoMessage_new(IOSTATE);
  int i;
  int putIndex = 0;
  IoMessage_addArg_(blockMsg, argMsg);
  for (i = 0; i < List_count(DATA(self)->list); i++)
  {
    IoObject *result;
    IoObject *v = LIST_AT_(DATA(self)->list, i);
    IoMessage_cachedResult_(argMsg, v);
    IoState_pushRetainPool(IOSTATE);
    result = IoBlock_target_locals_call_(block, locals, locals, blockMsg);
    IoState_popRetainPool(IOSTATE);
    if (i != putIndex) List_at_put_(DATA(self)->list, putIndex, v);
    if (ISNIL(result)) putIndex++;
  }
  List_setLength_(DATA(self)->list, putIndex);
  return self;
}

IoObject *IoList_detect(IoList *self, IoObject *locals, IoMessage *m)
{
  IoBlock *block = IoMessage_locals_blockArgAt_(m, locals, 0);
  IoMessage *blockMsg = IoMessage_new(IOSTATE);
  IoMessage *argMsg   = IoMessage_new(IOSTATE);
  int i;
  IoMessage_addArg_(blockMsg, argMsg);
  for (i = 0; i < List_count(DATA(self)->list); i++)
  {
    IoObject *result;
    IoObject *v = LIST_AT_(DATA(self)->list, i);
    IoMessage_cachedResult_(argMsg, v);
    IoState_pushRetainPool(IOSTATE);
    result = IoBlock_target_locals_call_(block, locals, locals, blockMsg);
    IoState_popRetainPool(IOSTATE);
    if (!ISNIL(result)) { IoState_stackRetain_(IOSTATE, v); return v; }
  }
  return IONIL(self);
}

IoObject *IoList_translateBlock(IoList *self, IoObject *locals, IoMessage *m)
{
   IoState *state = IOSTATE;
   IoBlock *block = IoMessage_locals_blockArgAt_(m, locals, 0);
   IoMessage *blockMsg = IoMessage_new(IOSTATE);
   IoMessage *argMsg   = IoMessage_new(IOSTATE);
   int i;
   IoMessage_addArg_(blockMsg, argMsg);
   for (i = 0; i < List_count(DATA(self)->list); i++)
   {
     IoObject *result;
     IoObject *v = LIST_AT_(DATA(self)->list, i);
     IoMessage_cachedResult_(argMsg, v);
     IoState_pushRetainPool(state);
     result = IoBlock_target_locals_call_(block, locals, locals, blockMsg);
     IoState_popRetainPool(state);
     IoList_rawAtPut(self, i, result); /* numbers copied by value! */
   }
   return self;
}

IoObject *IoList_translate(IoList *self, IoObject *locals, IoMessage *m)
{
  IoMessage_assertArgCount_receiver_(m, 3, self);
  {
    IoObject *result = IONIL(self);
    IoMessage *indexMessage = IoMessage_rawArgAt_(m, 0);
    IoMessage *valueMessage = IoMessage_rawArgAt_(m, 1);
    IoMessage *doMessage    = IoMessage_rawArgAt_(m, 2);
    
    IoString *slotName = IoMessage_rawMethod(indexMessage);
    IoNumber *index = IoObject_getSlot_(locals, slotName);
    IoObject *value;
    int i;
    if (!index || !ISNUMBER(index))
    {
      IoObject_setSlot_to_(locals, slotName, IoNumber_newWithDouble_(IOSTATE, 0));
      index = IoObject_getSlot_(locals, slotName); /* numbers copied by value! */
    }
    for (i = 0; i < List_count(DATA(self)->list); i++)
    {
      value = (IoObject *)LIST_AT_(DATA(self)->list, i);
      IoNumber_rawSet(index, i);
      IoObject_setSlot_to_(locals, IoMessage_rawMethod(valueMessage), value);
      IoState_pushRetainPool(IOSTATE);
      result = IoMessage_locals_performOn_(doMessage, locals, locals);
      IoList_rawAtPut(self, i, result);
      IoState_popRetainPool(IOSTATE);
    }
    IoState_stackRetain_(IOSTATE, result);
    return result;
  }
}
