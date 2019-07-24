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
## $Id: Makefile.in,v 1.2 2003/05/01 08:44:48 sjw Exp $
##                
######################################################################

srcdir=		.
top_srcdir=	../..
top_builddir=	../../

include $(top_srcdir)/Mk/l4.base.mk

OBJCOPY=	$(TOOLPREFIX)objcopy
STRIP=		$(TOOLPREFIX)strip
NM=		$(TOOLPREFIX)nm

SIGMA0?=	$(top_builddir)/serv/sigma0/sigma0
ROOT_TASK?=	$(top_builddir)/apps/bench/pingpong/pingpong
#ROOT_TASK?=	$(top_builddir)/apps/l4test/l4test
KERNEL?=	$(kerneldir)/$(ARCH)-kernel
PRINT=		$(top_builddir)/lib/io/print.o

MODULES+=	sigma0.img root_task.img kernel.img
OBJS+=		$(PRINT)

TARGET=	$(ARCH)-loader
PROGRAM_DEPS=	$(SIGMA0) $(ROOT_TASK) $(KERNEL)

SRCS_powerpc=	ofppc/
SRCS_mips64=	platform/$(PLAT)/main.cc platform/$(PLAT)/head.S
SRCS_alpha=	platform/srm/main.cc platform/srm/head.S platform/srm/console.cc platform/srm/utils.cc platform/srm/alpha-divrem.S platform/srm/dispatch.S

SRCS=	common/elf.cc common/loader.cc $(SRCS_$(ARCH))

CFLAGS_mips64= -I$(top_srcdir)/contrib/elf-loader/include/mips64
CFLAGS_alpha= -I$(top_srcdir)/contrib/elf-loader/include/alpha -O2 -mno-fp-regs -freg-struct-return -mcpu=ev4

LDS_alpha = $(top_srcdir)/contrib/elf-loader/platform/srm/linker.lds
LDS_mips64 = $(top_srcdir)/contrib/elf-loader/platform/$(PLAT)/linker.lds

CFLAGS+=	-fno-builtin -I$(top_srcdir)/contrib/elf-loader/include $(CFLAGS_$(ARCH))

ifeq ($(ARCH),mips64)
ifeq ($(PLAT),u4600)
OFORMAT_mips64= elf32-bigmips
else
OFORMAT_mips64= elf32-littlemips
endif
endif

OFORMAT_alpha= elf64-alpha

OFORMAT= $(OFORMAT_$(ARCH))

pre-all:
	@if test ! -d common; then mkdir common; fi
	@if test ! -d platform; then mkdir platform; fi
	@if test ! -d platform/$(PLAT); then mkdir platform/$(PLAT); fi

extract_symbol = $(shell $(NM) $(1) | awk '$$3 == $(2) {print "0x"$$1;}')

KIP_ADDR = $(call extract_symbol,$(KERNEL),"kip")

CFLAGS += -DKIP_ADDR=$(KIP_ADDR)

include $(top_srcdir)/Mk/l4.prog.mk

.INTERMEDIATE: lds.tmp
lds.tmp: $(LDS_$(ARCH))
#	@$(CPP) $(PPFLAGS) -P -C $< -o $@
	cp $< lds.tmp

$(TARGET): $(MODULES) $(OBJS) lds.tmp
	@echo "Linking $(NAME) ($(TARGET))"
	$(LD) $(LDFLAGS) -Tlds.tmp -o $@ $(OBJS) $(MODULES) $(LIBS)
	@$(STRIP) -g $@
	@$(OBJCOPY) -O binary $@ $@.bin

kernel.img: $(KERNEL)
	@echo "SECTIONS { .mod_$(basename $@) : { *(.data) } }" > $@.bin.lds
	$(LD) -T $@.bin.lds -r --oformat $(OFORMAT) -o $@ -bbinary $^

sigma0.img: $(SIGMA0)
	@echo "SECTIONS { .mod_$(basename $@) : { *(.data) } }" > $@.bin.lds
	$(LD) -T $@.bin.lds -r --oformat $(OFORMAT) -o $@ -bbinary $^

root_task.img: $(ROOT_TASK)
	@echo "SECTIONS { .mod_root : { *(.data) } }" > $@.bin.lds
	$(LD) -T $@.bin.lds -r --oformat $(OFORMAT) -o $@ -bbinary $^

all: $(TARGET)

