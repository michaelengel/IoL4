/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 *
 * Basic command line interace with input callback
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CLI.h"

CLI *CLI_new(void)
{
  CLI *self = (CLI *)calloc(1, sizeof(CLI));
  self->bufferSize = 100;
  self->buffer = (char *)malloc(self->bufferSize);
  memset(self->buffer, 0x0, self->bufferSize);
  CLI_prompt_(self, ">");
  return self;
}

void CLI_free(CLI *self)
{
  free(self->buffer);
  free(self);
}

void CLI_prompt_(CLI *self, char *s)
{
  self->prompt = realloc(self->prompt, strlen(s)+1);
  strcpy(self->prompt, s);
}

void CLI_target_callback_(CLI *self, void *target, CLICallback *callback)
{
  self->target = target;
  self->callback = callback;
}

void CLI_input(CLI *self)
{
  int i = 0;
  for (;;) 
  {    
    int c = fgetc(stdin);
    if (c == EOF) { self->eof = 1; break; }
    else if ((c == '\n') || (c== 13)) { break; }
    else self->buffer[i++] = (char)c;
      
    if (i >= self->bufferSize - 2) 
    {
      self->bufferSize *= 2;
      self->buffer = realloc(self->buffer, self->bufferSize);
    }
  }
  self->buffer[i] = '\0';
}

int CLI_eof(CLI *self) { return self->eof; }

void CLI_run(CLI *self) 
{
  while (!self->eof)
  {
    fputs(self->prompt, stdout);
    if (0)
    {
      char *a = "a := 1";
      strcpy(self->buffer, a);
      printf("%s\n", a);
      (self->callback)(self->target, self->buffer);
      printf("exiting CLI test\n");
      exit(0);
    }
    else
    { 
      CLI_input(self); 
      (self->callback)(self->target, self->buffer);
    }
  }
}
