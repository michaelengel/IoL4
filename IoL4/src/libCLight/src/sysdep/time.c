/*
 * Copyright 2003, Kentaro A. Kurahone
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of Kentaro A. Kurahone nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY KENTARO A. KURAHONE AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL KENTARO A. KURAHONE OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <l4/schedule.h>
#include <time.h>

/* Global externs. */
extern clock_t __clockBase; /* Base clock.  Initialised in notmain() */

clock_t clock(void)
{
  L4_Clock_t sysClock;
  clock_t rval;

  sysClock.raw = L4_SystemClock(); /* Get time from L4. */
  rval = sysClock.raw - __clockBase;

  return rval;
}

int nanosleep(const struct timespec* rqpt, struct timespec* rmtp)
{
  L4_Clock_t sysClock;
  L4_Clock_t doneTime;

  sysClock.raw = L4_SystemClock();
  doneTime.raw = rqpt->tv_sec * CLOCKS_PER_SEC + rqpt->tv_nsec * 1000;

  do {
    L4_Yield(); /* Yeild our time slice. */
    sysClock.raw = L4_SystemClock();
  } while(sysClock.raw < doneTime.raw);

  return 0;
}
