/**
 * @file
 *  @brief Header file for BCM2835 GPIO registers.
 *
 *  This is is part of https://github.com/alanbarr/RaspberryPi-GPIO
 *  a C library for basic control of the Raspberry Pi's GPIO pins.
 *  Copyright (C) Alan Barr 2012
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
 *  The addresses of the GPIO registers are the physical addresses of the GPIO
 *  registers as available through /dev/mem.
 *  From page 6 of BCM2835 ARM Peripherals:
 *      Physical addresses range from 0x20000000 to 0x20FFFFFF for peripherals.
 *      The bus addresses for peripherals are set up to map onto the peripheral
 *      bus address range starting at 0x7E000000.
 *      Thus a peripheral advertised here at bus address 0x7Ennnnnn is available
 *      at physical address 0x20nnnnnn.
*/

#ifndef _BCM_2835_
#define _BCM_2835_

/******************************************************************************/
/* The following are the physical GPIO addresses                              */
/******************************************************************************/
#define GPFSEL0                 0x20200000  /**<GPIO Function Select 0 Register Address */
#define GPFSEL1                 0x20200004  /**<GPIO Function Select 1 Register Address */
#define GPFSEL2                 0x20200008  /**<GPIO Function Select 2 Register Address */
#define GPFSEL3                 0x2020000C  /**<GPIO Function Select 3 Register Address */
#define GPFSEL4                 0x20200010  /**<GPIO Function Select 4 Register Address */
#define GPFSEL5                 0x20200014  /**<GPIO Function Select 5 Register Address */

#define GPSET0                  0x2020001C  /**<GPIO Pin Output Set 0 Register Address */
#define GPSET1                  0x20200020  /**<GPIO Pin Output Set 1 Register Address */

#define GPCLR0                  0x20200028  /**<GPIO Pin Output Clear 0 Register Address */
#define GPCLR1                  0x2020002C  /**<GPIO Pin Output Clear 1 Register Address */

#define GPLEV0                  0x20200034  /**<GPIO Pin Level 0 Register Address */
#define GPLEV1                  0x20200038  /**<GPIO Pin Level 1 Register Address */

#define GPEDS0                  0x20200040  /**<GPIO Pin Event Detect Status 0 Register Address */
#define GPEDS1                  0x20200044  /**<GPIO Pin Event Detect Status 1 Register Address */

#define GPREN0                  0x2020004C  /**<GPIO Pin Rising Edge Detect Enable 0 Register Address */
#define GPREN1                  0x20200050  /**<GPIO Pin Rising Edge Detect Enable 1 Register Address */

#define GPHEN0                  0x20200064  /**<GPIO Pin High Detect Enable 0 Register Address */
#define GPHEN1                  0x20200068  /**<GPIO Pin High Detect Enable 1 Register Address */

#define GPAREN0                 0x2020007C  /**<GPIO Pin Async. Rising Edge Detect 0 Register Address */
#define GPAREN1                 0x20200080  /**<GPIO Pin Async. Rising Edge Detect 1 Register Address */

#define GPAFEN0                 0x20200088  /**<GPIO Pin Async. Falling Edge Detect 0 Register Address */
#define GPAFEN1                 0x2020008C  /**<GPIO Pin Async. Falling Edge Detect 1 Register Address */

#define GPPUD                   0x20200094  /**<GPIO Pin Pull-up/down Enable Register Address */

#define GPPUDCLK0               0x20200098  /**<GPIO Pin Pull-up/down Enable Clock 0 Register Address */
#define GPPUDCLK1               0x2020009C  /**<GPIO Pin Pull-up/down Enable Clock 1 Register Address */


/**********************************************************************************/
/* The following are offset address which can be used with a pointer to GPIO_BASE */
/**********************************************************************************/
#define GPIO_BASE               GPFSEL0   /**< First GPIO address of interest. */

#define GPFSEL0_OFFSET          0x000000  /**< GPIO Function Select 0 Offset from GPIO_BASE */
#define GPFSEL1_OFFSET          0x000004  /**< GPIO Function Select 1 Offset from GPIO_BASE */
#define GPFSEL2_OFFSET          0x000008  /**< GPIO Function Select 2 Offset from GPIO_BASE */
#define GPFSEL3_OFFSET          0x00000C  /**< GPIO Function Select 3 Offset from GPIO_BASE */
#define GPFSEL4_OFFSET          0x000010  /**< GPIO Function Select 4 Offset from GPIO_BASE */
#define GPFSEL5_OFFSET          0x000014  /**< GPIO Function Select 5 Offset from GPIO_BASE */

#define GPSET0_OFFSET           0x00001C  /**< GPIO Pin Output Set 0 Offset from GPIO_BASE */
#define GPSET1_OFFSET           0x000020  /**< GPIO Pin Output Set 1 Offset from GPIO_BASE */

#define GPCLR0_OFFSET           0x000028  /**< GPIO Pin Output Clear 0 Offset from GPIO_BASE */
#define GPCLR1_OFFSET           0x00002C  /**< GPIO Pin Output Clear 1 Offset from GPIO_BASE */

#define GPLEV0_OFFSET           0x000034  /**< GPIO Pin Level 0 Offset from GPIO_BASE */
#define GPLEV1_OFFSET           0x000038  /**< GPIO Pin Level 1 Offset from GPIO_BASE */

#define GPEDS0_OFFSET           0x000040  /**< GPIO Pin Event Detect Status 0 Offset from GPIO_BASE */
#define GPEDS1_OFFSET           0x000044  /**< GPIO Pin Event Detect Status 1 Offset from GPIO_BASE */

#define GPREN0_OFFSET           0x00004C  /**< GPIO Pin Rising Edge Detect Enable 0 Offset from GPIO_BASE */
#define GPREN1_OFFSET           0x000050  /**< GPIO Pin Rising Edge Detect Enable 1 Offset from GPIO_BASE */

#define GPHEN0_OFFSET           0x000064  /**< GPIO Pin High Detect Enable 0 Offset from GPIO_BASE */
#define GPHEN1_OFFSET           0x000068  /**< GPIO Pin High Detect Enable 1 Offset from GPIO_BASE */

#define GPAREN0_OFFSET          0x00007C  /**< GPIO Pin Async. Rising Edge Detect 0 Offset from GPIO_BASE */
#define GPAREN1_OFFSET          0x000080  /**< GPIO Pin Async. Rising Edge Detect 1 Offset from GPIO_BASE */

#define GPAFEN0_OFFSET          0x000088  /**< GPIO Pin Async. Falling Edge Detect 0 Offset from GPIO_BASE */
#define GPAFEN1_OFFSET          0x00008C  /**< GPIO Pin Async. Falling Edge Detect 1 Offset from GPIO_BASE */

#define GPPUD_OFFSET            0x000094  /**< GPIO Pin Pull-up/down Enable Offset from GPIO_BASE */

#define GPPUDCLK0_OFFSET        0x000098  /**< GPIO Pin Pull-up/down Enable Clock 0 Offset from GPIO_BASE */
#define GPPUDCLK1_OFFSET        0x00009C  /**< GPIO Pin Pull-up/down Enable Clock 1 Offset from GPIO_BASE */


/**********************************************************************************/
/* Function select bits for GPFSELX. In GPFSELX registers each pin has three
 * bits associated with it. */
/**********************************************************************************/
#define GPFSEL_INPUT            0x0 /**< Sets a pin to input mode */
#define GPFSEL_OUTPUT           0x1 /**< Sets a pin to output mode */
#define GPFSEL_ALT0             0x4 /**< Sets a pin to alternative function 0 */
#define GPFSEL_ALT1             0x5 /**< Sets a pin to alternative function 1 */
#define GPFSEL_ALT2             0x6 /**< Sets a pin to alternative function 2 */
#define GPFSEL_ALT3             0x7 /**< Sets a pin to alternative function 3 */
#define GPFSEL_ALT4             0x3 /**< Sets a pin to alternative function 4 */
#define GPFSEL_ALT5             0x2 /**< Sets a pin to alternative function 5 */
#define GPFSEL_BITS             0x7 /**< Three bits per GPIO in the GPFSEL register */

/* Function select bits for GPPUD - the pullup/pulldown resistor register */
#define GPPUD_DISABLE           0x0 /**< Disables the resistor */
#define GPPUD_PULLDOWN          0x1 /**< Enables a pulldown resistor */
#define GPPUD_PULLUP            0x2 /**< Enables a pullup resistor */


/******************************************************************************/
/* The following are the physical BSC / I2C addresses                         */
/******************************************************************************/
#define BSC0_C                  0x20205000 /**< BSC0 Control Register Address */
#define BSC0_S                  0x20205004 /**< BSC0 Status Register Address */
#define BSC0_DLEN               0x20205008 /**< BSC0 Data Length Register Address */
#define BSC0_A                  0x2020500C /**< BSC0 Slave Address Register Address */
#define BSC0_FIFO               0x20205010 /**< BSC0 Data FIFO Register Address */
#define BSC0_DIV                0x20205014 /**< BSC0 Clock Divider Register Address */
#define BSC0_DEL                0x20205018 /**< BSC0 Data Delay Register Address */

#define BSC1_C                  0x20804000 /**< BSC1 Control Register Address */
#define BSC1_S                  0x20804004 /**< BSC1 Status Register Address */
#define BSC1_DLEN               0x20804008 /**< BSC1 Data Length Register Address */
#define BSC1_A                  0x2080400C /**< BSC1 Slave Address Register Address */
#define BSC1_FIFO               0x20804010 /**< BSC1 Data FIFO Register Address */
#define BSC1_DIV                0x20804014 /**< BSC1 Clock Divider Register Address */
#define BSC1_DEL                0x20804018 /**< BSC1 Data Delay Register Address */

#define BSC2_C                  0x20805000 /**< BSC2 Control Register Address */
#define BSC2_S                  0x20805004 /**< BSC2 Status Register Address */
#define BSC2_DLEN               0x20805008 /**< BSC2 Data Length Register Address */
#define BSC2_A                  0x2080500C /**< BSC2 Slave Address Register Address */
#define BSC2_FIFO               0x20805010 /**< BSC2 Data FIFO Register Address */
#define BSC2_DIV                0x20805014 /**< BSC2 Clock Divider Register Address */
#define BSC2_DEL                0x20805018 /**< BSC2 Data Delay Register Address */

/**********************************************************************************/
/* The following are the base addresses for each BSC module                       */
/**********************************************************************************/
#define BSC0_BASE           BSC0_C      /**< BSC0 Base Address */
#define BSC1_BASE           BSC1_C      /**< BSC1 Base Address */
#define BSC2_BASE           BSC2_C      /**< BSC2 Base Address */

/**********************************************************************************/
/* The following are offset addresses which can be used with a pointer to the
 * appropriate BSC base */
/**********************************************************************************/
#define BSC_C_OFFSET        0x00000000  /**< BSC Control offset from BSCx_BASE */
#define BSC_S_OFFSET        0x00000004  /**< BSC Status offset from BSCx_BASE */
#define BSC_DLEN_OFFSET     0x00000008  /**< BSC Data Length offset from BSCx_BASE */
#define BSC_A_OFFSET        0x0000000C  /**< BSC Slave Address offset from BSCx_BASE */
#define BSC_FIFO_OFFSET     0x00000010  /**< BSC Data FIFO offset from BSCx_BASE */
#define BSC_DIV_OFFSET      0x00000014  /**< BSC Clock Divider offset from BSCx_BASE */
#define BSC_DEL_OFFSET      0x00000018  /**< BSC Data Delay offset from BSCx_BASE */


/**********************************************************************************/
/* The following are the BSC Control Register Bits                                */
/**********************************************************************************/
#define BSC_I2CEN           0x8000      /**< BSC Control: I2C Enable Bit */
#define BSC_INTR            0x0400      /**< BSC Control: Interrupt on RX bit */
#define BSC_INTT            0x0200      /**< BSC Control: Interrupt on TX bit */
#define BSC_INTD            0x0100      /**< BSC Control: Interrupt on DONE bit */
#define BSC_ST              0x0080      /**< BSC Control: Start transfer bit */
#define BSC_CLEAR           0x0010      /**< BSC Control: Clear FIFO bit */
#define BSC_READ            0x0001      /**< BSC Control: Read Packet Transfer bit */

/**********************************************************************************/
/* The following are the BSC Status Register Bits                                 */
/**********************************************************************************/
#define BSC_CLKT            0x200       /**< BSC Status: Clock Stretch Timeout bit */
#define BSC_ERR             0x100       /**< BSC Status: Ack Error bit */
#define BSC_RXF             0x080       /**< BSC Status: FIFO Full bit */
#define BSC_TXE             0x040       /**< BSC Status: FIFO Empty bit */
#define BSC_RXD             0x020       /**< BSC Status: FIFO Contains Data */
#define BSC_TXD             0x010       /**< BSC Status: FIFO Can Accept Data bit */
#define BSC_RXR             0x008       /**< BSC Status: FIFO Needs Reading bit */
#define BSC_TXW             0x004       /**< BSC Status: FIFO Needs Writing bit */
#define BSC_DONE            0x002       /**< BSC Status: Transfer Done */
#define BSC_TA              0x001       /**< BSC Status: Transfer Active */

#define BSC_FIFO_SIZE       16          /**< BSC FIFO Size */
#endif /* _BCM_2835_ */
