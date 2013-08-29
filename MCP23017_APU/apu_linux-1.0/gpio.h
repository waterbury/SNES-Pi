/**
 * @file
 *  @brief Contains defines for gpio.c.
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

#ifndef _GPIO_H_
#define _GPIO_H_

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

/** The size the GPIO mapping is required to be. GPPUDCLK1_OFFSET is the last
 ** register offset of interest. */
#define GPIO_MAP_SIZE               (GPPUDCLK1_OFFSET)

/** Number of GPIO pins which are available on the Raspberry Pi. */
#define NUMBER_GPIO                 17

/** Delay for changing pullup/pulldown resistors. It should be at least 150
 ** cycles which is 0.6 uS (1 / 250 MHz * 150).  (250 Mhz is the core clock)*/
#define RESISTOR_SLEEP_US           1

/** @brief GPSET_0 register */
#define GPIO_GPSET0     *(gGpioMap + GPSET0_OFFSET / sizeof(uint32_t))
/** @brief GPIO_GPCLR0 register */
#define GPIO_GPCLR0     *(gGpioMap + GPCLR0_OFFSET / sizeof(uint32_t))
/** @brief GPIO_GPLEV0 register */
#define GPIO_GPLEV0     *(gGpioMap + GPLEV0_OFFSET / sizeof(uint32_t))
/** @brief GPIO_GPPUD register */
#define GPIO_GPPUD      *(gGpioMap + GPPUD_OFFSET / sizeof(uint32_t))
/** @brief GPIO_GPPUDCLK0 register */
#define GPIO_GPPUDCLK0  *(gGpioMap + GPPUDCLK0_OFFSET / sizeof(uint32_t))

#endif /*_GPIO_H_*/


