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
#include "apu_ppio.h"
#ifdef PPIO_SUPPORTED
#include <time.h>
#include <sys/time.h>
#include "io.h"
#include <unistd.h>
#include <stdio.h>

#include "parport.h"

int ByteTransferCount=0;
int ParallelPortCount=0;

#define SETUP_TIME 1
#define SETUP_LOOPS	1

//#define TRACE_RW

/******************** D E F I N I T I O N S ****************************/
#define OUTPUT_PIN	0x01
#define SCK_PIN		0x02
#define CART_PIN	0x40
#define RESET_PIN	0x80

#define _RD_PIN		0x08
#define WR_PIN		0x04
#define _ADDR1		0x02
#define _ADDR0		0x01

#define INPUT_PIN	0x10
#define RDY_PIN		0x20

static int apu_ppio_init(char *cmdline)
{
	if (parport_init()<0) {
		printf("Failed to get io permissions. Are you root?\n");
		return -1;
	}
	
	if (!BidirAvailable())
	{
		if (!EnableBidir()) {
			printf("Failed to enter bidirectional mode. Try setting the parallel port mode to ECP, EPP or b\n");
			return -1;
		}
	}
	return 0;
}

static void apu_ppio_shutdown(void)
{
	parport_shutdown();
}

static void apu_ppio_reset(void)
{
	ClrPins(control_pins,WR_PIN);		//Once the OR gate is in place
	SetPins(control_pins,_RD_PIN);		//Pulling both /RD and /WR low
		io_outp(CONTROL, control_pins);	//Will Reset the APU, since
	usleep(50000);							// /RESET will be tied to the
//usleep(1000000);

	SetPins(control_pins,WR_PIN);		//output of the OR gate.
	ClrPins(control_pins,_RD_PIN);
		io_outp(CONTROL, control_pins);
	usleep(50000);
}

static void apu_ppio_write(int address, unsigned char data)
{
	int i;
	
	/* put the paralle port in output mode,
	 * prepare address bits */
	ClrPins(control_pins, INPUT_MODE);
		io_outp(CONTROL, control_pins);
//	usleep(SETUP_TIME);

	SetPins(control_pins, (_ADDR0 + _ADDR1));
	ClrPins(control_pins, address);
	for (i=0; i<SETUP_LOOPS; i++) {
		io_outp(CONTROL, control_pins);
	}

//	usleep(SETUP_TIME);
	/* Prepare write bit */
	ClrPins(control_pins, WR_PIN);
	ClrPins(control_pins, _RD_PIN);

	io_outp(DATA, data);

	for (i=0; i<SETUP_LOOPS; i++) {
		io_outp(CONTROL, control_pins);
	}


//	usleep(SETUP_TIME);

//	usleep(SETUP_TIME);
	
	SetPins(control_pins, WR_PIN);
	for (i=0; i<SETUP_LOOPS; i++) {
		io_outp(CONTROL, control_pins);
	}

//	usleep(SETUP_TIME);
}

static unsigned char apu_ppio_read(int address)
{
	int i;
	unsigned char data;
	if (!(control_pins & INPUT_MODE))
	{ 
		SetPins(control_pins, (_ADDR0 + _ADDR1 + INPUT_MODE));
	}
	else
	{
		SetPins(control_pins, (_ADDR0 + _ADDR1));
	} 
	SetPins(control_pins, WR_PIN);
	ClrPins(control_pins, address);
	SetPins(control_pins, _RD_PIN);
	io_outp(CONTROL, control_pins);

//	usleep(SETUP_TIME);

	for (i=0; i<SETUP_LOOPS; i++) {
		data=io_inp(DATA);
	}
	
	
	ClrPins(control_pins, _RD_PIN);
	
	for (i=0; i<SETUP_LOOPS; i++) {
		io_outp(CONTROL, control_pins);
	}
	
	return data;
}

static APU_ops ops = { 
	apu_ppio_read, 
	apu_ppio_write, 
	apu_ppio_reset,
	apu_ppio_init,
	apu_ppio_shutdown
};

APU_ops *apu_ppio_getOps(void)
{
	return &ops;
}

#endif // PPIO_SUPPORTED
