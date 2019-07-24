/* 
 * Scheduler - implements and manages coroutines
 * Based entirely on E. Toernig's Minimalistic cooperative multitasking
 * with conversion to single static variable help by Chis Double
 * - Steve Dekorte, 2002
 */

#ifndef SCHEDULER_DEFINED
#define SCHEDULER_DEFINED 1

#include "Common.h"
#include "List.h"
#include <setjmp.h>

#if defined(__SYMBIAN32__)
  #define SCHEDULER_STACKSIZE 8192
#else
  #define SCHEDULER_STACKSIZE 8192
  /*32768*/
#endif
#define SCHEDULER_MAXFREE 128

typedef struct Coro Coro;

struct Coro
{
  Coro *next;
  Coro *prev;
  jmp_buf env;
  void *context;
};

typedef void (SchedulerStartCallback)(void *);

typedef struct
{
  void *state; /* just here in case other implementations need it */
  Coro mainCoro;
  Coro *currentCoro;
  int coroCount; /* main is already there */
  Coro **freeCoros;
  List *pausedCoros;
  int nr_free;
  void *context;
  void (*start_func)(void *);
  long nanoSleepPeriod;
} Scheduler;

Scheduler *Scheduler_new(void *state);
void Scheduler_free(Scheduler *self);

void Scheduler_nanoSleepPeriod_(Scheduler *self, long nanoseconds);
long Scheduler_nanoSleepPeriod(Scheduler *self);

Coro *Scheduler_createCoro(Scheduler *self, void *aContext, void (*func)(void *));
void Scheduler_killCoro(Scheduler *self, Coro *coro);
void Scheduler_yield(Scheduler *self);
int Scheduler_coroCount(Scheduler *self);

void Scheduler_pauseCoro_(Scheduler *self, Coro *coro);
char Scheduler_hasCoro_(Scheduler *self, Coro *coro);
void Scheduler_resumeCoro_(Scheduler *self, Coro *coro);
double Scheduler_waitSeconds_(Scheduler *self, double seconds);

#endif
