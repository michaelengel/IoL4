/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  Karlsruhe University
 *                
 * File path:     glue/v4-ia32/exception.cc
 * Description:   exception handling
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
 * $Id: exception.cc,v 1.20 2003/04/25 16:52:55 uhlig Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include <kdb/tracepoints.h>
#include INC_ARCH(traps.h)
#include INC_ARCH(trapgate.h)
#include INC_API(tcb.h)
#include INC_API(space.h)
#include INC_API(kernelinterface.h)

DECLARE_TRACEPOINT (IA32_GP);
DECLARE_TRACEPOINT (IA32_UD);
DECLARE_TRACEPOINT (IA32_NOMATH);

static bool send_exception_ipc(ia32_exceptionframe_t * frame, word_t exception)
{
    tcb_t * current = get_current_tcb();
    if (current->get_exception_handler().is_nilthread())
	return false;

    /* setup exception IPC */
    word_t saved_mr[13];
    msg_tag_t tag;

    // save message registers 
    for (int i = 0; i < 13; i++)
	saved_mr[i] = current->get_mr(i);
    word_t saved_br0 = current->get_br(0);
    current->set_saved_partner (current->get_partner());

    tag.set(0, 12, -5 << 4);
    current->set_mr(0, tag.raw);
    current->set_mr(1, frame->eip);
    current->set_mr(2, frame->eflags);
    current->set_mr(3, exception);
    current->set_mr(4, frame->error);
    current->set_mr(5, frame->edi);
    current->set_mr(6, frame->esi);
    current->set_mr(7, frame->ebp);
    current->set_mr(8, frame->esp);
    current->set_mr(9, frame->ebx);
    current->set_mr(10, frame->edx);
    current->set_mr(11, frame->ecx);
    current->set_mr(12, frame->eax);

    tag = current->do_ipc(current->get_exception_handler(), 
	current->get_exception_handler(), 
	timeout_t::never());

    if (!tag.is_error())
    {
	frame->eip = current->get_mr(1);
	current->set_user_flags(current->get_mr(2));
	frame->edi = current->get_mr(5);
	frame->esi = current->get_mr(6);
	frame->ebp = current->get_mr(7);
	frame->esp = current->get_mr(8);
	frame->ebx = current->get_mr(9);
	frame->edx = current->get_mr(10);
	frame->ecx = current->get_mr(11);
	frame->eax = current->get_mr(12);
    }
    else
    {
	enter_kdebug("exception delivery error");
    }

    for (int i = 0; i < 13; i++)
	current->set_mr(i, saved_mr[i]);
    current->set_br(0, saved_br0);
    current->set_partner(current->get_saved_partner ());
    current->set_saved_partner (NILTHREAD);

    return !tag.is_error();
}

IA32_EXC_NO_ERRORCODE(exc_catch, -1)
{
    printf("exception caught\n");
    while(1);
}

IA32_EXC_NO_ERRORCODE(exc_invalid_opcode, IA32_EXC_INVALIDOPCODE)
{
    tcb_t * current = get_current_tcb();
    space_t * space = current->get_space();
    addr_t addr = (addr_t)frame->eip;

    TRACEPOINT_TB (IA32_UD, ("ia32_ud at %x (current=%p)", (u32_t)addr, (u32_t)current),
		   printf ("%p: invalid opcode at IP %p\n", current, addr));

    /* instruction emulation, only in user area! */
    if (space->is_user_area(addr))
    {
	switch(space->get_from_user(addr))
	{
	case 0xf0: /* lock prefix */
	    if (space->get_from_user(addr_offset(addr, 1)) == 0x90)
	    {
		/* lock; nop */
		frame->eax = (u32_t)space->get_kip_page_area().get_base();
		frame->ecx = get_kip()->api_version;
		frame->edx = get_kip()->api_flags;
		frame->esi = get_kip()->get_kernel_descriptor()->kernel_id.get_raw();
		frame->eip+= 2;
		return;
	    }
	default:
	    printf("invalid opcode\n");
	    enter_kdebug("invalid opcode");
	}
    }

    if (send_exception_ipc(frame, IA32_EXC_INVALIDOPCODE))
	return;
    
    get_current_tcb()->set_state(thread_state_t::halted);
    get_current_tcb()->switch_to_idle();
}

IA32_EXC_WITH_ERRORCODE(exc_gp, IA32_EXC_GENERAL_PROTECTION)
{
    TRACEPOINT_TB (IA32_GP, ("ia32_gp at %x (error=%d)",
			     frame->eip, frame->error),
		   printf ("general protection fault @ %p, error: %x\n", 
			   frame->eip, frame->error));

    if (send_exception_ipc(frame, IA32_EXC_GENERAL_PROTECTION))
        return;

    enter_kdebug("#GP");

    get_current_tcb()->set_state(thread_state_t::halted);
    get_current_tcb()->switch_to_idle();
}

IA32_EXC_NO_ERRORCODE(exc_nomath_coproc, IA32_EXC_NOMATH_COPROC)
{
    tcb_t * current = get_current_tcb();

    TRACEPOINT(IA32_NOMATH, 
        printf("IA32_NOMATH %p @ %p\n", current, frame->eip));

    current->resources.ia32_no_math_exception(current);
}

IA32_EXC_NO_ERRORCODE(exc_fpu_fault, IA32_EXC_FPU_FAULT)
{
    printf("fpu fault exception @ %p", frame->eip);

    if (send_exception_ipc(frame, IA32_EXC_FPU_FAULT))
	return;
    
    get_current_tcb()->set_state(thread_state_t::halted);
    get_current_tcb()->switch_to_idle();
}

IA32_EXC_NO_ERRORCODE(exc_simd_fault, IA32_EXC_SIMD_FAULT)
{
    printf("simd fault exception @ %p", frame->eip);

    if (send_exception_ipc(frame, IA32_EXC_SIMD_FAULT))
	return;
    
    get_current_tcb()->set_state(thread_state_t::halted);
    get_current_tcb()->switch_to_idle();
}

