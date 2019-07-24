/*********************************************************************
 *                
 * Copyright (C) 2003,  Karlsruhe University
 *                
 * File path:     kickstart.cc
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
 * $Id: kickstart.cc,v 1.5.2.1 2003/06/03 19:52:08 skoglund Exp $
 *                
 ********************************************************************/
/* startup stuff */

#include <config.h>

#include <l4io.h>

#include "mbi.h"
#include "elf.h"
#include "kipmgr.h"

// Prototypes for architecture-specific functions
void install_memory(mbi_t* mbi, kip_manager_t* kip);
void flush_cache(void);
void launch_kernel(L4_Word_t entry);
void fail(int ec);
#define FAIL() do { fail(__LINE__); } while (0)



/*
 * We make the memcopy function weak to allow architecture-specific
 * implementations take precedence.
 */
void memcopy(L4_Word_t dst, L4_Word_t src, L4_Word_t len) __attribute__((weak));

/**
 * Copy a block of memory
 *
 * @param dst   destination address
 * @param src   source address
 * @param len   length of memory block in bytes
 *
 * This function copies a block of memory byte-by-byte from the source
 * address to the destination address. The function does not check for
 * overlapping of source and destination region.
 */
void memcopy(L4_Word_t dst, L4_Word_t src, L4_Word_t len)
{
    L4_Word8_t* s = (L4_Word8_t*) src;
    L4_Word8_t* d = (L4_Word8_t*) dst;
    
    while (len--)
        *d++ = *s++;
}


/**
 * ELF-load an ELF memory image
 *
 * @param file_start    First byte of source ELF image in memory
 * @param file_end      First byte behind source ELF image in memory
 * @param memory_start  Pointer to address of first byte of loaded image
 * @param memory_end    Pointer to address of first byte behind loaded image
 * @param entry         Pointer to address of entry point
 *
 * This function ELF-loads an ELF image that is located in memory.  It
 * interprets the program header table and copies all segments marked
 * as load. It stores the lowest and highest+1 address used by the
 * loaded image as well as the entry point into caller-provided memory
 * locations.
 *
 * @returns false if ELF loading failed, true otherwise.
 */
bool elf_load(L4_Word_t file_start,
              L4_Word_t file_end,
              L4_Word_t *memory_start,
              L4_Word_t *memory_end,
              L4_Word_t *entry)
{
    // Pointer to ELF file header
    ehdr_t* eh = (ehdr_t*) file_start;

    // Is it an ELF file? Check ELF magic
    if (!((eh->ident[0] == 0x7F) &&
          (eh->ident[1] == 'E') &&
          (eh->ident[2] == 'L') &&
          (eh->ident[3] == 'F')))
    {
        // No. Bail out
        printf("  No ELF header at %p\n", (void *)file_start);
        return false;
    }

    // Is it an executable?
    if (eh->type != 2)
    {
        // No. Bail out
        printf("  No executable\n");
        return false;
    }

    // Is the address of the PHDR table valid?
    if (eh->phoff == 0)
    {
        // No. Bail out
        printf("Wrong PHDR table offset\n");
        return false;
    }

        
    printf("   => %p\n", (void *)eh->entry);

    // Locals to find the enclosing memory range of the loaded file
    L4_Word_t max_addr = 0;
    L4_Word_t min_addr = ~0;

    // Walk the program header table
    for (L4_Word_t i = 0; i < eh->phnum; i++)
    {
        // Locate the entry in the program header table
        phdr_t* ph = (phdr_t*) (file_start + eh->phoff + eh->phentsize * i);
        
        // Is it to be loaded?
        if (ph->type == PT_LOAD)
        {
            printf("  (%p,%p)->%p\n",
                   (void *)(file_start + ph->offset), (void *)ph->fsize, (void *)ph->paddr);
            // Copy bytes from "file" to memory - load address
            memcopy(ph->paddr, file_start + ph->offset, ph->fsize);
            // Update min and max
            min_addr <?= ph->paddr;
            max_addr >?= ph->paddr + (ph->msize >? ph->fsize);
        }
        
    }
    // Write back the values into the caller-provided locations
    *memory_start = min_addr;
    *memory_end = max_addr;
    *entry = eh->entry;

    // Signal successful loading
    return true;
}


/**
 * Load L4 system modules as referenced by the MBI structure
 *
 * @param mbi   Pointer to MultiBoot info structure
 *
 * The first three modules in the MBI modules list are ELF-loaded. The
 * memory range of each of these modules is updated with the enclosing
 * memory range of the loaded image and the entry point is set for
 * each successfully loaded image. Modules are loaded in the order
 * they appear in the MBI's module list. No checks for overlapping of
 * ELF images and loaded images are performed.
 *
 * @returns     true on successful load, false otherwise.
 */
bool load_modules(mbi_t* mbi)
{
    // Is the modules info in the MBI valid?
    if (mbi->flags.mods)
    {
        printf(" MBI.mods @ %p\n", mbi->mods);
        /* We need at least three modules: kernel, sigma0, roottask */
        if (mbi->modcount >= 3)
        {
#define LOADIT(idx)                                                     \
            if (!elf_load(mbi->mods[idx].start, mbi->mods[idx].end,     \
                          &mbi->mods[idx].start, &mbi->mods[idx].end,   \
                          &mbi->mods[idx].entry))                       \
            {                                                           \
                FAIL();                                                   \
            }

            printf(" kernel  ");
            LOADIT(0);
            printf(" sigma0  ");
            LOADIT(1);
            printf(" roottask");
            LOADIT(2);

            // Signal success
            return true;
        }
        else
        {
            FAIL();
        }
                
    }
    else
        FAIL();

    return false;
}

/**
 * Main kickstart loader function
 *
 * The procedure goes as follows:
 * - Find/prepare an MBI structure
 * - ELF-load the first three modules (kernel,sigma0,roottask)
 * - Find the KIP in the kernel
 * - Install memory descriptors from the MBI in the KIP
 * - Install initial servers (sigma0,roottask) in the KIP
 * - Store the bootinfo value in the KIP
 * - Flush caches
 * - Launch the kernel
 */

void loader(mbi_t* mbi)
{
    printf("KickStart 0.1\n");

    // 0 is not a valid address for the MBI
    if (mbi == 0)
    {
        printf("No MBI found\n");
        FAIL();
    }
    printf(" MBI @ %p\n", mbi);

    // Load the first three modules as ELF images into memory
    if (!load_modules(mbi))
    {
        // Bail out if loading failed
        printf("Failed to load all necessary modules\n");
        FAIL();
    }

    kip_manager_t kip;
    
    // The KIP is somewhere in the loaded kernel (module 0)
    if (!kip.find_kip(mbi->mods[0].start, mbi->mods[0].end))
    {
        // Bail out if we couldn't find a KIP
        FAIL();
    }

    // Set up the memory descriptors in the KIP
    install_memory(mbi, &kip);

    // Install sigma0's memory region and entry point in the KIP
    kip.install_sigma0(mbi->mods[1].start, mbi->mods[1].end,
                       mbi->mods[1].entry);
    // Install the root_task's memory region and entry point in the KIP
    kip.install_root_task(mbi->mods[2].start, mbi->mods[2].end,
                         mbi->mods[2].entry);
    
    // Install the bootinfo into the KIP
    kip.update_kip((L4_Word_t) mbi);
    
    // Flush caches (some archs don't like code in their D-cache)
    flush_cache();

    printf("Launching kernel ...\n");
    // Start the kernel (module 0) at its entry point
    launch_kernel(mbi->mods[0].entry);
    
    // We're not supposed to return from the kernel. Signal if we do
    FAIL();
}
