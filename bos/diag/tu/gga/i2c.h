/* @(#)77	1.1  src/bos/diag/tu/gga/i2c.h, tu_gla, bos41J, 9515A_all 4/6/95 09:27:09 */
/*
*
*   COMPONENT_NAME: tu_gla
*
*   FUNCTIONS: none
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
*
*/
/****************************************************************************
** I2C.H
**    Header file for use with I2C.C
****************************************************************************/

/****************************************************************************
** History (most recent first)
** ---------------------------
** May 13, 1994
**    First version
**
****************************************************************************/

/****************************************************************************
** Constants
****************************************************************************/
#define  SET_I2C_CLOCK  (0x03)   /* Control fields */
#define  CLR_I2C_CLOCK  (0x02)
#define  SET_I2C_DATA   (0x30)
#define  CLR_I2C_DATA   (0x20)

#define  GET_I2C_CLOCK  (8)      /* Field extraction */
#define  GET_I2C_DATA   (12)

#define  I2C_OK         (0x00)   /* Return codes */
#define  I2C_NOACK      (0x01)
#define  I2C_STUCK      (0x02)

#define  SAA7196        (0x40)   /* Slave addresses */
#define  CAMERA_ADDR    (0x20)   /* Slave addresses  */
#define  RESET_ADDR     (0x22)   /* Slave addresses  */

#define  RESET_CARD       0
#define  RESET_7196       1
#define  TURN_OFF_CAMERA  2
#define  TURN_ON_CAMERA   3

/****************************************************************************
** Prototypes
****************************************************************************/
int GetSDA();
int GetSCL();
int I2CDelay();
int SCLHigh();
int SCLLow();
int SDALow();
int SDAHigh();
int SendI2CStop();
int SendI2CByte(unsigned char);
int GoI2CMaster(unsigned char);
int SendI2CData(unsigned char, unsigned char *, int);
unsigned char RcvI2CByte(int);
int RcvI2CData(unsigned char, unsigned char *, int);

