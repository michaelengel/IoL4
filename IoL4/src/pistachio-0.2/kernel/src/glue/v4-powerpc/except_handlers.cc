/****************************************************************************
 *
 * Copyright (C) 2002, Karlsruhe University
 *
 * File path:	glue/v4-powerpc/except_handlers.cc
 * Description:	PowerPC exception handlers.
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
 * $Id: except_handlers.cc,v 1.48 2003/04/30 00:22:32 joshua Exp $
 *
 ***************************************************************************/

#include <debug.h>
#include <kdb/tracepoints.h>

#include INC_ARCH(phys.h)
#include INC_ARCH(debug.h)
#include INC_ARCH(string.h)
#include INC_ARCH(except.h)
#include INC_ARCH(msr.h)

#include INC_PLAT(frame.h)

#include INC_API(tcb.h)
#include INC_API(schedule.h)
#include INC_API(syscalls.h)
#include INC_API(interrupt.h)
#include INC_API(kernelinterface.h)

#include INC_GLUE(exception.h)
#include INC_GLUE(syscalls.h)


#if defined(CONFIG_DEBUG)
extern void putc(char c);
extern char getc(void);
#endif

DECLARE_TRACEPOINT(except_isi_cnt);
DECLARE_TRACEPOINT(except_dsi_cnt);
DECLARE_TRACEPOINT(except_prog_cnt);
DECLARE_TRACEPOINT(except_decr_cnt);

/*  The EXCEPT_ID() macro identifies an exception, based on its physical 
 *  location as defined by the PowerPC ISA, for the exception IPC.
 */
#define EXCEPT_ID(a) (EXCEPT_OFFSET_##a - EXCEPT_OFFSET_BASE)

/*  EXCDEF is a macro which helps consistantly declare exception handlers,
 *  while reducing typing :)
 */
#define EXCDEF(n,params...) extern "C" __attribute__((noreturn)) void except_##n (word_t srr0 , word_t srr1 , word_t *frame , ## params )

/* except_return() short circuits the C code return path.
 * We declare the exception handlers as noreturn, to avoid
 * the C prolog (which redundantly spills registers which the assembler
 * path already spills).
 */
#define except_return()			\
do {					\
    asm volatile (			\
	    "mtlr %0 ;"			\
	    "mr %%r1, %1 ;"		\
	    "blr ;"			\
	    :				\
	    : "r" (__builtin_return_address(0)), \
	      "b" (__builtin_frame_address(1)) \
	    );				\
    while(1);				\
} while(0)

static void dump( const char *msg, word_t srr0, word_t srr1, word_t *frame )
{
    int j;
    word_t *gpr = &frame[ FRAME_IDX(KSTACK_R0) ];

    printf( "%s\n", msg );
    printf( "** srr0: %p  srr1: %p\n", srr0, srr1 );
    printf( "**  ctr: %p    lr: %p    cr: %p   xer: %p\n",
	    frame[ FRAME_IDX(KSTACK_CTR) ], 
	    frame[ FRAME_IDX(KSTACK_LR) ], 
	    frame[ FRAME_IDX(KSTACK_CR) ], 
	    frame[ FRAME_IDX(KSTACK_XER) ] );

    for( j = 0; j < 32; j += 4 )
	printf( "** r%d %p  r%d %p  r%d %p r%d %p\n",
		j, gpr[j], j+1, gpr[j+1], j+2, gpr[j+2], j+3, gpr[j+3] );
}

static void try_to_debug( const char *msg, word_t srr0, word_t srr1, word_t *frame )
{
    if( MSR_BIT(srr1, MSR_RI) == 0 ) {
	printf( "**** unrecoverable exception ****\n" );
    	dump( msg, srr0, srr1, frame );
      	spin_forever();
    }

    if( !get_kip()->kdebug_entry ) {
	dump( msg, srr0, srr1, frame );
	spin_forever();
    }

    printf( "--- %s ---\n", msg );
    printf( "CPU %d, IP: 0x%08x, MSR: 0x%08x\n", 
	    get_current_cpu(), srr0, srr1 );

    get_kip()->kdebug_entry( (void *)frame );
}

static bool send_exception_ipc( word_t exc_no, word_t exc_code )
{
    tcb_t *current = get_current_tcb();
    if( current->get_exception_handler().is_nilthread() )
	return false;

    // Setup exception IPC.
#define EXC_IPC_SAVED_REGISTERS (GENERIC_EXC_MR_MAX+1)
    word_t saved_mr[EXC_IPC_SAVED_REGISTERS];
    msg_tag_t tag;

    // Save message registers.
    for( int i = 0; i < EXC_IPC_SAVED_REGISTERS; i++ )
	saved_mr[i] = current->get_mr(i);
    current->set_saved_partner( current->get_partner() );

    // Create the message tag.
    tag.set( 0, GENERIC_EXC_MR_MAX, GENERIC_EXC_LABEL );
    current->set_mr( 0, tag.raw );

    // Create the message.
    current->set_mr( GENERIC_EXC_MR_UIP,      (word_t)current->get_user_ip() );
    current->set_mr( GENERIC_EXC_MR_USP,      (word_t)current->get_user_sp() );
    current->set_mr( GENERIC_EXC_MR_UFLAGS,   (word_t)current->get_user_flags() );
    current->set_mr( GENERIC_EXC_MR_NO,       exc_no );
    current->set_mr( GENERIC_EXC_MR_CODE,     exc_code );
    current->set_mr( GENERIC_EXC_MR_LOCAL_ID, current->get_local_id().get_raw() );

    // Deliver the exception IPC.
    tag = current->do_ipc( current->get_exception_handler(),
	    current->get_exception_handler(), timeout_t::never() );

    // Alter the user context if necessary.
    if( !tag.is_error() )
    {
	current->set_user_ip( (addr_t)current->get_mr(GENERIC_EXC_MR_UIP) );
	current->set_user_sp( (addr_t)current->get_mr(GENERIC_EXC_MR_USP) );
	current->set_user_flags( current->get_mr(GENERIC_EXC_MR_UFLAGS) );
    }
    else
	WARNING( "exception delivery error" );

    // Clean-up.
    for( int i = 0; i < EXC_IPC_SAVED_REGISTERS; i++ )
	current->set_mr( i, saved_mr[i] );
    current->set_partner( current->get_saved_partner() );
    current->set_saved_partner( NILTHREAD );

    return !tag.is_error();
}

EXCDEF( dsi_handler, word_t dar, word_t dsisr )
{
    TRACEPOINT(except_dsi_cnt);

    if( EXPECT_FALSE(dar == 0) ) 
    {
	if( ppc_is_kernel_mode(srr1) )
	    try_to_debug( "** kernel null pointer **", srr0, srr1, frame );
	else
	    try_to_debug( "** null pointer **", srr0, srr1, frame );
    }

    tcb_t *tcb = get_current_tcb();
    space_t *space = tcb->get_space();
    if( EXPECT_FALSE(space == NULL) )
	space = get_kernel_space();

    // Do we have a page hash miss?
    if( EXCEPT_IS_DSI_MISS(dsisr) )
    {

	// Is the page hash miss in the copy area?
	if( EXPECT_FALSE(space->is_copy_area((addr_t)dar)) )
	{
	    // Resolve the fault using the partner's address space!
	    tcb_t *partner = space->get_tcb( tcb->get_partner() );
	    if( partner )
	    {
		addr_t real_fault = tcb->copy_area_real_address( (addr_t)dar );
		if( partner->get_space()->handle_hash_miss(real_fault) )
	    	    except_return();
	    }
	}

	// Normal page hash miss.
	if( EXPECT_TRUE(space->handle_hash_miss((addr_t)dar)) )
	    except_return();
    }

    space->handle_pagefault( (addr_t)dar, (addr_t)srr0, 
	    EXCEPT_IS_DSI_WRITE(dsisr) ?  space_t::write : space_t::read,
	    ppc_is_kernel_mode(srr1) );

    except_return();
}

EXCDEF( unknown_handler )
{
    try_to_debug( "** unknown exception **", srr0, srr1, frame );
    except_return();
}

EXCDEF( sys_reset_handler )
{
    try_to_debug( "** system reset exception **", srr0, srr1, frame );
    except_return();
}

EXCDEF( machine_check_handler )
{
    try_to_debug( "** machine check exception **", srr0, srr1, frame );
    except_return();
}

EXCDEF( isi_handler )
{
    TRACEPOINT(except_isi_cnt);

    if( EXPECT_FALSE(srr0 == 0) ) 
    {
	if( ppc_is_kernel_mode(srr1) )
	    try_to_debug( "** kernel null pointer **", srr0, srr1, frame );
	else
	    try_to_debug( "** null pointer **", srr0, srr1, frame );
	except_return();
    }

    if( EXPECT_FALSE(ppc_is_kernel_mode(srr1)) )
    {
	if( srr0 < USER_AREA_END )
	    TRACEF( "kernel execution in user area: 0x%08x\n", srr0 );
	else if( (srr0 < KERNEL_AREA_START) || (srr0 > KERNEL_AREA_START) )
	    TRACEF( "kernel execution in data area: 0x%08x\n", srr0 );
    }
    else if( EXPECT_FALSE(srr0 >= USER_AREA_END) )
    {
	TRACEF( "user attempt to execute in kernel area: 0x%08x\n", srr0 );
	srr0 = 0;
    }

    space_t *space = get_current_tcb()->get_space();
    if( EXPECT_FALSE(space == NULL) )
	space = get_kernel_space();

    if( EXCEPT_IS_ISI_MISS(srr1) ) 
	if( EXPECT_TRUE(space->handle_hash_miss((addr_t)srr0)) ) 
	    except_return();

    space->handle_pagefault( (addr_t)srr0, (addr_t)srr0, 
	    space_t::execute, ppc_is_kernel_mode(srr1) );

    except_return();
}

EXCDEF( extern_int_handler )
{
    if( EXPECT_FALSE(ppc_is_kernel_mode(srr1)) ) {
	srr1 = processor_wake( srr1 );
	frame[ FRAME_IDX(KSTACK_SRR1) ] = srr1;
    }

    get_interrupt_ctrl()->handle_irq( 0 );

    except_return();
}

EXCDEF( alignment_handler )
{
    if( !send_exception_ipc( EXCEPT_ID(ALIGNMENT), srr1) )
	try_to_debug( "** alignment exception **", srr0, srr1, frame );
    except_return();
}

EXCDEF( program_handler )
{
    word_t instr;
    char *debug_msg;
    space_t *space;
    
    space = get_current_space();
    if( EXPECT_FALSE(space == NULL) )
	space = get_kernel_space();

    TRACEPOINT(except_prog_cnt);

    if( EXCEPT_IS_TRAP(srr1) )
    {
	switch( frame[ FRAME_IDX(KSTACK_R0) ] )
	{
	    case L4_TRAP_KDEBUG:
		instr = space->get_from_user( (addr_t)(srr0 + 4) );
		if( DEBUG_IS_MAGIC(instr) )
		    debug_msg = (char *)(srr0 + 8);
		else
		    debug_msg = "debug request";
		if( get_kip()->kdebug_entry )
		    try_to_debug( debug_msg, srr0, srr1, frame );
		frame[ FRAME_IDX(KSTACK_UIP) ] += 4;
		except_return();

	    case L4_TRAP_KPUTC:
#if defined(CONFIG_DEBUG)
		putc( (char)frame[ FRAME_IDX(KSTACK_R3) ] );
#endif
		frame[ FRAME_IDX(KSTACK_UIP) ] += 4;
		except_return();

	    case L4_TRAP_KGETC:
#if defined(CONFIG_DEBUG)
		frame[ FRAME_IDX(KSTACK_R3) ] = (word_t)getc();
#else
		frame[ FRAME_IDX(KSTACK_R3) ] = 0;
#endif
		frame[ FRAME_IDX(KSTACK_UIP) ] += 4;
		except_return();
	}

    }

    instr = space->get_from_user( (addr_t)srr0 );
    if( instr == KIP_EXCEPT_INSTR ) {
	frame[ FRAME_IDX(KSTACK_R3) ] = (word_t)space->get_kip_page_area().get_base();
	frame[ FRAME_IDX(KSTACK_R4) ] = get_kip()->api_version;
	frame[ FRAME_IDX(KSTACK_R5) ] = get_kip()->api_flags;
       	frame[ FRAME_IDX(KSTACK_R6) ] = get_kip()->get_kernel_descriptor()->kernel_id.get_raw();
	frame[ FRAME_IDX(KSTACK_UIP) ] += 4;
	except_return();
    }

    if( !send_exception_ipc( EXCEPT_ID(PROGRAM), srr1) )
	try_to_debug( "** program exception **", srr0, srr1, frame );
    except_return();
}

EXCDEF( fp_unavail_handler )
{
    tcb_t *current_tcb = get_current_tcb();
    current_tcb->resources.fpu_unavail_exception( current_tcb );

    except_return();
}

EXCDEF( decrementer_handler )
{
    extern word_t decrementer_interval;

    /* Don't go back to sleep if the thread was in power savings mode.
     * We will only see timer interrupts from user mode, or from the sleep
     * function in kernel mode.
     */
    if( EXPECT_FALSE(ppc_is_kernel_mode(srr1)) ) {
	srr1 = processor_wake( srr1 );
	frame[ FRAME_IDX(KSTACK_SRR1) ] = srr1;
    }

    TRACEPOINT(except_decr_cnt);

    ppc_set_dec( decrementer_interval );
    get_current_scheduler()->handle_timer_interrupt();

    except_return();
}

EXCDEF( trace_handler )
{
    /* Disable the single stepping and branch tracing. */
    srr1 = MSR_CLR(srr1, MSR_SE);
    srr1 = MSR_CLR(srr1, MSR_BE);
    frame[ FRAME_IDX(KSTACK_SRR1) ] = srr1;

    try_to_debug( "trap: single step / branch trace", srr0, srr1, frame );
    except_return();
}

EXCDEF( fp_assist_handler )
{
    if( !send_exception_ipc( EXCEPT_ID(FP_ASSIST), srr1) )
	try_to_debug( "** floating point assist exception **", 
		srr0, srr1, frame );
    except_return();
}


