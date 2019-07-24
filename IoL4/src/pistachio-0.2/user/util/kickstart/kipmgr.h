/****************************************************************************
 *
 * Copyright (C) 2002-2003, Karlsruhe University
 *
 * File path:	include/piggybacker/kip.h
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
 * $Id: kipmgr.h,v 1.1 2003/04/16 22:36:22 ud3 Exp $
 *
 ***************************************************************************/
#ifndef __KICKSTART__KIP_H__
#define __KICKSTART__KIP_H__

#include <l4/types.h>
#include <l4/kcp.h>
#include <l4/kip.h>

class kip_manager_t
{
protected:
    L4_KernelConfigurationPage_t *kip;

    L4_Word_t mem_desc_cnt;

public:
    kip_manager_t();

    bool find_kip( L4_Word_t kernel_start, L4_Word_t kernel_end );

    void install_sigma0( L4_Word_t mod_start, L4_Word_t mod_end,
                         L4_Word_t entry );
    void install_root_task( L4_Word_t mod_start, L4_Word_t mod_end,
                         L4_Word_t entry );
    void update_kip(L4_Word_t boot_info);
    bool dedicate_memory( L4_Word_t start, L4_Word_t end, L4_Word_t type, L4_Word_t sub_type );

};

#endif	/* __KICKSTART__KIP_H__ */
