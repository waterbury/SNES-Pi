#include <stdio.h>
//#include <conio.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#include "parport.h"
#include "apu.h"
#include "MCP23017_outb-inb.h"

#include <sys/io.h>
#include <unistd.h>

//#define _outp(a, b) outb(b, a)
#define _outp(a, b) outb_MCP23017((uint8_t)b, a)

//#define _inp(a) inb(a)
#define _inp(a) inb_MCP23017(a)



int ByteTransferCount=0;
int ParallelPortCount=0;

#define SETUP_TIME 1
#define SETUP_LOOPS	1

//#define TRACE_RW

//unsigned char port0;

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

/******************** S T R U C T U R E S ******************************/


/******************** G L O B A L S ************************************/


unsigned char port0=0;

int WriteSPC700 (int address, unsigned char data)
{
	int dummy=0, i;
#ifdef TRACE_RW
	printf("WriteSPC700: a=%d, %02x\n", address, data);
#endif
	
	/* put the paralle port in output mode,
	 * prepare address bits */
	ClrPins(control_pins, INPUT_MODE);
		_outp(CONTROL, control_pins);
//	usleep(SETUP_TIME);

	SetPins(control_pins, (_ADDR0 + _ADDR1));
	ClrPins(control_pins, address);
	for (i=0; i<SETUP_LOOPS; i++) {
		_outp(CONTROL, control_pins);
	}

//	usleep(SETUP_TIME);
	/* Prepare write bit */
	ClrPins(control_pins, WR_PIN);
	ClrPins(control_pins, _RD_PIN);

	_outp(DATA, data);

	for (i=0; i<SETUP_LOOPS; i++) {
		_outp(CONTROL, control_pins);
	}


//	usleep(SETUP_TIME);

//	usleep(SETUP_TIME);
	
	SetPins(control_pins, WR_PIN);
	for (i=0; i<SETUP_LOOPS; i++) {
		_outp(CONTROL, control_pins);
	}

//	usleep(SETUP_TIME);

	return dummy;
}

unsigned char ReadSPC700 (int address)
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
	_outp(CONTROL, control_pins);

//	usleep(SETUP_TIME);

	for (i=0; i<SETUP_LOOPS; i++) {
		data=_inp(DATA);
	}
	
	
	ClrPins(control_pins, _RD_PIN);
	
	for (i=0; i<SETUP_LOOPS; i++) {
		_outp(CONTROL, control_pins);
	}
#ifdef TRACE_RW
	printf("ReadSPC700: a=%d -> %02x\n", address, data);
#endif
	
	return data;

}

void  WriteSPC700_16 (int address, unsigned char data0, unsigned char data1)
{
	WriteSPC700(address, data1);
	WriteSPC700(address, data0);
}

int WriteSPC700_WP0I (int address, unsigned char data)
{
	int i;
	i = 0;
	WriteSPC700(address, data);
        WriteSPC700(0,port0);
	while (ReadSPC700(0)!=port0)
	{
		i++;
		if (i > 1024)
                    {
                        //printf("B:%u", port0);
			return 1;
                    }
	}
	port0++;
	if (port0 == 256)
		port0 = 0;

	return 0;
	

}

unsigned char SetPort0 (unsigned char data)
{
	
//	xfer_byte_ready ( 0x10 );
//	return xfer_byte_ready ( data );
	
	port0=data;
	return 0;
}

int WriteSPC700Bytes(unsigned char *data, int len)
{
       // printf("| ");
	int i;
        for (i=0; i<len; i++) {
//         printf(" %03x ", data[i]);
	 if (WriteSPC700_WP0I(1, data[i])) 
         { 
          printf(" OH OH OH OH");
          return 1; 
         }
        }
	return 0;
}

int ResetAPU(void)	//Reset the APU by whatever means it needs reset.
{


	
	ClrPins(control_pins,WR_PIN);		//Once the OR gate is in place
	SetPins(control_pins,_RD_PIN);		//Pulling both /RD and /WR low

		_outp(CONTROL, control_pins);	//Will Reset the APU, since


	usleep(50000);							// /RESET will be tied to the
	SetPins(control_pins,WR_PIN);		//output of the OR gate.
	ClrPins(control_pins,_RD_PIN);
		_outp(CONTROL, control_pins);
	usleep(50000);
	return 0;
}

