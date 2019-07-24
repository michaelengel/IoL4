/*   Copyright (c) 2003, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */
 
#ifndef IOTOKEN_DEFINED
#define IOTOKEN_DEFINED 1

#include "base/List.h"
#include <string.h>

typedef enum { 
  NO_TOKEN, 
  
  OPENPAREN_TOKEN, 
  COMMA_TOKEN, 
  CLOSEPAREN_TOKEN, 
  
  MONOQUOTE_TOKEN, 
  TRIQUOTE_TOKEN, 
  
  OPERATOR_TOKEN, 
  IDENTIFIER_TOKEN, 
  TERMINATOR_TOKEN, 
  
  COMMENT_TOKEN,
  NUMBER_TOKEN, 
  HEXNUMBER_TOKEN 
} IoTokenType;
 
typedef struct IoToken IoToken;

struct IoToken
{
  char *name;
  size_t length;
  IoTokenType type;
  int charNumber;
  int lineNumber;
  IoToken *nextToken;
  
  /* parsed */
  IoToken *parent;
  List *args;
  IoToken *attached;
  IoToken *next;
  unsigned char openParen;
  char *error;
};

IoToken *IoToken_new(void);
void IoToken_free(IoToken *self);

void IoToken_name_length_(IoToken *self, char *name, size_t len);
void IoToken_name_(IoToken *self, char *name);
char *IoToken_name(IoToken *self);
int IoToken_nameIs_(IoToken *self, char *name);
IoTokenType IoToken_type(IoToken *self);

int IoToken_lineNumber(IoToken *self);
int IoToken_charNumber(IoToken *self);

void IoToken_quoteName_(IoToken *self, char *name);
void IoToken_type_(IoToken *self, IoTokenType type);
void IoToken_nextToken_(IoToken *self, IoToken *nextToken);
void IoToken_print(IoToken *self);
void IoToken_printSelf(IoToken *self);

#endif
