/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     glue/v4-tmplarch/tcb.h
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
 * $Id: tcb.h,v 1.12 2003/04/29 23:27:44 uhlig Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_TEMPLATE__TCB_H__
#define __GLUE__V4_TEMPLATE__TCB_H__

#ifndef __API__V4__TCB_H__
#error not for stand-alone inclusion
#endif


#include INC_API(syscalls.h)		/* for sys_ipc */


#warning PORTME
/**
 * sets the error descriptor of the tcb
 * @param err new value
 */
INLINE void tcb_t::set_ipc_error(word_t err)
{
    UNIMPLEMENTED();

    //get_utcb()->error_code = err;
}

#warning PORTME
/**
 * get the current ipc error code
 */
INLINE word_t tcb_t::get_ipc_error()
{
    UNIMPLEMENTED();
    return 0;

    //return get_utcb()->error_code;
}

#warning PORTME
/**
 * read value of message register
 * @param index number of message register
 */
INLINE word_t tcb_t::get_mr(word_t index)
{
    UNIMPLEMENTED();
    return 0;

    //return get_utcb()->mr[index];
}

#warning PORTME
/**
 * set the value of a message register
 * @param index number of message register
 * @param value value to set
 */
INLINE void tcb_t::set_mr(word_t index, word_t value)
{
    UNIMPLEMENTED();

    //get_utcb()->mr[index] = value;
}


#warning PORTME
/**
 * copies a set of message registers from one UTCB to another
 * @param dest destination TCB
 * @param start MR start index
 * @param count number of MRs to be copied
 */
INLINE void tcb_t::copy_mrs(tcb_t * dest, word_t start, word_t count)
{
    ASSERT(start + count < IPC_NUM_MR);
    UNIMPLEMENTED();

    // for (word_t idx = start; idx < start + count; idx++)
    //      dest->set_mr(idx, this->get_mr(idx));
}


#warning PORTME
/**
 * read value of buffer register
 * @param index number of buffer register
 */
INLINE word_t tcb_t::get_br(word_t index)
{
    UNIMPLEMENTED();
    return 0;

    //return get_utcb()->br[index];
}

#warning PORTME
/**
 * set the value of a buffer register
 * @param index number of buffer register
 * @param value value to set
 */
INLINE void tcb_t::set_br(word_t index, word_t value)
{
    UNIMPLEMENTED();

    //get_utcb()->br[index] = value;
}


/**
 * allocate the tcb
 * The tcb pointed to by this will be allocated.
 */
INLINE void tcb_t::allocate()
{
    UNIMPLEMENTED();

    // possible implementation: touch *this
}


#warning PORTME
/**
 * set the address space a TCB belongs to
 * @param space address space the TCB will be associated with
 */
INLINE void tcb_t::set_space(space_t * space)
{
    UNIMPLEMENTED();

    this->space = space;
    // sometimes it might be desirable to use a pdir cache,
    // like in cases where it's not cheap to derive the page
    // directory from the space
    //this->pdir_cache = (word_t)space->get_pdir();
}




#warning PORTME
/**
 * set local ID of a thread
 * @param utcb ???
 *
 * For some reason we generally use a utcb pointer to derive the local
 * ID from.  For IA-32 the local ID is specified to be a pointer into
 * the UTCB, but this is not a must.
 */
INLINE void tcb_t::set_local_id(utcb_t * utcb)
{
    UNIMPLEMENTED();

    //myself_local.set_raw((word_t)&utcb->mr[0]);
}

/**
 * set the global thread ID in a TCB
 * @param tid	new thread ID
 */
INLINE void tcb_t::set_global_id(threadid_t tid)
{
#warning PORTME
    UNIMPLEMENTED();
    //myself_global = tid;
    //get_utcb()->my_global_id = tid;
}

#warning PORTME
/**
 * set new pager for a thread
 * @param tid threadid of new pager
 *
 * The pager is stored in a TCR, probably in the UTCB
 */
INLINE void tcb_t::set_pager(const threadid_t tid)
{
    UNIMPLEMENTED();

    //get_utcb()->pager = tid;
}

#warning PORTME
/**
 * set new exception handler for a thread
 * @param tid threadid of new exception handler
 *
 * The exception handler is stored in a TCR, probably in the UTCB
 */
INLINE void tcb_t::set_exception_handler(const threadid_t tid)
{
    UNIMPLEMENTED();

    //get_utcb()->exception_handler = tid;
}

#warning PORTME
/**
 * set user-defined handle for a thread
 * @param handle new value for user-defined handle
 *
 * The user-defined handle is stored in a TCR, probably in the UTCB
 */
INLINE void tcb_t::set_user_handle(const word_t handle)
{
    UNIMPLEMENTED();

    //get_utcb()->user_defined_handle = handle;
}

#warning PORTME
/**
 * get thread ID of a thread's pager
 *
 * The pager is stored in a TCR, probably in the UTCB
 */
INLINE threadid_t tcb_t::get_pager()
{
    UNIMPLEMENTED();
    return threadid_t::nilthread();
    
    //return get_utcb()->pager;
}

#warning PORTME
/**
 * get thread ID of a thread's exception handler
 *
 * The exception handler is stored in a TCR, probably in the UTCB
 */
INLINE threadid_t tcb_t::get_exception_handler()
{
    UNIMPLEMENTED();
    return threadid_t::nilthread();

    //return get_utcb()->exception_handler;
}

#warning PORTME
/**
 * get a thread's user-defined handle
 *
 * The user-defined handle is stored in a TCR, probably in the UTCB
 */
INLINE word_t tcb_t::get_user_handle()
{
    UNIMPLEMENTED();
    return 0;
    
    //return get_utcb()->user_defined_handle;
}

#warning PORTME
/**
 * get a thread's send transfer timeout
 *
 * The transfer timeout is stored in a TCR, probably in the UTCB
 */
INLINE time_t tcb_t::get_xfer_timeout_snd (void)
{
    UNIMPLEMENTED();
    return time_t::never();
}

#warning PORTME
/**
 * get a thread's receive transfer timeout
 *
 * The transfer timeout is stored in a TCR, probably in the UTCB
 */
INLINE time_t tcb_t::get_xfer_timeout_rcv (void)
{
    UNIMPLEMENTED();
    return time_t::never();
}

#warning PORTME
/**
 * get a the previous IPC operation's actual sender
 *
 * The actual sender is stored in a TCR, probably in the UTCB
 */
INLINE void tcb_t::set_actual_sender (threadid_t tid)
{
    UNIMPLEMENTED();
}

#warning PORTME
/**
 * get a thread's virtual sender
 *
 * The virtual sender is stored in a TCR, probably in the UTCB
 */
INLINE threadid_t tcb_t::get_virtual_sender (void)
{
    UNIMPLEMENTED();
    return threadid_t::nilthread ();
}

#warning PORTME
/**
 * get the previous IPC operation's intended receiver
 *
 * The intended receiver is stored in a TCR, probably in the UTCB
 */
INLINE threadid_t tcb_t::get_intended_receiver (void)
{
    UNIMPLEMENTED();
    return threadid_t::nilthread ();
}

#warning PORTME
/**
 * get the thread's preemption flags
 *
 * The preemption flags are stored in a TCR, probably in the UTCB
 */
INLINE preempt_flags_t tcb_t::get_preempt_flags (void)
{
    UNIMPLEMENTED();
    preempt_flags_t flags;
    flags.raw = 0;
    return flags;
}

#warning PORTME
/**
 * sets the thread's preemption flags
 *
 * The preemption flags are stored in a TCR, probably in the UTCB
 */
INLINE void tcb_t::set_preempt_flags (preempt_flags_t flags)
{
    UNIMPLEMENTED();
}


#warning PORTME
/**
 * get the thread's coprocessor flags
 *
 * The coprocessor flags are stored in a TCR, probably in the UTCB
 */
INLINE u8_t tcb_t::get_cop_flags (void)
{
    UNIMPLEMENTED();
    return 0;
}

#warning PORTME
/**
 * Short circuit a return path from an IPC system call.  The error
 * code TCR and message registers are already set properly.  The
 * function only needs to restore the appropriate user context and
 * return execution to the instruction directly following the IPC
 * system call.
 */
INLINE void tcb_t::return_from_ipc (void)
{
    UNIMPLEMENTED();
}


#warning PORTME
/**
 * Short circuit a return path from a user-level interruption or
 * exception.  That is, restore the complete exception context and
 * resume execution at user-level.
 */
INLINE void tcb_t::return_from_user_interruption (void)
{
    UNIMPLEMENTED();
}


/********************************************************************** 
 *
 *                      thread switch routines
 *
 **********************************************************************/

#warning PORTME
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
    UNIMPLEMENTED();
}

#warning PORTME
/**
 * switches to another tcb thereby switching address spaces if needed
 * @param dest tcb to switch to
 */
INLINE void tcb_t::switch_to(tcb_t * dest)
{
    UNIMPLEMENTED();
}

#warning PORTME
/**
 * switch to idle thread
 */
INLINE void tcb_t::switch_to_idle()
{
    UNIMPLEMENTED();
}




/**
 * intialize stack for given thread
 */
INLINE void tcb_t::init_stack()
{
#warning PORTME
    UNIMPLEMENTED();
    // stack = get_stack_top();
}




/**********************************************************************
 *
 *                        notification functions
 *
 **********************************************************************/

#warning PORTME
/**
 * create stack frame to invoke notify procedure
 * @param func notify procedure to invoke
 *
 * Create a stack frame in TCB so that next thread switch will invoke
 * the indicated notify procedure.
 */
INLINE void tcb_t::notify (void (*func)())
{
    UNIMPLEMENTED();
}

#warning PORTME
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
    UNIMPLEMENTED();
}





/**********************************************************************
 * 
 *            access functions for ex-regs'able registers
 *
 **********************************************************************/

#warning PORTME
/**
 * read the user-level instruction pointer
 * @return	the user-level stack pointer
 */
INLINE addr_t tcb_t::get_user_ip()
{
    UNIMPLEMENTED();
    return NULL;
}

#warning PORTME
/**
 * read the user-level stack pointer
 * @return	the user-level stack pointer
 */
INLINE addr_t tcb_t::get_user_sp()
{
    UNIMPLEMENTED();
    return NULL;
}

#warning PORTME
/**
 * set the user-level instruction pointer
 * @param ip	new user-level instruction pointer
 */
INLINE void tcb_t::set_user_ip(addr_t ip)
{
    UNIMPLEMENTED();
}

#warning PORTME
/**
 * set the user-level stack pointer
 * @param sp	new user-level stack pointer
 */
INLINE void tcb_t::set_user_sp(addr_t sp)
{
    UNIMPLEMENTED();
}


/**
 * read the user-level flags (one word)
 * @return	the user-level flags
 */
INLINE word_t tcb_t::get_user_flags (void)
{
#warning PORTME
    UNIMPLEMENTED();
    return 0;
}

/**
 * set the user-level flags
 * @param flags	new user-level flags
 */
INLINE void tcb_t::set_user_flags (const word_t flags)
{
#warning PORTME
    UNIMPLEMENTED();
}

/**********************************************************************
 *
 *                  copy-area related functions
 *
 **********************************************************************/

/**
 * Enable copy area for current thread.
 *
 * @param dst		destination TCB for IPC copy operation
 * @param addr		destination address
 *
 * @return addresss which the kernel should use for destination
 */
INLINE addr_t tcb_t::get_copy_area (tcb_t * dst, addr_t addr)
{
#warning PORTME
    UNIMPLEMENTED ();
    return addr;
}

/**
 * Release copy area(s) for current thread.
 */
INLINE void tcb_t::release_copy_area (void)
{
#warning PORTME
    UNIMPLEMENTED ();
}

/**
 * Retrieve the real address associated with a copy area address.
 *
 * @param addr		address within copy area
 *
 * @return address translated into a regular user-level address
 */
INLINE addr_t tcb_t::copy_area_real_address (addr_t addr)
{
#warning PORTME
    UNIMPLEMENTED ();
    return addr;
}

/**********************************************************************
 *
 *                        global tcb functions
 *
 **********************************************************************/

#warning PORTME
/**
 * Locate current TCB by using current stack pointer and return it.
 */
INLINE tcb_t * get_current_tcb (void)
{
    UNIMPLEMENTED();
    return NULL;
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
#warning PORTME
    msg_tag_t tag;

    sys_ipc (to_tid, from_tid, timeout);
    tag.raw = get_mr (0);

    return tag;
}


#endif /* !__GLUE__V4_TEMPLATE__TCB_H__ */
