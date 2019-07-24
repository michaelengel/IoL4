/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     glue/v4-ia64/resources.h
 * Description:   Resource definitions for ia64
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
 * $Id: resources.h,v 1.4 2003/03/05 09:06:44 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_IA64__RESOURCES_H__
#define __GLUE__V4_IA64__RESOURCES_H__

#define IA64_RESOURCE_FPU		(1UL << 0)
#define IA64_RESOURCE_COPYAREA		(1UL << 1)
#define IA64_RESOURCE_BREAKPOINT	(1UL << 2)
#define IA64_RESOURCE_PERFMON		(1UL << 3)

enum resource_type_e {
    fpu		= 0,
    copy_area	= 1,
    breakpoint	= 2,
    perfmon	= 3,
};

class thread_resources_t : public generic_thread_resources_t
{
    word_t partner_rid;

public:
    void save (tcb_t * tcb, tcb_t * dest);
    void load (tcb_t * tcb);
    void purge (tcb_t * tcb);
    void init (tcb_t * tcb);
    void free (tcb_t * tcb);
    void dump (tcb_t * tcb);

    void enable_copy_area (tcb_t * tcb, tcb_t * partner);
    void disable_copy_area (tcb_t * tcb, bool disable_resource);
    void enable_global_breakpoint (tcb_t * tcb);
    void disable_global_breakpoint (tcb_t * tcb);
    void enable_global_perfmon (tcb_t * tcb);
    void disable_global_perfmon (tcb_t * tcb);
};


#endif /* !__GLUE__V4_IA64__RESOURCES_H__ */
