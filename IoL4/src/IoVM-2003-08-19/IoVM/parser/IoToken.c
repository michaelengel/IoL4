/*   Copyright (c) 2003, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */
 
#include "IoToken.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

IoToken *IoToken_new(void)
{
  IoToken *self = (IoToken *)calloc(1, sizeof(IoToken));
  self->name = 0x0;
  self->charNumber = -1;
  /* parsed */
  self->args = List_new();
  return self;
}

void IoToken_free(IoToken *self)
{
  if (self->nextToken) IoToken_free(self->nextToken);
  if (self->name) free(self->name);
  if (self->error) free(self->error);
  
  /* parsed */
  /*
  List_do_(self->args, (ListDoCallback *)IoToken_free);
  */
  List_free(self->args);
  /*
  if (self->attached) IoToken_free(self->attached);
  if (self->next) IoToken_free(self->next);
  */
  free(self);
}

void IoToken_name_length_(IoToken *self, char *name, size_t len)
{ 
  self->name = strncpy(realloc(self->name, len+1), name, len);
  self->name[len] = (char)0x0;
  self->length = len;
}

void IoToken_name_(IoToken *self, char *name)
{ 
  self->name = strcpy(realloc(self->name, strlen(name)+1), name);
  self->length = strlen(name);
}

char *IoToken_name(IoToken *self) 
{ return self->name ? self->name : (char *)""; }

int IoToken_nameIs_(IoToken *self, char *name)
{ return !strncmp(self->name, name, self->length); }

IoTokenType IoToken_type(IoToken *self) { return self->type; }

int IoToken_lineNumber(IoToken *self) { return self->lineNumber; }
int IoToken_charNumber(IoToken *self) { return self->charNumber; }

void IoToken_quoteName_(IoToken *self, char *name)
{ 
  char *old = self->name;
  self->name = malloc(strlen(name)+3); 
  sprintf(self->name, "\"%s\"", name);
  if (old) free(old);
}

void IoToken_type_(IoToken *self, IoTokenType type)
{ self->type = type; }

void IoToken_nextToken_(IoToken *self, IoToken *nextToken)
{
  if (self == nextToken) 
  { printf("next == self!\n"); exit(1); }
  if (self->nextToken) IoToken_free(self->nextToken);
  self->nextToken = nextToken;
}

void IoToken_print(IoToken *self)
{
  IoToken_printSelf(self);
  /*
  if (self->nextToken) 
  {
    printf(", ");
    IoToken_print(self->nextToken);
  }
  */
}

void IoToken_printSelf(IoToken *self)
{
  size_t i;
  printf("'");
  for (i=0; i<self->length; i++) putchar(self->name[i]);
  printf("' ");
}





