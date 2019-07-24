/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     ia64-syscall-stubs.S
 * Description:   IA64 system call library stubs
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
 * $Id: ia64-syscall-stubs.S,v 1.5.4.1 2003/06/23 17:43:29 skoglund Exp $
 *                
 ********************************************************************/


#define DEFINE_SYSCALL(syscall)					\
								;\
	/* Stub for calling system call trampoline */		;\
	.align 16						;\
	.global __L4_##syscall					;\
	.proc	__L4_##syscall					;\
								;\
__L4_##syscall:							;\
	add	r31 = @gprel(__L4_##syscall##_ptr), gp ;;	;\
	ld8	r31 = [r31] ;;					;\
	mov	b6 = r31 ;;					;\
	br.sptk.few b6						;\
								;\
	.endp	__L4_##syscall					;\
								;\
	/* Pointer to system call function */			;\
	.data							;\
__L4_##syscall##_ptr:						;\
	data8	__L4_##syscall##_fixup				;\
	.previous						;\
								;\
	/* Fixuup stub */					;\
	.section .text.syscallfix, "ax", "progbits"		;\
	.align 16						;\
	.proc __L4_##syscall##_fixup				;\
								;\
__L4_##syscall##_fixup:						;\
	movl	r31 = __L4_##syscall ;;				;\
	mov	b6 = r31					;\
	br.sptk.few __L4_fixup_syscalls				;\
								;\
	.endp __L4_##syscall##_fixup				;\
	.previous


	//
	// Function for fixing all system call links.  Address of
	// invoking stub resides in b6.
	//

	.align	16
	.proc	__L4_fixup_syscalls
__L4_fixup_syscalls:

	// Get Kernel Interface Page
{ .mlx
	break.m	0x1face
	movl	r0 = 0x0 ;;
}
	// Offset for system call links

	add	r9 = 0x1a0, r8

	// Syscall offsets greater than 1MB is treated as absolute links

	mov	r31 = (1 << 20) ;;

	// Calculate system call locations and store the results

	ld8	r10 = [r9], 8 
	movl	r11 = __L4_SpaceControl_ptr ;;
	cmp.gt	p6,p0 = r10, r31 ;;
(p6)	add	r10 = r8, r10 ;;
	st8	[r11] = r10

	ld8	r10 = [r9], 8
	movl	r11 = __L4_ThreadControl_ptr ;;
	cmp.gt	p6,p0 = r10, r31 ;;
(p6)	add	r10 = r8, r10 ;;
	st8	[r11] = r10

	ld8	r10 = [r9], 8
	movl	r11 = __L4_ProcessorControl_ptr ;;
(p6)	add	r10 = r8, r10 ;;
	st8	[r11] = r10

	ld8	r10 = [r9], 8
	movl	r11 = __L4_MemoryControl_ptr ;;
	cmp.gt	p6,p0 = r10, r31 ;;
(p6)	add	r10 = r8, r10 ;;
	st8	[r11] = r10

	ld8	r10 = [r9], 8
	movl	r11 = __L4_Ipc_ptr ;;
	cmp.gt	p6,p0 = r10, r31 ;;
(p6)	add	r10 = r8, r10 ;;
	st8	[r11] = r10

	ld8	r10 = [r9], 8
	movl	r11 = __L4_Lipc_ptr ;;
	cmp.gt	p6,p0 = r10, r31 ;;
(p6)	add	r10 = r8, r10 ;;
	st8	[r11] = r10

	ld8	r10 = [r9], 8
	movl	r11 = __L4_Unmap_ptr ;;
	cmp.gt	p6,p0 = r10, r31 ;;
(p6)	add	r10 = r8, r10 ;;
	st8	[r11] = r10

	ld8	r10 = [r9], 8
	movl	r11 = __L4_ExchangeRegisters_ptr ;;
	cmp.gt	p6,p0 = r10, r31 ;;
(p6)	add	r10 = r8, r10 ;;
	st8	[r11] = r10

	ld8	r10 = [r9], 8
	movl	r11 = __L4_SystemClock_ptr ;;
	cmp.gt	p6,p0 = r10, r31 ;;
(p6)	add	r10 = r8, r10 ;;
	st8	[r11] = r10

	ld8	r10 = [r9], 8
	movl	r11 = __L4_ThreadSwitch_ptr ;;
	cmp.gt	p6,p0 = r10, r31 ;;
(p6)	add	r10 = r8, r10 ;;
	st8	[r11] = r10

	ld8	r10 = [r9], 8
	movl	r11 = __L4_Schedule_ptr ;;
	cmp.gt	p6,p0 = r10, r31 ;;
(p6)	add	r10 = r8, r10 ;;
	st8	[r11] = r10

	add	r9 = 0x28, r9 ;;

	ld8	r10 = [r9], 8
	movl	r11 = __L4_PAL_Call_ptr ;;
	cmp.gt	p6,p0 = r10, r31 ;;
(p6)	add	r10 = r8, r10 ;;
	st8	[r11] = r10

	ld8	r10 = [r9], 8
	movl	r11 = __L4_SAL_Call_ptr ;;
	cmp.gt	p6,p0 = r10, r31 ;;
(p6)	add	r10 = r8, r10 ;;
	st8	[r11] = r10

	// Jump back to system call stub

	br.sptk.few b6

	.endp	__L4_fixup_syscalls


DEFINE_SYSCALL (Ipc)
DEFINE_SYSCALL (Lipc)
DEFINE_SYSCALL (ExchangeRegisters)
DEFINE_SYSCALL (ThreadControl)
DEFINE_SYSCALL (SystemClock)
DEFINE_SYSCALL (ThreadSwitch)
DEFINE_SYSCALL (Schedule)
DEFINE_SYSCALL (Unmap)
DEFINE_SYSCALL (SpaceControl)
DEFINE_SYSCALL (ProcessorControl)
DEFINE_SYSCALL (MemoryControl)
DEFINE_SYSCALL (PAL_Call)
DEFINE_SYSCALL (SAL_Call)


