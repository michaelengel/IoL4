/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  Karlsruhe University
 *                
 * File path:     platform/pc99/intctrl-pic.cc
 * Description:   Implementation of class handling the PIC cascade in
 *                standard PCs
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
 * $Id: intctrl-pic.cc,v 1.9 2003/03/11 10:25:20 uhlig Exp $
 *                
 ********************************************************************/

#include <debug.h>
#include INC_GLUE(intctrl.h)
#include INC_GLUE(idt.h)
#include INC_ARCH(trapgate.h)

/* the global interrupt controller */
intctrl_t intctrl;


#define IA32_HW_IRQ(num)					\
extern "C" void hwirq_##num();					\
void hwirq_##num##_wrapper()					\
{								\
    __asm__ (							\
        ".global hwirq_"#num"				\n"	\
	"\t.type hwirq_"#num",@function			\n"	\
	"hwirq_"#num":					\n"	\
	"pusha			/* save regs	*/	\n"	\
	"pushl	$"#num"		/* irq number	*/	\n"	\
	"jmp	hwirq_common	/* common stuff	*/	\n"	\
	:							\
	:							\
	);							\
}								\

IA32_HW_IRQ( 0);
IA32_HW_IRQ( 1);
IA32_HW_IRQ( 2);
IA32_HW_IRQ( 3);
IA32_HW_IRQ( 4);
IA32_HW_IRQ( 5);
IA32_HW_IRQ( 6);
IA32_HW_IRQ( 7);
IA32_HW_IRQ( 8);
IA32_HW_IRQ( 9);
IA32_HW_IRQ(10);
IA32_HW_IRQ(11);
IA32_HW_IRQ(12);
IA32_HW_IRQ(13);
IA32_HW_IRQ(14);
IA32_HW_IRQ(15);

__asm__ (
    ".text						\n"
    "	.globl hwirq_common				\n"
    "	.type hwirq_common ,@function			\n"
    "hwirq_common:					\n"
    "pushl	$intctrl	/* this pointer */	\n"
    "call	intctrl_t_handle_irq			\n"
    "addl	$8,%esp		/* clear stack	*/	\n"
    "popa			/* restore regs	*/	\n"
    "iret						\n"
    );


void intctrl_t::init_arch()
{
    const int base = 0x20;
    /* setup the IDT */
#if 0
    for (int i=0; i<16; i++)
	idt.add_int_gate(base+i, (void(*)()) &hwirq_0+((hwirq1-hwirq0)*i));
#else    
    idt.add_int_gate(base+ 0, (void(*)()) &hwirq_0);
    idt.add_int_gate(base+ 1, (void(*)()) &hwirq_1);
    idt.add_int_gate(base+ 2, (void(*)()) &hwirq_2);
    idt.add_int_gate(base+ 3, (void(*)()) &hwirq_3);
    idt.add_int_gate(base+ 4, (void(*)()) &hwirq_4);
    idt.add_int_gate(base+ 5, (void(*)()) &hwirq_5);
    idt.add_int_gate(base+ 6, (void(*)()) &hwirq_6);
    idt.add_int_gate(base+ 7, (void(*)()) &hwirq_7);
    idt.add_int_gate(base+ 8, (void(*)()) &hwirq_8);
    idt.add_int_gate(base+ 9, (void(*)()) &hwirq_9);
    idt.add_int_gate(base+10, (void(*)()) &hwirq_10);
    idt.add_int_gate(base+11, (void(*)()) &hwirq_11);
    idt.add_int_gate(base+12, (void(*)()) &hwirq_12);
    idt.add_int_gate(base+13, (void(*)()) &hwirq_13);
    idt.add_int_gate(base+14, (void(*)()) &hwirq_14);
    idt.add_int_gate(base+15, (void(*)()) &hwirq_15);
#endif

    /* initialize master
       - one slave connected to pin 2
       - reports to vectors 0x20-0x21,0x23-0x27 */
    master.init(base, (1 << 2));

    /* initialize slave
       - its slave id is 2
       - reports to vectors 0x28-0x2F*/
    slave.init(base+8, 2);

    /* unmask the slave on the master */
    master.unmask(2);
}

    
void intctrl_t::handle_irq(word_t irq) 
{
    mask_and_ack(irq);
    //TRACE("IRQ %d raised\n", irq);
    handle_interrupt(irq);
}

