#include <stdint.h>

const char *byte_to_binary(int);
uint8_t remapSNESpins(uint8_t);

int changeDataPortDir(int);
void outb_MCP23X17(uint8_t,int);
uint8_t inb_MCP23X17(int);


void change_MCP23017_dir(int, int);
int init_MCP23017(int);
int init_MCP23017(int);
void write_MCP23017(int, uint8_t);
void write_MCP23S17(int, uint8_t);
uint8_t read_MCP23017_data();
uint8_t read_MCP23S17_data();
int close_MCP23017();

void write_GPIO_CONTROL(uint8_t);

uint8_t inb_MCP23S17(int);
void outb_MCP23S17(uint8_t,int);
int init_MCP23S17(int);
int close_MCP23S17();

