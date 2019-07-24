/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  Karlsruhe University
 *                
 * File path:     glue/v4-ia32/space.h
 * Description:   ia32-specific space implementation
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
 * $Id: space.h,v 1.38 2003/04/28 18:10:05 uhlig Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_IA32__SPACE_H__
#define __GLUE__V4_IA32__SPACE_H__

#include INC_API(types.h)

#include <debug.h>
#include INC_API(fpage.h)
#include INC_API(thread.h)
#include INC_ARCH(ptab.h)
#include INC_ARCH(pgent.h)
#include INC_GLUE(config.h)


class utcb_t;
class tcb_t;
class pgent_t;

class space_t
{
public:
    enum access_e {
	read		= 0,
	write		= 2,
	readwrite	= -1,
	execute		= 0
    };

    void init(fpage_t utcb_area, fpage_t kip_area);
    void free();
    bool sync_kernel_space(addr_t addr);
    void handle_pagefault(addr_t addr, addr_t ip, access_e access, bool kernel);
    bool is_initialized();

    /* mapping */
    void map_sigma0(addr_t addr);
    void map_fpage(fpage_t snd_fp, word_t base, 
	space_t * t_space, fpage_t rcv_fp, bool grant);
    fpage_t unmap_fpage(fpage_t fpage, bool flush, bool unmap_all);

    
    /* tcb management */
    void allocate_tcb(addr_t addr);
    void map_dummy_tcb(addr_t addr);
    utcb_t *  allocate_utcb(tcb_t * tcb);

    tcb_t * get_tcb(threadid_t tid);
    tcb_t * get_tcb(void * ptr);

    /* address ranges */
    bool is_user_area(addr_t addr);
    bool is_user_area(fpage_t fpage);
    bool is_tcb_area(addr_t addr);
    bool is_mappable(addr_t addr);
    bool is_mappable(fpage_t fpage);

    /* Copy area related methods */
    bool is_copy_area (addr_t addr);
    word_t get_copy_limit (addr_t addr, word_t limit);
    void delete_copy_area (word_t n, word_t cpu);
    void populate_copy_area (word_t n, ia32_pgent_t * src, word_t cpu);

    /* kip and utcb handling */
    fpage_t get_kip_page_area();
    fpage_t get_utcb_page_area();

    /* reference counting */
    void add_tcb(tcb_t * tcb);
    bool remove_tcb(tcb_t * tcb);

public:
    /* ia-32 specific functions */
    void init_kernel_mappings();
    void init_cpu_mappings(cpuid_t cpu);
    ia32_pgent_t * get_pdir(cpuid_t cpu = 0);
    bool lookup_mapping (addr_t vaddr, pgent_t ** r_pg,
			 pgent_t::pgsize_e * r_size);
    bool readmem (addr_t vaddr, word_t * contents);
    static word_t readmem_phys (addr_t paddr);

    void release_kernel_mapping (addr_t vaddr, addr_t paddr, word_t log2size);

    void flush_tlb (space_t * curspace);
    void flush_tlbent (space_t * curspace, addr_t vaddr, word_t log2size);
    bool does_tlbflush_pay (word_t log2size)
	{ return log2size >= 32; }

    /* user space access */
    u8_t get_from_user(addr_t);

    /* update hooks */
    static void begin_update() { }
    static void end_update();

    /* generic page table walker */
    pgent_t * pgent (word_t num, word_t cpu = 0);
    void add_mapping(addr_t vaddr, addr_t paddr, pgent_t::pgsize_e size, bool writable, bool kernel, bool cacheable = true);
    void remap_area(addr_t vaddr, addr_t paddr, pgent_t::pgsize_e pgsize, word_t len, bool writable, bool kernel);
    void free_pagetables(fpage_t fpage);

private:
    union {
	ia32_pgent_t pdir[1024 * (CONFIG_SMP_MAX_CPUS + 1)];
	struct {
	    ia32_pgent_t user[USER_AREA_END >> IA32_PAGEDIR_BITS];
	    ia32_pgent_t small[SMALLSPACE_AREA_SIZE >> IA32_PAGEDIR_BITS];
	    ia32_pgent_t copy_area[COPY_AREA_COUNT][COPY_AREA_SIZE >> IA32_PAGEDIR_BITS];
	    ia32_pgent_t readmem_area;
	    // abuse some pagedir entries, make sure the valid bit is never set
	    fpage_t kip_area;
	    fpage_t utcb_area;
	    word_t thread_count;
	    
	    // afterwards, we have MYUTCB, APIC, TCBs, Kernel code
	    word_t __pad [((0U - THREAD_COUNT_INFO) >> IA32_PAGEDIR_BITS) - 1];
	} x [CONFIG_SMP_MAX_CPUS] __attribute__((aligned(IA32_PAGE_SIZE)));
    };
};


/**********************************************************************
 *
 *                      inline functions
 *
 **********************************************************************/

INLINE ia32_pgent_t * space_t::get_pdir(cpuid_t cpu)
{
#if !defined(CONFIG_SMP)
    ASSERT(cpu == 0);
#else
    ASSERT(cpu < CONFIG_SMP_MAX_CPUS);
#endif
    return virt_to_phys(&pdir[cpu * 1024]);
}

INLINE bool space_t::is_user_area(addr_t addr)
{
    return (addr >= (addr_t)USER_AREA_START && 
	    addr < (addr_t)USER_AREA_END);
}
	
INLINE bool space_t::is_tcb_area(addr_t addr)
{
    return (addr >= (addr_t)KTCB_AREA_START &&
	    addr < (addr_t)KTCB_AREA_END);
}

INLINE bool space_t::is_copy_area(addr_t addr)
{
    return (addr >= (addr_t)COPY_AREA_START &&
	    addr < (addr_t)COPY_AREA_END);
}

INLINE word_t space_t::get_copy_limit (addr_t addr, word_t limit)
{
    word_t end = (word_t) virt_to_phys (addr) + limit;

    if (is_user_area (addr))
    {
	// Address in user area.  Do not go beyond user-area boundary.
	if (end >= USER_AREA_END)
	    return (USER_AREA_END - (word_t) addr);
    }
    else
    {
	// Address in copy-area.  Make sure that we do not go beyond
	// the boundary of current copy area.
	ASSERT (is_copy_area (addr));
	if (addr_align (addr, COPY_AREA_SIZE) !=
	    addr_align ((addr_t) end, COPY_AREA_SIZE))
	{
	    return (word_t) addr_align_up (addr, COPY_AREA_SIZE) -	
		(word_t) addr;
	}
    }

    return limit;
}

INLINE void space_t::delete_copy_area (word_t n, word_t cpu)
{
    ASSERT(cpu < CONFIG_SMP_MAX_CPUS);
    for (word_t i = 0; i < (COPY_AREA_SIZE >> IA32_PAGEDIR_BITS); i++)
	x[cpu].copy_area[n][i].clear ();
}

INLINE void space_t::populate_copy_area (word_t n, ia32_pgent_t * src, word_t cpu)
{
    ASSERT(cpu < CONFIG_SMP_MAX_CPUS);
    for (word_t i = 0; i < (COPY_AREA_SIZE >> IA32_PAGEDIR_BITS); i++, src++)
	x[cpu].copy_area[n][i].copy (*src);
}

INLINE fpage_t space_t::get_kip_page_area()
{
    return x[0].kip_area;
}

INLINE fpage_t space_t::get_utcb_page_area()
{
    return x[0].utcb_area;
}

INLINE u8_t space_t::get_from_user(addr_t addr)
{
    return *(u8_t*)(addr);
}

/**
 * add a thread to the space
 * @param tcb	pointer to thread control block
 */
INLINE void space_t::add_tcb(tcb_t * tcb)
{
    x[0].thread_count += 2;
}

/**
 * remove a thread from a space
 * @param tcb	thread control block
 * @return	true if it was the last thread
 */
INLINE bool space_t::remove_tcb(tcb_t * tcb)
{
    ASSERT(x[0].thread_count != 0);
    x[0].thread_count -= 2;
    return (x[0].thread_count == 0);
}

/**
 * translates a threadid into a tcb pointer
 * @param tid thread id of the thread
 * @return pointer to thread control block
 */
INLINE tcb_t * space_t::get_tcb(threadid_t tid)
{
    return (tcb_t*)((KTCB_AREA_START) + ((tid.get_raw() >> THREADNO_SHIFT) & (THREADNO_MASK << KTCB_BITS)));
}

INLINE tcb_t * space_t::get_tcb(void * ptr)
{
   return (tcb_t*)((word_t)(ptr) & KTCB_MASK);
}

INLINE pgent_t * space_t::pgent (word_t num, word_t cpu)
{
    return ((pgent_t *) phys_to_virt (get_pdir (cpu)))->
	next (this, pgent_t::size_4m, num);
}

#ifndef CONFIG_SMP
/**
 * Flush complete TLB
 */
INLINE void space_t::flush_tlb (space_t * curspace)
{
    if (this == curspace)
	ia32_mmu_t::flush_tlb ();
}

/**
 * Flush a specific TLB entry
 * @param addr	virtual address of TLB entry
 */
INLINE void space_t::flush_tlbent (space_t * curspace, addr_t addr,
				   word_t log2size)
{
    if (this == curspace)
	ia32_mmu_t::flush_tlbent ((u32_t) addr);
}

/**
 * Update functions are empty in non-SMP case
 */
inline void space_t::end_update() { }

#else

INLINE void set_active_space (cpuid_t cpu, space_t * space)
{
    /* glue/space.cc */
    extern struct {
	space_t * space;
	char __pad [CACHE_LINE_SIZE - sizeof(space_t*)];
    } active_cpu_space[CONFIG_SMP_MAX_CPUS];
    
    if (space)
	active_cpu_space[cpu].space = space;
}
#endif /* !CONFIG_SMP */

/**********************************************************************
 *
 *                 global function declarations
 *
 **********************************************************************/

INLINE space_t* get_kernel_space()
{
    extern space_t * kernel_space;
    return kernel_space;
}

/**
 * converts the current page table into a space_t
 */
INLINE space_t * ptab_to_space(u32_t ptab, cpuid_t cpu = 0)
{
#if defined(CONFIG_SMP)
    return phys_to_virt((space_t*)(ptab - (cpu * IA32_PAGE_SIZE)));
#else
    return phys_to_virt((space_t*)(ptab));
#endif
}

void init_kernel_space();

#endif /* !__GLUE__V4_IA32__SPACE_H__ */
