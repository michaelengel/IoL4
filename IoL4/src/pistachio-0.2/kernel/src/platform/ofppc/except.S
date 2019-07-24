/****************************************************************************
 *
 * Copyright (C) 2002, Karlsruhe University
 *
 * File path:	platform/ofppc/except.S
 * Description:	Exception handler entry and exit points.
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
 * $Id: except.S,v 1.35 2003/04/28 08:34:56 joshua Exp $
 *
 ***************************************************************************/

/*
 *  WARNING: to coexist with Open Firmware, the exception code is linked
 *  within the kernel address space.  When the kernel installs the 
 *  exception handlers, it moves them to a new point in the address space,
 *  invalidating all symbols in this file.  So do not use absolute addresses 
 *  based on these symbols, unless you account for the relocation.
 */

#include INC_ARCH(phys.h)
#include INC_ARCH(ppc_registers.h)
#include INC_ARCH(msr.h)
#include INC_ARCH(bat.h)

#include INC_PLAT(frame.h)

#include INC_GLUE(offsets.h)
#include INC_GLUE(bat.h)
#include INC_GLUE(exception.h)
#include INC_GLUE(abi.h)

#include <tcb_layout.h>
#include <asmsyms.h>

/* priorities (highest first):
 *  1. system reset
 *  2. machine check
 *  3. synchronous, precise
 *  4. imprecise
 *  5. external interrupt
 *  6. decrementer
 */

/* For function calls, the nonvolatile registers are r1, r14 through r31,
 * f14 through f31.  They belong to the calling function, and the called
 * function must save these registers.
 * The volatile registers are r0, r3 through r12, f0 through f13, CTR, XER.
 * The calling function must save these registers prior to invoking the 
 * called function.
 *
 * r1 is the stack pointer.
 * r3 and r4 are function return values.
 * r3 through r10 are function parameters.
 * r11 and r12 may be modified during function linkage.
 *
 * Simple exception entry (assuming that we interrupt the kernel):
 * 0. Enable address translation.
 * 1. Stash r1 & r12 in SPRG2 & SPRG3.
 * 2. Point r1 at an appropriate stack.
 * 3. Using r1, save r0-r12, CTR, XER, CR, LR in the stack.
 * 4. Set up exception parameters:
 *    r3 = SRR0, r4 = SRR1, ...
 *    Try to quickly save SRR0 & SRR1 to prepare for nested exceptions.
 *    And quickly save the spilled registers SPRG2 & SPRG3.
 * 5. Jump to C code.
 * 
 * Exception exit:
 * 1. Restore r0-r12, CTR, XER, CR, LR from the stack.
 * 2. rfi
 */

#define UTCB_MR_OFFSET(n)	(OFS_UTCB_MR + 4*(n))

#define GRAB_SYM(dst,sym)		\
	lis	dst, sym @ha ;		\
	la	dst, sym @l(dst) ;

#define SPILL_CR	0
#define SPILL_R3	4
#define SPILL_R4	8

#define SPRG_R12	SPRG_TMP1
#define SPRG_R1		SPRG_TMP0
#define SPRG_KSP	SPRG_CURRENT_TCB
#define SPRG_SPILL	SPRG_CPU

#define EXCEPT_STACK							\
	mtsprg	SPRG_R12, %r12 ;	/* preserve r12			*/\
	mtsprg	SPRG_R1, %r1 ;		/* preserve the stack		*/\
	mfcr	%r1 ;			/* grab the cr			*/\
	mfsprg	%r12, SPRG_SPILL ;	/* grab the spill location	*/\
	stw	%r1, SPILL_CR(%r12) ;	/* temp spill cr		*/\
	stw	%r3, SPILL_R3(%r12) ;	/* temp spill r3		*/\
	mfspr	%r3, SPR_SRR0 ;		/* put srr0 in the 1st argument	*/\
	stw	%r4, SPILL_R4(%r12) ;	/* temp spill r4		*/\
	mfspr	%r4, SPR_SRR1 ;		/* grab srr1			*/\
	\
	andi.	%r4, %r4, (MSR_PR_USER << MSR_PR) ; /* What priv level? */\
	mfspr	%r4, SPR_SRR1 ;		/* put srr1 in the 2nd argument	*/\
	bne 0f ;		/* Jump to kernel stack allocation.	*/\
	mfsprg	%r1, SPRG_R1 ;	/* Grab the interrupted kernel stack.	*/\
	addi	%r1, %r1, -EXC_STACK_SIZE; /* allocate stack space.	*/\
	b 1f ;			/* Skip kernel stack allocation.	*/\
	\
	0: \
	mfsprg	%r1, SPRG_KSP ;		/* Switch to the kernel stack.	*/\
	addi	%r1, %r1, KTCB_SIZE-EXC_STACK_SIZE; /* allocate stack space */\
	\
	1: 	/* Safe to switch to virtual addressing mode.		*/


#define EXCEPT_DEBUG_STACK						\
	mtsprg	SPRG_R12, %r12 ;	/* preserve r12			*/\
	mtsprg	SPRG_R1, %r1 ;		/* preserve the stack		*/\
	mfcr	%r1 ;			/* grab the cr			*/\
	mfsprg	%r12, SPRG_SPILL ;	/* grab the spill location	*/\
	stw	%r1, SPILL_CR(%r12) ;	/* temp spill cr		*/\
	stw	%r3, SPILL_R3(%r12) ;	/* temp spill r3		*/\
	mfspr	%r3, SPR_SRR0 ;		/* put srr0 in the 1st argument	*/\
	stw	%r4, SPILL_R4(%r12) ;	/* temp spill r4		*/\
	mfspr	%r4, SPR_SRR1 ;		/* grab srr1			*/\
	lis	%r1, _init_stack_top@ha ;	/* get debug stack	*/\
	la	%r1, _init_stack_top@l(%r1) ;	/* get debug stack	*/\
	addi	%r1, %r1, -EXC_STACK_SIZE; /* allocate stack space.	*/


#define EXCEPT_PROLOG							\
	li	%r12, MSR_KERNEL ;					\
	mtmsr	%r12 ;							\
	isync ;								\
	stw	%r13, OF_R13(%r1) ;	/* spill r13			*/\
	stw	%r14, OF_R14(%r1) ;	/* spill r14			*/\
	stw	%r15, OF_R15(%r1) ;	/* spill r15			*/\
	mfctr	%r13 ;

#if !defined(CONFIG_PPC_MULTIWORD_INSTR)
#define PROLOG_GP_SPILL		\
	stw	%r16, OF_R16(%r1) ;	/* spill r16			*/\
	stw	%r17, OF_R17(%r1) ;	/* spill r17			*/\
	stw	%r18, OF_R18(%r1) ;	/* spill r18			*/\
	stw	%r19, OF_R19(%r1) ;	/* spill r19			*/\
	stw	%r20, OF_R20(%r1) ;	/* spill r20			*/\
	stw	%r21, OF_R21(%r1) ;	/* spill r21			*/\
	stw	%r22, OF_R22(%r1) ;	/* spill r22			*/\
	stw	%r23, OF_R23(%r1) ;	/* spill r23			*/\
	stw	%r24, OF_R24(%r1) ;	/* spill r24			*/\
	stw	%r25, OF_R25(%r1) ;	/* spill r25			*/\
	stw	%r26, OF_R26(%r1) ;	/* spill r26			*/\
	stw	%r27, OF_R27(%r1) ;	/* spill r27			*/\
	stw	%r28, OF_R28(%r1) ;	/* spill r28			*/\
	stw	%r29, OF_R29(%r1) ;	/* spill r29			*/\
	stw	%r30, OF_R30(%r1) ;	/* spill r30			*/\
	stw	%r31, OF_R31(%r1) ;	/* spill r31			*/
#else
#define PROLOG_GP_SPILL		\
	stmw	%r16, OF_R16(%r1) ;	/* spill r16 through r31	*/
#endif

#define EXCEPT_SPILL							\
	stw	%r3, OF_SRR0(%r1) ;	/* spill srr0			*/\
	stw	%r4, OF_SRR1(%r1) ;	/* spill srr1			*/\
	stw	%r0, OF_R0(%r1) ;	/* spill r0			*/\
	stw	%r5, OF_R5(%r1) ;	/* spill r5			*/\
	stw	%r6, OF_R6(%r1) ;	/* spill r6			*/\
	mfsprg	%r5, SPRG_SPILL ;	/* Grab the spill location.	*/\
	lis	%r6, KERNEL_OFFSET@ha ;	/* Get the kernel offset.	*/\
	add	%r5, %r5, %r6 ;		/* Calc va of spill location.	*/\
	lwz	%r0, SPILL_CR(%r5) ;	/* Grab cr.			*/\
	lwz	%r6, SPILL_R3(%r5) ;	/* Grab r3.			*/\
	stw	%r7, OF_R7(%r1) ;	/* spill r7			*/\
	stw	%r8, OF_R8(%r1) ;	/* spill r8			*/\
	stw	%r9, OF_R9(%r1) ;	/* spill r9			*/\
	stw	%r10, OF_R10(%r1) ;	/* spill r10			*/\
	stw	%r11, OF_R11(%r1) ;	/* spill r11			*/\
	lwz	%r11, SPILL_R4(%r5) ;	/* grab r4			*/\
	mfsprg	%r12, SPRG_R12 ;	/* grab r12			*/\
	stw	%r12, OF_R12(%r1) ;	/* spill r12			*/\
	stw	%r13, OF_CTR(%r1) ;	/* spill ctr			*/\
	mfsprg	%r10, SPRG_R1 ;		/* grab the interrupted stack	*/\
	stw	%r10, OF_R1(%r1) ;	/* spill r1 (sp)		*/\
	stw	%r2, OF_R2(%r1) ;	/* spill r2			*/\
	mfspr	%r7, SPR_XER ;		/* buffer xer			*/\
	mfspr	%r8, SPR_LR ;		/* buffer lr			*/\
	stw	%r0, OF_CR(%r1) ;	/* spill cr			*/\
	stw	%r11, OF_R4(%r1) ;	/* spill r4			*/\
	stw	%r6, OF_R3(%r1) ;	/* spill r3			*/\
	stw	%r7, OF_XER(%r1) ;	/* spill xer			*/\
	stw	%r8, OF_LR(%r1) ;	/* spill lr			*/\
	PROLOG_GP_SPILL							\
	addi	%r5, %r1, 8 ;		/* put spill base in the 3rd arg */

#if !defined(CONFIG_PPC_MULTIWORD_INSTR)
#define EPILOG_GP_RESTORE						\
	lwz	%r3, OF_R3(%r1) ;	/* restore r3			*/\
	lwz	%r4, OF_R4(%r1) ;	/* restore r4			*/\
	lwz	%r5, OF_R5(%r1) ;	/* restore r5			*/\
	lwz	%r6, OF_R6(%r1) ;	/* restore r6			*/\
	lwz	%r7, OF_R7(%r1) ;	/* restore r7			*/\
	lwz	%r8, OF_R8(%r1) ;	/* restore r8			*/\
	lwz	%r9, OF_R9(%r1) ;	/* restore r9			*/\
	lwz	%r10, OF_R10(%r1) ;	/* restore r10			*/\
	lwz	%r11, OF_R11(%r1) ;	/* restore r11			*/\
	lwz	%r12, OF_R12(%r1) ;	/* restore r12			*/\
	lwz	%r13, OF_R13(%r1) ;	/* restore r13			*/\
	lwz	%r14, OF_R14(%r1) ;	/* restore r14			*/\
	lwz	%r15, OF_R15(%r1) ;	/* restore r15			*/\
	lwz	%r16, OF_R16(%r1) ;	/* restore r16			*/\
	lwz	%r17, OF_R17(%r1) ;	/* restore r17			*/\
	lwz	%r18, OF_R18(%r1) ;	/* restore r18			*/\
	lwz	%r19, OF_R19(%r1) ;	/* restore r19			*/\
	lwz	%r20, OF_R20(%r1) ;	/* restore r20			*/\
	lwz	%r21, OF_R21(%r1) ;	/* restore r21			*/\
	lwz	%r22, OF_R22(%r1) ;	/* restore r22			*/\
	lwz	%r23, OF_R23(%r1) ;	/* restore r23			*/\
	lwz	%r24, OF_R24(%r1) ;	/* restore r24			*/\
	lwz	%r25, OF_R25(%r1) ;	/* restore r25			*/\
	lwz	%r26, OF_R26(%r1) ;	/* restore r26			*/\
	lwz	%r27, OF_R27(%r1) ;	/* restore r27			*/\
	lwz	%r28, OF_R28(%r1) ;	/* restore r28			*/\
	lwz	%r29, OF_R29(%r1) ;	/* restore r29			*/\
	lwz	%r30, OF_R30(%r1) ;	/* restore r30			*/\
	lwz	%r31, OF_R31(%r1) ;	/* restore r31			*/
#else
#define EPILOG_GP_RESTORE	\
	lmw	%r3, OF_R3(%r1) ;	/* restore r3 through r31	*/
#endif


#define EXCEPT_EPILOG							\
	lwz	%r3, OF_SRR0(%r1) ;	/* buffer srr0			*/\
	lwz	%r4, OF_SRR1(%r1) ;	/* buffer srr1			*/\
	lwz	%r5, OF_CTR(%r1) ;	/* buffer ctr			*/\
	lwz	%r6, OF_XER(%r1) ;	/* buffer xer			*/\
	mtspr	SPR_CTR, %r5 ;		/* restore ctr			*/\
	mtspr	SPR_XER, %r6 ;		/* restore xer			*/\
	lwz	%r7, OF_CR(%r1) ;	/* buffer cr			*/\
	lwz	%r8, OF_LR(%r1) ;	/* buffer lr			*/\
	mtspr	SPR_SRR0, %r3 ;		/* restore srr0			*/\
	mtspr	SPR_SRR1, %r4 ;		/* restore srr1			*/\
	mtcr	%r7 ;			/* restore cr			*/\
	mtspr	SPR_LR, %r8 ;		/* restore lr			*/\
	lwz	%r0, OF_R0(%r1) ;	/* restore r0			*/\
	EPILOG_GP_RESTORE						\
	lwz	%r2, OF_R2(%r1) ;	/* restore r2			*/\
	lwz	%r1, OF_R1(%r1) ;	/* restore r1			*/\
	rfi ;


#define CALL_UNKNOWN_HANDLER						\
	lis	%r14, except_unknown_handler@ha ;			\
	la	%r14, except_unknown_handler@l(%r14) ;			\
	lis	%r15, _except_return@ha	;				\
	la	%r15, _except_return@l(%r15) ;				\
	mtctr	%r15 ;							\
	bctr ;

#define CALL_HANDLER( handler )						\
	lis	%r14, handler @ha ;					\
	la	%r14, handler @l(%r14) ;				\
	lis	%r15, _except_return@ha	;				\
	la	%r15, _except_return@l(%r15) ;				\
	mtctr	%r15 ;							\
	bctr ;

/* The exception return path.
 * Most exceptions will share this return path.  Hopefully
 * reduces cache pollution.
 */
	.section ".text"
	.align 2
	.global _except_return
	.global _except_return_shortcircuit
_except_return:
	EXCEPT_SPILL
	mfspr	%r6, SPR_DAR
	mfspr	%r7, SPR_DSISR
	mtctr	%r14	/* Prepare to jump to the C handler. */
	bctrl		/* Call C code. */
_except_return_shortcircuit:
	EXCEPT_EPILOG

/* system reset and machine check exceptions:
 * - have highest priority and can occur while other exceptions are being
 *   processed.
 * - never delayed; therefore, if two of these exceptions occur in immediate
 *   succession, the state info saved by the first exception may be
 *   overwritten by the second exception.
 * - these are context-synchronizing if they are recoverable 
 *   (MSR[RI] is copied from MSR to SRR1 if the exception does not cause
 *   loss of state).
 * - for system reset exceptions, SRR0 addresses the instruction that 
 *   would have attempted to execute next.
 * - for machine check, SRR0 holds either holds an instruction that would
 *   have completed or some instruction following it that would have 
 *   completed.
 * - all instructions prior to SRR0 appear to have completed with respect
 *   to the executing processor.
 */
	.section ".except", "ax"
	.align	2
	.global	_start_except
	.global _except_system_reset
	. = EXCEPT_OFFSET_SYSTEM_RESET
_except_system_reset:
_start_except:
	EXCEPT_STACK
	EXCEPT_PROLOG
	CALL_HANDLER( except_sys_reset_handler )


	.section ".except"
	.align	2
	. = EXCEPT_OFFSET_MACHINE_CHECK
_except_machine_check:
	EXCEPT_STACK
	EXCEPT_PROLOG
	CALL_HANDLER( except_machine_check_handler )


/* DSI exception:
 * - read the DSISR register to determine the reason for the DSI.
 *   The DSISR register is SPR 18, and readable by mfspr.
 * - the effective address of the addressed memory element is stored in DAR.
 *   Not necessarily the actual memory addressed.
 */
	.section ".except"
	.globl	_except_dsi
	.type	_except_dsi,@function
	. = EXCEPT_OFFSET_DSI
_except_dsi:
	EXCEPT_STACK
	EXCEPT_PROLOG
	CALL_HANDLER( except_dsi_handler )
.Ldsi:
	.size	_except_dsi,.Ldsi-_except_dsi


/* ISI exception:
 * - SRR0 contains the address of the instruction that the processor would
 *   have executed next.  If a branch instruction, then the branch target.
 * - SRR1 contains bits that indicate the reason for the exception.
 */
	.section ".except"
	.align	2
	. = EXCEPT_OFFSET_ISI
_except_isi:
	EXCEPT_STACK
	EXCEPT_PROLOG
	CALL_HANDLER( except_isi_handler )


/* external interrupt:
 * - context synchronizing
 * - SRR0 addresses the instruction that would have been executed.
 * - disables further interrupts (clears MSR[EE])
 */
	.section ".except"
	.align	2
	. = EXCEPT_OFFSET_EXTERNAL_INT
_except_ipi:

#if !defined(CONFIG_SMP)
	/* Give some sentinel values.  They will eventually be overwritten
	 * when the kernel installs the external interrupt handler.
	 */
	.long	0xdeadbeaf
	.long	0xdeadbeaf
	.long	0xdeadbeaf

#else
	/* Define SMP cpu initialization code.  It will eventually be
	 * overwritten when the kernel installs the external interrupt handler.
	 */
	lis	%r13, KERNEL_OFFSET@ha	/* Get the kernel offset. */

	/* Include the BAT initialization code.
	 * It expects r13 to hold the KERNEL_OFFSET.
	 */
#include "bat.S"

	/* Use the boot stack for cpu intialization. */
	GRAB_SYM( %r1, _init_stack_top )
	addi	%r1, %r1, -8		/* Allocate some space on the stack. */

	/* Enter C code, and finish cpu initialization. */
	GRAB_SYM( %r10, MSR_KERNEL_INIT ) /* Grab the kernel's init msr. */
	GRAB_SYM( %r11, l4_powerpc_cpu_start )	/* Load the C code entry point. */
	mtsrr1	%r10			/* Prepare to install the kernel msr. */
	mtsrr0	%r11			/* Prepare to jump to the C code. */
	rfi				/* Enter the kernel. */
#endif	/* CONFIG_SMP */


/* alignment exception:
 * - reference DSISR to determine the source of the exception.
 * - DAR contains the effective address of the data access.
 */
	.section ".except"
	.align	2
	. = EXCEPT_OFFSET_ALIGNMENT
_except_alignment:
	EXCEPT_STACK
	EXCEPT_PROLOG
	CALL_HANDLER( except_alignment_handler )


/* program exception:
 *
 */
	.section ".except"
	.align	2
	. = EXCEPT_OFFSET_PROGRAM
_except_program:
	EXCEPT_STACK
	EXCEPT_PROLOG
	CALL_HANDLER( except_program_handler )


/* floating point unavailable exception:
 * - imprecise
 * - SRR0 contains the address of either the instruction that caused
 *   the exception, or some instruction following that instruction.
 * - the instruction addressed by SRR0 may be partially executed.
 */
	.section ".except"
	.align	2
	. = EXCEPT_OFFSET_FP_UNAVAILABLE
_except_fp_unavailable:
	EXCEPT_STACK
	EXCEPT_PROLOG
	CALL_HANDLER( except_fp_unavail_handler )


/* decrementer exception:
 * - context synchronizing
 * - SRR0 addresses the instruction that would have been executed.
 * - disables further interrupts (clears MSR[EE])
 */
	.section ".except"
	.align	2
	. = EXCEPT_OFFSET_DECREMENTER
_except_decrementer:
	EXCEPT_STACK
	EXCEPT_PROLOG
	CALL_HANDLER( except_decrementer_handler )


	.section ".except"
	.align	2
	. = EXCEPT_OFFSET_RESERVED1
_except_reserved1:
	EXCEPT_STACK
	EXCEPT_PROLOG
	CALL_UNKNOWN_HANDLER


	.section ".except"
	.align	2
	. = EXCEPT_OFFSET_RESERVED2
_except_reserved2:
	EXCEPT_STACK
	EXCEPT_PROLOG
	CALL_UNKNOWN_HANDLER


/* system call exception:
 *  1. Switch to kernel stack.
 *  2. Is this an L4 syscall?  
 *  3. If yes, install kernel MSR into srr1 and rfi.
 *  4. Preserve some state, and jump to syscall emulation.
 */
	.section ".except"
	.align	2
	. = EXCEPT_OFFSET_SYSCALL
_except_syscall:
	/* Security check.  Is the user IP in the kernel address space?
	 * Preserve SVR4 calling convention registers until we know whether
	 * this is an L4 system call.
	 */
	mfsrr0	%r11
	lis	%r12, KERNEL_OFFSET@ha
	cmplw	%cr0, %r11, %r12
	blt	0f

	/* The IP is in the kernel space.  Allocate stack space.
	 * And we can start trashing non-preserved registers.
	 */
	mfsrr1	%r28		/* Prepare to save srr1. */
	li	%r12, MSR_KERNEL

	mr	%r29, %r1	/* Preserve the user stack. */
	mfsprg	%r1, SPRG_KSP	/* Switch to kernel stack. Important. */
	addi	%r1, %r1, KTCB_SIZE-SC_STACK_SIZE /* Allocate stack space. */

	/* Promote to supervisor mode and rfi.
	 */
	mtsrr1	%r12		/* Install the kernel msr. */
	rfi


	/* Emulated system call.  We convert into a fast-path IPC.  If the
	 * user's UTCB is missing a destination tid for emulated system calls,
	 * then convert into a trap sent to user space.  We assume that
	 * the caller expects the semantics of a function call, so we take
	 * some liberties in overwriting volatile register state,
	 * even if we convert to a user space trap.  But we protect register r0
	 * since many system calls use it as a function lookup index.  So
	 * we have two volatile registers at our disposal: r11 and r12.
	 */

	/* It is possible to get a page fault while spilling register contents.
	 * So we must extract srr0 and srr1 prior to causing page faults.
	 */
0:
	li	%r12, MSR_KERNEL	/* Load the kernel's MSR. */
	mtmsr	%r12
	isync				/* Enable virtual addressing. */

	/* Get exceptions state.  srr0 is already in r11. */
	mfsrr1	%r12			/* Get srr1. */

	/* Switch stacks. */
	mtsprg	SPRG_TMP0, %r1		/* Preserve user stack. */
	mfsprg	%r1, SPRG_KSP		/* Switch to kernel stack. */
	addi	%r1, %r1, KTCB_SIZE-EXC_STACK_SIZE /* Allocate stack space. */

	/* Spill lots of user register state.
	 */
#if defined(CONFIG_PPC_MULTIWORD_INSTR)
        stmw    %r13, OF_R13(%r1)	/* Spill r13 through r31.	*/
#else
	stw	%r13, OF_R13(%r1)	/* Spill r13.			*/
	stw	%r14, OF_R14(%r1)	/* Spill r14.			*/
	stw	%r15, OF_R15(%r1)	/* Spill r15.			*/
	PROLOG_GP_SPILL			/* Spills r16 through r31.	*/
#endif

	/* Spill more user register state.
	 */
	mflr	%r16			/* Grab lr.			*/
	mfcr	%r17			/* Grab cr.			*/
	stw	%r2,  OF_R2(%r1)	/* Spill r2.			*/
	stw	%r16, OF_LR(%r1)	/* Spill lr.			*/
	stw	%r17, OF_CR(%r1)	/* Spill cr.			*/

	/* Find the UTCB. */
	extlwi	%r20, %r1, 32-KTCB_BITS, 0	/* Get the current TCB. */
	lwz	%r21, OFS_TCB_UTCB (%r20)	/* Get the UTCB.	*/

	/* Put user's IP, SP, FLAGS in the UTCB as part of the IPC message.
	 */
	mfsprg	%r22, SPRG_TMP0		/* Get the user's stack.	*/
	stw	%r11, UTCB_MR_OFFSET(SC_EXC_MR_UIP) (%r21)	/* Save srr0. */
	stw	%r22, UTCB_MR_OFFSET(SC_EXC_MR_USP) (%r21)	/* Save USP.  */
	andi.	%r12, %r12, MSR_USER_MASK	/* Apply user flags mask to srr1. */
	stw	%r12, UTCB_MR_OFFSET(SC_EXC_MR_UFLAGS) (%r21)	/* Save UFLAGS. */

	/* Generate fake user state, so that the fast path IPC
	 * returns to our syscall cleanup code.
	 */
	stw	%r1, OF_R1(%r1)		/* Save our stack as the user stack. */
	lis	%r24, _sc_emul_return@ha	/* Construct a return point. */
	la	%r24, _sc_emul_return@l (%r24)	/* Construct a return point. */
	stw	%r24, OF_SRR0(%r1)	/* Set the return point. */
	lis	%r25, MSR_KERNEL	/* Construct the return flags. */
	stw	%r25, OF_SRR1(%r1)	/* Use the kernel flags for the return point. */

	/* Construct the IPC.
	 * TODO: handle local TIDs for the exception handler TID.
	 */
	li	IPC_ABI_TIMEOUTS, 0			/* We never time-out. */
	lis	IPC_ABI_MR0, SC_EXC_TAG@ha		/* Construct the message tag. */
	la	IPC_ABI_MR0, SC_EXC_TAG@l (IPC_ABI_MR0)	/* Construct the message tag. */
	lwz	IPC_ABI_FROM_TID, OFS_TCB_MYSELF_GLOBAL (%r20)	/* Get my TID. */
	lwz	IPC_ABI_TO_TID, OFS_UTCB_EXCEPTION_HANDLER (%r21) /* Get the exception handler TID. */


	/* Jump to the fast path setup code.
	 */
	lis	%r18, _ipc_start@ha
	la	%r18, _ipc_start@l(%r18)
	mtctr	%r18
	bctrl


	.section ".text"
	.align	2
	.globl	_sc_emul_return
_sc_emul_return:
	// TODO: implement return path.
1:	b 1b



/* trace exception:
 */
	.section ".except"
	.align	2
	. = EXCEPT_OFFSET_TRACE
_except_trace:
	EXCEPT_DEBUG_STACK
	EXCEPT_PROLOG
	CALL_HANDLER( except_trace_handler )


/* floating point assist exception:
 */
	.section ".except"
	.align	2
	. = EXCEPT_OFFSET_FP_ASSIST
_except_fp_assist:
	EXCEPT_STACK
	EXCEPT_PROLOG
	CALL_HANDLER( except_fp_assist_handler )


/* Performance monitor exception.
 */
	.section ".except"
	.align	2
	. = EXCEPT_OFFSET_PERFMON
_except_perfmon:
	EXCEPT_STACK
	EXCEPT_PROLOG
	CALL_UNKNOWN_HANDLER


/* System management interrupt.
 */
	.section ".except"
	.align	2
	. = EXCEPT_OFFSET_SYS_MANAGE
_except_sys_manage:
	EXCEPT_STACK
	EXCEPT_PROLOG
	CALL_UNKNOWN_HANDLER


	.section ".except"
	.align	2
	. = EXCEPT_OFFSET_RESERVED3
_except_reserved3:
	EXCEPT_STACK
	EXCEPT_PROLOG
	CALL_UNKNOWN_HANDLER


	.section ".except"
	.align	2
	. = EXCEPT_OFFSET_RESERVED4
_except_reserved4:
	EXCEPT_STACK
	EXCEPT_PROLOG
	CALL_UNKNOWN_HANDLER



/* Thermal assist exception.
 */
	.section ".except"
	.align	2
	. = EXCEPT_OFFSET_THERMAL
_except_sys_thermal:
	EXCEPT_STACK
	EXCEPT_PROLOG
	CALL_UNKNOWN_HANDLER


	.global _end_except
_end_except:


/*****************************************************************************
 * The external interrupt handler, to be copied into place after
 * ipi initialization.
 */
	.section ".init"
	.global	_except_extern_int
	.global _except_extern_int_end
	.balign	4
_except_extern_int:
	EXCEPT_STACK
	EXCEPT_PROLOG
	CALL_HANDLER( except_extern_int_handler )
_except_extern_int_end:

