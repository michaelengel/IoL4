/*********************************************************************
 *                
 * Copyright (C) 2002-2003,  Karlsruhe University
 *                
 * File path:     kdb/generic/print.cc
 * Description:   Implementation of printf
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
 * $Id: print.cc,v 1.59 2003/04/07 21:46:33 skoglund Exp $
 *                
 ********************************************************************/
#include <stdarg.h>	/* for va_list, ... comes with gcc */

#include <sync.h>	/* spinlocks	*/
#include INC_API(thread.h)
#include INC_API(tcb.h)
#include <linear_ptab.h>

#define SEC_KDEBUG	".kdebug"

extern void putc(const char c);


/* convert nibble to lowercase hex char */
#define hexchars(x) (((x) < 10) ? ('0' + (x)) : ('a' + ((x) - 10)))

/**
 *	Print hexadecimal value
 * 
 *	@param val	value to print
 *	@param width	width in caracters
 *
 *	Prints a hexadecimal value with leading zeroes of given width
 *	using putc().
 * 
 *	@returns the number of charaters printed (should be same as width).
 */
static int SECTION(SEC_KDEBUG) print_hex(word_t val, int width)
{
    int i, n = 0;
    if (!width) 
    {
	while( (val >> (4*width)) &&
	       ((unsigned)width < 2*sizeof(val)) ) width++;
	if (!width) width = 1;
    }
    for ( i = 4*(width-1); i >= 0; i -= 4, n++ )
	putc(hexchars((val >> i) & 0xF));
    return n;
}

/**
 *	Print a string
 *
 *	@param s	zero-terminated string to print
 *	@param width	minimum width of printed string
 *
 *	Prints the zero-terminated string using putc().  The printed
 *	string will be right padded with space to so that it will be
 *	at least WIDTH characters wide.
 *
 *      @returns the number of charaters printed.
 */
static int SECTION(SEC_KDEBUG) print_string(char * s, int width = 0,
					    int precision = 0)
{
    int n = 0;

    for (;;)
    {
	if (*s == 0)
	    break;

	putc(*s++);
	n++;
	if (precision && n >= precision)
	    break;
    }

    while (n < width) { putc(' '); n++; }

    return n;
}


/**
 *	Print decimal value
 *
 *	@param val	value to print
 *	@param width	width of field
 *
 *	Prints a value as a decimal in the given WIDTH with leading
 *	whitespaces.
 *
 *	@returns the number of characters printed (may be more than WIDTH)
 */
static int SECTION(SEC_KDEBUG) print_dec(word_t val, int width)
{
    word_t divisor;
    int digits;

    /* estimate number of spaces and digits */
    for (divisor = 1, digits = 1; val/divisor >= 10; divisor *= 10, digits++);

    /* print spaces */
    for ( ; digits < width; digits++ )
	putc(' ');
    
    /* print digits */
    do {
	putc(((val/divisor) % 10) + '0');
    } while (divisor /= 10);

    /* report number of digits printed */
    return digits;
}


DEFINE_SPINLOCK(printf_spin_lock);


/**
 *	Does the real printf work
 *
 *	@param format_p		pointer to format string
 *	@param args		list of arguments, variable length
 *
 *	Prints the given arguments as specified by the format string.
 *	Implements a subset of the well-known printf plus some L4-specifics.
 *
 *	@returns the number of characters printed
 */
int SECTION(SEC_KDEBUG) do_printf(const char* format_p, va_list args)
{
    const char* format = format_p;
    int n = 0;
    int i = 0;
    int width = 8;
    int precision = 0;
 
#define arg(x) va_arg(args, x)

    printf_spin_lock.lock();

    /* sanity check */
    if (format == NULL)
    {
	printf_spin_lock.unlock();
	return 0;
    }

    while (*format)
    {
	switch (*(format))
	{
	case '%':
	    width = 0;
	reentry:
	    switch (*(++format))
	    {
		/* modifiers */
	    case '.':
		for (format++; *format >= '0' && *format <= '9'; format++)
		    precision = precision * 10 + (*format) - '0';
		format--;
		goto reentry;
	    case '0'...'9':
		width = width*10 + (*format)-'0';
		goto reentry;
		break;
	    case 'l':
		goto reentry;
		break;
	    case 'c':
		putc(arg(int));
		n++;
		break;
	    case 'd':
	    {
		long val = arg(long);
		if (val < 0)
		{
		    putc('-');
		    val = -val;
		}
		n += print_dec(val, width);
		break;
	    }
	    case 'u':
		n += print_dec(arg(long), width);
		break;
	    case 'p':
		width = sizeof (word_t) * 2;
	    case 'x':
		n += print_hex(arg(long), width);
		break;
	    case 's':
	    {
		char* s = arg(char*);
		if (s)
		    n += print_string(s, width, precision);
		else
		    n += print_string("(null)", width, precision);
	    }
	    break;
	    case 'T':
	    {
#if 1
		print_hex(arg(word_t), 2*sizeof(word_t));
#else
		l4_threadid_t tid = arg(l4_threadid_t);
		if (l4_is_invalid_id(tid))
		{
		    n += print_string("INVALID "); break;
		}
		if (l4_is_nil_id(tid))
		{
		    n += print_string("NIL_ID  "); break;
		}
		if (tid.raw < 17)
		{
		    n += print_string("IRQ_");
		    if (tid.raw < 11) { putc(' '); n++; }
		    n += print_dec(tid.raw-1); if (tid.raw < 11) putc(' ');
		    n += print_string("  ");
		}
		print_hex(tid.raw);
	    }
	    break;
	    case 't':
	    {
		l4_threadid_t tid = arg(l4_threadid_t);
		if (l4_is_invalid_id(tid))
		{
		    n += print_string("INVALID"); break;
		}
		if (l4_is_nil_id(tid))
		{
		    n += print_string("NIL_ID"); break;
		}
		if (tid.raw < 17)
		{
		    n += print_string("IRQ_");
		    n += print_dec(tid.raw-1);
		}
		else
		{
# if defined(CONFIG_VERSION_X0)
		    print_hex((u8_t) tid.x0id.task);
		    putc('.');
		    print_hex((u8_t) tid.x0id.thread);
		    n += 5;
# else
		    n += print_hex(tid.raw);
# endif
		}
#endif
	    }
	    break;
	    case '%':
		putc('%');
		n++;
		format++;
		continue;
	    default:
		n += print_string("?");
		break;
	    };
	    i++;
	    break;
	default:
	    putc(*format);
	    n++;
	    break;
	}
	format++;
    }

    printf_spin_lock.unlock();
    return n;
}

/**
 *	Flexible print function
 *
 *	@param format	string containing formatting and parameter type
 *			information
 *	@param ...	variable list of parameters
 *
 *	@returns the number of characters printed
 */
extern "C" int SECTION(SEC_KDEBUG) printf(const char* format, ...)
{
    va_list args;
    int i;

    va_start(args, format);
    i = do_printf(format, args);
    va_end(args);
    return i;
};
