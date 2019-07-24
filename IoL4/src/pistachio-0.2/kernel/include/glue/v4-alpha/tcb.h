/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  University of New South Wales
 *                
 * File path:     glue/v4-alpha/tcb.h
 * Description:   
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
 * $Id: tcb.h,v 1.25 2003/05/01 14:56:33 sjw Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_TEMPLATE__TCB_H__
#define __GLUE__V4_TEMPLATE__TCB_H__

#ifndef __API__V4__TCB_H__
#error not for stand-alone inclusion
#endif

#include INC_ARCH(types.h)
#include INC_API(syscalls.h)		/* for sys_ipc */
#include INC_ARCH(thread.h)             /* for alpha_switch_to */
#include INC_API(space.h)
#include INC_ARCH(page.h)
#include INC_ARCH(palcalls.h)
#include INC_GLUE(ipcregs.h)


/**********************************************************************
 *
 *                        global tcb functions
 *
 **********************************************************************/

INLINE tcb_t * addr_to_tcb (addr_t addr)
{
    return (tcb_t *) ((word_t) addr & KTCB_MASK);
}

/**
 * Locate current TCB by using current stack pointer and return it.
 */
INLINE tcb_t * get_current_tcb (void)
{
    /* sjw (27/07/2002): Is this nicer than usem asm(...)? */
    unsigned long stack_var;

    return (tcb_t *) ((word_t) &stack_var & KTCB_MASK);
};


INLINE alpha_context_t *get_alpha_context(tcb_t *tcb)
{
    alpha_context_t *ctx = (alpha_context_t *) tcb->get_stack_top();
    return --ctx;
}

/**
 * sets the error descriptor of the tcb
 * @param err new value
 */
INLINE void tcb_t::set_ipc_error(word_t err)
{
    get_utcb()->error_code = err;
}

/**
 * get the current ipc error code
 */
INLINE word_t tcb_t::get_ipc_error()
{
    return get_utcb()->error_code;
}

/**
 * read value of message register
 * @param index number of message register
 */
INLINE word_t tcb_t::get_mr(word_t index)
{
    /* sjw (26/07/2002): When I start putting mr's in registers, this will change */
    return get_utcb()->mr[index];
}

/**
 * returns value of message tag
 */
INLINE msg_tag_t tcb_t::get_tag()
{
    msg_tag_t tag;
    tag.raw = get_mr(0);
    return tag;
}

/**
 * sets the message tag
 */
INLINE void tcb_t::set_tag(msg_tag_t tag)
{
    set_mr(0, tag.raw);
}

/**
 * set the value of a message register
 * @param index number of message register
 * @param value value to set
 */
INLINE void tcb_t::set_mr(word_t index, word_t value)
{
    get_utcb()->mr[index] = value;
}

/**
 * copies a set of message registers from one UTCB to another
 * @param dest destination TCB
 * @param start MR start index
 * @param count number of MRs to be copied
 */
INLINE void tcb_t::copy_mrs(tcb_t * dest, word_t start, word_t count)
{
    ASSERT(start + count <= IPC_NUM_MR);

    for (word_t idx = start; idx < start + count; idx++)
	dest->set_mr(idx, this->get_mr(idx));
}

/**
 * read value of buffer register
 * @param index number of buffer register
 */
INLINE word_t tcb_t::get_br(word_t index)
{
    return get_utcb()->br[index];
}

/**
 * set the value of a buffer register
 * @param index number of buffer register
 * @param value value to set
 */
INLINE void tcb_t::set_br(word_t index, word_t value)
{
    get_utcb()->br[index] = value;
}


/**
 * allocate the tcb
 * The tcb pointed to by this will be allocated.
 */
INLINE void tcb_t::allocate()
{
    /* sjw (26/07/2002): Possible performance problem here --- easier to touch, but
     * perhaps more expensive?
     */
    this->kernel_stack[0] = 0;
//    this->myself_global = this->myself_global;
}


/**
 * set the address space a TCB belongs to
 * @param space address space the TCB will be associated with
 */
INLINE void tcb_t::set_space(space_t * space)
{
    this->space = space;
    
    if(space) {
	/* Set up the tcb's PCB ... basically the ptbr etc. */
	/* sjw (26/07/2002): If we start pre-empting the ASNs this will have to be moved into tcb.h as well */
	alpha_pcb_t *pcb = &get_arch()->pcb;
	
	pcb->ptbr = (word_t) virt_to_phys((addr_t) space->pgent(0)) >> ALPHA_PAGE_BITS;    
    }
}

#if 0
/**
 * set local ID of a thread
 * @param utcb ???
 *
 * utcb == local_id on alpha (not sure about lower bits?)
 */
INLINE void tcb_t::set_local_id(utcb_t * utcb)
{
    myself_local.set_raw((word_t) utcb);

    if(this == get_current_tcb()) {
	PAL::wrunique((word_t) utcb);
    } else
	get_arch()->pcb.unique = (word_t) utcb;
}
#endif

/**
 * set the global thread ID in a TCB
 * @param tid	new thread ID
 */
INLINE void tcb_t::set_global_id(threadid_t tid)
{
    myself_global = tid;
    get_utcb()->my_global_id = tid;
}

INLINE void tcb_t::set_cpu(cpuid_t cpu) 
{ 
    this->cpu = cpu;
    get_utcb()->processor_no = cpu;
}

/**
 * set new pager for a thread
 * @param tid threadid of new pager
 *
 * The pager is stored in a TCR, probably in the UTCB
 */
INLINE void tcb_t::set_pager(const threadid_t tid)
{
    get_utcb()->pager = tid;
}

/**
 * set new exception handler for a thread
 * @param tid threadid of new exception handler
 *
 * The exception handler is stored in a TCR, probably in the UTCB
 */
INLINE void tcb_t::set_exception_handler(const threadid_t tid)
{
    get_utcb()->exception_handler = tid;
}

/**
 * set user-defined handle for a thread
 * @param handle new value for user-defined handle
 *
 * The user-defined handle is stored in a TCR, probably in the UTCB
 */
INLINE void tcb_t::set_user_handle(const word_t handle)
{
    get_utcb()->user_defined_handle = handle;
}


INLINE void tcb_t::set_actual_sender(threadid_t tid)
{
    get_utcb()->virtual_sender = tid;
}

INLINE threadid_t tcb_t::get_virtual_sender()
{
    return get_utcb()->virtual_sender;
}

/**
 * get thread ID of a thread's pager
 *
 * The pager is stored in a TCR, probably in the UTCB
 */
INLINE threadid_t tcb_t::get_pager()
{
    return get_utcb()->pager;
}

/**
 * get thread ID of a thread's exception handler
 *
 * The exception handler is stored in a TCR, probably in the UTCB
 */
INLINE threadid_t tcb_t::get_exception_handler()
{
    return get_utcb()->exception_handler;
}

/**
 * get a thread's user-defined handle
 *
 * The user-defined handle is stored in a TCR, probably in the UTCB
 */
INLINE word_t tcb_t::get_user_handle()
{
    return get_utcb()->user_defined_handle;
}

INLINE time_t tcb_t::get_xfer_timeout_snd (void)
{
    UNIMPLEMENTED();
    return time_t::never();
}

INLINE time_t tcb_t::get_xfer_timeout_rcv (void)
{
    UNIMPLEMENTED();
    return time_t::never();
}

/* This function is very seedy.  Basically, we don't save any S registers
 * on the IPC path, so we can just return as per usual. To cleanup the stack,
 * we need to do the whole thing in asm to make sure C doesn't interfere.
 *
 */
INLINE void tcb_t::return_from_ipc (void)
{
    alpha_context_t *ctx = get_alpha_context(this);
    register word_t ret asm(R_MR0) = get_mr(0);

    __asm__ volatile ("mov %0, $30; bsr $31, __finish_sys" : : "r" (ctx), "r" (ret) : "memory");
}

INLINE void tcb_t::return_from_user_interruption (void)
{
    UNIMPLEMENTED();
}

/**
 * invoke an IPC from within the kernel
 *
 * @param to_tid destination thread id
 * @param from_tid from specifier
 * @param timeout IPC timeout
 * @return IPC message tag (MR0)
 */
INLINE msg_tag_t tcb_t::do_ipc (threadid_t to_tid, threadid_t from_tid,
                                timeout_t timeout)
{
    msg_tag_t tag;

    sys_ipc (to_tid, from_tid, timeout);
    tag.raw = get_mr (0);

    return tag;
}

/********************************************************************** 
 *
 *                      thread switch routines
 *
 **********************************************************************/

/**
 * switch to initial thread
 * @param tcb TCB of initial thread
 *
 * Initializes context of initial thread and switches to it.  The
 * context (e.g., instruction pointer) has been generated by inserting
 * a notify procedure context on the stack.  We simply restore this
 * context.
 */
INLINE void NORETURN initial_switch_to (tcb_t * tcb)
{
    alpha_pcb_t *pcb = &(tcb->get_arch()->pcb);
    
    /* sjw (29/07/2002): Will this work? */
    pcb->asn = get_kernel_space()->get_asid()->get();
    pcb->ptbr = (word_t) virt_to_phys(get_kernel_space()->pgent(0)) >> ALPHA_PAGE_BITS;

    alpha_switch_to((word_t) virt_to_phys(pcb));
    
    /* sjw (29/07/2002): If we get here we have a problem */
    ASSERT(!"We shouldn't get here!");
    while(true) {}
}

/**
 * switch to idle thread
 */
INLINE void do_switch_to_idle()
{
    alpha_pcb_t *idle, *current;

    /* sjw (29/07/2002): Probably not as efficient as it could be */
    idle = &get_idle_tcb()->get_arch()->pcb;
    current = &get_current_tcb()->get_arch()->pcb;

    /* Make sure that the idle thread runs in the initial address space */
    idle->ptbr = current->ptbr;
    idle->asn = current->asn;

    alpha_switch_to((word_t) virt_to_phys(idle));
}

/**
 * switches to another tcb thereby switching address spaces if needed
 * @param dest tcb to switch to
 */
INLINE void tcb_t::switch_to(tcb_t * dest)
{
    /* sjw (27/07/2002): Stuff that isn't done here, but should be:
     *   - saving/restoring FP state 
     */
    if(dest == get_idle_tcb()) {
	do_switch_to_idle();
	return;
    }

    alpha_pcb_t *pcb = &dest->get_arch()->pcb;
    space_t *space = dest->get_space();
       
    if(space) 
	pcb->asn = space->get_asid()->get();
    else 
	pcb->asn = get_kernel_space()->get_asid()->get();

    /* This requires the _physical_ address of the pcb */
    word_t pcb_paddr = dest->get_arch()->pcb_paddr;

    ASSERT(pcb_paddr != NULL);

    alpha_switch_to(pcb_paddr);    
}

#if 0
/**
 * switch to idle thread
 */
INLINE void tcb_t::switch_to_idle()
{
    alpha_pcb_t *idle, *current;

    /* sjw (29/07/2002): Probably not as efficient as it could be */
    idle = &get_idle_tcb()->get_arch()->pcb;
    current = &get_current_tcb()->get_arch()->pcb;

    /* Make sure that the idle thread runs in the initial address space */
    idle->ptbr = current->ptbr;
    idle->asn = current->asn;

    alpha_switch_to((word_t) virt_to_phys(idle));
}
#endif

INLINE word_t *tcb_t::get_stack_top(void)
{
    /* sjw (29/07/2002): Evil? */
    return (word_t *) ((char *) this + KTCB_SIZE);
}

/**
 * intialize stack for given thread
 */
INLINE void tcb_t::init_stack()
{
    get_arch()->pcb.ksp = (word_t) get_alpha_context(this);
}



/**********************************************************************
 *
 *                        notification functions
 *
 **********************************************************************/

/**
 * create stack frame to invoke notify procedure
 * @param func notify procedure to invoke
 *
 * Create a stack frame in TCB so that next thread switch will invoke
 * the indicated notify procedure.
 */
INLINE void tcb_t::notify (void (*func)())
{
    notify((void (*)(word_t, word_t)) func, 0, 0);
}


/**
 * create stack frame to invoke notify procedure
 * @param func notify procedure to invoke
 * @param arg1 1st argument to notify procedure
 *
 * Create a stack frame in TCB so that next thread switch will invoke
 * the indicated notify procedure.
 */
INLINE void tcb_t::notify (void (*func)(word_t), word_t arg1)
{
    notify((void (*)(word_t, word_t)) func, arg1, 0);
}


/**
 * create stack frame to invoke notify procedure
 * @param func notify procedure to invoke
 * @param arg1 1st argument to notify procedure
 * @param arg2 2nd argument to notify procedure
 *
 * Create a stack frame in TCB so that next thread switch will invoke
 * the indicated notify procedure.
 */

INLINE void tcb_t::notify (void (*func)(word_t, word_t), word_t arg1, word_t arg2)
{
    /* We need to fake a stack frame as in alpha_switch_to */
    word_t stack = get_arch()->pcb.ksp - sizeof(alpha_switch_stack_t);
    alpha_switch_stack_t *fake = (alpha_switch_stack_t *) stack;

    fake->r15 = (word_t) func;
    fake->r14 = arg1;
    fake->r13 = arg2;
    fake->ra = (word_t) alpha_return_from_notify;
    
    get_arch()->pcb.ksp = stack;
}

/**********************************************************************
 * 
 *            access functions for ex-regs'able registers
 *
 **********************************************************************/

/**
 * read the user-level instruction pointer
 * @return	the user-level stack pointer
 */
INLINE addr_t tcb_t::get_user_ip()
{
    return (addr_t) get_alpha_context(this)->pc;
}

/**
 * read the user-level stack pointer
 * @return	the user-level stack pointer
 */
INLINE addr_t tcb_t::get_user_sp()
{
    if(this == get_current_tcb()) 
	return (addr_t) PAL::rdusp();

    return (addr_t) get_arch()->pcb.usp;
}

/**
 * set the user-level instruction pointer
 * @param ip	new user-level instruction pointer
 */
INLINE void tcb_t::set_user_ip(addr_t ip)
{
    get_alpha_context(this)->pc = (word_t) ip;
}

/**
 * set the user-level stack pointer
 * @param sp	new user-level stack pointer
 */
INLINE void tcb_t::set_user_sp(addr_t sp)
{
    if(this == get_current_tcb()) {
	PAL::wrusp((word_t) sp);
    } else
	get_arch()->pcb.usp = (word_t) sp;
}


/**
 * read the user-level flags (one word)
 * @return	the user-level flags
 */
INLINE word_t tcb_t::get_user_flags (void)
{
    /* sjw (29/07/2002): Probably a waste of time */
    return get_arch()->pcb.unique;
}

/**
 * set the user-level flags
 * @param flags	new user-level flags
 */
INLINE void tcb_t::set_user_flags (const word_t flags)
{
    get_arch()->pcb.unique = flags;
}

INLINE preempt_flags_t tcb_t::get_preempt_flags (void)
{
    preempt_flags_t flags;
    flags.raw = get_utcb ()->preempt_flags;
    return flags;
}

INLINE void tcb_t::set_preempt_flags (preempt_flags_t flags)
{
    get_utcb ()->preempt_flags = flags.raw;
}

INLINE threadid_t tcb_t::get_intended_receiver (void)
{
    return get_utcb ()->intended_receiver;
}

INLINE u8_t tcb_t::get_cop_flags (void)
{
    return get_utcb ()->cop_flags;
}


INLINE word_t tcb_t::get_utcb_location()
{
    utcb_t * dummy = (utcb_t*)NULL;
    return myself_local.get_raw() - ((word_t)&dummy->mr[0]);
}

INLINE void tcb_t::set_utcb_location(word_t utcb_location)
{
    utcb_t * dummy = (utcb_t*)NULL;
    myself_local.set_raw (utcb_location + ((word_t)&dummy->mr[0]));
}

/**********************************************************************
 *
 *                  copy-area related functions
 *
 **********************************************************************/

INLINE addr_t tcb_t::get_copy_area (tcb_t * dst, addr_t addr)
{
    UNIMPLEMENTED ();
    return addr;
}

INLINE void tcb_t::release_copy_area (void)
{
/* sjw (21/11/2002): FIXME */
//    UNIMPLEMENTED ();
}

INLINE addr_t tcb_t::copy_area_real_address (addr_t addr)
{
    UNIMPLEMENTED ();
    return addr;
}


#endif /* !__GLUE__V4_TEMPLATE__TCB_H__ */
