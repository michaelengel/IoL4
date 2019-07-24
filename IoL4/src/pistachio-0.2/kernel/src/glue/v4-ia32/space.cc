/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  Karlsruhe University
 *                
 * File path:     glue/v4-ia32/space.cc
 * Description:   address space management
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
 * $Id: space.cc,v 1.28 2003/04/05 00:18:54 uhlig Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include <kmemory.h>
#include <generic/lib.h>

#include INC_API(tcb.h)
#include INC_API(smp.h)

#include INC_ARCH(mmu.h)
#include INC_ARCH(trapgate.h)
#include INC_ARCH(pgent.h)
#include <linear_ptab.h>

#include INC_PLAT(memory.h)
#include INC_GLUE(space.h)
#include INC_API(kernelinterface.h)

#define PGSIZE_KTCB	(pgent_t::size_4k)
#define PGSIZE_UTCB	(pgent_t::size_4k)
#define PGSIZE_KERNEL	(KERNEL_PAGE_SIZE == IA32_SUPERPAGE_SIZE) ? pgent_t::size_4m : pgent_t::size_4k
#define PGSIZE_KIP	(pgent_t::size_4k)

EXTERN_KMEM_GROUP (kmem_space);
DECLARE_KMEM_GROUP (kmem_tcb);
DECLARE_KMEM_GROUP (kmem_utcb);

/* helpers */
INLINE void align_memregion(mem_region_t & region, word_t size)
{
    region.low = addr_t((word_t)region.low & ~(size - 1));
    region.high = addr_t(((word_t)region.high + size - 1) & ~(size - 1));
}

space_t * kernel_space = NULL;
tcb_t * dummy_tcb = NULL;

static tcb_t * get_dummy_tcb()
{
    if (!dummy_tcb)
    {
	dummy_tcb = (tcb_t*)kmem.alloc(kmem_tcb, IA32_PAGE_SIZE);
	ASSERT(dummy_tcb);
	dummy_tcb = virt_to_phys(dummy_tcb);
    }
    return dummy_tcb;
}

void space_t::add_mapping(addr_t vaddr, addr_t paddr, pgent_t::pgsize_e size, 
			  bool writable, bool kernel, bool cacheable)
{
    pgent_t::pgsize_e curr_size = pgent_t::size_max;
    pgent_t * pgent = this->pgent(page_table_index(curr_size, vaddr));

    //TRACEF("space=%p, v=%p, p=%p, size=%d, w=%d, k=%d\n", this, vaddr, paddr, size, writable, kernel);

    if (size < curr_size)
    {
	if (!pgent->is_valid(this, curr_size))
	{
	    pgent->make_subtree(this, curr_size, kernel);
	}
	else
	{
	    /* check if already mapped as a 4MB page */
	    if (!pgent->is_subtree(this, curr_size))
	    {
		/* check that alignement of virtual and physical page fits */
		ASSERT(addr_mask(vaddr, ~IA32_SUPERPAGE_MASK) ==
		       addr_mask(paddr,~IA32_SUPERPAGE_MASK));

		if (((addr_t)pgent->address(this, curr_size)) == 
		    addr_mask(paddr, IA32_SUPERPAGE_MASK))
		    return;
		ASSERT(0);
	    }
	}
	curr_size--;
	//TRACEF("%p, %p\n", pgent->subtree(this, curr_size + 1), pgent);
	pgent = pgent->subtree(this, curr_size + 1)->next(
	    this, curr_size, page_table_index(curr_size, vaddr));
    }
    //TRACEF("%p\n", pgent);
    pgent->set_entry(this, curr_size, paddr, true, writable, true, kernel);

    // default is cacheable
    if (!cacheable) pgent->set_cacheability (this, curr_size, false);
}

void space_t::remap_area(addr_t vaddr, addr_t paddr, pgent_t::pgsize_e pgsize,
			 word_t len, bool writable, bool kernel)
{
    TRACE_INIT("remap area %p->%p len=%x, w=%d, k=%d, size=%d\n", 
	       vaddr, paddr, len, writable, kernel, pgsize);

    word_t page_size = (pgsize == pgent_t::size_4k) ? IA32_PAGE_SIZE : 
	IA32_SUPERPAGE_SIZE;

    // length must be page-size aligned
    ASSERT((len & (page_size - 1)) == 0);

    for (word_t offset = 0; offset <= len; offset += page_size)
	add_mapping(addr_offset(vaddr, offset), addr_offset(paddr, offset), 
		    pgsize, writable, kernel);
}


/**
 * reads a word from a given physical address, uses a remap window and
 * maps a 4MB page for the access
 *
 * @param paddr		physical address to read from
 * @return the value at the given address
 */
word_t space_t::readmem_phys (addr_t paddr)
{
    int cpu = get_current_cpu();

    // get the _real_ space, use CR3 for that
    space_t * space = ptab_to_space(ia32_mmu_t::get_active_pagetable(), cpu);

    // map physical 4MB page into remap window
    if (!space->x[cpu].readmem_area.is_valid() ||
	( space->x[cpu].readmem_area.get_address() != 
	  addr_mask(paddr, IA32_SUPERPAGE_MASK) ))
    {
	space->x[cpu].readmem_area.set_entry(addr_mask(paddr, IA32_SUPERPAGE_MASK),
					     false, 
					     ia32_pgent_t::size_4m, 
					     IA32_PAGE_KERNEL | IA32_PAGE_VALID);
	// kill potentially stale TLB entry in remap-window
	ia32_mmu_t::flush_tlbent (MEMREAD_AREA_START);
#if 0
	printf("readmem_phys: space=%p, mapped %p @ %p\n", 
	       space, space->x[cpu].readmem_area.get_raw(), 
	       &space->x[cpu].readmem_area);
#endif
    }
    return *(word_t*)addr_offset(addr_mask(paddr,~IA32_SUPERPAGE_MASK), 
				 MEMREAD_AREA_START);
}


/**********************************************************************
 *
 *                        SMP handling
 *
 **********************************************************************/

#ifdef CONFIG_SMP

struct {
    space_t * space;
    char __pad [CACHE_LINE_SIZE - sizeof(space_t*)];
} active_cpu_space[CONFIG_SMP_MAX_CPUS];

static word_t cpu_remote_flush UNIT("cpulocal");

static void do_xcpu_flush_tlb(cpu_mb_entry_t * entry)
{
    spin(60, get_current_cpu());
    ia32_mmu_t::flush_tlb ();
}

INLINE void tag_flush_remote (space_t * curspace)
{
    for (int cpu = 0; cpu < CONFIG_SMP_MAX_CPUS; cpu++)
    {
	if (cpu == get_current_cpu())
	    continue;
	if (active_cpu_space[cpu].space == curspace)
	    cpu_remote_flush |= (1 << cpu);
    }
}

void space_t::flush_tlb (space_t * curspace)
{
    if (this == curspace)
	ia32_mmu_t::flush_tlb ();
    tag_flush_remote (this);
}

void space_t::flush_tlbent (space_t * curspace, addr_t addr, word_t log2size)
{
    if (this == curspace)
	ia32_mmu_t::flush_tlbent ((u32_t) addr);
    tag_flush_remote (this);
}

void space_t::end_update ()
{
    for (int cpu = 0; cpu < CONFIG_SMP_MAX_CPUS; cpu++)
	if (cpu_remote_flush & (1 << cpu))
	    sync_xcpu_request(cpu, do_xcpu_flush_tlb);
    cpu_remote_flush = 0;
}
#endif



/**********************************************************************
 *
 *                         System initialization 
 *
 **********************************************************************/

#if (KERNEL_PAGE_SIZE != IA32_SUPERPAGE_SIZE) && \
    (KERNEL_PAGE_SIZE != IA32_PAGE_SIZE)
# error invalid kernel page size - please adapt
#endif

void SECTION(".init.memory") space_t::init_kernel_mappings()
{
    /* we map both reserved areas into the kernel area */
    mem_region_t reg = get_kip()->reserved_mem0;
    ASSERT(!reg.is_empty());
    
    align_memregion(reg, KERNEL_PAGE_SIZE);
    remap_area(phys_to_virt(reg.low), reg.low, PGSIZE_KERNEL, reg.get_size(), 
	       true, true);

    if (!get_kip()->reserved_mem1.is_empty())
    {
	reg = get_kip()->reserved_mem1;
	align_memregion(reg, IA32_PAGEDIR_SIZE);
	remap_area(phys_to_virt(reg.low), reg.low, pgent_t::size_4m, 
		   reg.get_size(), true, true);
    }

    /* map init memory */
    reg.set(start_init, end_init);
    align_memregion(reg, KERNEL_PAGE_SIZE);
    remap_area(reg.low, reg.low, PGSIZE_KERNEL, reg.get_size(), true, true);

    /* map low 4MB pages for initialization */
    reg.set((addr_t)0, (addr_t)0x00400000);
    align_memregion(reg, IA32_SUPERPAGE_SIZE);
    remap_area(reg.low, reg.low, pgent_t::size_4m, reg.get_size(), true, true);

    /* map video mem */
    add_mapping(phys_to_virt((addr_t)VIDEO_MAPPING), (addr_t)VIDEO_MAPPING,
		pgent_t::size_4k, true, true);

    /* MYUTCB mapping
     * allocate a full page for all myutcb pointers.
     * access must be performed via gs:0, when setting up the gdt
     * each processor gets a full cache line to avoid bouncing */
    EXTERN_KMEM_GROUP(kmem_misc);
    add_mapping((addr_t)MYUTCB_MAPPING,	
		virt_to_phys(kmem.alloc(kmem_misc, IA32_PAGE_SIZE)), 
		pgent_t::size_4k, true,	false);
}


void space_t::init_cpu_mappings(cpuid_t cpu)
{
#if defined(CONFIG_SMP)
    /* CPU 0 gets the always initialized page table */
    if (cpu == 0) return;

    TRACE_INIT("init cpu mappings for cpu %d\n", cpu);

    mem_region_t reg = { start_cpu_local, end_cpu_local };
    align_memregion(reg, IA32_PAGEDIR_SIZE);

    TRACE_INIT("remapping CPU local memory %p - %p (%p - %p)\n", 
	       start_cpu_local, end_cpu_local, reg.low, reg.high);

    pgent_t::pgsize_e size = pgent_t::size_max;

    for (addr_t addr = reg.low; addr < reg.high; 
	 addr = addr_offset(addr, IA32_PAGEDIR_SIZE))
    {
	pgent_t * dst_pgent = this->pgent(page_table_index(size, addr), cpu);
	pgent_t * src_pgent = this->pgent(page_table_index(size, addr), 0);
	dst_pgent->make_cpu_subtree(this, size, true);

	ASSERT(src_pgent->is_subtree(this, size));

	src_pgent = src_pgent->subtree(this, size);
	dst_pgent = dst_pgent->subtree(this, size);
	
	size--;

	// now copy the page table
	for (addr_t pgaddr = addr; 
	     pgaddr < addr_offset(addr, IA32_PAGEDIR_SIZE);
	     pgaddr = addr_offset(pgaddr, IA32_PAGE_SIZE))
	{
	    //TRACE_INIT("copying ptab @ %p (%p, %p)\n", pgaddr, src_pgent, dst_pgent);
	    if (pgaddr < start_cpu_local || pgaddr > end_cpu_local)
		*dst_pgent = *src_pgent; // global data
	    else
	    {
		addr_t page = kmem.alloc(kmem_pgtab, IA32_PAGE_SIZE);
		TRACE_INIT("allocated cpu local page %p -> %p\n", pgaddr, page);
		dst_pgent->set_entry(this, size, virt_to_phys(page), 
				     true, true, true, true);
		TRACE_INIT("pgent = %x\n", dst_pgent->raw);
		memcpy(page, pgaddr, IA32_PAGE_SIZE);
	    }
	    src_pgent = src_pgent->next(this, size, 1);
	    dst_pgent = dst_pgent->next(this, size, 1);
	}
	size++;
    }
    TRACE_INIT("switching to CPU local pagetable %p\n", get_pdir(cpu));
    ia32_mmu_t::set_active_pagetable((u32_t)get_pdir(cpu));
    ia32_mmu_t::flush_tlb(true);
    TRACE_INIT("cpu pagetable activated (%x)\n", 
	       ia32_mmu_t::get_active_pagetable());
#endif
}

void SECTION(".init.memory") init_kernel_space()
{
    ASSERT(!kernel_space); // on UP kernel space is only initialized once

    kernel_space = (space_t*)kmem.alloc(kmem_space, sizeof(space_t));
    ASSERT(kernel_space);

    kernel_space->init_kernel_mappings();
    ia32_mmu_t::set_active_pagetable((u32_t)kernel_space->get_pdir(0));
}

/**********************************************************************
 *
 *                    space_t implementation
 *
 **********************************************************************/

bool space_t::sync_kernel_space(addr_t addr)
{
    if (this == kernel_space) return false;

    pgent_t::pgsize_e size = pgent_t::size_max;
    pgent_t * dst_pgent = this->pgent(page_table_index(size, addr));
    pgent_t * src_pgent = kernel_space->pgent(page_table_index(size, addr));

    /* (already valid) || (kernel space invalid) */
    if (dst_pgent->is_valid(this, size) || 
	(!src_pgent->is_valid(kernel_space, size)))
    {
#if 0
	TRACE("sync ksp @ %p (src=%p (%d), dst=%p (%d))\n", 
	      addr, kernel_space, src_pgent->is_valid(kernel_space, size),
	      this, dst_pgent->is_valid(this, size));
#endif
	return false;
    }

#if !defined(CONFIG_SMP)
    *dst_pgent = *src_pgent;
#else
    for (unsigned cpu = 0; cpu < CONFIG_SMP_MAX_CPUS; cpu++)
	*this->pgent(page_table_index(size, addr), cpu) = *kernel_space->pgent(page_table_index(size, addr), cpu);
#endif
    return true;
}

/**
 * space_t::init initializes the space_t
 *
 * maps the kernel area and initializes shadow ptabs etc.
 */
void space_t::init(fpage_t utcb_area, fpage_t kip_area)
{
#warning VU: space_t has to be restructured having an init and activate function
    for (addr_t addr = (addr_t)KERNEL_AREA_START;
	 addr < (addr_t)KERNEL_AREA_END;
	 addr = addr_offset(addr, IA32_PAGEDIR_SIZE))
	sync_kernel_space(addr);

    this->x[0].utcb_area = utcb_area;
    this->x[0].kip_area = kip_area;
    
    // map kip read-only to user
    add_mapping(kip_area.get_base(), virt_to_phys((addr_t)get_kip()), PGSIZE_KIP, false, false);
}

void space_t::free_pagetables(fpage_t fpage)
{
    pgent_t * ptab;
    pgent_t::pgsize_e size = pgent_t::size_max;

    //TRACEF("freeing pagetables (%p->%p)\n", fpage.get_address(), addr_offset(fpage.get_address(), fpage.get_size()));
    for (word_t idx = page_table_index(size, fpage.get_base());
	idx <= page_table_index(size, addr_offset(fpage.get_base(), fpage.get_size()));
	idx++)
    {
	ptab = this->pgent(idx);
	//TRACEF("ptab=%p, idx=%d, valid=%d, subtree=%d\n", ptab, idx, ptab->is_valid(this, size), ptab->is_subtree(this, size));
	if (ptab->is_valid(this, size) && ptab->is_subtree(this, size))
	    ptab->remove_subtree(this, size, true);
    }
}


void space_t::allocate_tcb(addr_t addr)
{
    //enter_kdebug("allocate tcb");

    addr_t page = kmem.alloc(kmem_tcb, IA32_PAGE_SIZE);
    ASSERT(page);
    //TRACEF("tcb=%p, page=%p\n", addr, page);

    // map tcb kernel-writable 
    kernel_space->add_mapping(addr, virt_to_phys(page), PGSIZE_KTCB, true, true);
    sync_kernel_space(addr);
}

void space_t::release_kernel_mapping (addr_t vaddr, addr_t paddr,
				      word_t log2size)
{
    // Free up memory used for UTCBs
    if (get_utcb_page_area ().is_addr_in_fpage (vaddr))
	kmem.free (kmem_utcb, phys_to_virt (paddr), 1UL << log2size);
}


/* precondition: this is a valid utcb location for the space */
utcb_t * space_t::allocate_utcb(tcb_t * tcb)
{
    ASSERT(tcb);
    addr_t utcb = (addr_t)tcb->get_utcb_location();

    /* walk ptab, to see if a page is already mapped */
    pgent_t::pgsize_e size = pgent_t::size_max;
    pgent_t * pgent = this->pgent(page_table_index(size, utcb));
    while (size > PGSIZE_UTCB && pgent->is_valid(this, size)) 
    {
	ASSERT(pgent->is_subtree(this, size));

	pgent = pgent->subtree(this, size);
	size--;
	pgent = pgent->next(this, size, page_table_index(size, utcb));
    }

    /* if pgent is valid a page is mapped, otherwise allocate a new one */
    if (pgent->is_valid(this, size))
	return (utcb_t*)phys_to_virt(addr_offset(pgent->address(this, size), 
						 (word_t)utcb & (~IA32_PAGE_MASK)));
    else 
    {
	// allocate new UTCB page
	addr_t page = kmem.alloc(kmem_utcb, IA32_PAGE_SIZE);
	ASSERT(page);
	add_mapping((addr_t)utcb, virt_to_phys(page), PGSIZE_UTCB, true, false);
	return (utcb_t*)addr_offset(page, (word_t)utcb & (~IA32_PAGE_MASK));
    }
}


void space_t::map_dummy_tcb(addr_t addr)
{
    //TRACEF("%p\n", addr);
    //enter_kdebug("map dummy");
    add_mapping(addr, (addr_t)get_dummy_tcb(), PGSIZE_KTCB, false, false);
}

void space_t::map_sigma0(addr_t addr)
{
    //TRACEF("%p\n", addr);
    add_mapping(addr, addr, pgent_t::size_4m, true, false);
}


/**********************************************************************
 *
 *                    global functions
 *
 **********************************************************************/

/**
 * exc_pagefault: trap gate for ia32 pagefault handler
 */
IA32_EXC_WITH_ERRORCODE(exc_pagefault, 0)
{
    u32_t pf = ia32_mmu_t::get_pagefault_address();
    //TRACEF("pagefault @ %p, ip=%p, sp=%p\n", pf, frame->eip, frame->esp);

    space_t * space = get_current_space();
    

    /* if the idle thread accesses the tcb area - 
     * we will get a pagefault with an invalid space
     * so we use CR3 to figure out the space
     */
    if (EXPECT_FALSE( space == NULL ))
	space = ptab_to_space(ia32_mmu_t::get_active_pagetable(), 
			      get_current_cpu());

    space->handle_pagefault(
	(addr_t)pf, 
	(addr_t)frame->eip,
	(space_t::access_e)(frame->error & IA32_PAGE_WRITABLE),
	(frame->error & 4) ? false : true);
}
