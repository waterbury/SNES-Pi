#ifndef PARPORT_H
#define PARPORT_H	1

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
int BidirAvailable(void);
int EnableBidir(void);
void begin_config_mode(int chip);
void end_config_mode(void);
void begin_EPP(int port_addr, int chip);
int SetupBidir(void);
void TestPort(void);
  

#endif
