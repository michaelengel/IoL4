/*********************************************************************
 *                
 * Copyright (C) 2003,  Karlsruhe University
 *                
 * File path:     glue/v4-ia64/context_handle.h
 * Description:   Context handling macros
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
 * $Id: context_handle.h,v 1.4 2003/03/20 15:40:32 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_IA64__CONTEXT_HANDLE_H__
#define __GLUE__V4_IA64__CONTEXT_HANDLE_H__

#include <asmsyms.h>
#include INC_ARCH(asm.h)
#include INC_GLUE(registers.h)
#include INC_GLUE(config.h)

#define TPR_INT_ENABLE_SOME	((0 << 16) + (14 << 4))
#define TPR_INT_ENABLE_ALL	((0 << 16) +  (0 << 4))
#define TPR_INT_DISABLE		((1 << 16))


#define SAVE_EXCEPTION_CONTEXT(num)					\
									;\
sp1		= r1							;\
sp2		= r2							;\
									;\
r_b0		= r3							;\
r_b1		= r4							;\
r_b2		= r5							;\
r_b3		= r6							;\
r_b4		= r7							;\
r_b5		= r8							;\
r_b6		= r9							;\
r_b7		= r10							;\
r_pr		= r11							;\
									;\
tmp_pr		= r21							;\
tmp_sp		= r20							;\
r_sp		= r13							;\
new_bspstore	= r14							;\
									;\
r_bspstore	= r16							;\
num_dirty	= r17							;\
r_pfs		= r18							;\
r_rsc		= r19							;\
r_ccv		= r20							;\
r_rnat		= r21							;\
r_unat		= r15							;\
tmp_unat	= r22							;\
r_lc		= r23							;\
r_ec		= r24							;\
r_fpsr		= r25							;\
r_isr		= r26							;\
r_ipsr		= r27							;\
r_ifs		= r28							;\
r_iipa		= r29							;\
r_iip		= r30							;\
r_unat_kern	= r31							;\
r_iim		= r10							;\
r_ifa		= r9							;\
r_iha		= r8							;\
r_tpr		= r7							;\
									;\
	/* Predicates after context is saved:	*/			;\
	/*   p1 = is kernel_level		*/			;\
	/*   p2 = enable interrupts		*/			;\
	/*   p3 = switch to kernel sp		*/			;\
	/*   p4 = switch to kernel bsp		*/			;\
	/*   p5 = keep current bsp		*/			;\
									;\
	mov	r16 = cr.ipsr						;\
	mov	r17 = r_KERNEL_STACK_COUNTER				;\
	mov	r18 = cr.tpr						;\
	mov	tmp_pr = pr						;\
	;;								;\
	tbit.z	p1,p0 = r16,32		/* psr.cpl == 0? */		;\
	tbit.nz	p2,p0 = r16,14		/* psr.i == 1 */		;\
	cmp.eq	p3,p0 = 0, r17						;\
	cmp.geu	p4,p5 = 1, r17						;\
	extr	r18 = r18, 4, 4						;\
	mov	tmp_sp = sp						;\
	mov	tmp_unat = ar.unat					;\
	;;								;\
(p2)	cmp.eq	p2,p0 = r18, r0						;\
	rsm	psr.ic|psr.i|psr.dt					;\
	;;								;\
	srlz.d								;\
	srlz.i								;\
									;\
(p3)	mov	sp = r_KERNEL_SP					;\
	mov	r16 = r_PHYS_TCB_ADDR					;\
	;;								;\
	add	sp = -SIZEOF_EXCEPTION_CONTEXT,sp			;\
	;;								;\
	mov	r17 = r1						;\
	mov	r18 = r2						;\
	dep	sp1 = sp,r16,0,KTCB_BITSIZE				;\
	;;								;\
	add	sp2 = 16+16, sp1					;\
	mov	r16 = num						;\
	;;								;\
	st8	[sp1] = r16, 16		/* exc num */			;\
	;;								;\
	stf.spill [sp1] = f6, 32	/* f6 */			;\
	stf.spill [sp2] = f7, 32 ;;	/* f7 */			;\
	stf.spill [sp1] = f8, 32	/* f8 */			;\
	stf.spill [sp2] = f9, 32 ;;	/* f9 */			;\
	stf.spill [sp1] = f10, 32	/* f10 */			;\
	stf.spill [sp2] = f11, 32 ;;	/* f11 */			;\
	stf.spill [sp1] = f12, 32	/* f12 */			;\
	stf.spill [sp2] = f13, 32 ;;	/* f13 */			;\
	stf.spill [sp1] = f14, 40	/* f14 */			;\
	stf.spill [sp2] = f15, 32 ;;	/* f15 */			;\
									;\
	.mem.offset 0,0							;\
	st8.spill [sp1] = r17, 16	/* r1 */			;\
	.mem.offset 8,0							;\
	st8.spill [sp2] = r18, 16 ;;	/* r2 */			;\
	.mem.offset 0,0							;\
	st8.spill [sp1] = r3, 16	/* r3 */			;\
	.mem.offset 8,0							;\
	st8.spill [sp2] = r4, 16 ;;	/* r4 */			;\
	.mem.offset 0,0							;\
	st8.spill [sp1] = r5, 16	/* r5 */			;\
	.mem.offset 8,0							;\
	st8.spill [sp2] = r6, 16 ;;	/* r6 */			;\
	.mem.offset 0,0							;\
	st8.spill [sp1] = r7, 16	/* r7 */			;\
	.mem.offset 8,0							;\
	st8.spill [sp2] = r8, 16 ;;	/* r8 */			;\
	.mem.offset 0,0							;\
	st8.spill [sp1] = r9, 16	/* r9 */			;\
	.mem.offset 8,0							;\
	st8.spill [sp2] = r10, 16 ;;	/* r10 */			;\
	.mem.offset 0,0							;\
	st8.spill [sp1] = r11, 16	/* r11 */			;\
	.mem.offset 8,0							;\
	st8.spill [sp2] = r13, 16 ;;	/* r13 */			;\
	.mem.offset 0,0							;\
	st8.spill [sp1] = r14, 16	/* r14 */			;\
	.mem.offset 8,0							;\
	st8.spill [sp2] = r15, 16 ;;	/* r15 */			;\
									;\
	mov	r_b0 = b0						;\
	mov	r_b1 = b1						;\
	mov	r_b2 = b2						;\
	mov	r_b3 = b3						;\
	mov	r_b4 = b4						;\
	mov	r_b5 = b5						;\
	mov	r_b6 = b6						;\
	mov	r_b7 = b7						;\
	mov	r_pr = tmp_pr						;\
	mov	r_sp = tmp_sp						;\
	mov	r_unat = tmp_unat					;\
(p4)	mov	new_bspstore = r_KERNEL_SP ;;				;\
(p4)	add	new_bspstore = -IA64_SP_TO_RSE_OFFSET, new_bspstore	;\
	bsw.1								;\
	;;								;\
	ITANIUM_A_STEP_BSW						;\
	;;								;\
									;\
	.mem.offset 0,0							;\
	st8.spill [sp1] = r16, 16	/* r16 */			;\
	.mem.offset 8,0							;\
	st8.spill [sp2] = r17, 16 ;;	/* r17 */			;\
	.mem.offset 0,0							;\
	st8.spill [sp1] = r18, 16	/* r18 */			;\
	.mem.offset 8,0							;\
	st8.spill [sp2] = r19, 16 ;;	/* r19 */			;\
	.mem.offset 0,0							;\
	st8.spill [sp1] = r20, 16	/* r20 */			;\
	.mem.offset 8,0							;\
	st8.spill [sp2] = r21, 16 ;;	/* r21 */			;\
	.mem.offset 0,0							;\
	st8.spill [sp1] = r22, 16	/* r22 */			;\
	.mem.offset 8,0							;\
	st8.spill [sp2] = r23, 16 ;;	/* r23 */			;\
	.mem.offset 0,0							;\
	st8.spill [sp1] = r24, 16	/* r24 */			;\
	.mem.offset 8,0							;\
	st8.spill [sp2] = r25, 16 ;;	/* r25 */			;\
	.mem.offset 0,0							;\
	st8.spill [sp1] = r26, 16	/* r26 */			;\
	.mem.offset 8,0							;\
	st8.spill [sp2] = r27, 16 ;;	/* r27 */			;\
	.mem.offset 0,0							;\
	st8.spill [sp1] = r28, 16	/* r28 */			;\
	.mem.offset 8,0							;\
	st8.spill [sp2] = r29, 16 ;;	/* r29 */			;\
	.mem.offset 0,0							;\
	st8.spill [sp1] = r30, 16	/* r30 */			;\
	.mem.offset 8,0							;\
	st8.spill [sp2] = r31, 16 ;;	/* r31 */			;\
									;\
	st8	[sp1] = r_b0, 16	/* b0 */			;\
	st8	[sp2] = r_b1, 16 ;;	/* b1 */			;\
	st8	[sp1] = r_b2, 16	/* b2 */			;\
	st8	[sp2] = r_b3, 16 ;;	/* b3 */			;\
	st8	[sp1] = r_b4, 16	/* b4 */			;\
	st8	[sp2] = r_b5, 16 ;;	/* b5 */			;\
	st8	[sp1] = r_b6, 16	/* b6 */			;\
	st8	[sp2] = r_b7, 16 ;;	/* b7 */			;\
									;\
	mov	r_rsc =	ar.rsc						;\
	mov	r_pfs = ar.pfs						;\
	cover								;\
	;;								;\
	mov	ar.rsc = 0						;\
	;;								;\
(p5)	mov	new_bspstore = ar.bspstore				;\
	mov	r_bspstore = ar.bspstore				;\
	mov	r_ccv = ar.ccv						;\
	mov	r_lc = ar.lc						;\
	mov	r_ec = ar.ec						;\
	mov	r_fpsr = ar.fpsr					;\
	mov	r_isr = cr.isr						;\
	mov	r_ipsr = cr.ipsr					;\
	mov	r_ifs = cr.ifs						;\
	mov	r_rnat = ar.rnat					;\
	mov	r_iipa = cr.iipa					;\
	mov	r_iip = cr.iip						;\
	mov	r_ifa = cr.ifa						;\
	mov	r_iim = cr.iim						;\
	mov	r_iha = cr.iha						;\
	mov	r_tpr = cr.tpr						;\
	;;								;\
(p4)	mov	ar.bspstore = new_bspstore				;\
	mov	ar.fpsr = 0x3f			/* Disable fp traps */	;\
	;;								;\
	mov	num_dirty = ar.bsp					;\
	mov	ar.rsc = 3						;\
	;;								;\
	sub	num_dirty = num_dirty, new_bspstore			;\
	;;								;\
	st8	[sp1] = r_pr, 16		/* pr */		;\
	st8	[sp2] = r_pfs, 16 ;;		/* ar.pfs */		;\
	st8	[sp1] = num_dirty, 16 		/* num_dirty */		;\
	st8	[sp2] = r_bspstore, 16 ;;	/* ar.bspstore */	;\
	st8	[sp1] = r_rsc, 16		/* ar.rsc */		;\
	st8	[sp2] = r_ccv, 16 ;;		/* ar.ccv */		;\
	st8	[sp1] = r_unat, 16		/* ar.unat */		;\
	st8	[sp2] = r_rnat, 16 ;;		/* ar.rnat */		;\
	st8	[sp1] = r_lc, 16		/* ar.lc */		;\
	st8	[sp2] = r_ec, 16 ;;		/* ar.ec */		;\
	st8	[sp1] = r_fpsr, 16		/* ar.fpsr */		;\
	st8	[sp2] = r_iipa, 16 ;;		/* cr.iipa */		;\
	st8	[sp1] = r_isr, 16		/* cr.isr */		;\
	st8	[sp2] = r_ifs, 16 ;;		/* cr.ifs */		;\
	st8	[sp1] = r_iim, 16		/* cr.iim */		;\
	st8	[sp2] = r_ifa, 32 ;;		/* cr.ifa */		;\
	st8	[sp1] = r_iha, 32		/* cr.iha */		;\
						/* ar.unat (kern) */	;\
						/* ar.rnat (kern) */	;\
	st8	[sp2] = r_tpr, 16 ;;		/* cr.tpr */		;\
	st8	[sp1] = r_ipsr, 16		/* cr.ipsr */		;\
	st8.spill [sp2] = r_sp, -32 ;;		/* r12 */		;\
	st8	[sp1] = r_iip			/* cr.iip */		;\
	mov	r_unat_kern = ar.unat					;\
	mov	r8 = r_KERNEL_STACK_COUNTER				;\
	movl	r9 = TPR_INT_ENABLE_SOME				;\
	;;								;\
	st8	[sp2] = r_unat_kern					;\
	movl	gp = __gp						;\
	mov	cr.tpr = r9						;\
	add	r8 = 2, r8						;\
	;;								;\
	mov	r_KERNEL_STACK_COUNTER = r8				;\
	srlz.d								;\
	;;								;\
	ssm	psr.ic|psr.dt						;\
	;;								;\
	srlz.d								;\
	srlz.i								;\
	;;								;\
(p2)	ssm	psr.i							;\
	;;								;\
(p2)	srlz.d								;\
	;;

#define LOAD_EXCEPTION_CONTEXT()					\
									;\
sp1		= r1							;\
sp2		= r2							;\
									;\
r_b0		= r3							;\
r_b1		= r4							;\
r_b2		= r5							;\
r_b3		= r6							;\
r_b4		= r7							;\
r_b5		= r8							;\
r_b6		= r9							;\
r_b7		= r10							;\
r_pr		= r11							;\
tmp_unat	= r13							;\
									;\
r_bspstore	= r16							;\
num_dirty	= r17							;\
r_pfs		= r18							;\
r_rsc		= r19							;\
r_ccv		= r20							;\
r_rnat		= r21							;\
r_unat		= r22							;\
r_lc		= r23							;\
r_ec		= r24							;\
r_fpsr		= r25							;\
r_isr		= r26							;\
r_ipsr		= r27							;\
r_ifs		= r28							;\
r_iipa		= r29							;\
r_unat_kern	= r30							;\
r_tpr		= r31							;\
									;\
tmp_r1		= r30							;\
tmp_r2		= r31							;\
									;\
r_iip		= r18							;\
r_sp		= r19							;\
r_tmp_ksc	= r31							;\
									;\
	mov	r14 = r_KERNEL_STACK_COUNTER				;\
	;;								;\
	cmp.leu	p6,p0 = 4, r14						;\
	cmp.eq	p7,p0 = 2, r14						;\
									;\
	add	sp1 = SIZEOF_EXCEPTION_CONTEXT-32, sp			;\
	add	sp2 = SIZEOF_EXCEPTION_CONTEXT-48, sp			;\
	;;								;\
	ld8	r_tpr = [sp1], 8		/* cr.tpr */		;\
	;;								;\
(p7)	mov	cr.tpr = r_tpr						;\
	;;								;\
	srlz.d								;\
	;;								;\
	ld8	r_ipsr = [sp1], -56		/* cr.ipsr */		;\
	ld8	r_unat_kern = [sp2], -40 ;;	/* ar.unat (kern) */	;\
	ld8	r_ifs = [sp1], -16		/* cr.ifs */		;\
	ld8	r_isr = [sp2], -16 ;;		/* cr.isr */		;\
	ld8	r_iipa = [sp1], -16		/* cr.iipa */		;\
	ld8	r_fpsr = [sp2], -16 ;;		/* ar.fpsr */		;\
	ld8	r_ec = [sp1], -16		/* ar.ec */		;\
	ld8	r_lc = [sp2], -16 ;;		/* ar.lc */		;\
	ld8	r_rnat = [sp1], -16		/* ar.rnat */		;\
	ld8	tmp_unat = [sp2], -16 ;;	/* ar.unat */		;\
	ld8	r_ccv = [sp1], -16		/* ar.ccv */		;\
	ld8	r_rsc = [sp2], -16 ;;		/* ar.rsc */		;\
	ld8	r_bspstore = [sp1], -16		/* ar.bspstore */	;\
	ld8	num_dirty = [sp2], -16 ;;	/* num_dirty */		;\
	ld8	r_pfs = [sp1], -16		/* ar.pfs */		;\
	ld8	r_pr = [sp2], -16 ;;		/* pr */		;\
	ld8	r_b7 = [sp1], -16		/* b7 */		;\
	ld8	r_b6 = [sp2], -16 ;;		/* b6 */		;\
	ld8	r_b5 = [sp1], -16		/* b5 */		;\
	ld8	r_b4 = [sp2], -16 ;;		/* b4 */		;\
	ld8	r_b3 = [sp1], -16		/* b3 */		;\
	ld8	r_b2 = [sp2], -16 ;;		/* b2 */		;\
	ld8	r_b1 = [sp1], -16 		/* b1 */		;\
	ld8	r_b0 = [sp2], -16 ;;		/* b0 */		;\
									;\
(p6)	br.cond.dpnt.few 1f						;\
	shl	num_dirty = num_dirty,16	/* RSC.loadrs */	;\
	;;								;\
	alloc	r14 = ar.pfs,0,0,0,0					;\
	mov	ar.rsc = num_dirty					;\
	;;								;\
	loadrs								;\
	;;								;\
	mov	ar.bspstore = r_bspstore				;\
	;;								;\
	mov	ar.rnat = r_rnat					;\
1:	rsm	psr.ic|psr.i						;\
	;;								;\
	srlz.i								;\
	mov	ar.pfs = r_pfs						;\
	mov	ar.ccv = r_ccv						;\
	;;								;\
	mov	cr.tpr = r_tpr						;\
	mov	cr.ifs = r_ifs						;\
	mov	cr.ipsr = r_ipsr					;\
	mov	cr.isr = r_isr						;\
	mov	ar.fpsr = r_fpsr					;\
	mov	ar.ec = r_ec						;\
	mov	ar.lc = r_lc						;\
	mov	ar.unat = r_unat_kern					;\
	;;								;\
	srlz.d								;\
	mov	ar.rsc = r_rsc						;\
	mov	pr = r_pr, 0x1ffff					;\
	mov	b0 = r_b0						;\
	mov	b1 = r_b1						;\
	mov	b2 = r_b2						;\
	mov	b3 = r_b3						;\
	mov	b4 = r_b4						;\
	mov	b5 = r_b5						;\
	mov	b6 = r_b6						;\
	mov	b7 = r_b7						;\
	mov	tmp_unat = r_unat					;\
									;\
	ld8.fill r31 = [sp1], -16		/* r31 */		;\
	ld8.fill r30 = [sp2], -16 ;;		/* r30 */		;\
	ld8.fill r29 = [sp1], -16		/* r29 */		;\
	ld8.fill r28 = [sp2], -16 ;;		/* r28 */		;\
	ld8.fill r27 = [sp1], -16		/* r27 */		;\
	ld8.fill r26 = [sp2], -16 ;;		/* r26 */		;\
	ld8.fill r25 = [sp1], -16		/* r25 */		;\
	ld8.fill r24 = [sp2], -16 ;;		/* r16 */		;\
	ld8.fill r23 = [sp1], -16		/* r23 */		;\
	ld8.fill r22 = [sp2], -16 ;;		/* r22 */		;\
	ld8.fill r21 = [sp1], -16		/* r21 */		;\
	ld8.fill r20 = [sp2], -16 ;;		/* r20 */		;\
	ld8.fill r19 = [sp1], -16		/* r19 */		;\
	ld8.fill r18 = [sp2], -16 ;;		/* r18 */		;\
	ld8.fill r17 = [sp1], -16		/* r17 */		;\
	ld8.fill r16 = [sp2], -16 ;;		/* r16 */		;\
	bsw.0								;\
	;;								;\
	ITANIUM_A_STEP_BSW						;\
	mov	r_unat = tmp_unat					;\
	ld8.fill r15 = [sp1], -16		/* r15 */		;\
	ld8.fill r14 = [sp2], -16 ;;		/* r14 */		;\
	ld8.fill r13 = [sp1], -16		/* r13 */		;\
	ld8.fill r11 = [sp2], -16 ;;		/* r11 */		;\
	ld8.fill r10 = [sp1], -16		/* r10 */		;\
	ld8.fill r9 = [sp2], -16 ;;		/* r9 */		;\
	ld8.fill r8 = [sp1], -16		/* r8 */		;\
	ld8.fill r7 = [sp2], -16 ;;		/* r7 */		;\
	ld8.fill r6 = [sp1], -16		/* r6 */		;\
	ld8.fill r5 = [sp2], -16 ;;		/* r5 */		;\
	ld8.fill r4 = [sp1], -16		/* r4 */		;\
	ld8.fill r3 = [sp2], -16 ;;		/* r3 */		;\
	ld8.fill tmp_r2 = [sp1], -32		/* r2 */		;\
	ld8.fill tmp_r1 = [sp2], -40 ;;		/* r1 */		;\
	;;								;\
	ldf.fill f15 = [sp1], - 32		/* f15 */		;\
	ldf.fill f14 = [sp2], - 32 ;;		/* f14 */		;\
	ldf.fill f13 = [sp1], - 32		/* f13 */		;\
	ldf.fill f12 = [sp2], - 32 ;;		/* f12 */		;\
	ldf.fill f11 = [sp1], - 32		/* f11 */		;\
	ldf.fill f10 = [sp2], - 32 ;;		/* f10 */		;\
	ldf.fill f9 = [sp1], - 32		/* f9 */		;\
	ldf.fill f8 = [sp2], - 32 ;;		/* f8 */		;\
	ldf.fill f7 = [sp1]			/* f7 */		;\
	ldf.fill f6 = [sp2]			/* f6 */		;\
	;;								;\
	mov	r1 = tmp_r1						;\
	mov	r2 = tmp_r2						;\
	add	r16 = SIZEOF_EXCEPTION_CONTEXT-8, sp			;\
	add	r17 = SIZEOF_EXCEPTION_CONTEXT-16, sp			;\
	mov	r_tmp_ksc = r_KERNEL_STACK_COUNTER			;\
	;;								;\
	ld8	r_iip = [r16]			/* cr.iip */		;\
	ld8.fill r_sp = [r17]			/* r12 */		;\
	add	r_tmp_ksc = -2, r_tmp_ksc				;\
	;;								;\
	mov	ar.unat = r_unat					;\
	mov	cr.iip = r_iip						;\
	mov	sp = r_sp						;\
	mov	r_KERNEL_STACK_COUNTER = r_tmp_ksc			;\
	;;


#endif /* !__GLUE__V4_IA64__CONTEXT_HANDLE_H__ */
