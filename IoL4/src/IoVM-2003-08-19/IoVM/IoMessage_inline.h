/* Copyright (c) 2002, Steve Dekorte
 * All rights reserved. See _BSDLicense.txt.
 */


#ifdef IOMESSAGE_C 
#define IO_IN_C_FILE
#endif
#include "base/Common_inline.h"
#ifdef IO_DECLARE_INLINES

/*
#include "IoObject.h"

#define IOMESSAGEDATA(self) ((IoMessageData *)self->data)

IOINLINE IoString *IoMessage_rawMethod(IoMessage *self)
{ return IOMESSAGEDATA(self)->method; }

IOINLINE void IoMessage_addArg_(IoMessage *self, IoMessage *m)
{ List_add_(IOMESSAGEDATA(self)->args, IOREF(m)); }

IOINLINE IoMessage *IoMessage_rawArgAt_(IoMessage *self, int n)
{ return List_at_(IOMESSAGEDATA(self)->args, n); }

IOINLINE IoString *IoMessage_rawLabel(IoMessage *self)
{ return IOMESSAGEDATA(self)->label; }

IOINLINE List *IoMessage_rawArgs(IoMessage *self)
{ return IOMESSAGEDATA(self)->args; }
*/

#undef IO_IN_C_FILE
#endif
