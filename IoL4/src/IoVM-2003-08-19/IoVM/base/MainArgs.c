/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MainArgs.h"

MainArgs *MainArgs_new(void)
{
  MainArgs *self = (MainArgs *)calloc(1, sizeof(MainArgs));
  return self;
}

void MainArgs_free(MainArgs *self)
{
  if (self->argv)
  {
    int i;
    for (i=0; i<self->argc; i++) free(self->argv[i]);
    free(self->argv);
  }
  free(self);
}

void MainArgs_argc_argv_(MainArgs *self, int argc, char **argv)
{
  int i;
  self->argc = argc;
  self->argv = malloc(sizeof(char *)*argc);
  for (i=0; i<argc; i++)
  {
    char *s = argv[i];
    self->argv[i] = strcpy((char *)malloc(strlen(s)+1), s);
    /*printf("argv[%i] = '%s'\n", i, s);*/
  }
}

