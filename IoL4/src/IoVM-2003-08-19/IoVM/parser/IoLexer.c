/*   Copyright (c) 2003, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */
 
#include "IoLexer.h"
#include "IoToken_parser.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>

/*#define LEXER_DEBUG*/

IoLexer *IoLexer_new(void)
{
  IoLexer *self = (IoLexer *)calloc(1, sizeof(IoLexer));
  self->s = (char *)calloc(1, 1);
  self->posStack = Stack_new();
  self->tokenStack = Stack_new();
  self->tokenStream = List_new();
  return self;
}

void IoLexer_free(IoLexer *self)
{
  IoLexer_clear(self);
  free(self->s);
  Stack_free(self->posStack);
  Stack_free(self->tokenStack);
  List_free(self->tokenStream);
  free(self);
}

void IoLexer_clear(IoLexer *self)
{
  if (List_count(self->tokenStream))
  {
    IoToken *first = List_at_(self->tokenStream, 0);
    IoToken_free(first);
    List_clear(self->tokenStream);
  }
  Stack_clear(self->posStack);
  Stack_clear(self->tokenStack);
  
  self->current = self->s;
  self->resultIndex = 0;
  self->maxChar = 0;
  self->errorToken = 0x0;
}

IoToken *IoLexer_errorToken(IoLexer *self) { return self->errorToken; }

/* --- lexing --- */

void IoLexer_string_(IoLexer *self, char *string)
{
  self->s = strcpy(realloc(self->s, strlen(string)+1), string);
  self->current = self->s;
}

void IoLexer_printLast_(IoLexer *self, int max)
{
  char *s = self->s + self->maxChar;
  int i;
  for (i=0; i<max && s[i]; i++) putchar(s[i]);
}


int IoLexer_lex(IoLexer *self)
{
  IoLexer_clear(self);
  IoLexer_pushPos(self);

  IoLexer_messageChain(self);
  
  if (*(self->current)) 
  {
    /*printf("Lexing error after: ");*/
    IoLexer_printLast_(self, 30);
    printf("\n");
    if (!self->errorToken)
    {
      if (List_count(self->tokenStream)) 
      { self->errorToken = List_top(self->tokenStream); }
      else
      { self->errorToken = IoLexer_addTokenString_length_type_(self, self->current, 30, NO_TOKEN); }
     IoToken_error_(self->errorToken, "Syntax error near this location");
    }
    return -1;
  }
  return 0;
}

/* --- getting results --- */

IoToken *IoLexer_top(IoLexer *self)
{ return List_at_(self->tokenStream, self->resultIndex); }

IoTokenType IoLexer_topType(IoLexer *self)
{ 
  if (!IoLexer_top(self)) return 0;
  return IoLexer_top(self)->type; 
}

IoToken *IoLexer_pop(IoLexer *self)
{
  IoToken *t = IoLexer_top(self); 
  self->resultIndex++;
  return t;
}


/* --- stack management --- */

void IoLexer_print(IoLexer *self)
{
  IoToken *first = List_at_(self->tokenStream, 0);
  if (first) IoToken_print(first);
  printf("\n");
}

/* --- token and character position stacks --- */

char *IoLexer_lastPos(IoLexer *self)
{ return Stack_top(self->posStack); }

void IoLexer_pushPos(IoLexer *self)
{ 
  int index = self->current - self->s;
  if (index > self->maxChar) self->maxChar = index;
  
  Stack_push_(self->tokenStack, (void *)(ptrdiff_t)(List_count(self->tokenStream)-1));
  Stack_push_(self->posStack, self->current);
  
  #ifdef LEXER_DEBUG
  printf("push: "); IoLexer_print(self);
  #endif
}

void IoLexer_popPos(IoLexer *self)
{ 
  Stack_pop(self->tokenStack);
  Stack_pop(self->posStack);
  #ifdef LEXER_DEBUG
  printf("pop:  "); IoLexer_print(self);
  #endif
}

void IoLexer_popPosBack(IoLexer *self)
{ 
  ptrdiff_t i = (ptrdiff_t)Stack_pop(self->tokenStack);
  ptrdiff_t topIndex = (ptrdiff_t)Stack_top(self->tokenStack);
  
  if (i > -1)
  {
    List_setLength_(self->tokenStream, i+1);
    if (i != topIndex) /* ok to free token */
    {
      IoToken *parent = List_top(self->tokenStream);
      if (parent) 
      IoToken_nextToken_(parent, NULL); 
    }
  }
  
  self->current = Stack_pop(self->posStack); 
  #ifdef LEXER_DEBUG
  printf("back: "); IoLexer_print(self);
  #endif
}

/* --- grabbing --- */

int IoLexer_grabLength(IoLexer *self)
{
  char *s1 = IoLexer_lastPos(self);
  char *s2 = IoLexer_current(self);
  return s2 - s1;
}
  
void IoLexer_grabTokenType_(IoLexer *self, IoTokenType type)
{
  char *s1 = IoLexer_lastPos(self);
  char *s2 = IoLexer_current(self);
  size_t len = (s2 - s1);
  if (!len) { printf("IoLexer fatal error: empty token\n"); exit(1); }
  IoLexer_addTokenString_length_type_(self, s1, len, type);
}

size_t IoLexer_currentLineNumber(IoLexer *self)
{
  size_t lineNumber = 1;
  char *s = self->s;
  while (s < self->current)
  { 
    if (*s == '\n') lineNumber++;
    s++;
  }
  return lineNumber;
}

IoToken *IoLexer_addTokenString_length_type_(IoLexer *self, char *s1, size_t len, IoTokenType type)
{
  IoToken *top = List_top(self->tokenStream);
  IoToken *t = IoToken_new();
  t->lineNumber = IoLexer_currentLineNumber(self);
  /*t->charNumber = (int)(s1 - self->s);*/
  t->charNumber = (int)(self->current - self->s);
  if (t->charNumber < 0)
  {
    printf("bad t->charNumber = %i\n", t->charNumber);
  }
  IoToken_name_length_(t, s1, len);
  IoToken_type_(t, type);
  if (top) IoToken_nextToken_(top, t);
  List_push_(self->tokenStream, t);
  /*printf("token '%s' %i\n", t->name, t->type);*/
  return t;
}

/* --- next/prev character --- */

char IoLexer_nextChar(IoLexer *self)
{
  char c = *(self->current);
  if (c) self->current ++;
  return c;
}

char IoLexer_prevChar(IoLexer *self)
{
  if (self->current > self->s) self->current --;
  return *(self->current);
}

char *IoLexer_current(IoLexer *self)
{ return self->current; }

int IoLexer_onNULL(IoLexer *self)
{ return (*(self->current) == 0); }

/* --- reading --- */

void IoLexer_messageChain(IoLexer *self)
{
  while (IoLexer_readMessage(self)) 
  {
    while (IoLexer_readTerminator(self) || 
           IoLexer_readSeparator(self) || 
           IoLexer_readComment(self)) {}
  }
}

/* --- message --- */

int IoLexer_readMessage(IoLexer *self)
{
  IoLexer_pushPos(self);
  IoLexer_readPadding(self);
  if (IoLexer_readSymbol(self))
  {
    while (IoLexer_readSeparator(self) || IoLexer_readComment(self)) {}
    if (IoLexer_readTokenChar_type_(self, '(', OPENPAREN_TOKEN))
    {
      IoLexer_readPadding(self);
      do {
        IoLexer_readPadding(self);
        IoLexer_messageChain(self);
        IoLexer_readPadding(self);
      } while (IoLexer_readTokenChar_type_(self, ',', COMMA_TOKEN));
      
      if (!IoLexer_readTokenChar_type_(self, ')', CLOSEPAREN_TOKEN)) 
      {
        char c = *IoLexer_current(self);
        IoLexer_popPosBack(self);
        self->errorToken = List_top(self->tokenStream);
        if (c == '(')
        { IoToken_error_(self->errorToken, "expected a message but instead found a '('"); }
        else
        { IoToken_error_(self->errorToken, "missing closing paren for argument list"); }
        /*printf("Token %p error: %s - %s\n", t, t->error, IoToken_error(t)); */
        return 0;
      }
    }
    IoLexer_popPos(self);
    return 1;
   }
  IoLexer_popPosBack(self);
  return 0;
}

int IoLexer_readPadding(IoLexer *self)
{
  int r = 0;
  while (IoLexer_readWhitespace(self) || IoLexer_readComment(self)) { r = 1; }
  return r;
}

/* --- symbols --- */

int IoLexer_readSymbol(IoLexer *self)
{
  if (IoLexer_readNumber(self)) return 1;
  if (IoLexer_readOperator(self)) return 1;
  if (IoLexer_readIdentifier(self)) return 1;
  if (IoLexer_readQuote(self)) return 1;
  return 0;
}

int IoLexer_readIdentifier(IoLexer *self)
{
  IoLexer_pushPos(self);
  while (IoLexer_readLetter(self) ||
         IoLexer_readDigit(self) ||
         IoLexer_readSpecialChar(self))
  { }
  if (IoLexer_grabLength(self))
  {
    IoLexer_grabTokenType_(self, IDENTIFIER_TOKEN);
    IoLexer_popPos(self);
    return 1;
  }
  IoLexer_popPosBack(self);
  return 0;
}

int IoLexer_readOperator(IoLexer *self)
{
  char c;
  IoLexer_pushPos(self);
  /* ok if first character is a colon */
  c = IoLexer_nextChar(self);
  /*printf("IoLexer_nextChar(self) = %c %i\n", c, c);*/
  if (c == 0) { IoLexer_popPosBack(self); return 0; }
  if (c != ':') { IoLexer_prevChar(self); }
  while (IoLexer_readOpChar(self))
  { }
  if (IoLexer_grabLength(self))
  {
    IoLexer_grabTokenType_(self, OPERATOR_TOKEN);
    IoLexer_popPos(self);
    return 1;
  }
  IoLexer_popPosBack(self);
  return 0;
}

/* --- comments --- */

int IoLexer_readComment(IoLexer *self)
{
  if (IoLexer_readSlashStarComment(self)) return 1;
  if (IoLexer_readSlashSlashComment(self)) return 1;
  if (IoLexer_readPoundComment(self)) return 1;
  return 0;
}

int IoLexer_readSlashStarComment(IoLexer *self)
{
  IoLexer_pushPos(self);
  if (IoLexer_readString_(self, "/*"))
  {
    while (!IoLexer_readString_(self, "*/")) 
    { IoLexer_nextChar(self); }
    /*IoLexer_grabTokenType_(self, COMMENT_TOKEN);*/
    IoLexer_popPos(self);
    return 1;
  }
  IoLexer_popPosBack(self);
  return 0;
}

int IoLexer_readSlashSlashComment(IoLexer *self)
{
  IoLexer_pushPos(self);
  if (IoLexer_nextChar(self) == '/')
  {
    if (IoLexer_nextChar(self) == '/')
    {
      while (IoLexer_readNonReturn(self)) { }
      /*IoLexer_grabTokenType_(self, COMMENT_TOKEN);*/
      IoLexer_popPos(self);
      return 1;
    }
  }
  IoLexer_popPosBack(self);
  return 0;
}

int IoLexer_readPoundComment(IoLexer *self)
{
  IoLexer_pushPos(self);
  if (IoLexer_nextChar(self) == '#')
  {
    while (IoLexer_readNonReturn(self)) 
    { }
    /*IoLexer_grabTokenType_(self, COMMENT_TOKEN);*/
    IoLexer_popPos(self);
    return 1;
  }
  IoLexer_popPosBack(self);
  return 0;
}

/* --- quotes --- */

int IoLexer_readQuote(IoLexer *self)
{
  if (IoLexer_readTriQuote(self)) return 1;
  if (IoLexer_readMonoQuote(self)) return 1;
  return 0;
}

int IoLexer_readMonoQuote(IoLexer *self)
{
  IoLexer_pushPos(self);
  if (IoLexer_nextChar(self) == '"')
  {
    for (;;)
    {
      char c = IoLexer_nextChar(self);
      if (c == '"') break;
      if (c == '\\') { IoLexer_nextChar(self); continue; }
      if (c == 0x0) 
      { 
        self->errorToken = List_top(self->tokenStream);
        if (self->errorToken) IoToken_error_(self->errorToken, "unterminated quote");
        IoLexer_popPosBack(self); 
        return 0; 
      }
    }
    IoLexer_grabTokenType_(self, MONOQUOTE_TOKEN);
    IoLexer_popPos(self);
    return 1;
  }
  IoLexer_popPosBack(self);
  return 0;
}

int IoLexer_readTriQuote(IoLexer *self)
{
  IoLexer_pushPos(self);
  if (IoLexer_readString_(self, "\"\"\""))
  {
    while (!IoLexer_readString_(self, "\"\"\""))
    { 
      char c = IoLexer_nextChar(self); 
      if (c == 0x0) { IoLexer_popPosBack(self); return 0; }
    }
    IoLexer_grabTokenType_(self, TRIQUOTE_TOKEN);
    IoLexer_popPos(self);
    return 1;
  }
  IoLexer_popPosBack(self);
  return 0;
}

/* --- helpers ---------------------------- */

int IoLexer_readTokenChar_type_(IoLexer *self, char c, IoTokenType type)
{
  IoLexer_pushPos(self);
  if (IoLexer_readChar_(self, c))
  {
    IoLexer_grabTokenType_(self, type);
    IoLexer_popPos(self);
    return 1;
  }
  IoLexer_popPosBack(self);
  return 0;
}

int IoLexer_readTokenString_(IoLexer *self, const char *s)
{
  IoLexer_pushPos(self);
  if (IoLexer_readString_(self, s))
  {
    IoLexer_grabTokenType_(self, IDENTIFIER_TOKEN);
    IoLexer_popPos(self);
    return 1;
  }
  IoLexer_popPosBack(self);
  return 0;
}


int IoLexer_readString_(IoLexer *self, const char *s)
{
  int len = strlen(s);
  if (IoLexer_onNULL(self)) return 0;
  if (strncmp(self->current, s, len) == 0)
  {
    self->current += len;
    return 1;
  }
  return 0;
}

int IoLexer_readCharIn_(IoLexer *self, const char *s)
{
  if (!IoLexer_onNULL(self))
  {
    char c = IoLexer_nextChar(self);
    if (strchr(s, c)) return 1;
    IoLexer_prevChar(self);
  }
  return 0;
}

int IoLexer_readChar_(IoLexer *self, char c)
{
  if (!IoLexer_onNULL(self))
  {
    char nc = IoLexer_nextChar(self);
    if (nc && nc == c) return 1;
    IoLexer_prevChar(self);
  }
  return 0;
}

int IoLexer_readCharAnyCase_(IoLexer *self, char c)
{
  if (!IoLexer_onNULL(self))
  {
    char nc = IoLexer_nextChar(self);
    if (nc && tolower(nc) == tolower(c)) return 1;
    IoLexer_prevChar(self);
  }
  return 0;
}

int IoLexer_readNonReturn(IoLexer *self)
{
  if (IoLexer_onNULL(self)) return 0;
  if (IoLexer_nextChar(self) != '\n') return 1;
  IoLexer_prevChar(self);
  return 0;
}

int IoLexer_readNonQuote(IoLexer *self)
{
  if (IoLexer_onNULL(self)) return 0;
  if (IoLexer_nextChar(self) != '"') return 1;
  IoLexer_prevChar(self);
  return 0;
}

/* --- character definitions ---------------------------- */

int IoLexer_readCharacters(IoLexer *self)
{
  int read = 0;
  while (IoLexer_readCharacter(self)) { read = 1; }
  return read;
}

int IoLexer_readCharacter(IoLexer *self)
{
  if (IoLexer_readLetter(self)) return 1;
  if (IoLexer_readDigit(self)) return 1;
  if (IoLexer_readSpecialChar(self)) return 1;
  if (IoLexer_readOpChar(self)) return 1;
  return 0;
}

int IoLexer_readOpChar(IoLexer *self)
{ return IoLexer_readCharIn_(self, "'~!@$%^&*-+={}[]|\\<>?/"); }

int IoLexer_readSpecialChar(IoLexer *self)
{ return IoLexer_readCharIn_(self, ":._"); }

int IoLexer_readDigit(IoLexer *self)
{ return IoLexer_readCharIn_(self, "0123456789"); }

int IoLexer_readLetter(IoLexer *self)
{ return IoLexer_readCharIn_(self, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"); }

/* --- terminator --- */

int IoLexer_readTerminator(IoLexer *self)
{
  int terminated = 0;
  IoLexer_pushPos(self);
  IoLexer_readSeparator(self);
  
  while (IoLexer_readTerminatorChar(self))
  { terminated = 1; IoLexer_readSeparator(self); }
  
  if (terminated)
  {
    IoToken *top = List_top(self->tokenStream);
    if (top && IoToken_type(top) == TERMINATOR_TOKEN) { return 1; } /*avoid double terminators */
    IoLexer_addTokenString_length_type_(self, ";", 1, TERMINATOR_TOKEN);
    IoLexer_popPos(self);
    return 1;
  }
  IoLexer_popPosBack(self);
  return 0;
}

int IoLexer_readTerminatorChar(IoLexer *self)
{ return IoLexer_readCharIn_(self, ";\n"); }

/* --- separator --- */

int IoLexer_readSeparator(IoLexer *self)
{
  IoLexer_pushPos(self);
  while (IoLexer_readSeparatorChar(self)) { }
  if (IoLexer_grabLength(self))
  {
    /*IoLexer_grabTokenType_(self, SEPERATOR_TOKEN);*/
    IoLexer_popPos(self);
    return 1;
  }
  IoLexer_popPosBack(self);
  return 0;
}

int IoLexer_readSeparatorChar(IoLexer *self)
{ return IoLexer_readCharIn_(self, " \f\r\t\v"); }

/* --- whitespace --- */

int IoLexer_readWhitespace(IoLexer *self)
{
  IoLexer_pushPos(self);
  while (IoLexer_readWhitespaceChar(self)) { }
  if (IoLexer_grabLength(self))
  {
    /*IoLexer_grabTokenType_(self, WHITESPACE_TOKEN);*/
    IoLexer_popPos(self);
    return 1;
  }
  IoLexer_popPosBack(self);
  return 0;
}

int IoLexer_readWhitespaceChar(IoLexer *self)
{ return IoLexer_readCharIn_(self, " \f\r\t\v\n"); }

/* --- whitespace --- */

int IoLexer_readDigits(IoLexer *self)
{
  int read = 0;
  IoLexer_pushPos(self);
  while (IoLexer_readDigit(self)) { read = 1; }
  if (!read) { IoLexer_popPosBack(self); return 0; }
  IoLexer_popPos(self);
  return read;
}

int IoLexer_readNumber(IoLexer *self)
{ return (IoLexer_readHexNumber(self) || IoLexer_readDecimal(self)); }

int IoLexer_readExponent(IoLexer *self)
{
  if (IoLexer_readCharAnyCase_(self, 'e')) 
  { 
    IoLexer_readChar_(self, '-');
    if (!IoLexer_readDigits(self)) return -1;
    return 1;
  }  
  return 0;
}

int IoLexer_readDecimalPlaces(IoLexer *self)
{
  if (IoLexer_readChar_(self, '.'))
  { 
    if (!IoLexer_readDigits(self)) return -1; 
    return 1;
  }
  return 0;
}

int IoLexer_readDecimal(IoLexer *self)
{
  IoLexer_pushPos(self);
  if (IoLexer_readDigits(self))
  { if (IoLexer_readDecimalPlaces(self) == -1) goto error; }
  else
  { if (IoLexer_readDecimalPlaces(self) != 1) goto error; }
  
  if (IoLexer_readExponent(self) == -1) goto error;
    
  if (IoLexer_grabLength(self))
  {
      IoLexer_grabTokenType_(self, NUMBER_TOKEN);
      IoLexer_popPos(self);
      return 1;
  }  
  error:
    IoLexer_popPosBack(self);
  return 0;
}

int IoLexer_readHexNumber(IoLexer *self)
{
  int read = 0;
  IoLexer_pushPos(self);
  if (IoLexer_readChar_(self, '0') && IoLexer_readCharAnyCase_(self, 'x'))
  {
    while (IoLexer_readDigits(self) || IoLexer_readCharacters(self))
   { read ++; }
  }

  if (read  && IoLexer_grabLength(self))
  {
    IoLexer_grabTokenType_(self, HEXNUMBER_TOKEN);
    IoLexer_popPos(self);
    return 1;
  }
  IoLexer_popPosBack(self);
  return 0;
}

/* --- parsing ------------------------------------------- */

IoToken *IoLexer_rootMessageToken(IoLexer *self, IoToken **error) 
{
  IoToken *root = IoToken_popFromLexer_andSetParent_(self, NULL);
  if (!root) return self->errorToken;
  IoToken_read(root, self, error);
  if (*error && !strcmp((*error)->error, "EOF")) { *error = 0x0; }
  /*printf("parsed: "); IoToken_printParsed(root); printf("\n");*/
  if (*error) { printf("Parse error: %s\n", (*error)->error); }
  return root;
}

