/****************************************************************************
 *                
 * Copyright (C) 2002, Karlsruhe University
 *                
 * File path:	arch/powerpc/pvr.h
 * Description:	Constants and functions related to the Processor Version
 * 		Register.
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
 * $Id: pvr.h,v 1.2 2002/02/19 16:17:45 joshua Exp $
 *
 ***************************************************************************/

#ifndef __ARCH__POWERPC__PVR_H__
#define __ARCH__POWERPC__PVR_H__

#define PVR_VERSION(pvr)	(((pvr) >> 16) & 0x0000ffff)
#define PVR_REVISION(pvr)	(((pvr) >>  0) & 0x0000ffff)

#define PPC_IS_SIMULATOR(pvr)	(pvr == 0)

#ifndef ASSEMBLY
INLINE u32_t ppc_get_pvr( void )
{
	u32_t pvr;
	asm ("mfspr %0, 287" : "=r" (pvr) );
	return pvr;
}
#endif	/* ASSEMBLY */

#endif	/* __ARCH__POWERPC__PVR_H__ */

