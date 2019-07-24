/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 *
 *   Structure for copying and storing command line arguments
 */
 
#include "Common.h"

typedef struct
{
  int argc;
  char **argv;
} MainArgs;

MainArgs *MainArgs_new(void);
void MainArgs_free(MainArgs *self);

void MainArgs_argc_argv_(MainArgs *self, int argc, char **argv);




