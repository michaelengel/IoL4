/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     x86-syscall-stubs.S
 * Description:   syscall gate relocation
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
 * $Id: x86-syscall-stubs.S,v 1.1 2002/04/26 12:55:09 skoglund Exp $
 *                
 ********************************************************************/

#define DEFINE_SYSCALL(syscall)				\
							;\
	.section .text.syscalls, "wax", "progbits"	;\
	.global	__L4_##syscall				;\
__L4_##syscall:						;\
	jmp	__L4_fixup_##syscall			;\
							;\
	.previous					;\
							;\
__L4_fixup_##syscall:					;\
	call	__L4_init_syscalls			;\
	jmp	__L4_##syscall


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
