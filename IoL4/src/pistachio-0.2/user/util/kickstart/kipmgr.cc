/*********************************************************************
 *                
 * Copyright (C) 2003,  Karlsruhe University
 *                
 * File path:     kipmgr.cc
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
 * $Id: kipmgr.cc,v 1.3 2003/05/01 18:46:25 uhlig Exp $
 *                
 ********************************************************************/

#include "kipmgr.h"
#include <l4/kip.h>

kip_manager_t::kip_manager_t()
{
    this->mem_desc_cnt = 0;
}

bool kip_manager_t::find_kip(L4_Word_t start, L4_Word_t end)
{
    // The KIP magic must be 4K aligned 
    for (L4_Word_t p = start & ~0xFFF; p < end; p += 0x1000)
    {
        if (((L4_KernelConfigurationPage_t*) p)->magic == L4_MAGIC)
        {
            kip = (L4_KernelConfigurationPage_t*) p;
            return true;
        }
    }
    return false;
}



void kip_manager_t::install_sigma0( L4_Word_t mod_start, L4_Word_t mod_end,
                                    L4_Word_t entry)
{
    kip->sigma0.low = mod_start;
    kip->sigma0.high = mod_end;
    kip->sigma0.ip = entry;
    
    //this->dedicate_memory(mod_start, mod_end, L4_BootLoaderSpecificMemoryType, 6);
}

void kip_manager_t::install_root_task( L4_Word_t mod_start, L4_Word_t mod_end,
                                    L4_Word_t entry )
{
    kip->root_server.low = mod_start;
    kip->root_server.high = mod_end;
    kip->root_server.ip = entry;

    //this->dedicate_memory(mod_start, mod_end, L4_BootLoaderSpecificMemoryType, 7);
}

void kip_manager_t::update_kip(L4_Word_t boot_info)
{
    this->kip->BootInfo = boot_info;

    this->kip->MemoryInfo.n = this->mem_desc_cnt;
}

bool kip_manager_t::dedicate_memory( L4_Word_t start, L4_Word_t end, L4_Word_t type, L4_Word_t sub_type )
{
    L4_MemoryDesc_t *desc = 
	L4_MemoryDesc( (void *)this->kip, this->mem_desc_cnt );
    if( desc == (L4_MemoryDesc_t *)0 )
    {
        return false;
    }

    desc->x.type = type;
    desc->x.t = sub_type;
    desc->x.v = 0;
    desc->x.low = start >> 10;
    desc->x.high = (end + (1 << 10) - 1) >> 10;

    this->mem_desc_cnt++;

    return true;
}
