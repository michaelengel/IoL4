######################################################################
##                
## Copyright (C) 2003,  Karlsruhe University
##                
## File path:     Makefile.in
##                
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions
## are met:
## 1. Redistributions of source code must retain the above copyright
##    notice, this list of conditions and the following disclaimer.
## 2. Redistributions in binary form must reproduce the above copyright
##    notice, this list of conditions and the following disclaimer in the
##    documentation and/or other materials provided with the distribution.
## 
## THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
## ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
## IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
## ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
## FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
## DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
## OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
## HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
## LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
## OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
## SUCH DAMAGE.
##                
## $Id: Makefile.in,v 1.3 2003/04/30 03:30:00 sjw Exp $
##                
######################################################################

srcdir=		.
top_srcdir=	../..
top_builddir=	../../

include $(top_srcdir)/Mk/l4.base.mk


ia32_SRCS=	ia32-getc.cc ia32-putc.cc
ia64_SRCS=	ia64-getc.cc ia64-putc.cc
powerpc_SRCS=	powerpc-ofppc-getc.cc powerpc-ofppc-putc.cc
mips64_SRCS=	mips64-getc.cc mips64-putc.cc
alpha_SRCS=	alpha-getc.cc alpha-putc.cc alpha-divrem.S

CFLAGS_alpha = -O2 -mno-fp-regs -freg-struct-return -mcpu=ev4

LIBRARY=	io
SRCS=		get_hex.cc print.cc $($(ARCH)_SRCS)


include $(top_srcdir)/Mk/l4.lib.mk


test:
	@echo SRCS=${SRCS}
	@echo OBJS=${OBJS}
