/**
 * @file
 *  @brief Contains defines for i2c.c.
 *
 *  This is is part of https://github.com/alanbarr/RaspberryPi-GPIO
 *  a C library for basic control of the Raspberry Pi's GPIO pins.
 *  Copyright (C) Alan Barr 2012
 *
 *  This code was loosely based on the example code
 *  provided by Dom and Gert found at:
 *      http://elinux.org/RPi_Low-level_peripherals
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
 */

#ifndef _I2C_H_
#define _I2C_H_

#include "rpiGpio.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>

/** @brief The size the I2C mapping is required to be. */
#define I2C_MAP_SIZE                BSC_DEL_OFFSET

/** @brief Default I2C clock frequency (Hertz) */
#define I2C_DEFAULT_FREQ_HZ         100000

/** @brief nano seconds in a second */
#define NSEC_IN_SEC                 1000000000

/** @brief Clock pulses per I2C byte - 8 bits + ACK */
#define CLOCKS_PER_BYTE             9

/** @brief BSC_C register */
#define I2C_C                       *(gI2cMap + BSC_C_OFFSET / sizeof(uint32_t))
/** @brief BSC_DIV register */
#define I2C_DIV                     *(gI2cMap + BSC_DIV_OFFSET / sizeof(uint32_t))
/** @brief BSC_A register */
#define I2C_A                       *(gI2cMap + BSC_A_OFFSET / sizeof(uint32_t))
/** @brief BSC_DLEN register */
#define I2C_DLEN                    *(gI2cMap + BSC_DLEN_OFFSET / sizeof(uint32_t))
/** @brief BSC_S register */
#define I2C_S                       *(gI2cMap + BSC_S_OFFSET / sizeof(uint32_t))
/** @brief BSC_FIFO register */
#define I2C_FIFO                    *(gI2cMap + BSC_FIFO_OFFSET / sizeof(uint32_t))


#endif /*_I2C_H_*/

