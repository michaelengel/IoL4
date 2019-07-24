/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     glue/v4-ia64/resources.cc
 * Description:   Thread resource management
 *                
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *                
 * $Id: resources.cc,v 1.5 2003/03/06 05:09:08 skoglund Exp $
 *                
 ********************************************************************/
#include INC_API(tcb.h)
#include INC_ARCH(rr.h)
#include INC_GLUE(resources.h)

#include <kdb/tracepoints.h>


DECLARE_TRACEPOINT (RESOURCES);

void thread_resources_t::dump (tcb_t * tcb)
{
    if (tcb->resource_bits & IA64_RESOURCE_FPU)
	printf ("<fpu> ");
    if (tcb->resource_bits & IA64_RESOURCE_COPYAREA)
	printf ("<copy-area, rid=%07x> ", partner_rid);
    if (tcb->resource_bits & IA64_RESOURCE_BREAKPOINT)
	printf ("<breakpoint> ");
    if (tcb->resource_bits & IA64_RESOURCE_PERFMON)
	printf ("<perfmon> ");

    if (tcb->resource_bits != 0)
	printf ("\b");
}

void thread_resources_t::save (tcb_t * tcb, tcb_t * dest)
{
    TRACEPOINT (RESOURCES,
		printf ("Resources save: tcb=%p  rsc=%p [",
			tcb, tcb->resource_bits);
		dump (tcb);
		printf ("]\n"));

    if (tcb->resource_bits & IA64_RESOURCE_COPYAREA)
	disable_copy_area (tcb, false);

    if (tcb->resource_bits & IA64_RESOURCE_BREAKPOINT)
    {
	disable_global_breakpoint (tcb);
	enable_global_breakpoint (dest);
    }

    if (tcb->resource_bits & IA64_RESOURCE_PERFMON)
    {
	disable_global_perfmon (tcb);
	enable_global_perfmon (dest);
    }
}

void thread_resources_t::load (tcb_t * tcb)
{
    TRACEPOINT (RESOURCES,
		printf ("Resources load: tcb=%p  rsc=%p [",
			tcb, tcb->resource_bits);
		dump (tcb);
		printf ("]\n"));

    if (tcb->resource_bits & IA64_RESOURCE_COPYAREA)
    {
	// Associate the copy area region with the partner's space.
	rr_t rr (false, partner_rid, 12);
	rr.put (4);
	ia64_srlz_d ();
    }
}

void thread_resources_t::purge (tcb_t * tcb)
{
    TRACEPOINT (RESOURCES,
		printf ("Resources purge: tcb=%p  rsc=%p [",
			tcb, tcb->resource_bits);
		dump (tcb);
		printf ("]\n"));

    if (tcb->resource_bits & IA64_RESOURCE_COPYAREA)
	disable_copy_area (tcb, false);
}

void thread_resources_t::free (tcb_t * tcb)
{
    TRACEPOINT (RESOURCES,
		printf ("Resources free: tcb=%p  rsc=%p [",
			tcb, tcb->resource_bits);
		dump (tcb);
		printf ("]\n"));

    if (tcb->resource_bits & IA64_RESOURCE_COPYAREA)
	disable_copy_area (tcb, false);
}

void thread_resources_t::init (tcb_t * tcb)
{
    tcb->resource_bits = 0;
    partner_rid = 0;
}

