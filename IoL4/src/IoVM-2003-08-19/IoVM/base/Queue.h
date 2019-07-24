/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 *
 * Queue - a queue of void pointers
 */

#ifndef QUEUE_DEFINED
#define QUEUE_DEFINED 1

#include "Common.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef LOW_MEMORY_SYSTEM
#define QUEUE_START_SIZE 8
#define QUEUE_RESIZE_FACTOR 1.5
#else
#define QUEUE_START_SIZE 8
#define QUEUE_RESIZE_FACTOR 2
#endif

typedef struct
{
  void **items;
  int size; /* # of pointers that can fit in allocated memory for items */
  int top;
  int bottom;
} Queue;

typedef unsigned char (QueueFirstTrueForWithCallback)(void *, void *);
typedef unsigned char (QueueDoCallback)(void *);

Queue *Queue_new(void);
Queue *Queue_clone(Queue *self);
void Queue_free(Queue *self);
int Queue_length(Queue *self);
void *Queue_at_(Queue *self, int i);

void Queue_compact(Queue *self);
size_t Queue_memorySize(Queue *self);
void Queue_print(Queue *self);

void Queue_do_(Queue *self, QueueDoCallback *callback);
  
void *Queue_firstTrueFor_with_(Queue *self, 
  QueueFirstTrueForWithCallback *callback, void *arg);
  
#include "Queue_inline.h"

#endif
