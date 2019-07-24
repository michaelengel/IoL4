/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     api/v4/space.cc
 * Description:   architecture independent parts of space_t
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
 * $Id: space.cc,v 1.52 2003/04/30 14:42:50 ud3 Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include <kmemory.h>
#include <kdb/tracepoints.h>
#include INC_API(space.h)
#include INC_API(tcb.h)
#include INC_API(kernelinterface.h)
#include INC_API(schedule.h)
#include INC_API(syscalls.h)
#include INC_API(threadstate.h)
#include INC_GLUE(syscalls.h)


space_t * sigma0_space;
space_t * sigma1_space;
space_t * roottask_space;

DECLARE_TRACEPOINT (PAGEFAULT_USER);
DECLARE_TRACEPOINT (PAGEFAULT_KERNEL);
DECLARE_TRACEPOINT (PAGEFAULT_TUNNEL);
DECLARE_TRACEPOINT (SYSCALL_SPACE_CONTROL);
DECLARE_TRACEPOINT (SYSCALL_UNMAP);

DECLARE_KMEM_GROUP (kmem_space);


#if defined(CONFIG_SMP)

void tunnel_pagefault (word_t addr);

/**
 * Handler invoked to resume the IPC copy opearation after a tunneled
 * pagefault has been completed.
 */
static void do_xcpu_tunnel_pf_done (cpu_mb_entry_t * entry)
{
    tcb_t * tcb = entry->tcb;

    //TRACEF ("current=%p, tcb=%p\n", get_current_tcb (), tcb);

    if (! tcb->is_local_cpu ())
    {
	xcpu_request (tcb->get_cpu (), do_xcpu_tunnel_pf_done, tcb);
	return;
    }

    // Restore state for current thread.
    tcb->set_state (thread_state_t::locked_running);
    get_current_scheduler ()->enqueue_ready (tcb);
}

/**
 * Handler invoked to tunnel a IPC copy pagefault to the destination
 * thread (running on another CPU).
 */
static void do_xcpu_tunnel_pf (cpu_mb_entry_t * entry)
{
    tcb_t * tcb = entry->tcb;
    word_t addr = entry->param[0];

    //TRACEF ("current=%p, tcb=%p, addr=%p\n", get_current_tcb (), tcb, addr);

    if (! tcb->is_local_cpu ())
    {
	xcpu_request (tcb->get_cpu (), do_xcpu_tunnel_pf, tcb, addr);
	return;
    }

    // Set up a pagefault notify for destination thread.
    tcb->notify (tunnel_pagefault, addr);
    tcb->set_state (thread_state_t::locked_running_nested);
    get_current_scheduler ()->enqueue_ready (tcb);
}
#endif


/**
 * Handler invoked when IPC copy page-fault has been tunnelled to
 * current space.  Invoke a page-fault IPC to pager, and resume the
 * IPC copy operation.
 *
 * @param addr			fault address
 */
void tunnel_pagefault (word_t addr)
{
    tcb_t * current = get_current_tcb ();
    tcb_t * partner = current->get_partner_tcb ();

    TRACEPOINT (PAGEFAULT_TUNNEL,
		printf ("tunnelled page fault @ %p (current=%p, partner=%p)\n",
			addr, current, partner));

    current->send_pagefault_ipc ((addr_t) addr, (addr_t) ~0UL, space_t::write);
    current->set_state (thread_state_t::locked_waiting);

#if defined(CONFIG_SMP)
    if (! partner->is_local_cpu ())
    {
	// Partner on remote CPU.  Need to send wake-up request.
	xcpu_request (partner->get_cpu (), do_xcpu_tunnel_pf_done, partner);
	current->switch_to_idle ();
    }
    else
#endif
    {
	// Partner on same CPU.  Switch back directly.
	partner->set_state (thread_state_t::locked_running);
	get_current_scheduler ()->enqueue_ready (partner);
	current->switch_to (partner);
    }
}


/**
 * Handle transfer timeouts.  We do not set up any timeouts if both
 * xfer timeouts are infinite.  If any xfer timeout is zero, we abort
 * IPC immediately.  Otherwise, we find the earliest timeout and set
 * up a timeout for the sender thread.
 *
 * @param sender		sender thread
 */
static void handle_xfer_timeouts (tcb_t * sender)
{
#warning Handle priority inversion for xfer timeouts
    
    // Skip timeout calculation if already in wakeup queue.
    if (sender->has_xfer_timeout)
	return;

    tcb_t * partner = sender->get_partner_tcb();

    ASSERT (! sender->get_partner ().is_nilthread ());
    ASSERT (sender->get_state() == thread_state_t::locked_running);
    ASSERT (partner->get_state() == thread_state_t::locked_waiting);
 
    time_t snd_to = sender->get_xfer_timeout_snd ();
    time_t rcv_to = partner->get_xfer_timeout_rcv ();

    if (snd_to.is_zero () || rcv_to.is_zero ())
    {
	// Timeout immediately.
	handle_ipc_timeout (thread_state_t::locked_running);
    }
    else if (snd_to.is_never () && rcv_to.is_never ())
    {
	// No timeouts.
	return;
    }

    // Set timeout on the sender side.
    get_current_scheduler ()->set_timeout (sender,
					   snd_to < rcv_to ? snd_to : rcv_to);
    sender->has_xfer_timeout = true;
}

void space_t::handle_pagefault(addr_t addr, addr_t ip, access_e access, bool kernel)
{
    tcb_t * current = get_current_tcb();
    bool user_area = is_user_area(addr);

    if (user_area || (!kernel))
    {
	TRACEPOINT_TB (PAGEFAULT_USER,
		       ("user page fault at %x (current=%p, ip=%x, access=%x)",
			(u32_t)addr, (u32_t)current, (u32_t)ip, access),
		       printf("%p: user %s pagefault at %p, ip=%p\n", 
			      current,
			      access == space_t::write     ? "write" :
			      access == space_t::read	   ? "read"  :
			      access == space_t::execute   ? "execute" :
			      access == space_t::readwrite ? "read/write" :
			      "unknown", 
			      addr, ip));

#warning sigma0-check in default pagefault handler. 
	/* VU: with software loaded tlbs that could be handled elsewhere...*/
	if (EXPECT_TRUE( !is_sigma0_space(current->get_space()) ))
	{
	    if (kernel &&
		current->get_state() != thread_state_t::locked_running)
	    {
		printf("kernel access raised user pagefault @ %p, ip=%p, "
		       "space=%p\n", addr, ip, this);
		enter_kdebug("kpf");
	    }
	    if (!user_area)
	    {
		printf("%p pf @ %p, ip=%p\n", current, addr, ip);
		enter_kdebug("user touches kernel area");
	    }

	    if (current->get_state() == thread_state_t::locked_running)
	    {
		// Pagefault during IPC copy.  Initiate xfer timeout
		// counters before handling pagefault.
		current->misc.ipc_copy.copy_fault = addr;
		handle_xfer_timeouts (current);
	    }

	    // if we have a user fault we may have a stale partner
	    if (!kernel)
		current->set_partner(threadid_t::nilthread());

	    current->send_pagefault_ipc (addr, ip, access);
	}
	else
	{
	    if (user_area)
		map_sigma0(addr);
	    else
	    {
		printf("sigma0 accessed kernel space @ %p, ip=%p - deny\n",
		       addr, ip);
		enter_kdebug("sigma0 kpf");
	    }
	}
	return;
    }
    else
    {
	/* fault in kernel area */
	TRACEPOINT_TB (PAGEFAULT_KERNEL,
		       ("kernel page fault at %x "
			"(current=%p, ip=%x, space=%p)",
			(u32_t)addr, (u32_t)current, (u32_t)ip, (u32_t)this),
		       printf ("%p: kernel pagefault in space "
			       "%p @ %p, ip=%p, type=%x\n",
			       current, this, addr, ip, access));

	if (sync_kernel_space(addr))
	    return;

	if (is_tcb_area(addr))
	{
	    /* write access to tcb area? */
	    if (access == space_t::write)
	    {
		//TRACEF("allocate tcb %p\n", addr);
		allocate_tcb(addr);
	    }
	    else
	    {
		map_dummy_tcb(addr);
	    }
	    return;
	}
	else if (is_copy_area (addr))
	{
	    // Fault in copy area.  Tunnel pagefault through partner.
	    current->misc.ipc_copy.copy_fault = addr;
	    handle_xfer_timeouts (current);

	    // On PF tunneling we temporarily set the current thread
	    // into waiting for partner.
	    current->set_state (thread_state_t::waiting_tunneled_pf);

	    tcb_t * partner = get_tcb (current->get_partner ());
	    word_t faddr = (word_t) current->copy_area_real_address (addr);

#if defined(CONFIG_SMP)
	    if (! partner->is_local_cpu ())
	    {
		// Partner on remote CPU.  Need to send xcpu request.
		xcpu_request (partner->get_cpu (), do_xcpu_tunnel_pf,
			      partner, faddr);
		current->switch_to_idle ();
	    }
	    else
#endif
	    {
		// Partner on same CPU.  Just switch directly.
		partner->set_state (thread_state_t::locked_running_nested);
		partner->notify (tunnel_pagefault, faddr);
		current->switch_to (partner);
		get_current_scheduler ()->enqueue_ready (current);
	    }
	    return;
	}
    }
    TRACEF("unhandled pagefault @ %p, %p\n", addr, ip);
    enter_kdebug("unhandled pagefault");

    current->set_state(thread_state_t::halted);
    if (current != get_idle_tcb())
	current->switch_to_idle();
    printf("wrong access - unable to recover\n");
    spin_forever(1);
}



SYS_SPACE_CONTROL (threadid_t space_tid, word_t control, fpage_t kip_area,
		   fpage_t utcb_area, threadid_t redirector_tid)
{
    TRACEPOINT (SYSCALL_SPACE_CONTROL,
		printf("SYS_SPACE_CONTROL: space=%T, control=%p, kip_area=%p, "
		       "utcb_area=%p, redir=%T\n",  TID (space_tid),
		       control, kip_area.raw, utcb_area.raw,
		       TID (redirector_tid)));

    // invalid request - thread not privileged, or space id not global
    if (!is_priviledged_space(get_current_space()) || !space_tid.is_global())
	return_space_control(0, 0);

    tcb_t * space_tcb = get_current_space()->get_tcb(space_tid);
    if ( space_tcb->get_global_id() != space_tid )
	return_space_control(0, 0);

    space_t * space = space_tcb->get_space();
    
#warning redirector and control ignored, space cannot be modified even if no thread is active
    /* if space already active do not fiddle with config */
    if ( space->is_initialized() )
	return_space_control(0, 0);

    /* areas too small || areas not in valid user space */
    if ( ( get_kip()->kip_area_info.get_size() > kip_area.get_size()) ||
	 ( get_kip()->utcb_info.get_minimal_size() > utcb_area.get_size() ) ||
	 (!space->is_user_area(kip_area)) ||
	 (!space->is_user_area(utcb_area)) )
    {
	return_space_control(0, 0);
    }
    else 
    {
	/* ok, everything seems fine, now setup the space */
	space->init(utcb_area, kip_area);
	return_space_control(1, 0);
    }

    spin_forever();
}

SYSCALL_ATTR("unmap") void sys_unmap(word_t control)
{
    fpage_t fpage;
    word_t num = control & 0x3f;
    bool flush = control & (1 << 6) ? true : false;

    TRACEPOINT(SYSCALL_UNMAP,
	printf("SYS_UNMAP: control=0x%x (num=%d, flush=%d)\n", 
	       control, num, flush));

#warning VU: adapt to new msg_tag scheme
    for (word_t i = 0; i <= num; i++)
    {
	fpage.raw = get_current_tcb()->get_mr(i);
	fpage = get_current_space()->unmap_fpage(fpage, flush, false);
	get_current_tcb()->set_mr(i, fpage.raw);
    }
    return_unmap();
}

/**
 * allocate_space: allocates a new space_t
 */
#if !defined(CONFIG_ARCH_ALPHA)
space_t * allocate_space()
{
    space_t * space = (space_t*)kmem.alloc(kmem_space, sizeof(space_t));
    ASSERT(space);
    return space;
}

/**
 * free_space: frees a previously allocated space
 */
void free_space(space_t * space)
{
    ASSERT(space);
    kmem.free(kmem_space, (addr_t)space, sizeof(space_t));
}
#endif /* !defined(CONFIG_ARCH_ALPHA) */


void space_t::free (void)
{
    // Unmap everything, including KIP and UTCB
    fpage_t fp = fpage_t::complete ();
    fp.set_rwx ();
    unmap_fpage (fp, true, true);
}
