/*********************************************************************
 *                
 * Copyright (C) 2003,  Karlsruhe University
 *                
 * File path:     elf.h
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
 * $Id: elf.h,v 1.2.2.1 2003/06/03 19:52:08 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __KICKSTART__ELF_H__
#define __KICKSTART__ELF_H__

/**
   \file        elf.h
   \brief       Rudimentary ELF file format structures

   Assumptions:

   We load only files for the platform we're being built for,
   i.e. ELF64 when L4_64BIT is set, ELF32 when L4_32BIT is set.
   
*/

#include <l4/types.h>

class ehdr_t 
{
public:
    unsigned char ident[16];
    L4_Word16_t type;
    L4_Word16_t machine;
    L4_Word32_t version;
    L4_Word_t   entry;          // Program start address
    L4_Word_t   phoff;          // File offset of program header table
    L4_Word_t   shoff;
    L4_Word32_t flags;
    L4_Word16_t ehsize;         // Size of this ELF header
    L4_Word16_t phentsize;      // Size of a program header
    L4_Word16_t phnum;          // Number of program headers
    L4_Word16_t shentsize;
    L4_Word16_t shnum;
    L4_Word16_t shstrndx;
};

class phdr_t
{
public:
    L4_Word32_t type;
#if defined(L4_64BIT)
    L4_Word32_t flags;
#endif
    L4_Word_t   offset;
    L4_Word_t   vaddr;
    L4_Word_t   paddr;
    L4_Word_t   fsize;
    L4_Word_t   msize;
#if defined(L4_32BIT)
    L4_Word32_t   flags;
#endif
    L4_Word_t   align;
};

enum phdr_type_e 
{
    PT_LOAD =   1       /* Loadable program segment */
};

#endif /* !__KICKSTART__ELF_H__ */
