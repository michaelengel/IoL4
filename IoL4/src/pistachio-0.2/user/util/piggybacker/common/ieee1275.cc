/****************************************************************************
 *
 * Copyright (C) 2002-2003, Karlsruhe University
 *
 * File path:	piggybacker/common/ieee1275.cc
 * Description:	Provides access to the Open Firmware client access callback.
 * 		Ripped out of the kernel.
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
 * $Id: ieee1275.cc,v 1.2 2003/04/02 13:49:31 joshua Exp $
 *
 ***************************************************************************/

#include <piggybacker/ieee1275.h>

__unused prom_entry_t prom_entry = 0;
__unused prom_handle_t prom_stdout = INVALID_PROM_HANDLE;
__unused prom_handle_t prom_stdin = INVALID_PROM_HANDLE;
__unused prom_handle_t prom_chosen = INVALID_PROM_HANDLE;
__unused prom_handle_t prom_options = INVALID_PROM_HANDLE;
__unused prom_handle_t prom_memory = INVALID_PROM_HANDLE;

int prom_get_path( prom_handle_t phandle, const char *service, char *path, int pathlen )
{
	struct {
		const char *service;
		int nargs;
		int nret;
		prom_handle_t phandle;
		char *path;
		int pathlen;
		int size;
	} args;

	args.service = service;
	args.nargs = 3;
	args.nret = 1;
	args.phandle = phandle;
	args.path = path;
	args.pathlen = pathlen;
	args.size = -1;
	(*prom_entry)( &args );
	if( args.size > -1 )
		path[args.size] = '\0';
	return args.size;
}

int prom_package_to_path( prom_handle_t phandle, char *path, int pathlen )
{
	return prom_get_path( phandle, "package-to-path", path, pathlen );
}

int prom_instance_to_path( prom_handle_t ihandle, char *path, int pathlen )
{
	return prom_get_path( ihandle, "instance-to-path", path, pathlen );
}

prom_handle_t prom_instance_to_package( prom_handle_t ihandle )
{
    struct {
	const char *service;
	int nargs;
	int nret;
	prom_handle_t ihandle;
	prom_handle_t phandle;
    } args;

    args.service = "instance-to-package";
    args.nargs = 1;
    args.nret = 1;
    args.ihandle = ihandle;
    args.phandle = INVALID_PROM_HANDLE;
    (*prom_entry)( &args );
    return args.phandle;
}

int prom_next_prop( prom_handle_t node, const char *prev_name, char *name )
{
	struct {
		const char *service;
		int nargs;
		int nret;
		prom_handle_t node;
		const char *prev_name;
		char *name;
		int flag;
	} args;

	args.service = "nextprop";
	args.nargs = 3;
	args.nret = 1;
	args.node = node;
	args.prev_name = prev_name;
	args.name = name;
	args.flag = -1;
	(*prom_entry)( &args );
	return args.flag;
}

int prom_read( prom_handle_t phandle, void *buf, int len )
{
	struct {
		const char *service;
		int nargs;
		int nret;
		prom_handle_t phandle;
		const void *buf;
		int len;
		int actual;
	} args;

	args.service = "read";
	args.nargs = 3;
	args.nret = 1;
	args.phandle = phandle;
	args.buf = buf;
	args.len = len;
	args.actual = -1;
	(*prom_entry)( &args );
	return args.actual;
}

int prom_write( prom_handle_t phandle, const void *buf, int len )
{
	struct {
		const char *service;
		int nargs;
		int nret;
		prom_handle_t phandle;
		const void *buf;
		int len;
		int actual;
	} args;

	args.service = "write";
	args.nargs = 3;
	args.nret = 1;
	args.phandle = phandle;
	args.buf = buf;
	args.len = len;
	args.actual = -1;
	(*prom_entry)( &args );
	return args.actual;
}

void prom_puts( const char *msg )
{
        unsigned start, current, len;
	char nl[] = "\r\n";

	start = current = 0;
	do {
		/*  Look for a newline.  */
		while( msg[current] && (msg[current] != '\n') )
			current++;

		/*  If we have a string, print it.  */
		len = current-start;
		if( len > 0 )
			prom_write( prom_stdout, msg, len );
		/*  Print a newline.  */
		prom_write( prom_stdout, nl, sizeof(nl) );

		/*  Skip the newline.  */
		if( msg[current] == '\n' )
			current++;
	} while( msg[current] );
}

prom_handle_t prom_find_device( const char *name )
{
	struct {
		char *service;
		int nargs;
		int nret;
		const char *name;
		prom_handle_t phandle;
	} args;

	args.service = "finddevice";
	args.nargs = 1;
	args.nret = 1;
	args.name = name;
	args.phandle = INVALID_PROM_HANDLE;
	(*prom_entry)( &args );
	return args.phandle;
}

int prom_get_prop( prom_handle_t phandle, const char *name, void *buf, int buflen )
{
	struct {
		const char *service;
		int nargs;
		int nret;
		prom_handle_t phandle;
		const char *name;
		void *buf;
		int buflen;
		int size;
	} args;

	args.service = "getprop";
	args.nargs = 4;
	args.nret = 1;
	args.phandle = phandle;
	args.name = name;
	args.buf = buf;
	args.buflen = buflen;
	args.size = -1;
	(*prom_entry)( &args );
	return args.size;
}

int prom_get_prop_len( prom_handle_t phandle, const char *name )
{
    struct {
		const char *service;
		int nargs;
		int nret;
		prom_handle_t phandle;
		const char *name;
		int size;
	} args;

	args.service = "getproplen";
	args.nargs = 2;
	args.nret = 1;
	args.phandle = phandle;
	args.name = name;
	args.size = -1;
	(*prom_entry)( &args );
	return args.size;
}

prom_handle_t prom_nav_tree( prom_handle_t node, const char *which )
{
	struct {
		const char *service;
		int nargs;
		int nret;
		prom_handle_t node;
		prom_handle_t out;
	} args;

	args.service = which;
	args.nargs = 1;
	args.nret = 1;
	args.node = node;
	args.out = INVALID_PROM_HANDLE;
	(*prom_entry)( &args );
	if( args.out == 0 )
		args.out = INVALID_PROM_HANDLE;
	return args.out;
}

__noreturn void prom_exit( void )
{
	struct {
		const char *service;
		int nargs;
		int nret;
	} args;

	args.service = "exit";
	args.nargs = 0;
	args.nret = 0;
	(*prom_entry)( &args );

	/*  We should never reach this point.  */
	while( 1 ) ;
}

__noreturn void prom_fatal( const char *msg )
{
	char caption[] = "\r\nfatal error: ";

	if( prom_stdout != INVALID_PROM_HANDLE ) {
		prom_write( prom_stdout, caption, sizeof(caption) );
		prom_puts( msg );
	}
	prom_exit();
}

void prom_enter( void )
{
	struct {
		const char *service;
		int nargs;
		int nret;
	} args;

	args.service = "enter";
	args.nargs = 0;
	args.nret = 0;
	(*prom_entry)( &args );
}

int prom_interpret( const char *forth )
{
	struct {
		const char *service;
		int nargs;
		int nret;
		const char *forth;
		int result;
	} args;

	args.service = "interpret";
	args.nargs = 1;
	args.nret = 1;
	args.forth = forth;
	args.result = -1;
	(*prom_entry)( &args );
	return args.result;
}

prom_handle_t prom_open( char *device )
{
    struct {
	const char *service;
	int nargs;
	int nret;
	char *name;
	prom_handle_t handle;
    } args;

    args.service = "open";
    args.nargs = 1;
    args.nret = 1;
    args.name = device;
    args.handle = INVALID_PROM_HANDLE;
    (*prom_entry)( &args );
    return args.handle;
}

word_t prom_instantiate_rtas( word_t rtas_base_address )
{
    prom_handle_t prom_rtas;

    prom_rtas = prom_open( "/rtas" );
    if( prom_rtas == INVALID_PROM_HANDLE )
	return 0;

    struct {
	const char *service;
	int nargs;
	int nret;
	char *method;
	prom_handle_t handle;
	word_t rtas_base_address;
	int result;
	word_t rtas_call;
    } args;

    args.service = "call-method";
    args.nargs = 3;
    args.nret = 2;
    args.method = "instantiate-rtas";
    args.handle = prom_rtas;
    args.rtas_base_address = rtas_base_address;
    args.result = -1;
    args.rtas_call = 0;
    (*prom_entry)( &args );
    return args.rtas_call;
}

void prom_get_rom_range( word_t ranges[], unsigned len, int *cnt )
{
	prom_handle_t rom;

	rom = prom_find_device( "/rom" );
	if( rom == INVALID_PROM_HANDLE )
		*cnt = 0;
	else {
		*cnt = prom_get_prop( rom, "ranges", ranges, len );
		*cnt /= sizeof(word_t);
	}
}

void prom_quiesce( void )
{
	struct {
		const char *service;
		int nargs;
		int nret;
	} args;

	args.service = "quiesce";
	args.nargs = args.nret = 0;
	(*prom_entry)( &args );
}

prom_callback_t prom_set_callback( prom_callback_t new_cb )
{
	struct {
		const char *service;
		int nargs;
		int nret;
		prom_callback_t new_cb;
		prom_callback_t old_cb;
	} args;

	args.service = "set-callback";
	args.nargs = 1;
	args.nret = 1;
	args.new_cb = new_cb;
	args.old_cb = 0;
	(*prom_entry)( &args );
	return args.old_cb;
}

/**************************************************************************/

void prom_init( word_t r5 )
{
	char path[256];
	int len;

	prom_entry = (prom_entry_t)r5;

	prom_chosen = prom_find_device( "/chosen" );
	if( prom_chosen == INVALID_PROM_HANDLE )
		prom_exit();

	if( prom_get_prop(prom_chosen, "stdout", &prom_stdout, 
				sizeof(prom_stdout)) != sizeof(prom_stdout) )
		prom_exit();
	if( prom_get_prop(prom_chosen, "stdin", &prom_stdin, 
				sizeof(prom_stdin)) != sizeof(prom_stdin) )
		prom_fatal( "unable to open /chosen/stdin" );
	if( prom_get_prop(prom_chosen, "memory", &prom_memory, 
				sizeof(prom_memory)) != sizeof(prom_memory) )
		prom_fatal( "unable to open /chosen/memory" );

	prom_options = prom_find_device( "/options" );
	if( prom_options == INVALID_PROM_HANDLE )
		prom_fatal( "unable to find /options" );

	len = prom_instance_to_path( prom_memory, path, sizeof(path) );
	if( len != -1 )
		prom_memory = prom_find_device( path );
	if( (len == -1) || (prom_memory == INVALID_PROM_HANDLE) )
		prom_fatal( "unable to open the memory node" );
}

void get_prom_range( word_t *start, word_t *size )
{
	prom_handle_t rom;
	word_t ranges[2];

	rom = prom_find_device( "/rom" );
	if( rom == INVALID_PROM_HANDLE )
		prom_fatal( "unable to open /rom" );

	if( prom_get_prop(rom, "ranges", ranges, sizeof(ranges)) !=
			sizeof(ranges) )
		prom_fatal( "unable to read /rom/ranges" );

	*start = ranges[0];
	*size = ranges[1];
}

