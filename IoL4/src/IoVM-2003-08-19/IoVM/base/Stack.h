/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 *
 * Stack - array of void pointers
 * supports setting marks - when a mark is popped,
 * all stack items above it are popped as well 
 */

#ifndef STACK_DEFINED
#define STACK_DEFINED 1

#include "Common.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#ifdef LOW_MEMORY_SYSTEM
  #define STACK_START_SIZE 2
  #define STACK_RESIZE_FACTOR 2
#else
  #define STACK_START_SIZE 1024
  #define STACK_RESIZE_FACTOR 2
#endif

typedef void (StackDoCallback)(void *);
typedef void (StackDoOnCallback)(void *, void *);

typedef struct
{
  void **items;
  void **memEnd;
  void **top;
  ptrdiff_t lastMark;
  /* only used for walking */
  /*
  int current; 
  int currentMark; 
  */
} Stack;

Stack *Stack_new(void);
void Stack_free(Stack *self);
void Stack_clear(Stack *self);

size_t Stack_memorySize(Stack *self);
void Stack_compact(Stack *self);

void Stack_resize(Stack *self);
size_t Stack_totalSize(Stack *self);

/* not high performance */
void Stack_do_on_(Stack *self, StackDoOnCallback *callback, void *target);
Stack *Stack_newCopyWithoutMarks(Stack *self);

#include "Stack_inline.h"

#endif
