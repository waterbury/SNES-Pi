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

/*******************************************************************************
These routines are part of an old program I wrote to control a robot and camera 
from the parallel port.  As you can see everything is old DOS stuff (I miss the 
days where code writers were given FULL control of the computer ... sigh).  
Anyway, Win9x seems to still allow direct control of the ports I need.
However, WinNT based systems do not.  For other operating systems ... 
sorry, I have no idea.

You CAN get around the NT problem by using allowio.exe which allows control of 
ports by the user.  You can get allowio here:
http://www.beyondlogic.org/porttalk/porttalk.htm

The following ports are used -

(BASE = 0x378 for LPT1, 0x278 for LPT2)
for normal use:           ports Base - Base+2
to enable bidir via EPP:  ports 0x3F0 - 0x3F1
to enable bidir via ECP:  port BASE+0x402


-kevin mantey
(Neviksti)

P.S.  Some of the functions had to be re-typed (arghh... don't ask).  So if I
retyped them in wrong from my hard copy, I'm sorry.  The mistakes should only 
be small ones that are easily fixed.

*******************************************************************************/

#include <stdio.h>
//#include <conio.h>

#include <unistd.h>
//#include <sys/io.h>
#include "parport.h"

#ifdef HAVE_SYS_IO_H

#include "io.h"

#include <time.h>
#include "MCP23X17_outb-inb.h"


int BASE = 0x378;	
//default BASE = 0x378 ---> LPT1
//        BASE = 0x278 ---> LPT2

int control_pins = 0;	//default - start port in output mode, no interrupt
int data_pins = 0;	//default - start with pins low

/*********************

CONTROL port:

bit 0 = (inverted to pin 1)
bit 1 = (inverted to pin 14)
bit 2 = (pin 16)
bit 3 = (inverted to pin 17)

bit 4 (interrupt enable) = 1 to enable interrupt on (0 = off) ...
    An IRQ will be triggered on each rising edge of the -ACK signal on pin 10 
    Important note: some older parallel ports trigger the interrupt on the 
    falling edge of -ACK. Even worse, not all cards support the parallel port
    interrupt.  Don't worry, a large majority do allow this interrupt.

bit 5 (bi-dir control) = 0 data is output from computer
			 1 data is input to computer

bit 6,7 (reserved) = 0

*********************

STATUS port:

bit 0,1,2 (undefined)

bit 3 = (pin 15)				
bit 4 = (pin 13)
bit 5 = (pin 12)
bit 6 = (pin 10)				
bit 7 = (inverted from pin 11)

***********************/


/*================= FUNCTIONS =======================*/

int parport_init(void)
{
return 0;
//	return get_io_permissions();
}

void parport_shutdown(void)
{
	//release_io_perms();
}

int BidirAvailable(void)
{
	int bidir=0;

	//try to change to input mode
	io_outp(CONTROL, INPUT_MODE);

	//see if it worked
	io_outp(DATA,0x55);
	if(io_inp(DATA) != 0x55)
		bidir=1;
	
	//make sure we didn't accidentally choose the state
	//the port was in
	io_outp(DATA,0xAA);
	if(io_inp(DATA) != 0xAA)
		bidir=1;

	//return port to output mode and data = $00
	io_outp(CONTROL, OUTPUT_MODE);
	control_pins=OUTPUT_MODE;
	io_outp(DATA, 0);
	data_pins=0;

	return bidir;
} //end of BidirAvailable()

int EnableBidir(void)
{
    //Some computers/ParallelPorts need to have something enabled
    //before they can switch to and from input/output mode.
    //This will try a bunch of ways to enable bidir.

    if(BidirAvailable())
        return 1;

    //try using ECP first
    //
    // 0x34 = 
    //   bit 7:5 = 001, PS2/ParallelPort mode
    //   bit 4 = 1, Disables the interrupt generated on 
    //              the asserting edge of nFault.
    //   bit 3 = 0, Disables DMA unconditionally.
    //   bit 2 = 1, Disables DMA and all of the service interrupts.
    //   bit 1,0 are ReadOnly

    io_outp(ECR, 0x34);
    printf("\nTrying to get computer to allow bidir via ECP...");
    if(BidirAvailable())
        return 1;
    printf("failed!");

    //try some chipset specific enable code

    printf("\nTrying to get computer to allow bidir via EPP... chipset 666");
    begin_EPP(DATA, 666); 
    if(BidirAvailable())
        return 1;
    printf("failed!");

    printf("\nTrying to get computer to allow bidir via EPP... chipset 665");
    begin_EPP(DATA, 665);
    if(BidirAvailable())
        return 1;
    printf("failed!");

    return 0;
} //end of EnableBidir()


/*******************************************************************************  
These next three functions are based on page 117 of the tech spec   
 and also in "Parallel Port Complete", Page 214               

 I actually got them from 
 "Programming the Enhanced Parallel Port"
 http://madang.ajou.ac.kr/~ydpark/archive/computer/ppi/epp.html

 I know they look weird, but I've needed these functions to enable bidir on some
 computers.  They're especially helpful when you can't get to the BIOS setup to
 enable the bidir on a parallel port. (Or the people who are using your software
 don't know how to do such a thing).

   -kevin mantey
   (Neviksti)
*******************************************************************************/

void begin_config_mode(int chip)
{
    unsigned char init_code;

    switch(chip)
    {
        case 666:  init_code = 0x44; break;
        case 665:  init_code = 0x55; break;
        default:   printf("\nChip %d not supported!!!\n", chip); 
				   return;
    }

//    __asm cli;
    io_outp(0x3f0, init_code);
    io_outp(0x3f0, init_code);
//    __asm sti;
} //end of begin_config_mode

void end_config_mode(void)
{
    // Note that there is a typo in Parallel Port Complete, page 214
    // it says write to 0x3f1 instead of 0x3f0
    io_outp( 0x3f0, 0xaa );
}

void begin_EPP(int port_addr, int chip)
{
    // port_addr = 0x278 or 0x378, chip = '666' or '665'
    begin_config_mode( chip );

    // control word for configuring
    // CR   Bits 1,  Port address   00 -> disabled
    //                              01 -> 0x3bc
    //                              10 -> 0x378
    //                              11 -> 0x278 (default)
    //       Bit 2   Port power     1  -> power supplied (default)
    //                              0  -> low power mode
    //       Bit 3   Mode           1  -> SPP (default)
    //                              0  -> Extended modes allowed (CR4-bits 0,1)
    //       Bit 4   IRQ polarity   1  -> active high, inactive low (default)
    //                              0  -> active low, inactive high-Z 
    //                                      (always true in ECP,EPP)
        
    // Set CR1
    io_outp( 0x3f0, 1 ); 
    if (port_addr == 0x378)
        io_outp( 0x3f1, 0x96 ); // use 0x378
    else
        io_outp( 0x3f1, 0x97 );  // use 0x278

    // CR4  Bits 0,1  Ext modes     10 -> SPP, PS/2 (default)
    //                              01 -> SPP and EPP
    //                              10 -> ECP
    //                              11 -> ECP and EPP
    //         Bit 6  EPP type      0  -> EPP 1.9 (default)
    //                              1  -> EPP 1.7
        
    io_outp( 0x3f0, 4 );   // use CR4
    io_outp( 0x3f1, 3 );   // use EPP

    // CRA  Bits 0-3  ECP FIFO thres -> threshold for ECP service requests
    //                                  default 0

    /*      Use if you need
      io_outp( 0x3f0, 0xa );  // use CRA
      io_outp( 0x3f1, 8 );    // threshold for ECP requests
    */

    // 0x34 == <0011 0100>
    // PS/2 (byte, bidirectional) type (bits 7-5) == 001,
    //      no interrupts at nError (bit 4) == 1,
    //      disable DMA (bit 3) == 0,
    // disable DMA and service interrupts (bit 2) == 1
    // bits 1,0 read only, so don't care
    io_outp((unsigned short)(port_addr + ECR_OFFSET), 0x34 );

    /* pulse - nInit (bit 2) low */

    io_outp((unsigned short)(port_addr + CONTROL_PORT), 0x00 );
    end_config_mode();

    // ECP emulating EPP 0x80 == <1000 0000>
    //      For EPP mode, set ECR of ECP to mode (bits 7-5) == 100
    //      Falling edge of nError generates interrupt (bit 4) == 0,
    //      disable DMA (bit 3) == 0,
    // enable service interrupts (bit 2) == 0
    // bits 1,0 read only, so don't care

	//Set ECR to EPP mode
    io_outp((unsigned short)(port_addr + ECR_OFFSET), 0x80 );

    // pulse - nInit (bit 2) high; reset the peripheral, 
    // min pulse width 50 micro-sec
    io_outp((unsigned short)(port_addr + CONTROL_PORT), 0x04 );
} //end of begin_EPP

/*****************************************************************************/

int SetupBidir(void)
{
	if(!BidirAvailable())
	{
		printf("\nBidirectional Parallel Port not detected");
		printf("\ntrying to enable...");

		if(!EnableBidir())
			return 0;
		
		printf("success!\n");
	}
		printf("\nBidirectional Parallel Port detected");

	return -1;
} //end of SetupBidir()

void TestPort(void)
{
    int        i;
    double     avrg;
    clock_t    time;

    //test "write to DATA port" speed here
    time = clock();
    for(i=0;i<75000;i++)
    {
        // should take roughly 4 usec * 75000 = 300 msec
        io_outp(DATA, 0xFF);
        io_outp(DATA, 0x00);
        io_outp(DATA, 0x55);
        io_outp(DATA, 0xAA);
    }
    time = clock() - time;
    avrg = 1.0*time/CLOCKS_PER_SEC;
    avrg = avrg/(75000*4.0);
    avrg *= 1000000;	//keep average in usec

    printf("\nAverage time per write to DATA port \t=  %f usec", avrg);
    if(avrg < 0.75)
        printf("\nWARNING: port speed is not regulated correctly (too fast)");

    //test "write to CONTROL port" speed here
    time = clock();
    for(i=0;i<75000;i++)
    {
        // should take roughly 4 usec * 75000 = 300 msec
        io_outp(CONTROL, control_pins ^ 0x00);
        io_outp(CONTROL, control_pins ^ 0x01);
        io_outp(CONTROL, control_pins ^ 0x02);
        io_outp(CONTROL, control_pins ^ 0x03); 
    }
    time = clock() - time;
    avrg = 1.0*time/CLOCKS_PER_SEC;
    avrg = avrg/(75000*4.0);
    avrg *= 1000000;	//keep average in usec

    printf("\nAverage time per write to CONTROL port \t=  %f usec", avrg);
    if(avrg < 0.75)
        printf("\nWARNING: port speed is not regulated correctly (too fast)");

    //test "read from DATA port" speed here
    time = clock();
    for(i=0;i<75000;i++)
    {
	// should take roughly 4 usec * 75000 = 300 msec
	io_inp(DATA);
	io_inp(DATA);
	io_inp(DATA);
	io_inp(DATA);
    }
    time = clock() - time;
    avrg = 1.0*time/CLOCKS_PER_SEC;
    avrg = avrg/(75000*4.0);
    avrg *= 1000000;	//keep average in usec

    printf("\nAverage time per read from DATA port \t=  %f usec", avrg);

    //test "read from STATUS port" speed here
    time = clock();
    for(i=0;i<75000;i++)
    {
        // should take roughly 4 usec * 75000 = 300 msec
        io_inp(STATUS);
        io_inp(STATUS);
        io_inp(STATUS);
        io_inp(STATUS);
    }
    time = clock() - time;
    avrg = 1.0*time/CLOCKS_PER_SEC;
    avrg = avrg/(75000*4.0);
    avrg *= 1000000;	//keep average in usec

    printf("\nAverage time per read from STATUS port \t=  %f usec\n", avrg);

} //end of TestPort()

#endif 
