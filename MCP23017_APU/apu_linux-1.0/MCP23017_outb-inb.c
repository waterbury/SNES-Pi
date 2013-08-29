#include <stdint.h>
#include "parport.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "rpiGpio.h"

#include <wiringPiI2C.h>
#include <wiringPi.h>


#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"


#define CHIPADDR 0x20
#define IOCON 0x0A
#define IODIRA 0x00
#define IPOLA 0x02
#define GPINTENA 0x04
#define DEFVALA 0x06
#define INTCONA 0x08
#define GPPUA 0x0C
#define INTFA 0x0E
#define INTCAPA 0x10
#define GPIOA 0x12
#define GPIOB 0x13
#define OLATA 0x14
#define IODIRB 0x01
#define IPOLB 0x03
#define GPINTENB 0x05
#define DEFVALB 0x07
#define INTCONB 0x09
#define GPPUB 0x0D
#define INTFB 0x0F
#define INTCAPB 0x11
#define OLATB 0x15

#define DEBUG_MODE 0

const char *byte_to_binary(uint8_t value)
{
    char x = (char)value;
    static char b[9];
    b[0] = '\0';

    int z;
    for (z = 128; z > 0; z >>= 1)
    {
        strcat(b, ((x & z) == z) ? "1" : "0");
    }

    return b;
}


//static int hasBeenInit = 0;


int changePortDir(int direction)
{
static int currentDirection = -10;
int fd = init_MCP23017(CHIPADDR);

int value = 0;


if ( (direction == 1) | (direction == 0) )
 {
  if (direction != currentDirection)
   {
    if (direction == 1)
     value = 0xFF;
    else
     value = 0;

    wiringPiI2CWriteReg8 (fd, IODIRB, value);
    currentDirection = direction;

//    if (DEBUG_MODE)
//     printf("DIR: %i",currentDirection);
    }
  }
 return currentDirection;
}

uint8_t remapSNESpins(uint8_t data)
{
uint8_t newData = 0x00;
int resetCounter = 0;

#define PA0_PIN 0x80
#define PA1_PIN 0x40
#define WR_PIN 0x20
#define RD_PIN 0x10
#define RESET_PIN 0x01
#define RESET_LED 0x08

#define ORIG_PA0_PIN 0x01
#define ORIG_PA1_PIN 0x02
#define ORIG_WR_PIN 0x04
#define ORIG_RD_PIN 0x08
//#define ORIG_RESET_PIN 0x01
//#define ORIG_RESET_LED 0x08

if ( (data & ORIG_PA0_PIN) == ORIG_PA0_PIN)
 newData = newData | PA0_PIN;
if ( (data & ORIG_PA1_PIN) == ORIG_PA1_PIN)
 newData = newData | PA1_PIN;

if ( (data & ORIG_RD_PIN) == ORIG_RD_PIN)
 newData = newData | RD_PIN;
else
 resetCounter++;

if ( (data & ORIG_WR_PIN) == ORIG_WR_PIN)
 newData = newData | WR_PIN;
else
 resetCounter++;

if ( ((data & ORIG_WR_PIN) == ORIG_WR_PIN) && ((data & ORIG_RD_PIN) == ORIG_RD_PIN) )
{
//newData = ((newData ^ 0xFF) | RD_PIN) ^ 0xFF; //clear RD Pin
}


if (resetCounter < 2)//Holds reset high, unless WR and RD are low
 newData = newData | (RESET_PIN + RESET_LED);

return newData;

}


void    outb_MCP23017(uint8_t data,int port)
{
int fd = init_MCP23017(CHIPADDR);

if (port == CONTROL)
 {
  if ( (data & 0x20) == 0x20 )//change direction bit
{
   changePortDir(1);
}
  else
{
   changePortDir(0);
}

  data = data ^ 0x0B;//Inverts control Pins 0, 1, and 3
  data = remapSNESpins(data);//Maps original pins of sch to GPIO pins

  wiringPiI2CWriteReg8(fd, GPIOA, (int)data ) ;

 }
else if(port == DATA)
{
//changePortDir(0);
if (changePortDir(-1) == 0)//If in output mode
{
 wiringPiI2CWriteReg8 (fd, GPIOB, (int)data ) ;
}
else 
{
 printf("Nothing sent");
}

}


//printf("%i ", hasBeenInit );
if (DEBUG_MODE)
{
printf("%s > OUT: ",KWHT);

if (port == DATA)
 printf("%s   DATA",KGRN );
else if (port == STATUS)
 printf("%s STATUS",KCYN );
else if (port == CONTROL)
 printf("%sCONTROL",KYEL );
else if (port == ECR)
 printf("%s   ECR",KMAG);
else
 printf("UNKNOWN PORT %u", port);

printf("| ");

printf("%03u-%s\n%s",data,byte_to_binary(data),KWHT);
}


}




uint8_t inb_MCP23017(int port)
{
int fd = init_MCP23017(CHIPADDR);
//printf("%i ", hasBeenInit );

uint8_t data = 0;
int value = 0;

changePortDir(1);
if (changePortDir(-1) == 1)//input mode
{
value = wiringPiI2CReadReg8 (fd, GPIOB);
//printf("VALUE: %i\n", value);
data = (uint8_t)value;

}
else
 data = 0;

if (DEBUG_MODE)
{
printf("%s <  IN: ",KWHT);

if (port == DATA)
 printf("%s   DATA",KGRN );
else if (port == STATUS)
 printf("%s STATUS",KCYN );
else if (port == CONTROL)
 printf("%sCONTROL",KYEL );
else if (port == ECR)
 printf("%s   ECR",KMAG);
else
 printf("UNKNOWN PORT %u", port);

printf("| ");

printf("%03u\n%s",data,KWHT);
}
 return  data;
}




int init_MCP23017(int chipAddr)
{
static int fd = 0;
static int hasBeenInit = 0;
//int errCode = 0;
const char *device;
device = "/dev/i2c-0" ;

if (hasBeenInit == 1)//Don't Set Up
 return fd;

else//Set up
 {
 hasBeenInit = 1;
 fd = wiringPiI2CSetupInterface (device, chipAddr) ;

  //if (errCode != 1)
   //printf("Init Error #%i",errCode);

 //Set Port A to outputs (CONTROL)
 wiringPiI2CWriteReg8 (fd, IODIRA, 0x00) ;
 changePortDir(1);//Set Port B to Inputs

 return fd;
 }

return -1;
}




int close_MCP23017(int chipAddr)
{
return 1;
}


