/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  University of New South Wales
 *                
 * File path:     glue/v4-mips64/memcontrol.cc
 * Description:   Temporary memory_control implementation
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
 * $Id: memcontrol.cc,v 1.2 2003/05/01 14:56:45 sjw Exp $
 *                
 ********************************************************************/

#include INC_API(config.h)
#include INC_API(tcb.h)
#include INC_API(thread.h)
#include INC_API(fpage.h)
#include INC_GLUE(syscalls.h)
#include INC_API(syscalls.h)
#include INC_PLAT(cache.h)

#include <kdb/tracepoints.h>

DECLARE_TRACEPOINT(SYS_MEMORY_CONTROL);

enum attribute_e {
    a_l4default		= 0,
    a_uncached		= 1,
    a_write_back	= 2,
    a_write_through	= 3,
    a_write_through_noalloc = 4,
    a_flush		= 31,
};


#include INC_ARCH(pgent.h)
#include INC_API(space.h)
#include INC_GLUE(space.h)
#include <linear_ptab.h>

static inline addr_t address (fpage_t fp, word_t size)
{
    return (addr_t) (fp.raw & ~((1UL << size) - 1));
}

/**
 * @param fpage		fpage to change
 * @param attrib	new fpage attributes
 *
 * @returns 
 */
fpage_t attrib_fpage (fpage_t fpage, attribute_e attrib)
{
    pgent_t::pgsize_e size, pgsize;
    pgent_t * pg;
    //mapnode_t * map;
    addr_t vaddr;
    word_t num;

    pgent_t *r_pg[pgent_t::size_max];
    word_t r_num[pgent_t::size_max];
    space_t *space = get_current_space ();

/*
    TRACEPOINT (FPAGE_UNMAP,
		printf ("%s_fpage (f_spc=%p  f_fp=%p)\n",
			flush ? "flush" : "unmap", space, fpage.raw));
*/

    num = fpage.get_size_log2 ();
    vaddr = address (fpage, num);

    if (num < hw_pgshifts[0])
    {
	printf ("attrib_fpage(): invalid fpage size (%d)\n", num);
	enter_kdebug ("invalid fpage size");
	fpage.set_rwx (0);
	return fpage;
    }

    /*
     * Some architectures may not support a complete virtual address
     * space.  Enforce attrib to only cover the supported space.
     */

    if (num > hw_pgshifts[pgent_t::size_max+1])
	num = hw_pgshifts[pgent_t::size_max+1];

    /*
     * Find pagesize to use, and number of pages to map.
     */

    for (pgsize = pgent_t::size_max; hw_pgshifts[pgsize] > num; pgsize--) {}

    num = 1UL << (num - hw_pgshifts[pgsize]);
    size = pgent_t::size_max;
    pg = space->pgent (page_table_index (size, vaddr));

    while (num)
    {
	translation_t::memattrib_e new_att = translation_t::l4default;

	if (! space->is_user_area (vaddr))
	    /* Do not mess with kernel area. */
	    break;

	if (size > pgsize)
	{
	    /* We are operating on too large page sizes. */
	    if (! pg->is_valid (space, size))
		break;
	    else if (pg->is_subtree (space, size))
	    {
		size--;
		pg = pg->subtree (space, size+1)->next
		    (space, size, page_table_index (size, vaddr));
		continue;
	    }
	    else
	    {
		enter_kdebug ("fpage_attrib: page is to large");
		break;
	    }
	}

	if (! pg->is_valid (space, size))
	    goto Next_entry;

	if (pg->is_subtree (space, size))
	{
	    /* We have to modify each single page in the subtree. */
	    size--;
	    r_pg[size] = pg;
	    r_num[size] = num - 1;

	    pg = pg->subtree (space, size+1);
	    num = page_table_size (size);
	    continue;
	}

#if 0
	/* Only attrib from mapping database if user-mapping. */
	if (is_mappable (vaddr))
	{
	    map = pg->mapnode (space, size,
			       addr_mask (vaddr, ~page_mask (size)));

	    rwx |= mdb_flush (map, pg, size, vaddr, pgsize, fpage, flush);
	    if (! flush)
		map->set_rwx (0);
	}
	else if (unmap_all)
	{
	    release_kernel_mapping (vaddr, pg->address (space, size),
				    page_shift (size));

	    pg->clear (space, size, true, vaddr);
	    if (! space_t::does_tlbflush_pay (fpage.get_size_log2 ()))
		flush_tlbent (get_current_space (), vaddr, page_shift (size));
	}
#endif

	if (space->is_mappable (vaddr))
	{
	    space->flush_tlbent (space, vaddr, page_shift (size));

	    switch (attrib)
	    {
	    case a_l4default: new_att = translation_t::l4default; break;
	    case a_uncached: new_att = translation_t::uncached; break;
	    case a_write_back: new_att = translation_t::write_back; break;
	    case a_write_through: new_att = translation_t::write_through; break;
	    case a_write_through_noalloc: new_att = translation_t::write_through_noalloc; break;
	    case a_flush:
		UNIMPLEMENTED();
	    	break;
	    default: enter_kdebug ("invalid attribute\n"); break;
	    }
	    pg->translation() -> set_attrib (new_att);
	}

    Next_entry:

	pg = pg->next (space, size, 1);
	vaddr = addr_offset (vaddr, page_size (size));
	num--;
    }

    return fpage;
}

SYS_MEMORY_CONTROL (word_t control, word_t attribute0, word_t attribute1,
		    word_t attribute2, word_t attribute3)
{
    tcb_t * current = get_current_tcb();
    space_t *space = current->get_space();
    word_t fp_idx, att;

    TRACEPOINT (SYS_MEMORY_CONTROL, 
		printf ("SYS_MEMORY_CONTROL: control=%lx, attribute0=%lx, "
			"attribute1=%lx, attribute2=%lx, attribute3=%lx\n",
    			control, attribute0, attribute1, attribute2,
    			attribute3));

    // invalid request - thread not privileged
    if (!is_priviledged_space(get_current_space()))
	return_memory_control(0);

    // flush cpu cache
    if (control == -2UL)
    {
	cache_t::flush_cache_all();
	return_memory_control(1);
    }

    ASSERT(control < (1<<6));

    for (fp_idx = 0; fp_idx <= control; fp_idx++)
    {
	fpage_t fpage;
	addr_t addr;
	pgent_t * pg;
	pgent_t::pgsize_e pgsize;

	fpage.raw = current->get_mr(fp_idx);

	/* nil pages act as a no-op */
	if (fpage.is_nil_fpage() )
	    continue;

	switch(fpage.raw & 0x3)
	{
	    case 0: att = attribute0; break;
	    case 1: att = attribute1; break;
	    case 2: att = attribute2; break;
	    default: att = attribute3; break;
	}

	addr = address (fpage, fpage.get_size_log2 ());
	// Check if mapping exist in page table
	if (!space->lookup_mapping (
	    addr, &pg, &pgsize))
	{
	    if (!is_sigma0_space(current->get_space()))
		enter_kdebug("page does not exist! Request, not from sigma0");

	    space->map_sigma0(addr);
	}

	attrib_fpage(fpage, (attribute_e)att);
    }

    return_memory_control(1);
}
