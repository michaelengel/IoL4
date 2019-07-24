/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#include "IoMessage_parser.h"
#include "IoObject.h"
#include "IoNil.h"
#include "IoString.h"
#include "IoNumber.h"
#include "IoState.h"
#include "parser/IoLexer.h"
#include "parser/IoToken_parser.h"
#include <ctype.h>

#define DATA(self) ((IoMessageData *)self->data)

void IoMessage_cacheIfPossible(IoMessage *self) 
{
  IoMessage *m = IoMessage_newFromText_label_(IOSTATE, CSTRING(DATA(self)->method), CSTRING(DATA(self)->label));
  DATA(self)->cachedResult = IOREF(DATA(m)->cachedResult);
}

void IoMessage_ifPossibleCacheToken_(IoMessage *self, IoToken *p) 
{
  IoString *method = DATA(self)->method;
  /*printf("%s %i\n", CSTRING(method), p->token->type);*/
  switch ((int)IoToken_type(p))
  {
    case TRIQUOTE_TOKEN:
      DATA(self)->cachedResult = IoString_newWithCString_length_(IOSTATE, 
        IoString_asCString(method)+3, IoString_cStringLength(method)-6); 
      break;
    case MONOQUOTE_TOKEN:
      IoMessage_cachedResult_(self, IoString_rawUnescaped(IoString_rawUnquoted(DATA(self)->method))); 
      break;
    case NUMBER_TOKEN:
      IoMessage_cachedResult_(self, IONUMBER(IoString_asDouble(method)));
      break;
    case HEXNUMBER_TOKEN:
      IoMessage_cachedResult_(self, IONUMBER(IoString_asDoubleFromHex(method)));
      break;
    default:
      if (IoString_equalsCString_(method, "Nil"))
      { IoMessage_cachedResult_(self, IONIL(self)); }
  }
}

IoMessage *IoMessage_newFromIoToken_(void *state, IoToken *p) 
{
  IoMessage *m = IoMessage_newWithName_(state, IoState_stringWithCString_(state, IoToken_name(p)));
  int i;
  for (i = 0; i < List_count(p->args); i++)
  {
    IoToken *parg = List_at_(p->args, i);
    IoMessage_addArg_(m, IoMessage_newFromIoToken_(state, parg));
  }
  if (p->attached) DATA(m)->attachedMessage = IoMessage_newFromIoToken_(state, p->attached);
  if (p->next) DATA(m)->nextMessage = IoMessage_newFromIoToken_(state, p->next);
  
  IoMessage_ifPossibleCacheToken_(m, p);
  DATA(m)->lineNumber = IoToken_lineNumber(p);
  DATA(m)->charNumber = IoToken_charNumber(p);
  return m;
}

IoMessage *IoMessage_newFromText_label_(void *state, char *text, char *label)
{
  IoToken *error = 0x0;
  IoString *errorString = 0x0;
  IoString *slabel = IoState_stringWithCString_(state, label);
  IoMessage *m;
  IoToken *p;
  IoLexer *lexer = IoLexer_new();
  IoLexer_string_(lexer, text);
  IoLexer_lex(lexer);

  if (IoLexer_errorToken(lexer))
  { 
    m = IoMessage_newFromIoToken_(state, IoLexer_errorToken(lexer)); 
    IoMessage_label_(m, IoState_stringWithCString_(state, "[lexer]"));
    errorString = IoState_stringWithCString_(state, IoToken_error(IoLexer_errorToken(lexer)));
  }
  else
  {
    p = IoLexer_rootMessageToken(lexer, &error);
    if (error) p = error;
    if (!p) return IoMessage_newWithName_(state, IoState_stringWithCString_(state, "Nil"));
    m = IoMessage_newFromIoToken_(state, p);
    if (error) 
    { IoMessage_label_(m, IoState_stringWithCString_(state, "[parser]")); } 
    else 
    { IoMessage_label_(m, slabel); }
    if (error) errorString = IoState_stringWithCString_(state, error->error);
  }
  IoLexer_free(lexer);

  if (errorString) IoState_error_description_(state, m, "Io.compiler", "%s", CSTRING(errorString));
  
  if (!(IoMessage_rawMethod(m)))
  {
    IoState_error_description_(state, 0x0, "Io.compiler.internal", "message with no method");
  }
  /*IoMessage_markTails(m);*/
  return m;
}

/*
void IoMessage_findTails(IoMessage *self, char onTail)
{
  IoState *state = IOSTATE;
  
  if ((DATA(self)->method == state->returnString) && self->attachedMessage)
  { IoMessage_findTails(self->attachedMessage, 1); }
  
  if (self->nextMessage)     { IoMessage_findTails(self->nextMessage, onTail); }
  if (self->attachedMessage) { IoMessage_findTails(self->attachedMessage, onTail && !self->nextMessage); }
  
  else if ((!self->nextMessage) && (!self->attachedMessage) && onTail)
  { 
    printf("marking %s as tail\n", CSTRING(DATA(self)->method));
    self->info.isTail = 1; 
  }
}

void IoMessage_markTails(IoMessage *self)
{ IoMessage_findTails(self, 1); }
*/


