/**
 * @file
 *  @brief Contains source for the GPIO functionality.
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

#include "gpio.h"

/* Local / internal prototypes */
static errStatus gpioValidatePin(int gpioNumber);

/**** Globals ****/
/** @brief Pointer which will be mmap'd to the GPIO memory in /dev/mem */
static volatile uint32_t * gGpioMap = NULL;

/** @brief PCB revision that executable is being run on */
static tPcbRev pcbRev = pcbRevError;

/**
 * @brief   Maps the memory used for GPIO access. This function must be called
 *          prior to any of the other GPIO calls.
 * @return  An error from #errStatus. */
errStatus gpioSetup(void)
{
    int mem_fd = 0;
    errStatus rtn = ERROR_DEFAULT;

    if ((mem_fd = open("/dev/mem", O_RDWR)) < 0)
    {
        dbgPrint(DBG_INFO, "open() failed. /dev/mem. errno %s.", strerror(errno));
        rtn = ERROR_EXTERNAL;
    }

    else if ((gGpioMap = (volatile uint32_t *)mmap(NULL,
                                                   GPIO_MAP_SIZE,
                                                   PROT_READ|PROT_WRITE,
                                                   MAP_SHARED,
                                                   mem_fd,
                                                   GPIO_BASE)) == MAP_FAILED)
    {
        dbgPrint(DBG_INFO, "mmap() failed. errno: %s.", strerror(errno));
        rtn = ERROR_EXTERNAL;
    }

    /* Close the fd, we have now mapped it */
    else if (close(mem_fd) != OK)
    {
        dbgPrint(DBG_INFO, "close() failed. errno: %s.", strerror(errno));
        rtn = ERROR_EXTERNAL;
    }

    else
    {
        FILE* cpuinfo = fopen("/proc/cpuinfo", "r");
        if (cpuinfo)
        {
            char* line = NULL;
            ssize_t linelen;
            size_t foo;

            while (((linelen = getline(&line, &foo, cpuinfo)) >= 0))
            {
                if (strstr(line, "Revision") == line)
                {
                    char* rev = strstr(line, ":");
                    if (rev)
                    {
                        long revision = strtol(rev + 1, NULL, 16);

                        if (revision <= 3)
                        {
                            pcbRev = pcbRev1;
                        }
                        
                        else
                        {
                            pcbRev = pcbRev2;
                        }
                    }
                }
            } /* while */
            if (pcbRev != pcbRevError)
            {
                rtn = OK;
            }
            else
            {
                dbgPrint(DBG_INFO, "did not find revision in cpuinfo.");
                rtn = ERROR_EXTERNAL;
            }

            if (line)
            {
                free(line);
            }
            fclose(cpuinfo);
        }
        else
        {
            dbgPrint(DBG_INFO, "can't open /proc/cpuinfo. errno: %s.", strerror(errno));
            rtn = ERROR_EXTERNAL;
        }
    }

    return rtn;
}


/**
 * @brief   Unmaps the memory used for the gpio pins. This function should be
 *          called when finished with the GPIO pins.
 * @return  An error from #errStatus. */
errStatus gpioCleanup(void)
{
    errStatus rtn = ERROR_DEFAULT;

    if (gGpioMap == NULL)
    {
        dbgPrint(DBG_INFO, "gGpioMap was NULL. Ensure gpioSetup() was called successfully.");
        rtn = ERROR_NULL;
    }

    else if (munmap((void *)gGpioMap, GPIO_MAP_SIZE) != OK)
    {
        dbgPrint(DBG_INFO, "mummap() failed. errno %s.", strerror(errno));
        rtn = ERROR_EXTERNAL;
    }

    else
    {
        gGpioMap = NULL;
        rtn = OK;
    }
    return rtn;
}


/**
 * @brief               Sets the functionality of the desired pin.
 * @param gpioNumber    The gpio pin number to change.
 * @param function      The desired functionality for the pin.
 * @return              An error from #errStatus. */
errStatus gpioSetFunction(int gpioNumber, eFunction function)
{
    errStatus rtn = ERROR_DEFAULT;

    if (gGpioMap == NULL)
    {
        dbgPrint(DBG_INFO, "gGpioMap was NULL. Ensure gpioSetup() called successfully.");
        rtn = ERROR_NULL;
    }

    else if (function < eFunctionMin || function > eFunctionMax)
    {
        dbgPrint(DBG_INFO, "eFunction was out of range. %d", function);
        rtn = ERROR_RANGE;
    }

    else if ((rtn = gpioValidatePin(gpioNumber)) != OK)
    {
        dbgPrint(DBG_INFO, "gpioValidatePin() failed. Ensure pin %d is valid.", gpioNumber);
    }

    else
    {
        /* Clear what ever function bits currently exist - this puts the pin
         * into input mode.*/
        *(gGpioMap + (gpioNumber / 10)) &= ~(GPFSEL_BITS << ((gpioNumber % 10) * 3));

        /* Set the three pins for the pin to the desired value */
        *(gGpioMap + (gpioNumber / 10)) |=  (function << ((gpioNumber % 10) * 3));

        rtn = OK;
    }

    return rtn;
}


/**
 * @brief               Sets a pin to high or low.
 * @details             The pin should be configured as an ouput with
 *                      gpioSetFunction() prior to this.
 * @param gpioNumber    The pin to set.
 * @param state         The desired state of the pin.
 * @return              An error from #errStatus.*/
errStatus gpioSetPin(int gpioNumber, eState state)
{
    errStatus rtn = ERROR_DEFAULT;

    if (gGpioMap == NULL)
    {
       dbgPrint(DBG_INFO, "gGpioMap was NULL. Ensure gpioSetup() was called successfully.");
       rtn = ERROR_NULL;
    }

    else if ((rtn = gpioValidatePin(gpioNumber)) != OK)
    {
       dbgPrint(DBG_INFO, "gpioValidatePin() failed. Ensure pin %d is valid.", gpioNumber);
    }

    else if (state == high)
    {
        /* The offsets are all in bytes. Divide by sizeof uint32_t to allow
         * pointer addition. */
        GPIO_GPSET0 = 0x1 << gpioNumber;
        rtn = OK;
    }

    else if (state == low)
    {
        /* The offsets are all in bytes. Divide by sizeof uint32_t to allow
         * pointer addition. */
        GPIO_GPCLR0 = 0x1 << gpioNumber;
        rtn = OK;
    }

    else
    {
       dbgPrint(DBG_INFO,"state %d should have been %d or %d", state, low, high);
       rtn = ERROR_RANGE;
    }

    return rtn;
}


/**
 * @brief               Reads the current state of a gpio pin.
 * @param gpioNumber    The number of the GPIO pin to read.
 * @param[out] state    Pointer to the variable in which the GPIO pin state is
 *                      returned.
 * @return              An error from #errStatus. */
errStatus gpioReadPin(int gpioNumber, eState * state)
{
    errStatus rtn = ERROR_DEFAULT;

    if (gGpioMap == NULL)
    {
        dbgPrint(DBG_INFO, "gGpioMap was NULL. Ensure gpioSetup() was called successfully.");
        rtn = ERROR_NULL;
    }

    else if (state == NULL)
    {
        dbgPrint(DBG_INFO, "Parameter state was NULL.");
        rtn = ERROR_NULL;
    }

    else if ((rtn = gpioValidatePin(gpioNumber)) != OK)
    {
        dbgPrint(DBG_INFO, "gpioValidatePin() failed. Pin %d isn't valid.", gpioNumber);
    }

    else
    {
        /* Check if the appropriate bit is high */
        if (GPIO_GPLEV0 & (0x1 << gpioNumber))
        {
            *state = high;
        }

        else
        {
            *state = low;
        }

        rtn = OK;
    }

    return rtn;
}

/**
 * @brief                Allows configuration of the internal resistor at a GPIO pin.
 * @details              The GPIO pins on the BCM2835 have the option of configuring a
 *                       pullup, pulldown or no resistor at the pin.
 * @param gpioNumber     The GPIO pin to configure.
 * @param resistorOption The available resistor options.
 * @return               An error from #errStatus. */
errStatus gpioSetPullResistor(int gpioNumber, eResistor resistorOption)
{
    errStatus rtn = ERROR_DEFAULT;
    struct timespec sleepTime;

    if (gGpioMap == NULL)
    {
       dbgPrint(DBG_INFO, "gGpioMap was NULL. Ensure gpioSetup() was called successfully.");
       rtn = ERROR_NULL;
    }

    else if ((rtn = gpioValidatePin(gpioNumber)) != OK)
    {
       dbgPrint(DBG_INFO, "gpioValidatePin() failed. Pin %d isn't valid.", gpioNumber);
    }

    else if (resistorOption < pullDisable || resistorOption > pullup)
    {
       dbgPrint(DBG_INFO, "resistorOption value: %d was out of range.", resistorOption);
       rtn = ERROR_RANGE;
    }

    else
    {
        sleepTime.tv_sec  = 0;
        sleepTime.tv_nsec = 1000 * RESISTOR_SLEEP_US;

        /* Set the GPPUD register with the desired resistor type */
        GPIO_GPPUD = resistorOption;
        /* Wait for control signal to be set up */
        nanosleep(&sleepTime, NULL);
        /* Clock the control signal for desired resistor */
        GPIO_GPPUDCLK0 = (0x1 << gpioNumber);
        /* Hold to set */
        nanosleep(&sleepTime, NULL);
        GPIO_GPPUD = 0;
        GPIO_GPPUDCLK0 = 0;

        rtn = OK;
    }


    return rtn;
}

/**
 * @brief                       Get the correct I2C pins.
 * @details                     The different revisions of the PI have their I2C
 *                              ports on different GPIO
 *                              pins which require different BSC modules.
 * @param[out] gpioNumberScl    Integer to be populated with scl gpio number.
 * @param[out] gpioNumberSda    Integer to be populated with sda gpio number.
 * @todo TODO                   Does this need to be public or internal only?
 * @return                      An error from #errStatus. */
errStatus gpioGetI2cPins(int * gpioNumberScl, int * gpioNumberSda)
{
    errStatus rtn = ERROR_DEFAULT;

    if (gGpioMap == NULL)
    {
        dbgPrint(DBG_INFO, "gGpioMap was NULL. Ensure gpioSetup() was called successfully.");
        rtn = ERROR_NULL;
    }

    else if (gpioNumberScl == NULL)
    {
        dbgPrint(DBG_INFO, "Parameter gpioNumberScl is NULL.");
        rtn = ERROR_NULL;
    }
    
    else if (gpioNumberSda == NULL)
    {
        dbgPrint(DBG_INFO, "Parameter gpioNumberSda is NULL.");
        rtn = ERROR_NULL;
    }

    else if (pcbRev == pcbRev1)
    {
        *gpioNumberScl = REV1_SCL;
        *gpioNumberSda = REV1_SDA;
        rtn = OK;
    }

    else if (pcbRev == pcbRev2)
    {
        *gpioNumberScl = REV2_SCL;
        *gpioNumberSda = REV2_SDA;
        rtn = OK;
    }

    return rtn;
}


#undef  ERROR
/** Redefining to replace macro with x as a string, i.e. "x". For use in
  * gpioErrToString() */
#define ERROR(x) #x,

/**
 * @brief       Debug function which converts an error from errStatus to a string.
 * @param error Error from #errStatus.
 * @return      String representation of errStatus parameter error. */
const char * gpioErrToString(errStatus error)
{
    static const char * errorString[] = { ERRORS };

    if (error < 0 || error >= ERROR_MAX)
    {
        return "InvalidError";
    }

    else
    {
        return errorString[error];
    }
}


/**
 * @brief            Debug function wrapper for fprintf().
 * @details          Allows file and line information to be added easier
 *                   to output strings. #DBG_INFO is a macro which is useful
 *                   to call as the "first" parameter to this function. Note
 *                   this function will add on a newline to the end of a format
 *                   string so one is generally not required in \p format.
 * @param[in] stream Output stream for strings, e.g. stderr, stdout.
 * @param[in] file   Name of file to be printed. Should be retrieved with __FILE__.
 * @param line       Line number to print. Should be retrieved with __LINE__.
 * @param[in] format Formatted string in the format which printf() would accept.
 * @param ...        Additional arguments - to fill in placeholders in parameter
 *                   \p format.
 * @return           This function uses the printf() family functions and the
 *                   returned integer is what is returned from these calls: If
 *                   successful the number or characters printed is returned,
 *                   if unsuccessful a negative value.
 */
int dbgPrint(FILE * stream, const char * file, int line, const char * format, ...)
{
    va_list arguments;
    int rtn = 0;
    int tempRtn = 0;

    if (stream != NULL)
    {
        if ((tempRtn = fprintf(stream,"[%s:%d] ", file, line)) < 0)
        {
            return tempRtn;
        }
        rtn += tempRtn;

        va_start(arguments, format);
        if ((tempRtn = vfprintf(stream, format, arguments)) < 0)
        {
            return tempRtn;
        }
        rtn += tempRtn;
        va_end(arguments);

        if ((tempRtn = fprintf(stream,"\n")) < 0)
        {
            return tempRtn;
        }
        rtn += tempRtn;

    }
    return rtn;
}

/****************************** Internal Functions ******************************/

/**
 * @brief               Internal function which Validates that the pin
 *                      \p gpioNumber is valid for the Raspberry Pi.
 * @details             The first time this function is called it will perform
 *                      some basic initalisation on internal variables.
 * @param gpioNumber    The pin number to check.
 * @return              An error from #errStatus. */
static errStatus gpioValidatePin(int gpioNumber)
{
    errStatus rtn = ERROR_INVALID_PIN_NUMBER;
    int index = 0;
    /* TODO REV1 and REV2 have the same pincount. REV2 technically has more if 
     * P5 is supported. If there is a REV3 the size of this array will need to
     * be addressed. */
    static uint32_t validPins[REV2_PINCNT] = {0};
    static uint32_t pinCnt = 0;

    if (pinCnt == 0)
    {
        if (pcbRev == pcbRevError)
        {
            rtn = ERROR_RANGE;
        }

        else if (pcbRev == pcbRev1)
        {
            const uint32_t validPinsForRev1[REV1_PINCNT] = REV1_PINS;
            memcpy(validPins, validPinsForRev1, sizeof(validPinsForRev1));
            pinCnt = REV1_PINCNT;
        }
        else if (pcbRev == pcbRev2)
        {
            const uint32_t validPinsForRev2[REV2_PINCNT] = REV2_PINS;
            memcpy(validPins, validPinsForRev2, sizeof(validPinsForRev2));
            pinCnt = REV2_PINCNT;
        }
    }

    for (index = 0; index < pinCnt; index++)
    {
        if (gpioNumber == validPins[index])
        {
            rtn = OK;
            break;
        }
    }

    return rtn;
}


