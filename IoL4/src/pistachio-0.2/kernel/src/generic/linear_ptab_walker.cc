/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     generic/linear_ptab_walker.cc
 * Description:   Linear page table manipulation
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
 * $Id: linear_ptab_walker.cc,v 1.36 2003/04/04 00:11:29 uhlig Exp $
 *                
 ********************************************************************/
#ifndef __GENERIC__LINEAR_PTAB_WALKER_CC__
#define __GENERIC__LINEAR_PTAB_WALKER_CC__

#include INC_ARCH(pgent.h)
#include INC_API(fpage.h)
#include INC_API(tcb.h)
#include INC_API(space.h)

#include <kdb/tracepoints.h>
#include <linear_ptab.h>
#include <mapping.h>


DECLARE_TRACEPOINT (FPAGE_MAP);
DECLARE_TRACEPOINT (FPAGE_UNMAP);
DECLARE_TRACEPOINT (MDB_MAP);
DECLARE_TRACEPOINT (MDB_UNMAP);

DECLARE_KMEM_GROUP (kmem_pgtab);

word_t hw_pgshifts[] = HW_PGSHIFTS;

class mapnode_t;


/*
 * Helper functions.
 */

static inline word_t base_mask (fpage_t fp, word_t size)
{
    return ((~0UL) >> ((sizeof (word_t) * 8) - fp.get_size_log2 ())) &
	~((~0UL) >> ((sizeof (word_t) * 8) - size));
}

static inline addr_t address (fpage_t fp, word_t size)
{
    return (addr_t) (fp.raw & ~((1UL << size) - 1));
}


/**
 * Map fpage to another address space.  If mapping is a grant
 * operation the mappings in the current address space are
 * subsequently removed.  The access bits in the send fpage indicate
 * which access rights the mappings in the destination address space
 * should have (access bits in the send fpage are first bitwise and'ed
 * with access bits of existing source mappings).
 *
 * @param snd_fp	fpage to map
 * @param base		send base
 * @param t_space	destination address space
 * @param rcv_fp	receive window in destination space
 * @param grant		is mapping a grant operation
 */
void space_t::map_fpage (fpage_t snd_fp, word_t base,
			 space_t * t_space, fpage_t rcv_fp,
			 bool grant)
{
    word_t offset, f_num, t_num, f_off;
    pgent_t *fpg, *tpg;
    pgent_t::pgsize_e f_size, t_size, pgsize;
    mapnode_t *newmap, *map;
    addr_t f_addr, t_addr;

    pgent_t * r_fpg[pgent_t::size_max];
    pgent_t * r_tpg[pgent_t::size_max];
    word_t r_fnum[pgent_t::size_max];
    word_t r_tnum[pgent_t::size_max];

    TRACEPOINT (FPAGE_MAP,
		printf ("%s_fpage (f_spc=%p  f_fp=%p  t_spc=%p  t_fp=%p)\n",
			grant ? "grant" : "map",
			this, snd_fp.raw, t_space, rcv_fp.raw));

    /*
     * Calculate the actual send and receive address to use.
     */

    if (snd_fp.get_size_log2 () <= rcv_fp.get_size_log2 ())
    {
	f_num = snd_fp.get_size_log2 ();
	base &= base_mask (rcv_fp, f_num);
	f_addr = address (snd_fp, f_num);
	t_addr = addr_offset (addr_mask (address (rcv_fp, f_num),
					 ~base_mask (rcv_fp, f_num)), base);
    }
    else
    {
	f_num = t_num = rcv_fp.get_size_log2 ();
	base &= base_mask (snd_fp, t_num);
	f_addr = addr_offset (addr_mask (address (snd_fp, t_num),
					 ~base_mask (snd_fp, t_num)), base);
	t_addr = address (rcv_fp, t_num);
    }

    if (f_num < hw_pgshifts[0])
    {
	if (f_num != 0)
	    enter_kdebug ("map_fpage(): invalid fpage size");
	return;
    }


    /*
     * Find pagesize to use, and number of pages to map.
     */

    for (pgsize = pgent_t::size_max;
	 (hw_pgshifts[pgsize] > f_num) || (!is_page_size_valid (pgsize));
	 pgsize--) {}

    f_num = t_num = 1UL << (f_num - hw_pgshifts[pgsize]);
    f_size = t_size = pgent_t::size_max;
    f_off = 0;

    fpg = this->pgent (page_table_index (f_size, f_addr));
    tpg = t_space->pgent (page_table_index (t_size, t_addr));

    while (f_num > 0 || t_num > 0)
    {
	if ((! is_user_area (f_addr) && ! is_sigma0_space (this)) ||
	    (! is_user_area (t_addr)))
	    /* Do not mess with kernel area. */
	    break;

	if (is_sigma0_space (this))
	{
	    /*
	     * When mapping from sigma0 we bypass the page table lookup.
	     */
	    f_size = pgsize;
	}
	else
	{
	    if (!fpg->is_valid (this, f_size))
	    {
		while (t_size < f_size)
		{
		    /* Recurse up. */
		    f_off += page_size (t_size);
		    tpg = r_tpg[t_size];
		    t_num = r_tnum[t_size];
		    t_size++;
		}

		if (t_size == f_size)
		    goto Next_receiver_entry;

		/* t_size > f_size */
		goto Next_sender_entry;
	    }

	    if ((f_size > pgsize) && fpg->is_subtree (this, f_size))
	    {
		/*
		 * We are currently working on to large page sizes.
		 */
		f_size--;
		fpg = fpg->subtree (this, f_size+1)->next
		    (this, f_size, page_table_index (f_size, f_addr));
		continue;
	    }
	    else if (fpg->is_subtree (this, f_size))
	    {
		/*
		 * The mappings in the senders address space are to small.
		 * We have to map each single entry in the subtree.
		 */
		f_size--;
		r_fpg[f_size] = fpg->next (this, f_size+1, 1);
		r_fnum[f_size] = f_num - 1;

		fpg = fpg->subtree (this, f_size+1);
		f_num = page_table_size (f_size);
		continue;
	    }
	    else if (f_size > pgsize)
	    {
		f_num = 1;
	    }
	}

	/*
	 * If we get here `fpg' is a valid mapping in the senders
	 * address space.
	 */


	if ((t_size > f_size) || ((t_size > pgsize) && (f_size > pgsize)))
	{
	    /*
	     * We are currently working on larger receive pages than
	     * send pages.
	     */
	    t_size--;
	    r_tpg[t_size] = tpg->next (t_space, t_size+1, 1);
	    r_tnum[t_size] = t_num - 1;

	    if (! tpg->is_valid (t_space, t_size+1))
	    {
		/*
		 * Subtree does not exist.  Create one.
		 */
		tpg->make_subtree (t_space, t_size+1, false);
	    }
	    else if (! tpg->is_subtree (t_space, t_size+1))
	    {
		/*
		 * There alredy exists a larger mapping.  Just
		 * continue.  BEWARE: This may cause extension of
		 * access rights to be refused even though they are
		 * perfectly legal.  I.e. if all the mappings in the
		 * subtree of the sender's address space are valid.
		 */
		printf ("map_fpage(): Larger mapping already exists.\n");
		enter_kdebug ("larger mapping exists");
		goto Next_receiver_entry;
	    }

	    if (t_size >= pgsize)
	    {
		tpg = tpg->subtree (t_space, t_size+1)->next
		    (t_space, t_size, page_table_index (t_size, t_addr));
		continue;
	    }
	    else
	    {
		tpg = tpg->subtree (t_space, t_size+1);
		t_num = page_table_size (t_size);
	    }

	    /* Adjust destination according to where source is located. */
	    tpg = tpg->next (t_space, t_size,
			     page_table_index (t_size, f_addr));
	    t_num -= page_table_index (t_size, f_addr);
	    t_addr = addr_offset (t_addr, page_table_index (t_size, f_addr) <<
				  hw_pgshifts[t_size]);
	    continue;
	}
	else if (tpg->is_valid (t_space, t_size) &&
		 tpg->is_subtree (t_space, t_size) )
	{
	    /*
	     * Target mappings are of smaller page size.  We have to
	     * map each single entry in the subtree.
	     */
	    t_size--;
	    r_tpg[t_size] = tpg->next (t_space, t_size+1, 1);
	    r_tnum[t_size] = t_num - 1;

	    tpg = tpg->subtree (t_space, t_size+1);
	    t_num = page_table_size (t_size);
	}
	else if (! is_page_size_valid (t_size))
	{
	    /*
	     * Pagesize is ok but is not a valid hardware pagesize.
	     * Need to create mappings of smaller size.
	     */
	    t_size--;
	    r_tpg[t_size] = tpg->next (t_space, t_size+1, 1);
	    r_tnum[t_size] = t_num - 1;
	    tpg->make_subtree (t_space, t_size+1, false);

	    tpg = tpg->subtree (t_space, t_size+1);
	    t_num = page_table_size (t_size);
	    continue;
	}

	if ((! is_mappable (f_addr) && ! is_sigma0_space (this)) ||
	    (! t_space->is_mappable (t_addr)))
	    goto Next_receiver_entry;

	/*
	 * If we get here `tpg' will be the page table entry that we
	 * are going to change.
	 */

	offset = (word_t) f_addr & page_mask (f_size) & ~page_mask (t_size);
	
	if (tpg->is_valid (t_space, t_size))
	{
	    /*
	     * If a mapping already exists, it might be that we are
	     * just extending the current access rights.
	     */
	    if (is_sigma0_space (this) ?
		(tpg->address (t_space, t_size) != f_addr) :
		(tpg->address (t_space, t_size) !=
		 addr_offset (fpg->address (this, f_size), offset)))
	    {
		printf ("map_fpage(from=%p, to=%p, base=%x, "
			"sndfp=%x, rcvfp=%x)\n", this, t_space, base,
			snd_fp.raw, rcv_fp.raw);
		enter_kdebug ("map_fpage(): Mapping already exists.");
		goto Next_receiver_entry;
	    }

	    /* Extend access rights. */
	    word_t rights = 0;
    
	    if (snd_fp.is_execute () && fpg->is_executable (this, f_size))
		rights += 1;
	    if (snd_fp.is_write () && fpg->is_writable (this, f_size))
		rights += 2;
	    if (snd_fp.is_read () && fpg->is_readable (this, f_size))
		rights += 4;
	    
	    tpg->update_rights (t_space, t_size, rights);
	}
	else
	{
	    /*
	     * This is where the real work is done.
	     */

	    if  (is_sigma0_space (this))
	    {
		/*
		 * If mapping from sigma0, fpg will not be a valid
		 * page table entry.
		 */

		TRACEPOINT (MDB_MAP,
			    word_t fsz = page_size (f_size);
			    word_t tsz = page_size (t_size);
			    printf ("mdb_map (from {sigma0 "
				    "pg=%p addr=%p %d%cB} to {"
				    "spc=%p pg=%p addr=%p %d%cB})  "
				    "paddr=%p\n",
				    fpg, addr_offset (f_addr, offset),
				    (fsz >= GB (1) ? fsz >> 30 :
				     fsz >= MB (1) ? fsz >> 20 : fsz >> 10),
				    (fsz >= GB (1) ? 'G' :
				     fsz >= MB (1) ? 'M' : 'K'),
				    t_space, tpg, t_addr,
				    (tsz >= GB (1) ? tsz >> 30 :
				     tsz >= MB (1) ? tsz >> 20 : tsz >> 10),
				    (tsz >= GB (1) ? 'G' :
				     tsz >= MB (1) ? 'M' : 'K'),
				    addr_offset (f_addr, offset + f_off)));

		newmap = mdb_map (sigma0_mapnode, fpg, pgent_t::size_max+1,
				  addr_offset (f_addr, offset + f_off),
				  tpg, t_size, t_space, grant);

		tpg->set_entry (t_space, t_size,
				addr_offset (f_addr, offset + f_off),
				snd_fp.is_read(), snd_fp.is_write (),
				snd_fp.is_execute(), false);
		tpg->set_linknode (t_space, t_size, newmap, t_addr);
	    }
	    else
	    {
		map = fpg->mapnode (this, f_size,
				    addr_mask (f_addr, ~page_mask (f_size)));

		TRACEPOINT (MDB_MAP,
			    word_t fsz = page_size (f_size);
			    word_t tsz = page_size (t_size);
			    printf ("mdb_map (node=%p from {"
				    "spc=%p pg=%p addr=%p %d%cB} to {"
				    "spc=%p pg=%p addr=%p %d%cB})  "
				    "paddr=%p\n",
				    map, this, fpg, f_addr,
				    (fsz >= GB (1) ? fsz >> 30 :
				     fsz >= MB (1) ? fsz >> 20 : fsz >> 10),
				    (fsz >= GB (1) ? 'G' :
				     fsz >= MB (1) ? 'M' : 'K'),
				    t_space, tpg, t_addr,
				    (tsz >= GB (1) ? tsz >> 30 :
				     tsz >= MB (1) ? tsz >> 20 : tsz >> 10),
				    (tsz >= GB (1) ? 'G' :
				     tsz >= MB (1) ? 'M' : 'K'),
				    addr_offset (fpg->address (this, f_size),
						 offset + f_off)));

		newmap = mdb_map (map, fpg, f_size, f_addr,
				  tpg, t_size, t_space, grant);

		tpg->set_entry
		    (t_space, t_size,
		     addr_offset (fpg->address (this, f_size), offset + f_off),
		     fpg->is_readable (this, f_size) && snd_fp.is_read (),
		     fpg->is_writable (this, f_size) && snd_fp.is_write (),
		     fpg->is_executable (this, f_size) && snd_fp.is_execute(),
		     false);
		tpg->set_linknode (t_space, t_size, newmap, t_addr);

		if (grant)
		{
		    /* Grant operation.  Remove mapping from current space. */
		    fpg->clear (this, f_size, false, f_addr);
		    flush_tlbent (get_current_space (), f_addr,
				  page_shift (f_size));
		}
	    }
	}

    Next_receiver_entry:

	t_addr = addr_offset (t_addr, page_size (t_size));
	t_num--;

	if (t_num > 0)
	{
	    /* Go to next entry */
	    tpg = tpg->next (t_space, t_size, 1);
	    if (t_size < f_size)
	    {
		f_off += page_size (t_size);
		continue;
	    }
	}
	else if (t_size < f_size && f_size < pgsize)
	{
	    /* Recurse up */
	    f_off += page_size (t_size);
	    tpg = r_tpg[t_size];
	    t_num = r_tnum[t_size];
	    t_size++;
	    continue;
	}
	else if (t_size > f_size)
	{
	    /* Skip to next fpg entry.  Happens if tpg is already mapped. */
	    f_addr = addr_offset (f_addr,
				  page_size (t_size) - page_size (f_size));
	    f_num = 1;
	}

    Next_sender_entry:

	f_addr = addr_offset (f_addr, page_size (f_size));
	f_off = 0;
	f_num--;

	if (f_num > 0)
	{
	    /* Go to next entry */
	    if (! is_sigma0_space (this))
		fpg = fpg->next (this, f_size, 1);
	    continue;
	}
	else if (f_size < pgsize)
	{
	    /* Recurse up */
	    fpg = r_fpg[f_size];
	    f_num = r_fnum[f_size];
	    f_size++;
	}
	else
	{
	    /* Finished */
	    t_num = 0;
	}
    }
}


/**
 * Unmaps or flushes indicated fpage.  Unmap indicates that the unmap
 * operation should exclude the current address space, flush indicates
 * that the unmap operation should include mapping in the current
 * space.  The access bits in the fpage indicates which rights to
 * revoke for the unmap operaion.  E.g., the "rwx" revokes all access
 * rights and removes mappings completely, whilst "w" only revoke
 * write permissions in the fpage.
 *
 * @param fpage		fpage to unmap
 * @param flush		does unmap operation also flush current space
 * @param unmap_all	also unmap kernel mappings (i.e., UTCB and KIP)
 *
 * @returns 
 */
fpage_t space_t::unmap_fpage (fpage_t fpage, bool flush, bool unmap_all)
{
    pgent_t::pgsize_e size, pgsize;
    pgent_t * pg;
    mapnode_t * map;
    addr_t vaddr;
    word_t num, rwx = 0;

    pgent_t *r_pg[pgent_t::size_max];
    word_t r_num[pgent_t::size_max];

    TRACEPOINT (FPAGE_UNMAP,
		printf ("%s_fpage (f_spc=%p  f_fp=%p)\n",
			flush ? "flush" : "unmap", this, fpage.raw));

    num = fpage.get_size_log2 ();
    vaddr = address (fpage, num);

    if (num < hw_pgshifts[0])
    {
	printf ("fpage_unmap(): invalid fpage size (%d)\n", num);
	enter_kdebug ("invalid fpage size");
	fpage.set_rwx (0);
	return fpage;
    }

    /*
     * Some architectures may not support a complete virtual address
     * space.  Enforce unmaps to only cover the supported space.
     */

    if (num > hw_pgshifts[pgent_t::size_max+1])
	num = hw_pgshifts[pgent_t::size_max+1];

    /*
     * Find pagesize to use, and number of pages to map.
     */

    for (pgsize = pgent_t::size_max; hw_pgshifts[pgsize] > num; pgsize--) {}

    num = 1UL << (num - hw_pgshifts[pgsize]);
    size = pgent_t::size_max;
    pg = this->pgent (page_table_index (size, vaddr));

    while (num)
    {
	if (! is_user_area (vaddr))
	    /* Do not mess with kernel area. */
	    break;

	if (size > pgsize)
	{
	    /* We are operating on too large page sizes. */
	    if (! pg->is_valid (this, size))
		break;
	    else if (pg->is_subtree (this, size))
	    {
		size--;
		pg = pg->subtree (this, size+1)->next
		    (this, size, page_table_index (size, vaddr));
		continue;
	    }
	    else if (fpage.is_rwx () && flush)
	    {
		printf ("fpage_unmap (%x, %x, %d)\n", this, fpage.raw, flush);
		enter_kdebug ("fpage_unmap: page is to large");
		break;
	    }
	}

	if (! pg->is_valid (this, size))
	    goto Next_entry;

	if (pg->is_subtree (this, size))
	{
	    /* We have to flush each single page in the subtree. */
	    size--;
	    r_pg[size] = pg;
	    r_num[size] = num - 1;

	    pg = pg->subtree (this, size+1);
	    num = page_table_size (size);
	    continue;
	}

	/* Only unmap from mapping database if user-mapping. */
	if (is_mappable (vaddr))
	{
	    map = pg->mapnode (this, size,
			       addr_mask (vaddr, ~page_mask (size)));

	    TRACEPOINT (MDB_UNMAP,
			word_t fsz = page_size (size);
			word_t tsz = page_size (pgsize);
			printf ("mdb_%s (spc=%p pg=%p map=%p vaddr=%p %c%c%c "
				"fsz=%d%cB tsz=%d%cB)  paddr=%p\n",
				flush ? "flush" : "unmap",
				this, pg, map, vaddr,
				fpage.is_read () ? 'r' : '~',
				fpage.is_write () ? 'w' : '~',
				fpage.is_execute () ? 'x' : '~',
				(fsz >= GB (1) ? fsz >> 30 :
				 fsz >= MB (1) ? fsz >> 20 : fsz >> 10),
				(fsz >= GB (1) ? 'G' : fsz >= MB (1) ? 'M' :
				 'K'),
				(tsz >= GB (1) ? tsz >> 30 :
				 tsz >= MB (1) ? tsz >> 20 : tsz >> 10),
				(tsz >= GB (1) ? 'G' : tsz >= MB (1) ? 'M' :
				 'K'),
				pg->address (this, size)));

	    rwx |= mdb_flush (map, pg, size, vaddr, pgsize, fpage, flush);
	    if (! flush)
		map->set_rwx (0);
	}
	else if (unmap_all)
	{
	    release_kernel_mapping (vaddr, pg->address (this, size),
				    page_shift (size));

	    pg->clear (this, size, true, vaddr);
	    if (! space_t::does_tlbflush_pay (fpage.get_size_log2 ()))
		flush_tlbent (get_current_space (), vaddr, page_shift (size));
	}

    Next_entry:

	pg = pg->next (this, size, 1);
	vaddr = addr_offset (vaddr, page_size (size));
	num--;

	while (num == 0 && size < pgsize)
	{
	    /* Recurse up */
	    pg = r_pg[size];
	    num = r_num[size];
	    size++;

	    fpage_t fp;
	    fp.set ((word_t) vaddr, page_size (size), false, false, false);

	    if (flush && fpage.is_rwx () && (unmap_all || is_mappable (fp)))
		pg->remove_subtree (this, size, false);

	    pg = pg->next (this, size, 1);	
	}
    }

    if (space_t::does_tlbflush_pay (fpage.get_size_log2 ()))
	flush_tlb (get_current_space ());

    fpage.set_rwx (rwx);
    return fpage;
}


/**
 * Read word from address space.  Parses page tables to find physical
 * address of mapping and reads the indicated word directly from
 * kernel-mapped physical memory.
 *
 * @param vaddr		virtual address to read
 * @param contents	returned contents in given virtual location
 *
 * @return true if mapping existed, false otherwise
 */
bool space_t::readmem (addr_t vaddr, word_t * contents)
{
    pgent_t * pg;
    pgent_t::pgsize_e pgsize;

    if (! lookup_mapping (vaddr, &pg, &pgsize))
	return false;

    addr_t paddr = pg->address (this, pgsize);
    paddr = addr_offset (paddr, (word_t) vaddr & page_mask (pgsize));
    paddr = addr_mask (paddr, ~(sizeof (word_t) - 1));

    *contents = readmem_phys (paddr);
    return true;
}


/**
 * Lookup mapping in address space.  Parses the page table to find a
 * mapping for the indicated virtual address.  Also returns located
 * page table entry and page size in R_PG and R_SIZE (if non-nil).
 *
 * @param vaddr		virtual address
 * @param r_pg		returned page table entry for mapping
 * @param r_size	returned page size for mapping
 *
 * @return true if mapping exists, false otherwise
 */
bool space_t::lookup_mapping (addr_t vaddr, pgent_t ** r_pg,
			      pgent_t::pgsize_e * r_size)
{
    pgent_t * pg = this->pgent (page_table_index (pgent_t::size_max, vaddr));
    pgent_t::pgsize_e pgsize = pgent_t::size_max;

    for (;;)
    {
	if (pg->is_valid (this, pgsize))
	{
	    if (pg->is_subtree (this, pgsize))
	    {
		// Recurse into subtree
		if (pgsize == 0)
		    return false;

		pg = pg->subtree (this, pgsize)->next
		    (this, pgsize-1, page_table_index (pgsize-1, vaddr));
		pgsize--;
	    }
	    else
	    {
		// Return mapping
		if (r_pg)
		    *r_pg = pg;
		if (r_size)
		    *r_size = pgsize;
		return true;
	    }
	}
	else
	    // No valid mapping or subtree
	    return false;
    }

    /* NOTREACHED */
    return false;
}

#endif /* !__GENERIC__LINEAR_PTAB_WALKER_CC__ */
