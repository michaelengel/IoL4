/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     arch/ia32/tracebuffer.h
 * Description:   Functions for accessing the tracebuffer
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
 ********************************************************************/
#ifndef __ARCH__IA32__TRACEBUFFER_H__
#define __ARCH__IA32__TRACEBUFFER_H__

typedef struct {
                 u32_t cycles;
                 u32_t pmc0;
                 u32_t pmc1;
               } tracestatus_t;  
                 
typedef struct {
                 tracestatus_t status;
                 u32_t identifier;
                 u32_t data[4];
               } trace_t;

typedef struct {
                 u32_t current;
                 u32_t magic;
                 u32_t counter;
                 u32_t threshold;
                 u32_t pad[4];
                 trace_t trace[];
               } tracebuffer_t;  

extern tracebuffer_t *tracebuffer;

#define TBUF_MAGIC 0x20121977

#if defined(CONFIG_PERFMON) && defined(CONFIG_CPU_IA32_I686)
  #define RDPMC "rdpmc\n"
#else
  #define RDPMC "xor %%eax, %%eax\n" 
#endif

#define TBUF_GET_NEXT_RECORD(addr) \
   asm volatile (                               \
        "movl	%%fs:0, %%esi	        \n"     \
        "addl	$32, %%esi		\n"     \
        "movl   %%esi, %%edi            \n"     \
        "andl   $0x1fffff, %%esi	\n"     \
        "movl	%%esi, %%fs:0     	\n"     \
        "rdtsc				\n"     \
        "movl 	%%eax, %%fs:0(%%edi)    \n"     \
        "xor	%%ecx, %%ecx		\n"     \
        RDPMC                                   \
        "movl   %%eax, %%fs:4(%%edi)    \n"     \
        "inc	%%ecx			\n"     \
        RDPMC                                   \
        "movl	%%eax, %%fs:8(%%edi)   \n"      \
        : "=D" (addr)                           \
        :                                       \
        : "eax", "ecx", "edx", "esi"            \
    );

#define TBUF_STORE_ITEM(addr, offset, item)     \
    asm volatile (                              \
        "movl   %0, %%fs:(" #offset "*4+12)(%%edi)\n"  \
        : : "r" (item), "D" (addr)              \
    );

extern inline void TBUF_RECORD_EVENT(unsigned _id)

{
    unsigned addr;
    TBUF_GET_NEXT_RECORD(addr);
    TBUF_STORE_ITEM(addr, 0, _id & 0x7FFFFFFF);
}

extern inline void TBUF_RECORD_EVENT(char *str)

{
    unsigned addr;
    TBUF_GET_NEXT_RECORD(addr);
    TBUF_STORE_ITEM(addr, 0, (unsigned)str);
}

extern inline void TBUF_RECORD_EVENT(char *str, unsigned _p0)

{
    unsigned addr;
    TBUF_GET_NEXT_RECORD(addr);
    TBUF_STORE_ITEM(addr, 0, (unsigned)str);
    TBUF_STORE_ITEM(addr, 1, _p0);
}

extern inline void TBUF_RECORD_EVENT(char *str, unsigned _p0, unsigned _p1)

{
    unsigned addr;
    TBUF_GET_NEXT_RECORD(addr);
    TBUF_STORE_ITEM(addr, 0, (unsigned)str);
    TBUF_STORE_ITEM(addr, 1, _p0);
    TBUF_STORE_ITEM(addr, 2, _p1);
}

extern inline void TBUF_RECORD_EVENT(char *str, unsigned _p0, unsigned _p1, unsigned _p2)

{
    unsigned addr;
    TBUF_GET_NEXT_RECORD(addr);
    TBUF_STORE_ITEM(addr, 0, (unsigned)str);
    TBUF_STORE_ITEM(addr, 1, _p0);
    TBUF_STORE_ITEM(addr, 2, _p1);
    TBUF_STORE_ITEM(addr, 3, _p2);
}

extern inline void TBUF_RECORD_EVENT(char *str, unsigned _p0, unsigned _p1, unsigned _p2, unsigned _p3)

{
    unsigned addr;
    TBUF_GET_NEXT_RECORD(addr);
    TBUF_STORE_ITEM(addr, 0, (unsigned)str);
    TBUF_STORE_ITEM(addr, 1, _p0);
    TBUF_STORE_ITEM(addr, 2, _p1);
    TBUF_STORE_ITEM(addr, 3, _p2);
    TBUF_STORE_ITEM(addr, 4, _p3);
}

extern inline void TBUF_RECORD_EVENT(unsigned _id, unsigned _p0)

{
    unsigned addr;
    TBUF_GET_NEXT_RECORD(addr);
    TBUF_STORE_ITEM(addr, 0, _id & 0x7FFFFFFF);
    TBUF_STORE_ITEM(addr, 1, _p0);
}

extern inline void TBUF_RECORD_EVENT(unsigned _id, unsigned _p0, unsigned _p1)

{
    unsigned addr;
    TBUF_GET_NEXT_RECORD(addr);
    TBUF_STORE_ITEM(addr, 0, _id & 0x7FFFFFFF);
    TBUF_STORE_ITEM(addr, 1, _p0);
    TBUF_STORE_ITEM(addr, 2, _p1);
}

extern inline void TBUF_RECORD_EVENT(unsigned _id, unsigned _p0, unsigned _p1, unsigned _p2)

{
    unsigned addr;
    TBUF_GET_NEXT_RECORD(addr);
    TBUF_STORE_ITEM(addr, 0, _id & 0x7FFFFFFF);
    TBUF_STORE_ITEM(addr, 1, _p0);
    TBUF_STORE_ITEM(addr, 2, _p1);
    TBUF_STORE_ITEM(addr, 3, _p2);
}

extern inline void TBUF_RECORD_EVENT(unsigned _id, unsigned _p0, unsigned _p1, unsigned _p2, unsigned _p3)

{
    unsigned addr;
    TBUF_GET_NEXT_RECORD(addr);
    TBUF_STORE_ITEM(addr, 0, _id & 0x7FFFFFFF);
    TBUF_STORE_ITEM(addr, 1, _p0);
    TBUF_STORE_ITEM(addr, 2, _p1);
    TBUF_STORE_ITEM(addr, 3, _p2);
    TBUF_STORE_ITEM(addr, 4, _p3);
}

#define TBUF_INCREMENT_TRACE_COUNTER() \
	asm volatile ( \
			"movl	%%fs:8, %%edx	        \n" \
			"movl	%%fs:12, %%edi	        \n" \
			"inc	%%edx			\n" \
			"movl	%%edx, %%fs:8   	\n" \
			"cmpl	%%edx, %%edi		\n" \
			"jge	1f			\n" \
			"int3				\n" \
			"xor	%%edx, %%edx		\n" \
			"movl	%%edx, %%fs:8   	\n" \
			"1: 				\n" \
			:				    \
			: 				    \
			: "edx", "edi"		    	    \
		     );	

#define TBUF_CLEAR_BUFFER() \
	asm volatile ( \
			"movl	$0, %fs:0		\n" \
		     );	

#endif /* !__ARCH__IA32__TRACEBUFFER_H__ */
