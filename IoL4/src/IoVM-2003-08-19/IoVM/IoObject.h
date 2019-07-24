/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#ifndef OBJECT_DEFINED
#define OBJECT_DEFINED 1

#include "base/Common.h"
#include "base/PHash.h"
#include "IoStoreHandle.h"

/*
#define IOOBJECT_SETSLOT_TO_(self, slotName, value) \
  if (!self->mark.ownsSlots) \
  {\
    self->slots = PHash_new();\
    self->mark.ownsSlots = 1;\
    PHash_at_put_(self->slots, IOSTATE->protoString, self->proto);\
  }\
  if (ISNUMBER(value)) value = (void *)IoNumber_newCopyOf_((IoNumber *)value);\
  if(PHash_at_put_(self->slots, IOREF(slotName), IOREF(value)))\
  { self->mark.isDirty = 1; }\
  \
  if (slotName == IOSTATE->protoString) self->proto = value;
*/

#define IoObject_clean(self) PHash_clean(self->slots);
#define IOREF(value) IoObject_addingRef_((IoObject *)self, (IoObject *)value)

#include "IoObject_struct.h"

/* --- inline these --- */

void IoObject_mark(IoObject *self);
void IoObject_makeGrayIfWhite(IoObject *self);
IoObject *IoObject_addingRef_(IoObject *self, IoObject *ref);

#include "IoMessage.h"
#include "IoString.h"
#include "IoCoroutine.h"

IoObject *IoObject_proto(void *state);
IoObject *IoObject_protoFinish(void *state);
IoObject *IoObject_rawClone(IoObject *self);
IoObject *IoObject_justClone(IoObject *self);
IoObject *IoObject_rawClonePrimitive(IoObject *self);

IoObject *IoObject_new(void *state);

void IoObject_addMethod_(IoObject *self, IoString *slotName, void *fp);

void IoObject_dealloc(IoObject *self);
void IoObject_free(IoObject *self);

int IoObject_compare(IoObject *self, IoObject *v);
IoObject *IoObject_activate(IoObject *self, IoObject *target, IoObject *locals, IoMessage *m);

/* ----------------------------------------------------------- */
void IoObject_setSlot_to_(IoObject *self, IoString *slotName, IoObject *value);

IoObject *IoObject_rawGetSlot_(IoObject *self, IoString *slotName);
IoObject *IoObject_getSlot_(IoObject *self, IoString *slotName);
IoObject *IoObject_objectWithSlotValue_(IoObject *self, IoObject *slotValue);
void IoObject_removeSlot_(IoObject *self, IoString *slotName);

/* -- perform ---------------------------------------------------- */
IoObject *IoObject_perform(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_protoPerformWithArgList(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_forward(IoObject *self, IoObject *locals, IoMessage *m);

/* -------------------------------------------------------- */
const char *IoObject_name(IoObject *self);
void IoObject_print(IoObject *self);
size_t IoObject_memorySize(IoObject *self);
void IoObject_compact(IoObject *self);
void IoObject_printFunc(IoObject *self);

/* -- proto methods ---------------------------------------------- */
IoObject *IoObject_protoPerform(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_protoPrint(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_protoWrite(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_clone(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_shallowCopy(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_rawClone(IoObject *self);
IoObject *IoObject_initClone_(IoObject *self, IoObject *locals, IoMessage *m, IoObject *newObject);

IoObject *IoObject_protoSet_to_(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_protoUpdateSlot_to_(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_protoGet_(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_protoHasSlot(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_protoRemoveSlot(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_protoSlotNames(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoObject_protoBlock(IoObject *self, IoObject *locals, IoMessage *m);
/*IoObject *IoObject_protoForward_(IoObject *self, IoObject *locals, IoMessage *m);*/
IoObject *IoObject_protoWhile(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_protoFor(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_protoIf(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_collectGarbage(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoObject_return(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_break(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_continue(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoObject_exit(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_protoForward_(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_super(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_rawDoString_label_(IoObject *self, IoString *string, IoString *label);
IoObject *IoObject_doMessage(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_doString(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_doFile(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoObject_equals(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_notEquals(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoObject_catchException(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_raiseException(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_actorYield(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_actorResume(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_actorPause(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoObject_memorySizeOfState(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_compactState(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_getenv(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_foreach(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_try(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_subtract(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_system(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_self(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_compileString(IoObject *self, IoObject *locals, IoMessage *m);

/* --- stuff that used to be in IoValue ----------------------- */
/* --- message callbacks -------------------------------------- */

IoObject *IoObject_activateFunc(IoObject *self, IoObject *target, IoObject *locals, IoMessage *m);
int IoObject_sortCompare(IoObject **self, IoObject **v);

/* -- lobby methods ---------------------------------------------- */

IoObject *IoObject_memorySizeMethod(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_compactMethod(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_type(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_lobbyPrint(IoObject *self, IoObject *locals, IoMessage *m);

void IoObject_defaultPrint(IoObject *self);

IoObject *IoObject_evalArg(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_nop(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoObject_and(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_or(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoObject_isLessThan_(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_isLessThanOrEqualTo_(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_isGreaterThan_(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_isGreaterThanOrEqualTo_(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoObject_uniqueId(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_do(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoObject_isNil(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_ifNil(IoObject *self, IoObject *locals, IoMessage *m);

IoObject *IoObject_ifTrue(IoObject *self, IoObject *locals, IoMessage *m);
IoObject *IoObject_ifFalse(IoObject *self, IoObject *locals, IoMessage *m);

char *IoObject_markColorName(IoObject *self);

#endif
