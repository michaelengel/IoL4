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
#include <l4/thread.h>
#include <l4/message.h>
#include <l4/sigma0.h>
#include <l4/kdebug.h>
#include <unistd.h>
#include <stdio.h>

/*
 * Note:
 * This is overly simplistic, as it assumes that each process knows in
 * advance how much memory it wants.  Once we get a VM subsystem
 * this will need to and should be changed.
 *
 * Bugs:
 * Will fail to init if it can't allocate the requested amount of
 * memory contiguously.  Not thread safe.  Does not actually trim
 * the core on a negative sbrk.
 */

static void*  core_base;
static size_t core_size;
static size_t core_max_size;

int __init_core(size_t size, L4_ThreadId_t threadId)
{
  int expToAlloc = 0;
  int i;
  int *j;
  L4_Fpage_t addrSpace;

  /* Initialize "core" */
  core_base = NULL;
  core_size = 0;
  core_max_size = 0;

  /* Round up to nearest power of 2. */
  for(expToAlloc = 0, i = 0; i < 32; expToAlloc |= size >> ++i);
  expToAlloc = expToAlloc << 1;
  printf("Will allocate %x bytes of memory.\n", 1 << expToAlloc);

  /* Fuck it. */
  expToAlloc = 24; /* 16MB */

  /* Request RAM from sigma0, rounded up to closest power of 2. */
  addrSpace = L4_Sigma0_GetAny(threadId,
                               (L4_Word_t) expToAlloc,
                               L4_CompleteAddressSpace);
  if (!L4_IsNilFpage(addrSpace))
  {
    core_max_size = 1 << expToAlloc;
    core_base = (void *) addrSpace.raw; 
  }
  printf("core_base: %p core_max_size %x\n", core_base, core_max_size);

  for(i = 0; i < core_max_size/4096 ; i++)
  {
    j = core_base + i * 4096;
    *j = 0;
  }

  return 0;
}

int brk(const void* addr) {

  if (core_base + core_max_size > addr)
  {
    core_size = (size_t) addr - (size_t) core_base;
    return 0;
  }

  return -1;
}

void* sbrk(size_t incr)
{
  if (core_size + incr < core_max_size)
  {
    core_size += incr;
    return core_base + core_size;
  }

  return (void*) -1;
}
