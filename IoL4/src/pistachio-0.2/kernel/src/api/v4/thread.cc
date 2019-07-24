/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     api/v4/thread.cc
 * Description:   thread manipulation
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
 * $Id: thread.cc,v 1.65 2003/04/24 13:36:48 skoglund Exp $
 *                
 ********************************************************************/

#include INC_API(config.h)
#include INC_API(tcb.h)
#include INC_API(thread.h)
#include INC_API(interrupt.h)
#include INC_API(schedule.h)
#include INC_API(space.h)
#include INC_API(kernelinterface.h)
#include INC_GLUE(syscalls.h)
#include INC_API(syscalls.h)
#include INC_API(smp.h)

#include <kdb/tracepoints.h>

DECLARE_TRACEPOINT(SYSCALL_THREAD_CONTROL);

//#define TRACE_XCPU(x...) printf(x)
#define TRACE_XCPU(x...)

void handle_ipc_error (void);

bool tcb_t::is_interrupt_thread ()
{ 
    return (get_global_id().is_interrupt() && 
	    get_global_id().get_threadno() < get_kip()->thread_info.get_system_base()); 
}

/**
 * Stub invoked after a startup message has been received from the
 * thread's pager.
 */
static void thread_startup()
{
    tcb_t * current = get_current_tcb();

#if 0
    printf ("Startup %p: ip=%p  sp=%p\n", current,
	    current->get_mr (1), current->get_mr (2));
#endif

    // Poke received IP/SP into exception frame (or whatever is used
    // by the architecture).  No need to check for valid IP/SP.
    // Thread will simply fault if values are not valid.
    current->set_user_ip((addr_t)current->get_mr(1));
    current->set_user_sp((addr_t)current->get_mr(2));
    current->set_saved_state (thread_state_t::aborted);
}


/**
 * Fake that thread is waiting for IPC startup message.
 *
 * @param tcb		thread to wait for startup message
 * @param pager		thread id to receive startup message from
 */
static void fake_wait_for_startup (tcb_t * tcb, threadid_t pager)
{
    // Fake that we are waiting to receive untyped words from our
    // pager.
    tcb->set_state (thread_state_t::waiting_forever);
    tcb->set_partner (pager);
    tcb->set_br (0, 0);

    // Make sure that unwind will work on waiting thread.
    tcb->set_saved_partner (threadid_t::nilthread ());

#warning VU: revise fake_wait_for_startup
    // Make sure that IPC abort will restore user-level exception
    // frame instead of trying to return from IPC syscall.
    tcb->set_saved_state (thread_state_t::running);
}


void thread_return()
{
    /* normal return function - do nothing */
}

void tcb_t::init(threadid_t dest)
{
    ASSERT(this);

    allocate();

    /* clear utcb and space */
    utcb = NULL;
    space = NULL;

#warning fix locking for thread creation
    tcb_lock.init();

    /* make sure, nobody messes around with the thread */
    set_state(thread_state_t::aborted);
    set_saved_state (thread_state);
    partner = threadid_t::nilthread();
    set_saved_partner (threadid_t::nilthread());
    
    /* set the global id */
    myself_global = dest;
    myself_local = NILTHREAD;

    /* initialize thread resources */
    resources.init(this);

    /* queue initialization */
    queue_state.init();
    send_head = NULL;

#ifdef CONFIG_SMP
#warning VU: uninitialized threads assigned to CPU
    /* initially assign to this CPU */
    cpu = get_current_cpu();
#endif

    /* initialize scheduling */
    get_current_scheduler()->init_tcb(this);

    /* enqueue into present list, do not enqueue 
     * the idle thread since they are CPU local and break 
     * the pointer list */
    if (this != get_idle_tcb())
	enqueue_present();
    
    init_stack();
}


void tcb_t::create_kernel_thread(threadid_t dest, utcb_t * utcb)
{
    //TRACEF("dest=%T\n", TID(dest));
    init(dest);
    this->utcb = utcb;
    // kernel threads have prio 0 by default
    get_current_scheduler()->set_priority(this, 0);
}

void tcb_t::create_inactive(threadid_t dest, threadid_t scheduler)
{
    //TRACEF("tcb=%p, %T\n", this, TID(dest));
    init(dest);
    this->scheduler = scheduler;
}

bool tcb_t::activate(void (*startup_func)(), threadid_t pager)
{
    ASSERT(this);
    ASSERT(this->space);
    ASSERT(!this->is_activated());
    
    // check that the UTCB location is valid for the space
    if ( !get_space()->get_utcb_page_area().is_range_in_fpage (
	    (addr_t)get_utcb_location(), (addr_t)(get_utcb_location() + sizeof(utcb_t))) )
	return false;

    //TRACEF("%T %x (%x)\n", TID(get_local_id()), get_utcb_location(), ((1 << get_kip()->utcb_info.alignment) - 1));

    // check that the utcb location conforms to utcb info
    if (!get_kip()->utcb_info.is_valid_utcb_location(get_utcb_location()))
	return false;

    /* allocate UTCB */
    this->utcb = get_space()->allocate_utcb(this);
    if (!this->utcb)
	return false;

    /* update global id in UTCB */
    set_global_id (this->get_global_id());

    /* initialize scheduler, pager and exception handler */
    set_pager(pager);
    set_cpu(get_current_cpu());
    set_scheduler(this->scheduler);
    set_exception_handler(NILTHREAD);

    /* initialize the startup stack */
    create_startup_stack(startup_func);
    return true;
}

#ifdef CONFIG_SMP
static void do_xcpu_delete_done(cpu_mb_entry_t * entry)
{
    tcb_t * tcb = entry->tcb;
    ASSERT(tcb);

    // still on same CPU? --> otherwise forward
    if (!tcb->is_local_cpu())
	UNIMPLEMENTED();

    if (tcb->get_state() != thread_state_t::xcpu_waiting_deltcb)
	UNIMPLEMENTED();

    tcb->xcpu_status = entry->param[0];
    tcb->set_state(thread_state_t::running);
    get_current_scheduler()->enqueue_ready(tcb);
}

static void do_xcpu_delete(cpu_mb_entry_t * entry)
{
    tcb_t * tcb = entry->tcb;
    tcb_t * src = (tcb_t*)entry->param[0];
    word_t done = 1;

    //TRACEF("xcpu_delete of %p requested by %p\n", tcb, src);

    // migrated meanwhile?
    if (tcb->is_local_cpu())
    {
	if (get_current_tcb() == tcb)
	    UNIMPLEMENTED();

	tcb->delete_tcb();
	done = 0;
    }
    xcpu_request(src->get_cpu(), do_xcpu_delete_done, src, done);
}
#endif

void tcb_t::delete_tcb()
{
    ASSERT(this->exists());

#ifdef CONFIG_SMP
 delete_tcb_retry:
    if ( !this->is_local_cpu() )
    {
	xcpu_request(this->get_cpu(), do_xcpu_delete, this, (word_t)get_current_tcb());
	get_current_tcb()->set_state(thread_state_t::xcpu_waiting_deltcb);
	get_current_tcb()->switch_to_idle();

	// wait for re-activation, if not successfull simply retry
	if (get_current_tcb()->xcpu_status)
	    goto delete_tcb_retry;
	return;
    }
#endif

    if ( is_activated() )
    {
	ASSERT(get_utcb());

	// dequeue from ready queue
	get_current_scheduler()->dequeue_ready(this);
	
	// unwind ongoing IPCs
	if (get_state().is_sending() || get_state().is_receiving())
	    unwind (tcb_t::abort);
	
	lock();
	
	// dequeue pending send requests
	while(send_head)
	{
	    send_head->dequeue_send(this);
	    // what do we do with these guys?
	}
	    
	unlock();
	    
	// free any used resources
	resources.free(this);
	
	// free UTCB
	this->utcb = NULL;
    }

    // clear ids
    this->myself_global = NILTHREAD;
    this->myself_local = NILTHREAD;

    this->set_space(NULL);
    this->set_state(thread_state_t::aborted);
    dequeue_present();
}


/**
 * Caclulate the sender and receiver errorcodes when an IPC operation
 * has been aborted either by exchange_registers or by a timeout.
 *
 * @param reason		reason for abort (abort or timeout)
 * @param snd			sender thread
 * @param rcv			receiver thread
 * @param err_s			returned sender error code
 * @param err_r			returned receiver error code
 */
static void calculate_errorcodes (tcb_t::unwind_reason_e reason,
				  tcb_t * snd, tcb_t * rcv,
				  word_t * err_s, word_t * err_r)
{
    word_t offset = snd->misc.ipc_copy.copy_length;

    if (snd->get_space ()->is_copy_area (snd->misc.ipc_copy.copy_fault))
	offset += (word_t) snd->misc.ipc_copy.copy_fault -
	    (word_t) snd->misc.ipc_copy.copy_start_dst;
    else
	offset += (word_t) snd->misc.ipc_copy.copy_fault -
	    (word_t) snd->misc.ipc_copy.copy_start_src;

    if (reason == tcb_t::timeout)
    {
	time_t snd_to = snd->get_xfer_timeout_snd ();
	time_t rcv_to = rcv->get_xfer_timeout_rcv ();

	if (rcv_to < snd_to)
	{
	    *err_s = IPC_SND_ERROR (ERR_IPC_XFER_TIMEOUT_PARTNER (offset));
	    *err_r = IPC_RCV_ERROR (ERR_IPC_XFER_TIMEOUT_CURRENT (offset));
	}
	else
	{
	    *err_s = IPC_SND_ERROR (ERR_IPC_XFER_TIMEOUT_CURRENT (offset));
	    *err_r = IPC_RCV_ERROR (ERR_IPC_XFER_TIMEOUT_PARTNER (offset));
	}
    }
    else
    {
	*err_s = IPC_SND_ERROR (ERR_IPC_ABORTED (offset));
	*err_r = IPC_RCV_ERROR (ERR_IPC_ABORTED (offset));
    }
}


#if defined(CONFIG_SMP)

DECLARE_TRACEPOINT (XCPU_UNWIND);

/**
 * Handler invoked when our IPC partner has aborted/timed out the IPC.
 * Arguments are as follows:
 *
 * param[0]	thread id of partner
 * param[1]	expected thread state
 * param[2]	message tag to return
 * param[3]	error code to return
 */
static void do_xcpu_unwind_partner (cpu_mb_entry_t * entry)
{
    tcb_t * tcb = entry->tcb;
    threadid_t partner_id = threadid (entry->param[0]);
    thread_state_t expected_state (entry->param[1]);
    msg_tag_t tag = msgtag (entry->param[2]);
    word_t err = entry->param[3];

    TRACEPOINT (XCPU_UNWIND,
		printf ("tcb=%p, partner=%p, state=%s, tag=%p, err=%p\n",
			TID (tcb->get_global_id ()), TID (partner_id),
			expected_state.string (), tag.raw, err));

    if (EXPECT_FALSE (! tcb->is_local_cpu ()))
    {
	// Forward request.
	xcpu_request (tcb->get_cpu (), do_xcpu_unwind_partner, tcb,
		      entry->param[0], entry->param[1],
		      entry->param[2], entry->param[3]);
	return;
    }

    if (EXPECT_FALSE (tcb->get_state () != expected_state) ||
	EXPECT_FALSE (tcb->get_partner () != partner_id))
    {
	// Request is outdated.
	return;
    }

    if (! tcb->get_saved_partner ().is_nilthread ())
    {
	// We have a nested IPC operation.  Perform another unwind.
	tcb->restore_state ();
	word_t e = (err >> 1) & 0x7;
	tcb->unwind ((e == 1 || e == 5 || e == 6) ?
		     tcb_t::timeout : tcb_t::abort);
    }
    else
    {
	tcb->set_ipc_error (err);
	tcb->set_tag (tag);
	tcb->set_state (thread_state_t::running);
    }

    // Reactivate thread.
    tcb->notify (handle_ipc_error);
    get_current_scheduler ()->enqueue_ready (tcb);
}

#endif /* CONFIG_SMP */


DECLARE_TRACEPOINT (UNWIND);

/**
 * Unwinds a thread from an ongoing IPC.
 *
 * @param reason		reason for unwind (abort or timeout)
 */
void tcb_t::unwind (unwind_reason_e reason)
{
    msg_tag_t tag = get_tag ();
    thread_state_t cstate;
    tcb_t * partner;

    TRACEPOINT (UNWIND,
		printf ("Unwind: tcb=%p p=%p s=%s (saved: p=%p s=%s)\n",
			TID (get_global_id ()), TID (get_partner ()),
			get_state ().string (),	TID (get_saved_partner ()),
			get_saved_state ().string ()));

redo_unwind:

    cstate = get_state ();
    set_state (thread_state_t::running);
    partner = get_partner_tcb ();

    if (cstate.is_polling () || cstate.is_waiting ())
    {
	// IPC operation has not yet started.  I.e., partner is not
	// yet involved.

	get_current_scheduler ()->cancel_timeout (this);

	if (cstate.is_polling ())
	{
	    // The thread is enqueued in the send queue of the partner.
	    partner->lock ();
	    dequeue_send (partner);
	    partner->unlock ();
	}
	else
	{
	    // Thread is not yet receiving.  Cancel receive phase.
	    tag = tag.error_tag ();
	}

	if (! get_saved_partner ().is_nilthread ())
	{
	    // We're handling a nested IPC.
	    restore_state ();
	    goto redo_unwind;
	}

	// Set appropriate error code
	tag.set_error ();
	set_tag (tag);
	word_t err = (reason == timeout) ? ERR_IPC_TIMEOUT : ERR_IPC_CANCELED;
	set_ipc_error ((cstate.is_polling ()) ?
		       IPC_SND_ERROR (err) : IPC_RCV_ERROR (err));
	return;
    }

    else if (cstate == thread_state_t::waiting_tunneled_pf)
    {
	// We have tunneled a pagefault to our partner.  Inform the
	// partner that we're breaking off the IPC operation.

	// Get appropriate error codes
	word_t err_s, err_r;
	calculate_errorcodes (reason, this, partner, &err_s, &err_r);
	tag.set_error ();

#if defined(CONFIG_SMP)
	if (! partner->is_local_cpu ())
	{
	    xcpu_request (partner->get_cpu (), do_xcpu_unwind_partner, partner,
			  get_global_id ().get_raw (),
			  (word_t) thread_state_t::locked_waiting,
			  tag.raw, err_r);
	}
	else
#endif
	{
	    // Reactivate partner directly
	    partner->set_ipc_error (err_r);
	    partner->set_tag (tag);
	    partner->set_state (thread_state_t::running);
	    partner->notify (handle_ipc_error);
	    get_current_scheduler ()->enqueue_ready (partner);
	}

	set_tag (tag);
	set_ipc_error (err_s);
	return;
    }

    else if (cstate == thread_state_t::locked_running_ipc_done)
    {
	// Nested pagefault (almost) completed.  No partner.
	restore_state ();
	goto redo_unwind;
    }

    else if (cstate == thread_state_t::locked_running)
    {
	// Thread is an active sender.  Abort both threads.

	word_t err_s, err_r;
	calculate_errorcodes (reason, this, partner, &err_s, &err_r);
	tag.set_error ();

#if defined(CONFIG_SMP)
	if (! partner->is_local_cpu ())
	{
	    xcpu_request (partner->get_cpu (), do_xcpu_unwind_partner,
			  partner, get_global_id ().get_raw (),
			  (word_t) thread_state_t::locked_waiting,
			  tag.raw, err_r);
	}
	else
#endif
	{
	    // The receiver should be locked waiting.  It might also
	    // happen that we just completed a pagefault RPC without
	    // being scheduled yet.  In this case, our partner will
	    // not be communicating with us (and we should not abort
	    // any of his ongoing IPC operations).
	    if (partner->get_state () == thread_state_t::locked_waiting &&
		partner->get_partner () == get_global_id ())
	    {
		partner->set_ipc_error (err_r);
		partner->set_tag (tag);
		partner->set_state (thread_state_t::running);
		partner->notify (handle_ipc_error);
		get_current_scheduler ()->enqueue_ready (partner);
	    }
	}

	if (! get_saved_partner ().is_nilthread ())
	{
	    restore_state ();
	    goto redo_unwind;
	}

	set_tag (tag);
	set_ipc_error (err_s);
	return;
    }

    else if (cstate == thread_state_t::locked_running_nested)
    {
	// Thread is handling tunneled pagefault, but IPC operation
	// has not yet started.  Abort both current thread and waiting
	// sender.

	word_t err_s, err_r;
	calculate_errorcodes (reason, partner, this, &err_s, &err_r);
	tag.set_error ();

#if defined(CONFIG_SMP)
	if (! partner->is_local_cpu ())
	{
	    xcpu_request (partner->get_cpu (), do_xcpu_unwind_partner,
			  partner, get_global_id ().get_raw (),
			  (word_t) thread_state_t::waiting_tunneled_pf,
			  tag.raw, err_s);
	}
	else
#endif
	{
	    // The sender should be waiting for tunneled page fault
	    if (partner->get_state() == thread_state_t::waiting_tunneled_pf)
	    {
		partner->set_ipc_error (err_s);
		partner->set_tag (tag);
		partner->set_state (thread_state_t::running);
		partner->notify (handle_ipc_error);
		get_current_scheduler()->enqueue_ready(partner);
	    }
	}

	set_tag (tag);
	set_ipc_error (err_r);
	return;
    }

    else if (cstate == thread_state_t::locked_waiting)
    {
	// Thread is an active receiver.  Abort both threads.

	word_t err_s, err_r;
	calculate_errorcodes (reason, partner, this, &err_s, &err_r);
	tag.set_error ();

#if defined(CONFIG_SMP)
	if (! partner->is_local_cpu ())
	{
	    xcpu_request (partner->get_cpu (), do_xcpu_unwind_partner,
			  partner, get_global_id ().get_raw (),
			  (word_t) thread_state_t::locked_running,
			  tag.raw, err_s);
	}
	else
#endif
	{
	    // The sender should be locked running.
	    if (partner->get_state() == thread_state_t::locked_running)
	    {
		partner->set_ipc_error (err_s);
		partner->set_tag (tag);
		partner->set_state (thread_state_t::running);
		partner->notify (handle_ipc_error);
		get_current_scheduler()->enqueue_ready(partner);
	    }
	}

	if (! get_saved_partner ().is_nilthread ())
	{
	    restore_state ();
	    goto redo_unwind;
	}

	set_tag (tag);
	set_ipc_error (err_r);
	return;
    }

    else if (cstate.is_xcpu_waiting ())
    {
	// Waiting for xcpu TCB deletion or exregs completion.  Just
	// ignore.
	return;
    }

    WARNING ("Unresolved unwind: tcb=%p p=%p s=%s (saved: p=%p s=%s)\n",
	     TID (get_global_id ()),
	     TID (get_partner ()), cstate.string (),
	     TID (get_saved_partner ()), get_saved_state ().string ());
    return;
}


bool tcb_t::migrate_to_space(space_t * space)
{
    ASSERT(space);
    ASSERT(get_space());

    // allocate utcb in destination space
    utcb_t * new_utcb = space->allocate_utcb(this);
    if (!new_utcb)
	return false;

    // make sure nobody messes around with the tcb
    unwind (tcb_t::abort);

    // remove from old space
    if (get_space()->remove_tcb(this))
    {
	space->free();
	free_space(get_space());
    }

    // now change the space and set new UTCB ptr
    this->set_space(space);
    this->utcb = new_utcb;

    return true;
}

#ifdef CONFIG_SMP

/**
 * releases all resources held by the thread
 */
static void xcpu_release_thread(tcb_t * tcb)
{
    // thread is on the current CPU
    get_current_scheduler()->dequeue_ready(tcb);

    // remove from wakeup queue, we use the absolute timeout
    // as identifier whether a thread was in the wakeup list or not
    if (!tcb->queue_state.is_set(queue_state_t::wakeup))
	tcb->absolute_timeout = 0;
    else
	get_current_scheduler()->cancel_timeout(tcb);

    tcb->resources.purge(tcb);

    // when migrating IRQ threads disable interrupts
    if (tcb->is_interrupt_thread())
	migrate_interrupt_start (tcb);
}

/**
 * re-integrates a thread into the CPUs queues etc.
 */
static void xcpu_integrate_thread(tcb_t * tcb)
{
    ASSERT(!tcb->queue_state.is_set(queue_state_t::wakeup));

    // interrupt threads are handled specially
    if (tcb->is_interrupt_thread())
    {
	migrate_interrupt_end(tcb);
	return;
    }

    /* VU: the thread may have received an IPC meanwhile hence we
     * check whether the thread is already running again.  to make it
     * fully working the waiting timeout must be set more carefull! */
    if (tcb->get_state().is_runnable())
	get_current_scheduler()->enqueue_ready(tcb);
    else if (tcb->absolute_timeout &&
	     tcb->get_state().is_waiting_with_timeout())
	get_current_scheduler()->set_timeout(tcb, tcb->absolute_timeout);
}

/**
 * puts the thread onto the CPU the function is invoked on
 */
static void do_xcpu_put_thread(cpu_mb_entry_t * entry)
{
    tcb_t * tcb = entry->tcb;
    TRACE_XCPU("CPU%d (%p): %s %p\n", get_current_cpu(), 
	       get_current_tcb(), __func__, tcb);

    xcpu_integrate_thread(tcb);
}

static void xcpu_put_thread(tcb_t * tcb, word_t processor)
{
    TRACE_XCPU("%p migrating thread %p\n", get_current_tcb(), tcb);

    /* VU: it is only necessary to notify the other CPU if the thread
     * is in one of the scheduling queues (wakeup, ready) or is an
     * interrupt thread */
    bool need_xcpu = tcb->get_state().is_runnable() || 
	(tcb->absolute_timeout != 0) || tcb->is_interrupt_thread();

    tcb->set_cpu(processor);
    
    if (need_xcpu)
	xcpu_request(processor, do_xcpu_put_thread, tcb);
}

/**
 * remote handler for tcb_t::migrate_to_processor
 */
static void do_xcpu_set_thread(cpu_mb_entry_t * entry)
{
    if (!entry->tcb)
    {
	enter_kdebug("do_xcpu_set_thread");
	return;
    }

    entry->tcb->migrate_to_processor(entry->param[0]);
}

bool tcb_t::migrate_to_processor(cpuid_t processor)
{
    ASSERT(this);
    
    // check if the thread is already on that processor
    if (processor == this->get_cpu())
	return true;

    TRACE_XCPU("CPU%d (%p): %s %p\n", get_current_cpu(), 
	       get_current_tcb(), __func__, this);

    if (EXPECT_FALSE( this->get_cpu() != get_current_cpu() ))
    {
	// thread not on current CPU (or migrated away meanwhile)
	xcpu_request(this->get_cpu(), do_xcpu_set_thread, this, processor);
    }
    else if ( this->get_cpu() != processor )
    {
	// thread is on local CPU and should be migrated
	xcpu_release_thread(this);

	// if we migrate ourself we use the idle thread to perform the notification
	if (get_current_tcb() == this)
	{
	    get_idle_tcb()->notify(xcpu_put_thread, this, processor);
	    this->switch_to_idle();
	}
	else
	    xcpu_put_thread(this, processor);
    }

    return true;
}
#endif /* CONFIG_SMP */

/**
 * Handler invoked when IPC errors (aborts or timeouts) occur.  Any
 * IPC resources are released and control is transferred to
 * user-level.  If the aborted IPC happens to be a pagefault IPC
 * (caused by user-level memory access) we need to restore the thread
 * prior to the pagefault and return to user-level.
 */
void handle_ipc_error (void)
{
    tcb_t * current = get_current_tcb ();

    current->release_copy_area ();
    current->has_xfer_timeout = false;

    //TRACEF("saved state == %x\n", (word_t)current->get_saved_state ());

    if (current->get_saved_state ().is_running ())
    {
	// Thread was doing a pagefault IPC.  Restore thread state
	// prior to IPC operation and return directly to user-level.
	for (int i = 0; i < 3; i++)
	    current->set_mr (i, current->misc.ipc_copy.saved_mr[i]);
	current->set_br (0, current->misc.ipc_copy.saved_br0);
	current->set_partner (current->get_saved_partner ());
	current->set_state (current->get_saved_state ());

	current->set_saved_partner (threadid_t::nilthread ());
	current->set_saved_state (thread_state_t::aborted);

	current->return_from_user_interruption ();
    }
    else
    {
	current->set_saved_state (thread_state_t::aborted); // sanity
	current->return_from_ipc ();
    }

    /* NOTREACHED */
}


/**
 * Handler invoked when an IPC timeout has occured.  The handler
 * aborts any ongoing IPC operations (including partners) and returns
 * to user-level with an IPC error code.
 *
 * @param state		thread state when timeout occured
 */
void handle_ipc_timeout (word_t state)
{
    tcb_t * current = get_current_tcb ();
    //TRACEF("%p (%x, %x)\n", current, state, (word_t)current->saved_state ());

    // Restore thread state when timeout occured
    current->set_state ((thread_state_t) state);

    current->unwind (tcb_t::timeout);
    current->set_state (thread_state_t::running);
    handle_ipc_error ();
}


void tcb_t::save_state (void)
{
    for (int i = 0; i < 3; i++)
	misc.ipc_copy.saved_mr[i] = get_mr (i);
    misc.ipc_copy.saved_br0 = get_br (0);
    ASSERT (get_saved_partner () == threadid_t::nilthread ());
    ASSERT (get_saved_state () == thread_state_t::aborted);
    set_saved_partner (get_partner ());
    set_saved_state (get_state ());
}

void tcb_t::restore_state (void)
{
    for (int i = 0; i < 3; i++)
	set_mr (i, misc.ipc_copy.saved_mr[i]);
    set_br (0, misc.ipc_copy.saved_br0);
    set_partner (get_saved_partner ());
    set_state (get_saved_state ());

    set_saved_partner (threadid_t::nilthread());
    set_saved_state (thread_state_t::aborted);
}

void tcb_t::send_pagefault_ipc (addr_t addr, addr_t ip,
				space_t::access_e access)
{
    save_state ();

    /* generate pagefault message */
    msg_tag_t tag;
    tag.set(0, 2, IPC_MR0_PAGEFAULT | 
	    ((access == space_t::read)      ? (1 << 2) : 0) |
	    ((access == space_t::write)     ? (1 << 1) : 0) |
	    ((access == space_t::execute)   ? (1 << 0) : 0) |
	    ((access == space_t::readwrite) ? (1 << 2)+(1 << 1) : 0));

    /* create acceptor for whole address space */
    acceptor_t acceptor;
    acceptor.clear();
    acceptor.set_rcv_window(fpage_t::complete());
    
    set_tag(tag);
    set_mr(1, (word_t)addr);
    set_mr(2, (word_t)ip);
    set_br(0, acceptor.raw);

    //TRACEF("send pagefault IPC (%T)\n", TID(get_pager()));
    tag = do_ipc(get_pager(), get_pager(), timeout_t::never());
    if (tag.is_error())
    {
	printf("result tag = %p, ip = %p, addr = %p\n", tag.raw, ip, addr);
	enter_kdebug("pagefault IPC error");
    }

    restore_state ();
}

bool tcb_t::send_preemption_ipc(u64_t time)
{
    save_state ();

    /* generate preemption message */
    set_tag (msg_tag_t::preemption_tag());
    set_mr (1, (word_t)time);
    set_mr (2, (word_t)((time >> (BITS_WORD-1)) >> 1)); // Avoid gcc warn
    set_br (0, 0); // no acceptor
    
#warning preemption IPC with timeout never -- should be zero
    msg_tag_t tag = do_ipc(get_scheduler(), threadid_t::nilthread(), timeout_t::never());

    restore_state ();

    if (tag.is_error())
    {
	enter_kdebug("preemption IPC error");
    }
    return tag.is_error();
}

/**********************************************************************
 *
 *             global V4 thread management
 *
 **********************************************************************/

tcb_t SECTION(".init") * 
create_root_server(threadid_t dest_tid, threadid_t scheduler_tid, 
		   threadid_t pager_tid, fpage_t utcb_area, 
		   fpage_t kip_area, word_t utcb_location)
{
    ASSERT(dest_tid.is_global());
    ASSERT(scheduler_tid.is_global());
    ASSERT(pager_tid.is_global() || pager_tid.is_nilthread());
    ASSERT(!utcb_area.is_nil_fpage() && !(kip_area.is_nil_fpage()));

    tcb_t * tcb = get_current_space()->get_tcb(dest_tid);
    space_t * space = allocate_space();

    /* VU: we always assume these calls succeed for the root servers
     * if not - we are in deep shit anyway */
    ASSERT(space);
    ASSERT(tcb);

    tcb->create_inactive(dest_tid, scheduler_tid);
    space->init(utcb_area, kip_area);

    /* set the space */
    tcb->set_space(space);
    space->add_tcb(tcb);
    
    tcb->set_utcb_location (utcb_location);

    /* activate the guy */
    if (!tcb->activate(&thread_return, pager_tid))
	panic("failed to activate root server\n");

    /* and off we go... */
    tcb->set_state(thread_state_t::running);
    get_current_scheduler()->set_priority(tcb, ROOT_PRIORITY);
    get_current_scheduler()->enqueue_ready(tcb);

    return tcb;
}


SYS_THREAD_CONTROL (threadid_t dest_tid, threadid_t space_tid,
		    threadid_t scheduler_tid, threadid_t pager_tid, 
		    word_t utcb_location)
{
    TRACEPOINT (SYSCALL_THREAD_CONTROL, 
		printf ("SYS_THREAD_CONTROL: dest=%T, space=%T, "
			"scheduler=%T, pager=%T, utcb=%x\n", 
			TID (dest_tid), TID (space_tid), 
			TID (scheduler_tid), TID (pager_tid), 
			utcb_location));
    
    // invalid request - thread not privileged, or invalid thread id
    if (!is_priviledged_space(get_current_space()) || !dest_tid.is_global())
	return_thread_control(0);

    tcb_t * dest_tcb = get_current_space()->get_tcb(dest_tid);

    /* interrupt thread id ? */
    if (dest_tid.get_threadno() < get_kip()->thread_info.get_system_base())
    	return_thread_control(thread_control_interrupt(dest_tid, pager_tid) ? 1 : 0);

    /* do not allow the user to mess with kernel threads */
    if (dest_tid.get_threadno() < get_kip()->thread_info.get_user_base())
	goto error;

    if (space_tid.is_nilthread())
    {
	// do not allow deletion of ourself
	if (dest_tcb != get_current_tcb() && dest_tcb->exists())
	{
	    space_t * space = dest_tcb->get_space();
	    dest_tcb->delete_tcb();

	    if (space->remove_tcb(dest_tcb))
	    {
		// was the last thread
		space->free();
		free_space(space);
	    }
	    return_thread_control(1);
	}
    }
    else
    {
	/* 
	 * thread creation/modification 
	 * VU: since we are going to manipulate the thread allocate the tcb 
	 * before checking it. If it already exists this is a no-op, otherwise
	 * it saves unmapping the dummy tcb directly afterwards
	 */
	dest_tcb->allocate();

	// get the tcb of the space 
	tcb_t * space_tcb = get_current_space()->get_tcb(space_tid);

	if (dest_tcb->exists())
	{
	    //TRACEF("thread modification (%p)\n", dest_tcb->get_global_id().get_raw());
	    if (utcb_location != ~0UL)
	    {
		// do not allow modification of UTCB locations of 
		// already activated threads
		if (dest_tcb->is_activated())
		    goto error;
		dest_tcb->set_utcb_location(utcb_location);
	    }

	    // the hardest part first - space modifications
	    if (space_tcb->get_global_id() != space_tid)
		goto error;

	    space_t * space = space_tcb->get_space();
	    if (dest_tcb->get_space() != space)
	    {
		// space migration
		if (!dest_tcb->migrate_to_space(space))
		    goto error;
	    }

	    if (!pager_tid.is_nilthread())
	    {
		/* if the thread was inactive, setting the pager will
		 * activate the thread */
		if (!dest_tcb->is_activated())
		{
		    if (!dest_tcb->activate(&thread_startup, pager_tid))
			goto error;

		    fake_wait_for_startup (dest_tcb, pager_tid);
		}
		else
		    dest_tcb->set_pager(pager_tid);
	    }

	    if (!scheduler_tid.is_nilthread())
		dest_tcb->set_scheduler(scheduler_tid);


	    // change global id
	    if (dest_tcb->get_global_id() != dest_tid)
		dest_tcb->set_global_id(dest_tid);

	    return_thread_control(1);

	}
	else
	{
	    /* on creation of a new space scheduler must not be nilthread */
	    if (scheduler_tid.is_nilthread())
		goto error;

	    /* if the thread is not created in a fresh space
	     * make sure the space id is valid */
	    if ((dest_tid != space_tid) && (space_tcb->get_global_id() != space_tid))
		goto error;

	    /* if the thread is actively created make sure the space
             * is initialized and utcb specified by utcb_location fits
             * entirely into the utcb area */
	    if ( !pager_tid.is_nilthread() &&
		 ( (dest_tid == space_tid) || 
		   (!space_tcb->get_space()->is_initialized()) ||
		   (utcb_location == ~0U) ||
		   (!space_tcb->get_space()->get_utcb_page_area().is_range_in_fpage (
		       (addr_t)utcb_location, (addr_t)(utcb_location + sizeof(utcb_t)))) )
		)
		goto error;

	    /* ok, we can create the thread */
	    dest_tcb->create_inactive(dest_tid, scheduler_tid);

	    /* set UTCB location, the multiple checks are necessary to
	     * be comlient with the manual which says the operation
	     * succeeds or fails _completely_, do this after
	     * create_inactive -- it overwrites the local_tid */
	    if (utcb_location != ~0U)
		dest_tcb->set_utcb_location(utcb_location);

	    space_t * space;
	    if (dest_tid != space_tid)
		space = space_tcb->get_space();
	    else
		space = allocate_space();

	    /* VU: at that point we must have a space.
	     * do we have to handle a failing allocate_space? */
	    ASSERT(space); 

	    // set the space for the tcb
	    dest_tcb->set_space (space);
	    space->add_tcb (dest_tcb);

	    // if pager is not nil the thread directly goes into an IPC
	    if (! pager_tid.is_nilthread() )
	    {
		if (!dest_tcb->activate(thread_startup, pager_tid))
		{
		    // clean up tcb
		    UNIMPLEMENTED();
		}
		fake_wait_for_startup (dest_tcb, pager_tid);
	    }
	    return_thread_control(1);
	}
    }

error:
    return_thread_control(0);
}


static utcb_t kernel_utcb;

void SECTION(".init") init_kernel_threads()
{
    // Initialize the user base.  Currently simply leave some space.
    get_kip ()->thread_info.set_user_base
	(get_kip ()->thread_info.get_system_base () + 32);

    // Create a dummy kernel thread.
    threadid_t ktid;
    ktid.set_global_id (get_kip ()->thread_info.get_system_base (), 1);

    tcb_t * tcb = get_kernel_space ()->get_tcb (ktid);
    tcb->create_kernel_thread (ktid, &kernel_utcb);
    tcb->set_state (thread_state_t::aborted);
}


/**
 * initializes the root servers
 * Uses the configuration of the kernel interface page and sets up the
 * corresponding servers. At least sigma0 must be configured, otherwise the
 * function panics. The thread ids of sigma0, sigma1, and the root servers
 * are taken from the kip. Currently, UTCB and KIP area are compile-time 
 * constants.
 */
void SECTION(".init") init_root_servers()
{
    TRACE_INIT ("Initializing root servers\n");
    word_t ubase = get_kip()->thread_info.get_user_base();
    tcb_t * tcb;

    fpage_t utcb_area, kip_area;
    word_t size_utcb = 0;

    /* calculate size of UTCB area for root servers */
    
    while((1U << size_utcb) < max( get_kip()->utcb_info.get_minimal_size(), 
				   get_kip()->utcb_info.get_utcb_size() * ROOT_MAX_THREADS ))
	size_utcb++;

    utcb_area.set(ROOT_UTCB_START, size_utcb , 0, 0, 0);
    kip_area.set(ROOT_KIP_START, get_kip()->kip_area_info.get_size_log2(),
		 0, 0, 0);

    TRACE_INIT ("root-servers: utcb_area: %p (%dKB), kip_area: %p (%dKB)\n", 
		utcb_area.raw, utcb_area.get_size() / 1024, 
		kip_area.raw, kip_area.get_size() / 1024);

    // stop if system has no sigma0
    if (get_kip()->sigma0.mem_region.is_empty())
	panic ("Sigma0's memory region is empty, "
	       "system will not be functional.  Halting.\n");

    threadid_t sigma0, sigma1, root_server;
    sigma0.set_global_id(ubase, ROOT_VERSION);
    sigma1.set_global_id(ubase+1, ROOT_VERSION);
    root_server.set_global_id(ubase+2, ROOT_VERSION);
    
    TRACE_INIT ("creating sigma0 (%T)\n", TID(sigma0));
    tcb = create_root_server(
	sigma0,			// tid and space
	root_server,		// scheduler
	NILTHREAD,		// pager
	utcb_area,
	kip_area,
	ROOT_UTCB_START);
			
    tcb->set_user_ip((addr_t)get_kip()->sigma0.ip);
    tcb->set_user_sp((addr_t)get_kip()->sigma0.sp);
    sigma0_space = tcb->get_space();

    /* start sigma1 */
    if (!get_kip()->sigma1.mem_region.is_empty())
    {
	TRACE_INIT ("creating sigma1 (%T)\n", TID(sigma1));
	tcb = create_root_server(
	    sigma1,		// tid and space
	    root_server,	// scheduler
	    sigma0,		// pager
	    utcb_area,
	    kip_area,
	    ROOT_UTCB_START);


	tcb->set_user_ip((addr_t)get_kip()->sigma1.ip);
	tcb->set_user_sp((addr_t)get_kip()->sigma1.sp);
	sigma1_space = tcb->get_space();
    }

    /* start root task */
    if (!get_kip()->root_server.mem_region.is_empty())
    {
	TRACE_INIT ("creating root server (%T)\n", TID(root_server));
	tcb = create_root_server(
	    root_server,	// tid and space
	    root_server,	// scheduler
	    sigma0,		// pager
	    utcb_area,
	    kip_area,
	    ROOT_UTCB_START);

	tcb->set_user_ip((addr_t)get_kip()->root_server.ip);
	tcb->set_user_sp((addr_t)get_kip()->root_server.sp);
	roottask_space = tcb->get_space();
    }
}
