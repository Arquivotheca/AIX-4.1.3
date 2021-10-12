static char sccsid[] = "@(#)76	1.1  src/bos/diag/tu/gga/i2c.c, tu_gla, bos41J, 9515A_all 4/6/95 09:27:07";
/*
 *   COMPONENT_NAME: tu_gla
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/****************************************************************************
** I2C.C
**    The I2C protocol is implemented is software for use in
**    Video Power systems. Video Power must be the only bus
**    master, all other devices must be slaves.
**
** Written by Keith Evans at Weitek based on 80C51 assembler code
** by G. Goodhue of Philips Semiconductor.
**
** NOTE: Purchase of Philips I2C components conveys a license under
**       the Philips I2C patent to use the components in the I2C
**       system provided the system conforms to the I2C specifications
**       defined by Philips. (U.S. Patent Number 4,689,740)
**
****************************************************************************/  

/****************************************************************************
** History (most recent first)
** ---------------------------
** May 13, 1994
**    First version
**
****************************************************************************/


/****************************************************************************
** Libraries
****************************************************************************/
#include "i2c.h"
#include "ggamisc.h"


/****************************************************************************
** GetSDA
**
** Arguments
**    None
**
** Return value
**    State of the external SDA pin, 0 or 1
**
** Description
**    Returns the state of the external SDA pin.
**
** Example
**    result = GetSDA();
****************************************************************************/
int GetSDA()
        {
        return((RL(W9130_I2C_INTERFACE) >> GET_I2C_DATA) & 0x01);
        }

/****************************************************************************
** GetSCL
**
** Arguments
**    None
**
** Return value
**    State of the external SCL pin, 0 or 1
**
** Description
**    Returns the state of the external SCL pin.
**
** Example
**    result = GetSCL();
****************************************************************************/
int GetSCL()
        {
        return((RL(W9130_I2C_INTERFACE) >> GET_I2C_CLOCK) & 0x01);
        }

/****************************************************************************
** I2CDelay
**
** Arguments
**    None
**
** Return value
**    I2C_OK
**
** Description
**    Delays for 5 microseconds to meet the minimum i2c clock high and
**    low times.
**
** Example
**    result = I2CDelay();
****************************************************************************/
int I2CDelay()
        {
        int i;

        for (i = 0 ; i < 10 ; i++)
                RL(W9130_ID);
        return(I2C_OK);
        }

/****************************************************************************
** SCLHigh
**
** Arguments
**    None
**
** Return value
**    I2C_OK if the SCL pin was pulled high
**    I2C_STUCK if the SCL pin is stuck low
**
** Description
**    Sets the Video Power SCL high and waits until the external SCL
**    is high in case any clock stretching peripherals are pulling
**    SCL low.
**
** Example
**    result = SCLHigh();
****************************************************************************/
int SCLHigh()
        {
        int i2cwatchdog = 0;

        WL(SET_I2C_CLOCK, W9130_I2C_INTERFACE);

        /* NOTE: I have put a very simple watchdog timer in this loop */
        /*       in case the SCL pin is stuck low. You may wish to    */
        /*       implement this differently.                          */
        
        while((GetSCL() == 0) && (i2cwatchdog != 0x100))
                        i2cwatchdog++;

        if (i2cwatchdog == 0x100)
                {
             /* Bapi_Report("SCLHigh: SCL stuck low.\n"); */
                return(I2C_STUCK);
                }
        I2CDelay();
        return(I2C_OK);
        }

/****************************************************************************
** SCLLow
**
** Arguments
**    None
**
** Return value
**    I2C_OK
**
** Description
**    Sets the Video Power SCL low.
**
** Example
**    result = SCLLow();
****************************************************************************/
int SCLLow()
        {
        WL(CLR_I2C_CLOCK, W9130_I2C_INTERFACE);
        I2CDelay();
        return(I2C_OK);
        }

/****************************************************************************
** SDALow
**
** Arguments
**    None
**
** Return value
**    I2C_OK
**
** Description
**    Sets the Video Power SDA low.
**
** Example
**    result = SDALow();
****************************************************************************/
int SDALow()
        {
        WL(CLR_I2C_DATA, W9130_I2C_INTERFACE);
        I2CDelay();
        return(I2C_OK);
        }

/****************************************************************************
** SDAHigh
**
** Arguments
**    None
**
** Return value
**    I2C_OK
**
** Description
**    Sets the Video Power SDA high.
**
** Example
**    result = SDAHigh();
****************************************************************************/
int SDAHigh()
        {
        WL(SET_I2C_DATA, W9130_I2C_INTERFACE);
        I2CDelay();
        return(I2C_OK);
        }


/****************************************************************************
** SendI2CStop
**
** Arguments
**    None
**
** Return value
**    I2C_OK if the STOP could be sent
**    I2C_STUCK if the STOP could not be sent
**
** Description
**    Sends an I2C stop to release the I2C bus.
**
** Example
**    result = SendI2CStop();
****************************************************************************/
int SendI2CStop()
        {
        int result;

        SDALow();               /* Prepare SDA for stop */
        result = SCLHigh();     /* Prepare SCL for stop */
        SDAHigh();              /* Send the stop */
        return(result);
        }

/****************************************************************************
** SendI2CByte
**
** Arguments
**    data - one byte of data to be sent
**
** Return value
**    I2C_OK if the byte could be sent
**    I2C_STUCK if the SCL pin was stuck
**    I2C_NOACK if the byte was not acknowledged
**
** Description
**    Sends one byte of data to an I2C slave.
**
** Example
**    result = SendI2CByte(0x00);
****************************************************************************/
int SendI2CByte(unsigned char data)
        {
        int i, result;

        for (i = 0 ; i < 8 ; i++)
                {
                if ((data & 0x80) == 0)  /* Send one data bit */
                        SDALow();
                else
                        SDAHigh();
                data = data << 1;
                result = SCLHigh();      /* Clock the data bit */
                SCLLow();
                }
        SDAHigh();                       /* Release the SDA line for acknowledge */
        result = SCLHigh();              /* Send acknowledge clock */
        if (GetSDA() == 1)               /* Check for acknowledge bit */
                {
             /* Bapi_Report("SendI2CByte: no acknowledge.\n"); */
                SCLLow();
                return(I2C_NOACK);
                }
        SCLLow();                        /* Finish the acknowledge clock */
        return(I2C_OK);
        }

/****************************************************************************
** GoI2CMaster
**
** Arguments
**    slave_address - the address of the slave to be selected
**
** Return value
**    I2C_OK if the slave was successfully selected
**    I2C_ACK if the slave failed to respond
**    I2C_STUCK if the SCL pin was stuck
**
** Description
**    Sends an I2C start and slave address.
**
** Example
**    result = GoI2CMaster(SAA7196);
****************************************************************************/
int GoI2CMaster(unsigned char slave_address)
        {
        /* Reset our I2C register if it was left in a busy state. */
        if ((RL(W9130_I2C_INTERFACE) & 0x33) != 0x33)
                WL(0x33, W9130_I2C_INTERFACE);

        /* Check for stuck I2C bus */
        if (GetSCL() == 0)
                {
             /* Bapi_Report("GoI2CMaster: I2C SCL stuck.\n"); */
                return(I2C_STUCK);
                }
        if (GetSDA() == 0)
                {
             /* Bapi_Report("GoI2CMaster: I2C SDA stuck.\n"); */
                SCLLow();                           /* Try to remove the stuck bit */
                SCLHigh();
                return(I2C_STUCK);
                }
        SDALow();                              /* Begin I2C start */
        SCLLow();
        return(SendI2CByte(slave_address));    /* Send slave address */
        }

/****************************************************************************
** SendI2CData
**
** Arguments
**    slave_address - the address of the slave to be selected
**    data - pointer to the data to be sent
**    count - the number of bytes of data to be sent
**
** Return value
**    I2C_OK if the data was sent successfully
**    I2C_ACK if the slave failed to respond
**    I2C_STUCK if the SCL pin was stuck
**
** Description
**    Sends an arbitrary number of bytes of data to an I2C slave device.
**
** Example
**    data[0] = SUBADDRESS;
**    data[1] = DATAWORD1;
**    data[1] = DATAWORD2;
**    result = SDAHigh(SAA7196, data, 3);
****************************************************************************/
int SendI2CData(unsigned char slave_address, unsigned char *data, int count)
        {
        int result;

        result = GoI2CMaster(slave_address);   /* Acquire slave for writing */
        if (result != I2C_OK)
                {
                SendI2CStop();                 /* Abort if no response */
                return(result);
                }

        while (count-- != 0)
                {
                result = SendI2CByte(*data++);
                if (result != I2C_OK)          /* Abort if no acknowledge */
                        {
                        SendI2CStop();
                        return(result);
                        }
                }
        SendI2CStop();                         /* Release the I2C bus */
        return(I2C_OK);
        }

/****************************************************************************
** RcvI2CByte
**
** Arguments
**    send_ack: 0 - don't send an acknowledgement
**              1 - send an acknowledgement
**
** Return value
**    Returns the byte of data received.
**
** Description
**    Receives one byte of data from an I2C slave.
**
** Example
**    result = RcvI2CByte(1);
****************************************************************************/
unsigned char RcvI2CByte(int send_ack)
        {
        unsigned char result = 0;
        int i;

        for (i = 0 ; i < 8 ; i++)
                {
                SCLHigh();         /* Clock out one data bit */
                result = (result << 1) | GetSDA();
                SCLLow();
                }
        if (send_ack != 0)         /* Send the acknowledge bit if required */
                SDALow();
        SCLHigh();                 /* Send the acknowledge clock */
        SCLLow();
        SDAHigh();                 /* Remove the acknowledge bit (if set) */
        return(result);
        }

/****************************************************************************
** RcvI2CData
**
** Arguments
**    slave_address - the address of the slave to be selected
**    data - pointer to the data storage area
**    count - the number of bytes of data to be received
**
** Return value
**    I2C_OK if the data was received successfully
**    I2C_ACK if the slave failed to respond
**    I2C_STUCK if the SCL pin was stuck
**
** Description
**    Receives an arbitrary number of bytes of data from an I2C slave device.
**
** Example
**    result = SDAHigh(SAA7196, &status, 1);
****************************************************************************/
int RcvI2CData(unsigned char slave_address, unsigned char *data, int count)
        {
        int result;

        if (count == 0)
                return(I2C_OK);
        result = GoI2CMaster(slave_address | 0x01);  /* Acquire the bus for reading */
        if (result != I2C_OK)
                {
                SendI2CStop();                       /* Abort if no response */
                return(result);
                }
        while (count-- != 1)
                *data++ = RcvI2CByte(1);
        *data++ = RcvI2CByte(0);
        SendI2CStop();                               /* Release the I2C bus */
        return(I2C_OK);
        }
