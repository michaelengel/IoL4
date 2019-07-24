/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     kdb/api/v4/input.cc
 * Description:   Version 4 specific input functions
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
 * $Id: input.cc,v 1.5 2003/03/05 09:10:47 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/input.h>
#include <kdb/kdb.h>
#include INC_API(space.h)
#include INC_API(tcb.h)


/**
 * Prompt for an address space using PROMPT and return the space
 * pointer.  Input value can be a TCB address or a physical or virtual
 * space pointer.

 * @param prompt	prompt
 *
 * @return pointer to space
 */
space_t SECTION(SEC_KDEBUG) * get_space (const char * prompt)
{
    space_t * dummy = NULL;
    space_t * space;
    addr_t val;

    val = (addr_t) get_hex (prompt, (word_t) kdb.kdb_current->get_space (),
			    "current");

    if (dummy->is_tcb_area (val))
    {
	// Pointer into the TCB area
	tcb_t * tcb = addr_to_tcb (val);
	space = tcb->get_space ();
    }
    else if (dummy->is_user_area (val))
    {
	// Pointer in lower memory area.  Probably a physical address.
	val = phys_to_virt (val);
	space = (space_t *) val;
    }
    else
    {
	// Hopefuly a valid space pointer
	space = (space_t *) val;
    }

    return space;
}


/**
 * Prompt for a thread using PROMPT and return the tcb pointer.  Input
 * value can be a TCB address od a thread id.
 *
 * @param prompt	prompt
 *
 * @return pointer to tcb
 */
tcb_t * SECTION (SEC_KDEBUG) get_thread (const char * prompt)
{
    space_t * dummy = kdb.kdb_current->get_space ();
    addr_t val;

    val = (addr_t) get_hex (prompt, (word_t) kdb.kdb_current, "current");

    if (dummy->is_tcb_area (val))
	return addr_to_tcb (val);
    else
    {
	threadid_t tid;
	tid.set_raw ((word_t) val);
	return dummy->get_tcb (tid);
    }
}
