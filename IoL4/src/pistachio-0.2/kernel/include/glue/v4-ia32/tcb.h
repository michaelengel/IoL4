/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  Karlsruhe University
 *                
 * File path:     glue/v4-ia32/tcb.h
 * Description:   specific implementations
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
 * $Id: tcb.h,v 1.38 2003/05/01 12:25:48 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_IA32__TCB_H__
#define __GLUE__V4_IA32__TCB_H__

#include INC_ARCH(tss.h)
#include INC_ARCH(trapgate.h)
#include INC_API(syscalls.h)
#include INC_GLUE(resource_functions.h)

/* forward declaration */
tcb_t * get_idle_tcb();

INLINE void tcb_t::set_utcb_location(word_t utcb_location)
{
    utcb_t * dummy = (utcb_t*)NULL;
    myself_local.set_raw (utcb_location + ((word_t)&dummy->mr[0]));
}

INLINE word_t tcb_t::get_utcb_location()
{
    utcb_t * dummy = (utcb_t*)NULL;
    return myself_local.get_raw() - ((word_t)&dummy->mr[0]);
}

INLINE void tcb_t::set_pager(const threadid_t tid)
{
    get_utcb()->pager = tid;
}

INLINE void tcb_t::set_exception_handler(const threadid_t tid)
{
    get_utcb()->exception_handler = tid;
}

INLINE void tcb_t::set_user_handle(const word_t handle)
{
    get_utcb()->user_defined_handle = handle;
}

INLINE threadid_t tcb_t::get_pager()
{
    return get_utcb()->pager;
}

INLINE threadid_t tcb_t::get_exception_handler()
{
    return get_utcb()->exception_handler;
}

INLINE word_t tcb_t::get_user_handle()
{
    return get_utcb()->user_defined_handle;
}

INLINE void tcb_t::set_actual_sender(threadid_t tid)
{
    get_utcb()->virtual_sender = tid;
}

INLINE threadid_t tcb_t::get_virtual_sender()
{
    return get_utcb()->virtual_sender;
}

INLINE threadid_t tcb_t::get_intended_receiver (void)
{
    return get_utcb ()->intended_receiver;
}

INLINE preempt_flags_t tcb_t::get_preempt_flags (void)
{
    preempt_flags_t flags;
    flags.raw = get_utcb()->preempt_flags;
    return flags;
}

INLINE void tcb_t::set_preempt_flags (preempt_flags_t flags)
{
    get_utcb()->preempt_flags = flags.raw;
}

INLINE u8_t tcb_t::get_cop_flags (void)
{
    return get_utcb ()->cop_flags;
}

INLINE void tcb_t::set_global_id(threadid_t tid)
{
    myself_global = tid;
    get_utcb()->my_global_id = tid;
}

INLINE void tcb_t::set_cpu(cpuid_t cpu) 
{ 
    this->cpu = cpu;

    // only update UTCB if there is one
    if (get_utcb())
	get_utcb()->processor_no = cpu;

    // update the pdir cache on migration
    if (space) 
	this->pdir_cache = (word_t)space->get_pdir(get_cpu());
}


/**
 * tcb_t::set_ipc_error: sets the error descriptor of the tcb
 */
INLINE void tcb_t::set_ipc_error(word_t err)
{
    get_utcb()->error_code = err;
}

/**
 * tcb_t::get_ipc_error: returns the current ipc error code
 */
INLINE word_t tcb_t::get_ipc_error()
{
    return get_utcb()->error_code;
}

/**
 * tcb_t::get_mr: returns value of message register
 * @index: number of message register
 */
INLINE word_t tcb_t::get_mr(word_t index)
{
    ASSERT(index < IPC_NUM_MR);
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
 * tcb_t::set_mr: sets the value of a message register
 * @index: number of message register
 * @value: value to set
 */
INLINE void tcb_t::set_mr(word_t index, word_t value)
{
    ASSERT(index < IPC_NUM_MR);
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
    ASSERT(count > 0);
    word_t dummy;

    /* use optimized IA32 copy loop -- uses complete cacheline
       transfers */
    __asm__ __volatile__ (
	"repnz movsl (%%esi), (%%edi)\n"
	: /* output */
	"=S"(dummy), "=D"(dummy), "=c"(dummy)
	: /* input */
	"c"(count), "S"(&get_utcb()->mr[start]), 
	"D"(&dest->get_utcb()->mr[start]));
}

/**
 * tcb_t::get_br: returns value of buffer register
 * @index: number of buffer register
 */
INLINE word_t tcb_t::get_br(word_t index)
{
    ASSERT(index < IPC_NUM_BR);
    return get_utcb()->br[32U-index];
}

/**
 * tcb_t::set_br: sets the value of a buffer register
 * @index: number of buffer register
 * @value: value to set
 */
INLINE void tcb_t::set_br(word_t index, word_t value)
{
    ASSERT(index < IPC_NUM_BR);
    get_utcb()->br[32U-index] = value;
}




INLINE void tcb_t::allocate()
{
    __asm__ __volatile__(
	"orl $0, %0\n"
	: 
	: "m"(*this));

}

INLINE void tcb_t::set_space(space_t * space)
{
    this->space = space;
    this->pdir_cache = (word_t)space->get_pdir(get_cpu());
}

INLINE word_t * tcb_t::get_stack_top()
{
    return (word_t*)addr_offset(this, KTCB_SIZE);
}

INLINE void tcb_t::init_stack()
{
    stack = get_stack_top();
    //TRACE("stack = %p\n", stack);
}



/********************************************************************** 
 *
 *                      thread switch routines
 *
 **********************************************************************/

#ifndef BUILD_TCB_LAYOUT
#include <tcb_layout.h>

/**
 * tcb_t::switch_to: switches to specified tcb
 */
INLINE void tcb_t::switch_to(tcb_t * dest)
{
    extern ia32_tss_t tss;
    word_t dummy;

    ASSERT(dest->stack);
    ASSERT(dest != this);
    ASSERT(get_cpu() == dest->get_cpu());

    if ( EXPECT_FALSE(resource_bits) )
	resources.save(this);

    /* modify stack in tss */
    tss.set_esp0((u32_t)dest->get_stack_top());

    TBUF_RECORD_EVENT("switch %p => %p", (unsigned)this, (unsigned)dest);

#ifdef CONFIG_SMP
    set_active_space(get_cpu(), dest->space);
#endif

    __asm__ __volatile__ (
	"/* switch_to_thread */	\n\t"
	"pushl	%%ebp		\n\t"
	
	"pushl	$3f		\n\t"	/* store return address	*/
	
	"movl	%%esp, %c4(%1)	\n\t"	/* switch stacks	*/
	"movl	%c4(%2), %%esp	\n\t"
#ifndef CONFIG_ARCH_IA32_P4
	"movl	%%cr3, %8	\n\t"	/* load current ptab */
	"cmpl	$0, %c5(%2)	\n\t"	/* if kernel thread-> use current */
	"je	2f		\n\t"
#endif
	"cmpl	%7, %8		\n\t"	/* same page dir?	*/
	"je	2f		\n\t"
#ifdef CONFIG_ARCH_IA32_P4
	"cmpl	$0, %c5(%2)	\n\t"	/* kernel thread (space==NULL)?	*/
	"jne	1f		\n\t"
	"movl	%5, %c6(%2)	\n\t"	/* rewrite dest->pdir_cache */
	"jmp	2f		\n\t"

	"1:			\n\t"
#endif
	"movl	%7, %%cr3	\n\t"	/* reload pagedir */
	"2:			\n\t"
	"popl	%%edx		\n\t"	/* load activation addr */
	"movl	%3, %%gs:0	\n\t"	/* update current UTCB */

	"jmp	*%%edx		\n\t"
	"3:			\n\t"
	"movl	%2, %1		\n\t"	/* restore this */
	"popl	%%ebp		\n\t"
	"/* switch_to_thread */	\n\t"
	: /* trash everything */
	"=a" (dummy)				// 0
	:
	"b" (this),				// 1
	"S" (dest),				// 2
	"D" (dest->get_local_id().get_raw()),	// 3
	"i" (OFS_TCB_STACK),			// 4
	"i" (OFS_TCB_SPACE),			// 5
	"i" (OFS_TCB_PDIR_CACHE),		// 6
	"a" (dest->pdir_cache),			// 7
#ifdef CONFIG_ARCH_IA32_P4
	"c" (this->pdir_cache)			// 8
#else
	"c" (dest->pdir_cache)			// dummy
#endif
	: "edx", "memory"
	);

    if ( EXPECT_FALSE(resource_bits) )
	resources.load(this);
}
#endif


/**********************************************************************
 *
 *                        in-kernel IPC invocation 
 *
 **********************************************************************/

/**
 * tcb_t::do_ipc: invokes an in-kernel IPC 
 * @param to_tid destination thread id
 * @param from_tid from specifier
 * @param timeout IPC timeout
 * @return IPC message tag (MR0)
 */
INLINE msg_tag_t tcb_t::do_ipc(threadid_t to_tid, threadid_t from_tid, timeout_t timeout)
{
    msg_tag_t tag;
    word_t mr1, mr2, dummy;
    asm("pushl	%%ebp		\n"
	"pushl	%%ecx		\n"
	"call	sys_ipc		\n"
	"addl	$4, %%esp	\n"
	"movl	%%ebp, %%ecx	\n"
	"popl	%%ebp		\n"
	: "=S"(tag.raw),
	"=b"(mr1),
	"=c"(mr2),
	"=a"(dummy),
	"=d"(dummy)
	: "a"(to_tid.get_raw()),
	  "d"(from_tid.get_raw()),
	  "c"(timeout.raw)
	: "edi");
    set_mr(1, mr1);
    set_mr(2, mr2);
    return tag;
}


			  
/**********************************************************************
 *
 *                        notification functions
 *
 **********************************************************************/

/* notify trampoline always removes two parameters */
extern "C" void notify_trampoline(void);

INLINE void tcb_t::notify(void (*func)())
{
    *(--stack) = (word_t)func;
}

INLINE void tcb_t::notify(void (*func)(word_t), word_t arg1)
{
    stack--;
    *(--stack) = arg1;
    *(--stack) = (word_t)notify_trampoline;
    *(--stack) = (word_t)func;
}

INLINE void tcb_t::notify(void (*func)(word_t, word_t), word_t arg1, word_t arg2)
{
    *(--stack) = arg2;
    *(--stack) = arg1;
    *(--stack) = (word_t)notify_trampoline;
    *(--stack) = (word_t)func;
}

/* 
 * access functions for ex-regs'able registers
 */
INLINE addr_t tcb_t::get_user_ip()
{
    return (addr_t)get_stack_top()[KSTACK_UIP];
}

INLINE addr_t tcb_t::get_user_sp()
{
    return (addr_t)get_stack_top()[KSTACK_USP];
}

INLINE word_t tcb_t::get_user_flags()
{
    return (word_t)get_stack_top()[KSTACK_UFLAGS];
}

INLINE void tcb_t::set_user_ip(addr_t ip)
{
    get_stack_top()[KSTACK_UIP] = (u32_t)ip;
}

INLINE void tcb_t::set_user_sp(addr_t sp)
{
    get_stack_top()[KSTACK_USP] = (u32_t)sp;
}

INLINE void tcb_t::set_user_flags(const word_t flags)
{
    get_stack_top()[KSTACK_UFLAGS] = (get_user_flags() & (~IA32_USER_FLAGMASK)) | (flags & IA32_USER_FLAGMASK);
}

INLINE time_t tcb_t::get_xfer_timeout_snd (void)
{
    return get_utcb()->xfer_timeout.get_snd();
}

INLINE time_t tcb_t::get_xfer_timeout_rcv (void)
{
    return get_utcb()->xfer_timeout.get_rcv();
}

INLINE void tcb_t::return_from_ipc (void)
{
    asm("movl %0, %%esp\n"
	"ret\n"
	:
	: "r"(&get_stack_top()[KSTACK_RET_IPC]),
	  "S"(get_tag().raw));
}

INLINE void tcb_t::return_from_user_interruption (void)
{
    asm("movl %0, %%esp\n"
	"ret\n"
	:
	: "r"(&get_stack_top()[- sizeof(ia32_exceptionframe_t)/4 - 2]));
}


/**********************************************************************
 *
 *                  copy-area related functions
 *
 **********************************************************************/

INLINE addr_t tcb_t::get_copy_area (tcb_t * dst, addr_t addr)
{
    return resources.enable_copy_area (this, dst, addr);
}

INLINE void tcb_t::release_copy_area (void)
{
    resources.release_copy_area (this, true);
}

INLINE addr_t tcb_t::copy_area_real_address (addr_t addr)
{
    ASSERT (space->is_copy_area (addr));

    word_t copyarea_num = 
	(((word_t) addr - COPY_AREA_START) >> IA32_PAGEDIR_BITS) /
	(COPY_AREA_SIZE >> IA32_PAGEDIR_BITS);

    return addr_offset (resources.copy_area_real_address (copyarea_num),
			(word_t) addr & (COPY_AREA_SIZE-1));
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

INLINE tcb_t * get_current_tcb()
{
    addr_t stack;
    asm ("mov %%esp, %0" :"=r" (stack));
    return addr_to_tcb (stack);
}

#ifdef CONFIG_SMP
INLINE cpuid_t get_current_cpu()
{
    return get_idle_tcb()->get_cpu();
}
#endif

/**
 * initial_switch_to: switch to first thread
 */
INLINE void __attribute__((noreturn)) initial_switch_to(tcb_t * tcb) 
{
    asm("movl %0, %%esp\n"
	"ret\n"
	:
	: "r"(tcb->stack));

    while (true)
	/* do nothing */;
}

#endif /* __GLUE__V4_IA32__TCB_H__ */
