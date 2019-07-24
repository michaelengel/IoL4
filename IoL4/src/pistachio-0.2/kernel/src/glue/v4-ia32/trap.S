/*********************************************************************
 *
 * Copyright (C) 2002,  Karlsruhe University
 *
 * File path:    glue/v4-ia32/trap.S 
 * Description:  Trap handlers
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
 * $Id: trap.S,v 1.10 2003/05/01 16:11:13 ud3 Exp $
 *
 *********************************************************************/
#include INC_ARCH(asm.h)
#include <tcb_layout.h>
#include <asmsyms.h>

.macro TID_TO_TCB reg
	shr	$THREADNO_SHIFT, \reg
	andl	$(THREADNO_MASK << KTCB_BITS), \reg
	add	$KTCB_AREA_START, \reg
.endm

.macro CURRENT_TCB reg
	mov	%esp, \reg
	and	$(KTCB_MASK), \reg
.endm

/* Pre-Condition:
 * EAX	= To
 * ECX	= Timeout
 * EDX	= FromSpecifier
 * ESI	= MR0 (stored in UTCB)
 * EDI	= UTCB
 * EBX	= UIP
 * EBP	= USP / undef for int
 * ESP	= &KSP for sysenter, KSP+20 for int
 *
 * Stack layout: SYSENTER	INT
 *		 (undef)	ss
 *		 usp		usp
 *		 (undef)	eflags
 *		 (undef)	cs
 *		 uip		uip
 *		 timeout	timeout
 */

BEGIN_PROC(exc_user_sysipc)
#if defined(CONFIG_IA32_SYSENTER)
	movl	(%esp), %esp
	subl	$24, %esp
	movl	%ebp, 16(%esp)			// save USP
#else
	subl	$4, %esp
#endif

	movl	%ebx, 4(%esp)			// save UIP
	movl	%ecx, 0(%esp)			// save timeout
	
#ifdef CONFIG_IPC_FASTPATH

	/* msgtag only untyped items and no flags */
	movl	%esi, %ebx
	andl	$(0x3ff << 6), %ebx

	/* no receive timeout */
	or	%cx, %bx
	jne	slowpath

	/* has send phase? (catch no receive later) */
	test	%eax, %eax
	je	slowpath

	/* calculate TCB pointers of current and dest */
	mov	%eax, %ebp
	TID_TO_TCB	%ebp
	CURRENT_TCB	%ebx

	/* check thread id of destination
	 * here we could also bail out!!! */
	cmp	%eax, OFS_TCB_MYSELF_GLOBAL(%ebp)
	jne	slowpath

	/* get myself */
	movl	OFS_TCB_MYSELF_GLOBAL(%ebx), %edi

	/* check that dest thread is waiting for current or any */
	cmp	$TSTATE_WAITING_FOREVER, OFS_TCB_THREAD_STATE(%ebp)
	jne	slowpath
	movl	OFS_TCB_PARTNER(%ebp), %ecx
	cmp	%edi, %ecx
	je	1f
	add	$1, %ecx	// any?
	jnc	slowpath

1:	
#ifdef CONFIG_SMP
	/* check that both threads are on the same CPU */
	mov	OFS_TCB_CPU(%ebp), %ecx
	cmp	%ecx, OFS_TCB_CPU(%ebx)
	jne	slowpath
#endif

	/* make sure that current really blocks
	 * (FromSpec == To) || (SendHead == NULL) */
	cmp	%eax, %edx
	je	2f

	/* edx == -1 && current->sendhead == 0 */
	cmp	$-1, %edx
	jne	slowpath
	cmpl	$0, OFS_TCB_SEND_HEAD(%ebx)
	jne	slowpath

2:	
	/* set partner field */
	movl	%edi, OFS_TCB_PARTNER(%ebp)

	/* set current thread state to waiting */
	movl	%edx, OFS_TCB_PARTNER(%ebx)
	movl	$TSTATE_WAITING_FOREVER, OFS_TCB_THREAD_STATE(%ebx)
	
	/**************************************************
	 * if we reach this point the transfer can 
	 * take place
	 **************************************************/

	mov	OFS_TCB_UTCB(%ebp), %edi	// dst UTCB
	mov	%esi, OFS_UTCB_MR(%edi)		// store MR0

	and	$0x3f, %esi
	test	%esi, %esi
	jnz	fp_copy_loop
fp_copy_loop_done:

4:	/* resource handling */
	cmpl	$0, OFS_TCB_RESOURCE_BITS(%ebx)		// any resources?
	jnz	fp_save_resources
fp_save_resources_done:

5:	/* perform thread switch: (see glue/tcb.h)
	 * EBX = current
	 * ESI = dest
	 */
	movl	%ebp, %esi

	// set tss.esp0
	addl	$KTCB_SIZE, %ebp
	movl	%ebp, (tss + 4)
	
	pushl	$fp_ipc_done
	movl	OFS_TCB_MYSELF_LOCAL(%esi), %edi	// local id
	movl	OFS_TCB_PDIR_CACHE(%esi), %eax		// new PDIR
	movl	%edi, %gs:0
	movl	%esp, OFS_TCB_STACK(%ebx)		// store stack ptr
	movl	OFS_TCB_STACK(%esi), %esp		// load stack ptr
	popl	%ecx					// load dest addr

#ifndef CONFIG_ARCH_IA32_P4
	cmpl	$0, OFS_TCB_SPACE(%esi)			// kernel thread?
	je	7f
	movl	%cr3, %edx				// load ptab
	cmpl	%eax, %edx
	je	7f
#else
	cmpl	%eax, OFS_TCB_PDIR_CACHE(%ebx)
	je	7f					// same ptab
	cmpl	$0, OFS_TCB_SPACE(%esi)
	jnz	6f					// kernel thread?
	movl	%eax, OFS_TCB_PDIR_CACHE(%esi)		// update pdir_cache
	jmp	7f
#endif

6:	mov	%eax, %cr3				// switch ptab
7:
/* VU:	unclear which one pays... */	
#if 0
	cmpl	$fp_ipc_done, %ecx
	jne	9f
	/* fall through otherwise */
#else
	jmp	*%ecx
#endif

fp_ipc_done:	
	/* reactivation after IPC:
	 *  ESI = current
	 *  EDI = UTCB
	 */
	cmpl	$0, OFS_TCB_RESOURCE_BITS(%esi)
	jne	fp_load_resources
fp_load_resources_done:
	
	movl	$TSTATE_RUNNING, OFS_TCB_THREAD_STATE(%esi)
	movl	OFS_TCB_PARTNER(%esi), %eax	// load partner

	movl	0(%edi), %esi			// load MRs
	movl	4(%edi), %ebx
	movl	8(%edi), %ebp

#if defined(CONFIG_IA32_SYSENTER)
	movl	16(%esp), %ecx			// user stack
	movl	4 (%esp), %edx			// user ip
	sti
	sysexit
#else
	addl	$4, %esp
	iret
#endif

9:
	jmp	*%ecx				// activate dest

slowpath:
#endif /* CONFIG_IPC_FASTPATH */

	call	sys_ipc

#if defined(CONFIG_IA32_SYSENTER)
	movl	16(%esp), %ecx			// user stack
	movl	4 (%esp), %edx			// user ip
	sti
	sysexit
#else
	addl	$4, %esp
	iret
#endif

#ifdef CONFIG_IPC_FASTPATH
/* precondition %ebx == tcb */
fp_save_resources:
	/* no caller-saved registers contain values!!! */
	mov	%ebx, %edx
	add	$OFS_TCB_RESOURCES, %edx
	push	%ebx	/* tcb */
	push	%edx	/* tcb->resources == this */
	call	tcb_resources_save
	add	$8, %esp
	jmp	fp_save_resources_done

fp_load_resources:
	mov	%esi, %eax
	add	$OFS_TCB_RESOURCES, %eax
	push	%esi	/* tcb */
	push	%eax	/* tcb->resources == this */
	call	tcb_resources_load
	add	$8, %esp
	jmp	fp_load_resources_done

/* preconditions: 
	%esi == length 
	%edi == dest->utcb */
fp_copy_loop:
	/* copy loop */
	mov	OFS_TCB_UTCB(%ebx), %ecx	// source UTCB
	add	$(OFS_UTCB_MR+4), %ecx
	add	$(OFS_UTCB_MR+4), %edi
	xchg	%ecx, %esi
	repnz	movsl	(%esi), (%edi)
	jmp	fp_copy_loop_done
#endif

END_PROC(exc_user_sysipc)

.end
