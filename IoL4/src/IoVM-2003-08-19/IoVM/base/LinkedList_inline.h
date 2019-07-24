/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 *
 * LinkedList - A  double linked list
 * each item holds a single value
 * a count is maintained so asking for a count is fast
 *
 */

#ifdef LINKEDLIST_C 
#define IO_IN_C_FILE
#endif
#include "Common_inline.h"
#ifdef IO_DECLARE_INLINES

IOINLINE int LinkedList_count(LinkedList *self) { return self->count; }

#undef IO_IN_C_FILE
#endif


