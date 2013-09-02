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
#include <stdio.h>
#include <string.h>
#include "apu.h"
#include <time.h>
#include <sys/time.h>
#include "MCP23X17_outb-inb.h"

//#define TRACE_RW

extern int g_verbose; // from main.c

/******************** G L O B A L S ************************************/
static int port0=0;
static APU_ops *apu_ops = NULL;

void apu_setOps(APU_ops *ops)
{
	apu_ops = ops;
}

static int SetPort0 (short data)
{
	port0=data;
	return 0;
}

void apu_write (int address, unsigned char data)
{
//#ifdef TRACE_RW
//	printf("apu_write: a=%d, %02x\n", address, data);
//#endif
	apu_ops->write(address, data);	
}

unsigned char apu_read (int address)
{
	unsigned char tmp = apu_ops->read(address);
//#ifdef TRACE_RW
//	printf("apu_read: a=%d -> %02x\n", address, tmp);
//#endif

	return tmp;
}

/* Write to address 'address', write the previously
 * read value from port0 back to port 0 and wait
 * for port0 value to be different from the written one.
 */
int apu_writeHandshake(int address, int data)
{
//	int i;
//	i = 0;
	apu_write(address, data);
	apu_write(0,port0);
	
	if (!apu_waitInport(0, port0, 500)) {
		return 1;
	}
	port0++;
	if (port0 == 256)
		port0 = 0;

	return 0;
	

}


int apu_writeBytes(unsigned char *data, int len)
{
	int i;
#ifdef TRACE_RW
	printf("apu_writeBytes: %d...\n", len);
#endif
	for (i=0; i<len; i++) {
		if (apu_writeHandshake(1, data[i])) { return 1; }
	}
	return 0;
}

void apu_reset(void)	//Reset the APU by whatever means it needs reset.
{
	apu_ops->reset();	

}

/* return false on timeout, otherwise true */
int apu_waitInport(int port, unsigned char data, int timeout_ms)
{
	struct timeval tv_before, tv_now;
	int elaps_milli;
	gettimeofday(&tv_before, NULL);

#ifdef TRACE_RW
	printf("apu_waitInport: addr: %d, data: %02x\n",port,data);
#endif
	
	while(apu_read(port)!=data) {
		//usleep(1);
		gettimeofday(&tv_now, NULL);
		elaps_milli = (tv_now.tv_sec - tv_before.tv_sec) * 1000;
		elaps_milli += (tv_now.tv_usec-tv_before.tv_usec)/1000;
		if (elaps_milli > timeout_ms )
		{
			if (g_verbose) 
				printf("timeout after %d milli\n", elaps_milli);
			return 0;
		}
	}
	return 1;
}

int apu_initTransfer(unsigned short address)
{
	/* Initializing the transfer */
	/* Wait for port 2140 to be $aa */
	if (!apu_waitInport(0, 0xaa, 500)) {
		if (g_verbose) 
			printf("Should read 0xaa, but reads %02x\n", apu_read(0));
		return -1;
	}
	/* and Wait for port 2141 to be $bb */
	if (!apu_waitInport(1, 0xbb, 500)) {
		if (g_verbose) 
			printf("Should read 0xaa, but reads %02x\n", apu_read(0));
		return -1;
	}
	/* The spc is now ready */

	/* Write any value other than 0 to 2141 */
	apu_write(1, 1);
	/* Write the destination address to poirt $2142 and $2143, with the
	 * low byte written at $2142 */
	apu_write(2, address&0xff); // low
	apu_write(3, (address&0xff00)>>8); // high So our code will go at $0002
	/* Write $CC to port $2140 */
	apu_write(0, 0xCC);

	/* Wait for $2140 to be $CC */
	if (!apu_waitInport(0, 0xcc, 500)) {
		if (g_verbose) 
			printf("Should read 0xcc, but reads %02x\n", apu_read(0));
		return -1;
	}

	SetPort0(0);
	
	return 0;
}

int apu_newTransfer(unsigned short address)
{
	int i;
	apu_write(1,1);
	apu_write(3, (address&0xff00)>>8);
	apu_write(2, (address & 0xff));

	i = apu_read(0);
	i += 2; i &= 0xff;
	if (!i) { i += 2; } // if it's 0, increase it again
	apu_write(0, i);
	if (!apu_waitInport(0, i, 500)) {
		fprintf(stderr, "apu_newTransfer: timeout\n"); return -1;
	}
	
	SetPort0(0);
	
	return 0;
}

void apu_endTransfer(unsigned short start_address)
{
	int i;
	
	apu_write(1, 0);
	apu_write(3, (start_address & 0xff00)>>8);
	apu_write(2, (start_address & 0xff));

	i = apu_read(0);
	i +=2; i &= 0xff;
	if (!i) { i+=2 ; } // if it's 0, increase it again
	apu_write(0, i);
}


