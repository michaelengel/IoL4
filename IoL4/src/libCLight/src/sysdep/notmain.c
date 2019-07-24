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

#include <config.h>
#include <l4/kip.h>
#include <l4/thread.h>
#include <l4/schedule.h>
#include <l4/kdebug.h>
#include <time.h>
#include <stdio.h>

extern int main(int argc, char *argv[]);
extern int __init_core(size_t size, L4_ThreadId_t threadId);

/*
 * Code neccecary to init the system and setup the libCLight enviorment
 * live here.
 */

#define MAX_CORE_SIZE (1024*1024*10)

clock_t __clockBase;

static void __do_main(void);

int __notmain(int argc, char* argv[])
{
  L4_KernelInterfacePage_t* kip;
  L4_ThreadId_t threadId;
  L4_ThreadId_t sigma0Id;
  L4_ThreadId_t mainThreadId;
  L4_Clock_t sysClock;
  L4_Word_t utcbArea;

  /* Grab kernel interface page. */
  kip = (L4_KernelInterfacePage_t*) L4_GetKernelInterface();

  /* Determine our global thread id (PID) */
  threadId = L4_MyGlobalId();

  /* Determing main thread's thread id. */
  mainThreadId = L4_GlobalId(L4_ThreadNo(threadId) + 1, 1);

  /* Determine sigma0's thread id. */
  sigma0Id = L4_GlobalId(L4_ThreadIdUserBase(kip), 1);

  /* Setup utcb area.  For main thread. */
  utcbArea = L4_MyLocalId().raw;
  utcbArea = (utcbArea & ~(L4_UtcbSize(kip) - 1)) + L4_UtcbSize(kip);

  /* Init our core. */
  printf("Attempting to init core.\n");
  __init_core(MAX_CORE_SIZE, sigma0Id);

  /* Init our clock. */
  sysClock.raw = L4_SystemClock();
  __clockBase = sysClock.raw;

  /* Call IoVM's entry point. */
  __do_main();

  return 0;
}

static void __do_main(void)
{
  char* argv[] = { "IoOS/L4\0" };
  printf("Calling IoVM main() cross your fingers.\n");

  L4_KDB_Enter("PreIoVM breakpoint.");
  main(0, argv);
}
