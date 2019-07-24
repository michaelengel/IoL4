/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     api/v4/tcb.h
 * Description:   V4 TCB
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
 * $Id: tcb.h,v 1.51 2003/04/29 23:28:18 uhlig Exp $
 *                
 ********************************************************************/
#ifndef __API__V4__TCB_H__
#define __API__V4__TCB_H__

#include <debug.h>

#include INC_API(types.h)
#include INC_API(queuestate.h)
#include INC_API(queueing.h)
#include INC_API(threadstate.h)
#include INC_API(space.h)
#include INC_API(resources.h)
#include INC_API(thread.h)
#include INC_API(preempt.h)
#include INC_API(fpage.h)
#include INC_API(ipc.h)

/* implementation specific functions */
#include INC_GLUE(ktcb.h)
#include INC_GLUE(utcb.h)

typedef int prio_t;
class space_t;
class prio_queue_t;

template <class T> class ringlist_t
{
public:
    T * next;
    T * prev;
};


/**
 * tcb_t: kernel thread control block
 */
class tcb_t
{
public:
    enum unwind_reason_e {
	abort		= 1,
	timeout		= 2,
    };

    /* public functions */
    bool activate(void (*startup_func)(), threadid_t pager);
    
    void create_inactive(threadid_t dest, threadid_t scheduler);
    void create_kernel_thread(threadid_t dest, utcb_t * utcb);
    
    void delete_tcb();
    bool migrate_to_space(space_t * space);
    bool migrate_to_processor(cpuid_t processor);
    bool exists() 
	{ return space != NULL; }
    bool is_activated()
	{ return utcb != NULL; }

    void unwind (unwind_reason_e reason);
    
    /* queue manipulations */
    void enqueue_send(tcb_t * tcb);
    void dequeue_send(tcb_t * tcb);
    void enqueue_present();
    void dequeue_present();

    /* thread id access functions */
    void set_global_id(threadid_t tid);
    threadid_t get_global_id();

    threadid_t get_local_id();
    void set_utcb_location (word_t utcb_location);
    word_t get_utcb_location();

    /* thread state */
    void set_state(thread_state_t state);
    thread_state_t get_state();
    arch_ktcb_t * get_arch();
    void save_state (void);
    void restore_state (void);

    /* ipc */
    void set_ipc_error(word_t err);
    word_t get_ipc_error();
    void set_partner(threadid_t tid);
    threadid_t get_partner();
    tcb_t * get_partner_tcb();

    time_t get_xfer_timeout_snd ();
    time_t get_xfer_timeout_rcv ();
    void set_actual_sender(threadid_t tid);
    threadid_t get_virtual_sender();
    threadid_t get_intended_receiver();

    msg_tag_t get_tag();
    void set_tag(msg_tag_t tag);
    word_t get_mr(word_t index);
    void set_mr(word_t index, word_t value);
    void copy_mrs(tcb_t * dest, word_t start, word_t count);
    word_t get_br(word_t index);
    void set_br(word_t index, word_t value);

    msg_tag_t do_ipc(threadid_t to_tid, threadid_t from_tid, timeout_t timeout);
    void send_pagefault_ipc(addr_t addr, addr_t ip, space_t::access_e access);
    bool send_preemption_ipc(u64_t abstime);
    void return_from_ipc (void);
    void return_from_user_interruption (void);

    /* synchronization */
    void lock() { tcb_lock.lock(); }
    void unlock() { tcb_lock.unlock(); }

    /* thread notification */
    void notify(void (*func)());
    void notify(void (*func)(word_t), word_t arg1);
    void notify(void (*func)(word_t, word_t), word_t arg1, word_t arg2);
    void notify(void (*func)(tcb_t*, word_t), tcb_t * tcb, word_t arg)
	{ notify((void(*)(word_t, word_t))func, (word_t)tcb, arg); }

    /* thread manipulation */
    addr_t get_user_ip();
    addr_t get_user_sp();
    void set_user_ip(addr_t ip);
    void set_user_sp(addr_t sp);

    /* thread switching */
    void switch_to(tcb_t * current);
    void switch_to_idle();

    /* space */
    space_t * get_space() { return space; }
    void set_space(space_t * space);

    /* Copy area */
    addr_t get_copy_area (tcb_t * dst, addr_t addr);
    void release_copy_area (void);
    addr_t copy_area_real_address (addr_t addr);

    /* processor */
    cpuid_t get_cpu() 
#if defined(CONFIG_SMP)
	{ return this->cpu; }
#else
	{ return 0; }
#endif
    void set_cpu(cpuid_t cpu);
    bool is_local_cpu();

    /* utcb access functions */
    utcb_t * get_utcb() const { return this->utcb; }

public:
    void set_scheduler(const threadid_t tid);
    void set_pager(const threadid_t tid);
    void set_exception_handler(const threadid_t tid);
    void set_user_handle(const word_t handle);
    void set_user_flags(const word_t flags);

    threadid_t get_pager();
    threadid_t get_scheduler();
    threadid_t get_exception_handler();
    word_t get_user_handle();
    word_t get_user_flags();
    preempt_flags_t get_preempt_flags();
    void set_preempt_flags(preempt_flags_t flags);
    u8_t get_cop_flags();
    word_t * get_reg_stack_bottom (void);

    /* interrupt management */
    void set_irq_handler(const threadid_t tid);
    threadid_t get_irq_handler();
    bool is_interrupt_thread();

public:
    void allocate();

private:
    void create_tcb(const threadid_t dest);
    void init(threadid_t dest);
    
    /* stack manipulation */
public:
    word_t * get_stack_top();
    void init_stack();
private:
    void create_startup_stack(void (*func)());
    

    friend void make_offsets(); /* generates OFS_TCB_ stuff */

public:

    typedef union {
	struct {
	    /* IPC copy */
	    word_t		saved_mr[3];
	    word_t		saved_br0;

	    word_t		copy_length;
	    addr_t		copy_start_src;
	    addr_t		copy_start_dst;
	    addr_t		copy_fault;
	} ipc_copy;

	struct {
	    /* Exchange registers */
	    word_t		control;
	    word_t		sp;
	    word_t		ip;
	    word_t		flags;
	    threadid_t		pager;
	    word_t		user_handle;
	} exregs;
    } misc_tcb_t;

    threadid_t get_saved_partner (void) { return saved_partner; }
    void set_saved_partner (threadid_t t) { saved_partner = t; }

    thread_state_t get_saved_state (void)
	{ return saved_state; }
    void set_saved_state (thread_state_t s)
	{ saved_state = s; }
    void set_saved_state (thread_state_t::thread_state_e s)
	{ saved_state = (thread_state_t) s; }

    /* do not delete this TCB_START_MARKER */

    // have relatively static values here
    threadid_t		myself_global;
    threadid_t		myself_local;

private:
    cpuid_t		cpu;
    utcb_t *		utcb;
    
    thread_state_t 	thread_state;
    threadid_t		partner;

    word_t		resource_bits;
public:
    word_t *		stack;
private:
    /* VU: pdir_cache should be architecture-specific!!! */
    word_t		pdir_cache;

public:
    queue_state_t	queue_state;

    /* queues */
    ringlist_t<tcb_t>	present_list;
    ringlist_t<tcb_t>	ready_list;
    ringlist_t<tcb_t>	wait_list;
    ringlist_t<tcb_t>	send_list;
    tcb_t *		send_head;

    spinlock_t		tcb_lock;

#ifdef CONFIG_SMP
    ringlist_t<tcb_t>	xcpu_list;
    cpuid_t		xcpu;
    word_t		xcpu_status;
#endif

public:
    /* scheduling */
    u64_t		total_quantum;
    u64_t		timeslice_length;
    s64_t		current_timeslice;
    u64_t		absolute_timeout;
    prio_t		priority;

    /* delayed preemption */
    prio_t		sensitive_prio;
    u16_t		current_max_delay;
    u16_t		max_delay;

private:
    /* pager etc */
    threadid_t		scheduler;
    space_t *		space;

public:
    arch_ktcb_t		arch;

public:
    misc_tcb_t		misc;
    threadid_t		saved_partner;
    thread_state_t	saved_state;
    bool		has_xfer_timeout;
    thread_resources_t	resources;

private:
    word_t		kernel_stack[0];
    /* do not delete this TCB_END_MARKER */

    /* class friends */
    friend void dump_tcb(tcb_t *);
    friend class thread_resources_t;
};

/* union to allow allocation of tcb including stack */
typedef union {
    u8_t pad[KTCB_SIZE];
} whole_tcb_t __attribute__((aligned(KTCB_SIZE)));
    

/**********************************************************************
 *
 *               Operations on thread ids and settings
 *
 **********************************************************************/

INLINE threadid_t tcb_t::get_global_id()
{
    return myself_global;
}

INLINE threadid_t tcb_t::get_local_id()
{
    return myself_local;
}

INLINE void tcb_t::set_scheduler(const threadid_t tid)
{
    this->scheduler = tid;
}

INLINE threadid_t tcb_t::get_scheduler()
{
    return this->scheduler;
}

INLINE void tcb_t::set_irq_handler(const threadid_t tid)
{
    set_scheduler(tid);
}

INLINE threadid_t tcb_t::get_irq_handler()
{
    return get_scheduler();
}


/**********************************************************************
 *
 *                  Access functions
 *
 **********************************************************************/
INLINE void tcb_t::set_state(thread_state_t state)
{
    this->thread_state = state;
}

INLINE thread_state_t tcb_t::get_state()
{
    return this->thread_state;
}

INLINE void tcb_t::set_partner(threadid_t tid)
{
    this->partner = tid;
}

INLINE threadid_t tcb_t::get_partner()
{
    return this->partner;
}

INLINE tcb_t* tcb_t::get_partner_tcb()
{
    return this->get_space()->get_tcb(partner);
}

/**
 * enqueues the tcb into the send queue of tcb
 * @param tcb the thread control block to enqueue
 */
INLINE void tcb_t::enqueue_send(tcb_t * tcb)
{
    ASSERT( !queue_state.is_set(queue_state_t::send) );

    ENQUEUE_LIST_TAIL(tcb->send_head, this, send_list);
    queue_state.set(queue_state_t::send);
}

/**
 * dequeues the tcb from the send queue of tcb
 * @param tcb the thread control block to dequeue from
 */
INLINE void tcb_t::dequeue_send(tcb_t * tcb)
{
    ASSERT( queue_state.is_set(queue_state_t::send) );
    DEQUEUE_LIST(tcb->send_head, this, send_list);
    queue_state.clear(queue_state_t::send);
}

/**
 * enqueue a tcb into the present list
 * the present list primarily exists for debugging reasons, since task 
 * manipulations now happen on a per-thread basis */
#ifdef CONFIG_DEBUG
extern tcb_t * global_present_list;
extern spinlock_t present_list_lock;
#endif

INLINE void tcb_t::enqueue_present()
{
#ifdef CONFIG_DEBUG
    present_list_lock.lock();
    ENQUEUE_LIST_TAIL(global_present_list, this, present_list);
    present_list_lock.unlock();
#endif
}

INLINE void tcb_t::dequeue_present()
{
#ifdef CONFIG_DEBUG
    present_list_lock.lock();
    DEQUEUE_LIST(global_present_list, this, present_list);
    present_list_lock.unlock();
#endif
}

INLINE arch_ktcb_t *tcb_t::get_arch()
{
    return &this->arch;
}

INLINE tcb_t * get_idle_tcb()
{
    extern whole_tcb_t __idle_tcb;
    return (tcb_t*)&__idle_tcb;
}

INLINE void tcb_t::switch_to_idle()
{
    switch_to(get_idle_tcb());
}


/* 
 * include glue header file 
 */
#include INC_GLUE(tcb.h)


/**********************************************************************
 *
 *             global V4 thread management
 *
 **********************************************************************/

INLINE space_t * get_current_space()
{
    return get_current_tcb()->get_space();
}

#ifndef CONFIG_SMP
INLINE cpuid_t get_current_cpu()
{
    return 0;
}

INLINE bool tcb_t::migrate_to_processor(cpuid_t processor)
{
    return processor == 0;
}
#endif

INLINE bool tcb_t::is_local_cpu()
{
    // on non-SMP always evaluates to true
    return (get_current_cpu() == get_cpu());
}

void handle_ipc_timeout (word_t state);


/**
 * creates a root server thread and a fresh space, if the 
 * creation fails the function does not return (assuming that root 
 * servers are functional necessary for the system)
 * 
 * @param dest_tid id of the thread to be created
 * @param scheduler_tid thread id of the scheduler
 * @param pager_tid thread id of the pager
 * @param utcb_area fpage describing the UTCB area
 * @param kip_area fpage describing the kernel interface page area
 * @return the newly created tcb
 */
tcb_t * create_root_server(threadid_t dest_tid, threadid_t scheduler_tid, 
			   threadid_t pager_tid, fpage_t utcb_area, fpage_t kip_area);

/**
 * initializes the root servers
 */
void init_root_servers();

/**
 * initializes the kernel threads
 */
void init_kernel_threads();


#endif /* !__API__V4__TCB_H__ */

