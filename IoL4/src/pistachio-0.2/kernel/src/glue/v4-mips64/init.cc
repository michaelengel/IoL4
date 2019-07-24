/*********************************************************************
 *                
 * Copyright (C) 2002-2003,   University of New South Wales
 *                
 * File path:     glue/v4-mips64/init.cc
 * Description:   
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
 * $Id: init.cc,v 1.14 2003/05/01 14:56:45 sjw Exp $
 *                
 ********************************************************************/

#include <mapping.h>

#include INC_API(kernelinterface.h)
#include INC_API(schedule.h)
#include INC_API(space.h)

#include INC_GLUE(memory.h)
#include INC_GLUE(intctrl.h)
#include INC_GLUE(timer.h)
#include INC_ARCH(tlb.h)
#include INC_API(syscalls.h)
#include INC_GLUE(syscalls.h)
#include INC_PLAT(cache.h)

#define BOOTMEM_PAGES (CONFIG_BOOTMEM_PAGES)

static void SECTION (".init") dump_info(void)
{
    printf("Kernel configuration:\n");
    printf("\tKTCB area: 0x%lx -> 0x%lx (0x%lx)\n", KTCB_AREA_START, KTCB_AREA_END, KTCB_AREA_SIZE);
}

static void SECTION (".init") init_cpu_cache(void)
{
    cache_t::init_cpu();
}

/*
 * Initialize kernel debugger with initial boot memory, and register
 * kernel memory in the kernel info page.
 */
static void SECTION (".init") init_bootmem (void)
{
    word_t bootmem_size = BOOTMEM_PAGES<<MIPS64_PAGE_BITS;

    extern char _bootstack_top[];
    addr_t start_bootmem = (addr_t)(_bootstack_top);
    addr_t start_bootmem_phys = virt_to_phys(start_bootmem);

    if ((word_t)start_bootmem_phys < (word_t)get_kip()->sigma0.mem_region.high)
	start_bootmem_phys = get_kip()->sigma0.mem_region.high;
    if ((word_t)start_bootmem_phys < (word_t)get_kip()->sigma1.mem_region.high)
	start_bootmem_phys = get_kip()->sigma1.mem_region.high;
    if ((word_t)start_bootmem_phys < (word_t)get_kip()->root_server.mem_region.high)
	start_bootmem_phys = get_kip()->root_server.mem_region.high;

    start_bootmem = phys_to_virt(start_bootmem_phys);

    addr_t end_bootmem = (addr_t)((word_t)start_bootmem + bootmem_size);
    addr_t end_bootmem_phys = (addr_t)((word_t)start_bootmem_phys + bootmem_size);

    kmem.init (start_bootmem, end_bootmem);

    /* carl - yuck! */
    get_kip ()->memory_info.n = 0;

    /* Register reservations in kernel info page. */
//    get_kip ()->reserved_mem0.set (
//		    addr_align (start_text_phys, KB(4)),
//		    addr_align_up (end_text_phys, KB (4)));
//    get_kip ()->reserved_mem1.set (
//		    addr_align (start_bootmem_phys, KB(4)),
//		    addr_align_up (end_bootmem_phys, KB (4)));
//    get_kip ()->dedicated_mem0.set ((addr_t)0, (addr_t)MIPS64_PAGE_SIZE); should be reserved!

    /* feed the kernel memory allocator */

    // Define the user's virtual address space.
    get_kip ()->memory_info.insert( memdesc_t::conventional, true,
	    (addr_t)0, (addr_t)(1ULL<<CONFIG_MIPS64_ADDRESS_BITS));

    /* Register reservations in kernel interface page. */
    get_kip ()->memory_info.insert( memdesc_t::conventional, false, 
	    (addr_t)MIPS64_PAGE_SIZE, addr_align (get_kip()->main_mem.high, KB(4)));
    get_kip ()->memory_info.insert( memdesc_t::reserved, false, 
	    addr_align_up (get_kip()->main_mem.high, KB(4)), (addr_t)~0UL);

    get_kip ()->memory_info.insert (memdesc_t::reserved, false,
		    addr_align (start_text_phys, KB(4)),
		    addr_align_up (end_text_phys, KB (4)));

    get_kip ()->memory_info.insert (memdesc_t::reserved, false,
		    addr_align (start_bootmem_phys, KB(4)),
		    addr_align_up (end_bootmem_phys, KB (4)));

    // Define the area reserved for the exception vectors.
    get_kip ()->memory_info.insert( memdesc_t::reserved, false, 
	    (addr_t)0, (addr_t)MIPS64_PAGE_SIZE);

#ifdef CONFIG_PLAT_ERPCN01
    get_kip ()->dedicated_mem0.set ((addr_t)0x14000000, (addr_t)0x14001000);
    get_kip ()->memory_info.insert( memdesc_t::dedicated, false, 
	    (addr_t)0x14000000, (addr_t)0x14001000);
#elif CONFIG_PLAT_U4600
    get_kip ()->dedicated_mem0.set ((addr_t)0x10000000, (addr_t)0x14001000);
    get_kip ()->memory_info.insert( memdesc_t::dedicated, false, 
	    (addr_t)0x10000000, (addr_t)0x14001000);
    get_kip ()->dedicated_mem1.set ((addr_t)0x1c800000, (addr_t)0x1cc00000);
    get_kip ()->memory_info.insert( memdesc_t::dedicated, false, 
	    (addr_t)0x1c800000, (addr_t)0x1cc00000);
#endif

}


/*
 * Setup MIPS CPU
 */
extern "C" void init_cpu(void);

/*
 * Setup the Page tables and ASIDs
 */
extern "C" void SECTION(".init") init_pageing(void)
{
    get_asid_cache()->init();

    get_asid_cache()->set_valid(0, CONFIG_MAX_NUM_ASIDS-1);

    init_tlb();

    /* Create and init kernel space */
    init_kernel_space();
}


/*
 * Setup MIPS Architecture
 */
extern "C" void SECTION(".init") init_arch(void)
{
    /* start and end are link variables */
    init_bootmem();
    
    /* initialize kernel interface page */
    get_kip()->init();

    /* initialise page tables */
    init_pageing();

    /* initialize mapping database */
    init_mdb ();
    
    /* initialize kernel debugger if any */
    if (get_kip()->kdebug_init)
	get_kip()->kdebug_init();

    /* configure IRQ hardware - global part */
    get_interrupt_ctrl()->init_arch();
    /* configure IRQ hardware - local part */
    get_interrupt_ctrl()->init_cpu();
    
    /* initialize the kernel's timer source */
    get_timer()->init_global();
    get_timer()->init_cpu();
}

/*
 * Entry point from ASM into C kernel
 * Precondition: paging is initialized with init_paging
 */
extern "C" void SECTION(".init") startup_system()
{
    init_cpu();
    init_cpu_cache();
    
//    init_platform();

    init_console();

    init_hello();

    init_arch();

    dump_info();

    /* initialize the scheduler */
    get_current_scheduler()->init();
    /* get the thing going - we should never return */
    get_current_scheduler()->start();
    
    printf("\nShould never get here!\nKernel Halted\n");
    /* make sure we don't fall off the edge */
    spin_forever(1);
}
