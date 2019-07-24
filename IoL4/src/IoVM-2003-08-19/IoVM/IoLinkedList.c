/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#include "IoLinkedList.h"
#include "IoState.h"
#include "IoCFunction.h"
#include "IoNil.h"
#include "IoString.h"
#include "IoState.h"
#include "IoNumber.h"
#include "IoBlock.h"
#include <math.h>

#define DATA(self) ((IoLinkedListData *)self->data)

IoTag *IoLinkedList_tag(void *state)
{
  IoTag *tag = IoTag_newWithName_("LinkedList");
  tag->state = state;
  tag->cloneFunc   = (TagCloneFunc *)IoLinkedList_rawClone;
  tag->freeFunc    = (TagFreeFunc *)IoLinkedList_free;
  tag->markFunc    = (TagMarkFunc *)IoLinkedList_mark;
  tag->compareFunc = (TagCompareFunc *)IoLinkedList_compare;
  return tag;
}

IoLinkedList *IoLinkedList_proto(void *state)
{
  IoObject *self = IoObject_new(state);
  self->tag = IoLinkedList_tag(state);
  
  self->data = calloc(1, sizeof(IoLinkedListData));
  DATA(self)->list = LinkedList_new();
  IoState_registerProtoWithFunc_(state, self, IoLinkedList_proto);
  
  IoObject_addMethod_(self, IOSTRING("add"), (void *)IoLinkedList_add_);
  IoObject_addMethod_(self, IOSTRING("remove"), (void *)IoLinkedList_remove_);
  IoObject_addMethod_(self, IOSTRING("indexOf"), (void *)IoLinkedList_indexOf_);
  IoObject_addMethod_(self, IOSTRING("contains"), (void *)IoLinkedList_contains_);
  IoObject_addMethod_(self, IOSTRING("push"), (void *)IoLinkedList_push_);
  IoObject_addMethod_(self, IOSTRING("pop"), (void *)IoLinkedList_pop);
  IoObject_addMethod_(self, IOSTRING("empty"), (void *)IoLinkedList_clear);
  IoObject_addMethod_(self, IOSTRING("count"), (void *)IoLinkedList_count);
  IoObject_addMethod_(self, IOSTRING("insertAt"), (void *)IoLinkedList_insert_at_);
  IoObject_addMethod_(self, IOSTRING("removeAt"), (void *)IoLinkedList_removeAt_);
  IoObject_addMethod_(self, IOSTRING("at"), (void *)IoLinkedList_at_);
  IoObject_addMethod_(self, IOSTRING("atPut"), (void *)IoLinkedList_at_put_);
  IoObject_addMethod_(self, IOSTRING("swapIndexWithIndex"), (void *)IoLinkedList_swapIndex_withIndex_);
  IoObject_addMethod_(self, IOSTRING("doBlock"), (void *)IoLinkedList_do_);
  IoObject_addMethod_(self, IOSTRING("sort"), (void *)IoLinkedList_sort);
  IoObject_addMethod_(self, IOSTRING("sortBy"), (void *)IoLinkedList_sortBy_);
  IoObject_addMethod_(self, IOSTRING("foreach"), (void *)IoLinkedList_foreach);
  IoObject_addMethod_(self, IOSTRING("print"), (void *)IoLinkedList_protoPrint);
  IoObject_addMethod_(self, IOSTRING("reverse"), (void *)IoLinkedList_reverse);
  IoObject_addMethod_(self, IOSTRING("reverseForeach"), (void *)IoLinkedList_reverseForeach);
  IoObject_addMethod_(self, IOSTRING("random"), (void *)IoLinkedList_random);
  IoObject_addMethod_(self, IOSTRING("randomize"), (void *)IoLinkedList_randomize);
  IoObject_addMethod_(self, IOSTRING("translate"), (void *)IoLinkedList_translate);
  return self;
}

IoLinkedList *IoLinkedList_rawClone(IoLinkedList *self)
{
  IoObject *child = IoObject_rawClonePrimitive(self);
  child->data = calloc(1, sizeof(IoLinkedListData));
  DATA(child)->list = LinkedList_clone(DATA(self)->list);
  return child;
}

IoLinkedList *IoLinkedList_new(void *state)
{
  IoObject *proto = IoState_protoWithInitFunction_(state, IoLinkedList_proto);
  return IoLinkedList_rawClone(proto);
}

void IoLinkedList_free(IoLinkedList *self) 
{ 
  LinkedList_free(DATA(self)->list);
  free(self->data);
}

void IoLinkedList_mark(IoLinkedList *self) 
{ LinkedList_do_(DATA(self)->list, (ListDoCallback *)IoObject_makeGrayIfWhite); }

int IoLinkedList_compare(IoLinkedList *self, IoLinkedList *otherList)
{
  if(!ISLINKEDLIST(otherList)) 
  { return -(self->tag - otherList->tag); }
  return LinkedList_compare_(DATA(self)->list, DATA(otherList)->list, (LinkedListCompareCallback *)IoObject_compare);
}

int IoLinkedList_rawIndexOf_(IoLinkedList *self, IoObject *v)
{
  int i;
  for (i=0; i< LinkedList_count(DATA(self)->list); i++)
  {
    IoObject *item = LinkedList_at_(DATA(self)->list, i); /* this is slow - move to using LinkedListItem */
    if (IoObject_compare(v, item) == 0) return i;
  }
  return -1;
}

/* ----------------------------------------------------------- */

IoObject *IoLinkedList_add_(IoLinkedList *self, IoObject *locals, IoMessage *m)
{
  int n, max = IoMessage_argCount(m);
  for (n=0; n < max; n++)
  {
    IoMessage *argm = IoMessage_rawArgAt_(m, n);
    IoObject *v = IoMessage_locals_performOn_(argm, locals, locals); 
    IoState_stackRetain_(IOSTATE, v);
  
    if (ISNUMBER(v))
    { LinkedList_add_(DATA(self)->list, IOREF(IoNumber_newCopyOf_((IoNumber *)v))); }
    else
    { LinkedList_add_(DATA(self)->list, IOREF(v)); }
  }
  return self;

}

IoObject *IoLinkedList_remove_(IoLinkedList *self, IoObject *locals, IoMessage *m)
{
  IoObject *v = IoMessage_locals_valueArgAt_(m, locals, 0);
  int i = IoLinkedList_rawIndexOf_(self, v); /* slow - use LinkedListItem */
  if (i != -1) LinkedList_removeIndex_(DATA(self)->list, i); 
  return self;
}

IoObject *IoLinkedList_indexOf_(IoLinkedList *self, IoObject *locals, IoMessage *m)
{
  IoObject *v = IoMessage_locals_valueArgAt_(m, locals, 0);
  int i = IoLinkedList_rawIndexOf_(self, v); /* slow - use LinkedListItem */
  return i == -1 ? IONIL(self) :
    IoNumber_newWithDouble_(IOSTATE, (double)i);
}

IoObject *IoLinkedList_contains_(IoLinkedList *self, IoObject *locals, IoMessage *m)
{
  IoObject *v = IoMessage_locals_valueArgAt_(m, locals, 0);
  int index = LinkedList_indexOf_(DATA(self)->list, v);
  if (index == -1) { return IONIL(self); }
  return self;
}

IoObject *IoLinkedList_push_(IoLinkedList *self, IoObject *locals, IoMessage *m)
{
  IoObject *v = IoMessage_locals_valueArgAt_(m, locals, 0);
  LinkedList_push_(DATA(self)->list, IOREF(v));
  return v;
}

IoObject *IoLinkedList_pop(IoLinkedList *self, IoObject *locals, IoMessage *m)
{
  IoObject *v = LinkedList_pop(DATA(self)->list);
  if (!v) { return IONIL(self); }
  return v;
}

IoObject *IoLinkedList_count(IoLinkedList *self, IoObject *locals, IoMessage *m)
{ return IoNumber_newWithDouble_(IOSTATE, LinkedList_count(DATA(self)->list)); }

IoObject *IoLinkedList_insert_at_(IoLinkedList *self, IoObject *locals, IoMessage *m)
{
  IoObject *v = IoMessage_locals_valueArgAt_(m, locals, 0);
  int index = IoMessage_locals_intArgAt_(m, locals, 1);
  LinkedList_at_insert_(DATA(self)->list, index, IOREF(v));
  return v;
}

IoObject *IoLinkedList_removeAt_(IoLinkedList *self, IoObject *locals, IoMessage *m)
{
  int index = IoMessage_locals_intArgAt_(m, locals, 0);
  IoObject *v = LinkedList_removeIndex_(DATA(self)->list, index);
  return (!v) ? IONIL(self) : v;
}

IoObject *IoLinkedList_at_(IoLinkedList *self, IoObject *locals, IoMessage *m)
{
  int index = IoMessage_locals_intArgAt_(m, locals, 0);
  if (index < DATA(self)->list->count)
  { 
    IoObject *v = LinkedList_at_(DATA(self)->list, index); 
    if (v) return v;
  }
  return IONIL(self);
}

IoObject *IoLinkedList_at_put_(IoLinkedList *self, IoObject *locals, IoMessage *m)
{
  int index = IoMessage_locals_intArgAt_(m, locals, 0);
  IoObject *v = IoMessage_locals_valueArgAt_(m, locals, 1);
  if (ISNUMBER(v))
  { LinkedList_at_put_(DATA(self)->list, index, IOREF(IoNumber_newCopyOf_((IoNumber *)v))); }
  else
  { LinkedList_at_put_(DATA(self)->list, index, IOREF(v)); }
  return v;
}

IoObject *IoLinkedList_clear(IoLinkedList *self, IoObject *locals, IoMessage *m)
{
  LinkedList_clear(DATA(self)->list);
  return self;
}

IoObject *IoLinkedList_swapIndex_withIndex_(IoLinkedList *self, IoObject *locals, IoMessage *m)
{
  int i = IoMessage_locals_intArgAt_(m, locals, 0);
  int j = IoMessage_locals_intArgAt_(m, locals, 1);
  LinkedList_swap_with_(DATA(self)->list, i, j);
  return self;
}

IoObject *IoLinkedList_reverse(IoLinkedList *self, IoObject *locals, IoMessage *m)
{ LinkedList_reverse(DATA(self)->list); return self; }

IoObject *IoLinkedList_sort(IoLinkedList *self, IoObject *locals, IoMessage *m)
{
  LinkedList_qsort(DATA(self)->list, (ListSortCallback *)IoObject_sortCompare);
  return self;
}

IoObject *IoLinkedList_sortBy_(IoLinkedList *self, IoObject *locals, IoMessage *m)
{
  /* crummy sort */
  IoBlock *block = IoMessage_locals_blockArgAt_(m, locals, 0);
  IoMessage *blockMsg = IoMessage_new(IOSTATE);
  IoMessage *argMsg1  = IoMessage_new(IOSTATE);
  IoMessage *argMsg2  = IoMessage_new(IOSTATE);
  int i;
  IoMessage_addArg_(blockMsg, argMsg1);
  IoMessage_addArg_(blockMsg, argMsg2);
  for (i = 0; i < LinkedList_count(DATA(self)->list); i++)
  {
    int done = 1;
    int j;
    IoState_pushRetainPool(IOSTATE);
    for (j = 0; j < LinkedList_count(DATA(self)->list)-1; j++)
    {
      IoObject *iValue = LinkedList_at_(DATA(self)->list, j);
      IoObject *jValue = LinkedList_at_(DATA(self)->list, j+1);
      IoMessage_cachedResult_(argMsg1, iValue);
      IoMessage_cachedResult_(argMsg2, jValue);
      /* this could be optimized a bunch */
      {
        IoObject *cr = IoBlock_target_locals_call_(block, locals, locals, blockMsg);
        if (ISNIL(cr))
        { LinkedList_swap_with_(DATA(self)->list, j, j + 1); done = 0; }
      }
    }
    IoState_popRetainPool(IOSTATE);
    if (done) break;
  }
  return self;
}

IoObject *IoLinkedList_do_(IoLinkedList *self, IoObject *locals, IoMessage *m)
{
  IoBlock *block = IoMessage_locals_blockArgAt_(m, locals, 0);
  IoMessage *blockMsg = IoMessage_new(IOSTATE);
  IoMessage *argMsg   = IoMessage_new(IOSTATE);
  int i;
  IoMessage_addArg_(blockMsg, argMsg);
  for (i = 0; i < LinkedList_count(DATA(self)->list); i++)
  {
    IoObject *v = LinkedList_at_(DATA(self)->list, i);
    IoMessage_cachedResult_(argMsg, v);
    IoBlock_target_locals_call_(block, locals, locals, blockMsg);
  }
  return self;
}

IoObject *IoLinkedList_foreach(IoLinkedList *self, IoObject *locals, IoMessage *m)
{
  IoMessage_assertArgCount_receiver_(m, 3, self);
  {
    IoMessage *indexMessage = IoMessage_rawArgAt_(m, 0);
    IoMessage *valueMessage = IoMessage_rawArgAt_(m, 1);
    IoMessage *doMessage    = IoMessage_rawArgAt_(m, 2);
    IoObject *result = IONIL(self);
    
    IoString *slotName = IoMessage_rawMethod(indexMessage);
    IoNumber *index = IoObject_getSlot_(locals, slotName);
    int i;
    if (!index || !ISNUMBER(index))
    {
      IoObject_setSlot_to_(locals, slotName, IoNumber_newWithDouble_(IOSTATE, 0));
      index = IoObject_getSlot_(locals, slotName); /* numbers  copied by value! */
    }
    for (i = 0; i < LinkedList_count(DATA(self)->list); i++)
    {
      IoObject *value = LinkedList_at_(DATA(self)->list, i);
      IoNumber_rawSet(index, i);
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
}

IoObject *IoLinkedList_protoPrint(IoLinkedList *self, IoObject *locals, IoMessage *m)
{
  IoString *separator = IOSTRING(", ");
  LinkedListItem *i;
  
  if (IoMessage_argCount(m) > 0)
  { separator = IoMessage_locals_stringArgAt_(m, locals, 0); }

  for (i = DATA(self)->list->first; i; i = i->next)
  {
    IoObject_print(i->value);
    if (i->next) 
    { IoState_justPrint_(IOSTATE, CSTRING(separator)); }
  }
  return self;
}

IoObject *IoLinkedList_reverseForeach(IoLinkedList *self, IoObject *locals, IoMessage *m)
{
  IoMessage_assertArgCount_receiver_(m, 3, self);
  {
    IoMessage *indexMessage = IoMessage_rawArgAt_(m, 0);
    IoMessage *valueMessage = IoMessage_rawArgAt_(m, 1);
    IoMessage *doMessage    = IoMessage_rawArgAt_(m, 2);
    
    IoString *slotName = IoMessage_rawMethod(indexMessage);
    IoNumber *index = IoObject_getSlot_(locals, slotName);
    int i;
    if (!index || !ISNUMBER(index))
    {
      IoObject_setSlot_to_(locals, slotName, IoNumber_newWithDouble_(IOSTATE, 0));
      index = IoObject_getSlot_(locals, slotName); /* numbers  copied by value! */
    }
    for (i = LinkedList_count(DATA(self)->list)-1; i >=0; i--)
    {
      IoObject *value = LinkedList_at_(DATA(self)->list, i);
      IoNumber_rawSet(index, i);
      IoObject_setSlot_to_(locals, IoMessage_rawMethod(valueMessage), value);
      IoMessage_locals_performOn_(doMessage, locals, locals);
    }
    return IONIL(self);
  }
}

IoObject *IoLinkedList_random(IoLinkedList *self, IoObject *locals, IoMessage *m)
{ 
  if (!LinkedList_count(DATA(self)->list)) 
  { return IONIL(self); }
  else
  {
    int i = ((LinkedList_count(DATA(self)->list)-1)*rand())/RAND_MAX;
    return LinkedList_at_(DATA(self)->list, i);
  }
}

IoObject *IoLinkedList_randomize(IoLinkedList *self, IoObject *locals, IoMessage *m)
{ 
  int i;
  int max = LinkedList_count(DATA(self)->list) - 1;
  for (i = 0; i <= max; i++)
  {
    int j = (max*rand())/RAND_MAX;
    LinkedList_swap_with_(DATA(self)->list, i, j);
  }
  return self;
}

IoObject *IoLinkedList_translate(IoLinkedList *self, IoObject *locals, IoMessage *m)
{
  IoMessage_assertArgCount_receiver_(m, 3, self);
  {
    IoObject *result = IONIL(self);
    IoMessage *indexMessage = IoMessage_rawArgAt_(m, 0);
    IoMessage *valueMessage = IoMessage_rawArgAt_(m, 1);
    IoMessage *doMessage    = IoMessage_rawArgAt_(m, 2);
    
    IoString *slotName = IoMessage_rawMethod(indexMessage);
    IoNumber *index = IoObject_getSlot_(locals, slotName);
    int i;
    LinkedListItem *item;

    if (!index || !ISNUMBER(index))
    {
      IoObject_setSlot_to_(locals, slotName, IoNumber_newWithDouble_(IOSTATE, 0));
      index = IoObject_getSlot_(locals, slotName); /* numbers copied by value! */
    }

    i = 0;
    item = DATA(self)->list->first;
    while(item)
    {
        IoObject *value = item->value;
        IoNumber_rawSet(index, i);
        IoObject_setSlot_to_(locals, IoMessage_rawMethod(valueMessage), value);
        result = IoMessage_locals_performOn_(doMessage, locals, locals);
        item->value = result;
        i++;
        item = item->next;
    }
    return result;
  }
}
