#include <stdint.h>
#include "parport.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiringPiI2C.h>
#include "wiringPiSPI.h"
//#include "mcp23x0817.h"
#include <wiringPi.h>
#include <mcp23s17.h>

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

#define GPIO_RD_PIN 3
#define GPIO_WR_PIN 2
#define GPIO_ADDR_0 7
#define GPIO_ADDR_1 0
#define GPIO_RESET  5
#define GPIO_LVLSFT_EN 4

#define MCPBASE    123
#define devId 0
#define	CMD_WRITE	0x40
#define CMD_READ	0x41
#define USE_SPI_MODE      1
#define USE_GPIO_CONTROL  1
#define DEBUG_MODE        0

//#define	MCP_SPEED	4000000

const char *byte_to_binary(int x)
{
    static char b[9];
    b[0] = '\0';

    int z;
    for (z = 128; z > 0; z >>= 1)
    {
        strcat(b, ((x & z) == z) ? "1" : "0");
    }

    return b;
}

void write_GPIO_CONTROL(uint8_t data)
 {
  init_MCP23S17();
  digitalWrite (GPIO_LVLSFT_EN,0);

  static int  previous_PA0_PIN   = -1;
  static int  previous_PA1_PIN   = -1;
  static int  previous_WR_PIN    = -1;
  static int  previous_RD_PIN    = -1;
  static int  previous_RESET_PIN = -1;
  int current_PA0_PIN   = 0;
  int current_PA1_PIN   = 0;
  int current_WR_PIN    = 0;
  int current_RD_PIN    = 0;
  int current_RESET_PIN = 0;

  if ( (data & PA0_PIN) == PA0_PIN  )
    current_PA0_PIN = 1;
  if ( (data & PA1_PIN) == PA1_PIN  )
    current_PA1_PIN = 1;
  if ( (data &  WR_PIN) ==  WR_PIN  )
    current_WR_PIN = 1;
  if ( (data &  RD_PIN) ==  RD_PIN  )
    current_RD_PIN = 1;
  if ( (data & RESET_PIN) == RESET_PIN  )
    current_RESET_PIN = 1;

//printf("previous_PA1_PIN: %i\n", previous_PA1_PIN);
  if (  current_PA0_PIN != previous_PA0_PIN  )
  {
    digitalWrite (GPIO_ADDR_0,  current_PA0_PIN) ;
    previous_PA0_PIN = current_PA0_PIN;
  }
  if ( current_PA1_PIN != previous_PA1_PIN  )
  {
    digitalWrite (GPIO_ADDR_1,  current_PA1_PIN) ;
    previous_PA1_PIN = current_PA1_PIN;
  }
  if ( current_RD_PIN != previous_RD_PIN  )
  {
    digitalWrite (GPIO_RD_PIN,  current_RD_PIN) ;
    previous_RD_PIN = current_RD_PIN;
  }
  if ( current_WR_PIN != previous_WR_PIN  )
  {
    digitalWrite (GPIO_WR_PIN,  current_WR_PIN) ;
    previous_WR_PIN = current_WR_PIN;
  }
  if ( current_RESET_PIN != previous_RESET_PIN  )
  {
    digitalWrite (GPIO_RESET,  current_RESET_PIN) ;
    previous_RESET_PIN = current_RESET_PIN;
  }

 //printf("current_PA1_PIN: %i\n", current_PA1_PIN);
 }




void write_MCP23017(int MCPregister, uint8_t data)
 {
  int fd = 0;
  fd = init_MCP23017(CHIPADDR);
  wiringPiI2CWriteReg8 (fd, MCPregister, (int)data);
 }

void write_MCP23S17(int MCPregister, uint8_t data)
 {
  init_MCP23S17();
  uint8_t spiData [4] ;

  spiData [0] = CMD_WRITE | ((devId & 7) << 1) ;
  spiData [1] = (uint8_t)MCPregister ;
  spiData [2] = data ;
  wiringPiSPIDataRW (0, spiData, 3) ;
 }


uint8_t read_MCP23017_data()
 {
  int fd = 0;
  uint8_t data = 0;
  int value =0;
  fd = init_MCP23017(CHIPADDR);
  value = wiringPiI2CReadReg8 (fd, GPIOB);
  data = (uint8_t)value;
  return data;
 }

uint8_t read_MCP23S17_data()
 {
 init_MCP23S17();
 uint8_t spiData [4] ;

  spiData [0] = CMD_READ | ((devId & 7) << 1) ;
  spiData [1] = GPIOB ;

  wiringPiSPIDataRW (0, spiData, 3) ;

  return spiData [2] ;
 }


int changeDataPortDir(int direction)
{
static int currentDirection = -10;
if ( (direction == 1) | (direction == 0) )
 {
  if (direction != currentDirection)
   {
   if(USE_SPI_MODE)
    change_MCP23S17_dir(IODIRB, direction);
   else
    change_MCP23017_dir(IODIRB, direction);

   currentDirection = direction;
   }
  }
 return currentDirection;
}

void change_MCP23017_dir(int IODIRport, int direction)
{
uint8_t value = 0;
if (direction == 1)
 value = 0xFF;
else
 value = 0;

write_MCP23017(IODIRport, value);
}

void change_MCP23S17_dir(int IODIRport, int direction)
{
 init_MCP23S17();
 uint8_t value = 0;
 if (direction == 1)
  value = 0xFF;
 else
  value = 0;

 write_MCP23S17(IODIRport, value);
}






uint8_t remapSNESpins(uint8_t data)
{
uint8_t newData = 0x00;
int resetCounter = 0;

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


void    outb_MCP23X17(uint8_t data,int port)
{
static uint8_t previous_data = 0;
init_MCP23S17();


if (port == CONTROL)
 {
  if ( (data & 0x20) == 0x20 )//change direction bit
   changeDataPortDir(1);
  else
   changeDataPortDir(0);

  data = data ^ 0x0B;//Inverts control Pins 0, 1, and 3
  data = remapSNESpins(data);//Maps original pins of sch to GPIO pins

  if (previous_data != data)//Don't Rewrite control if not needed
   {

    if (USE_GPIO_CONTROL == 1)
      write_GPIO_CONTROL(data);
    else
     {
      if (USE_SPI_MODE)
       write_MCP23S17(GPIOA, data ) ;
      else
       write_MCP23017(GPIOA, data ) ;
     }
   }
  else
   return;

  previous_data = data;
 }
else if(port == DATA)
{
 //changeDataPortDir(0);
 if (changeDataPortDir(-1) == 0)//If in output mode
  {
     if (USE_SPI_MODE)
       write_MCP23S17(GPIOB, data ) ;
      else
       write_MCP23017(GPIOB, data ) ;

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




uint8_t inb_MCP23X17(int port)
{
uint8_t data = 0;

changeDataPortDir(1);
if (changeDataPortDir(-1) == 1)//input mode
{
if (USE_SPI_MODE)
 data = read_MCP23S17_data();
else
 data = read_MCP23017_data();

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

    if(USE_GPIO_CONTROL == 1)
    {
     write_MCP23017(IODIRA, 0xFF);//Set Port A to inputs (CONTROL)
     //wiringPiSetupGpio();
     pinMode (GPIO_LVLSFT_EN, OUTPUT) ;
     pinMode (GPIO_RD_PIN, OUTPUT) ;
     pinMode (GPIO_WR_PIN, OUTPUT) ;
     pinMode (GPIO_ADDR_0, OUTPUT) ;
     pinMode (GPIO_ADDR_1, OUTPUT) ;

     digitalWrite (GPIO_LVLSFT_EN, 0) ;
    }
    else
     write_MCP23017(IODIRA, 0x00);//Set Port A to outputs (CONTROL)

 changeDataPortDir(1);//Set Port B to Inputs

 return fd;
 }

return -1;
}




int close_MCP23017(int chipAddr)
{
return 1;
}






void init_MCP23S17()
{
static int hasBeenInit = 0;

if (hasBeenInit == 1)//Don't Set Up
 return;

else//Set up
 {
 hasBeenInit = 1;

 wiringPiSetup () ;
 wiringPiSPISetup(0,10000000);
 //mcp23s17Setup (MCPBASE, 0, 0) ;

 if (USE_GPIO_CONTROL)
  {
   change_MCP23S17_dir(IODIRA, 1);

   //wiringPiSetupGpio();
   pinMode (GPIO_LVLSFT_EN, OUTPUT) ;
   pinMode (GPIO_RD_PIN, OUTPUT) ;
   pinMode (GPIO_WR_PIN, OUTPUT) ;
   pinMode (GPIO_ADDR_0, OUTPUT) ;
   pinMode (GPIO_ADDR_1, OUTPUT) ;

  }
 else
  change_MCP23S17_dir(IODIRA, 0);

 changeDataPortDir(1);//Set Port B to Inputs

 return;
 }

return;
}



int close_MCP23S17()
{
return 1;

}


