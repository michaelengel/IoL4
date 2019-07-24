/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     glue/v4-ia64/user.S
 * Description:   User level syscall stubs
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
 * $Id: user.S,v 1.37.2.2 2003/06/03 18:24:43 skoglund Exp $
 *                
 ********************************************************************/
#include INC_ARCH(asm.h)
#include INC_ARCH(ar.h)
#include INC_GLUE(registers.h)
#include INC_GLUE(config.h)
#include INC_GLUE(context_handle.h)
#include INC_API(procdesc.h)
#include <asmsyms.h>
#include <tcb_layout.h>

#if defined(CONFIG_SMP)
#define IFSMP(x...)	x
#else
#define IFSMP(x...)
#endif


/*
 * Define in order to store MRs into the UTCB on the IPC fast path.
 * Useful for debugging purposes.
 */
//#define STORE_MRS_ON_FASTPATH


/**
 * Wrapper for doing enter KDB within syscall bindings.  Control is
 * transferred to a regular kernel executable page where the KDB
 * entry is actually performed.
 *
 * @param str	string to print upon KDB entry
 */
#define SYSCALL_KDB(str)		;\
	.text				;\
8:					;\
{ .mlx					;\
	break.m	0x0			;\
	movl	r0 = 9f ;;		;\
}					;\
	br.sptk.few b7			;\
					;\
	.previous			;\
	.rodata				;\
9:	stringz str			;\
	.previous			;\
					;\
	;;				;\
9:	mov	r31 = ip ;;		;\
	add	r31 = 9f-9b, r31 ;;	;\
	mov	b7 = r31		;\
	movl	r31 = 8b ;;		;\
	mov	b6 = r31 ;;		;\
	br.sptk.few b6			;\
9:

#define CHANGE_TO_KERNEL_BACKING_STORE(extra_stack_space)	\
	/* Switch to kernel stack pointers */			;\
	mov	r_sp = sp					;\
	mov	r_rsc = ar.rsc					;\
								;\
	mov	sp = r_KERNEL_SP				;\
	mov	ar.rsc = 0					;\
	;;							;\
	mov	r_bsp = ar.bspstore				;\
	mov	r_rnat = ar.rnat				;\
	add	num_dirty_ptr = -KTCB_SIZE, sp			;\
	add	kernel_bspstore = -IA64_SP_TO_RSE_OFFSET, sp	;\
	;;							;\
	mov	ar.bspstore = kernel_bspstore			;\
	add	num_dirty_ptr = OFS_TCB_ARCH_NUM_DIRTY, num_dirty_ptr ;\
	;;							;\
	mov	r_KERNEL_STACK_COUNTER = 2			;\
	mov	num_dirty = ar.bsp				;\
	;;							;\
	sub	num_dirty = num_dirty, kernel_bspstore		;\
	add	sp = -16-SIZEOF_SWITCH_CONTEXT-(extra_stack_space), sp;\
	mov	ar.rsc = 3					;\
	;;							;\
	st8	[num_dirty_ptr] = num_dirty

#define STORE_CURRENT_CONTEXT(syscall, num_inputs, num_outputs, \
			      extra_stack_space)		\
	/* Store context to local stack frame */		;\
	alloc	loc2 = ar.pfs,num_inputs,4,num_outputs,0	;\
	add	loc0 = 16+8+(extra_stack_space), sp		;\
	add	loc1 = 16+16+(extra_stack_space), sp		;\
1:	mov	loc3 = ip					;\
	;;							;\
	st8	[loc0] = num_dirty, 16	/* num_dirty */		;\
	st8	[loc1] = r_rsc, 16	/* rsc */		;\
	add	loc3 = sys_##syscall##_return-1b, loc3		;\
	;;							;\
	st8	[loc0] = gp, 16		/* gp */		;\
	st8	[loc1] = loc2, 16	/* pfs */		;\
	;;							;\
	st8	[loc0] = r0, 16		/* cfm */		;\
	st8	[loc1] = loc3, 16	/* ip */		;\
	;;							;\
	st8	[loc0] = r_bsp, 16	/* bspstore */		;\
	st8	[loc1] = r_rnat, 16	/* rnat */		;\
	mov	loc2 = ar.unat					;\
	mov	loc3 = pr					;\
	;;							;\
	st8	[loc0] = loc2, 16	/* unat */		;\
	st8	[loc1] = loc3, 16	/* pr */		;\
	mov	loc2 = psr					;\
	;;							;\
	st8	[loc0] = loc2, 16	/* psr */		;\
	st8	[loc1] = r_sp		/* sp */		;\
	mov	loc2 = rp					;\
	;;							;\
	st8	[loc0] = loc2		/* rp */		;\
	;;

#define INVOKE_SYSCALL(name)					\
	/* Invoke system call */				;\
	movl	loc0 = TPR_INT_ENABLE_SOME			;\
	;;							;\
	mov	cr.tpr = loc0					;\
	movl	gp = __gp					;\
	;;							;\
	srlz.d							;\
	br.call.sptk.many rp = sys_##name			;\
								;\
	.globl	sys_##name##_return				;\
sys_##name##_return:						;\
	mov	loc0 = TPR_INT_ENABLE_ALL			;\
	;;							;\
	mov	cr.tpr = loc0					;\
	;;							;\
	srlz.d

#define INVOKE_SYSCALL_1(name, a0)				\
	mov	out0 = a0					;\
	INVOKE_SYSCALL(name)

#define INVOKE_SYSCALL_2(name, a0, a1)				\
	mov	out1 = a1					;\
	INVOKE_SYSCALL_1(name, a0)

#define INVOKE_SYSCALL_3(name, a0, a1, a2)			\
	mov	out2 = a2					;\
	INVOKE_SYSCALL_2(name, a0, a1)

#define INVOKE_SYSCALL_4(name, a0, a1, a2, a3)			\
	mov	out3 = a3					;\
	INVOKE_SYSCALL_3(name, a0, a1, a2)

#define INVOKE_SYSCALL_5(name, a0, a1, a2, a3, a4)		\
	mov	out4 = a4					;\
	INVOKE_SYSCALL_4(name, a0, a1, a2, a3)

#define INVOKE_SYSCALL_6(name, a0, a1, a2, a3, a4, a5)		\
	mov	out5 = a5					;\
	INVOKE_SYSCALL_5(name, a0, a1, a2, a3, a4)

#define INVOKE_SYSCALL_7(name, a0, a1, a2, a3, a4, a5, a6)	\
	mov	out6 = a6					;\
	INVOKE_SYSCALL_6(name, a0, a1, a2, a3, a4, a5)

#define INVOKE_SYSCALL_8(name, a0, a1, a2, a3, a4, a5, a6, a7)	\
	mov	out7 = a7					;\
	INVOKE_SYSCALL_7(name, a0, a1, a2, a3, a4, a5, a6)

#define RESTORE_CONTEXT(extra_stack_space)			\
	/* Restore context from local stack frame */		;\
	add	loc0 = 16+8+(extra_stack_space), sp		;\
	add	loc1 = 16+16+(extra_stack_space), sp		;\
	;;							;\
	ld8	num_dirty = [loc0], 16	/* num_dirty */		;\
	ld8	r_rsc = [loc1], 16	/* rsc */		;\
	;;							;\
	ld8	gp = [loc0], 32		/* gp */		;\
	ld8	r_pfs = [loc1], 32	/* pfs */		;\
	;;							;\
	ld8	r_bsp = [loc0], 16	/* bspstore */		;\
	ld8	r_rnat = [loc1], 16	/* rnat */		;\
	;;							;\
	ld8	loc2 = [loc0], 16	/* unat */		;\
	ld8	loc3 = [loc1], 16	/* pr */		;\
	;;							;\
	mov	ar.unat = loc2					;\
	mov	pr = loc3, -1					;\
	ld8	loc2 = [loc0], 16	/* psr */		;\
	ld8	r_sp = [loc1]		/* sp */		;\
	;;							;\
	ld8	loc3 = [loc0]		/* rp */		;\
	;;							;\
	mov	rp = loc3					;\
	mov	psr.l = loc2					;\
	;;							;\
	srlz.d							;\
	;;

#define LOAD_DIRTY_USER_REGS()					\
	/* Load dirty user registers off the kernel stack */	;\
	shl	num_dirty = num_dirty, 16			;\
	;;							;\
	alloc	r31 = ar.pfs,0,0,0,0				;\
	mov	ar.rsc = num_dirty				;\
	;;							;\
	loadrs							;\
	mov	r_KERNEL_STACK_COUNTER = 0			;\
	;;

#define SWITCH_TO_USER_BACKING_STORE_AND_RETURN()		\
	/* Change register backing store and return */		;\
	mov	ar.bspstore = r_bsp				;\
	mov	sp = r_sp					;\
	;;							;\
	mov	ar.rnat = r_rnat				;\
	mov	ar.pfs = r_pfs					;\
	mov	ar.rsc = r_rsc					;\
	br.ret.sptk.many rp


	.section .user.syscall, "ax", "progbits"

#if !defined(CONFIG_IPC_FASTPATH)

	//
	// Regular IPC path
	//

BEG_PROC (user_ipc)

	.regstk 8,0,0,0

to			= r14
fromspecifier		= r15
timeouts		= r16
from			= r9
mr0			= r32
mr1			= r33
mr2			= r34
mr3			= r35
mr4			= r36
mr5			= r37
mr6			= r38
mr7			= r39

r_sp			= r30
r_bsp			= r29
r_rsc			= r28
r_rnat			= r8
r_pfs			= r27
kernel_bspstore		= r26
num_dirty		= r25
ptr_ip			= r24
ptr_sp			= r23
cur_tcb			= r22
mr_ptr1			= r21
mr_ptr2			= r20
mr_ptr3			= r19
mr_ptr4			= r18
num_dirty_ptr		= r17

	// Convert local IDs into global ones
	cmp.ne	p10,p0 = to, r0			// ! is_nilthread (to)
	cmp.ne	p11,p0 = fromspecifier, r0	// ! is_nilthread (fromspec)

	and	r8 = 0x3f, to
	and	r9 = 0x3f, fromspecifier
	shr	r10 = fromspecifier, 6
	;;
(p10)	cmp.eq	p10,p0 = r8, r0			// is_local (to)
(p11)	cmp.eq	p11,p0 = r9, r0			// is_local (fromspec)
	;;
(p11)	cmp.ne	p11,p0 = -1, r10		// ! is_anylocal (fromspec)
	;;
(p10)	ld8	to = [to]
(p11)	ld8	fromspecifier = [fromspecifier]

	flushrs
	cmp.eq	p6,p0 = r0, r0		// p6 = true
	epc
	;;

	// Validate NaT bits in arguments
	cmp.eq.and p6,p0 = r14, r14
	cmp.eq.and p6,p0 = r15, r15
	cmp.eq.and p6,p0 = r16, r16
	cmp.eq	p7,p0 = r0, r0		// p7 = true
	;;
(p6)	cmp.ne	p7,p0 = r0, r0		// p7 = false if no args are NaTed
	;;
(p7)	br.cond.spnt.few 1f


	CHANGE_TO_KERNEL_BACKING_STORE (0)

	// Store MRs in UTCB
	mov	mr_ptr1 = r_LOCAL_ID
	;;
	add	mr_ptr4 = UTCB_MR_OFFSET+24, mr_ptr1
	add	mr_ptr3 = UTCB_MR_OFFSET+16, mr_ptr1
	add	mr_ptr2 = UTCB_MR_OFFSET+8,  mr_ptr1
	add	mr_ptr1 = UTCB_MR_OFFSET,    mr_ptr1
	;;
	st8	[mr_ptr1] = mr0, 32
	st8	[mr_ptr2] = mr1, 32
	st8	[mr_ptr3] = mr2, 32
	st8	[mr_ptr4] = mr3, 32
	;;
	st8	[mr_ptr1] = mr4
	st8	[mr_ptr2] = mr5
	st8	[mr_ptr3] = mr6
	st8	[mr_ptr4] = mr7

	STORE_CURRENT_CONTEXT (ipc, 0, 3, 0)
	
	INVOKE_SYSCALL_3 (ipc, to, fromspecifier, timeouts)

	RESTORE_CONTEXT (0)
	LOAD_DIRTY_USER_REGS ()

	// Store MRs in UTCB
	mov	mr_ptr1 = r_LOCAL_ID
	;;
	alloc	r14 = ar.pfs,8,0,0,0
	add	mr_ptr4 = UTCB_MR_OFFSET+24, mr_ptr1
	add	mr_ptr3 = UTCB_MR_OFFSET+16, mr_ptr1
	add	mr_ptr2 = UTCB_MR_OFFSET+8,  mr_ptr1
	add	mr_ptr1 = UTCB_MR_OFFSET,    mr_ptr1
	;;
	ld8	mr0 = [mr_ptr1], 32
	ld8	mr1 = [mr_ptr2], 32
	ld8	mr2 = [mr_ptr3], 32
	ld8	mr3 = [mr_ptr4], 32
	;;
	ld8	mr4 = [mr_ptr1]
	ld8	mr5 = [mr_ptr2]
	ld8	mr6 = [mr_ptr3]
	ld8	mr7 = [mr_ptr4]

	SWITCH_TO_USER_BACKING_STORE_AND_RETURN ()

	// One or more NaTed arguments.  Return to user mode, read
	// all args to cause exception and try again.
1:	mov	r18 = ip ;;
	add	r18 = 2f-1b, r18 ;;
	mov	b6 = r18 ;;
	br.ret.sptk.few b6 ;;
2:	mov	b6 = r14 ;;
	mov	b6 = r15 ;;
	mov	b6 = r16 ;;
	br.call.sptk.few b6 = user_ipc

END_PROC (user_ipc)


#else /* CONFIG_IPC_FASTPATH */

	//
	// Fast path IPC
	//

BEG_PROC (user_ipc)

	.regstk 8,0,0,0

to			= r14
fromspecifier		= r15
timeouts		= r16
from			= r9
mr0			= r32
mr1			= r33
mr2			= r34
mr3			= r35
mr4			= r36
mr5			= r37
mr6			= r38
mr7			= r39

src			= r17
dst			= r18
fromtcb			= r19
myself			= r9		// Same as FROM parameter
tmp1			= r20
tmp2			= r21
tmp3			= r22
tmp4			= r23
tmp5			= r4
tmp6			= r5
tmp7			= r6
tmp8			= r7
tmp9			= r13

kern_sp1		= r24
kern_sp2		= r25
user_sp			= r26

snd_state		= r31
snd_partner		= r30
snd_send_head		= r29
snd_stack		= r8
snd_space		= r19
snd_cpu			= r8
rcv_state		= r28
rcv_partner		= r27
rcv_stack		= r10
rcv_space		= r11
rcv_cpu			= r10
frm_cpu			= r11

Pfast			= p6
Pslow			= p7
Pow			= p8		// openwait
Pcw			= p9		// closedwait
Pcopy			= p10

	.pred.rel "mutex", p6,p7
	.pred.rel "mutex", p8,p9

	// Convert local IDs into global ones
	cmp.ne	p10,p0 = to, r0			// ! is_nilthread (to)
	cmp.ne	p11,p0 = fromspecifier, r0	// ! is_nilthread (fromspec)

	and	r8 = 0x3f, to
	and	r9 = 0x3f, fromspecifier
	shr	r10 = fromspecifier, 6
	;;
(p10)	cmp.eq	p10,p0 = r8, r0			// is_local (to)
(p11)	cmp.eq	p11,p0 = r9, r0			// is_local (fromspec)
	;;
(p11)	cmp.ne	p11,p0 = -1, r10		// ! is_anylocal (fromspec)
	;;
(p10)	ld8	to = [to]
(p11)	ld8	fromspecifier = [fromspecifier]

	flushrs
	cmp.eq	p6,p0 = r0, r0		// p6 = true
	epc
	;;
	// Validate NaT bits in arguments
	cmp.eq.and p6,p0 = r14, r14
	cmp.eq.and p6,p0 = r15, r15
	cmp.eq.and p6,p0 = r16, r16
	cmp.eq	p7,p0 = r0, r0		// p7 = true
	;;
(p6)	cmp.ne	p7,p0 = r0, r0		// p7 = false if no args are NaTed
	;;
(p7)	br.cond.spnt.few 1f

#if defined(STORE_MRS_ON_FASTPATH)
	// Store MRs in UTCB
	mov	tmp1 = r_LOCAL_ID
	;;
	add	tmp4 = UTCB_MR_OFFSET+24, tmp1
	add	tmp3 = UTCB_MR_OFFSET+16, tmp1
	add	tmp2 = UTCB_MR_OFFSET+8,  tmp1
	add	tmp1 = UTCB_MR_OFFSET,    tmp1
	;;
	st8	[tmp1] = mr0, 32
	st8	[tmp2] = mr1, 32
	st8	[tmp3] = mr2, 32
	st8	[tmp4] = mr3, 32
	;;
	st8	[tmp1] = mr4
	st8	[tmp2] = mr5
	st8	[tmp3] = mr6
	st8	[tmp4] = mr7
#endif

	// Calculate TCB locations for src and dst
	mov	kern_sp1 = r_KERNEL_SP
	and	tmp2 = 0x3f, fromspecifier
	cmp.eq	Pow,Pcw = -1, fromspecifier // is_anythread (from)
	;;
	cmp.eq.and.orcm Pow,Pcw = tmp2, r0 // is_local (from) (impl. anylocal)
	add	src = -8, kern_sp1
	add	kern_sp2 = -SIZEOF_SWITCH_CONTEXT+8, kern_sp1
	add	kern_sp1 = -SIZEOF_SWITCH_CONTEXT+16, kern_sp1
	mov	tmp1 = KTCB_REGION_ID
	shr	dst = to, 32
	shr	fromtcb = fromspecifier, 32
	;;
	dep	src = 0, src, 0, KTCB_BITSIZE
	dep.z	dst = dst, KTCB_BITSIZE, VALID_THREADNO_BITS
	dep.z	fromtcb = fromtcb, KTCB_BITSIZE, VALID_THREADNO_BITS
	;;
	dep	dst = tmp1, dst, 61, 3
	dep	fromtcb = tmp1, fromtcb, 61, 3

	extr	tmp2 = timeouts, 0, 16
	extr	tmp3 = mr0, 6, 10
	and	tmp4 = (7 << 3), mr0		// msgtag.u (bits 3..5)

	// We can do special optimization if we have a call
	.pred.rel "mutex", p8, p11
(Pcw)	cmp.ne	p11,p0 = to, fromspecifier
(Pow)	cmp.ne	p11,p0 = r0, r0

	add	snd_state =	OFS_TCB_THREAD_STATE, src
	add	snd_partner =	OFS_TCB_PARTNER, src
	add	snd_send_head = OFS_TCB_SEND_HEAD, src
	add	r2 =		OFS_TCB_RESOURCE_BITS, src
IFSMP(	add	snd_cpu =	OFS_TCB_CPU, src)
	;;
	add	rcv_state =	OFS_TCB_THREAD_STATE, dst
	add	rcv_partner =	OFS_TCB_PARTNER, dst
	add	r3 =		OFS_TCB_RESOURCE_BITS, dst
IFSMP(	add	rcv_cpu =	OFS_TCB_CPU, dst)
	add	tmp9 = (KTCB_SIZE - SIZEOF_EXCEPTION_CONTEXT + \
		        EXC_CONTEXT_IPSR_OFFSET), dst

(p11)	add	tmp6 =		OFS_TCB_THREAD_STATE, fromtcb
(p11)	add	tmp7 =		OFS_TCB_PARTNER, fromtcb
IFSMP((p11)add	frm_cpu =	OFS_TCB_CPU, fromtcb)

	cmp.ne	Pfast,Pslow = 0, to		// to != nil
	;;
	cmp.ne.and.orcm Pfast,Pslow = 0, fromspecifier // FromSpecifier != nil
	cmp.eq.and.orcm Pfast,Pslow = 0, tmp2	// RcvTimeout == 0
	cmp.eq.and.orcm Pfast,Pslow = 0, tmp3	// t == 0, flags == 0
	cmp.ne	Pcopy,p0 = 0, tmp4	// u >= 8
	;;
(Pslow)	br.spnt.many slow_path_ipc
	mov	myself = r_GLOBAL_ID
	ld8	tmp1 = [dst]
	ld8	tmp2 = [rcv_state]
	ld8	tmp3 = [rcv_partner]
	ld8	tmp4 = [r2]
(Pow)	ld8	tmp5 = [snd_send_head]
(p11)	ld8	tmp5 = [fromtcb]
(p11)	ld8	tmp6 = [tmp6]
(p11)	ld8	tmp7 = [tmp7]
#if defined(CONFIG_SMP)
	ld2	snd_cpu = [snd_cpu]
	ld2	rcv_cpu = [rcv_cpu]
(p11)	ld2	frm_cpu = [frm_cpu]
#endif
	ld8	r3 = [r3]
	movl	r2 = TSTATE_WAITING_FOREVER
	movl	tmp8 = TSTATE_POLLING
	ld8	tmp9 = [tmp9]
	;;
	cmp.eq.and.orcm Pfast,Pslow = tmp1, to	// dst->myself == to
	cmp.eq.and.orcm Pfast,Pslow = tmp2, r2	// dst->state == waiting
	cmp.eq.and.orcm Pfast,Pslow = tmp4, r0	// src->resrouce_bits == 0
	cmp.eq.and.orcm Pfast,Pslow = r3, r0	// dst->resrouce_bits == 0
	cmp.ne	p12,p0 = tmp3, myself		// dst->partner != myself
#if defined(CONFIG_SMP)
	cmp.eq.and.orcm Pfast,Pslow = snd_cpu, rcv_cpu // dst->cpu == src->cpu
(p11)	cmp.eq.and.orcm Pfast,Pslow = snd_cpu, frm_cpu // frm->cpu == src->cpu
#endif
(p11)	cmp.eq.and.orcm Pfast,Pslow = tmp5, fromspecifier 
						// frm->myself == fromspecifier
(p11)	cmp.eq	p11,p0 = tmp6, tmp8		// frm->state == polling
(Pow)	cmp.eq.and.orcm Pfast,Pslow = tmp5, r0	// src->sendhead == 0
	mov	tmp4 = -1
	;;
	tbit.z.and.orcm Pfast,Pslow = tmp9, 32	// dst->psr.cpl == 0
(p12)	cmp.eq.and.orcm Pfast,Pslow = tmp3,tmp4 // dst->partner == any
(p11)	cmp.eq.and.orcm Pfast,Pslow = tmp7, to	// frm->partner != myself
	;;
(Pslow)	br.spnt.many slow_path_ipc

//	enter_kdebug ("fast path")

	//
	// We are now doing fast path IPC
	//

	add	snd_stack =	OFS_TCB_STACK, src
	add	snd_space =	OFS_TCB_STACK, src
	add	rcv_stack =	OFS_TCB_STACK, dst
	add	rcv_space =	OFS_TCB_SPACE, dst

	rsm	psr.i

	// Create context frame for sender

	mov	user_sp = sp
	mov	tmp1 = ar.rsc
	mov	ar.rsc = (3 << 2)	// pl=3, mode=lazy
	add	sp = -16, kern_sp1
	movl	tmp4 = ipc_fast_path_reactivation_stub
	st8	[kern_sp2] = r0, 16	// num_dirty
	;;
	alloc	tmp3 = ar.pfs, 8,0,0,0
	mov	r_KERNEL_STACK_COUNTER = 1 // we have a valid kernel sp
	;;
	st8	[kern_sp1] = tmp1, 16	// rsc
	st8	[kern_sp2] = gp, 16	// gp
	;;
	mov	tmp2 = ar.bsp
	st8	[kern_sp1] = tmp3, 16	// pfs
	st8	[kern_sp2] = r0, 16	// cfm
	;;
	st8	[kern_sp1] = tmp4, 16	// ip
	st8	[kern_sp2] = tmp2, 16	// bspstore
	mov	tmp3 = ar.rnat
	mov	tmp4 = ar.unat
	;;
	mov	tmp1 = pr
	mov	tmp2 = psr
	st8	[kern_sp1] = tmp3, 16	// rnat
	st8	[kern_sp2] = tmp4, 16	// unat
	;;
	mov	tmp4 = rp
	st8	[kern_sp1] = tmp1, 16	// pr
	st8	[kern_sp2] = tmp2, 16	// psr
	;;
	st8	[kern_sp1] = user_sp, 16 // sp
	st8	[kern_sp2] = tmp4, 16	// rp
	st8	[snd_stack] = sp

	// Do we need to copy MRs between UTCBs
(Pcopy)	br.spnt.few ipc_copy_mrs
ipc_copy_done:

	// Set up thread states

	ld8	tmp3 = [snd_space]
	ld8	tmp4 = [rcv_space]

	movl	tmp1 = TSTATE_WAITING_FOREVER
	mov	tmp2 = TSTATE_RUNNING
	;;
	st8	[snd_state] = tmp1
	st8	[rcv_state] = tmp2
	st8	[snd_partner] = fromspecifier

	// Switch to destination thread

	cmp.eq	p11,p12 = tmp3, tmp4	// p11 = (from->space == to->space)
					// p12 = (from->space != to->space)
	.pred.rel "mutex", p11,p12
	dep	tmp2 = 0, tmp4, 61, 3
	mov	tmp3 = (12 << 2) + 0
	mov	r2 = dst
	;;
	ld8	tmp1 = [dst]		// dst->myself_global
(p12)	shr	tmp2 = tmp2, (12-8)
	;;
(p11)	mov	from = r_LOCAL_ID
(p12)	or	tmp4 = tmp3, tmp2
	;;
(p12)	dep	tmp4 = 0, tmp4, (8+18), (64-8-18)
	;;
	add	tmp2 = OFS_TCB_MYSELF_LOCAL, dst
	add	tmp3 = OFS_TCB_ARCH_PHYS_ADDR, dst
(p12)	mov	rr[r0] = tmp4
	;;
(p12)	srlz.d
(p12)	srlz.i
(p12)	addl	tmp4 = KTCB_SIZE, r2
	;;
	ld8	tmp2 = [tmp2]		// dst->myself_local
	ld8	tmp3 = [tmp3]		// dst->phys_tcb_addr
	mov	r_KERNEL_SP = tmp4
	;;
	add	sp = -SIZEOF_SWITCH_CONTEXT, tmp4
	mov	r_GLOBAL_ID = tmp1
	mov	r_LOCAL_ID = tmp2
	mov	r_PHYS_TCB_ADDR = tmp3
	;;

	// Restore context frame for receiver

//	enter_kdebug ("restore context")

	add	kern_sp1 = 8+8, sp
	add	kern_sp2 = 8+16, sp
	mov	ar.rsc = (3 << 2)	// pl=0, mode=lazy
	;;
	ld8	tmp1 = [kern_sp1], 16	// rsc
	ld8	gp = [kern_sp2], 32	// gp
	;;
	ld8	tmp3 = [kern_sp1], 32	// pfs
	ld8	tmp4 = [kern_sp2], 16	// bspstore
	;;
	mov	ar.pfs = tmp3
	mov	ar.bspstore = tmp4
	;;
	loadrs				// invalidate stacked regs
	invala
	ld8	tmp3 = [kern_sp1], 16	// rnat
	ld8	tmp4 = [kern_sp2], 16	// unat
	;;
	mov	ar.rnat = tmp3
	mov	ar.unat = tmp4
	;;
	mov	ar.rsc = tmp1
	ld8	tmp3 = [kern_sp1], 16	// pr
	ld8	tmp4 = [kern_sp2], 16	// psr
	;;
	ld8	tmp1 = [kern_sp1]	// sp
	ld8	tmp2 = [kern_sp2]	// rp
	dep	tmp4 = -1, tmp4, 14, 1	// enable interrupts
	;;
	mov	r_KERNEL_STACK_COUNTER = 0 // we have an invalid kernel sp
	mov	pr = tmp3, -1
	mov	psr.l = tmp4
	;;
	srlz.d
	mov	sp = tmp1
	mov	rp = tmp2
	;;
//	enter_kdebug ("context restored")
	br.ret.sptk.many rp

	//
	// Copy MRs from source UTCB to destination UTCB.
	//

ipc_copy_mrs:

num		= tmp1
src_mr		= tmp2
src_mr_pre	= tmp3
dst_mr		= tmp4

//	enter_kdebug ("copy mrs")

	extr.u	num = mr0, 0, 6
	mov	src_mr = r_LOCAL_ID
	add	dst_mr = OFS_TCB_UTCB, dst
	;;
	add	num = -8, num
	add	src_mr = UTCB_MR_OFFSET + (8*8), src_mr
	ld8	dst_mr = [dst_mr]
	;;
	lfetch	[src_mr]
	add	src_mr_pre = 32, src_mr
	add	dst_mr = UTCB_MR_OFFSET + (8*8), dst_mr
	mov	ar.lc = num
	;;
1:	ld8	tmp5 = [src_mr], 8
	lfetch	[src_mr_pre], 8
	;;
	st8	[dst_mr] = tmp5, 8

	br.cloop.sptk.few 1b
	;;
	br.sptk.many ipc_copy_done


	//
	// Return stub to use if a thread doing a fast path IPC is
	// rescheduled using the regular thread_switch() (i.e., if
	// thread is replied to with a regular IPC).
	//

r_rp			= r16		// defined by thread switch
r_pfs			= r18		// defined by thread switch
current			= r19
r_ip			= r20
r_sp			= r21
mr_ptr1			= r22
mr_ptr2			= r23
mr_ptr3			= r24
mr_ptr4			= r25
tmp1			= r31
tmp2			= r30
tmp3			= r29
tmp4			= r28

ipc_fast_path_reactivation_stub:
	mov	tmp1 = r_KERNEL_SP
	;;
	add	sp = -SIZEOF_SWITCH_CONTEXT, tmp1
	mov	r_KERNEL_STACK_COUNTER = 1	// valid kernel sp, not bsp
	;;
//	enter_kdebug ("reactivation stub")

	add	tmp1 = 16, sp
	add	tmp2 = 24, sp
	add	tmp3 = SIZEOF_SWITCH_CONTEXT-16, sp
	;;
	ld8	tmp1 = [tmp1]		// ar.rsc
	ld8	gp = [tmp2]		// gp
	dep	current = 0, sp, 0, KTCB_BITSIZE
	ld8	r_sp = [tmp3]		// sp
	mov	rp = r_rp
	;;
	mov	r_KERNEL_STACK_COUNTER = 0 // we have an invalid kernel sp
	;;
	add	tmp1 = OFS_TCB_PARTNER, current
	add	tmp2 = OFS_TCB_THREAD_STATE, current
	mov	sp = r_sp
	movl	tmp3 = TSTATE_RUNNING
	;;

	// Set FROM parameter
	ld8	from = [tmp1]

	// Restore thread state
	st8	[tmp2] = tmp3

	// Restore MRs from UTCB
	mov	mr_ptr1 = r_LOCAL_ID
	;;
	alloc	tmp1 = ar.pfs,8,0,0,0
	add	mr_ptr4 = UTCB_MR_OFFSET+24, mr_ptr1
	add	mr_ptr3 = UTCB_MR_OFFSET+16, mr_ptr1
	add	mr_ptr2 = UTCB_MR_OFFSET+8,  mr_ptr1
	add	mr_ptr1 = UTCB_MR_OFFSET,    mr_ptr1
	;;
	ld8	mr0 = [mr_ptr1], 32
	ld8	mr1 = [mr_ptr2], 32
	ld8	mr2 = [mr_ptr3], 32
	ld8	mr3 = [mr_ptr4], 32
	;;
	ld8	mr4 = [mr_ptr1]
	ld8	mr5 = [mr_ptr2]
	ld8	mr6 = [mr_ptr3]
	ld8	mr7 = [mr_ptr4]

	mov	ar.pfs = r_pfs
	mov	ar.rsc = tmp1

	// Make sure that interrupts are enabled
	mov	tmp1 = TPR_INT_ENABLE_ALL
	;;
	ssm	psr.i
	mov	cr.tpr = tmp1
	;;
	srlz.d

	br.ret.sptk.many rp


	//
	// IPC fast path fallback into slow path
	//

r_sp			= r30
r_bsp			= r29
r_rsc			= r28
r_rnat			= r8
r_pfs			= r27
kernel_bspstore		= r26
num_dirty		= r25
ptr_ip			= r24
ptr_sp			= r23
cur_tcb			= r22
mr_ptr1			= r21
mr_ptr2			= r20
mr_ptr3			= r19
mr_ptr4			= r18
num_dirty_ptr		= r17

slow_path_ipc:

	CHANGE_TO_KERNEL_BACKING_STORE (0)

	// Store MRs in UTCB
	mov	mr_ptr1 = r_LOCAL_ID
	;;
	add	mr_ptr4 = UTCB_MR_OFFSET+24, mr_ptr1
	add	mr_ptr3 = UTCB_MR_OFFSET+16, mr_ptr1
	add	mr_ptr2 = UTCB_MR_OFFSET+8,  mr_ptr1
	add	mr_ptr1 = UTCB_MR_OFFSET,    mr_ptr1
	;;
	st8	[mr_ptr1] = mr0, 32
	st8	[mr_ptr2] = mr1, 32
	st8	[mr_ptr3] = mr2, 32
	st8	[mr_ptr4] = mr3, 32
	;;
	st8	[mr_ptr1] = mr4
	st8	[mr_ptr2] = mr5
	st8	[mr_ptr3] = mr6
	st8	[mr_ptr4] = mr7

	STORE_CURRENT_CONTEXT (ipc, 0, 3, 0)
	
	INVOKE_SYSCALL_3 (ipc, to, fromspecifier, timeouts)

	RESTORE_CONTEXT (0)
	LOAD_DIRTY_USER_REGS ()

	// Store MRs in UTCB
	mov	mr_ptr1 = r_LOCAL_ID
	;;
	alloc	r14 = ar.pfs,8,0,0,0
	add	mr_ptr4 = UTCB_MR_OFFSET+24, mr_ptr1
	add	mr_ptr3 = UTCB_MR_OFFSET+16, mr_ptr1
	add	mr_ptr2 = UTCB_MR_OFFSET+8,  mr_ptr1
	add	mr_ptr1 = UTCB_MR_OFFSET,    mr_ptr1
	;;
	ld8	mr0 = [mr_ptr1], 32
	ld8	mr1 = [mr_ptr2], 32
	ld8	mr2 = [mr_ptr3], 32
	ld8	mr3 = [mr_ptr4], 32
	;;
	ld8	mr4 = [mr_ptr1]
	ld8	mr5 = [mr_ptr2]
	ld8	mr6 = [mr_ptr3]
	ld8	mr7 = [mr_ptr4]

	SWITCH_TO_USER_BACKING_STORE_AND_RETURN ()

	// One or more NaTed arguments.  Return to user mode, read
	// all args to cause exception and try again.
1:	mov	r18 = ip ;;
	add	r18 = 2f-1b, r18 ;;
	mov	b6 = r18 ;;
	br.ret.sptk.few b6 ;;
2:	mov	b6 = r14 ;;
	mov	b6 = r15 ;;
	mov	b6 = r16 ;;
	br.call.sptk.few b6 = user_ipc

END_PROC (user_ipc)

#endif /* CONFIG_IPC_FATPATH */


BEG_PROC (user_thread_control)

arg_dest		= r14
arg_space_specifier	= r15
arg_scheduler		= r16
arg_pager		= r17
arg_utcb_location	= r18

r_sp			= r30
r_bsp			= r29
r_rsc			= r28
r_rnat			= r21
r_pfs			= r27
kernel_bspstore		= r26
num_dirty		= r25
ptr_ip			= r24
ptr_sp			= r23
num_dirty_ptr		= r22

	// Convert local IDs into global ones
	cmp.ne	p10,p0 = arg_dest, r0
	cmp.ne	p11,p0 = arg_space_specifier, r0
	cmp.ne	p12,p0 = arg_scheduler, r0
	cmp.ne	p13,p0 = arg_pager, r0

	and	r8  = 0x3f, arg_dest
	and	r9  = 0x3f, arg_space_specifier
	and	r10 = 0x3f, arg_scheduler
	and	r11 = 0x3f, arg_pager
	;;
(p10)	cmp.eq	p10,p0 = r8, r0
(p11)	cmp.eq	p11,p0 = r9, r0
(p12)	cmp.eq	p12,p0 = r10,r0
(p13)	cmp.eq	p13,p0 = r11,r0
	;;
(p10)	ld8	arg_dest = [arg_dest]
(p11)	ld8	arg_space_specifier = [arg_space_specifier]
(p12)	ld8	arg_scheduler = [arg_scheduler]
(p13)	ld8	arg_pager = [arg_pager]

	cmp.eq	p6,p0 = r0, r0		// p6 = true
	epc
	;;

	// Validate NaT bits in arguments
	cmp.eq.and p6,p0 = r14, r14
	cmp.eq.and p6,p0 = r15, r15
	cmp.eq.and p6,p0 = r16, r16
	cmp.eq.and p6,p0 = r17, r17
	cmp.eq.and p6,p0 = r18, r18
	cmp.eq	p7,p0 = r0, r0		// p7 = true
	;;
(p6)	cmp.ne	p7,p0 = r0, r0		// p7 = false if no args are NaTed
	;;
(p7)	br.cond.spnt.few 1f

	CHANGE_TO_KERNEL_BACKING_STORE (0)
	STORE_CURRENT_CONTEXT (thread_control, 0, 5, 0)

	INVOKE_SYSCALL_5 (thread_control, arg_dest, arg_space_specifier,
			  arg_scheduler, arg_pager, arg_utcb_location)

	RESTORE_CONTEXT (0)
	LOAD_DIRTY_USER_REGS ()
	SWITCH_TO_USER_BACKING_STORE_AND_RETURN ()

	// One or more NaTed arguments.  Return to user mode, read
	// all args to cause exception and try again.
1:	mov	r19 = ip ;;
	add	r19 = 2f-1b, r19 ;;
	mov	b6 = r19 ;;
	br.ret.sptk.few b6 ;;
2:	mov	b6 = r14 ;;
	mov	b6 = r15 ;;
	mov	b6 = r16 ;;
	mov	b6 = r17 ;;
	mov	b6 = r18 ;;
	br.call.sptk.few b6 = user_thread_control

END_PROC (user_thread_control)


BEG_PROC (user_space_control)

arg_space_specifier	= r14
arg_control		= r15
arg_kip_area		= r16
arg_utcb_area		= r17
arg_redirector		= r18

r_sp			= r30
r_bsp			= r29
r_rsc			= r28
r_rnat			= r21
r_pfs			= r27
kernel_bspstore		= r26
num_dirty		= r25
ptr_ip			= r24
ptr_sp			= r23
num_dirty_ptr		= r22

	// Convert local IDs into global ones
	cmp.ne	p10,p0 = arg_space_specifier, r0
	cmp.ne	p11,p0 = arg_redirector, r0

	and	r8  = 0x3f, arg_space_specifier
	and	r9  = 0x3f, arg_redirector
	;;
(p10)	cmp.eq	p10,p0 = r8, r0
(p11)	cmp.eq	p11,p0 = r9, r0
	;;
(p10)	ld8	arg_space_specifier = [arg_space_specifier]
(p11)	ld8	arg_redirector = [arg_redirector]

	cmp.eq	p6,p0 = r0, r0		// p6 = true
	epc
	;;

	// Validate NaT bits in arguments
	cmp.eq.and p6,p0 = r14, r14
	cmp.eq.and p6,p0 = r15, r15
	cmp.eq.and p6,p0 = r16, r16
	cmp.eq.and p6,p0 = r17, r17
	cmp.eq.and p6,p0 = r18, r18
	cmp.eq	p7,p0 = r0, r0		// p7 = true
	;;
(p6)	cmp.ne	p7,p0 = r0, r0		// p7 = false if no args are NaTed
	;;
(p7)	br.cond.spnt.few 1f

	CHANGE_TO_KERNEL_BACKING_STORE (0)
	STORE_CURRENT_CONTEXT (space_control, 0, 5, 0)

	INVOKE_SYSCALL_5 (space_control, arg_space_specifier, arg_control,
			  arg_kip_area, arg_utcb_area, arg_redirector)

	RESTORE_CONTEXT (0)
	LOAD_DIRTY_USER_REGS ()
	SWITCH_TO_USER_BACKING_STORE_AND_RETURN ()

	// One or more NaTed arguments.  Return to user mode, read
	// all args to cause exception and try again.
1:	mov	r18 = ip ;;
	add	r18 = 2f-1b, r18 ;;
	mov	b6 = r18 ;;
	br.ret.sptk.few b6 ;;
2:	mov	b6 = r14 ;;
	mov	b6 = r15 ;;
	mov	b6 = r16 ;;
	mov	b6 = r17 ;;
	mov	b6 = r18 ;;
	br.call.sptk.few b6 = user_space_control

END_PROC (user_space_control)



BEG_PROC (user_schedule)

arg_dest		= r14
arg_time_ctrl		= r15
arg_processor_ctrl	= r16
arg_prio		= r17
arg_preemption_ctrl	= r18

r_sp			= r30
r_bsp			= r29
r_rsc			= r28
r_rnat			= r21
r_pfs			= r27
kernel_bspstore		= r26
num_dirty		= r25
ptr_ip			= r24
ptr_sp			= r23
num_dirty_ptr		= r22

	// Convert local IDs into global ones
	cmp.ne	p10,p0 = arg_dest, r0
	and	r8  = 0x3f, arg_dest
	;;
(p10)	cmp.eq	p10,p0 = r8, r0
	;;
(p10)	ld8	arg_dest = [arg_dest]

	cmp.eq	p6,p0 = r0, r0		// p6 = true
	epc
	;;

	// Validate NaT bits in arguments
	cmp.eq.and p6,p0 = r14, r14
	cmp.eq.and p6,p0 = r15, r15
	cmp.eq.and p6,p0 = r16, r16
	cmp.eq.and p6,p0 = r17, r17
	cmp.eq.and p6,p0 = r18, r18
	cmp.eq	p7,p0 = r0, r0		// p7 = true
	;;
(p6)	cmp.ne	p7,p0 = r0, r0		// p7 = false if no args are NaTed
	;;
(p7)	br.cond.spnt.few 1f

	CHANGE_TO_KERNEL_BACKING_STORE (0)
	STORE_CURRENT_CONTEXT (schedule, 0, 5, 0)

	INVOKE_SYSCALL_5 (schedule, arg_dest, arg_time_ctrl,
			  arg_processor_ctrl, arg_prio, arg_preemption_ctrl)

	RESTORE_CONTEXT (0)
	LOAD_DIRTY_USER_REGS ()
	SWITCH_TO_USER_BACKING_STORE_AND_RETURN ()

	// One or more NaTed arguments.  Return to user mode, read
	// all args to cause exception and try again.
1:	mov	r18 = ip ;;
	add	r18 = 2f-1b, r18 ;;
	mov	b6 = r18 ;;
	br.ret.sptk.few b6 ;;
2:	mov	b6 = r14 ;;
	mov	b6 = r15 ;;
	mov	b6 = r16 ;;
	mov	b6 = r17 ;;
	mov	b6 = r18 ;;
	br.call.sptk.few b6 = user_schedule

END_PROC (user_schedule)


BEG_PROC (user_thread_switch)

arg_dest		= r14

r_sp			= r30
r_bsp			= r29
r_rsc			= r28
r_rnat			= r21
r_pfs			= r27
kernel_bspstore		= r26
num_dirty		= r25
ptr_ip			= r24
ptr_sp			= r23
num_dirty_ptr		= r22

	// Convert local IDs into global ones
	cmp.ne	p10,p0 = arg_dest, r0
	and	r8  = 0x3f, arg_dest
	;;
(p10)	cmp.eq	p10,p0 = r8, r0
	;;
(p10)	ld8	arg_dest = [arg_dest]

	cmp.eq	p6,p0 = r0, r0		// p6 = true
	epc
	;;

	// Validate NaT bits in arguments
	cmp.eq.and p6,p0 = r14, r14
	cmp.eq	p7,p0 = r0, r0		// p7 = true
	;;
(p6)	cmp.ne	p7,p0 = r0, r0		// p7 = false if no args are NaTed
	;;
(p7)	br.cond.spnt.few 1f

	CHANGE_TO_KERNEL_BACKING_STORE (0)
	STORE_CURRENT_CONTEXT (thread_switch, 0, 1, 0)

	INVOKE_SYSCALL_1 (thread_switch, arg_dest)

	RESTORE_CONTEXT (0)
	LOAD_DIRTY_USER_REGS ()
	SWITCH_TO_USER_BACKING_STORE_AND_RETURN ()

	// One or more NaTed arguments.  Return to user mode, read
	// all args to cause exception and try again.
1:	mov	r18 = ip ;;
	add	r18 = 2f-1b, r18 ;;
	mov	b6 = r18 ;;
	br.ret.sptk.few b6 ;;
2:	mov	b6 = r14 ;;
	br.call.sptk.few b6 = user_thread_switch

END_PROC (user_thread_switch)


BEG_PROC (user_unmap)

arg_control		= r14
mr0			= r32
mr1			= r33
mr2			= r34
mr3			= r35
mr4			= r36
mr5			= r37
mr6			= r38
mr7			= r39

r_sp			= r30
r_bsp			= r29
r_rsc			= r28
r_rnat			= r21
r_pfs			= r27
kernel_bspstore		= r26
num_dirty		= r25
ptr_ip			= r24
ptr_sp			= r23
num_dirty_ptr		= r22
mr_ptr1			= r21
mr_ptr2			= r20
mr_ptr3			= r19
mr_ptr4			= r18

	cmp.eq	p6,p0 = r0, r0		// p6 = true
	epc
	;;

	// Validate NaT bits in arguments
	cmp.eq.and p6,p0 = r14, r14
	cmp.eq	p7,p0 = r0, r0		// p7 = true
	;;
(p6)	cmp.ne	p7,p0 = r0, r0		// p7 = false if no args are NaTed
	;;
(p7)	br.cond.spnt.few 1f

	CHANGE_TO_KERNEL_BACKING_STORE (0)

	// Store MRs in UTCB
	mov	mr_ptr1 = r_LOCAL_ID
	;;
	add	mr_ptr4 = UTCB_MR_OFFSET+24, mr_ptr1
	add	mr_ptr3 = UTCB_MR_OFFSET+16, mr_ptr1
	add	mr_ptr2 = UTCB_MR_OFFSET+8,  mr_ptr1
	add	mr_ptr1 = UTCB_MR_OFFSET,    mr_ptr1
	;;
	st8	[mr_ptr1] = mr0, 32
	st8	[mr_ptr2] = mr1, 32
	st8	[mr_ptr3] = mr2, 32
	st8	[mr_ptr4] = mr3, 32
	;;
	st8	[mr_ptr1] = mr4
	st8	[mr_ptr2] = mr5
	st8	[mr_ptr3] = mr6
	st8	[mr_ptr4] = mr7

	STORE_CURRENT_CONTEXT (unmap, 0, 1, 0)

	INVOKE_SYSCALL_1 (unmap, arg_control)

	RESTORE_CONTEXT (0)
	LOAD_DIRTY_USER_REGS ()

	// Load MRs from UTCB
	mov	mr_ptr1 = r_LOCAL_ID
	;;
	alloc	r14 = ar.pfs,8,0,0,0
	add	mr_ptr4 = UTCB_MR_OFFSET+24, mr_ptr1
	add	mr_ptr3 = UTCB_MR_OFFSET+16, mr_ptr1
	add	mr_ptr2 = UTCB_MR_OFFSET+8,  mr_ptr1
	add	mr_ptr1 = UTCB_MR_OFFSET,    mr_ptr1
	;;
	ld8	mr0 = [mr_ptr1], 32
	ld8	mr1 = [mr_ptr2], 32
	ld8	mr2 = [mr_ptr3], 32
	ld8	mr3 = [mr_ptr4], 32
	;;
	ld8	mr4 = [mr_ptr1]
	ld8	mr5 = [mr_ptr2]
	ld8	mr6 = [mr_ptr3]
	ld8	mr7 = [mr_ptr4]

	SWITCH_TO_USER_BACKING_STORE_AND_RETURN ()

	// One or more NaTed arguments.  Return to user mode, read
	// all args to cause exception and try again.
1:	mov	r18 = ip ;;
	add	r18 = 2f-1b, r18 ;;
	mov	b6 = r18 ;;
	br.ret.sptk.few b6 ;;
2:	mov	b6 = r14 ;;
	br.call.sptk.few b6 = user_unmap

END_PROC (user_unmap)



BEG_PROC (user_exchange_registers)

arg_dest		= r14
arg_control		= r15
arg_sp			= r16
arg_ip			= r17
arg_flags		= r18
arg_user_def_handle	= r19
arg_pager		= r20
arg_is_local_id		= r21

r_sp			= r30
r_bsp			= r29
r_rsc			= r28
r_rnat			= r22
r_pfs			= r27
kernel_bspstore		= r26
num_dirty		= r25
num_dirty_ptr		= r24

	// Convert local IDs into global ones
	cmp.ne	p11,p0 = arg_pager, r0

	and	r8  = 0x3f, arg_dest
	and	r9  = 0x3f, arg_pager
	;;
	cmp.eq	p10,p12 = r8, r0
(p11)	cmp.eq	p11,p0 = r9, r0
	;;
(p10)	ld8	arg_dest = [arg_dest]
(p11)	ld8	arg_pager = [arg_pager]

	cmp.eq	p6,p0 = r0, r0		// p6 = true
	epc
	;;

	.pred.rel "mutex",p10,p12
(p10)	mov	arg_is_local_id = 1
(p12)	mov	arg_is_local_id = 0

	// Validate NaT bits in arguments
	cmp.eq.and p6,p0 = r14, r14
	cmp.eq.and p6,p0 = r15, r15
	cmp.eq.and p6,p0 = r16, r16
	cmp.eq.and p6,p0 = r17, r17
	cmp.eq.and p6,p0 = r18, r18
	cmp.eq.and p6,p0 = r19, r19
	cmp.eq.and p6,p0 = r20, r20
	cmp.eq	p7,p0 = r0, r0		// p7 = true
	;;
(p6)	cmp.ne	p7,p0 = r0, r0		// p7 = false if no args are NaTed
	;;
(p7)	br.cond.spnt.few 1f

	CHANGE_TO_KERNEL_BACKING_STORE (32)
	STORE_CURRENT_CONTEXT (exchange_registers, 0, 8, 32)

	INVOKE_SYSCALL_8 (exchange_registers, arg_dest, arg_control,
			  arg_sp, arg_ip, arg_flags, arg_user_def_handle,
			  arg_pager, arg_is_local_id)

	// Setup return values
	mov	r14 = r8
	mov	r15 = r9
	mov	r16 = r10
	mov	r17 = r11
	add	out4 = 16, sp
	add	out5 = 24, sp
	add	out6 = 32, sp
	;;
	ld8	r18 = [out4]
	ld8	r19 = [out5]
	ld8	r20 = [out6]

	RESTORE_CONTEXT (32)
	LOAD_DIRTY_USER_REGS ()
	SWITCH_TO_USER_BACKING_STORE_AND_RETURN ()

	// One or more NaTed arguments.  Return to user mode, read
	// all args to cause exception and try again.
1:	mov	r18 = ip ;;
	add	r18 = 2f-1b, r18 ;;
	mov	b6 = r18 ;;
	br.ret.sptk.few b6 ;;
2:	mov	b6 = r14 ;;
	mov	b6 = r15 ;;
	mov	b6 = r16 ;;
	mov	b6 = r17 ;;
	mov	b6 = r18 ;;
	mov	b6 = r19 ;;
	mov	b6 = r20 ;;
	br.call.sptk.few b6 = user_thread_control

END_PROC (user_exchange_registers)

BEG_PROC (user_system_clock)

clock		= r14
kip_ptr		= r15
multiplier	= r16
processor	= r17
freq_offset	= r17
procdesc_offset	= r18
freq_ptr	= r15
freq		= r15

	mov	processor = r_LOCAL_ID
	;;
	mov	clock = ar.itc
	mov	multiplier = 1000000
	ld8	processor = [processor]
	;;
	//
	// No reading of user memory after this point
	//
	epc
	;;
	and	processor = 15, processor	// avoid bogus pointers
	movl	kip_ptr = kip
	;;
	add	procdesc_offset = KIP_PROCDESC_PTR_OFFSET, kip_ptr
	shl	freq_offset = processor, KIP_PROC_DESC_LOG2SIZE
	;;
	ld8	procdesc_offset = [procdesc_offset]
	add	freq_offset = PROCDESC_ARCH1_OFFSET, freq_offset
	;;
	add	freq_offset = freq_offset, procdesc_offset
	;;
	add	freq_ptr = kip_ptr, freq_offset
	;;
	ld8	freq = [freq_ptr]
	;;

	// Transfer inputs to FP registers.
	setf.sig f8 = clock
	setf.sig f9 = freq
	setf.sig f10 = multiplier
	;;
	xmpy.lu	f8 = f8, f10
	;;
	// Convert the inputs to FP, to avoid FP software-assist faults.
	fcvt.xuf.s1 f8 = f8
	fcvt.xuf.s1 f9 = f9
	;;
	// Compute the reciprocal approximation.
	frcpa.s1 f10, p6 = f8, f9
	;;
	// 3 Newton-Raphson iterations.
(p6)	fnma.s1	f11 = f9, f10, f1
(p6)	fmpy.s1	f12 = f8, f10
	;;
(p6)	fmpy.s1	f13 = f11, f11
(p6)	fma.s1	f12 = f11, f12, f12
	;;
(p6)	fma.s1	f10 = f11, f10, f10
(p6)	fma.s1	f11 = f13, f12, f12
	;;
(p6)	fma.s1	f10 = f13, f10, f10
(p6)	fnma.s1	f12 = f9, f11, f8
	;;
(p6)	fma.s1	f10 = f12, f10, f11
	;;
	// Round quotient to an unsigned integer.
	fcvt.fxu.trunc.s1 f10 = f10
	;;
	// Transfer result to GP registers.
	getf.sig ret0 = f10

	br.ret.sptk.many rp

END_PROC (user_system_clock)


BEG_PROC (user_pal_call)
	mov	ret0 = -1
	br.ret.sptk.many rp
END_PROC (user_pal_call)


BEG_PROC (user_sal_call)

arg_idx			= r32
arg_1			= r33
arg_2			= r34
arg_3			= r35
arg_4			= r36
arg_5			= r37
arg_6			= r38

r_sp			= r30
r_bsp			= r29
r_rsc			= r28
r_rnat			= r21
r_pfs			= r27
kernel_bspstore		= r26
num_dirty		= r25
num_dirty_ptr		= r24

	cmp.eq	p6,p0 = r0, r0		// p6 = true
	epc
	;;

	// Validate NaT bits in arguments
	cmp.eq.and p6,p0 = r32, r32
	cmp.eq.and p6,p0 = r33, r33
	cmp.eq.and p6,p0 = r34, r34
	cmp.eq.and p6,p0 = r35, r35
	cmp.eq.and p6,p0 = r36, r36
	cmp.eq.and p6,p0 = r37, r37
	cmp.eq.and p6,p0 = r38, r38
	cmp.eq	p7,p0 = r0, r0		// p7 = true
	;;
(p6)	cmp.ne	p7,p0 = r0, r0		// p7 = false if no args are NaTed
	;;
(p7)	br.cond.spnt.few 1f

	CHANGE_TO_KERNEL_BACKING_STORE (0)
	STORE_CURRENT_CONTEXT (sal_call, 7, 7, 0)

	INVOKE_SYSCALL_7 (sal_call, arg_idx, arg_1, arg_2, arg_3,
			  arg_4, arg_5, arg_6)

	RESTORE_CONTEXT (0)
	LOAD_DIRTY_USER_REGS ()
	SWITCH_TO_USER_BACKING_STORE_AND_RETURN ()

	// One or more NaTed arguments.  Return to user mode, read
	// all args to cause exception and try again.
1:	mov	r18 = ip ;;
	add	r18 = 2f-1b, r18 ;;
	mov	b6 = r18 ;;
	br.ret.sptk.few b6 ;;
2:	mov	b6 = r32 ;;
	mov	b6 = r33 ;;
	mov	b6 = r34 ;;
	mov	b6 = r35 ;;
	mov	b6 = r36 ;;
	mov	b6 = r37 ;;
	mov	b6 = r38 ;;
	br.call.sptk.few b6 = user_sal_call

END_PROC (user_sal_call)



#define DEFINE_SYSCALL(syscall)					\
BEG_PROC (user_##syscall)					;\
	movl	r8 = user_##syscall##_notimplemented ;;		;\
	mov	b1 = r8						;\
	epc							;\
	;;							;\
	br.sptk.few b1						;\
END_PROC (user_##syscall)					;\
								;\
	.text							;\
BEG_PROC (user_##syscall##_notimplemented)			;\
{ .mlx								;\
	break.m	0x3						;\
	movl	r0 = 1f ;;					;\
}								;\
	br.ret.sptk.many rp					;\
END_PROC (user_##syscall##_notimplemented)			;\
	.previous						;\
								;\
	.rodata							;\
1:	string "unimplemented syscall: "			;\
	stringz #syscall					;\
	.previous


//DEFINE_SYSCALL (space_control)
//DEFINE_SYSCALL (thread_control)
DEFINE_SYSCALL (processor_control)
DEFINE_SYSCALL (memory_control)
DEFINE_SYSCALL (lipc)
//DEFINE_SYSCALL (ipc)
//DEFINE_SYSCALL (unmap)
//DEFINE_SYSCALL (exchange_registers)
//DEFINE_SYSCALL (system_clock)
//DEFINE_SYSCALL (thread_switch)
//DEFINE_SYSCALL (schedule)
//DEFINE_SYSCALL (pal_call)
//DEFINE_SYSCALL (sal_call)
