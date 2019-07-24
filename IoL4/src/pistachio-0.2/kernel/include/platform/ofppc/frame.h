/****************************************************************************
 *
 * Copyright (C) 2002, Karlsruhe University
 *
 * File path:	platform/ofppc/frame.h
 * Description:	Exception state structure, specific to the calling conventions
 * 		for this platform.
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
 * $Id: frame.h,v 1.7 2002/08/07 21:32:00 joshua Exp $
 *
 ***************************************************************************/

#ifndef __PLATFORM__OFPPC__FRAME_H__
#define __PLATFORM__OFPPC__FRAME_H__

/* NOTE: if you change some of these offsets, inlined assmebler in
 * glue/v4-powerpc/syscalls.h must be updated.  There are some hardcoded
 * offsets due to a limit on the number of parameters to inlined assembler.
 */

/* Stack offsets in units of word_t, relative to the top of the stack. 
 * It is important that the general purpose registers occupy consecutive
 * memory locations (starting at r3), with lower addresses for lower registers,
 * to support the multi-store and multi-load instructions (stmw, lmw).
 */
#define KSTACK_XER		(-38)
#define KSTACK_CR		(-37)
#define KSTACK_CTR		(-36)
#define KSTACK_R0		(-35)
#define KSTACK_R3		(-34)
#define KSTACK_R4		(-33)
#define KSTACK_R5		(-32)
#define KSTACK_R6		(-31)
#define KSTACK_R7		(-30)
#define KSTACK_R8		(-29)
#define KSTACK_R9		(-28)
#define KSTACK_R10		(-27)
#define KSTACK_R11		(-26)
#define KSTACK_R12		(-25)
#define KSTACK_R13		(-24)
#define KSTACK_R14		(-23)
#define KSTACK_R15		(-22)
#define KSTACK_R16		(-21)
#define KSTACK_R17		(-20)
#define KSTACK_R18		(-19)
#define KSTACK_R19		(-18)
#define KSTACK_R20		(-17)
#define KSTACK_R21		(-16)
#define KSTACK_R22		(-15)
#define KSTACK_R23		(-14)
#define KSTACK_R24		(-13)
#define KSTACK_R25		(-12)
#define KSTACK_R26		(-11)
#define KSTACK_R27		(-10)
#define KSTACK_R28		(-9)
#define KSTACK_R29		(-8)
#define KSTACK_R30		(-7)
#define KSTACK_R31		(-6)
#define KSTACK_R1		(-5)
#define KSTACK_R2		(-4)
#define KSTACK_LR		(-3)
#define KSTACK_SRR0		(-2)
#define KSTACK_SRR1		(-1)

#define KSTACK_USP		KSTACK_R1
#define KSTACK_UIP		KSTACK_SRR0
#define KSTACK_UFLAGS		KSTACK_SRR1

/* The stack bottoms must be 8-byte aligned!
 */ 
#define KSTACK_FRAME_BOTTOM	KSTACK_XER

/* Save space on the stack for system calls, and only allocate space for
 * some of the registers.  The kernel had better not try to access the
 * other registers saved in a normal exception frame!  If the kernel must,
 * then change SCSTACK_FRAME_BOTTOM to equal KSTACK_FRAME_BOTTOM.
 */
#define SCSTACK_FRAME_BOTTOM	KSTACK_R29

/* Convert the stack offset into a word_t array index. */
#define FRAME_IDX(a)		((a) - KSTACK_FRAME_BOTTOM)
#define SC_FRAME_IDX(a)		((a) - SCSTACK_FRAME_BOTTOM)

/* Convert the KSTACK word_t offset (which is relative to the top of the stack),
 * into a byte offset relative to the bottom of the stack.
 */
#define OF_BASE 8       /* Preserve room for LR + the back chain when
			 * calling kernel C code. */
#define KOFF(a)		(4*FRAME_IDX(a) + OF_BASE)
#define SCOFF(a)	(4*SC_FRAME_IDX(a) + OF_BASE)

#define EXC_STACK_SIZE	KOFF(0)
#define SC_STACK_SIZE	SCOFF(0)

/* Byte offsets, relative to the bottom of the exception frame.
 */
#define OF_R0	KOFF(KSTACK_R0)
#define OF_R1	KOFF(KSTACK_R1)
#define OF_R2	KOFF(KSTACK_R2)
#define OF_R3	KOFF(KSTACK_R3)
#define OF_R4	KOFF(KSTACK_R4)
#define OF_R5	KOFF(KSTACK_R5)
#define OF_R6	KOFF(KSTACK_R6)
#define OF_R7	KOFF(KSTACK_R7)
#define OF_R8	KOFF(KSTACK_R8)
#define OF_R9	KOFF(KSTACK_R9)
#define OF_R10	KOFF(KSTACK_R10)
#define OF_R11	KOFF(KSTACK_R11)
#define OF_R12	KOFF(KSTACK_R12)
#define OF_R13	KOFF(KSTACK_R13)
#define OF_R14	KOFF(KSTACK_R14)
#define OF_R15	KOFF(KSTACK_R15)
#define OF_R16	KOFF(KSTACK_R16)
#define OF_R17	KOFF(KSTACK_R17)
#define OF_R18	KOFF(KSTACK_R18)
#define OF_R19	KOFF(KSTACK_R19)
#define OF_R20	KOFF(KSTACK_R20)
#define OF_R21	KOFF(KSTACK_R21)
#define OF_R22	KOFF(KSTACK_R22)
#define OF_R23	KOFF(KSTACK_R23)
#define OF_R24	KOFF(KSTACK_R24)
#define OF_R25	KOFF(KSTACK_R25)
#define OF_R26	KOFF(KSTACK_R26)
#define OF_R27	KOFF(KSTACK_R27)
#define OF_R28	KOFF(KSTACK_R28)
#define OF_R29	KOFF(KSTACK_R29)
#define OF_R30	KOFF(KSTACK_R30)
#define OF_R31	KOFF(KSTACK_R31)
#define OF_LR	KOFF(KSTACK_LR)
#define OF_XER	KOFF(KSTACK_XER)
#define OF_CR	KOFF(KSTACK_CR)
#define OF_CTR	KOFF(KSTACK_CTR)
#define OF_SRR0	KOFF(KSTACK_SRR0)
#define OF_SRR1	KOFF(KSTACK_SRR1)

#define SC_OF_R1	SCOFF(KSTACK_R1)
#define SC_OF_R2	SCOFF(KSTACK_R2)
#define SC_OF_R30	SCOFF(KSTACK_R30)
#define SC_OF_R31	SCOFF(KSTACK_R31)
#define SC_OF_SRR0	SCOFF(KSTACK_SRR0)
#define SC_OF_SRR1	SCOFF(KSTACK_SRR1)
#define SC_OF_LR	SCOFF(KSTACK_LR)

#endif	/* __PLATFORM__OFPPC__FRAME_H__ */
