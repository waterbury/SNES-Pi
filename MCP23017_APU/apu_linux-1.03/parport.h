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
#ifndef PARPORT_H
#define PARPORT_H	1

#include "config.h"

#ifdef HAVE_SYS_IO_H

extern int BASE;	//holds base I/O address of parallel port

#define DATA			(unsigned short)(BASE + 0x000)
#define STATUS			(unsigned short)(BASE + 0x001)
#define CONTROL			(unsigned short)(BASE + 0x002)
#define ECR				(unsigned short)(BASE + 0x402)

#define ECR_OFFSET		0x402
#define CONTROL_PORT	0x002


#define OUTPUT_MODE	0x00
#define INPUT_MODE	0x20

extern int control_pins;	//should be used to hold value last written to control pins
extern int data_pins;		//should be used to hold value last written to data pins

#define SetPins(x,p)		x = (x | p)
#define ClrPins(x,p)		x = (x & (~p))
#define TogglePins(x,p)		x = (x ^ p)

int parport_init(void);
void parport_shutdown(void);
int BidirAvailable(void);
int EnableBidir(void);
void begin_config_mode(int chip);
void end_config_mode(void);
void begin_EPP(int port_addr, int chip);
int SetupBidir(void);
void TestPort(void);

#endif

#endif
