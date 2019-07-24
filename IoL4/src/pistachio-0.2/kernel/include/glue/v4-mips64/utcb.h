/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  University of New South Wales
 *                
 * File path:     glue/v4-mips64/utcb.h
 * Description:   UTCB definition
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
 * $Id: utcb.h,v 1.6 2003/05/01 14:56:35 sjw Exp $
 *                
 ********************************************************************/
#ifndef __GLUE__V4_MIPS64__UTCB_H__
#define __GLUE__V4_MIPS64__UTCB_H__

#include INC_API(thread.h)
#include INC_API(types.h)

class utcb_t {
public:
    bool allocate(threadid_t tid);
    void free();

public:
    threadid_t		my_global_id;		/* 0 */
    word_t		processor_no;		/* 8 */
    word_t		user_defined_handle;	/* 16 */
    threadid_t		pager;			/* 24 */
    threadid_t		exception_handler;	/* 32 */
    u8_t		preempt_flags;		/* 40 */
    u8_t		cop_flags;              /* 41 */
    u16_t		__reserved0;            /* 42 */
    u32_t               __reserved1;		/* 44 */
    timeout_t		xfer_timeout;		/* 48 */
    word_t		error_code;		/* 56 */
    threadid_t		intended_receiver;	/* 64 */
    threadid_t		virtual_sender;		/* 72 */
    word_t              __padding0[6];          /* 80 .. 128 */
    word_t		mr[IPC_NUM_MR];		/* 128 .. 640 */
    word_t		br[IPC_NUM_BR];		/* 640 .. 904 */
    word_t              __padding1[15];         /* 904 .. 1024 */
//}__attribute__((packed));
};


/**
 * try to allocate a UTCB and mark it as used
 * @param tid the global ID of the associated thread
 * @return true on success, false if UTCB is already allocated
 */
INLINE bool utcb_t::allocate(threadid_t tid)
{
#if defined(CONFIG_SMP)
# warning utcb_t::allocate not SMP safe
#endif
    if( this->my_global_id.is_nilthread() ) {
    	this->my_global_id.set_raw( tid.get_raw() );
	return true;
    }
    else
	return false;
}

/**
 * mark a UTCB as free
 */
INLINE void utcb_t::free()
{
    this->my_global_id = NILTHREAD;
}

#endif /* !__GLUE__V4_MIPS64__UTCB_H__ */
