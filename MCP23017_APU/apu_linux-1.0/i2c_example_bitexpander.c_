/*
 *  I2C Example Bit Expander:
 *  The following is an example of using a MCP23017 I2C bit expander 
 *  with the Raspberry Pi.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Tested Setup:
 *
 * ______        ____________
 *   3V3|-------|VCC     A.0|----LED----GND
 *   GND|-------|GND     A.1|----LED----GND
 *   SDA|-------|SDA     A.2|----LED----GND
 *   SCL|-------|SCL     A.3|----LED----GND
 *      |       |           |
 * R. Pi|       | MCP23017  |            
 * _____|       |___________|
 *
 *
 *
 */     

#include <stdio.h>
#include <unistd.h>
#include "rpiGpio.h"

#define MCP23017_IODIRA     0x00
#define MCP23017_GPIOA      0x12
#define MCP23017_ADDRESS    0x20

int main(void)
{ 
    uint8_t TxData[4] = {MCP23017_IODIRA, 0x00, MCP23017_GPIOA, 0x0F};
    int ctr;
    errStatus rtn;

    if ((rtn = gpioSetup()) != OK)
    {
        dbgPrint(DBG_INFO, "gpioSetup failed. Exiting\n");
        return 1;
    }

    else if (gpioI2cSetup() != OK)
    {
        dbgPrint(DBG_INFO, "gpioI2cSetup failed. Exiting\n");
        return 1;
    }
   
    if (gpioI2cSet7BitSlave(MCP23017_ADDRESS) != OK)
    {
        dbgPrint(DBG_INFO, "gpioI2cSet7BitSlave failed. Exiting\n");
        return 1;
    }

    if (gpioI2cWriteData(&TxData[0], 2) != OK)
    {
        dbgPrint(DBG_INFO, "gpioI2cWriteData failed. Exiting\n");
        return 1;
    }

    gpioI2cWriteData(&TxData[2], 2);  

    for(ctr = 15; ctr >= 0; ctr--)
    {
        /* Set the state of output pins to counter */
        TxData[3] = (uint8_t)ctr;

        sleep(1);

        gpioI2cWriteData(&TxData[2], 2);  
    }

    gpioI2cCleanup();
    gpioCleanup();
    return 0;
} 



