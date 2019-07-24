/*********************************************************************
 *                
 * Copyright (C) 2002, 2003,  Karlsruhe University
 *                
 * File path:     glue/v4-ia32/resources.h
 * Description:   ia32 specific resources
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
 * $Id: resources.h,v 1.5 2003/04/25 18:46:43 uhlig Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_IA32__RESOURCES_H__
#define __GLUE__V4_IA32__RESOURCES_H__

#include INC_API(resources.h)

#define IA32_RESOURCE_FPU	(1 << 0)
#define IA32_RESOURCE_COPYAREA	(1 << 1)

class thread_resources_t : public generic_thread_resources_t
{
public:
    void dump(tcb_t * tcb);
    void save(tcb_t * tcb) __asm__ ("tcb_resources_save");
    void load(tcb_t * tcb) __asm__ ("tcb_resources_load");
    void purge(tcb_t * tcb);
    void init(tcb_t * tcb);
    void free(tcb_t * tcb);

public:
    void ia32_no_math_exception(tcb_t * tcb);
    addr_t enable_copy_area (tcb_t * tcb, tcb_t * partner, addr_t addr);
    void release_copy_area (tcb_t * tcb, bool disable_copyarea);
    addr_t copy_area_address (word_t n);
    addr_t copy_area_real_address (word_t n);

private:
    addr_t fpu_state;
    word_t last_copy_area;
    word_t copy_area_pgdir_idx[COPY_AREA_COUNT];
};


#endif /* !__GLUE__V4_IA32__RESOURCES_H__ */
