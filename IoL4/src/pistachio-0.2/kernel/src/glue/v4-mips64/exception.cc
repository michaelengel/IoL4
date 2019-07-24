/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  University of New South Wales
 *                
 * File path:     glue/v4-mips64/intctrl.cc
 * Description:   Implementation of interrupt control functionality 
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
 * $Id: exception.cc,v 1.11 2003/05/01 14:56:45 sjw Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include <kdb/tracepoints.h>
#include INC_GLUE(context.h)
#include INC_API(tcb.h)
#include INC_API(schedule.h)
#include INC_API(syscalls.h)
#include INC_GLUE(syscalls.h)
#include INC_API(kernelinterface.h)

extern void putc(char);
extern char getc(void);

static bool send_exception_ipc(mips64_irq_context_t* frame)
{
    tcb_t * current = get_current_tcb();
    if (current->get_exception_handler().is_nilthread())
	return false;

    /* setup exception IPC */
    word_t saved_mr[15];
    msg_tag_t tag;

    // save message registers 
    for (int i = 0; i < 15; i++)
	saved_mr[i] = current->get_mr(i);
    word_t saved_br0 = current->get_br(0);
    current->saved_partner = current->get_partner();

    tag.set(0, 12, -5 << 4);
    current->set_mr(0, tag.raw);
    current->set_mr(1, frame->epc);
    current->set_mr(2, frame->sp);
    current->set_mr(3, (frame->cause>>2)&0x1F);
    current->set_mr(4, frame->a0);
    current->set_mr(5, frame->a1);
    current->set_mr(6, frame->a2);
    current->set_mr(7, frame->a3);
    current->set_mr(8, frame->t0);
    current->set_mr(9, frame->t1);
    current->set_mr(10, frame->t2);
    current->set_mr(11, frame->t3);
    current->set_mr(12, frame->at);
    current->set_mr(13, frame->v0);
    current->set_mr(14, frame->v1);

    tag = current->do_ipc(current->get_exception_handler(), 
	current->get_exception_handler(), 
	timeout_t::never());

    if (!tag.is_error())
    {
	frame->epc = current->get_mr(1);
	frame->sp = current->get_mr(2);
	frame->a0 = current->get_mr(4);
	frame->a1 = current->get_mr(5);
	frame->a2 = current->get_mr(6);
	frame->a3 = current->get_mr(7);
	frame->t0 = current->get_mr(8);
	frame->t1 = current->get_mr(9);
	frame->t2 = current->get_mr(10);
	frame->t3 = current->get_mr(11);
	frame->at = current->get_mr(12);
	frame->v0 = current->get_mr(13);
	frame->v1 = current->get_mr(14);
    }
    else
    {
	enter_kdebug("exception delivery error");
    }

    for (int i = 0; i < 15; i++)
	current->set_mr(i, saved_mr[i]);
    current->set_br(0, saved_br0);
    current->set_partner(current->saved_partner);
    current->saved_partner = NILTHREAD;

    return !tag.is_error();
}

extern "C" void mips_break(mips64_irq_context_t *context)
{

    switch (context->at) {
    /* Debug functions */
    case L4_TRAP_KPUTC: putc((char)context->a0); break;
    case L4_TRAP_KGETC: context->v0 = getc(); break;
//    case L4_TRAP_GETUTCB: context->v0 = get_current_tcb()->get_local_id().get_raw(); break;
    case L4_TRAP_GETUTCB: context->v0 = (word_t)get_current_tcb()->get_utcb_location(); break;
    default:
	printf("Unknown user debug call %d\n", context->at);
	kdebug_entry(context);
	break;
    case L4_TRAP_KDEBUG:
	printf (TXT_BRIGHT "--- KD# %s ---\n" TXT_NORMAL, (char*)context->v0);
	kdebug_entry(context);
	break;
    }
    /* XXX evil should check if in branch delay slot */
    context->epc += 4;
}

extern "C" void mips_exception(mips64_irq_context_t *context)
{
    printf ("Exception Frame at %p\n", context);
    enter_kdebug("exception");
}

extern "C" void mips_illegal(mips64_irq_context_t *context)
{
    if (context->at == MAGIC_KIP_REQUEST)
    {
	//TRACEF("KernelInterface() at %p\n", context->epc);
        space_t * space = get_current_space ();
    
	context->t0 = (u64_t) space->get_kip_page_area ().get_base ();
	context->t1 = get_kip ()->api_version;
	context->t2 = get_kip ()->api_flags;
	context->t3 = (NULL != get_kip()->kernel_desc_ptr) ?
		    *(word_t *)((word_t)get_kip() + get_kip()->kernel_desc_ptr) : 0;
    }
    else
    {
	printf(":Illegal Instruction: EPC=0x%16lx, STATUS=0x%16lx\n", context->epc, context->status);
	enter_kdebug("Illegal Instruction");
    }
    /* XXX evil should check if in branch delay slot */
    context->epc += 4;
}

extern "C" void mips_watch(mips64_irq_context_t *context)
{
    printf("MIPS Watchpoint at 0x%16lx, STATUS=0x%16lx %s\n", context->epc, context->status,
		context->cause & 0x80000000 ? "(branch delay slot)" : "");
    enter_kdebug("watch point");
}

