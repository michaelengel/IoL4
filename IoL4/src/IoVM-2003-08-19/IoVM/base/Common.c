/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 */
 
#include "Common.h"

void *cpalloc(void *p, size_t size)
{
  void *n = malloc(size);
  memcpy(n, p, size);
  return n;
}

