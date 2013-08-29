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
#include <stdint.h>
//#include <conio.h>

#include <unistd.h>
#include <sys/io.h>
//#define _outp(a, b) outb(b, a)
#define _outp(a, b) outb_MCP23017((uint8_t)b, a)

//#define _inp(a) inb(a)
#define _inp(a) inb_MCP23017(a)

#include <time.h>
#include "parport.h"
#include "MCP23017_outb-inb.h"

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
{/*
	int res;

#if 0
	res = ioperm(BASE, 3, 1); // includes base, status and control
	if (res<0) {
		return -1;
	}

	printf("Io area %X\n", BASE+ECR_OFFSET);
	res = ioperm(BASE+ECR_OFFSET, 1, 1);
	if (res<0) {
		return -1;
	}
#endif
	res = iopl(3);
	if (res<0) {
		perror("iopl");
		return -1;
	}
	*/
	return 0;
}

int BidirAvailable(void)
{
        //return 1;
	int bidir=0;

	//try to change to input mode
	_outp(CONTROL, INPUT_MODE);

	//see if it worked
	_outp(DATA,0x55);
	if(_inp(DATA) != 0x55)
		bidir=1;
	
	//make sure we didn't accidentally choose the state
	//the port was in
	_outp(DATA,0xAA);
	if(_inp(DATA) != 0xAA)
		bidir=1;

	//return port to output mode and data = $00
	_outp(CONTROL, OUTPUT_MODE);
	control_pins=OUTPUT_MODE;
	_outp(DATA, 0);
	data_pins=0;

	return bidir;

//        return 1;

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

    _outp(ECR, 0x34);
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
    _outp(0x3f0, init_code);
    _outp(0x3f0, init_code);
//    __asm sti;
} //end of begin_config_mode

void end_config_mode(void)
{
    // Note that there is a typo in Parallel Port Complete, page 214
    // it says write to 0x3f1 instead of 0x3f0
    _outp( 0x3f0, 0xaa );
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
    _outp( 0x3f0, 1 ); 
    if (port_addr == 0x378)
        _outp( 0x3f1, 0x96 ); // use 0x378
    else
        _outp( 0x3f1, 0x97 );  // use 0x278

    // CR4  Bits 0,1  Ext modes     10 -> SPP, PS/2 (default)
    //                              01 -> SPP and EPP
    //                              10 -> ECP
    //                              11 -> ECP and EPP
    //         Bit 6  EPP type      0  -> EPP 1.9 (default)
    //                              1  -> EPP 1.7
        
    _outp( 0x3f0, 4 );   // use CR4
    _outp( 0x3f1, 3 );   // use EPP

    // CRA  Bits 0-3  ECP FIFO thres -> threshold for ECP service requests
    //                                  default 0

    /*      Use if you need
      _outp( 0x3f0, 0xa );  // use CRA
      _outp( 0x3f1, 8 );    // threshold for ECP requests
    */

    // 0x34 == <0011 0100>
    // PS/2 (byte, bidirectional) type (bits 7-5) == 001,
    //      no interrupts at nError (bit 4) == 1,
    //      disable DMA (bit 3) == 0,
    // disable DMA and service interrupts (bit 2) == 1
    // bits 1,0 read only, so don't care
    _outp((unsigned short)(port_addr + ECR_OFFSET), 0x34 );

    /* pulse - nInit (bit 2) low */

    _outp((unsigned short)(port_addr + CONTROL_PORT), 0x00 );
    end_config_mode();

    // ECP emulating EPP 0x80 == <1000 0000>
    //      For EPP mode, set ECR of ECP to mode (bits 7-5) == 100
    //      Falling edge of nError generates interrupt (bit 4) == 0,
    //      disable DMA (bit 3) == 0,
    // enable service interrupts (bit 2) == 0
    // bits 1,0 read only, so don't care

	//Set ECR to EPP mode
    _outp((unsigned short)(port_addr + ECR_OFFSET), 0x80 );

    // pulse - nInit (bit 2) high; reset the peripheral, 
    // min pulse width 50 micro-sec
    _outp((unsigned short)(port_addr + CONTROL_PORT), 0x04 );
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
        _outp(DATA, 0xFF);
        _outp(DATA, 0x00);
        _outp(DATA, 0x55);
        _outp(DATA, 0xAA);
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
        _outp(CONTROL, control_pins ^ 0x00);
        _outp(CONTROL, control_pins ^ 0x01);
        _outp(CONTROL, control_pins ^ 0x02);
        _outp(CONTROL, control_pins ^ 0x03); 
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
	_inp(DATA);
	_inp(DATA);
	_inp(DATA);
	_inp(DATA);
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
        _inp(STATUS);
        _inp(STATUS);
        _inp(STATUS);
        _inp(STATUS);
    }
    time = clock() - time;
    avrg = 1.0*time/CLOCKS_PER_SEC;
    avrg = avrg/(75000*4.0);
    avrg *= 1000000;	//keep average in usec

    printf("\nAverage time per read from STATUS port \t=  %f usec\n", avrg);

} //end of TestPort()

  
