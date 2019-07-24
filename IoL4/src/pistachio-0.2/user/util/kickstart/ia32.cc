/*********************************************************************
 *                
 * Copyright (C) 2003,  Karlsruhe University
 *                
 * File path:     ia32.cc
 * Description:   IA-32 specific implementation fragments of kickstart
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
 * $Id: ia32.cc,v 1.5.2.2 2003/06/23 13:45:25 ud3 Exp $
 *                
 ********************************************************************/


// Reserve this much conventional memory for the kernel
#define ADDITIONAL_KMEM_SIZE    (16*1024*1024)

// The kernel cannot use memory beyond this limit
#define MAX_KMEM_END            (240*1024*1024)

// For alignment mess
#define SUPERPAGESIZE           (4*1024*1024)

#include <config.h>
#include <l4io.h>
#include <l4/types.h>
#include "mbi.h"                // MultiBoot Info structure
#include "kipmgr.h"             // KIP management

void fail(int ec)
{
    printf("PANIC: FAIL in line %d\n", ec);
    while(1);
}


void flush_cache()
{
    __asm__ __volatile__ ("wbinvd");
}



/*
 * Start kernel at its entry point. No preconditions
 */
void launch_kernel(L4_Word_t entry)
{
    __asm__ __volatile__ ("jmp *%0" : : "r"(entry));
}




class mmap_t {
public:
    L4_Word32_t   desc_size;
    L4_Word64_t   base;
    L4_Word64_t   size;
    L4_Word32_t   type;
};

void install_memory(mbi_t* mbi, kip_manager_t* kip)
{
    // Does the MBI contain a reference to the BIOS memory map?
    if (mbi->flags.mmap)
    {
        // Pointer to first entry in table
        mmap_t* m = (mmap_t*) mbi->mmap_addr;

        // Iterate over all entries
        while ((L4_Word_t) m < (mbi->mmap_addr + mbi->mmap_length))
        {
            /* Mark "usable" memory (type=1) as conventional physical
               memory, everything else as architecture specific with
               the BIOS memory map type as subtype */
            kip->dedicate_memory(m->base, m->base+m->size,
                                 (m->type == 1)
                                 ? L4_ConventionalMemoryType
                                 : L4_ArchitectureSpecificMemoryType,
                                 m->type);

            /* Skip forward by the number of bytes specified in the
               structure. This can be more than just the 24 bytes */
            m = (mmap_t*) ((L4_Word_t) m + 4 + m->desc_size);
        }
        
        /* The standard PC's VGA memory hasn't been seen in any BIOS
         * memory map so far. So we fake an entry for it. */
        kip->dedicate_memory(0xA0000, 0xC0000, 
                             L4_SharedMemoryType, 0);


        // Second round: Find a suitable KMEM area
        m = (mmap_t*) mbi->mmap_addr;
        // Iterate over all entries
        while ((L4_Word_t) m < (mbi->mmap_addr + mbi->mmap_length))
        {
            if ((m->type == 1) &&
                (m->size >= ADDITIONAL_KMEM_SIZE) &&
                (m->base <= (MAX_KMEM_END-ADDITIONAL_KMEM_SIZE)))
            {
                L4_Word_t base, end;

                // Is the end of this block within kernel's reach?
                if ((m->base+m->size) < MAX_KMEM_END)
                {
                    // Yes
                    base = m->base + m->size - ADDITIONAL_KMEM_SIZE;
                    // Round up to next 4MB boundary. Some older SMP
                    // systems and most ACPI systems lack a few KB at
                    // the end of physical memory
                    base = (base + (SUPERPAGESIZE-1)) & ~(SUPERPAGESIZE-1);
                    end = m->base + m->size;
                }
                else
                {
                    base = (MAX_KMEM_END-ADDITIONAL_KMEM_SIZE);
                    end = MAX_KMEM_END;
                }
                // Mark the memory block as in use by the kernel
                kip->dedicate_memory(base, end, 
                                     L4_ReservedMemoryType, 0);
            }
            /* Skip forward by the number of bytes specified in the
               structure. This can be more than just the 24 bytes */
            m = (mmap_t*) ((L4_Word_t) m + 4 + m->desc_size);
        }
        
    }
    else
    {
        printf("Ooops! No BIOS memory map.\n");
        while(1);
    }
}


// prototype
void loader(mbi_t* mbi);

extern "C" void loader_entry(void)
{
    mbi_t* _mbi = mbi_t::prepare();

    mbi_t mbi = *_mbi;
    
    if (mbi.flags.mods)
    {
        /* Allocate a new, properly sized mods array on the
         * stack. This works as a malloc replacement as long as we
         * never leave this function. */
        mbi_module_t mods[mbi.modcount];
        // Copy all mods array members into new mods array
        for (L4_Word_t i = 0; i < mbi.modcount; i++)
            mods[i] = mbi.mods[i];

        mbi.mods = mods;
        /* Install the roottask's command line as the kernel command
           line in the MBI. By convention, the roottask is the third
           module. */
        if (mbi.modcount > 2)
            mbi.cmdline = mbi.mods[2].cmdline;
        loader(&mbi);
    }
}




/* We override the weak symbol putc() which may go to the (yet not
 * initialized) serial port with our own implementation that writes to
 * the screen. */

#define DISPLAY ((char*)0xb8000)
#define COLOR 15
#define NUM_LINES 25

extern "C" void putc(int c)
{
    unsigned int i;
    static unsigned __cursor = 160 * (NUM_LINES - 1);

    switch(c) {
    case '\r':
        break;
    case '\n':
        do
	{
	    DISPLAY[__cursor++] = ' ';
	    DISPLAY[__cursor++] = COLOR;
	}
        while (__cursor % 160 != 0);
        break;
    case '\t':
        do
	  {
	    DISPLAY[__cursor++] = ' ';
	    DISPLAY[__cursor++] = COLOR;
	}
        while (__cursor % 16 != 0);
        break;
    default:
        DISPLAY[__cursor++] = c;
        DISPLAY[__cursor++] = COLOR;
    }
    if (__cursor == (160 * NUM_LINES)) {
	for (i = (160 / sizeof (L4_Word_t));
	     i < (160 / sizeof (L4_Word_t)) * NUM_LINES;
	     i++)
	    ((L4_Word_t *) DISPLAY)[i - 160 / sizeof (L4_Word_t)]
	      = ((L4_Word_t *) DISPLAY)[i];
	for (i = 0; i < 160 / sizeof (L4_Word_t); i++)
	    ((L4_Word_t *) DISPLAY)[160 / sizeof (L4_Word_t)
				  * (NUM_LINES-1) + i] = 0;
	__cursor -= 160;
    }
}

