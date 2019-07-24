/*********************************************************************
 *                
 * Copyright (C) 2002,  Karlsruhe University
 *                
 * File path:     kdb/generic/linear_ptab_dump.cc
 * Description:   Linear page table dump
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
 * $Id: linear_ptab_dump.cc,v 1.10 2002/08/29 13:50:31 joshua Exp $
 *                
 ********************************************************************/
#include <debug.h>
#include <kdb/cmd.h>
#include <kdb/kdb.h>
#include <kdb/input.h>
#include <linear_ptab.h>

#include INC_ARCH(pgent.h)
#include INC_API(tcb.h)



/**
 * cmd_dump_ptab: dump page table contents
 */
DECLARE_CMD (cmd_dump_ptab, root, 'p', "ptab", "dump page table");

CMD(cmd_dump_ptab, cg)
{
    static char spaces[] = "                                ";
    char * spcptr = spaces + sizeof (spaces) - 1;
    char * spcpad = spcptr - pgent_t::size_max * 2;

    space_t * space;
    addr_t vaddr;
    word_t num;
    pgent_t * pg;
    pgent_t::pgsize_e size;

    // Arrays to implement recursion
    pgent_t * r_pg[pgent_t::size_max];
    word_t r_num[pgent_t::size_max];

    // Get dump arguments
    vaddr = 0;
    space = get_space ("Space");
    size = pgent_t::size_max;
    num = page_table_size (size);

    pg = space->pgent (page_table_index (pgent_t::size_max, vaddr));

    while (num > 0)
    {
	if (pg->is_valid (space, size))
	{
	    if (pg->is_subtree (space, size))
	    {
		// Recurse into subtree
		printf ("%p [%p]:%s tree=%p\n", vaddr, pg->raw, spcptr,
			pg->subtree (space, size));

		size--;
		r_pg[size] = pg->next (space, size+1, 1);
		r_num[size] = num - 1;
		spcptr -= 2;
		spcpad += 2;

		pg = pg->subtree (space, size+1);
		num = page_table_size (size);
		continue;
	    }
	    else
	    {
		// Print valid mapping
		word_t pgsz = page_size (size);
		word_t rwx = pg->reference_bits (space, size, vaddr);
		printf ("%p [%p]:%s phys=%p  map=%p %s%3d%cB %c%c%c "
			"(%c%c%c) %s\n",
			vaddr, pg->raw, spcptr, pg->address (space, size),
			pg->mapnode (space, size, vaddr), spcpad,
			(pgsz >= GB (1) ? pgsz >> 30 :
			 pgsz >= MB (1) ? pgsz >> 20 : pgsz >> 10),
			pgsz >= GB (1) ? 'G' : pgsz >= MB (1) ? 'M' : 'K',
			pg->is_readable (space, size)   ? 'r' : '~',
			pg->is_writable (space, size)   ? 'w' : '~',
			pg->is_executable (space, size) ? 'x' : '~',
			rwx & 4 ? 'R' : '~',
			rwx & 2 ? 'W' : '~',
			rwx & 1 ? 'X' : '~',
			pg->is_kernel (space, size) ? "kernel" : "user");
	    }
	}

	// Goto next ptab entry
	vaddr = addr_offset (vaddr, page_size (size));
	pg = pg->next (space, size, 1);
	num--;

	while (num == 0 && size < pgent_t::size_max)
	{
	    // Recurse up from subtree
	    pg = r_pg[size];
	    num = r_num[size];
	    size++;
	    spcptr += 2;
	    spcpad -= 2;
	}
    }

    return CMD_NOQUIT;
}

