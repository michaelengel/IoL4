/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  University of New South Wales
 *                
 * File path:     glue/v4-mips64/traps.S
 * Created:       20/08/2002 by Carl van Schaik
 * Description:   Kernel entry points for various traps 
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
 * $Id: traps.S,v 1.14 2003/05/01 14:56:45 sjw Exp $
 *                
 ********************************************************************/

#include INC_ARCH(asm.h)
#include INC_GLUE(context.h)

#define _mips64_debug	    \
1:	.set	noat;	    \
	li	AT, 2;	    \
	break;		    \
	.set	at;	    \
	b 1b;

	.set noat
	.set reorder
BEGIN_PROC(__mips64_tlb_refill) /* we could be from user space 0x0 */

	_mips64_debug

END_PROC(__mips64_tlb_refill)

	.set noat
	.set noreorder
BEGIN_PROC(__mips64_xtlb_refill)

	j	_mips64_xtlb_miss
	nop

END_PROC(__mips64_xtlb_refill)

	.set noat
	.set noreorder
BEGIN_PROC(__mips64_cache_error)

	dla	sp, 0xffffffff80008000
	dla	a0, CACHE_STR
	jal	printf
	nop

	_mips64_debug

END_PROC(__mips64_cache_error)

	.set noat
	.set noreorder
BEGIN_PROC(__mips64_extra_vector)
	eret
END_PROC(__mips64_extra_vector)

	.set reorder
BEGIN_PROC(_mips64_interrupt)
	SAVE_ALL_INT

	mfc0    s0, CP0_CAUSE
	mfc0    t0, CP0_STATUS
	li	a0, 7		/* Interrupt source - start at 7 */
	and     s0, t0

	.set	noreorder
	andi	t0, s0, INT_IRQ5
	bnez	t0, 1f
	andi	t0, s0, INT_IRQ4
	bnez	t0, 1f
	li	a0, 6
	andi	t0, s0, INT_IRQ3
	bnez	t0, 1f
	li	a0, 5
	andi	t0, s0, INT_IRQ2
	bnez	t0, 1f
	li	a0, 4
	andi	t0, s0, INT_IRQ1
	bnez	t0, 1f
	li	a0, 3
	andi	t0, s0, INT_IRQ0
	bnez	t0, 1f
	li	a0, 2
	andi	t0, s0, INT_SW0
	bnez	t0, 1f
	li	a0, 0
//	andi	t0, s0, INT_SW1
//	bnez	t0, 1f
	li	a0, 1

	.set	reorder
1:
	sll	a1, a0, 3
	ld      t0, interrupt_handlers(a1)
	move	a1, sp
	jal     t0

	.set    noat
	RESTORE_ALL
	eret
END_PROC(_mips64_interrupt)


	.set reorder
BEGIN_PROC(_mips64_exception)
	SAVE_ALL_INT

	dmfc0	a1, CP0_CAUSE
	dmfc0	a2, CP0_EPC
	dmfc0	a3, CP0_BADVADDR
	mfc0	t0, CP0_STATUS

	srl	a1, a1, 2
	andi    a1, a1, 0x1F

	dla	a0, EX_STR
	jal	printf

	move	a0, sp
	jal	mips_exception

	RESTORE_ALL
	eret
END_PROC(_mips64_exception)


	.set reorder
BEGIN_PROC(_mips64_xtlb_miss)
	SAVE_ALL_XTLB

//mfc0	a3, CP0_STATUS
//dmfc0	a2, CP0_BADVADDR
//dmfc0	a1, CP0_EPC
//dla	a0, XTLB_STR
//jal	printf

	dmfc0	a0, CP0_BADVADDR
	move	a1, sp
	dla	t0, handle_xtlb_miss
	jal	t0

	RESTORE_XTLB
	eret
END_PROC(_mips64_xtlb_miss)

	.set reorder
BEGIN_PROC(_mips64_xtlb_fall_through)
	RESTORE_XTLB
END_PROC(_mips64_xtlb_fall_through)

BEGIN_PROC(_mips64_stlb_miss)
	SAVE_ALL_INT

//ld	a3, PT_STATUS(sp)
//dmfc0	a2, CP0_BADVADDR
//dmfc0	a1, CP0_EPC
//dla	a0, STLB_STR
//jal	printf

	dmfc0	a0, CP0_BADVADDR
	move	a1, sp
	jal	handle_stlb_miss

	RESTORE_ALL
	eret
END_PROC(_mips64_stlb_miss)

	.set reorder
BEGIN_PROC(_mips64_tlb_mod)
	SAVE_ALL_INT

//dmfc0	a2, CP0_BADVADDR
//dmfc0	a1, CP0_EPC
//dla	a0, MTLB_STR
//jal	printf
//nop

	dmfc0	a0, CP0_BADVADDR
	move	a1, sp
	dla	t0, handle_tlb_mod
	jal	t0

	RESTORE_ALL
	eret
END_PROC(_mips64_tlb_mod)


	.set reorder
BEGIN_PROC(_mips64_watch)
	SAVE_ALL_INT

	move	a0, sp
	jal	mips_watch

	RESTORE_ALL
	eret
END_PROC(_mips64_watch)


	.data
	.align 3
XTLB_REFILL_STACK:
	.space 1024
XTLB_REFILL_STACK_end:


XTLB_STR:
	.string ":XTLB Refill: EPC=0x%16lx VA=0x%16lx STATUS=0x%16lx\n"
STLB_STR:
	.string ":STLB Refill: EPC=0x%16lx VA=0x%16lx STATUS=0x%16lx\n"
MTLB_STR:
	.string ":TLB Mod: EPC=0x%16lx VA=0x%16lx\n"
CACHE_STR:
	.string ":Cache error occured:\n"
EX_STR:	
	.string ":Exception occured: 0x%02x, EPC=0x%016lx VA=0x%016lx STATUS=0x%08lx\n"
