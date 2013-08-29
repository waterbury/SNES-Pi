/**
 * @file
 *  @brief Contains C source for the I2C functionality.
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
 */

#include "i2c.h"

/** @brief Pointer which will be mmap'd to the I2C memory in /dev/mem */
static volatile uint32_t * gI2cMap = NULL;

/** @brief The time it takes ideally transmit 1 byte with current I2C clock */
static int i2cByteTxTime_ns;

/**
 * @brief       Initial setup of I2C functionality.
 * @details     gpioSetup() should be called prior to this.
 * @return      An error from #errStatus. */
errStatus gpioI2cSetup(void)
{
    int mem_fd = 0;
    int sda;
    int scl;
    errStatus rtn = ERROR_DEFAULT;
    off_t bscBase;

    if ((rtn = gpioGetI2cPins(&scl, &sda)) != OK)
    {
        dbgPrint(DBG_INFO, "gpioGetI2cPins() failed. %s", gpioErrToString(rtn));
    }

    /* Find the correct BSC to use from I2C pins */
    else if (sda == REV1_SDA && scl == REV1_SCL)
    {
        bscBase = BSC0_BASE;
    }
    else if (sda == REV2_SDA && scl == REV2_SCL)
    {
        bscBase = BSC1_BASE;
    }
    else 
    {
        rtn = ERROR_INVALID_PIN_NUMBER;
    }

    if (rtn != OK)
    {
        dbgPrint(DBG_INFO, "Return was not OK. %s", gpioErrToString(rtn));
    }

    else if (gI2cMap != NULL)
    {
        dbgPrint(DBG_INFO, "gpioI2cSetup was already called.");
        rtn = ERROR_ALREADY_INITIALISED;
    }

    else if ((mem_fd = open("/dev/mem", O_RDWR)) < 0)
    {
        dbgPrint(DBG_INFO, "open() failed for /dev/mem. errno: %s.",
                 strerror(errno));
        rtn = ERROR_EXTERNAL;
    }

    else if ((gI2cMap = (volatile uint32_t *)mmap(NULL,
                                                  I2C_MAP_SIZE,
                                                  PROT_READ|PROT_WRITE,
                                                  MAP_SHARED,
                                                  mem_fd,
                                                  bscBase)) == MAP_FAILED)
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

    /* There are external Pullup resistors on the Pi. Disable the internals */
    else if ((rtn = gpioSetPullResistor(sda, pullDisable)) != OK)
    {
        dbgPrint(DBG_INFO, "gpioSetPullResistor() failed for SDA. %s",
                 gpioErrToString(rtn));
    }

    else if ((rtn = gpioSetPullResistor(scl, pullDisable)) != OK)
    {
        dbgPrint(DBG_INFO, "gpioSetPullResistor() failed for SCL. %s",
                 gpioErrToString(rtn));
    }

    /* Set SDA pin to alternate function 0 for I2C */
    else if ((rtn = gpioSetFunction(sda, alt0)) != OK)
    {
        dbgPrint(DBG_INFO, "gpioSetFunction() failed for SDA. %s",
                 gpioErrToString(rtn));
    }

    /* Set SCL pin to alternate function 0 for I2C */
    else if ((rtn = gpioSetFunction(scl, alt0)) != OK)
    {
        dbgPrint(DBG_INFO, "gpioSetFunction() failed for SCL. %s",
                gpioErrToString(rtn));
    }

    /* Default the I2C speed to 100 kHz */
    else if ((rtn = gpioI2cSetClock(I2C_DEFAULT_FREQ_HZ)) != OK)
    {
        dbgPrint(DBG_INFO, "gpioI2cSetClock() failed. %s", gpioErrToString(rtn));
    }

    else
    {
        /* Setup the Control Register.
         * Enable the BSC Controller.
         * Clear the FIFO. */
        I2C_C = BSC_I2CEN | BSC_CLEAR ;

        /* Setup the Status Register
         * Clear NACK ERR flag.
         * Clear Clock stretch flag.
         * Clear Done flag. */
        I2C_S = BSC_ERR | BSC_CLKT | BSC_DONE;

        rtn = OK;
    }

    return rtn;
}

/**
 * @brief   Disables the I2C controller and unmaps the memory used for the
 *          i2c functionality. This function should be called when finished
 *          with the I2C module.
 * @return  An error from #errStatus. */
errStatus gpioI2cCleanup(void)
{
    errStatus rtn = ERROR_DEFAULT;
    int sda;
    int scl;

    if (gI2cMap == NULL)
    {
        dbgPrint(DBG_INFO, "gI2cMap was NULL. Ensure gpioI2cSetup() was called successfully.");
        rtn = ERROR_NOT_INITIALISED;
    }

    else if ((rtn = gpioGetI2cPins(&scl, &sda)) != OK)
    {
        dbgPrint(DBG_INFO, "gpioGetI2cPins() failed. %s",
                 gpioErrToString(rtn));
    }

    /* Set SDA pin to input */
    else if ((rtn = gpioSetFunction(sda, input)) != OK)
    {
        dbgPrint(DBG_INFO, "gpioSetFunction() failed for SDA. %s",
                 gpioErrToString(rtn));
    }

    /* Set SCL pin to input */
    else if ((rtn = gpioSetFunction(scl, input)) != OK)
    {
        dbgPrint(DBG_INFO, "gpioSetFunction() failed for SCL. %s",
                gpioErrToString(rtn));
    }

    else
    {
        /* Disable the BSC Controller */
        I2C_C &= ~BSC_I2CEN;

        /* Unmap the memory */
        if (munmap((void *)gI2cMap, I2C_MAP_SIZE) != OK)
        {
            dbgPrint(DBG_INFO, "mummap() failed. errno: %s.", strerror(errno));
            rtn = ERROR_EXTERNAL;
        }

        else
        {
            gI2cMap = NULL;
            rtn = OK;
        }
    }

    return rtn;
}


/**
 * @brief               Sets the 7-bit slave address to communicate with.
 * @details             This value can be set once and left if communicating
 *                      with the same device.
 * @param slaveAddress  7-bit slave address.
 * @return              An error from #errStatus. */
errStatus gpioI2cSet7BitSlave(uint8_t slaveAddress)
{
    errStatus rtn = ERROR_DEFAULT;

    if (gI2cMap == NULL)
    {
        dbgPrint(DBG_INFO, "gI2cMap was NULL. Ensure gpioI2cSetup() was called successfully.");
        rtn = ERROR_NOT_INITIALISED;
    }

    else
    {
        I2C_A = slaveAddress;
        rtn = OK;
    }

    return rtn;
}


/**
 * @brief               Writes \p data to the address previously specified by
 *                      gpioI2cSet7BitSlave().
 * @param[in] data      Pointer to the start of data to transmit.
 * @param dataLength    The length of \p data.
 * @return errStatus    An error from #errStatus */
errStatus gpioI2cWriteData(const uint8_t * data, uint16_t dataLength)
{
    errStatus rtn = ERROR_DEFAULT;
    uint16_t dataIndex = 0;
    uint16_t dataRemaining = dataLength;
    struct timespec sleepTime;

    if (gI2cMap == NULL)
    {
        dbgPrint(DBG_INFO, "gI2cMap was NULL. Ensure gpioI2cSetup() was called successfully.");
        rtn = ERROR_NOT_INITIALISED;
    }

    else if (data == NULL)
    {
        dbgPrint(DBG_INFO, "data was NULL.");
        rtn = ERROR_NULL;
    }

    else
    {
        sleepTime.tv_sec  = 0;

        /* Clear the FIFO */
        I2C_C |= BSC_CLEAR;

        /* Configure Control for a write */
        I2C_C &= ~BSC_READ;

        /* Set the Data Length register to dataLength */
        I2C_DLEN = dataLength;

        /* Configure Control Register for a Start */
        I2C_C |= BSC_ST;

        /* Main transmit Loop - While Not Done */
        while (!(I2C_S & BSC_DONE))
        {
            while ((I2C_S & BSC_TXD) && dataRemaining)
            {
                I2C_FIFO = data[dataIndex];
                dataIndex++;
                dataRemaining--;
            }

            /* FIFO should be full at this point. If data remaining to be added
             * sleep for time it should take to approximately half empty FIFO */
            if (dataRemaining)
            {
                sleepTime.tv_nsec = i2cByteTxTime_ns * BSC_FIFO_SIZE / 2;
            }

            /* Otherwise all data is currently in the FIFO, sleep for how many
             * bytes are in the FIFO to be transmitted */ /* TODO DOUBLE? */
            else
            {
                sleepTime.tv_nsec = I2C_DLEN * i2cByteTxTime_ns;
            }

            nanosleep(&sleepTime, NULL);
        }

        /* Received a NACK */
        if (I2C_S & BSC_ERR)
        {
            I2C_S |= BSC_ERR;
            dbgPrint(DBG_INFO, "Received a NACK.");
            rtn = ERROR_I2C_NACK;
        }

        /* Received Clock Timeout error */
        else if (I2C_S & BSC_CLKT)
        {
            I2C_S |= BSC_CLKT;
            dbgPrint(DBG_INFO, "Received a Clock Stretch Timeout.");
            rtn = ERROR_I2C_CLK_TIMEOUT;
        }

        else if (dataRemaining)
        {
            dbgPrint(DBG_INFO, "BSC signaled done but %d data remained.", dataRemaining);
            rtn = ERROR_I2C;
        }

        else
        {
            rtn = OK;
        }

        /* Clear the DONE flag */
        I2C_S |= BSC_DONE;

    }

    return rtn;
}


/**
 * @brief               Read a number of bytes from I2C. The slave address
 *                      should have been previously set with gpioI2cSet7BitSlave().
 * @param[out] buffer   A pointer to a user defined buffer which will store the bytes.
 * @param bytesToRead   The number of bytes to read.
 * @return              An error from #errStatus.
 */
errStatus gpioI2cReadData(uint8_t * buffer, uint16_t bytesToRead)
{
    errStatus rtn = ERROR_DEFAULT;
    uint16_t bufferIndex = 0;
    uint16_t dataRemaining = bytesToRead;
    struct timespec sleepTime;

    if (gI2cMap == NULL)
    {
        dbgPrint(DBG_INFO, "gI2cMap was NULL. Ensure gpioI2cSetup() was called successfully.");
        rtn = ERROR_NOT_INITIALISED;
    }

    else if (buffer == NULL)
    {
        dbgPrint(DBG_INFO, "buffer was NULL.");
        rtn = ERROR_NULL;
    }

    else
    {
        sleepTime.tv_sec  = 0;

        /* Clear the FIFO */
        I2C_C |= BSC_CLEAR;

        /* Configure Control for a write */
        I2C_C |= BSC_READ;

        /* Set the Data Length register to dataLength */
        I2C_DLEN = bytesToRead;

        /* Configure Control Register for a Start */
        I2C_C |= BSC_ST;

        /* Main Receive Loop - While Transfer is not done */
        while (!(I2C_S & BSC_DONE))
        {
            /* FIFO Contains Data. Read until empty */
            while ((I2C_S & BSC_RXD) && dataRemaining)
            {
                buffer[bufferIndex] = I2C_FIFO;
                bufferIndex++;
                dataRemaining--;
            }

            /* FIFO should be empty at this point. If more than one full FIFO
             * remains to be read sleep for time to approximately half fill
             * FIFO */
            if (dataRemaining > BSC_FIFO_SIZE)
            {
                sleepTime.tv_nsec = i2cByteTxTime_ns * BSC_FIFO_SIZE / 2;
            }

            /* Otherwise, sleep for the number of bytes to be received */ /*TODO DOUBLE ?*/
            else
            {
                sleepTime.tv_nsec = I2C_DLEN * i2cByteTxTime_ns;
            }

            /* Sleep for approximate time to receive half the FIFO */
            sleepTime.tv_nsec = i2cByteTxTime_ns * (I2C_DLEN > BSC_FIFO_SIZE ?
                                                    BSC_FIFO_SIZE/2 : I2C_DLEN/2);

            nanosleep(&sleepTime, NULL);
        }

        /* FIFO Contains Data. Read until empty */
        while ((I2C_S & BSC_RXD) && dataRemaining)
        {
            buffer[bufferIndex] = I2C_FIFO;
            bufferIndex++;
            dataRemaining--;
        }

        /* Received a NACK */
        if (I2C_S & BSC_ERR)
        {
            I2C_S |= BSC_ERR;
            dbgPrint(DBG_INFO, "Received a NACK");
            rtn = ERROR_I2C_NACK;
        }

        /* Received Clock Timeout error. */
        else if (I2C_S & BSC_CLKT)
        {
            I2C_S |= BSC_CLKT;
            dbgPrint(DBG_INFO, "Received a Clock Stretch Timeout");
            rtn = ERROR_I2C_CLK_TIMEOUT;
        }

        else if (dataRemaining)
        {
            dbgPrint(DBG_INFO, "BSC signaled done but data remained.");
            rtn = ERROR_I2C;
        }

        else
        {
            rtn = OK;
        }

        /* Clear the DONE flag */
        I2C_S |= BSC_DONE;

    }

    return rtn;
}


/**
 * @brief           Sets the I2C Clock Frequency
 * @details         @note The desired frequency should be in the range:
 *                  #I2C_CLOCK_FREQ_MIN <= \p frequency <= #I2C_CLOCK_FREQ_MAX.
 * @param frequency Desired frequency in Hertz.
 * @return          An error from #errStatus */
errStatus gpioI2cSetClock(int frequency)
{
    errStatus rtn = ERROR_DEFAULT;

    /*
     * CDIV = 0 then diviser actually 32768
     * Max freq 400,000*/
    if (frequency < I2C_CLOCK_FREQ_MIN || frequency > I2C_CLOCK_FREQ_MAX)
    {
        rtn = ERROR_RANGE;
    }

    else
    {
         /*Note CDIV is always rounded down to an even number */
        I2C_DIV = CORE_CLK_HZ / frequency;
        i2cByteTxTime_ns = (int)(1.0 / ((float)frequency / NSEC_IN_SEC)
                                 * CLOCKS_PER_BYTE);

        rtn = OK;
    }


    return rtn;

}
