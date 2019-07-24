/* Scheduler
 * Based entirely on E. Toernig's Minimalistic cooperative multitasking
 * Symbian support and help with conversion to single static variable by Chis Double
 * - Steve Dekorte, 2002
 */

#include "Scheduler.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stddef.h>

/* since currentScheduler is static, this code isn't safe for 
 * use in a scheduler-per-coro model unless you
 * add a bit of code for your platform to set it properly or
 * link it to some coro dependent variable.
 */

#if !defined(__SYMBIAN32__)
static Scheduler *currentScheduler;
#endif

/* This is the system dependent function.
 * Setup a jmp_buf so when it is longjmp'ed it will invoke 'func' using 'stack'.
 * ('func' will not (and mustn't) return!)
 *
 * Usually done by setting the program counter and stack pointer of a new, empty stack.
 * If you're adding a new platform, look in the setjmp.h for PC and SP members 
 * of the stack structure
 */

static void setup_jmpbuf(jmp_buf buf, void *func, void *stack, size_t stacksize)
{
  setjmp(buf);
  #if defined(_BSD_PPC_SETJMP_H_)
    /* ppc-osx */
    #define setjmp _setjmp /* no need for signal masks */
    #define longjmp _longjmp
    memset(stack+stacksize-64, 0, 64);	/* very crude... */
    buf[0] = ((int)stack + stacksize - 64 + 15) & ~15;
    buf[21] = (int)func;
  #elif defined(JB_SP)
    /* x86-linux with glibc2 */
    buf->__jmpbuf[JB_SP] = (int)stack + stacksize;
    buf->__jmpbuf[JB_PC] = (int)func;
  /*
  #elif IRIX
    buf[JB_SP] = (__uint64_t)((char *)stack + stacksize - 8);
    buf[JB_PC] = (__uint64_t)func;
  */
  #elif defined(_I386_JMP_BUF_H)
    /* x86-linux with libc5 */
    buf->__sp = stackend;
    buf->__pc = func;
  #elif defined(__JMP_BUF_SP)
    /* arm-linux on the sharp zauras */
    buf->__jmpbuf[__JMP_BUF_SP] = (int)stackend;
    buf->__jmpbuf[__JMP_BUF_SP+1] = (int)func;
  #elif defined(__CYGWIN__) 
    /* cygwin */
    buf[7] = (int)(stack + stacksize);
    buf[8] = (int)func;
  #elif defined(__MINGW32__) || defined(_MSC_VER)
    /* mingw32 or win32 visual c*/
    buf[4] = (int)((unsigned char *)stack + stacksize);  /* esp */
    buf[5] = (int)func; /* eip */
  #elif defined(__SYMBIAN32__)
    buf[0] = 0;
    buf[1] = 0;
    buf[2] = 0;
    buf[3] = (unsigned int)stack + SCHEDULER_STACKSIZE - 64;
    buf[9] = (int)func;
    buf[8] =  buf[3] + 32;
  #elif defined(_JBLEN) && (_JBLEN == 81)
    /* FreeBSD/Alpha */
    buf->_jb[2] = (long)func;     /* sc_pc */
    buf->_jb[26+4] = (long)func;  /* sc_regs[R_RA] */
    buf->_jb[27+4] = (long)func;  /* sc_regs[R_T12] */
    buf->_jb[30+4] = (long)(stack + stacksize); /* sc_regs[R_SP] */
  #elif defined(_JB_ATTRIBUTES)
    /*NetBSD i386? */
    buf[2] = (long)(stack + stacksize);
    buf[0] = (long)func;  
  #elif defined(_JBLEN)
    /* i386-BSD and NetBSD on IA32 */
    buf->_jb[2] = (long)(stack + stacksize);
    buf->_jb[0] = (long)func;
  #else
    printf("Io Scheduler error: no setup_jmpbuf entry for this platform\n.");
    exit(1);
  #endif
}

Scheduler *Scheduler_new(void *state)
{
  Scheduler *self = (Scheduler *)calloc(1, sizeof(Scheduler));
  self->state = state;
  self->pausedCoros = List_new();
  self->freeCoros = (Coro **)calloc(SCHEDULER_MAXFREE, sizeof(Coro *));
  self->mainCoro.next = &(self->mainCoro);
  self->mainCoro.prev = &(self->mainCoro);
  self->currentCoro = &(self->mainCoro);
  self->nr_free = 0;
  self->coroCount = 1;
  self->context = 0;
  self->start_func = 0;
  self->nanoSleepPeriod = 1000000000/100; /* .01 seconds */

  #if !defined(__SYMBIAN32__)
    currentScheduler = self;
  #else
    Scheduler_current_(self);
  #endif
  return self;
}

void Scheduler_free(Scheduler *self)
{
  List_free(self->pausedCoros);
  /* need to free coros */
  free(self->freeCoros);
  free(self);
}

void Scheduler_nanoSleepPeriod_(Scheduler *self, long nanoseconds)
{ self->nanoSleepPeriod = nanoseconds; }

long Scheduler_nanoSleepPeriod(Scheduler *self)
{ return self->nanoSleepPeriod; }

#if !defined(__SYMBIAN32__)
  Scheduler *Scheduler_current(void) { return currentScheduler; }
#endif

static void start_coro(void)
{
  Scheduler *self = Scheduler_current();
  self->start_func(self->context);
  Scheduler_killCoro(self, self->currentCoro);
}

Coro *Scheduler_createCoro(Scheduler *self, void *aContext, void (*func)(void *))
{
  Coro *th;

  if (self->nr_free)
    th = self->freeCoros[--self->nr_free];
   else
    th = malloc(sizeof(*th) + SCHEDULER_STACKSIZE);

  /* setjmp/longjmp is flakey under Symbian. If the setjmp is done
       inside the setup_jmpbuf call then a crash occurs. inlining it
       here solves the problem 
  */
  #if !defined(__SYMBIAN32__)
    setup_jmpbuf(th->env, start_coro, th + 1, SCHEDULER_STACKSIZE);
  #else
    setjmp(th->env);
	th->env[0] = 0;
	th->env[1] = 0;
	th->env[2] = 0;
	th->env[3] = (unsigned int)(th + 1) + SCHEDULER_STACKSIZE - 64;
	th->env[9] = (int)start_coro;
	th->env[8] =  th->env[3] + 32;
  #endif

  /* insert after currentCoro */
  /*
  th->next = self->currentCoro->next;
  th->prev = self->currentCoro;
  th->next->prev = th;
  th->prev->next = th;
  */
  
  /* before after currentCoro */
  th->next = self->currentCoro;
  th->prev = self->currentCoro->prev;
  th->next->prev = th;
  th->prev->next = th;
  
  self->coroCount++;

  self->start_func = func;
  self->context = aContext;
  
  /*Scheduler_yield(self);*/
  if (setjmp(self->currentCoro->env) == 0)
  {
    self->currentCoro = self->currentCoro->prev;
    longjmp(self->currentCoro->env, 1);
  }
  return th;
}

void Scheduler_killCoro(Scheduler *self, Coro *coro)
{
  if (self->nr_free == SCHEDULER_MAXFREE) free(self->freeCoros[--(self->nr_free)]);

  coro->prev->next = coro->next;
  coro->next->prev = coro->prev;
  self->coroCount--;

  self->freeCoros[self->nr_free++] = coro;
  Scheduler_yield(self);
}

#if defined(__SYMBIAN32)
void ProcessUIEvent();
#endif

void Scheduler_yield(Scheduler *self)
{
  #ifdef _TIMESPEC_DECLARED
  if (self->nanoSleepPeriod)
  {
    struct timespec rqtp;  
    struct timespec extra;
    int result;
    rqtp.tv_sec = 0;
    rqtp.tv_nsec = self->nanoSleepPeriod; 
    result = nanosleep(&rqtp, &extra);
    /*
    while (nanosleep(&rqtp, &extra) == -1)
    {
      rqtp.tv_sec = extra.tv_sec;
      rqtp.tv_nsec = extra.tv_nsec;     
    }
    */
  }
  #endif
  
  #if defined(__SYMBIAN32)
    ProcessUIEvent();
  #endif
  if (setjmp(self->currentCoro->env) == 0)
  {
    self->currentCoro = self->currentCoro->next;
    longjmp(self->currentCoro->env, 1);
  }
}

int Scheduler_coroCount(Scheduler *self) { return self->coroCount; }

void Scheduler_pauseCoro_(Scheduler *self, Coro *coro)
{
  coro->prev->next = coro->next;
  coro->next->prev = coro->prev;
  self->coroCount--;

  List_add_(self->pausedCoros, coro);
  /*if (coro == self->currentCoro) Scheduler_yield(self);*/
}

char Scheduler_hasCoro_(Scheduler *self, Coro *coro)
{
  Coro *aCoro = self->currentCoro;
  do
  { 
    if (aCoro == coro) return 1; 
    aCoro = aCoro->next;
  } 
  while (aCoro != self->currentCoro);
  return 0;
}

void Scheduler_resumeCoro_(Scheduler *self, Coro *coro)
{
  if (coro == self->currentCoro || 
      Scheduler_hasCoro_(self, coro) || 
      !List_contains_(self->pausedCoros, coro)) return;
  coro->next = self->currentCoro->next;
  coro->prev = self->currentCoro;
  coro->next->prev = coro;
  coro->prev->next = coro;
  self->coroCount++;

  List_remove_(self->pausedCoros, coro);
}

double Scheduler_waitSeconds_(Scheduler *self, double seconds)
{
  /* This implementation is simple but inefficient.
     Pausing the waiting coros and having a separate timer stack coro
     would be much faster when multiple coros are waiting. 
  */
  clock_t start = clock();
  seconds *= CLOCKS_PER_SEC;
  while (seconds > (double)(clock() - start))
  { Scheduler_yield(self); }
  /* return the number of seconds actually waited */
  return (double)((clock() - start)/CLOCKS_PER_SEC);
}
