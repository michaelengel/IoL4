/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  University of New South Wales
 *                
 * File path:     glue/v4-mips64/syscalls.S
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
 * $Id: syscalls.S,v 1.12 2003/05/01 14:56:45 sjw Exp $
 *                
 ********************************************************************/

#include INC_ARCH(asm.h)
#include INC_ARCH(regdef.h)
#include INC_GLUE(context.h)
#include INC_GLUE(syscalls.h)
#include <asmsyms.h>
#include <tcb_layout.h>

#define DECODE_SYSCALL_IPC		    \
1:;					    \
	.set push;			    \
	.set noreorder;			    \
	bnez	$1, 1f;			    \
	lui	ra, %hi(_mips64_l4sysipc_return);   \
	dli	t4, -4096;		    /* tcb mask */	    \
	and	t5, sp, t4;		    /* t5 = current tcb */  \
	ld	t5, OFS_TCB_UTCB(t5);	    /* t5 = current utcb */ \
	daddiu	ra, %lo(_mips64_l4sysipc_return);   \
	dsubu	sp, 8;			    \
flush_return:;				    \
	sd	s8, 0(sp);		    \
	sd	s0, 128(t5);		    \
	sd	s1, 136(t5);		    \
	sd	s2, 144(t5);		    \
	sd	s3, 152(t5);		    \
	sd	s4, 160(t5);		    \
	sd	s5, 168(t5);		    \
	sd	s6, 176(t5);		    \
	j	sys_ipc;		    \
	sd	s7, 184(t5);		    \
1:;	\
	lui	ra, %hi(_mips64_l4syscall_return);   \
	and	t4, $1, 1;		    \
	.set pop;

#define DECODE_SYSCALL(name, vector)	    \
	.set push;			    \
	.set noreorder;			    \
1:	beqz	t4, 1f;			    \
	and	t4, $1, (vector<<1);	    \
	j	sys_##name;		    \
	daddiu	ra, %lo(_mips64_l4syscall_return);   \
	.set pop;

#define SKIPDECODE_SYSCALL(name, vector)    \
	.set push;			    \
	.set noreorder;			    \
1:	beqz	t4, 1f;			    \
	and	t4, $1, (vector<<1);	    \
	li	AT, 2;			    \
	break;				    \
	.set pop;


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
	li	t2, 60;		\
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
BEGIN_PROC(__mips64_interrupt)
	mfc0    k1, CP0_CAUSE
	lui	k0, %hi(exception_handlers)
	andi    k1, k1, 0x7c
	dsll    k1, k1, 1
	add	k0, k0, k1
	ld      k0, %lo(exception_handlers)(k0)
	jr      k0
	nop
END_PROC(__mips64_interrupt)

	.set reorder
BEGIN_PROC(_mips64_l4syscall)
	START_SYSCALL_COUNTER
	/* Trashed registers:
	 *   t4, t5, t6, t7
	 * Calling Registers:
	 *   at : syscall number
	 *   a0, a1, a2, a3 : arguments 1 - 4
	 *   t0, t1, t2, t3 : arguments 5 - 8
	 * Returned Registers:
	 *   v0	    : returned 1
	 *   a0..a3 : returned 2 - 5
	 *   t0..t1 : returned 6 - 7
	 */
	.set noat
	mfc0	t7, CP0_STATUS		/* get STATUS register */
	lui	t5, %hi(K_STACK_BOTTOM)	/* Load kernel stack base address */
	move	t4, sp			/* Old stack in t4 */

	srl	t6, t7, 5		/* clear IE, EXL, ERL, KSU */
	sll	t6, t6, 5
	mtc0	t6, CP0_STATUS		/* Enter kernel mode */
	andi	t6, t7, ST_KSU		/* Isolate KSU bits */

	.set noreorder
	beq	t6, zero, 9f		/* Branch if from KERNEL mode */
	dmfc0	t6, CP0_EPC		/* Branch delay */
	.set reorder

	ld	sp, %lo(K_STACK_BOTTOM)(t5)	/* Load saved stack */

9:
	daddu	t6, t6, 4		/* Calc New EPC */
	dsubu	sp, sp, PT_SIZE		/* New stack pointer */
	sd	ra, PT_RA(sp)		/* Save RA */
	sd	t7, PT_STATUS(sp)	/* Save status */
	sd	t4, PT_SP(sp)		/* Save stack */
	sd	t6, PT_EPC(sp)		/* Save EPC */

	/* XXX decode must be in this order ! */
	DECODE_SYSCALL_IPC					/* 3 in, 1 out */
	DECODE_SYSCALL(thread_switch, SYSCALL_thread_switch)	/* 1 in, 0 out */
	DECODE_SYSCALL(thread_control, SYSCALL_thread_control)	/* 5 in, 1 out */
	DECODE_SYSCALL(exchange_registers, SYSCALL_exchange_registers)	/* 8 in, 7 out */
	DECODE_SYSCALL(schedule, SYSCALL_schedule)		/* 5 in, 2 out */
	DECODE_SYSCALL(unmap, SYSCALL_unmap)			/* 1 in, 0 out */
	DECODE_SYSCALL(space_control, SYSCALL_space_control)	/* 5 in, 2 out */
	SKIPDECODE_SYSCALL(processor_control,SYSCALL_processor_control)
	DECODE_SYSCALL(memory_control,SYSCALL_memory_control)   /* 3 in, 1 out */
	
	nop

1:
	li  AT, 2
	break
END_PROC(_mips64_l4syscall)

BEGIN_PROC(_mips64_l4sysipc_return)
	daddu	sp, 8
	dli	t4, -4096		    /* tcb mask */
	and	t5, sp, t4		    /* t5 = current tcb */
	ld	t5, OFS_TCB_UTCB(t5)	    /* t5 = current utcb */
	ld	s8, -8(sp)
	ld	s0, 128(t5)
	ld	s1, 136(t5)
	ld	s2, 144(t5)
	ld	s3, 152(t5)
	ld	s4, 160(t5)
	ld	s5, 168(t5)
	ld	s6, 176(t5)
	ld	s7, 184(t5)

_mips64_l4syscall_return:
	mfc0	t6, CP0_STATUS
	ld	t7, PT_SP(sp)		/* load stack */
	lbu	t4, STATUS_B(sp)	/* load status */
	ld	t5, PT_EPC(sp)		/* load epc */
	ld	ra, PT_RA(sp)		/* load ra */

	STOP_SYSCALL_COUNTER
	PRINT_SYSCALL_TIMES

	dmtc0	t5, CP0_EPC		/* restore EPC */ 
	move	sp, t7			/* restore stack */
	li	t7, CONFIG_MIPS64_STATUS_MASK
	and	t7, t7, t6		/* compute new status register */
	or	t7, t7, t4		/*            " "              */
	mtc0	t7, CP0_STATUS		/* new status value */

	eret

.set at
END_PROC(_mips64_l4sysipc_return)

	.set reorder
BEGIN_PROC(_mips64_restore_context)
	RESTORE_ALL
	eret
END_PROC(_mips64_restore_context)

	.set reorder
BEGIN_PROC(_mips64_break)
	.set	noreorder
//	dmfc0	k1, CP0_EPC
//	li	k0, L4_TRAP_GETUTCB
//
//	bne	AT, k0, other
//	daddu	k1, 4
//	dmtc0	k1, CP0_EPC
//
//	lui	k0, %hi(K_STACK_BOTTOM)		/* Load kernel stack base address */
//	ld	k1, %lo(K_STACK_BOTTOM)(k0)	/* Load saved stack */
//	dli	k0, -4096			/* tcb mask */
//	and	AT, k1, k0			/* AT = current tcb */
//	ld	v0, OFS_TCB_MYSELF_LOCAL(AT)	/* v0 = local id (utcb) */
//	eret
//	.set	reorder
//other:
	SAVE_ALL_INT

	move	a0, sp
	jal	mips_break

	RESTORE_ALL
	eret
END_PROC(_mips64_break)

	.set reorder
BEGIN_PROC(_mips64_illegal_ins)
	SAVE_ALL_INT

	move	a0, sp
	jal	mips_illegal

	RESTORE_ALL
	eret

END_PROC(_mips64_illegal_ins)

