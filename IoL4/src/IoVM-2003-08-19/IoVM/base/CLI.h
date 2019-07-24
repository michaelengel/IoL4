/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 *
 * Basic command line interace with input callback
 */
 
#include "Common.h"

typedef void (CLICallback)(void *target, char *);
typedef void (CLIInputFunction)(void *handler);

typedef struct
{
  char *buffer;
  int bufferSize;
  int eof;
  void *target;
  char *prompt;
  CLICallback *callback; /* callback to owner upon input */
  void *inputHandler;
  CLIInputFunction *inputFunction;
} CLI;

CLI *CLI_new(void);
void CLI_free(CLI *self);

void CLI_prompt_(CLI *self, char *s);
void CLI_target_callback_(CLI *self, void *target, CLICallback *callback);

void CLI_input(CLI *self);
int CLI_eof(CLI *self);
void CLI_run(CLI *self);

