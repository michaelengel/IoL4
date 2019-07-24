/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  Karlsruhe University
 *                
 * File path:     arch/ia32/trapgate.h
 * Description:   defines macros for implementation of trap and 
 *		  interrupt gates in C
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
 * $Id: trapgate.h,v 1.5 2003/02/25 17:21:56 uhlig Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__IA32__TRAPGATE_H__
#define __ARCH__IA32__TRAPGATE_H__

class ia32_exceptionframe_t
{
public:
    u32_t reason;
    u32_t edi;
    u32_t esi;
    u32_t ebp;
    u32_t __esp;
    u32_t ebx;
    u32_t edx;
    u32_t ecx;
    u32_t eax;
    /* default trapgate frame */
    u32_t error;
    u32_t eip;
    u32_t cs;
    u32_t eflags;
    u32_t esp;
    u32_t ss;
};

/**
 * IA32_EXC_WITH_ERRORCODE: allows C implementation of 
 *   exception handlers and trap/interrupt gates with error 
 *   code.
 *
 * Usage: IA32_EXC_WITH_ERRORCODE(exc_gp)
 */
#define IA32_EXC_WITH_ERRORCODE(name, reason)			\
extern "C" void name (void);					\
static void name##handler(ia32_exceptionframe_t * frame);	\
void name##_wrapper()						\
{								\
    __asm__ (							\
        ".global "#name "		\n"			\
	"\t.type "#name",@function	\n"			\
	#name":				\n"			\
	"pusha				\n"			\
	"push	%1			\n"			\
	"push	%%esp			\n"			\
	"call	%0			\n"			\
	"addl	$8, %%esp		\n"			\
	"popa				\n"			\
	"addl	$4, %%esp		\n"			\
	"iret				\n"			\
	:							\
	: "m"(*(u32_t*)name##handler), "i"(reason)		\
	);							\
}								\
static void name##handler(ia32_exceptionframe_t * frame)




#define IA32_EXC_NO_ERRORCODE(name, reason)			\
extern "C" void name (void);					\
static void name##handler(ia32_exceptionframe_t * frame);	\
void name##_wrapper()						\
{								\
    __asm__ (							\
        ".global "#name "		\n"			\
	"\t.type "#name",@function	\n"			\
	#name":				\n"			\
	"subl	$4, %%esp		\n"			\
	"pusha				\n"			\
	"push	%1			\n"			\
	"push	%%esp			\n"			\
	"call	%0			\n"			\
	"addl	$8, %%esp		\n"			\
	"popa				\n"			\
	"addl	$4, %%esp		\n"			\
	"iret				\n"			\
	:							\
	: "m"(*(u32_t*)name##handler), "i"(reason)		\
	);							\
}								\
static void name##handler(ia32_exceptionframe_t * frame)


#endif /* !__ARCH__IA32__TRAPGATE_H__ */
