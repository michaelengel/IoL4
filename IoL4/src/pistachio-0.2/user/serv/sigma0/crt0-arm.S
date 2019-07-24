/*********************************************************************
 *                
 * Copyright (C) 1999, 2000, 2001,  Karlsruhe University
 *                
 * File path:     sigma0/crt0-arm.S
 * Description:   Startup code for ARM family processors.
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
 * $Id: crt0-arm.S,v 1.1 2002/03/01 14:49:18 uhlig Exp $
 *                
 ********************************************************************/
#include <config.h>

	.globl	_start
_start:
	ldr	sp, 2f
#if defined(CONFIG_L4_NEWSIGMA0)
	bl	sigma0_main__FP16l4_kernel_info_t
#else
	bl	sigma0_main__FP18kernel_info_page_t
#endif
1:	b	1b
2:	.word	_sigma0_stack_top

	.bss
	.p2align 12
_sigma0_stack_bottom:
	.space	8192
_sigma0_stack_top:
