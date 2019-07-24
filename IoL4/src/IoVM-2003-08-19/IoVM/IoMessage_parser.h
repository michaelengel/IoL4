/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */

#ifndef IOMESSAGE_PARSER_DEFINED
#define IOMESSAGE_PARSER_DEFINED 1

#include "base/Common.h"
#include "IoMessage.h"

IoMessage *IoMessage_newFromText_label_(void *state, char *text, char *label);
void IoMessage_cacheIfPossible(IoMessage *self);

#endif
