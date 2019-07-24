/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     bench/pingpong/pingpong.cc
 * Description:   Pingpong test application
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
 * $Id: pingpong.cc,v 1.34.2.1 2003/06/03 19:40:47 skoglund Exp $
 *                
 ********************************************************************/
#include <config.h>
#include <l4/kip.h>
#include <l4/ipc.h>
#include <l4/schedule.h>
#include <l4/kdebug.h>
#include <l4io.h>

#if defined(L4_ARCH_IA64) || defined(L4_ARCH_ALPHA)
extern long _start_pager;
extern long _start_ping_thread;
extern long _start_pong_thread;
#define START_ADDR(func)	((L4_Word_t) &_start_##func)
#else
#define START_ADDR(func)	((L4_Word_t) func)
#endif

#define	MKSTR(sym)	MKSTR2(sym)
#define	MKSTR2(sym)	#sym

#define IPC_ARCH_OPTIMIZATION

//#define PERFMON

#define PING_PONG_PRIO 128
#define SMALLSPACE_SIZE	 (1UL << 24)

int MIGRATE = 0;
int INTER_AS = 1;
int SMALL_AS = 0;
int LIPC = 0;

extern "C" void memset (char * p, char c, int size)
{
    for (;size--;)
	*(p++)=c;
}


/*
 * Architecture dependent function stubs.
 */

#if defined(L4_ARCH_IA32)

#define HAVE_READ_CYCLES

L4_INLINE L4_Word_t read_cycles (void)
{
    L4_Word_t ret;
    asm volatile ("rdtsc" :"=a" (ret) : :"edx");
    return ret;
}

#define HAVE_ARCH_IPC
L4_INLINE L4_Word_t pingpong_ipc (L4_ThreadId_t dest, L4_Word_t untyped)
{
    L4_Word_t dummy;
    __asm__ __volatile__ (
	"/* pingpong_arch_ipc() */\n"
	"pushl	%%ebp		\n"
	"xorl	%%ecx, %%ecx	\n"
	"movl	%%esi, (%%edi)	\n"
#if 1
	"call	*__L4_Ipc	\n"	/* jump via KIP */
#else
	"leal	1f, %%ebx	\n"	/* enter kernel directly */
	"movl	%%esp, %%ebp	\n"
	"sysenter		\n"
	"1:			\n"
#endif
	"popl	%%ebp		\n"
	: "=d" (dummy), "=S"(untyped)
	: "a" (dest), "d" (dest), "S"(untyped), "D"(__L4_X86_Utcb())
	: "ebx", "ecx" );
    return untyped;
}

#elif defined(L4_ARCH_IA64)

#define HAVE_READ_CYCLES
L4_INLINE L4_Word_t read_cycles (void)
{
    L4_Word_t ret;
    asm volatile (";; mov %0 = ar.itc ;;" :"=r" (ret));
    return ret;
}

#define HAVE_ARCH_IPC
L4_INLINE L4_Word_t pingpong_ipc (L4_ThreadId_t dest, L4_Word_t untyped)
{
    register L4_ThreadId_t r_to			asm ("r14") = dest;
    register L4_ThreadId_t r_FromSpecifier	asm ("r15") = dest;
    register L4_Word_t r_Timeouts		asm ("r16") = 0;
    register L4_Word_t mr0 			asm ("out0") = untyped;

    // Make sure that we allocate 8 outputs on register stack
    register L4_Word_t mr7 asm ("out7");

    L4_Word_t ar_lc, ar_ec;
    asm volatile ("	;;			\n"
		  "	mov	%0 = ar.lc	\n"
		  "	mov	%1 = ar.ec	\n"
		  :
		  "=r" (ar_lc), "=r" (ar_ec));

    asm volatile (
	"/* pingpong_arch_ipc() */\n"
	__L4_ASM_CALL (__L4_Ipc)
	:
	"+r" (mr0), "=r" (mr7)
	:
	"r" (r_to), "r" (r_FromSpecifier), "r" (r_Timeouts)
	:
	"r2",  "r3", "r8", "r10", "r11",
	"r14", "r15", "r16", "r17", "r18", "r19", "r20", "r21", "r22",
	"r23", "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31",
	"out1", "out2", "out3", "out4", "out5", "out6", "out7",
	__L4_CALLER_SAVED_FP_REGS, __L4_CALLER_SAVED_PREDICATE_REGS,
	__L4_CALLER_SAVED_BRANCH_REGS, __L4_CALLEE_SAVED_REGS);

    asm volatile (
	"	mov	ar.lc = %0	\n"
	"	mov	ar.ec = %1	\n"
	"	;;			\n"
	:
	:
	"r" (ar_lc), "r" (ar_ec));

    return mr0 & 0x3f;
}

#elif defined(L4_ARCH_POWERPC)

#define HAVE_READ_CYCLES
L4_INLINE L4_Word_t read_cycles (void)
{
    L4_Word_t ret;
    /* We assume that the kernel configured pmc1 for cycle counting. */
    asm volatile ("mfspr %0, %1" : "=r" (ret) : "i" (937 /* upmc1 */));
    return ret;
}

#define HAVE_READ_INSTRS
L4_INLINE L4_Word_t read_instrs (void)
{
    L4_Word_t ret;
    /* We assume that the kernel configured pmc2 for cycle counting. */
    asm volatile ("mfspr %0, %1" : "=r" (ret) : "i" (938 /* upmc2 */));
    return ret;
}

#define HAVE_ARCH_IPC
L4_INLINE L4_Word_t pingpong_ipc (L4_ThreadId_t dest, L4_Word_t untyped)
{
    register L4_Word_t tag asm("r14") = untyped;
    register L4_Word_t to asm("r15") = dest.raw;
    register L4_Word_t from asm("r16") = dest.raw;
    register L4_Word_t timeouts asm("r17") = 0;

    asm volatile (
	    "mtctr %4 ;"
	    "bctrl ;"
	    : /* outputs */
	      "+r" (to), "+r" (tag)
	    : /* inputs */
	      "r" (from), "r" (timeouts), "r" (__L4_Ipc)
	    : /* clobbers */
              "r0", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11",
      	      "r12", "r13",
	      "r18", "r19", "r20", "r21", "r22", "r23", "r24", "r25",
	      "r26", "r27", "r28", "r29", "lr", "ctr",
	      "memory", __L4_PPC_CLOBBER_CR_REGS
	    );

    return tag;
}

#define HAVE_ARCH_LIPC
L4_INLINE L4_Word_t pingpong_lipc (L4_ThreadId_t dest, L4_Word_t untyped)
{
    register L4_Word_t tag asm("r14") = untyped;
    register L4_Word_t to asm("r15") = dest.raw;
    register L4_Word_t from asm("r16") = dest.raw;
    register L4_Word_t timeouts asm("r17") = 0;

    asm volatile (
	    "mtctr %4 ;"
	    "bctrl ;"
	    : /* outputs */
	      "+r" (to), "+r" (tag)
	    : /* inputs */
	      "r" (from), "r" (timeouts), "r" (__L4_Lipc)
	    : /* clobbers */
              "r0", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11",
      	      "r12", "r13",
	      "r18", "r19", "r20", "r21", "r22", "r23", "r24", "r25",
	      "r26", "r27", "r28", "r29", "lr", "ctr",
	      "memory", __L4_PPC_CLOBBER_CR_REGS
	    );

    return tag;
}

#define HAVE_ARCH_SPECIFIC
void arch_specific (void)
{
    L4_Word_t cycles1, cycles2, instrs1, instrs2;
    L4_Word_t usec1, usec2;
    int tot = 1000;
    L4_KernelInterfacePage_t * kip =
	(L4_KernelInterfacePage_t *) L4_KernelInterface ();

    usec1 = L4_SystemClock();
    cycles1 = read_cycles();
    instrs1 = read_instrs();

    asm volatile ( 
	    "mr %%r3, %0 ;"
	    "mtctr %1 ;"

	    "0: mtlr %%r3 ; "
	    "blrl ;"
	    "bdnz 0b ;"
	    :
	    : "r" (kip->ArchSyscall3), "r" (tot)
	    : "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10",
	    __L4_PPC_CLOBBER_REGS, "memory", __L4_PPC_CLOBBER_CR_REGS 
	    );

    usec2 = L4_SystemClock();
    cycles2 = read_cycles();
    instrs2 = read_instrs();

    printf( "\nNULL system call: %.2f cycles, %.2f usec, %.2f instrs\n",
	    ((double) (cycles2-cycles1))/tot,
	    ((double) (usec2-usec1))/tot,
	    ((double) (instrs2-instrs1))/tot );
}

#elif defined(L4_ARCH_ALPHA)

#define HAVE_READ_CYCLES
L4_INLINE L4_Word_t read_cycles (void)
{
    L4_Word_t ret;
    asm volatile ("rpcc %0" : "=r" (ret));
    return ret & 0xffffffffull;
}

#endif


/*
 * Default arch dependent function stubs.
 */

#if !defined(HAVE_READ_CYCLES)
L4_INLINE L4_Word_t read_cycles (void)
{
    return 0;
}
#endif

#if !defined(HAVE_READ_INSTRS)
L4_INLINE L4_Word_t read_instrs (void)
{
    return 0;
}
#endif

#if !defined(HAVE_ARCH_IPC)
L4_INLINE L4_Word_t pingpong_ipc (L4_ThreadId_t dest, L4_Word_t untyped)
{
    L4_MsgTag_t tag = L4_Niltag;
    tag.X.u = untyped;
    L4_Set_MsgTag (tag);
    return L4_UntypedWords (L4_Call (dest));
}
#endif

#if !defined(HAVE_ARCH_LIPC)
L4_INLINE L4_Word_t pingpong_lipc (L4_ThreadId_t dest, L4_Word_t untyped)
{
    L4_MsgTag_t tag = L4_Niltag;
    tag.X.u = untyped;
    L4_Set_MsgTag (tag);
    return L4_UntypedWords (L4_Lcall (dest));
}
#endif


static inline void rdpmc (int no, unsigned long long * res)
{
#if (defined(CONFIG_ARCH_X86))
    L4_Word_t dummy;
    __asm__ __volatile__ (
#if defined(PERFMON)
	"rdpmc"
#else
	""
#endif
	: "=a"(*(L4_Word_t*)res), "=d"(*((L4_Word_t*)res + 1)), "=c"(dummy)
	: "c"(no)
	: "memory");
#endif
}


L4_ThreadId_t  master_tid, pager_tid, ping_tid, pong_tid;

L4_Word_t ping_stack[2048] __attribute__ ((aligned (16)));
L4_Word_t pong_stack[2048] __attribute__ ((aligned (16)));
L4_Word_t pager_stack[2048] __attribute__ ((aligned (16)));

void pong_thread (void)
{
    L4_Word_t untyped = 0;

    if (LIPC)
    {
	L4_ThreadId_t ping_ltid = L4_LocalId (ping_tid);
	for (;;)
	    untyped = pingpong_lipc (ping_ltid, untyped);
    }
    else
	for (;;)
	    untyped = pingpong_ipc (ping_tid, untyped);
}


#define ROUNDS		1000
#define FACTOR		(8)
#define MRSTEPPING	4

void ping_thread (void)
{
    L4_Word_t cycles1, cycles2;
    L4_Word_t usec1, usec2;
    L4_Word_t instrs1, instrs2;
#ifdef PERFMON
    L4_Uint64 cnt0,cnt1;
#endif
    bool go = true;

    // Wait for pong thread to come up
    L4_Receive (pong_tid);
    L4_ThreadId_t pong_ltid = L4_LocalId (pong_tid);

    while (go)
    {
	printf("Benchmarking %s IPC...\n",
	       MIGRATE  ? "XCPU" :
	       INTER_AS ? "Inter-AS" : "Intra-AS");

	for (int j = 0; j < 64; j += MRSTEPPING)
	{
	    L4_Word_t i = ROUNDS;
	    i /= FACTOR;
	    i *= FACTOR;

#ifdef PERFMON
	    rdpmc (0,&cnt0);
#endif
	    cycles1 = read_cycles ();
	    usec1 = L4_SystemClock ();
	    instrs1 = read_instrs ();

	    if (LIPC)
		for (;i; i -= FACTOR)
		{
		    for (int k = 0; k < FACTOR; k++)
			pingpong_lipc (pong_tid, j);
		}
	    else
		for (;i; i -= FACTOR)
		{
		    for (int k = 0; k < FACTOR; k++)
			pingpong_ipc (pong_tid, j);
		}

	    cycles2 = read_cycles ();
	    usec2 = L4_SystemClock ();
	    instrs2 = read_instrs ();

#ifdef PERFMON
	    rdpmc (0,&cnt1);
	    printf ("rdpmc(0) = %Ld\n", cnt0);
	    printf ("rdpmc(1) = %Ld\n", cnt1);
	    printf ("events: %ld.%02ld\n",
		    (((long) (cnt1-cnt0) )/(ROUNDS*2)),
		    (((long) (cnt1-cnt0)*100)/(ROUNDS*2))%100);
#endif
	    printf ("IPC (%2d MRs): %ld.%02ld cycles, %ld.%02ldus, %ld.%02ld instrs\n", j,
                    ((long) (cycles2-cycles1))/(ROUNDS*2),
                    (((long) (cycles2-cycles1))*100/(ROUNDS*2))%100,
                    ((long) (usec2-usec1))/(ROUNDS*2),
                    (((long) (usec2-usec1))*100/(ROUNDS*2))%100,
                    ((long) (instrs2-instrs1))/(ROUNDS*2),
                    (((long) (instrs2-instrs1))*100/(ROUNDS*2))%100);
	}

	for (;;)
	{
	    printf ("\nWhat now?\n"
		    "    g - Continue\n"
		    "    q - Quit/New measurement\n"
		    "  ESC - Enter KDB\n");
	    char c = L4_KDB_ReadChar_Blocked ();
	    if ( c == 'g' ) { break; }
	    if ( c == 'q' ) { go = 0; break; }
	    if ( c == '\e' ) L4_KDB_Enter( "enter kdb" );
	}
    }

    // Tell master that we're finished
    L4_Set_MsgTag (L4_Niltag);
    L4_Send (master_tid);

    for (;;)
	L4_Sleep (L4_Never);

    /* NOTREACHED */
}


static L4_Word_t page_bits;

static void send_startup_ipc (L4_ThreadId_t tid, L4_Word_t ip, L4_Word_t sp)
{
#if 0
    printf ("sending startup message to %lx, (ip=%lx, sp=%lx)\n",
	    (long) tid.raw, (long) ip, (long) sp);
#endif
    L4_Msg_t msg;
    L4_Clear (&msg);
    L4_Append (&msg, ip);
    L4_Append (&msg, sp);
    L4_Load (&msg);
    L4_Send (tid);
}


void pager (void)
{
    L4_ThreadId_t tid;
    L4_MsgTag_t tag;
    L4_Msg_t msg;
    
    for (;;)
    {
	tag = L4_Wait (&tid);

	for (;;)
	{
	    L4_Store (tag, &msg);

//	    printf ("Pager got msg from %p (%p, %p, %p)\n",
//		    (void *) tid.raw, (void *) tag.raw,
//		    (void *) L4_Get (&msg, 0), (void *) L4_Get (&msg, 1));

	    if (L4_GlobalId (tid) == master_tid)
	    {
		// Startup notification, start ping and pong thread
		send_startup_ipc (ping_tid, START_ADDR (ping_thread),
				  (L4_Word_t) ping_stack +
				  sizeof (ping_stack) - 32);
		send_startup_ipc (pong_tid, START_ADDR (pong_thread),
				  (L4_Word_t) pong_stack +
				  sizeof (pong_stack) - 32);
		break;
	    }

	    if (L4_UntypedWords (tag) != 2 || L4_TypedWords (tag) != 0 ||
		!L4_IpcSucceeded (tag))
	    {
		printf ("pingpong: malformed pagefault IPC from %p (tag=%p)\n",
			(void *) tid.raw, (void *) tag.raw);
		L4_KDB_Enter ("malformed pf");
		break;
	    }

	    L4_Clear (&msg);
	    L4_Append (&msg,
		       L4_MapItem (L4_FpageLog2 (L4_Get (&msg, 0), page_bits) +
				   L4_FullyAccessible, L4_Get (&msg, 0)));
	    
	    L4_Load (&msg);
	    tag = L4_ReplyWait (tid, &tid);
	}
    }
}

#define UTCB(x) ((void*)(L4_Address(utcb_area) + (x) * utcb_size))
#define NOUTCB	((void*)-1)

int main (void)
{
    L4_Word_t control;
    L4_Fpage_t kip_area, utcb_area;
    L4_Word_t utcb_size;

    L4_KernelInterfacePage_t * kip =
	(L4_KernelInterfacePage_t *) L4_KernelInterface ();

    /* Find smallest supported page size. There's better at least one
     * bit set. */
    for (page_bits = 0;  
         !((1 << page_bits) & L4_PageSizeMask(kip)); 
         page_bits++);

    // Size for one UTCB
    utcb_size = L4_UtcbSize (kip);

    // Put the kip at the same location in all AS to make sure we can
    // reuse the syscall jump table.
    kip_area = L4_FpageLog2 ((L4_Word_t) kip, L4_KipAreaSizeLog2 (kip));

    // We need a maximum of two threads per task
    utcb_area = L4_FpageLog2 ((L4_Word_t) 0x80000000, 
			      L4_UtcbAreaSizeLog2 (kip) + 1);
    printf ("kip_area = %lx, utcb_area = %lx, utcb_size = %lx\n", 
	    kip_area.raw, utcb_area.raw, utcb_size);

    // Touch the memory to make sure we never get pagefaults
    extern L4_Word_t _end, _start;
    for (L4_Word_t * x = (&_start); x < &_end; x++)
    {
	L4_Word_t q;
	q = *(volatile L4_Word_t*) x;
    }

    // Create pager
    master_tid = L4_Myself ();
    pager_tid = L4_GlobalId (L4_ThreadNo (master_tid) + 1, 1);

    // VU: calculate UTCB address -- this has to be revised
    L4_Word_t pager_utcb = L4_MyLocalId().raw;
    pager_utcb = (pager_utcb & ~(utcb_size - 1)) + utcb_size;
    printf("local id = %lx, pager UTCB = %lx\n", L4_MyLocalId().raw,
	   pager_utcb);

    L4_ThreadControl (pager_tid, L4_Myself (), L4_Myself (), 
		      L4_nilthread, (void*)pager_utcb);
    L4_Start (pager_tid, (L4_Word_t) pager_stack + sizeof(pager_stack) - 32,
	      START_ADDR (pager));

    for (;;)
    {
	for (;;)
	{
	    printf ("\nPlease select ipc type:\n");
	    printf ("\r\n"
		    "1: INTER-AS\r\n"
		    "2: INTRA-AS (IPC)\r\n"
		    "3: INTRA-AS (LIPC)\r\n"
		    "4: XCPU\r\n"
#ifdef L4_ARCH_IA32
		    "5: INTER-AS (small)\r\n"
#endif
#ifdef HAVE_ARCH_SPECIFIC
		    "a: architecture specific\r\n"
#endif
		);
	    char c = L4_KDB_ReadChar_Blocked ();
	    if (c == '1') { INTER_AS=1; MIGRATE=0; SMALL_AS=0; LIPC=0; break; }
	    if (c == '2') { INTER_AS=0; MIGRATE=0; SMALL_AS=0; LIPC=0; break; }
	    if (c == '3') { INTER_AS=0; MIGRATE=0; SMALL_AS=0; LIPC=1; break; }
	    if (c == '4') { INTER_AS=0; MIGRATE=1; SMALL_AS=0; LIPC=0; break; }
#ifdef L4_ARCH_IA32
	    if (c == '5') { INTER_AS=1; MIGRATE=0; SMALL_AS=1; LIPC=0; break; }
#endif
#ifdef HAVE_ARCH_SPECIFIC
	    if (c == 'a') { arch_specific(); }
#endif
	    if (c == '\e') { L4_KDB_Enter ("enter kdb"); }
	}

	ping_tid = L4_GlobalId (L4_ThreadNo (master_tid) + 2, 1);
	pong_tid = L4_GlobalId (L4_ThreadNo (master_tid) + 3, 1);

	if (INTER_AS)
	{
	    L4_ThreadControl (ping_tid, ping_tid, master_tid,
			      L4_nilthread, UTCB(0));
	    L4_ThreadControl (pong_tid, pong_tid, master_tid, 
			      L4_nilthread, UTCB(0));
	    L4_SpaceControl (ping_tid, 0, kip_area, utcb_area, L4_nilthread,
			     &control);
	    L4_SpaceControl (pong_tid, 0, kip_area, utcb_area, L4_nilthread,
			     &control);
	    L4_ThreadControl (ping_tid, ping_tid, master_tid, pager_tid, 
			      NOUTCB);
	    L4_ThreadControl (pong_tid, pong_tid, master_tid, pager_tid, 
			      NOUTCB);

#if defined(L4_ARCH_IA32)
	    if (SMALL_AS)
	    {
		L4_SpaceControl (ping_tid, L4_SmallSpace (0, SMALLSPACE_SIZE), 
				 L4_Nilpage, L4_Nilpage, L4_nilthread,
				 &control);
		L4_SpaceControl (pong_tid, L4_SmallSpace (1, SMALLSPACE_SIZE), 
				 L4_Nilpage, L4_Nilpage, L4_nilthread,
				 &control);
	    }
#endif /* defined(L4_ARCH_IA32) */

	}
	else
	{
	    // Intra-as -- put both into the same space
	    L4_ThreadControl (ping_tid, ping_tid, master_tid, L4_nilthread, 
			      UTCB(0));
	    L4_SpaceControl (ping_tid, 0, kip_area, utcb_area, L4_nilthread,
			     &control);
	    L4_ThreadControl (ping_tid, ping_tid, master_tid, pager_tid, 
			      NOUTCB);
	    L4_ThreadControl (pong_tid, ping_tid, master_tid, pager_tid, 
			      UTCB(1));
	}

	if (MIGRATE)
	    L4_Set_ProcessorNo (pong_tid, (L4_ProcessorNo() + 1) % 2);

	// Send empty message to notify pager to startup both threads
	L4_LoadMR (0, 0);
	L4_Send (pager_tid);

	L4_Receive (ping_tid);
	
	// Kill both threads
	L4_ThreadControl (ping_tid, L4_nilthread, L4_nilthread, 
			  L4_nilthread, NOUTCB);
	L4_ThreadControl (pong_tid, L4_nilthread, L4_nilthread, 
			  L4_nilthread, NOUTCB);
    }

    for (;;)
	L4_KDB_Enter ("EOW");
}
