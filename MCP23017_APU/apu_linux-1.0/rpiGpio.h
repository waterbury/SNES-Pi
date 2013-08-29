/**
 * @file
 *  @brief The main header file of the GPIO library.
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
 */

#ifndef _RPI_GPIO_H_
#define _RPI_GPIO_H_

#include "bcm2835_gpio.h"
#include <stdio.h>
#include <stdint.h>

/**@brief Speed of the core clock core_clk */
#define CORE_CLK_HZ                 250000000

/** @brief The list of errors which may be returned from gpio functions.
 *  @details Errors are defined within #ERROR(x). */
#define ERRORS                          \
    ERROR(OK)                           \
    ERROR(ERROR_DEFAULT)                \
    ERROR(ERROR_INVALID_PIN_NUMBER)     \
    ERROR(ERROR_RANGE)                  \
    ERROR(ERROR_NULL)                   \
    ERROR(ERROR_EXTERNAL)               \
    ERROR(ERROR_NOT_INITIALISED)        \
    ERROR(ERROR_ALREADY_INITIALISED)    \
    ERROR(ERROR_I2C_NACK)               \
    ERROR(ERROR_I2C)                    \
    ERROR(ERROR_I2C_CLK_TIMEOUT)        \
    ERROR(ERROR_INVALID_BSC)        \


#undef  ERROR
/** @brief Redefining to replace the macro with x. */
#define ERROR(x) x,


/** @brief Minimum I2C frequency (Hertz) */
#define I2C_CLOCK_FREQ_MIN         10000

/** @brief Maximum I2C frequency (Hertz) */
#define I2C_CLOCK_FREQ_MAX         400000

/** @brief The enum of possible errors returned from gpio functions.
 *  Errors themselves are defined in the macro #ERRORS. */
typedef enum {
    ERRORS
    ERROR_MAX
} errStatus;

/** @brief The enum of possible pin states in input/output modes. */
typedef enum {
    low  = 0x0, /**< Pin low */
    high = 0x1  /**< Pin high */
} eState;

/** @brief The enum for possible pull resistors. */
typedef enum {
    pullDisable = GPPUD_DISABLE,  /**< No resistor */
    pulldown    = GPPUD_PULLDOWN, /**< Pulldown resistor */
    pullup      = GPPUD_PULLUP    /**< Pullup resistor */
} eResistor;

/** @brief The enum of pin functions available.
 ** @details The enum values are equivalent to those in the data sheet.
 ** @note Currently only pins configured for input and output have been
 ** tested. */
typedef enum {
    input  = GPFSEL_INPUT,        /**< Set pin to input */
    output = GPFSEL_OUTPUT,       /**< Set pin to output */
    alt0   = GPFSEL_ALT0,         /**< Set pin to alternative function 0 */
    alt1   = GPFSEL_ALT1,         /**< Set pin to alternative function 1 */
    alt2   = GPFSEL_ALT2,         /**< Set pin to alternative function 2 */
    alt3   = GPFSEL_ALT3,         /**< Set pin to alternative function 3 */
    alt4   = GPFSEL_ALT4,         /**< Set pin to alternative function 4 */
    alt5   = GPFSEL_ALT5,         /**< Set pin to alternative function 5 */
    eFunctionMin = GPFSEL_INPUT,  /**< Minimum valid value for enum */
    eFunctionMax = GPFSEL_ALT3    /**< Maximum valid value for enum */
} eFunction;

/* Function Prototypes */
errStatus gpioSetup(void);
errStatus gpioCleanup(void);
errStatus gpioSetFunction(int gpioNumber, eFunction function);
errStatus gpioSetPin(int gpioNumber, eState state);
errStatus gpioReadPin(int gpioNumber, eState * state);
errStatus gpioSetPullResistor(int gpioNumber, eResistor resistor);
errStatus gpioGetI2cPins(int * gpioNumberScl, int * gpioNumberSda);

errStatus gpioI2cSetup(void);
errStatus gpioI2cCleanup(void);
errStatus gpioI2cSetClock(int frequency);
errStatus gpioI2cSet7BitSlave(uint8_t slaveAddress);
errStatus gpioI2cWriteData(const uint8_t * data, uint16_t dataLength);
errStatus gpioI2cReadData(uint8_t * buffer, uint16_t bytesToRead);

const char * gpioErrToString(errStatus error);
int dbgPrint(FILE * stream, const char * file, int line, const char * format, ...);

/** @brief Macro which covers the first three arguments of dbgPrint. */
#define DBG_INFO stderr,__FILE__,__LINE__


/* Revision specific TODO not sure if it should be public maybe private revisions
 * header?*/
/** @brief Pin count on a PCB rev1 Raspberry Pi */
#define REV1_PINCNT 17
/** @brief Pin count on a PCB rev2 Raspberry Pi */
#define REV2_PINCNT 17

/** @ brief List of all BCM2835 pins available through the rev1 Raspberry Pi header */
#define REV1_PINS {0, 1, 4, 7, 8, 9, 10, 11, 14, 15, 17, 18, 21, 22, 23, 24, 25}
/** @ brief List of all BCM2835 pins available through the rev2 Raspberry Pi header */
#define REV2_PINS {2, 3, 4, 7, 8, 9, 10, 11, 14, 15, 17, 18, 22, 23, 24, 25, 27}

/** @brief The BCM2835 pin number of SDA on rev1 Raspberry Pi */
#define REV1_SDA 0
/** @brief The BCM2835 pin number of SCL on rev1 Raspberry Pi */
#define REV1_SCL 1
/** @brief The BCM2835 pin number of SDA on rev2 Raspberry Pi */
#define REV2_SDA 2
/** @brief The BCM2835 pin number of SCL on rev2 Raspberry Pi */
#define REV2_SCL 3

/** @brief valid PCB revision values */
typedef enum {
    pcbRevError = 0,
    pcbRev1 = 1,
    pcbRev2 = 2,
} tPcbRev;


#endif /* _RPI_GPIO_H_ */
