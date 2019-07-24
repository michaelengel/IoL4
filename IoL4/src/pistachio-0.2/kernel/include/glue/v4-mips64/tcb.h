/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,   University of New South Wales
 *                
 * File path:     glue/v4-mips64/tcb.h
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
 * $Id: tcb.h,v 1.24 2003/05/01 14:56:34 sjw Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_MIPS64__TCB_H__
#define __GLUE__V4_MIPS64__TCB_H__

#ifndef __API__V4__TCB_H__
#error not for stand-alone inclusion
#endif

#include <debug.h>

#include INC_ARCH(types.h)
#include INC_API(syscalls.h)		/* for sys_ipc */
#include INC_API(space.h)
#include INC_ARCH(page.h)
#include INC_GLUE(context.h)
#include INC_PLAT(cache.h)

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
 * returns value of message tag
 */
INLINE msg_tag_t tcb_t::get_tag()
{
    msg_tag_t tag;
    tag.raw = get_mr(0);
    return tag;
}

INLINE void tcb_t::set_tag (msg_tag_t tag)
{
    set_mr (0, tag.raw);
}

/**
 * read value of message register
 * @param index number of message register
 */
INLINE word_t tcb_t::get_mr(word_t index)
{
    return get_utcb()->mr[index];
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
    this->kernel_stack[0] = 0;

/**
 * tcb_t::allocate: allocate memory for TCB
 *
 * Allocate memory for the given TCB.  We do this by generating a
 * write to the TCB area.  If TCB area is not backed by writable
 * memory (i.e., already allocated) the pagefault handler will
 * allocate the memory and map it.
 */

}


/**
 * set the address space a TCB belongs to
 * @param space address space the TCB will be associated with
 */
INLINE void tcb_t::set_space(space_t * space)
{
    this->space = space;
    // sometimes it might be desirable to use a pdir cache,
    // like in cases where it's not cheap to derive the page
    // directory from the space
    //this->pdir_cache = (word_t)space->get_pdir();
}


/**
 * set the global thread ID in a TCB
 * @param tid	new thread ID
 */
INLINE void tcb_t::set_global_id(threadid_t tid)
{
    /* FIXME */

    myself_global = tid;
    get_utcb()->my_global_id = tid;
}

/**
 * set the cpu in a TCB
 * @param cpu	new cpu number
 */

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


/* XXX FIXME - This does not always get stack right! */
INLINE void tcb_t::return_from_ipc (void)
{
    mips64_irq_context_t * context =
	(mips64_irq_context_t *) get_stack_top () - 1;

//printf("warning - return_from_ipc to %p\n", context);
	/* subtract 8 from context as wel need to restore s8
	 * which was saved at context - 8 */
    context = (mips64_irq_context_t *)((word_t) context - 8);

    __asm__ __volatile__ (
	"move	$29, %0				\n"
	"j	_mips64_l4sysipc_return	\n"
	: : "r" (context)
    );
    // NOT REACHED
}

/**
 * Short circuit a return path from a user-level interruption or
 * exception.  That is, restore the complete exception context and
 * resume execution at user-level.
 */
INLINE void tcb_t::return_from_user_interruption (void)
{
    mips64_irq_context_t * context =
	(mips64_irq_context_t *) get_stack_top () - 1;

//printf("warning - return_from_user_interruption\n");
    __asm__ __volatile__ (
	"move	$29, %0			\n"
	"j	_mips64_restore_context	\n"
	: : "r" (context)
    );
    // NOT REACHED
}

/********************************************************************** 
 *
 *                      thread switch routines
 *
 **********************************************************************/

extern word_t K_STACK_BOTTOM;

#define mips64_initial_switch_to(d_stack, d_asid, d_space)	\
__asm__ __volatile__ (		\
    "dmtc0   %[asid], "STR(CP0_ENTRYHI)"\n\t" /* Set new ASID */		\
    "move    $29, %[stack]\n\t"	    /* Install the new stack */		\
    ".set noat;\n\t"	\
    "dsll    $1, %[space], 32\n\t"	    \
    "dmtc0   $1, "STR(CP0_CONTEXT)"\n\t" /* Save current Page Table */		\
    ".set at;\n\t"	\
    "or	    %[stack], 4096-1\n\t"	    \
    "daddiu  %[stack], 1\n\t"		\
/*    "lui    t0, %hi(K_STACK_BOTTOM)\n\t"	    \
    "sd	    %1, %lo(K_STACK_BOTTOM)(t0)\n\t"*/		\
    "sd     %[stack], 0(%[stack_bot])\n\t"	    \
	    \
    "ld      $31,32($29)\n\r"	    \
    "ld      $16,0($29)\n\t"	    \
    "ld      $17,8($29)\n\t"	    \
    "ld      $30,16($29)\n\t"	    \
    "ld      $28,24($29)\n\t"	    \
    "daddiu  $29,$29,40\n\t"	    \
    "jr	     $31\n\t"		    \
    "0:		\t\t"		 /* Return Address */   \
    : : [stack] "r" (d_stack),	    \
        [asid]  "r" (d_asid),		    \
	[space] "r" (d_space),	    \
	[stack_bot]"r" (&K_STACK_BOTTOM) :  \
	"$1", "$31"		    \
    );	\
    __asm__ __volatile__ ("" ::: "$2", "$3", "$4", "$5", "$6", "$7" );	    \
    __asm__ __volatile__ ("" ::: "$8", "$9", "$10", "$23", "$24", "$25" );  \
    __asm__ __volatile__ ("" ::: "$11", "$12", "$13", "$14", "$15" );	    \
    __asm__ __volatile__ ("" ::: "$18", "$19", "$20", "$21", "$22" );

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
    hw_asid_t new_asid = get_kernel_space()->get_asid()->get();

    //TRACEF("(%p), %p\n", tcb, tcb->stack);
    cache_t::flush_cache_all();
    mips64_initial_switch_to((word_t)tcb->stack, new_asid, (word_t)tcb->get_space());

    ASSERT(!"We shouldn't get here!");
    while(true) {}
}

/**
 * read the current instruction pointer
 * @return	the user-level stack pointer
 */
INLINE addr_t get_my_ip(tcb_t *tcb)
{
    mips64_switch_stack_t * context = (mips64_switch_stack_t *)tcb->stack;

    return (addr_t) (context)->ra;
}

/**
 * read the current stack pointer
 * @return	the user-level stack pointer
 */
INLINE addr_t get_my_sp(tcb_t *tcb)
{
    mips64_switch_stack_t * context = (mips64_switch_stack_t *)tcb->stack;
    context++;

    return (addr_t) (context);
}

#define mips64_switch_to(t_stack, d_stack, d_asid, d_space)	\
__asm__ __volatile__ (		    \
    "daddiu  $29,$29,-40\n\t"	    \
    "dla     $31,0f\n\t"	    \
    "sd      $16,0($29)\n\t"	    \
    "sd      $17,8($29)\n\t"	    \
    "sd      $30,16($29)\n\t"	    \
    "sd      $28,24($29)\n\t"	    \
    "sd      $31,32($29)\n\r"	    \
    \
    "sd	    $29, 0(%[old_stack])\n\t"	    /* Store current stack in old_stack */	    \
    \
    "dmtc0   %[asid], "STR(CP0_ENTRYHI)"\n\t" /* Set new ASID */		\
    "move    $29, %[new_stack]\n\t"	    /* Install the new stack */		\
    ".set noat;\n\t"	\
    "dsll    $1, %[space], 32\n\t"	    \
    "dmtc0   $1, "STR(CP0_CONTEXT)"\n\t" /* Save current Page Table */		\
    ".set at;\n\t"	\
    "or	    %[new_stack], 4096-1\n\t"	    \
    "daddiu  %[new_stack], 1\n\t"	    \
/*    "lui    t0, %hi(K_STACK_BOTTOM)\n\t"	\
    "sd	    %1, %lo(K_STACK_BOTTOM)(t0)\n\t"*/	\
    "sd     %[new_stack], 0(%[stack_bot])\n\t"	\
	    \
    "ld      $31,32($29)\n\r"	    \
    "ld      $16,0($29)\n\t"	    \
    "ld      $17,8($29)\n\t"	    \
    "ld      $30,16($29)\n\t"	    \
    "ld      $28,24($29)\n\t"	    \
    "daddiu  $29,$29,40\n\t"	    \
    "jr	     $31\n\t"		    \
    "0:		\t\t"		 /* Return Address */   \
    : : [old_stack] "r" (t_stack),  \
	[new_stack] "r" (d_stack),  \
        [asid]  "r" (d_asid),	    \
	[space] "r" (d_space),	    \
	[stack_bot]"r" (&K_STACK_BOTTOM) :  \
	"$1", "$31"		    \
    );	\
    __asm__ __volatile__ ("" ::: "$2", "$3", "$4", "$5", "$6", "$7" );	    \
    __asm__ __volatile__ ("" ::: "$8", "$9", "$10", "$23", "$24", "$25" );  \
    __asm__ __volatile__ ("" ::: "$11", "$12", "$13", "$14", "$15" );	    \
    __asm__ __volatile__ ("" ::: "$18", "$19", "$20", "$21", "$22" );

/**
 * switches to another tcb thereby switching address spaces if needed
 * @param dest tcb to switch to
 */
INLINE void tcb_t::switch_to(tcb_t * dest)
{
    /* carl - XXX need to save fp context ? */
    space_t *space = dest->get_space();
    if (space == NULL)
	space = get_kernel_space();
    hw_asid_t new_asid = space->get_asid()->get();
//    cache_t::flush_cache_all();
    mips64_switch_to((word_t)&this->stack, (word_t)dest->stack, new_asid, (word_t)space);
}

INLINE word_t *tcb_t::get_stack_top(void)
{
    /* Simon says : Evil? */
    return (word_t *) ((char *) this + KTCB_SIZE);
}


/**
 * intialize stack for given thread
 */
INLINE void tcb_t::init_stack()
{
    /* Create space for an exception context */
    mips64_irq_context_t * context =
	(mips64_irq_context_t *) get_stack_top () - 1;
    stack = (word_t *) context; /* Update new stack position */

    /* Clear whole context */
    for (word_t * t = (word_t *) context; t < get_stack_top (); t++)
	*t = 0;

    //TRACEF("[%p] stack = %p\n", this, stack);
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
    ((mips64_switch_stack_t *)stack)--;
    ((mips64_switch_stack_t *)stack)->s8 = (word_t) func;
    ((mips64_switch_stack_t *)stack)->ra = (word_t) mips64_return_from_notify0;

    //TRACEF("%p (%p) , %016lx\n", this, func, (word_t)stack);
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
    ((mips64_switch_stack_t *)stack)--;
    ((mips64_switch_stack_t *)stack)->s8 = (word_t) func;
    ((mips64_switch_stack_t *)stack)->s0 = arg1;
    ((mips64_switch_stack_t *)stack)->ra = (word_t) mips64_return_from_notify1;

    //TRACEF("%p (%p)(0x%x), %016lx\n", this, func, arg1, (word_t)stack);
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
    ((mips64_switch_stack_t *)stack)--;
    ((mips64_switch_stack_t *)stack)->s8 = (word_t) func;
    ((mips64_switch_stack_t *)stack)->s0 = arg1;
    ((mips64_switch_stack_t *)stack)->s1 = arg2;
    ((mips64_switch_stack_t *)stack)->ra = (word_t) mips64_return_from_notify2;

    //TRACEF("%p (%p)(0x%x,0x%x), %016lx\n", this, func, arg1, arg2, (word_t)stack);
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
    mips64_irq_context_t * context =
	(mips64_irq_context_t *) get_stack_top () - 1;

    return (addr_t) (context)->epc;
}

/**
 * read the user-level stack pointer
 * @return	the user-level stack pointer
 */
INLINE addr_t tcb_t::get_user_sp()
{
    mips64_irq_context_t * context =
	(mips64_irq_context_t *) get_stack_top () - 1;

    return (addr_t) (context)->sp;
}


/**
 * set the user-level instruction pointer
 * @param ip	new user-level instruction pointer
 */
INLINE void tcb_t::set_user_ip(addr_t ip)
{
    mips64_irq_context_t * context =
	(mips64_irq_context_t *) get_stack_top () - 1;

    context->epc = (word_t)ip;
}

/**
 * set the user-level stack pointer
 * @param sp	new user-level stack pointer
 */
INLINE void tcb_t::set_user_sp(addr_t sp)
{
    mips64_irq_context_t * context =
	(mips64_irq_context_t *) get_stack_top () - 1;

    context->sp = (word_t)sp;
}

INLINE threadid_t tcb_t::get_intended_receiver (void)
{
    return get_utcb ()->intended_receiver;
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


/**
 * read the user-level flags (one word)
 * @return	the user-level flags
 */
INLINE word_t tcb_t::get_user_flags (void)
{
    mips64_irq_context_t * context =
	(mips64_irq_context_t *) get_stack_top () - 1;

    return context->status;
}

/**
 * set the user-level flags
 * @param flags	new user-level flags
 */
INLINE void tcb_t::set_user_flags (const word_t flags)
{
    mips64_irq_context_t * context =
	(mips64_irq_context_t *) get_stack_top () - 1;

    context->status = flags;
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
//    UNIMPLEMENTED (); XXX if should not be a problem as long as get_copy_area is UNIMPLENTED
}

INLINE addr_t tcb_t::copy_area_real_address (addr_t addr)
{
    UNIMPLEMENTED ();
    return addr;
}


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
    register word_t stack_var asm("$29");

    return (tcb_t *) (stack_var & KTCB_MASK);

};

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
    sys_ipc(to_tid, from_tid, timeout);
    tag.raw = get_mr (0);

    return tag;
}


#endif /* !__GLUE__V4_MIPS64__TCB_H__ */
