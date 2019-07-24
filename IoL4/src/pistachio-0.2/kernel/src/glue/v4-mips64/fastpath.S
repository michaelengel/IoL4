/*********************************************************************
 *                
 * Copyright (C) 2002,  University of New South Wales
 *                
 * File path:     glue/v4-mips64/fastpath.S
 * Created:       20/08/2002 by Carl van Schaik
 * Description:   Kernel entry points for syscalls
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
 * $Id: fastpath.S,v 1.4 2003/05/02 07:16:22 sjw Exp $
 *
 ********************************************************************/

#if CONFIG_IPC_FASTPATH

#include INC_ARCH(asm.h)
#include INC_ARCH(regdef.h)
#include INC_GLUE(context.h)
#include INC_GLUE(syscalls.h)
#include <asmsyms.h>
#include <tcb_layout.h>


#if 0
#define	START_SYSCALL_COUNTER	    \
	mfc0	k0, CP0_COUNT;	    \
	nop;			    \
	nop;			    \
	mtc0	k0, CP0_ERROREPC

#define STOP_SYSCALL_COUNTER	    \
	nop;	\
	nop;	\
	nop;	\
	nop;	\
	nop;	\
	nop;	\
	mfc0	t2, CP0_COUNT;	    \
	mfc0	t3, CP0_ERROREPC;   \
	nop;	\
	nop;	\
	dsub	AT, t2, t3;	    \
	dsll	AT, AT, 1;

#if CONFIG_PLAT_ERPCN01
#define d0 k0
#define d1 k1
#define PRINT_SYSCALL_TIMES	\
	li	t2, 28;		\
1:;	\
	dsrlv	t3, AT, t2;	\
	andi	t3, 0xf;	\
	sub	t3, 10;		\
	bgez	t3, 2f;		\
	add	t3, '0'+10;	\
	b 3f;			\
2:;	\
	add	t3, 'a';	\
3:;	\
	dla     d0, propane_uart;   \
	ld	d0, 0(d0);	\
4:;	\
	lw      d1,8(d0);	\
	andi    d1,d1,0xf;	\
	sltiu   d1,d1,13;	\
	beqz    d1,4b;		\
	sw      t3,4(d0);	\
	sub	t2, 4;		\
	bgez	t2, 1b;		\
	\
	li	t3, '\r';	\
4:;	\
	lw      d1,8(d0);	\
	andi    d1,d1,0xf;	\
	sltiu   d1,d1,13;	\
	beqz    d1,4b;		\
	sw      t3,4(d0);	\
	li	t3, '\n';	\
4:;	\
	lw      d1,8(d0);	\
	andi    d1,d1,0xf;	\
	sltiu   d1,d1,13;	\
	beqz    d1,4b;		\
	sw      t3,4(d0)

#endif

#if CONFIG_PLAT_U4600
#define PRINT_SYSCALL_TIMES	\
	SAVE_ALL_INT		\
	\
	li	t2, 28;		\
1:;	\
	dsrlv	a0, AT, t2;	\
	andi	a0, 0xf;	\
	sub	a0, 10;		\
	bgez	a0, 2f;		\
	add	a0, '0'+10;	\
	b 3f;			\
2:;	\
	add	a0, 'a';	\
3:;	\
	jal	_Z11putc_serialc;\
	sub	t2, 4;		\
	bgez	t2, 1b;		\
	\
	li	a0, '\n';	\
	jal	_Z11putc_serialc;\
	\
	RESTORE_ALL
#endif

#else
#define	START_SYSCALL_COUNTER
#define STOP_SYSCALL_COUNTER
#define PRINT_SYSCALL_TIMES
#endif

	.set noat
	.set noreorder
BEGIN_PROC(__mips64_interrupt_fp)
	START_SYSCALL_COUNTER
	mfc0    k1, CP0_CAUSE			/* get interrupt cause */
	li	k0, 8<<2
	andi    k1, k1, 0x7c
	bne	k0, k1, other_exception		/* If not SYSCALL, goto normal path */
	mfc0	k0, CP0_STATUS			/* get STATUS register */
	bnez	$1, _goto_mips64_l4syscall	/* if not IPC goto normal syscall entry */
	lui     t5, %hi(K_STACK_BOTTOM)		/* Load kernel stack base address */

	j	_mips64_fastpath
	move	t7, k0

_goto_mips64_l4syscall:
	j _mips64_l4syscall
	nop
	
other_exception:
	dsll    k1, k1, 1
	lui	k0, %hi(exception_handlers)
	add	k0, k0, k1
	ld      k0, %lo(exception_handlers)(k0)
	jr      k0
	nop
END_PROC(__mips64_interrupt_fp)

#define IPC_MAX_REGS    8

/* Pistachio defines */
#define IPC_MR0_PROPAGATED (1 << 12)

#define to_tid	    a0
#define from_tid    a1 
#define timeout	    a2
#define current	    a3
#define to_tcb	    t0
#define from_tcb    t1
#define to_state    t2
#define dest_partner	t3
#define current_utcb	t4
#define current_global	v0

#define tmp0	    t5
#define tmp1	    t6
#define tmp2	    t7
#define tmp3	    t8
#define tmp4	    t9
#define tmp5	    v1

#define mr0	    s0
#define mr1	    s1
#define mr2	    s2
#define mr3	    s3
#define mr4	    s4
#define mr5	    s5
#define mr6	    s6
#define mr7	    s7

	.set reorder
BEGIN_PROC(_mips64_fastpath)
	.set noat
	srl	t6, t7, 5		/* clear IE, EXL, ERL, KSU */
	sll	t6, t6, 5
	move	t4, sp
	mtc0	t6, CP0_STATUS		/* Enter kernel mode */

	ld	sp, %lo(K_STACK_BOTTOM)(t5)	/* Load saved stack */

	dmfc0   t6, CP0_EPC

	dsubu	sp, sp, PT_SIZE		/* New stack pointer */

	sd	ra, PT_RA(sp)		/* Save RA */
	daddu	t6, t6, 4		/* Calc New EPC */

	sd	t7, PT_STATUS(sp)	/* Save status */
	sd	t4, PT_SP(sp)		/* Save stack */
	sd	t6, PT_EPC(sp)		/* Save EPC */

/*** START FASTPATH ***/
send_path:
	.set	noreorder
	/* Look for a nil from-tid / to-tid.			   TEST0 */
	beqz	to_tid, _mips64_slowpath			/* TEST0 */
	andi	tmp0, mr0, (0x3f << 6) | IPC_MR0_PROPAGATED	/* TEST1 | num_typed is bits 6 - 11 (6 bits) in mr0 */
	beqz	from_tid, _mips64_slowpath			/* TEST0 */
	dli	tmp4, -4096		/* tcb mask */		/* CALC2 */

	/* We don't do typed words or propagation.		   TEST1 */
	bnez	tmp0, _mips64_slowpath				/* TEST1 */

	and	current, sp, tmp4	/* get current tcb	   CALC2 */

	/* We only support in-register IPC (for the moment)	   TEST2 */
#if (IPC_MAX_REGS != 8)
# error "expecting max IPC Registers = 8"
#endif
	andi	tmp1, mr0, 0x38					/* TEST2 */
	bnez	tmp1, _mips64_slowpath				/* TEST2 */

	ld	tmp4, OFS_TCB_SEND_HEAD(current)		/* TEST8 | Require send_head to be empty */
	dsrl    tmp3, to_tid, 32    /* calculate to_tcb */	/* CALC1 */
	dsll    tmp3, 12					/* CALC1 */
	li      to_tcb, 0x4					/* CALC1 */
	dsll    to_tcb, 60					/* CALC1 */
	bnez	tmp4, _mips64_slowpath				/* TEST8 */
	or      to_tcb, tmp3, to_tcb				/* CALC1 */
	ld	tmp1, OFS_TCB_MYSELF_GLOBAL(to_tcb)		/* TEST4 */

	/* Check that the receive timeout is infinite		   TEST3    (lower 16 bits == 0) */
	andi	tmp2, timeout, 0xffff				/* TEST3 */
	bnez	tmp2, _mips64_slowpath				/* TEST3 */

	lw	to_state, OFS_TCB_THREAD_STATE(to_tcb)		/* TEST5 */
        /*
	 * Check partner ID	CALC1, TEST4
	 */

	/* Check to_tcb->get_global_id == to_tid		   TEST4 */
	bne	tmp1, to_tid, _mips64_slowpath			/* TEST4 | May be an error, may be just a receive etc. */

	/*
	 * Check partner is waiting				   TEST5
	 */

	/* is_waiting */
#if 0
#if (TSTATE_WAITING_FOREVER != 0xffffffff)
# error "expecting thread_state_t::waiting_forever to be -1"
#endif
#endif
	dli	tmp5, -1					/* TEST5 */
	bne	to_state, tmp5, _mips64_slowpath		/* TEST5 */
	
	ld	dest_partner, OFS_TCB_PARTNER(to_tcb)

	ld	current_global, OFS_TCB_MYSELF_GLOBAL(current)	/* TEST6 */

	/* IS_WAITING_GLOBAL:   (tcb->get_partner() == current->get_global_id()) || tcb->get_partner().is_anythread() */
	/* is_anythread() */					/* TEST6 */
//	dli	tmp5, -1	    /* anythread == -1 */	/* TEST6 / TEST7 */

	beq	tmp5, dest_partner, 1f				/* TEST6 */
	lui	ra, %hi(ipc_finish)				/* RA CALC */

        /* tcb->get_partner() == current->get_global_id() */	/* TEST6 */
	dsub	tmp1, current_global, dest_partner		/* TEST6 */
	bnez	tmp1, _mips64_slowpath				/* TEST6 */
1:
	addi	ra, %lo(ipc_finish)				/* RA CALC */
	/*
	 * Check that receive phase blocks			   TEST7
	 */
	bne	tmp5, from_tid, check_other_tcb			/* TEST7 */
        ld	tmp4, OFS_TCB_SPACE(to_tcb)	/* get: to_tcb->space */

        /* FALLTHRU */
continue_ipc:
	/* This is the point of no return --- after this we cannot go to the slow path */

	/*
	 * Set thread state to waiting				   STORE1
	 */
	sw	tmp5, OFS_TCB_THREAD_STATE(current)		/* STORE1 */
	sd	from_tid, OFS_TCB_PARTNER(current)

	/* At this point, we have set up the sending thread's TCB state.  We now setup the
	 * stack so that when we are next switched to we do the right thing (set state to running
	 * and return partner) --- this only happens in the generic send case.
	 */
	ld      tmp2, OFS_TCB_STACK(to_tcb)	/* get to_tcb->stack */
	lw	tmp1, SPACE_ASID_OFFSET(tmp4)	/* get: space->asid (assume no asid management) */

/* mips switch_to */
	sd	s8, -8(sp)
	dsubu	sp,sp,MIPS64_SWITCH_STACK_SIZE+8

	sd	ra,32(sp)			/* RA CALC */

	sd	sp, OFS_TCB_STACK(current)  /* Store current stack in old_stack */

	dmtc0	tmp1, CP0_ENTRYHI	/* Set new ASID */
	move	sp, tmp2		/* Install the new stack */
	dsll	tmp4, tmp4, 32
	dmtc0	tmp4, CP0_CONTEXT	/* Save current Page Table */
	.set at
	or	tmp2, 4096-1
	daddiu	tmp2, 1
	sd	tmp2, K_STACK_BOTTOM

	dsubu   sp, tmp2, PT_SIZE	/* get exception stack */
	ld	s8, -8(sp)		/* restore s8 */

	/* Mark self as runnable */
	dli	tmp0, TSTATE_RUNNING
	sw	tmp0, OFS_TCB_THREAD_STATE(to_tcb)

	/* Clean up mr0 (clear receive flags) */
	and	mr0, ~(0xe << 12)

	/* Set return value to sender's global ID (already in v0)*/

	mfc0	t6, CP0_STATUS
	ld	t7, PT_SP(sp)		/* load stack */
	lbu	t4, STATUS_B(sp)	/* load status */
	ld	t5, PT_EPC(sp)		/* load epc */
	ld	ra, PT_RA(sp)		/* load ra */

	.set	reorder

	STOP_SYSCALL_COUNTER
	PRINT_SYSCALL_TIMES

	dmtc0	t5, CP0_EPC		/* restore EPC */ 
	move	sp, t7			/* restore stack */
	li	t7, CONFIG_MIPS64_STATUS_MASK
	and	t7, t7, t6		/* compute new status register */
	or	t7, t7, t4		/*            " "              */
	mtc0	t7, CP0_STATUS		/* new status value */

	eret

ipc_finish:	/* Return Address */
	dli	tmp0, -4096			/* tcb mask */
	and	current, sp, tmp0		/* t5 = current tcb */

	daddu	sp, current, 4096-PT_SIZE-8

        /* first the state */
	dli	tmp0, TSTATE_RUNNING
	sw	tmp0, OFS_TCB_THREAD_STATE(current)

	ld	v0, OFS_TCB_PARTNER(current)

	j	_mips64_l4sysipc_return

check_other_tcb:
	.set	noreorder
        dsrl    tmp0, from_tid, 32                /* calculate from_tcb */
	dsll    tmp0, 12
	li      from_tcb, 0x4
	dsll    from_tcb, 60
	or      from_tcb, tmp0, from_tcb

	/* Check global ID */
	ld	tmp0, OFS_TCB_MYSELF_GLOBAL(from_tcb)
	li	tmp1, TSTATE_POLLING
	bne	tmp0, from_tid, _mips64_slowpath

	/*
	 * Check if the thread is polling us --- if so, go to slow path
	 */

	/* is_polling() */
	lw	tmp0, OFS_TCB_THREAD_STATE(from_tcb)
	bne	tmp1, tmp0, continue_ipc	/* from_tcb isn't polling */
	
	/* partner == current->global_id */
	ld	tmp1, OFS_TCB_PARTNER(from_tcb)
	beq	tmp1, current_global, _mips64_slowpath	/* If the other thread is polling us, goto the slowpath */

	/* partner == current->local_id */
	ld	tmp2, OFS_TCB_MYSELF_LOCAL(current)
	bne	tmp1, tmp2, continue_ipc
	nop

	j	_mips64_slowpath
	nop

END_PROC(_mips64_fastpath)

	.set reorder
BEGIN_PROC(_mips64_slowpath)
	lui	ra, %hi(_mips64_l4sysipc_return)
	dli	t4, -4096		    /* tcb mask */
	and	t5, sp, t4		    /* t5 = current tcb */
	ld	t5, OFS_TCB_UTCB(t5)	    /* t5 = current utcb */
	daddiu	ra, %lo(_mips64_l4sysipc_return)
	dsubu	sp, 8
	sd	s8, 0(sp)
	sd	s0, 128(t5)
	sd	s1, 136(t5)
	sd	s2, 144(t5)
	sd	s3, 152(t5)
	sd	s4, 160(t5)
	sd	s5, 168(t5)
	sd	s6, 176(t5)
	sd	s7, 184(t5)
	j	sys_ipc
END_PROC(_mips64_slowpath)

#endif /* CONFIG_IPC_FASTPATH */

