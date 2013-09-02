/* hwapu - SPC music playback tools for real snes apu
 * Copyright (C) 2004-2005  Raphael Assenat <raph@raphnet.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#ifndef _io_h__
#define _io_h__

#ifdef DJGPP

#include <pc.h>
#include "MCP23X17_outb-inb.h"

//#define io_outp(a, b) outportb((a), (b))
//#define io_inp(a) (inportb((a)))

#define get_io_permissions()  (0)
#define release_io_perms() 

#else

#include <stdio.h>
#include <errno.h>
#include <sys/io.h>
#include <stdint.h>

#define io_outp(a, b)   outb_MCP23X17((uint8_t)b,a)    //    outb(b, a)
#define io_inp(a) inb_MCP23X17(a) //(inb(a))

static inline void release_io_perms(void)
{
//	iopl(0);
}

static inline int get_io_permissions(void)
{
/*	int res;
	res = iopl(3);
	if (res<0) {
		perror("iopl");
		return -1;
	}
*/
	return 0;
}

#endif


#endif // _io_h__

