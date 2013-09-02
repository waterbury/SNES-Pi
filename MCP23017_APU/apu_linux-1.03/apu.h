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
#ifndef _apu_h__
#define _apu_h__


typedef struct {
	unsigned char (*read)(int address);
	void (*write)(int address, unsigned char data);
	void (*reset)(void);
	/* should return -1 if error and print an error message,
	 * return 0 on success. */
	int (*init)(char *cmdline);
	void (*shutdown)(void);
} APU_ops;


void apu_setOps(APU_ops *ops);

unsigned char apu_read(int address);

void apu_write(int address, unsigned char data);

/* Write to address 'address', write the previously
 * read value from port0 back to port 0 and wait
 * for port0 value to be different from the written one.
 */
int apu_writeHandshake(int address, int data);

/* Write many bytes using handshake (see apu_writeHandshake) */
int apu_writeBytes(unsigned char *data, int len);

/* reset the apu */
void apu_reset(void);

/* wait for a port to contain given value, with timeout */
int apu_waitInport(int port, unsigned char data, int timeout_ms);

/*
 * Initialise an spc transfer at 'address'. 
 * To be used after reset of after jumping to rom
 *
 * Use apu_newTransfer for additional transfers
 * 
 * return -1 on error, 0 if success
 */
int apu_initTransfer(unsigned short address);

/* initialise a new transfer after the first transfer.
 *
 * returns 0 on success, -1 on error */
int apu_newTransfer(unsigned short address);

/* End transfer and jump to address
 */
void apu_endTransfer(unsigned short start_address);

#endif // _apu_h__

