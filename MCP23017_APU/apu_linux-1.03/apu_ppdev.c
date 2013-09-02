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
#include "apu_ppdev.h"
#ifdef HAVE_LINUX_PARPORT_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include <linux/ppdev.h>
#include <linux/parport.h>



#include "parport.h"

static int ppdev_fd=-1;

#define SETUP_TIME 1
#define SETUP_LOOPS	1

//#define TRACE_RW

/******************** D E F I N I T I O N S ****************************/
#define DEFAULT_DEVICE	"/dev/parport0"
#define _RD_PIN		PARPORT_CONTROL_SELECT
#define WR_PIN		PARPORT_CONTROL_INIT
#define _ADDR1		PARPORT_CONTROL_AUTOFD
#define _ADDR0		PARPORT_CONTROL_STROBE
//#define _RD_PIN		0x08
//#define WR_PIN		0x04
//#define _ADDR1		0x02
//#define _ADDR0		0x01

#define INPUT_PIN	0x10
#define RDY_PIN		0x20

static int apu_ppdev_init(char *cmdline)
{
	int res;
	unsigned int modes=0;
	char *dev;

	if (cmdline && strlen(cmdline)) { 
		dev = cmdline; 
	} else { 
		dev = DEFAULT_DEVICE; 
	}
	
	if (ppdev_fd >= 0) {
		close(ppdev_fd);
	}
	
	ppdev_fd = open(dev, O_RDWR);
	if (ppdev_fd < 0) {
		perror("open");
		return -1;
	}
	
	res = ioctl(ppdev_fd, PPEXCL);
	if (res<0) {
		perror("Cound not claim exclusive access\n");
		close(ppdev_fd);
		return -1;
	}

	res = ioctl(ppdev_fd, PPCLAIM);
	if (res<0) {
		perror("Could not claim port\n");
		close (ppdev_fd);
		return -1;
	}

	modes = IEEE1284_MODE_COMPAT;
	if (ioctl (ppdev_fd, PPNEGOT, &modes)) {
		perror ("PPNEGOT");
		close (ppdev_fd);
		return 1;
	}
	
	res = ioctl(ppdev_fd, PPGETMODES, &modes);
	if (res<0) {
		perror("Could not get availables modes\n");
		close (ppdev_fd);
		return -1;
	}

//	printf("Modes: %08x :", modes);
	if (modes & PARPORT_MODE_PCSPP) { printf("SPP "); }
	if (modes & PARPORT_MODE_TRISTATE) { printf("TRISTATE "); }
	if (modes & PARPORT_MODE_EPP) { printf("EPP "); }
	if (modes & PARPORT_MODE_ECP) { printf("ECP "); }
	if (modes & PARPORT_MODE_COMPAT) { printf("COMPAT "); }
	if (modes & PARPORT_MODE_DMA) { printf("DMA "); }
//	printf("\n");	

	if (!(modes & PARPORT_MODE_PCSPP)) {
		fprintf(stderr, "Port does not support standard parallel port mode\n");
		return -1;
	}
	if (!(modes & PARPORT_MODE_TRISTATE)) {
		fprintf(stderr, "Port does not support bidirectional mode\n");
		return -1;
	}	
//	printf("OK\n");
	
	return 0;
}

static void apu_ppdev_shutdown(void)
{
	if (ppdev_fd >=0 )
	{
		close(ppdev_fd);
		ppdev_fd = -1;
	}
}

static void apu_ppdev_reset(void)
{
	unsigned char ctrl;

	ioctl(ppdev_fd, PPRCONTROL, &ctrl);
	
	ctrl &= ~WR_PIN;
	ctrl |= _RD_PIN;

//	printf("read ctrl: %02X\n", ctrl);
//	printf("write ctrl: %02X\n", ctrl);
	
	ioctl(ppdev_fd, PPWCONTROL, &ctrl);
	
	usleep(50000);							// /RESET will be tied to the
	
	ctrl |= WR_PIN;
	ctrl &= ~_RD_PIN;
	
	ioctl(ppdev_fd, PPWCONTROL, &ctrl);
	
//	printf("write ctrl: %02X\n", ctrl);
	usleep(50000);
}

static void apu_ppdev_write(int address, unsigned char data)
{
	unsigned char ctrl;
	int i;

	ioctl(ppdev_fd, PPRCONTROL, &ctrl);

	i = 0; // output
	ioctl(ppdev_fd, PPDATADIR, &i);
	
	// address
	ctrl |= 3;
	if (address & 1) {
		ctrl &= ~1;
	}
	if (address & 2) {
		ctrl &= ~2;
	}

	ioctl(ppdev_fd, PPWCONTROL, &ctrl); // address

	ctrl &= ~(WR_PIN|_RD_PIN); // write
	
	ioctl(ppdev_fd, PPWDATA, &data); // put data
	ioctl(ppdev_fd, PPWCONTROL, &ctrl); // validate data

	ctrl |= WR_PIN;
	ioctl(ppdev_fd, PPWCONTROL, &ctrl);
}

static unsigned char apu_ppdev_read(int address)
{
	unsigned char ctrl;
	unsigned char data=0xff;
	int i, res;

	res = ioctl(ppdev_fd, PPRCONTROL, &ctrl);
	if (res<0) { perror("PPRCONTROL"); }

	i = 0xff; // input
	ioctl(ppdev_fd, PPDATADIR, &i);
	if (res<0) { perror("PPDATADIR"); }

	// address
	ctrl |= 3;
	if (address & 1) {
		ctrl &= ~1;
	}
	if (address & 2) {
		ctrl &= ~2;
	}

	res= ioctl(ppdev_fd, PPWCONTROL, &ctrl); // address
	if (res<0) { perror("PPWCONTROL"); }

	ctrl |= (WR_PIN | _RD_PIN); // read
	res= ioctl(ppdev_fd, PPWCONTROL, &ctrl); // request data
	if (res<0) { perror("PPWCONTROL"); }
	
	res = ioctl(ppdev_fd, PPRDATA, &data); // read data
	if (res<0) { perror("PPRDATA"); }

	ctrl &= ~_RD_PIN;
	res = ioctl(ppdev_fd, PPWCONTROL, &ctrl);
	if (res<0) { perror("PPWCONTROL"); }
	
	return data;
}

static APU_ops ops = { 
	apu_ppdev_read, 
	apu_ppdev_write, 
	apu_ppdev_reset,
	apu_ppdev_init,
	apu_ppdev_shutdown
};

APU_ops *apu_ppdev_getOps(void)
{
	return &ops;
}
#endif
