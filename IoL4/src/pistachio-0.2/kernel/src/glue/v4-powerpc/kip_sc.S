/****************************************************************************
 *
 * Copyright (C) 2002, Karlsruhe University
 *
 * File path:	glue/v4-powerpc/kip_sc.S
 * Description:	Defines the user space syscall entry points.  
 *		They are prefixes for the actual system calls.  The linker is
 *		responsible for aligning these entry points with their
 *		respective system calls.
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
 * $Id: kip_sc.S,v 1.24 2003/04/28 08:34:17 joshua Exp $
 *
 ***************************************************************************/

#include INC_PLAT(frame.h)
#include INC_GLUE(abi.h)

#include <tcb_layout.h>
#include <asmsyms.h>


#define SYSCALL_ENTRY()							\
	/* Promote privilege level and switch to the kernel stack.	*/\
	sc ;								\
	/* Save state. */						\
	mflr	%r27 ;							\
	stw	%r29, SC_OF_R1(%r1) ;	/* Preserve the user sp.	*/\
	stw	%r30, SC_OF_R30(%r1) ;	/* Preserve r30.		*/\
	stw	%r31, SC_OF_R31(%r1) ;	/* Preserve r31.		*/\
	stw	%r28, SC_OF_SRR1(%r1) ;	/* Preserve srr1.		*/\
	stw	%r27, SC_OF_SRR0(%r1) ;	/* Preserve return address.	*/\
	stw	%r2,  SC_OF_R2(%r1) ;	/* Preserve r2 (utcb pointer).	*/

#define PUSH(r,off)					\
	stw	r, off(%r1) ;				\
	addi	r, %r1, off

#define PUSH2(dst,src,off)				\
	stw	src, off(%r1) ;				\
	addi	dst, %r1, off

#define GRAB_SYM(dst,sym)				\
	lis	dst, sym @ha ;				\
	la	dst, sym @l(dst) ;

#define KIP_SC_ENTRY(sect_name,label)					\
	.section sect_name ;						\
	.align	2 ;							\
	.globl	_sc_##label ;						\
	_sc_##label :


#define NILTHREAD	0
#define LOCAL_TID_MASK	0x3f

#define GLOBALIZE_TID(tid)								\
	cmplwi	cr1, tid, NILTHREAD ;		/* Compare TID to the nil TID. */	\
	andi.	%r11, tid, LOCAL_TID_MASK ;	/* Is the TID a local TID?     */	\
	/* Negate NIL result and AND with local TID result. */				\
	crandc	4*cr0+eq, 4*cr0+eq, 4*cr1+eq ;						\
	bne	0f ;									\
	lwz	tid, (OFS_UTCB_MY_GLOBAL_ID - OFS_UTCB_MR) (tid) ;			\
0:

#define GLOBALIZE_ANY_TID(tid)								\
	cmplwi	cr1, tid, NILTHREAD ;		/* Compare TID to the nil TID. */	\
	andi.	%r11, tid, LOCAL_TID_MASK ;	/* Is the TID a local TID?     */	\
	/* Negate NIL result and AND with local TID result. */				\
	crandc	4*cr1+eq, 4*cr0+eq, 4*cr1+eq ;						\
	/* OR a bunch of 1's with the TID, to see if it becomes the any TID. */		\
	/* Invert the result to see whether it equals 0. */				\
	li	%r11, LOCAL_TID_MASK & 0xffffffff ; 					\
	nor.	%r11, %r11, tid ;							\
	/* Negate the any thread result, and AND with the local TID result. */		\
	crandc	4*cr0+eq, 4*cr1+eq, 4*cr0+eq ;						\
	bne	0f ;									\
	lwz	tid, (OFS_UTCB_MY_GLOBAL_ID - OFS_UTCB_MR) (tid) ;			\
0:

/*****************************************************************************/

/* Define the IPC paths. */
#include "fastpath.S"

/*****************************************************************************/

KIP_SC_ENTRY(".sc_schedule", schedule)
	GLOBALIZE_TID( %r3 )
	SYSCALL_ENTRY()
#if defined(CONFIG_SYSV_ABI)
	addi	%r1, %r1, -8
	PUSH( %r3, 8 )	/* threadid_t dest_tid	*/
#endif

/*****************************************************************************/

KIP_SC_ENTRY(".sc_xchg_registers", xchg_registers)

	/* Convert the dest TID into a global TID.  But we must also
	 * tell the ExchangeRegisters system call whether the input TID
	 * was local, and thus we generate an extra parameter, is_local,
	 * for the ExchangeRegisters system call.
	 */
	GLOBALIZE_TID( EXREG_ABI_DEST )
	mfcr	EXREG_ABI_IS_LOCAL    /* Copy the condition register into the is_local parameter. */
	extrwi	EXREG_ABI_IS_LOCAL, EXREG_ABI_IS_LOCAL, 1, 2    /* Isolate cr0[eq] as bit 0. */

	/* Does the control register mention the pager TID?
	 * If so, we convert it into a global TID.
	 */
	andi.	%r11, EXREG_ABI_CONTROL, 1 << EXREG_ABI_CONTROL_P    /* Check the p-bit of the control register. */
	beq	1f			/* If zero, skip the globalize. */
	GLOBALIZE_TID( EXREG_ABI_PAGER )
1:

	SYSCALL_ENTRY()

#if defined(CONFIG_SYSV_ABI)
	addi	%r1, %r1, -8
	PUSH( %r3, 8 )	/* threadid_t dest_tid	*/
	PUSH( %r9, 12 )	/* threadid_t pager_tid	*/
#endif

/*****************************************************************************/

KIP_SC_ENTRY(".sc_unmap", unmap)
	SYSCALL_ENTRY()

/*****************************************************************************/

KIP_SC_ENTRY(".sc_memory_ctrl", memory_ctrl)
	SYSCALL_ENTRY()

/*****************************************************************************/

KIP_SC_ENTRY(".sc_processor_ctrl", processor_ctrl)
	SYSCALL_ENTRY()

/*****************************************************************************/

KIP_SC_ENTRY(".sc_thread_ctrl", thread_ctrl)
	GLOBALIZE_TID( TCTRL_ABI_DEST )
	GLOBALIZE_TID( TCTRL_ABI_SPACE )
	GLOBALIZE_TID( TCTRL_ABI_SCHEDULER )
	GLOBALIZE_TID( TCTRL_ABI_PAGER )

	SYSCALL_ENTRY()

#if defined(CONFIG_SYSV_ABI)
	addi	%r1, %r1, -16
	PUSH( TCTRL_ABI_DEST, 8 )	/* threadid_t dest_tid		*/
	PUSH( TCTRL_ABI_SPACE, 12 )	/* threadid_t space_tid		*/
	PUSH( TCTRL_ABI_SCHEDULER, 16 )	/* threadid_t scheduler_tid	*/
	PUSH( TCTRL_ABI_PAGER, 20 )	/* threadid_t pager_tid		*/
#endif

/*****************************************************************************/

KIP_SC_ENTRY(".sc_space_ctrl", space_ctrl)
	GLOBALIZE_TID( SPACE_ABI_SPACE )
	GLOBALIZE_ANY_TID( SPACE_ABI_REDIRECTOR )

	SYSCALL_ENTRY()

#if defined(CONFIG_SYSV_ABI)
	addi	%r1, %r1, -16
	PUSH( SPACE_ABI_SPACE, 8 )	/* threadid_t space_tid		*/
	PUSH( SPACE_ABI_KIP, 12 )	/* fpage_t kip_area		*/
	PUSH( SPACE_ABI_UTCB, 16 )	/* fpage_t utcb_area		*/
	PUSH( SPACE_ABI_REDIRECTOR, 20 )	/* threadid_t redirector_tid	*/
#endif

/*****************************************************************************/

	.section ".sc_system_clock"
	.align 2
	.global	_sc_system_clock
_sc_system_clock:
	/* Important: we never entered the kernel!! 
	 * We are still executing in user mode.
	 */
	mftbu	CLOCK_ABI_UPPER
	mftbl	CLOCK_ABI_LOWER
	mftbu	%r11
	cmpl	%cr0, CLOCK_ABI_UPPER, %r11
	bne	_sc_system_clock
	blr

/*****************************************************************************/

KIP_SC_ENTRY(".sc_perf", perf)
	SYSCALL_ENTRY()
	lwz	%r11, SC_OF_SRR0(%r1)
	lwz	%r12, SC_OF_SRR1(%r1)
	lwz	%r31, SC_OF_R31(%r1)
	lwz	%r30, SC_OF_R30(%r1)
	lwz	%r2,  SC_OF_R2 (%r1)
	lwz	%r1,  SC_OF_R1 (%r1)
	mtsrr0	%r11
	mtsrr1	%r12
	rfi

/*****************************************************************************/

KIP_SC_ENTRY(".sc_thread_switch", thread_switch)
	GLOBALIZE_TID( TSWITCH_ABI_DEST )

	SYSCALL_ENTRY()

#if defined(CONFIG_SYSV_ABI)
	addi	%r1, %r1, -8
	PUSH( TSWITCH_ABI_DEST, 8 )	/* threadid_t dest	*/
#endif

