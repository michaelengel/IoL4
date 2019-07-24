/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  Karlsruhe University
 *                
 * File path:     api/v4/schedule.cc
 * Description:   Scheduling functions
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
 * $Id: schedule.cc,v 1.44.2.1 2003/06/03 20:15:20 skoglund Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include INC_API(tcb.h)
#include INC_API(schedule.h)
#include INC_API(interrupt.h)
#include INC_API(queueing.h)
#include INC_API(syscalls.h)
#include INC_API(smp.h)
#include INC_API(kernelinterface.h)
#include INC_GLUE(syscalls.h)
#include INC_GLUE(config.h)

#include <kdb/tracepoints.h>

/* global idle thread, we allocate a utcb to make accessing MRs etc easier */
whole_tcb_t __idle_tcb UNIT("cpulocal") __attribute__((aligned(sizeof(whole_tcb_t))));
utcb_t	    __idle_utcb UNIT("cpulocal") __attribute__((aligned(sizeof(utcb_t))));

/* global scheduler object */
scheduler_t scheduler UNIT("cpulocal");
volatile u64_t scheduler_t::current_time = 0;

#define TOTAL_QUANTUM_EXPIRED (~0ULL)

DECLARE_TRACEPOINT(SYSCALL_THREAD_SWITCH);
DECLARE_TRACEPOINT(SYSCALL_SCHEDULE);
DECLARE_TRACEPOINT(TOTAL_QUANTUM_EXPIRED);
DECLARE_TRACEPOINT(PREEMPTION_DELAYED);
DECLARE_TRACEPOINT(PREEMPTION_FAULT);
DECLARE_TRACEPOINT(PREEMPTION_DELAY_REFRESH);
DECLARE_TRACEPOINT(WAKEUP_TIMEOUT);


/**
 * Compare two time_t values, either absoulte time points or time
 * points relative to current time.
 *
 * @param r		time to compare against
 *
 * @return true if current time point is earlier than R
 */
bool time_t::operator< (time_t & r)
{
    u64_t curtime = get_current_scheduler ()->get_current_time ();
    u64_t l_to, r_to;

    // Calculate absolute time of current time value
    if (this->is_point ())
	UNIMPLEMENTED ();
    else if (this->is_never ())
	l_to = ~0UL;
    else
	l_to = curtime + this->get_microseconds ();

    // Calculate absolute time of right time value.
    if (r.is_point ())
	UNIMPLEMENTED ();
    else if (r.is_never ())
	r_to = ~0UL;
    else
	r_to = curtime + r.get_microseconds ();

    return l_to < r_to;
}


/**
 * sends preemption IPC to the scheduler thread that the total quantum
 * has expired */
void scheduler_t::total_quantum_expired(tcb_t * tcb)
{
    TRACEPOINT(TOTAL_QUANTUM_EXPIRED, 
	       printf("total quantum expired for %p\n", tcb));
    tcb->send_preemption_ipc(current_time);
}


/**
 * find_next_thread: selects the next tcb to be dispatched
 *
 * returns the selected tcb, if no runnable thread is available, 
 * the idle tcb is returned.
 */
tcb_t * scheduler_t::find_next_thread(prio_queue_t * prio_queue)
{
    ASSERT(prio_queue);

    for (prio_t prio = prio_queue->max_prio; prio >= 0; prio--)
    {
	//TRACEF("prio=%d, (max=%d)\n", prio, max_prio);
	tcb_t *tcb = prio_queue->get(prio);
	while (tcb) {
	    ASSERT(tcb->queue_state.is_set(queue_state_t::ready));

	    if ( tcb->get_state().is_runnable() /*&& tcb->current_timeslice > 0 */)
	    {
		prio_queue->set(prio, tcb);
		prio_queue->max_prio = prio;
		//TRACE("find_next: returns %p\n", tcb);
		return tcb;
	    }
	    else {
		if (tcb->get_state().is_runnable())
		    TRACEF("dequeueing runnable thread without timeslice (%p)\n", tcb);
		/* dequeue non-runnable thread */
		prio_queue->dequeue(tcb);
		tcb = prio_queue->get(prio);
	    }
	}
    }
    /* if we can't find a schedulable thread - switch to idle */
    //TRACE("find_next: returns idle\n");
    prio_queue->max_prio = -1;
    return get_idle_tcb();
}

tcb_t * scheduler_t::parse_wakeup_queues(tcb_t * current)
{
    if (!wakeup_list)
	return current;

    tcb_t * highest_wakeup = current;
    tcb_t * tcb = wakeup_list;

    do
    {
	if ( has_timeout_expired(tcb, current_time) )
	{
	    /*
	     * We might try to wake up a thread which is waiting
	     * forever.  This can happen if:
	     *
	     *  1) we have issued a timeout IPC, an IPC fast path 
	     * 	   reply occured before the timeout triggered, and an
	     * 	   infinite timeout IPC was issued.  Since we're doing
	     * 	   fast IPC we will do the dequeing lazily (i.e.,
	     * 	   doing it now instead).
	     *
	     *  2) this is an xfer timeout, in which case we should
	     *     let the timeout trigger.
	     */
	    if (tcb->get_state().is_waiting_forever () &&
		! tcb->has_xfer_timeout)
	    {
		tcb_t * tmp = tcb->wait_list.next;
		cancel_timeout (tcb);
		tcb = tmp;
		continue;
	    }

	    /* we have to wakeup the guy */
	    if (check_dispatch_thread(highest_wakeup, tcb))
		highest_wakeup = tcb;

	    tcb_t * tmp = tcb->wait_list.next;
	    cancel_timeout(tcb);
	    tcb->has_xfer_timeout = false;

	    if (tcb->get_state().is_sending() ||
		tcb->get_state().is_receiving())
	    {
		/*
		 * The thread must invoke function which handles the
		 * IPC timeout.  The handler returns directly to user
		 * level with an error code.  As such, no special
		 * timeout code is needed in the IPC path.
		 */
		tcb->notify (handle_ipc_timeout, (word_t) tcb->get_state ());
	    }
	    
	    /* set it running and enqueue into ready-queue */
	    tcb->set_state(thread_state_t::running);
	    enqueue_ready(tcb);

	    TRACEPOINT(WAKEUP_TIMEOUT, printf("wakeup timeout (curr=%p wu=%p)\n",
					      current, tcb ));

	    /* was preempted -- give him a fresh timeslice */
	    //tcb->current_timeslice = tcb->timeslice_length;
	    tcb = tmp;
	}
	else
	    tcb = tcb->wait_list.next;
    } while ( tcb != wakeup_list && wakeup_list);

    return highest_wakeup;
}

/**
 * selects the next runnable thread and activates it.
 * @return true if a runnable thread was found, false otherwise
 */
bool scheduler_t::schedule(tcb_t * current)
{
    tcb_t * tcb = find_next_thread (&root_prio_queue);
    
    ASSERT(tcb);
    ASSERT(current);
    
    // do not switch to ourself
    if (tcb == current)
	return false;

    if (current != get_idle_tcb())
	enqueue_ready(current);

    current->switch_to(tcb);

    return true;
}

/**
 * selects the next runnable thread in a round-robin fashion
 */
void scheduler_t::end_of_timeslice (tcb_t * tcb)
{
    spin(74, get_current_cpu());
    ASSERT(tcb);
    ASSERT(tcb != get_idle_tcb()); // the idler never yields

    prio_queue_t * prio_queue = get_prio_queue ( tcb );
    ASSERT(prio_queue);
    
    /* make sure we are in the run queue, put the TCB in front to
     * maintain the RR order */
    prio_queue->enqueue (tcb, true);

    /* now schedule RR */
    prio_queue->set(get_priority (tcb), tcb->ready_list.next);

    // renew timeslice
    tcb->current_timeslice = prio_queue->current_timeslice + tcb->timeslice_length;

    /* now give the prio queue the newly scheduled thread's timeslice;
     * if we are the only thread in this prio we are going to be
     * rescheduled */
    prio_queue->current_timeslice = prio_queue->get(get_priority (tcb))->current_timeslice;
}


void scheduler_t::handle_timer_interrupt()
{
    spin(77, get_current_cpu());
    kdebug_check_breakin();

    if (get_current_cpu() == 0)
    {
	/* update the time global time*/
	current_time += get_timer_tick_length();
    }

#if defined(CONFIG_SMP)
    process_xcpu_mailbox();
#endif

    tcb_t * current = get_current_tcb();
    tcb_t * wakeup = parse_wakeup_queues(current);

    /* the idle thread schedules itself so no point to do it here.
     * Furthermore, it should not be preempted on end of timeslice etc.
     */
    if (current == get_idle_tcb())
	return;

    // tick timeslice
    bool reschedule = false;
    get_prio_queue(current)->current_timeslice -= get_timer_tick_length();

    if (EXPECT_FALSE( get_prio_queue(current)->current_timeslice <= 0 ) )
    {
	if ( !current->get_preempt_flags().is_delayed() ||
	     !this->delay_preemption(current, wakeup) )
	{
	    end_of_timeslice ( current );
	    reschedule = true;
	}
    }

    /* a higher priority thread was woken up - switch to him.
     * Note: wakeup respects delayed preemption flags */
    if (!reschedule)
    {
	if ( wakeup == current)
	    return;

	ASSERT(wakeup);
	//printf("wakeup preemption %p->%p\n", current, wakeup);
	preempt_thread(current, wakeup);
	current->switch_to(wakeup);
	return;
    }

    /* time slice expired */
    if (EXPECT_FALSE (current->total_quantum))
    {
	/* we have a total quantum - so do some book-keeping */
	if (current->total_quantum == TOTAL_QUANTUM_EXPIRED)
	{
	    /* VU: must be revised. If a thread has an expired time quantum
	     * and is activated with switch_to his timeslice will expire
	     * and the event will be raised multiple times */
	    total_quantum_expired (current);
	}
	else if (current->total_quantum <= current->timeslice_length)
	{
	    /* we are getting close... */
	    current->current_timeslice += current->total_quantum;
	    current->total_quantum = TOTAL_QUANTUM_EXPIRED;
	}
	else
	{
	    // account this time slice
	    current->total_quantum -= current->timeslice_length;
	}
    }

    /* schedule the next thread */
    enqueue_ready(current);
    schedule(current);
}

bool scheduler_t::delay_preemption( tcb_t * current, tcb_t * tcb )
{
    // we always allow ourself to delay our preemption
    if (current == tcb)
	return true;
    
    if ( get_sensitive_prio (current) < get_priority (tcb) )
	return false;

    if ( current->current_max_delay )
    {
	TRACEPOINT(PREEMPTION_DELAYED, 
		   printf("delayed preemption thread=%p, time=%dus\n", 
			  current, current->current_max_delay));
	
	/* VU: should we give max_delay? */
	get_prio_queue(current)->current_timeslice +=
	    current->current_max_delay;
	current->current_max_delay = 0;
	current->set_preempt_flags( current->get_preempt_flags().set_pending() );
	return true;
    }
    else
    {
#warning delayed preemption faults are not signaled to exception handler
	// preemption fault
	TRACEPOINT(PREEMPTION_FAULT, 
		   printf("delay preemption fault by %p\n", current));
	return false;
    }
}
	

static void init_all_threads(void)
{
    init_interrupt_threads();
    init_kernel_threads();
    init_root_servers();

#if defined(CONFIG_KDB_ON_STARTUP)
    enter_kdebug ("System started (press 'g' to continue)");
#endif
}

/**
 * the idle thread checks for runnable threads in the run queue
 * and performs a thread switch if possible. Otherwise, it 
 * invokes a system sleep function (which should normally result in
 * a processor halt)
 */
static void idle_thread()
{
    TRACE_INIT("idle thread started on CPU %d\n", get_current_cpu());

    while(1)
    {
	if (!get_current_scheduler()->schedule(get_idle_tcb()))
	{
	    spin(78, get_current_cpu());
	    processor_sleep();
	}
    }
}

SYS_THREAD_SWITCH (threadid_t dest)
{
    /* Make sure we are in the ready queue to 
     * find at least ourself and ensure that the thread 
     * is rescheduled */
    tcb_t * current = get_current_tcb();
    scheduler_t * scheduler = get_current_scheduler();
    scheduler->enqueue_ready(current);

    TRACEPOINT( SYSCALL_THREAD_SWITCH,
		printf("SYS_THREAD_SWITCH current=%p, dest=%T\n",
		       current, TID(dest)) );

    /* explicit timeslice donation */
    if (!dest.is_nilthread())
    {
	tcb_t * dest_tcb = get_current_space()->get_tcb(dest);
	if ( dest_tcb->get_state().is_runnable() &&
	     dest_tcb->myself_global == dest &&
	     dest_tcb->is_local_cpu() )
	{
	    current->switch_to(dest_tcb);
	    return_thread_switch();
	}
    }

    /* user cooperatively preempts */
    if ( current->current_max_delay < current->max_delay )
    {
	current->set_preempt_flags( current->get_preempt_flags().clear_pending() );
	/* refresh max delay */
	current->current_max_delay = current->max_delay;
	TRACEPOINT(PREEMPTION_DELAY_REFRESH, 
		   printf("delayed preemption refresh for %p\n", current));
    }


    /* eat up timeslice - we get a fresh one */
    scheduler->get_prio_queue(current)->current_timeslice = 0;
    scheduler->end_of_timeslice (current);
    scheduler->schedule (current);
    return_thread_switch();
}


/* local part of schedule */
bool do_schedule(tcb_t * tcb, word_t time_control, word_t prio, 
		 word_t preemption_control )
{
    ASSERT(tcb->get_cpu() == get_current_cpu());

    scheduler_t * scheduler = get_current_scheduler();

    if ( (prio != (~0UL)) && ((prio_t)prio != scheduler->get_priority(tcb)) )
    {
	scheduler->dequeue_ready(tcb);
	scheduler->set_priority(tcb, (prio_t) (prio & 0xff));
	scheduler->enqueue_ready(tcb);
    }

    if ( time_control != (~0UL) )
    {
	time_t time;
	
	// total quantum
	time.set_raw ((u16_t)time_control);
	if ( time.is_period() )
	{
	    scheduler->set_total_quantum (tcb, time);
	    scheduler->enqueue_ready (tcb);
	}
	
	// timeslice length
	time.set_raw (time_control >> 16);
	if ( time.is_period() )
	    scheduler->set_timeslice_length (tcb, time);
    }

    if ( preemption_control != (~0UL) )
    {
#warning VU: limit upper bound of sensitive prio!
	scheduler->set_maximum_delay (tcb, preemption_control & 0xffff);
	    
	/* only set sensitive prio if _at least_ equal to current prio */
	prio_t sens_prio = (preemption_control >> 16) & 0xff;
	if ( sens_prio >= scheduler->get_priority(tcb) )
	    scheduler->set_sensitive_prio(tcb, sens_prio);
    }

    return true;
}

#ifdef CONFIG_SMP
static void do_xcpu_schedule(cpu_mb_entry_t * entry)
{
    tcb_t * tcb = entry->tcb;

    // meanwhile migrated? 
    if (tcb->get_cpu() != get_current_cpu())
	xcpu_request (tcb->get_cpu(), do_xcpu_schedule, tcb, 
		      entry->param[0], entry->param[1], entry->param[2]);
    else
	do_schedule (tcb, entry->param[0], entry->param[1], entry->param[2]);
}
#endif

SYS_SCHEDULE (threadid_t dest_tid, word_t time_control, 
	      word_t processor_control, word_t prio,
	      word_t preemption_control )
{
    
    TRACEPOINT(SYSCALL_SCHEDULE, 
	printf("SYS_SCHEDULE: curr=%p, dest=%T, time_ctrl=%x, "
	       "proc_ctrl=%x, prio=%x, preemption_ctrl=%x\n",
	       get_current_tcb(), TID(dest_tid), time_control, 
	       processor_control, prio, preemption_control));

    tcb_t * dest_tcb = get_current_space()->get_tcb(dest_tid);
    
    // make sure the thread id is valid
    if (dest_tcb->get_global_id() != dest_tid)
	return_schedule(0, 0);

    // are we in the same address space as the scheduler of the thread?
    tcb_t * sched_tcb = get_current_space()->get_tcb(dest_tcb->get_scheduler());
    if (sched_tcb->get_global_id() != dest_tcb->get_scheduler() ||
	sched_tcb->get_space() != get_current_space())
	return_schedule(0, 0);

    if ( dest_tcb->is_local_cpu() )
    {
	do_schedule ( dest_tcb, time_control, prio, preemption_control);
	
	if ( processor_control != ~0UL ) 
	    dest_tcb->migrate_to_processor (processor_control);
    }
#ifdef CONFIG_SMP
    else
    {
	xcpu_request(dest_tcb->get_cpu(), do_xcpu_schedule, 
		     dest_tcb, time_control, prio, preemption_control);

	if ( processor_control != ~0UL ) 
	    dest_tcb->migrate_to_processor (processor_control);
    }
#endif

    thread_state_t state = dest_tcb->get_state();

    return_schedule(state == thread_state_t::aborted	? 1 : 
		    state.is_halted()			? 2 :
		    state.is_running()			? 3 :
		    state.is_polling()			? 4 :
		    state.is_sending()			? 5 :
		    state.is_waiting()			? 6 :
		    state.is_receiving()		? 7 : ({ TRACEF("invalid state (%x)\n", (word_t)state); 0; }),
		    0);
}


/**********************************************************************
 *
 *                     Initialization
 *
 **********************************************************************/

void SECTION(".init") scheduler_t::start(cpuid_t cpuid)
{
    TRACE_INIT ("Switching to idle thread (CPU %d)\n", cpuid);
    get_idle_tcb()->set_cpu(cpuid);

    initial_switch_to(get_idle_tcb());
}

void SECTION(".init") scheduler_t::init( bool bootcpu )
{
    TRACE_INIT ("Initializing threading\n");

    /* wakeup list */
    wakeup_list = NULL;
    root_prio_queue.init();

    get_idle_tcb()->create_kernel_thread(NILTHREAD, &__idle_utcb);
    /* set idle-magic */
    get_idle_tcb()->myself_global.set_raw((word_t)0x1d1e1d1e1d1e1d1eULL);
    get_idle_tcb()->notify(idle_thread);
    if( bootcpu )
    	get_idle_tcb()->notify(init_all_threads);
    return;
}
